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
* @file cpssDxChMacSec.c
*
* @brief CPSS declarations relate to MAC Security (or MACsec) feature which is a 802.1AE IEEE
*        industry-standard security technology that provides secure communication for traffic
*        on Ethernet links.
*
*   NOTEs:
*   GT_NOT_INITIALIZED will be return for any 'MACSec' APIs if called before cpssDxChMacSecInit(..)
*   (exclude cpssDxChMacSecInit(...) itself)
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecUtils.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecLog.h>
#include <Integration/Adapter_EIP164/incl/api_driver164_init.h>
#include <Integration/Adapter_EIP163/incl/api_cfye_ext.h>
#include <Integration/Adapter_EIP164/incl/api_secy_ext.h>
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <Integration/Adapter_EIP164/incl/adapter_init.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*************************************************************************************************************
 Internal functions
 *************************************************************************************************************/


/**
* @internal prvCpssMacSecAdapterInit function
* @endinternal
*
* @brief   MACSec adapter layer initialization
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum          - device number.
* @param[in] unitBmp         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                              - 1: select DP0, 2: select DP1.
*                              - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                              - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                              - for non multi data paths device this parameter is IGNORED.
* @retval GT_OK              - on success
* @retval GT_FAIL            - on failure
*/
static GT_32 prvCpssMacSecAdapterInit(GT_U8 devNum, GT_U32 unitBmp)
{
    Adapter_Report_Build_Params();

    if (!Adapter_Init(devNum, unitBmp))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#ifdef ADAPTER_WARMBOOT_ENABLE
    if (!Adapter_Init_WarmBoot_Init())
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#endif

    return GT_OK;   /* success */
}

/**
* @internal prvCpssMacSecAdapterExit function
* @endinternal
*
* @brief   Uninitialize MACSec adapter layer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum          - device number.
* @param[in] unitBmp         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                              - 1: select DP0, 2: select DP1.
*                              - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                              - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                              - for non multi data paths device this parameter is IGNORED.
* @param[in] lastDevice      - indication if this the last device working with MACSec
* @retval GT_OK              - on success
* @retval GT_FAIL            - on failure
*/
static void prvCpssMacSecAdapterExit(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice)
{
#ifdef ADAPTER_WARMBOOT_ENABLE
    Adapter_Init_WarmBoot_Uninit();
#endif
    Adapter_UnInit(devNum, unitBmp, lastDevice);
}


/**
* @internal prvCpssMacSecVportVsSaCheck function
* @endinternal
*
* @brief   Check vPort vs SA.
*          Apply for Egress direction only.
*          Need to make sure vPort packet expansion parameter is aligned with SA action type and include sci parameter.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] vPortNum             - vPort number
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] saIncludeSci         - (pointer to) SA parameters
*
* @retval GT_OK              - on success
* @retval GT_BAD_PARAM       - on invalid parameters
*/
static GT_STATUS prvCpssDxChMacSecVportVsSaCheck
(
    IN  GT_U8                            devNum,
    IN  GT_MACSEC_UNIT_BMP               unitBmp,
    IN  GT_U32                           vPortNum,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC     *saParamsPtr
)
{
    GT_STATUS  rc;
    CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_ENT  vPortPktExpansion;
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT            saActionType;
    GT_BOOL                                        saIncludeSci;
    CfyE_vPortHandle_t                             cfyeVportHandle = CfyE_vPortHandle_NULL;
    CfyE_vPort_t                                   vPortParams;
    GT_U32                                         macSecUnitId;

    /* Get Classifier MACSec unit ID */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check vPortNum is in valid range */
    if (vPortNum >= PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[macSecUnitId]->vPortCount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vPortNum(%d) is bigger than maximum allowed number of vPorts(%d)"
                                      ,vPortNum,PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[macSecUnitId]->vPortCount);
    }

    /* Get vPort handle given vPort id */
    rc = CfyE_vPortHandle_Get(macSecUnitId, vPortNum, &cfyeVportHandle);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* Get vPort parameters associated with this vPort handle */
    rc = CfyE_vPort_Read(macSecUnitId, cfyeVportHandle, &vPortParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check below parameters */
    vPortPktExpansion = vPortParams.PktExtension;
    saActionType      = saParamsPtr->actionType;
    saIncludeSci      = saParamsPtr->params.egress.includeSci;

    /* For vPort packet expansion 0 SA action must be drop or bypass */
    if ( (vPortPktExpansion == CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E) &&
         ( (saActionType != CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E) && (saActionType != CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E) ) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vPort vs SA failure. For vPort pkt expansion 0, SA action must be bypass or drop,"
                           "saParams.ActionType:[%d], vPortParams.PktExtension:[%d]",saActionType,vPortPktExpansion);
    }

    /* For vPort packet expansion 24 and for SA action of egress type, SA include sci must be false */
    if ( (vPortPktExpansion == CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E) &&
         (saActionType == CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E) &&
         (saIncludeSci == GT_TRUE) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vPort vs SA failure. For vPort packet expansion 24, SA egress include sci must be false,"
                 "SA actionType:[%d], SA include SCI:[%d], vPort pkt expansion:[%d]", saActionType, saIncludeSci, vPortPktExpansion);
    }

    return rc;
}

/**
* @internal prvCpssMacSecSecyPortsBypassModeSet function
* @endinternal
*
* @brief   Set all ports to bypass mode in Transformer unit
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] macSecUnitId    - MACSec unit number
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - operation failed
*/
static GT_STATUS prvCpssMacSecSecyPortsBypassModeSet
(
    IN  GT_U32              macSecUnitId
)
{
    GT_STATUS    rc;
    GT_U32       maxChannels;
    GT_U32       chNum;

    /* Get maximum number of channels */
    rc = SecY_Device_Limits(macSecUnitId, &maxChannels, NULL, NULL, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Loop over all channels */
    for (chNum = 0; chNum < maxChannels; chNum++)
    {
        /* Set each channel to bypass mode */
        rc = SecY_Channel_Bypass_Set(macSecUnitId, chNum, GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}


/**
* @internal prvCpssMacSecUnInit function
* @endinternal
*
* @brief   Un initialize all MACSec units including setting to bypass mode all units in PCA EXT wrapper domain
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if MACSec DB was not initialized
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssMacSecUnInit
(
    IN  GT_U8               devNum,
    IN  GT_MACSEC_UNIT_BMP  unitBmp
)
{
    GT_STATUS     rc;
    GT_U32        macSecMaxNumDp;
    GT_U32        macSecDpId;
    GT_U32        macSecType;
    GT_U32        macSecDir;
    GT_U32        macSecUnitId;
    GT_U32        i;                /* loop counter used for walk through all devices */
    GT_BOOL       lastMacSecDevice; /* Indication if this is the last MACSec device going "UnInit" procedure */

    /* Get maximum number of Data path units connected to MACSec */
    macSecMaxNumDp = ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp) >> 1) + 1;

    /* Loop over MACSec Data paths */
    for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
    {
        /* Checks Data path bits map. Step in only if bit is on */
        if ( (unitBmp & (1 << macSecDpId)) != 0 )
        {
            /* Verify MACSec DB for this device and this DP was indeed initialized */
            if ((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->enabledMacSecDpBitmap & (1 << macSecDpId)) == 0)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* Loop over the maximum number of Data paths. For Hawk:2 for Phoenix:1  */
    for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
    {
        /* Check Data path bits map. Step in only if bit is on */
        if ( (unitBmp & (1 << macSecDpId)) != 0 )
        {
            /* Clear MACSec DB per device and per DP */
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->enabledMacSecDpBitmap &= (~(1 << macSecDpId));

            /* Loop over MACSec Classifier and MACSec Transformer */
            for (macSecType = 0; (macSecType < 2); macSecType++)
            {
                /* Loop over ingress and egress directions */
                for (macSecDir = 0; (macSecDir < 2); macSecDir++)
                {
                    /* Get MACSec device ID */
                    macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,macSecDpId,macSecType,macSecDir);

                    /* Check if MACSec Classifier or Transformer */
                    if (macSecType == CPSS_DXCH_MACSEC_CLASSIFIER_E)
                    {
                        /* Un initialize MACSec EIP-163 (Classifier) */
                        rc = CfyE_Device_Uninit(macSecUnitId);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* Un initialize MACSec EIP-164 (Transform) */
                        rc = SecY_Device_Uninit(macSecUnitId);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }

                    /* Now Set MACSec unit to bypass mode by setting macsec_bypass field (bit#0) in MacSec Wrapper Misc register */
                    rc = prvCpssMacSecExtByPassEnable(devNum, macSecDpId, macSecType, macSecDir, GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }

    /* Scan all devices to find out if this is the last MACSec device going "UnInit" procedure */
    for (i=0;i<PRV_CPSS_MAX_PP_DEVICES_CNS;i++)
    {
        /* Do the check only for device that does exist */
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(i))
        {
            /* Check DB state. If not zero then there is at least one device that is still on */
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[i]->enabledMacSecDpBitmap != 0)
            {
                break;
            }
        }
    }

    /* Set lastMacSecDevice indication */
    if (i == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* All entries are zero therefore this is the last MACSec device going "UnInit" procedure */
        lastMacSecDevice = GT_TRUE;
    }
    else
    {
        /* One of the entries is not zero, threrefore this is not the last device */
        lastMacSecDevice = GT_FALSE;
    }

    /* Free MACSec memory allocations */
    prvCpssMacSecAdapterExit(devNum, unitBmp, lastMacSecDevice);

    return GT_OK;
}

/**
* @internal prvCpssMacSecSecySaHandlesCompare function
* @endinternal
*
* @brief  Check if two SA handles are equal
*         Return true if both handles are equal, false otherwise
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] saHandle1    - First SA handle
* @param[in] saHandle2    - Second SA handle
*
* @retval GT_TRUE         - Handles are equal
* @retval GT_FALSE        - Handles are not equal
*/
GT_BOOL prvCpssMacSecSecySaHandlesCompare
(
    IN CPSS_DXCH_MACSEC_SECY_SA_HANDLE     saHandle1,
    IN CPSS_DXCH_MACSEC_SECY_SA_HANDLE     saHandle2
)
{
    GT_BOOL          compareOutput;
    SecY_SAHandle_t  secySaHandle1;
    SecY_SAHandle_t  secySaHandle2;

    /* Set handles for comparision */
    secySaHandle1.p = (void*)saHandle1;
    secySaHandle2.p = (void*)saHandle2;

    /* Compare SA handles */
    compareOutput = SecYLib_SAHandle_IsSame(&secySaHandle1, &secySaHandle2);

    return compareOutput;
}

/**
* @internal prvCpssMacSecClassifyTcamCountVersionGet function
* @endinternal
*
* @brief  Get Classifier TCAM count version information
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum             - device number.
* @param[in]  unitBmp            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                  - 1: select DP0, 2: select DP1.
*                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                  - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction          - select Egress or Ingress MACSec classifier
* @param[out] majorVersionPtr    - Major HW version
* @param[out] minorVersionPtr    - Minor HW version
* @param[out] patchLevelPtr      - HW patch level
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS prvCpssMacSecClassifyTcamCountVersionGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    OUT GT_U8                             *majorVersionPtr,
    OUT GT_U8                             *minorVersionPtr,
    OUT GT_U8                             *patchLevelPtr
)
{
    GT_STATUS               rc;
    GT_U32                  macSecUnitId;
    CfyE_Device_Limits_t    unitTcamVersion;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(majorVersionPtr);
    CPSS_NULL_PTR_CHECK_MAC(minorVersionPtr);
    CPSS_NULL_PTR_CHECK_MAC(patchLevelPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get TCAM count version information */
    rc = CfyE_Device_Limits_Get(macSecUnitId, &unitTcamVersion);

    /* Update output parameters */
    *majorVersionPtr = unitTcamVersion.major_version;
    *minorVersionPtr = unitTcamVersion.minor_version;
    *patchLevelPtr   = unitTcamVersion.patch_level;

    return rc;
}


/**
* @internal prvCpssMacSecClassifyVportHandlesCompare function
* @endinternal
*
* @brief  Check if two vPort handles are equal
*         Return true if both handles are equal, false otherwise
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] vPortHandle1    - First vPort handle
* @param[in] vPortHandle2    - Second vPort handle
*
* @retval GT_TRUE         - Handles are equal
* @retval GT_FALSE        - Handles are not equal
*/
GT_BOOL prvCpssMacSecClassifyVportHandlesCompare
(
    IN CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE     vPortHandle1,
    IN CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE     vPortHandle2
)
{
    GT_BOOL             compareOutput;
    CfyE_vPortHandle_t  classifyVportHandle1;
    CfyE_vPortHandle_t  classifyVportHandle2;

    /* Set handles for comparision */
    classifyVportHandle1 = (CfyE_vPortHandle_t)vPortHandle1;
    classifyVportHandle2 = (CfyE_vPortHandle_t)vPortHandle2;

    /* Compare vPort handles */
    compareOutput = CfyE_vPortHandle_IsSame(classifyVportHandle1, classifyVportHandle2);

    return compareOutput;
}


/**
* @internal prvCpssMacSecClassifyRuleHandlesCompare function
* @endinternal
*
* @brief  Check if two rule handles are equal
*         Return true if both handles are equal, false otherwise
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] ruleHandle1    - First rule handle
* @param[in] ruleHandle2    - Second rule handle
*
* @retval GT_TRUE         - Handles are equal
* @retval GT_FALSE        - Handles are not equal
*/
GT_BOOL prvCpssMacSecClassifyRuleHandlesCompare
(
    IN CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE     ruleHandle1,
    IN CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE     ruleHandle2
)
{
    GT_BOOL             compareOutput;
    CfyE_RuleHandle_t  classifyRuleHandle1;
    CfyE_RuleHandle_t  classifyRuleHandle2;

    /* Set handles for comparision */
    classifyRuleHandle1 = (CfyE_RuleHandle_t)ruleHandle1;
    classifyRuleHandle2 = (CfyE_RuleHandle_t)ruleHandle2;

    /* Compare rule handles */
    compareOutput = CfyE_RuleHandle_IsSame(classifyRuleHandle1, classifyRuleHandle2);

    return compareOutput;
}


/*************************************************************************************************************
 CPSS APIs
 *************************************************************************************************************/

/**
* @internal internal_cpssDxChMacSecInit function
* @endinternal
*
* @brief   MACSec devices initialization which include:
*          - Enable MACSec devices for work. Enable access to MACSec registers.
*          - Set initial configurations for Classifier & Transformer devices
*               - initialize all channels to pass packets in low-latency bypass mode (disable MACSec mode)
*               - clear all statistics counters and set all threshold counters to zero
*               - configure the statistics module to clear-on-read
*               - ECC thresholds are set to zero
*               - Automatic EOP insertion is disabled
*
*   NOTEs: - This API must be executed first before any other MACSec APIs
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if MACSec DB was already initialized
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChMacSecInit
(
    IN  GT_U8               devNum,
    IN  GT_MACSEC_UNIT_BMP  unitBmp
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecMaxNumDp;
    GT_U32                    macSecDpId;
    GT_U32                    macSecType;
    GT_U32                    macSecDir;
    GT_U32                    macSecUnitId;
    CfyE_Init_t               settingsCfyE;
    SecY_Settings_t           settingsSecY;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check unitBmp is valid */
    rc = prvCpssMacSecBmpCheck(devNum, &unitBmp, GT_TRUE/*multipe units are allowed*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get maximum number of Data path units connected to MACSec */
    macSecMaxNumDp = ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp) >> 1) + 1;

    /* Loop over MACSec Data paths */
    for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
    {
        /* Check Data path bits map. Step in only if bit is on */
        if ( (unitBmp & (1 << macSecDpId)) != 0 )
        {
            /* Checks if MACSec DB was already initialized for this device and this DP */
            if ((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->enabledMacSecDpBitmap & (1 << macSecDpId)) != 0)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* Initialize MACSec Classifier and Transformer DB */
    rc = prvCpssMacSecAdapterInit(devNum, unitBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Loop over the maximum number of Data paths. For Hawk:2 for Phoenix:1  */
    for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
    {
        /* Check Data path bits map. Step in only if bit is on */
        if ( (unitBmp & (1 << macSecDpId)) != 0 )
        {
             /* Loop over MACSec Classifier and MACSec Transformer */
            for (macSecType = 0; (macSecType < 2); macSecType++)
            {
                 /* Loop over ingress and egress directions */
                for (macSecDir = 0; (macSecDir < 2); macSecDir++)
                {
                    /* Configure MAC SEC EXT unit to enable MACSec feature */
                    /* Set to not bypass mode by clearing macsec_bypass field (bit#0) in MacSec Wrapper Misc register */
                    rc = prvCpssMacSecExtByPassEnable(devNum, macSecDpId, macSecType, macSecDir, GT_FALSE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* Get MACSec device ID */
                    macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,macSecDpId,macSecType,macSecDir);

                    /* Check if MACSec Classifier or Transformer */
                    if (macSecType == CPSS_DXCH_MACSEC_CLASSIFIER_E)
                    {
                        /* Initialize MACSec EIP-163 (Classifier) */
                        cpssOsMemSet(&settingsCfyE,0,sizeof(settingsCfyE));
                        /* Set all ports to bypass (no MACSec!!!) */
                        settingsCfyE.fLowLatencyBypass = GT_TRUE;
                        rc = CfyE_Device_Init(macSecUnitId, macSecDir, &settingsCfyE);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* Initialize MACSec EIP-164 (Transform) */
                        cpssOsMemSet(&settingsSecY,0,sizeof(settingsSecY));
                        rc = SecY_Device_Init(macSecUnitId, macSecDir, &settingsSecY);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                        /* At this point (after init) all ports are set to MACSec mode.
                           Since Classifier's ports are set to bypass mode the units are not aligned.
                           Therefore need to set all Transformer's ports to bypass mode as well */
                        rc = prvCpssMacSecSecyPortsBypassModeSet(macSecUnitId);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
            /* Update MACSec DB Initialization state per device and per DP */
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->enabledMacSecDpBitmap |= (1 << macSecDpId);
        }
    }

    rc = prvCpssDrvInterruptsDuringMacSecInit(devNum);

    return rc;
}

/**
* @internal cpssDxChMacSecInit function
* @endinternal
*
* @brief   MACSec devices initialization which include:
*          - Enable MACSec devices for work. Enable access to MACSec registers.
*          - Set initial configurations for Classifier & Transformer devices
*               - initialize all channels to pass packets in low-latency bypass mode (disable MACSec mode)
*               - clear all statistics counters and set all threshold counters to zero
*               - configure the statistics module to clear-on-read
*               - ECC thresholds are set to zero
*               - Automatic EOP insertion is disabled
*
*   NOTEs: - This API must be executed first before any other MACSec APIs
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if MACSec DB was already initialized
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMacSecInit
(
    IN  GT_U8               devNum,
    IN  GT_MACSEC_UNIT_BMP  unitBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp));

    rc = internal_cpssDxChMacSecInit(devNum, unitBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChMacSecPortClassifyConfigSet function
* @endinternal
*
* @brief   Configuration settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] portCfgPtr           - (pointer to) port configuratios parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_Device_Control_t    unitControlParams;
    CfyE_Device_Exceptions_t unitExceptionsParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(portCfgPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitControlParams, 0, sizeof(unitControlParams));
    cpssOsMemSet(&unitExceptionsParams, 0, sizeof(unitExceptionsParams));

    /* Update parameters with MACSec mode (bypass or MACSec mode) */
    unitControlParams.fLowLatencyBypass = portCfgPtr->bypassMacsecDevice;

    /* Update exception parameters only if exceptionCfgEnable flag is enabled */
    if(portCfgPtr->exceptionCfgEnable)
    {
        /* Set drop action always to internal drop by crypto-core (packet is not seen outside) */
        unitExceptionsParams.DropAction         = CFYE_DROP_INTERNAL;
        unitExceptionsParams.fForceDrop         = portCfgPtr->forceDrop;
        unitExceptionsParams.fDefaultVPortValid = portCfgPtr->defaultVPortValid;
        unitExceptionsParams.DefaultVPort       = portCfgPtr->defaultVPort;
        unitControlParams.Exceptions_p = &unitExceptionsParams;
    }

    /* Update control params pointer */
    unitTopParams.Control_p = &unitControlParams;

    /* Call Classifier update API to apply these parameters */
    rc = CfyE_Device_Update(macSecUnitId, macSecChannelNum, &unitTopParams);

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyConfigSet function
* @endinternal
*
* @brief   Configuration settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] portCfgPtr           - (pointer to) port configuratios parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, portCfgPtr));

    rc = internal_cpssDxChMacSecPortClassifyConfigSet(devNum, portNum, direction, portCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, portCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyConfigGet function
* @endinternal
*
* @brief   Get configuration settings per port from MACSec classifier
*          note: exceptionCfgEnable flag is ignored by function and all
*                exception parameters are read in any case
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[OUT] portCfgPtr           - (pointer to) port configurations parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyConfigGet
(
    IN   GT_U8                                   devNum,
    IN   GT_PHYSICAL_PORT_NUM                    portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_Device_Control_t    unitControlParams;
    CfyE_Device_Exceptions_t unitExceptionsParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(portCfgPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitControlParams, 0, sizeof(unitControlParams));
    cpssOsMemSet(&unitExceptionsParams, 0, sizeof(unitExceptionsParams));

    /* Set pointers to structures  */
    unitTopParams.Control_p        = &unitControlParams;
    unitControlParams.Exceptions_p = &unitExceptionsParams;

    /* Call Classifier API to get port configuration parameters */
    rc = CfyE_Device_Config_Get(macSecUnitId, macSecChannelNum, &unitTopParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get port MACSec mode (bypass or MACSec mode) */
    portCfgPtr->bypassMacsecDevice = unitTopParams.Control_p->fLowLatencyBypass;

    /* Get port exception parameters */
    portCfgPtr->forceDrop         = unitTopParams.Control_p->Exceptions_p->fForceDrop;
    portCfgPtr->defaultVPortValid = unitTopParams.Control_p->Exceptions_p->fDefaultVPortValid;
    portCfgPtr->defaultVPort      = unitTopParams.Control_p->Exceptions_p->DefaultVPort;

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyConfigGet function
* @endinternal
*
* @brief   Get configuration settings per port from MACSec classifier
*          note: exceptionCfgEnable flag is ignored by function and all
*                exception parameters are read in any case
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[OUT] portCfgPtr           - (pointer to) port configurations parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyConfigGet
(
    IN   GT_U8                                   devNum,
    IN   GT_PHYSICAL_PORT_NUM                    portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, portCfgPtr));

    rc = internal_cpssDxChMacSecPortClassifyConfigGet(devNum, portNum, direction, portCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, portCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportAdd function
* @endinternal
*
* @brief   Adds a new vPort (vPort policy) set for one MACSec Classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortParamsPtr      - (pointer to) data for the new vPort
* @param[out] vPortHandlePtr      - (pointer to) vPort handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    CfyE_vPort_t              vPortParams;
    CfyE_vPortHandle_t        cfyeVportHandle = CfyE_vPortHandle_NULL;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vPortParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(vPortHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* For ingress direction packet expansion should be configured to no expansion ('zero') */
    if ( (direction == CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E) &&
         (vPortParamsPtr->pktExpansion != CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E)  )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "For ingress direction the packet expansion should be configured to no expansion ('zero')");
    }


    /* Clear structure before filling it  */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* Update vPort structure with vPort parameters */
    vPortParams.SecTagOffset = vPortParamsPtr->secTagOffset;
    vPortParams.PktExtension = vPortParamsPtr->pktExpansion;

    /* Call Classifier API to add vPort policy */
    rc = CfyE_vPort_Add(macSecUnitId, &cfyeVportHandle, &vPortParams);

    /* Get Classifier vPort handle */
    *vPortHandlePtr = (CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE)cfyeVportHandle;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportAdd function
* @endinternal
*
* @brief   Adds a new vPort (vPort policy) set for one MACSec Classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortParamsPtr      - (pointer to) data for the new vPort
* @param[out] vPortHandlePtr      - (pointer to) vPort handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortParamsPtr,vPortHandlePtr));

    rc = internal_cpssDxChMacSecClassifyVportAdd(devNum, unitBmp, direction, vPortParamsPtr, vPortHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortParamsPtr, vPortHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportRemove function
* @endinternal
*
* @brief   Removes an already added vPort (vPort policy) from one MACSec classifier
*
*   NOTEs: - vPort can be removed from MACSec classifier instance only after it has been added to it
*          - Before this function can be called all the Rules associated with this vPort must be removed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - the vPort handle to remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportRemove
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle
)
{
    GT_STATUS             rc;
    GT_U32                macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Call Classifier API to remove rule */
    rc = CfyE_vPort_Remove(macSecUnitId, (CfyE_vPortHandle_t)vPortHandle);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportRemove function
* @endinternal
*
* @brief   Removes an already added vPort (vPort policy) from one MACSec classifier
*
*   NOTEs: - vPort can be removed from MACSec classifier instance only after it has been added to it
*          - Before this function can be called all the Rules associated with this vPort must be removed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - the vPort handle to remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportRemove
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortHandle));

    rc = internal_cpssDxChMacSecClassifyVportRemove(devNum, unitBmp, direction, vPortHandle);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortHandle));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportIndexGet function
* @endinternal
*
* @brief   Get the vPort index from an existing vPort, using the vPort handle
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number
* @param[in]  vPortHandle         - this handle is a reference to a vPort
* @param[out] vPortIndexPtr       - (pointer to) vPort index
*
* @retval GT_OK                   - on success
* @retval GT_BAD_PTR              - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM            - on wrong parameter
* @retval GT_NOT_INITIALIZED      - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportIndexGet
(
    IN   GT_U32                                  devNum,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  GT_U32                                  *vPortIndexPtr
)
{
    GT_STATUS   rc;
    GT_U32      vPortId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vPortIndexPtr);

    /* Now get vPort index */
    rc = CfyE_vPortIndex_Get((CfyE_vPortHandle_t)vPortHandle, &vPortId);

    /* The requested vPort number */
    *vPortIndexPtr = vPortId;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportIndexGet function
* @endinternal
*
* @brief   Get the vPort index from an existing vPort, using the vPort handle
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number
* @param[in]  vPortHandle         - this handle is a reference to a vPort
* @param[out] vPortIndexPtr       - (pointer to) vPort index
*
* @retval GT_OK                   - on success
* @retval GT_BAD_PTR              - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM            - on wrong parameter
* @retval GT_NOT_INITIALIZED      - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportIndexGet
(
    IN   GT_U32                                  devNum,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  GT_U32                                  *vPortIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum, vPortHandle, vPortIndexPtr));

    rc = internal_cpssDxChMacSecClassifyVportIndexGet(devNum, vPortHandle, vPortIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vPortHandle, vPortIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleAdd function
* @endinternal
*
* @brief   Adds a new rule for matching a packet to a vPort identified by vPortHandle
*          for one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for the vPort where the packet matching rule must be added.
* @param[in]  ruleParamsPtr       - (pointer to) data for the rule.
* @param[out] ruleHandlePtr       - (pointer to) rule handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
)
{
    GT_STATUS             rc;
    GT_U32                macSecUnitId;
    CfyE_Rule_t           ruleParams;
    CfyE_RuleHandle_t     cfyeRuleHandle = CfyE_RuleHandle_NULL;
    GT_U32                channelNum;
    GT_U32                dpId;
    GT_U8                 byteArr[4];
    GT_U32                i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ruleParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));

    /* Update rule structure with rule parameters */
    ruleParams.Policy.fControlPacket = ruleParamsPtr->policy.controlPacket;
    ruleParams.Policy.fDrop          = ruleParamsPtr->policy.drop;
    ruleParams.Policy.Priority       = ruleParamsPtr->policy.rulePriority;
    ruleParams.Policy.vPortHandle    = (CfyE_vPortHandle_t)vPortHandle;

    ruleParams.Mask.NumTags          = ruleParamsPtr->mask.numTags;
    ruleParams.Key.NumTags           = ruleParamsPtr->key.numTags;

    ruleParams.Mask.PacketType       = ruleParamsPtr->mask.packetType;
    ruleParams.Key.PacketType        = ruleParamsPtr->key.packetType;

    ruleParams.Mask.ChannelID  = ruleParamsPtr->mask.portNum;

    /* Check mask of portNum. In case it is not zero need to convert physical port number into channel number */
    if(ruleParamsPtr->mask.portNum !=0 )
    {
        /* Check portNum is valid and get local channel */
        rc = prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert(devNum, ruleParamsPtr->key.portNum, &dpId, &channelNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set channel ID */
        ruleParams.Key.ChannelID  = channelNum;
    }

    /* Need to arrange data in little endian since TCAM packet data is stored in little endian byte order */
    for(i=0;i<CFYE_RULE_NON_CTRL_WORD_COUNT;i++)
    {
        /* Extract each byte and place it in little endian byte order */
        byteArr[0] = (GT_U8)( (ruleParamsPtr->data[i]>>24) & 0xFF );
        byteArr[1] = (GT_U8)( (ruleParamsPtr->data[i]>>16) & 0xFF );
        byteArr[2] = (GT_U8)( (ruleParamsPtr->data[i]>>8)  & 0xFF );
        byteArr[3] = (GT_U8)( (ruleParamsPtr->data[i])     & 0xFF );
        ruleParams.Data[i] = (byteArr[0] | byteArr[1]<<8 | byteArr[2]<<16 | byteArr[3]<<24) ;

        /* Now do it for the data mask */
        byteArr[0] = (GT_U8)( (ruleParamsPtr->dataMask[i]>>24) & 0xFF );
        byteArr[1] = (GT_U8)( (ruleParamsPtr->dataMask[i]>>16) & 0xFF );
        byteArr[2] = (GT_U8)( (ruleParamsPtr->dataMask[i]>>8)  & 0xFF );
        byteArr[3] = (GT_U8)( (ruleParamsPtr->dataMask[i])     & 0xFF );
        ruleParams.DataMask[i] = (byteArr[0] | byteArr[1]<<8 | byteArr[2]<<16 | byteArr[3]<<24) ;
    }

    /* Call Classifier API to add vPort policy */
    rc = CfyE_Rule_Add(macSecUnitId, (CfyE_vPortHandle_t)vPortHandle, &cfyeRuleHandle, &ruleParams);

    /* Get Classifier rule handle */
    *ruleHandlePtr = (CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE)cfyeRuleHandle;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleAdd function
* @endinternal
*
* @brief   Adds a new rule for matching a packet to a vPort identified by vPortHandle
*          for one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for the vPort where the packet matching rule must be added.
* @param[in]  ruleParamsPtr       - (pointer to) data for the rule.
* @param[out] ruleHandlePtr       - (pointer to) rule handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortHandle, ruleParamsPtr,ruleHandlePtr));

    rc = internal_cpssDxChMacSecClassifyRuleAdd(devNum, unitBmp, direction, vPortHandle, ruleParamsPtr,ruleHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortHandle, ruleParamsPtr,ruleHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleRemove function
* @endinternal
*
* @brief   Removes an already added rule from one MACSec classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - rule handle for the rule to be removed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleRemove
(
    IN  GT_U8                                  devNum,
    IN  GT_MACSEC_UNIT_BMP                     unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT         direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE  ruleHandle
)
{
    GT_STATUS             rc;
    GT_U32                macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Call Classifier API to remove rule */
    rc = CfyE_Rule_Remove(macSecUnitId, (CfyE_RuleHandle_t)ruleHandle);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleRemove function
* @endinternal
*
* @brief   Removes an already added rule from one MACSec classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - rule handle for the rule to be removed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleRemove
(
    IN  GT_U8                                  devNum,
    IN  GT_MACSEC_UNIT_BMP                     unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT         direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE  ruleHandle
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleHandle));

    rc = internal_cpssDxChMacSecClassifyRuleRemove(devNum, unitBmp, direction, ruleHandle);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleHandle));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleEnable function
* @endinternal
*
* @brief   Enables or disable an existing rule in one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - Rule handle for rule to be enabled/disabled.
* @param[in] enable               - GT_TRUE for enable or GT_FALSE for disable rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleEnable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;
    GT_BOOL         sync;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Option to track the number of packets in the pipeline for hardware/software synchronization purposes
       Do not synchronize with MACSec unit */
    sync = GT_FALSE;

    if(enable)
    {
        /* Call Classifier API to enable TCAM rule */
        rc = CfyE_Rule_Enable(macSecUnitId, (CfyE_RuleHandle_t)ruleHandle, sync);
    }
    else
    {
        /* Call Classifier API to disable TCAM rule */
        rc = CfyE_Rule_Disable(macSecUnitId, (CfyE_RuleHandle_t)ruleHandle, sync);
    }

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleEnable function
* @endinternal
*
* @brief   Enables or disable an existing rule in one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - Rule handle for rule to be enabled/disabled.
* @param[in] enable               - GT_TRUE for enable or GT_FALSE for disable rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleEnable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  GT_BOOL                                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleHandle, enable));

    rc = internal_cpssDxChMacSecClassifyRuleEnable(devNum, unitBmp, direction, ruleHandle, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleHandle, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleIndexGet function
* @endinternal
*
* @brief   Get the Rule index from an existing Rule, using the rule handle.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  ruleHandle           - rule handle of the rule to get the index from.
* @param[out] ruleIndexPtr         - (pointer to) rule index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleIndexGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT GT_U32                                  *ruleIndexPtr
)
{
    GT_STATUS   rc;
    GT_U32      ruleId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ruleIndexPtr);

    /* Now get rule index */
    rc = CfyE_RuleIndex_Get((CfyE_RuleHandle_t)ruleHandle, &ruleId);

    /* The requested rule number */
    *ruleIndexPtr = ruleId;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleIndexGet function
* @endinternal
*
* @brief   Get the Rule index from an existing Rule, using the rule handle.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  ruleHandle           - rule handle of the rule to get the index from.
* @param[out] ruleIndexPtr         - (pointer to) rule index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleIndexGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT GT_U32                                  *ruleIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ruleHandle, ruleIndexPtr));

    rc = internal_cpssDxChMacSecClassifyRuleIndexGet(devNum, ruleHandle, ruleIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ruleHandle, ruleIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaAdd function
* @endinternal
*
* @brief   Adds new SA set for one MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] vPortId              - vPort number to which the SA applies.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] saParamsPtr          - pointer to a memory location where the data for the new SA is stored
* @param[in] trRecParamsPtr       - (pointer to) SecY Transform Record parameters
*                                    Ignored for bypass and drop actions
* @param[out] saHandlePtr         - (pointer to) SA handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaAdd
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  GT_U32                              vPortId,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC        *saParamsPtr,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *trRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *saHandlePtr
)
{
    GT_STATUS                        rc;
    GT_U32                           macSecUnitId;
    SecY_SAHandle_t                  secySaHandle = SecY_SAHandle_NULL;
    SecY_SA_t                        saParams;
    GT_U32                           *trRecBufferPtr = NULL;
    GT_U32                           saWordCount = 0;
    PRV_CPSS_SECY_TR_REC_PARAMS_STC  trRecParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointers are not NULL */
    CPSS_NULL_PTR_CHECK_MAC(saParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(saHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* If egress, check vPort parameters are aligned with current SA parameters */
    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        rc = prvCpssDxChMacSecVportVsSaCheck(devNum, unitBmp, vPortId, saParamsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Clear structures before filling them */
    cpssOsMemSet(&saParams, 0, sizeof(saParams));
    cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));

    /* Set transform record data. Skip it if action type is Bypass or Drop */
    if ( (saParamsPtr->actionType != CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E) &&
         (saParamsPtr->actionType != CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E) )
    {
        /* Check pointer is not NULL */
        CPSS_NULL_PTR_CHECK_MAC(trRecParamsPtr);

        /* Set transform record parameters */
        trRecParams.direction     = direction;
        trRecParams.an            = trRecParamsPtr->an;
        trRecParams.keyPtr        = trRecParamsPtr->keyArr;
        trRecParams.keyByteCount  = trRecParamsPtr->keyByteCount;
        trRecParams.sciPtr        = trRecParamsPtr->sciArr;
        trRecParams.seqNumLo      = trRecParamsPtr->seqNumLo;
        trRecParams.seqMask       = trRecParamsPtr->seqMask;

        /* Set Hash key pointer to point to custom Hash key array in case custom
           Hash key is required otherwise Hash key pointer will remain NULL */
        if (trRecParamsPtr->customHkeyEnable)
        {
            trRecParams.hKeyPtr = trRecParamsPtr->customHkeyArr;
        }

        /* Check packet numbering is of extended type (64 bits) */
        if (trRecParamsPtr->seqTypeExtended)
        {
            /* Set the relevant parameters for extended type */
            trRecParams.seqNumHi = trRecParamsPtr->seqNumHi;
            trRecParams.ssciPtr  = trRecParamsPtr->ssciArr;
            trRecParams.saltPtr  = trRecParamsPtr->saltArr;
        }

        /* Build SA transform record. Return pointer to buffer containing this transform record and size of record in 4B words */
        rc = prvCpssMacSecBuildSaTransformRecord(&trRecParams,&saWordCount,&trRecBufferPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Update SA transform record parameters */
        saParams.TransformRecord_p = trRecBufferPtr;
        saParams.SA_WordCount      = saWordCount;
    }
    else
    {
        /* Update SA transform record parameters in case of bypass/drop action */
        saParams.TransformRecord_p = NULL;
        saParams.SA_WordCount      = 0;
    }

    /* Update SA structure with SA parameters */
    saParams.ActionType        = saParamsPtr->actionType;
    saParams.DestPort          = saParamsPtr->destPort;
    saParams.DropType          = SECY_SA_DROP_INTERNAL;

    /* Keep update according to SA action type */
    switch(saParamsPtr->actionType)
    {
        /* SA action egress */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E:
            saParams.Params.Egress.ConfidentialityOffset = saParamsPtr->params.egress.confidentialityOffset;
            saParams.Params.Egress.fAllowDataPkts        = saParamsPtr->params.egress.controlledPortEnable;
            saParams.Params.Egress.fConfProtect          = saParamsPtr->params.egress.confProtect;
            saParams.Params.Egress.fIncludeSCI           = saParamsPtr->params.egress.includeSci;
            saParams.Params.Egress.fProtectFrames        = saParamsPtr->params.egress.protectFrames;
            saParams.Params.Egress.fSAInUse              = saParamsPtr->params.egress.saInUse;
            saParams.Params.Egress.fUseES                = saParamsPtr->params.egress.useEs;
            saParams.Params.Egress.fUseSCB               = saParamsPtr->params.egress.useScb;
            saParams.Params.Egress.PreSecTagAuthLength   = saParamsPtr->params.egress.preSecTagAuthLength;
            saParams.Params.Egress.PreSecTagAuthStart    = saParamsPtr->params.egress.preSecTagAuthStart;
            break;

        /* SA action ingress */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E:
            saParams.Params.Ingress.AN                    = saParamsPtr->params.ingress.an;
            saParams.Params.Ingress.ConfidentialityOffset = saParamsPtr->params.ingress.confidentialityOffset;
            saParams.Params.Ingress.fAllowTagged          = saParamsPtr->params.ingress.allowTagged;
            saParams.Params.Ingress.fAllowUntagged        = saParamsPtr->params.ingress.allowUntagged;
            saParams.Params.Ingress.fReplayProtect        = saParamsPtr->params.ingress.replayProtect;
            saParams.Params.Ingress.fRetainICV            = saParamsPtr->params.ingress.retainIcv;
            saParams.Params.Ingress.fRetainSecTAG         = saParamsPtr->params.ingress.retainSecTag;
            saParams.Params.Ingress.fSAInUse              = saParamsPtr->params.ingress.saInUse;
            saParams.Params.Ingress.fValidateUntagged     = saParamsPtr->params.ingress.validateUntagged;
            saParams.Params.Ingress.PreSecTagAuthLength   = saParamsPtr->params.ingress.preSecTagAuthLength;
            saParams.Params.Ingress.PreSecTagAuthStart    = saParamsPtr->params.ingress.preSecTagAuthStart;
            saParams.Params.Ingress.SCI_p                 = saParamsPtr->params.ingress.sciArr;
            saParams.Params.Ingress.ValidateFramesTagged  = saParamsPtr->params.ingress.validateFramesTagged;
            break;

        /* SA action bypass drop */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E:
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E:
            saParams.Params.BypassDrop.fSAInUse = saParamsPtr->params.bypassDrop.saInUse;
            break;

        /* SA action crypt-auth */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_CRYPT_AUTH_E:
            saParams.Params.CryptAuth.ConfidentialityOffset = saParamsPtr->params.cryptAuth.confidentialityOffset;
            saParams.Params.CryptAuth.fConfProtect          = saParamsPtr->params.cryptAuth.confProtect;
            saParams.Params.CryptAuth.fICVAppend            = saParamsPtr->params.cryptAuth.icvAppend;
            saParams.Params.CryptAuth.fICVVerify            = saParamsPtr->params.cryptAuth.icvVerify;
            saParams.Params.CryptAuth.fZeroLengthMessage    = saParamsPtr->params.cryptAuth.zeroLengthMessage;
            saParams.Params.CryptAuth.IVMode                = saParamsPtr->params.cryptAuth.ivMode;
            break;

        default:
            /* If allocated then must free transform record buffer */
            if (trRecBufferPtr)
            {
                cpssOsFree(trRecBufferPtr);
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid SA action type:[%d]", saParams.ActionType);
    }

    /* Call Transformer API to add SA */
    rc = SecY_SA_Add(macSecUnitId, vPortId, &secySaHandle, &saParams);

    /* If allocated then must free transform record buffer */
    if (trRecBufferPtr)
    {
        cpssOsFree(trRecBufferPtr);
    }

    /* Get Transformer SA handle */
    *saHandlePtr = (CPSS_DXCH_MACSEC_SECY_SA_HANDLE)secySaHandle.p;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaAdd function
* @endinternal
*
* @brief   Adds new SA set for one MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] vPortId              - vPort number to which the SA applies.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] saParamsPtr          - pointer to a memory location where the data for the new SA is stored
* @param[in] trRecParamsPtr       - (pointer to) SecY Transform Record parameters
*                                    Ignored for bypass and drop actions
* @param[out] saHandlePtr         - (pointer to) SA handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecySaAdd
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  GT_U32                              vPortId,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC        *saParamsPtr,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *trRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *saHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortId, saParamsPtr,trRecParamsPtr,saHandlePtr));

    rc = internal_cpssDxChMacSecSecySaAdd(devNum, unitBmp, direction, vPortId, saParamsPtr,trRecParamsPtr,saHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortId, saParamsPtr,trRecParamsPtr,saHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaRemove function
* @endinternal
*
* @brief   Removes SA that was already added from one MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - the SA handle to remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaRemove
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle
)
{
    GT_STATUS             rc;
    GT_U32                macSecUnitId;
    SecY_SAHandle_t       secySaHandle;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Call Transformer API to remove SA */
    rc = SecY_SA_Remove(macSecUnitId, secySaHandle);

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaRemove function
* @endinternal
*
* @brief   Removes SA that was already added from one MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - the SA handle to remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaRemove
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saHandle));

    rc = internal_cpssDxChMacSecSecySaRemove(devNum, unitBmp, direction, saHandle);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saHandle));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyStatisticsTcamHitsGet function
* @endinternal
*
* @brief   This API reads the MACSec classifier TCAM hits statistics.
*          TCAM Hit counter per each TCAM entry using the TCAM statistics module.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                 - device number.
* @param[in]  unitBmp                - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                       - 1: select DP0, 2: select DP1.
*                                       - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                       - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction              - select Egress or Ingress MACSec classifier
* @param[in]  ruleId                 - The statistics counter index (number) to read the statistics for.
*                                      (APPLICABLE RANGES: AC5P 0..511)
*                                      (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[in]  syncEnable             - If true, ensure that all packets available in this MACSec unit at the time
*                                      of the API call are processed before the statistics are read
* @param[out] statTcamHitsCounterPtr - (pointer to) classifier Statistics counter per TCAM entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyStatisticsTcamHitsGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           ruleId,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statTcamHitsCounterPtr
)
{
    GT_STATUS              rc;
    GT_U32                 macSecUnitId;
    CfyE_Statistics_TCAM_t tcamHitsCounter;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statTcamHitsCounterPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it */
    cpssOsMemSet(&tcamHitsCounter, 0, sizeof(tcamHitsCounter));

    /* Call Classifier API to get TCAM hits statistics */
    rc = CfyE_Statistics_TCAM_Get(macSecUnitId, ruleId, &tcamHitsCounter, syncEnable);

    /* Update output: set TCAM hits counter */
    statTcamHitsCounterPtr->l[0] = tcamHitsCounter.Counter.Lo;
    statTcamHitsCounterPtr->l[1] = tcamHitsCounter.Counter.Hi;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyStatisticsTcamHitsGet function
* @endinternal
*
* @brief   This API reads the MACSec classifier TCAM hits statistics.
*          TCAM Hit counter per each TCAM entry using the TCAM statistics module.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                 - device number.
* @param[in]  unitBmp                - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                       - 1: select DP0, 2: select DP1.
*                                       - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                       - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction              - select Egress or Ingress MACSec classifier
* @param[in]  ruleId                 - The statistics counter index (number) to read the statistics for.
*                                      (APPLICABLE RANGES: AC5P 0..511)
*                                      (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[in]  syncEnable             - If true, ensure that all packets available in this MACSec unit at the time
*                                      of the API call are processed before the statistics are read
* @param[out] statTcamHitsCounterPtr - (pointer to) classifier Statistics counter per TCAM entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatisticsTcamHitsGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           ruleId,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statTcamHitsCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyStatisticsTcamHitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleId, syncEnable, statTcamHitsCounterPtr));

    rc = internal_cpssDxChMacSecClassifyStatisticsTcamHitsGet(devNum, unitBmp, direction, ruleId, syncEnable, statTcamHitsCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleId, syncEnable, statTcamHitsCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyStatisticsGet function
* @endinternal
*
* @brief   This API reads the MACSec classifier per-port statistics.
*          Port counters are counting events that for debugging purpose should be accounted per port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  portNum             - The physical port number to read the statistics for.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] portStatPtr         - (pointer to) Classifier statistics counter identified by portNum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS internal_cpssDxChMacSecPortClassifyStatisticsGet
(
    IN   GT_U8                                          devNum,
    IN   GT_PHYSICAL_PORT_NUM                           portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                 direction,
    IN   GT_BOOL                                        syncEnable,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC  *portStatPtr
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    CfyE_Statistics_Channel_t channelCounter;
    GT_U32                    macSecChannelNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(portStatPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it */
    cpssOsMemSet(&channelCounter, 0, sizeof(channelCounter));

    /* Call Classifier API to get TCAM hits statistics */
    rc = CfyE_Statistics_Channel_Get(macSecUnitId, macSecChannelNum, &channelCounter, syncEnable);

    /* Update output: set channel counters */
    portStatPtr->pktsCtrl.l[0] = channelCounter.PktsCtrl.Lo;
    portStatPtr->pktsCtrl.l[1] = channelCounter.PktsCtrl.Hi;

    portStatPtr->pktsData.l[0] = channelCounter.PktsData.Lo;
    portStatPtr->pktsData.l[1] = channelCounter.PktsData.Hi;

    portStatPtr->pktsDropped.l[0] = channelCounter.PktsDropped.Lo;
    portStatPtr->pktsDropped.l[1] = channelCounter.PktsDropped.Hi;

    portStatPtr->pktsErrIn.l[0] = channelCounter.PktsErrIn.Lo;
    portStatPtr->pktsErrIn.l[1] = channelCounter.PktsErrIn.Hi;

    portStatPtr->tcamHitMultiple.l[0] = channelCounter.TCAMHitMultiple.Lo;
    portStatPtr->tcamHitMultiple.l[1] = channelCounter.TCAMHitMultiple.Hi;

    portStatPtr->tcamMiss.l[0] = channelCounter.TCAMMiss.Lo;
    portStatPtr->tcamMiss.l[1] = channelCounter.TCAMMiss.Hi;

    portStatPtr->headerParserDroppedPkts.l[0] = channelCounter.HeaderParserDroppedPkts.Lo;
    portStatPtr->headerParserDroppedPkts.l[1] = channelCounter.HeaderParserDroppedPkts.Hi;

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyStatisticsGet function
* @endinternal
*
* @brief   This API reads the MACSec classifier per-port statistics.
*          Port counters are counting events that for debugging purpose should be accounted per port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  portNum             - The physical port number to read the statistics for.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] portStatPtr         - (pointer to) Classifier statistics counter identified by portNum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyStatisticsGet
(
    IN   GT_U8                                          devNum,
    IN   GT_PHYSICAL_PORT_NUM                           portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                 direction,
    IN   GT_BOOL                                        syncEnable,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC  *portStatPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyStatisticsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, syncEnable, portStatPtr));

    rc = internal_cpssDxChMacSecPortClassifyStatisticsGet(devNum, portNum, direction, syncEnable, portStatPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, syncEnable, portStatPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsSaGet function
* @endinternal
*
* @brief   Reads egress or ingress SA statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  saHandle            - SA handle of the SA for which the statistics must be read.
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statSaPtr           - (pointer to) egress or ingress SA statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsSaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    IN   GT_BOOL                            syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT  *statSaPtr
)
{
    GT_STATUS         rc;
    GT_U32            macSecUnitId;
    SecY_SA_Stat_E_t  saEgressCounters;
    SecY_SA_Stat_I_t  saIngressCounters;
    SecY_SAHandle_t   secySaHandle;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statSaPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void*)saHandle;

    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        /* Clear structure before filling it */
        cpssOsMemSet(&saEgressCounters, 0, sizeof(saEgressCounters));

        /* Read out egress SA statistics, */
        rc = SecY_SA_Statistics_E_Get(macSecUnitId, secySaHandle, &saEgressCounters, syncEnable);

        /* Update output: set SA egress counters */
        statSaPtr->egress.outOctetsEncryptedProtected.l[0] = saEgressCounters.OutOctetsEncryptedProtected.Lo;
        statSaPtr->egress.outOctetsEncryptedProtected.l[1] = saEgressCounters.OutOctetsEncryptedProtected.Hi;

        statSaPtr->egress.outPktsEncryptedProtected.l[0] = saEgressCounters.OutPktsEncryptedProtected.Lo;
        statSaPtr->egress.outPktsEncryptedProtected.l[1] = saEgressCounters.OutPktsEncryptedProtected.Hi;

        statSaPtr->egress.outPktsSANotInUse.l[0] = saEgressCounters.OutPktsSANotInUse.Lo;
        statSaPtr->egress.outPktsSANotInUse.l[1] = saEgressCounters.OutPktsSANotInUse.Hi;

        statSaPtr->egress.outPktsTooLong.l[0] = saEgressCounters.OutPktsTooLong.Lo;
        statSaPtr->egress.outPktsTooLong.l[1] = saEgressCounters.OutPktsTooLong.Hi;
    }
    else
    {
        /* Clear structure before filling it */
        cpssOsMemSet(&saIngressCounters, 0, sizeof(saIngressCounters));

        /* Read out ingress SA statistics, */
        rc = SecY_SA_Statistics_I_Get(macSecUnitId, secySaHandle, &saIngressCounters, syncEnable);

        /* Update output: set SA ingress counters */
        statSaPtr->ingress.inOctetsDecrypted.l[0] = saIngressCounters.InOctetsDecrypted.Lo;
        statSaPtr->ingress.inOctetsDecrypted.l[1] = saIngressCounters.InOctetsDecrypted.Hi;

        statSaPtr->ingress.inOctetsValidated.l[0] = saIngressCounters.InOctetsValidated.Lo;
        statSaPtr->ingress.inOctetsValidated.l[1] = saIngressCounters.InOctetsValidated.Hi;

        statSaPtr->ingress.inPktsDelayed.l[0] = saIngressCounters.InPktsDelayed.Lo;
        statSaPtr->ingress.inPktsDelayed.l[1] = saIngressCounters.InPktsDelayed.Hi;

        statSaPtr->ingress.inPktsInvalid.l[0] = saIngressCounters.InPktsInvalid.Lo;
        statSaPtr->ingress.inPktsInvalid.l[1] = saIngressCounters.InPktsInvalid.Hi;

        statSaPtr->ingress.inPktsLate.l[0] = saIngressCounters.InPktsLate.Lo;
        statSaPtr->ingress.inPktsLate.l[1] = saIngressCounters.InPktsLate.Hi;

        statSaPtr->ingress.inPktsNotUsingSA.l[0] = saIngressCounters.InPktsNotUsingSA.Lo;
        statSaPtr->ingress.inPktsNotUsingSA.l[1] = saIngressCounters.InPktsNotUsingSA.Hi;

        statSaPtr->ingress.inPktsNotValid.l[0] = saIngressCounters.InPktsNotValid.Lo;
        statSaPtr->ingress.inPktsNotValid.l[1] = saIngressCounters.InPktsNotValid.Hi;

        statSaPtr->ingress.inPktsOK.l[0] = saIngressCounters.InPktsOK.Lo;
        statSaPtr->ingress.inPktsOK.l[1] = saIngressCounters.InPktsOK.Hi;

        statSaPtr->ingress.inPktsUnchecked.l[0] = saIngressCounters.InPktsUnchecked.Lo;
        statSaPtr->ingress.inPktsUnchecked.l[1] = saIngressCounters.InPktsUnchecked.Hi;

        statSaPtr->ingress.inPktsUnusedSA.l[0] = saIngressCounters.InPktsUnusedSA.Lo;
        statSaPtr->ingress.inPktsUnusedSA.l[1] = saIngressCounters.InPktsUnusedSA.Hi;
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsSaGet function
* @endinternal
*
* @brief   Reads egress or ingress SA statistics from a MACSec Transformer
*           - Set fSynce to False
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  saHandle            - SA handle of the SA for which the statistics must be read.
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statSaPtr           - (pointer to) egress or ingress SA statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsSaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    IN   GT_BOOL                            syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT  *statSaPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsSaGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saHandle, syncEnable, statSaPtr));

    rc = internal_cpssDxChMacSecSecyStatisticsSaGet(devNum, unitBmp, direction, saHandle, syncEnable, statSaPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saHandle, syncEnable, statSaPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsSecyGet function
* @endinternal
*
* @brief   Reads egress or ingress SecY statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPort               - vPort for which the SecY statistics must be read.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statSecyPtr         - (pointer to) egress or ingress SecY statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsSecyGet
(
    IN   GT_U8                                devNum,
    IN   GT_MACSEC_UNIT_BMP                   unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    IN   GT_U32                               vPort,
    IN   GT_BOOL                              syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT  *statSecyPtr
)
{
    GT_STATUS          rc;
    GT_U32             macSecUnitId;
    SecY_SecY_Stat_I_t secyIngressCounters;
    SecY_SecY_Stat_E_t secyEgressCounters;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statSecyPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        /* Clear structure before filling it */
        cpssOsMemSet(&secyEgressCounters, 0, sizeof(secyEgressCounters));

        /* Read the SecY egress statistics counters. */
        rc = SecY_SecY_Statistics_E_Get(macSecUnitId, vPort, &secyEgressCounters, syncEnable);

        /* Update output: set SecY egress counters */
        statSecyPtr->egress.outPktsControl.l[0] = secyEgressCounters.OutPktsControl.Lo;
        statSecyPtr->egress.outPktsControl.l[1] = secyEgressCounters.OutPktsControl.Hi;

        statSecyPtr->egress.outPktsTransformError.l[0] = secyEgressCounters.OutPktsTransformError.Lo;
        statSecyPtr->egress.outPktsTransformError.l[1] = secyEgressCounters.OutPktsTransformError.Hi;

        statSecyPtr->egress.outPktsUntagged.l[0] = secyEgressCounters.OutPktsUntagged.Lo;
        statSecyPtr->egress.outPktsUntagged.l[1] = secyEgressCounters.OutPktsUntagged.Hi;
    }
    else
    {
        /* Clear structure before filling it */
        cpssOsMemSet(&secyIngressCounters, 0, sizeof(secyIngressCounters));

        /* Read the SecY ingress statistics counters. */
        rc = SecY_SecY_Statistics_I_Get(macSecUnitId, vPort, &secyIngressCounters, syncEnable);

        /* Update output: set SecY ingress counters */
        statSecyPtr->ingress.inPktsBadTag.l[0] = secyIngressCounters.InPktsBadTag.Lo;
        statSecyPtr->ingress.inPktsBadTag.l[1] = secyIngressCounters.InPktsBadTag.Hi;

        statSecyPtr->ingress.inPktsControl.l[0] = secyIngressCounters.InPktsControl.Lo;
        statSecyPtr->ingress.inPktsControl.l[1] = secyIngressCounters.InPktsControl.Hi;

        statSecyPtr->ingress.inPktsNoSCI.l[0] = secyIngressCounters.InPktsNoSCI.Lo;
        statSecyPtr->ingress.inPktsNoSCI.l[1] = secyIngressCounters.InPktsNoSCI.Hi;

        statSecyPtr->ingress.inPktsNoTag.l[0] = secyIngressCounters.InPktsNoTag.Lo;
        statSecyPtr->ingress.inPktsNoTag.l[1] = secyIngressCounters.InPktsNoTag.Hi;

        statSecyPtr->ingress.inPktsTaggedCtrl.l[0] = secyIngressCounters.InPktsTaggedCtrl.Lo;
        statSecyPtr->ingress.inPktsTaggedCtrl.l[1] = secyIngressCounters.InPktsTaggedCtrl.Hi;

        statSecyPtr->ingress.inPktsTransformError.l[0] = secyIngressCounters.InPktsTransformError.Lo;
        statSecyPtr->ingress.inPktsTransformError.l[1] = secyIngressCounters.InPktsTransformError.Hi;

        statSecyPtr->ingress.inPktsUnknownSCI.l[0] = secyIngressCounters.InPktsUnknownSCI.Lo;
        statSecyPtr->ingress.inPktsUnknownSCI.l[1] = secyIngressCounters.InPktsUnknownSCI.Hi;

        statSecyPtr->ingress.inPktsUntagged.l[0] = secyIngressCounters.InPktsUntagged.Lo;
        statSecyPtr->ingress.inPktsUntagged.l[1] = secyIngressCounters.InPktsUntagged.Hi;
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsSecyGet function
* @endinternal
*
* @brief   Reads egress or ingress SecY statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPort               - vPort for which the SecY statistics must be read.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statSecyPtr         - (pointer to) egress or ingress SecY statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsSecyGet
(
    IN   GT_U8                                devNum,
    IN   GT_MACSEC_UNIT_BMP                   unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    IN   GT_U32                               vPort,
    IN   GT_BOOL                              syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT  *statSecyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsSecyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPort, syncEnable, statSecyPtr));

    rc = internal_cpssDxChMacSecSecyStatisticsSecyGet(devNum, unitBmp, direction, vPort, syncEnable, statSecyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPort, syncEnable, statSecyPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsIfcGet function
* @endinternal
*
* @brief   Reads egress or ingress interface statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPort               - vPort for which the SecY statistics must be read.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statIfcPtr          - (pointer to) egress or ingress interface statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsIfcGet
(
    IN   GT_U8                               devNum,
    IN   GT_MACSEC_UNIT_BMP                  unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN   GT_U32                              vPort,
    IN   GT_BOOL                             syncEnable,
    OUT  CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT  *statIfcPtr
)
{
    GT_STATUS          rc;
    GT_U32             macSecUnitId;
    SecY_Ifc_Stat_I_t  ifcIngressCounters;
    SecY_Ifc_Stat_E_t  ifcEgressCounters;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statIfcPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        /* Clear structure before filling it */
        cpssOsMemSet(&ifcEgressCounters, 0, sizeof(ifcEgressCounters));

        /* Read the IFC/IFC1 egress statistics counters. */
        rc = SecY_Ifc_Statistics_E_Get(macSecUnitId, vPort, &ifcEgressCounters, syncEnable);

        /* Update output: set SecY egress counters */
        statIfcPtr->egress.outOctetsCommon.l[0] = ifcEgressCounters.OutOctetsCommon.Lo;
        statIfcPtr->egress.outOctetsCommon.l[1] = ifcEgressCounters.OutOctetsCommon.Hi;

        statIfcPtr->egress.outOctetsControlled.l[0] = ifcEgressCounters.OutOctetsControlled.Lo;
        statIfcPtr->egress.outOctetsControlled.l[1] = ifcEgressCounters.OutOctetsControlled.Hi;

        statIfcPtr->egress.outOctetsUncontrolled.l[0] = ifcEgressCounters.OutOctetsUncontrolled.Lo;
        statIfcPtr->egress.outOctetsUncontrolled.l[1] = ifcEgressCounters.OutOctetsUncontrolled.Hi;

        statIfcPtr->egress.outPktsBroadcastControlled.l[0] = ifcEgressCounters.OutPktsBroadcastControlled.Lo;
        statIfcPtr->egress.outPktsBroadcastControlled.l[1] = ifcEgressCounters.OutPktsBroadcastControlled.Hi;

        statIfcPtr->egress.outPktsBroadcastUncontrolled.l[0] = ifcEgressCounters.OutPktsBroadcastUncontrolled.Lo;
        statIfcPtr->egress.outPktsBroadcastUncontrolled.l[1] = ifcEgressCounters.OutPktsBroadcastUncontrolled.Hi;

        statIfcPtr->egress.outPktsMulticastControlled.l[0] = ifcEgressCounters.OutPktsMulticastControlled.Lo;
        statIfcPtr->egress.outPktsMulticastControlled.l[1] = ifcEgressCounters.OutPktsMulticastControlled.Hi;

        statIfcPtr->egress.outPktsMulticastUncontrolled.l[0] = ifcEgressCounters.OutPktsMulticastUncontrolled.Lo;
        statIfcPtr->egress.outPktsMulticastUncontrolled.l[1] = ifcEgressCounters.OutPktsMulticastUncontrolled.Hi;

        statIfcPtr->egress.outPktsUnicastControlled.l[0] = ifcEgressCounters.OutPktsUnicastControlled.Lo;
        statIfcPtr->egress.outPktsUnicastControlled.l[1] = ifcEgressCounters.OutPktsUnicastControlled.Hi;

        statIfcPtr->egress.outPktsUnicastUncontrolled.l[0] = ifcEgressCounters.OutPktsUnicastUncontrolled.Lo;
        statIfcPtr->egress.outPktsUnicastUncontrolled.l[1] = ifcEgressCounters.OutPktsUnicastUncontrolled.Hi;

    }
    else
    {
        /* Clear structure before filling it */
        cpssOsMemSet(&ifcIngressCounters, 0, sizeof(ifcIngressCounters));

        /* Read the IFC/IFC1 ingress statistics counters. */
        rc = SecY_Ifc_Statistics_I_Get(macSecUnitId, vPort, &ifcIngressCounters, syncEnable);

        /* Update output: set SecY ingress counters */
        statIfcPtr->ingress.inOctetsControlled.l[0] = ifcIngressCounters.InOctetsControlled.Lo;
        statIfcPtr->ingress.inOctetsControlled.l[1] = ifcIngressCounters.InOctetsControlled.Hi;

        statIfcPtr->ingress.inOctetsUncontrolled.l[0] = ifcIngressCounters.InOctetsUncontrolled.Lo;
        statIfcPtr->ingress.inOctetsUncontrolled.l[1] = ifcIngressCounters.InOctetsUncontrolled.Hi;

        statIfcPtr->ingress.inPktsBroadcastControlled.l[0] = ifcIngressCounters.InPktsBroadcastControlled.Lo;
        statIfcPtr->ingress.inPktsBroadcastControlled.l[1] = ifcIngressCounters.InPktsBroadcastControlled.Hi;

        statIfcPtr->ingress.inPktsBroadcastUncontrolled.l[0] = ifcIngressCounters.InPktsBroadcastUncontrolled.Lo;
        statIfcPtr->ingress.inPktsBroadcastUncontrolled.l[1] = ifcIngressCounters.InPktsBroadcastUncontrolled.Hi;

        statIfcPtr->ingress.inPktsMulticastControlled.l[0] = ifcIngressCounters.InPktsMulticastControlled.Lo;
        statIfcPtr->ingress.inPktsMulticastControlled.l[1] = ifcIngressCounters.InPktsMulticastControlled.Hi;

        statIfcPtr->ingress.inPktsMulticastUncontrolled.l[0] = ifcIngressCounters.InPktsMulticastUncontrolled.Lo;
        statIfcPtr->ingress.inPktsMulticastUncontrolled.l[1] = ifcIngressCounters.InPktsMulticastUncontrolled.Hi;

        statIfcPtr->ingress.inPktsUnicastControlled.l[0] = ifcIngressCounters.InPktsUnicastControlled.Lo;
        statIfcPtr->ingress.inPktsUnicastControlled.l[1] = ifcIngressCounters.InPktsUnicastControlled.Hi;

        statIfcPtr->ingress.inPktsUnicastUncontrolled.l[0] = ifcIngressCounters.InPktsUnicastUncontrolled.Lo;
        statIfcPtr->ingress.inPktsUnicastUncontrolled.l[1] = ifcIngressCounters.InPktsUnicastUncontrolled.Hi;
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsIfcGet function
* @endinternal
*
* @brief   Reads egress or ingress interface statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPort               - vPort for which the SecY statistics must be read.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statIfcPtr          - (pointer to) egress or ingress interface statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsIfcGet
(
    IN   GT_U8                               devNum,
    IN   GT_MACSEC_UNIT_BMP                  unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN   GT_U32                              vPort,
    IN   GT_BOOL                             syncEnable,
    OUT  CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT  *statIfcPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsIfcGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPort, syncEnable, statIfcPtr));

    rc = internal_cpssDxChMacSecSecyStatisticsIfcGet(devNum, unitBmp, direction, vPort, syncEnable, statIfcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPort, syncEnable, statIfcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsRxCamGet function
* @endinternal
*
* @brief   Reads RxCAM statistics (ingress only) from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  scIndex             - Index of the Secure Channel for which the RxCAM statistics must be read
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statRxCamCounterPtr - (pointer to) ingress RxCAM statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsRxCamGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   GT_U32                           scIndex,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statRxCamCounterPtr
)
{
    GT_STATUS          rc;
    GT_U32             macSecUnitId;
    SecY_RxCAM_Stat_t  rxCamCounter;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statRxCamCounterPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it */
    cpssOsMemSet(&rxCamCounter, 0, sizeof(rxCamCounter));

    /* Reads RxCAM statistics (ingress only) */
    rc = SecY_RxCAM_Statistics_Get(macSecUnitId, scIndex, &rxCamCounter, syncEnable);

    /* Update output: set SecY ingress counters */
    statRxCamCounterPtr->l[0] = rxCamCounter.CAMHit.Lo;
    statRxCamCounterPtr->l[1] = rxCamCounter.CAMHit.Hi;

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsRxCamGet function
* @endinternal
*
* @brief   Reads RxCAM statistics (ingress only) from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  scIndex             - Index of the Secure Channel for which the RxCAM statistics must be read
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statRxCamCounterPtr - (pointer to) ingress RxCAM statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsRxCamGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   GT_U32                           scIndex,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statRxCamCounterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsRxCamGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, scIndex, syncEnable, statRxCamCounterPtr));

    rc = internal_cpssDxChMacSecSecyStatisticsRxCamGet(devNum, unitBmp, scIndex, syncEnable, statRxCamCounterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, scIndex, syncEnable, statRxCamCounterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsVportClear function
* @endinternal
*
* @brief   Clear the SecY and IFC statistics counters belonging to a given vPort
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - device number.
* @param[in] unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction           - select Egress or Ingress MACSec transformer
* @param[in] vPort               - vPort number for which to clear the statistics
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsVportClear
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPort
)
{
    GT_STATUS          rc;
    GT_U32             macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear the SecY and IFC statistics counters belonging to a given vPort */
    rc = SecY_vPort_Statistics_Clear(macSecUnitId, vPort);

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsVportClear function
* @endinternal
*
* @brief   Clear the SecY and IFC statistics counters belonging to a given vPort
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - device number.
* @param[in] unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction           - select Egress or Ingress MACSec transformer
* @param[in] vPort               - vPort number for which to clear the statistics
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsVportClear
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsVportClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPort));

    rc = internal_cpssDxChMacSecSecyStatisticsVportClear(devNum, unitBmp, direction, vPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaChainSet function
* @endinternal
*
* @brief   Adds a new SA transform record and chain it to the current active SA
*
*   NOTEs: - This API should be used on the Egress direction only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] activeSaHandle       - SA handle of the current active SA
* @param[in] newTrRecParamsPtr    - (pointer to) new Transform Record parameters
* @param[out] newSaHandlePtr      - (pointer to) new SA handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaChainSet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     activeSaHandle,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *newTrRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *newSaHandlePtr
)
{
    GT_STATUS                        rc;
    GT_U32                           macSecUnitId;
    SecY_SAHandle_t                  newSecySaHandle = SecY_SAHandle_NULL;
    SecY_SAHandle_t                  secyActiveSaHandle;
    SecY_SA_t                        saParams;
    GT_U32                           *trRecBufferPtr = NULL;
    GT_U32                           saWordCount = 0;
    PRV_CPSS_SECY_TR_REC_PARAMS_STC  trRecParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointers are not NULL */
    CPSS_NULL_PTR_CHECK_MAC(newTrRecParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(newSaHandlePtr);

    /* Get MACSec unit ID  */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them */
    cpssOsMemSet(&saParams, 0, sizeof(saParams));
    cpssOsMemSet(&trRecParams, 0, sizeof(trRecParams));

    /* Set transform record parameters */
    trRecParams.direction     = CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E;
    trRecParams.an            = newTrRecParamsPtr->an;
    trRecParams.keyPtr        = newTrRecParamsPtr->keyArr;
    trRecParams.keyByteCount  = newTrRecParamsPtr->keyByteCount;
    trRecParams.sciPtr        = newTrRecParamsPtr->sciArr;
    trRecParams.seqNumLo      = newTrRecParamsPtr->seqNumLo;
    trRecParams.seqMask       = newTrRecParamsPtr->seqMask;

    /* Set Hash key pointer to point to custom Hash key array in case custom
       Hash key is required otherwise Hash key pointer will remain NULL */
    if (newTrRecParamsPtr->customHkeyEnable)
    {
        trRecParams.hKeyPtr = newTrRecParamsPtr->customHkeyArr;
    }

    /* Check packet numbering is of extended type (64 bits) */
    if (newTrRecParamsPtr->seqTypeExtended)
    {
        /* Set the relevant parameters for extended type */
        trRecParams.seqNumHi = newTrRecParamsPtr->seqNumHi;
        trRecParams.ssciPtr  = newTrRecParamsPtr->ssciArr;
        trRecParams.saltPtr  = newTrRecParamsPtr->saltArr;
    }

    /* Build SA transform record. Return pointer to buffer containing this transform record and size of record in 4B words */
    rc = prvCpssMacSecBuildSaTransformRecord(&trRecParams,&saWordCount,&trRecBufferPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set SA structure with SA parameters */
    saParams.TransformRecord_p = trRecBufferPtr;
    saParams.SA_WordCount      = saWordCount;
    saParams.ActionType        = CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E;

    /* Set active SA handle */
    secyActiveSaHandle.p = (void*)activeSaHandle;

    /* Call Transformer API to chain new SA to current active SA */
    rc = SecY_SA_Chain(macSecUnitId, secyActiveSaHandle, &newSecySaHandle, &saParams);

    /* If allocated then must free transform record buffer */
    if (trRecBufferPtr)
    {
        cpssOsFree(trRecBufferPtr);
    }

    /* Get Transformer new SA handle */
    *newSaHandlePtr = (CPSS_DXCH_MACSEC_SECY_SA_HANDLE)newSecySaHandle.p;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaChainSet function
* @endinternal
*
* @brief   Adds a new SA transform record and chain it to the current active SA
*
*   NOTEs: - This API should be used on the Egress direction only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] activeSaHandle       - SA handle of the current active SA
* @param[in] newTrRecParamsPtr    - (pointer to) new Transform Record parameters
* @param[out] newSaHandlePtr      - (pointer to) new SA handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaChainSet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     activeSaHandle,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *newTrRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *newSaHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaChainSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, activeSaHandle, newTrRecParamsPtr, newSaHandlePtr));

    rc = internal_cpssDxChMacSecSecySaChainSet(devNum, unitBmp, activeSaHandle, newTrRecParamsPtr, newSaHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, activeSaHandle, newTrRecParamsPtr, newSaHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaChainGet function
* @endinternal
*
* @brief   Return the SA handle of the SA that was chained to the given SA.
*          Return SecY_SAHandle_NULL if no SA handle is chained to the given SA handle.
*
*   NOTEs: - This API should be used on the Egress direction only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] currentSaHandle      - SA handle of the SA for which to find the chained SA
* @param[out] nextSaHandlePtr     - (pointer to) SA handle of the chained SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaChainGet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     currentSaHandle,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *nextSaHandlePtr
)
{
    GT_STATUS                        rc;
    GT_U32                           macSecUnitId;
    SecY_SAHandle_t                  nextSecySaHandle = SecY_SAHandle_NULL;
    SecY_SAHandle_t                  secyCurrentSaHandle;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(nextSaHandlePtr);

    /* Get MACSec unit ID  */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set current SA handle */
    secyCurrentSaHandle.p = (void*)currentSaHandle;

    /* Call Transformer API to chain new SA to current active SA */
    rc = SecY_SA_Chained_Get(macSecUnitId, secyCurrentSaHandle, &nextSecySaHandle);

    /* Get Transformer next SA handle that is chained to current SA */
    *nextSaHandlePtr = (CPSS_DXCH_MACSEC_SECY_SA_HANDLE)nextSecySaHandle.p;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaChainGet function
* @endinternal
*
* @brief   Return the SA handle of the SA that was chained to the given SA.
*          Return SecY_SAHandle_NULL if no SA handle is chained to the given SA handle.
*
*   NOTEs: - This API should be used on the Egress direction only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] currentSaHandle      - SA handle of the SA for which to find the chained SA
* @param[out] nextSaHandlePtr     - (pointer to) SA handle of the chained SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaChainGet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     currentSaHandle,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *nextSaHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaChainGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, currentSaHandle, nextSaHandlePtr));

    rc = internal_cpssDxChMacSecSecySaChainGet(devNum, unitBmp, currentSaHandle, nextSaHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, currentSaHandle, nextSaHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortSecyConfigSet function
* @endinternal
*
* @brief   Port configuration settings in MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] portCfgPtr           - (pointer to) channel configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortSecyConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  *portCfgPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    SecY_Device_Params_t     unitTopParams;
    SecY_ChannelConf_t       channelCfg;
    SecY_Channel_t           channelParams;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(portCfgPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&channelCfg, 0, sizeof(channelCfg));
    cpssOsMemSet(&channelParams, 0, sizeof(channelParams));

    /* Fill structure with parameters */
    channelParams.ChannelId                    = macSecChannelNum;
    channelParams.EtherType                    = 0x88E5; /* MACSec Ethertype */
    channelParams.fLowLatencyBypass            = portCfgPtr->bypassMacsecDevice;
    channelParams.fPktNumThrMode               = portCfgPtr->pktNumThrStrictCompareModeEnable;
    channelParams.RuleSecTAG.EtherType         = 0x88E5; /* MACSec Ethertype */
    channelParams.RuleSecTAG.fCheckCE          = portCfgPtr->ruleSecTag.checkCe;
    channelParams.RuleSecTAG.fCheckKay         = portCfgPtr->ruleSecTag.checkKay;
    channelParams.RuleSecTAG.fCheckPN          = portCfgPtr->ruleSecTag.checkPn;
    channelParams.RuleSecTAG.fCheckSC          = portCfgPtr->ruleSecTag.checkSc;
    channelParams.RuleSecTAG.fCheckSL          = portCfgPtr->ruleSecTag.checkSl;
    channelParams.RuleSecTAG.fCheckV           = portCfgPtr->ruleSecTag.checkV;
    channelParams.RuleSecTAG.fCompEType        = portCfgPtr->ruleSecTag.compEtype;
    channelParams.StatCtrl.SeqNrThreshold      = portCfgPtr->statCtrl.seqNrThreshold;
    channelParams.StatCtrl.SeqNrThreshold64.Lo = portCfgPtr->statCtrl.seqNrThreshold64.l[0];
    channelParams.StatCtrl.SeqNrThreshold64.Hi = portCfgPtr->statCtrl.seqNrThreshold64.l[1];

    /* Set single channel */
    channelCfg.Params.ChannelCount = 1;
    channelCfg.Params.Channel_p    = &channelParams;
    unitTopParams.ChConf_p         = &channelCfg;

    /* Call Transformer update API to apply these parameters */
    rc = SecY_Device_Update(macSecUnitId, &unitTopParams);

    return rc;
}

/**
* @internal cpssDxChMacSecPortSecyConfigSet function
* @endinternal
*
* @brief   Port configuration settings in MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] portCfgPtr           - (pointer to) channel configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortSecyConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  *portCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortSecyConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, portCfgPtr));

    rc = internal_cpssDxChMacSecPortSecyConfigSet(devNum, portNum, direction, portCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, portCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortSecyConfigGet function
* @endinternal
*
* @brief   Get port settings specified per port from Transformer device
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[out] portCfgPtr          - (pointer to) port configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortSecyConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    OUT CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC   *portCfgPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    SecY_Device_Params_t     unitTopParams;
    SecY_ChannelConf_t       channelCfg;
    SecY_Channel_t           *channelParamsPtr;
    GT_U32                   maxSecYChannels = 0;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(portCfgPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get maximum number of channels */
    rc = SecY_Device_Limits(macSecUnitId, &maxSecYChannels, NULL, NULL, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Allocate memory */
    channelParamsPtr = cpssOsMalloc( (sizeof(SecY_Channel_t)) * maxSecYChannels );
    if (channelParamsPtr == NULL)
    {
        return rc = GT_NO_RESOURCE;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&channelCfg, 0, sizeof(channelCfg));
    cpssOsMemSet(channelParamsPtr, 0, sizeof(SecY_Channel_t) * maxSecYChannels);

    /* Fill structure with parameters */
    channelCfg.Params.Channel_p    = channelParamsPtr;
    unitTopParams.ChConf_p         = &channelCfg;

    /* Call Transformer API to get port configuration parameters */
    rc = SecY_Device_Config_Get(macSecUnitId, &unitTopParams);
    if (rc != GT_OK)
    {
        cpssOsFree(channelParamsPtr);
        return rc;
    }

    /* Set output parameters */
    portCfgPtr->bypassMacsecDevice               = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].fLowLatencyBypass;
    portCfgPtr->pktNumThrStrictCompareModeEnable = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].fPktNumThrMode;
    portCfgPtr->ruleSecTag.checkCe               = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCheckCE;
    portCfgPtr->ruleSecTag.checkKay              = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCheckKay;
    portCfgPtr->ruleSecTag.checkPn               = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCheckPN;
    portCfgPtr->ruleSecTag.checkSc               = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCheckSC;
    portCfgPtr->ruleSecTag.checkSl               = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCheckSL;
    portCfgPtr->ruleSecTag.checkV                = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCheckV;
    portCfgPtr->ruleSecTag.compEtype             = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].RuleSecTAG.fCompEType;
    portCfgPtr->statCtrl.seqNrThreshold          = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].StatCtrl.SeqNrThreshold;
    portCfgPtr->statCtrl.seqNrThreshold64.l[0]   = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].StatCtrl.SeqNrThreshold64.Lo;
    portCfgPtr->statCtrl.seqNrThreshold64.l[1]   = unitTopParams.ChConf_p->Params.Channel_p[macSecChannelNum].StatCtrl.SeqNrThreshold64.Hi;

    /* Free memory */
    cpssOsFree(channelParamsPtr);

    return rc;
}

/**
* @internal cpssDxChMacSecPortSecyConfigGet function
* @endinternal
*
* @brief   Get port settings specified per port from Transformer device
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[out] portCfgPtr          - (pointer to) port configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortSecyConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    OUT CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC   *portCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortSecyConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, portCfgPtr));

    rc = internal_cpssDxChMacSecPortSecyConfigGet(devNum, portNum, direction, portCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, portCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet function
* @endinternal
*
* @brief   SecTAG parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] secTagParserPtr      - (pointer to) sectag parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet
(
    IN  GT_U8                                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_HeaderParser_t      unitHeaderParserParams;
    CfyE_SecTAG_Parser_t     unitSecTagParams;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(secTagParserPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitHeaderParserParams, 0, sizeof(unitHeaderParserParams));
    cpssOsMemSet(&unitSecTagParams, 0, sizeof(unitSecTagParams));

    /* Update SecTag parser parameters */
    unitSecTagParams.fCheckKay      = secTagParserPtr->checkKay;
    unitSecTagParams.fCheckVersion  = secTagParserPtr->checkVersion;
    unitSecTagParams.fCompType      = secTagParserPtr->compType;
    unitSecTagParams.fLookupUseSCI  = secTagParserPtr->lookupUseSci;
    unitSecTagParams.MACsecTagValue = secTagParserPtr->macSecTagValue;

    /* Update SecTag and HeaderParser pointers */
    unitHeaderParserParams.SecTAG_Parser_p = &unitSecTagParams;
    unitTopParams.HeaderParser_p = &unitHeaderParserParams;

    /* Call Classifier update API to apply these parameters */
    rc = CfyE_Device_Update(macSecUnitId, macSecChannelNum, &unitTopParams);

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet function
* @endinternal
*
* @brief   SecTAG parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] secTagParserPtr      - (pointer to) sectag parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet
(
    IN  GT_U8                                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, secTagParserPtr));

    rc = internal_cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet(devNum, portNum, direction, secTagParserPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, secTagParserPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet function
* @endinternal
*
* @brief   Get SecTAG parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[out] secTagParserPtr      - (pointer to) sectag parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet
(
    IN   GT_U8                                        devNum,
    IN   GT_PHYSICAL_PORT_NUM                         portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_HeaderParser_t      unitHeaderParserParams;
    CfyE_SecTAG_Parser_t     unitSecTagParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(secTagParserPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitHeaderParserParams, 0, sizeof(unitHeaderParserParams));
    cpssOsMemSet(&unitSecTagParams, 0, sizeof(unitSecTagParams));

    /* Set pointers to structures  */
    unitTopParams.HeaderParser_p = &unitHeaderParserParams;
    unitHeaderParserParams.SecTAG_Parser_p = &unitSecTagParams;

    /* Call Classifier API to get port configuration parameters */
    rc = CfyE_Device_Config_Get(macSecUnitId, macSecChannelNum, &unitTopParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get Header SecTag parser parameters */
    secTagParserPtr->checkKay       = unitTopParams.HeaderParser_p->SecTAG_Parser_p->fCheckKay;
    secTagParserPtr->checkVersion   = unitTopParams.HeaderParser_p->SecTAG_Parser_p->fCheckVersion;
    secTagParserPtr->compType       = unitTopParams.HeaderParser_p->SecTAG_Parser_p->fCompType;
    secTagParserPtr->lookupUseSci   = unitTopParams.HeaderParser_p->SecTAG_Parser_p->fLookupUseSCI;
    secTagParserPtr->macSecTagValue = unitTopParams.HeaderParser_p->SecTAG_Parser_p->MACsecTagValue;

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet function
* @endinternal
*
* @brief   Get SecTAG parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[out] secTagParserPtr      - (pointer to) sectag parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet
(
    IN   GT_U8                                        devNum,
    IN   GT_PHYSICAL_PORT_NUM                         portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, secTagParserPtr));

    rc = internal_cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet(devNum, portNum, direction, secTagParserPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, secTagParserPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyHdrParserVlanConfigSet function
* @endinternal
*
* @brief   VLAN parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vlanParserPtr        - (pointer to) VLAN parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyHdrParserVlanConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_HeaderParser_t      unitHeaderParserParams;
    CfyE_VLAN_Parser_t       unitVlanParams;
    GT_U32                   i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vlanParserPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitHeaderParserParams, 0, sizeof(unitHeaderParserParams));
    cpssOsMemSet(&unitVlanParams, 0, sizeof(unitVlanParams));

    /* Per-port VLAN parsing settings */
    unitVlanParams.CP.fParseQinQ   = vlanParserPtr->cp.parseQinQ;
    unitVlanParams.CP.fParseQTag   = vlanParserPtr->cp.parseQTag;
    unitVlanParams.CP.fParseStag1  = vlanParserPtr->cp.parseStag1;
    unitVlanParams.CP.fParseStag2  = vlanParserPtr->cp.parseStag2;
    unitVlanParams.CP.fParseStag3  = vlanParserPtr->cp.parseStag3;
    unitVlanParams.fSCP            = vlanParserPtr->scpFlag;
    unitVlanParams.SCP.fParseQinQ  = vlanParserPtr->scp.parseQinQ;
    unitVlanParams.SCP.fParseQTag  = vlanParserPtr->scp.parseQTag;
    unitVlanParams.SCP.fParseStag1 = vlanParserPtr->scp.parseStag1;
    unitVlanParams.SCP.fParseStag2 = vlanParserPtr->scp.parseStag2;
    unitVlanParams.SCP.fParseStag3 = vlanParserPtr->scp.parseStag3;

    /* Global VLAN parsing settings */
    unitVlanParams.DefaultUp       = vlanParserPtr->defaultUp;
    unitVlanParams.fQTagUpEnable   = vlanParserPtr->qTagUpEnable;
    unitVlanParams.fSTagUpEnable   = vlanParserPtr->sTagUpEnable;
    unitVlanParams.QTag            = vlanParserPtr->qTag;
    unitVlanParams.STag1           = vlanParserPtr->sTag1;
    unitVlanParams.STag2           = vlanParserPtr->sTag2;
    unitVlanParams.STag3           = vlanParserPtr->sTag3;

    /* Translation table for 1st & 2nd 802.1Q tag */
    for (i=0;i<CFYE_VLAN_UP_MAX_COUNT;i++)
    {
        unitVlanParams.UpTable1[i] = vlanParserPtr->upTable1[i];
        unitVlanParams.UpTable2[i] = vlanParserPtr->upTable2[i];
    }

    /* Update VLAN and HeaderParser pointers */
    unitHeaderParserParams.VLAN_Parser_p = &unitVlanParams;
    unitTopParams.HeaderParser_p = &unitHeaderParserParams;

    /* Call Classifier update API to apply these parameters */
    rc = CfyE_Device_Update(macSecUnitId, macSecChannelNum, &unitTopParams);

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyHdrParserVlanConfigSet function
* @endinternal
*
* @brief   VLAN parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vlanParserPtr        - (pointer to) VLAN parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserVlanConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyHdrParserVlanConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, vlanParserPtr));

    rc = internal_cpssDxChMacSecPortClassifyHdrParserVlanConfigSet(devNum, portNum, direction, vlanParserPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, vlanParserPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyHdrParserVlanConfigGet function
* @endinternal
*
* @brief   Get VLAN parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum             - device number.
* @param[in]  portNum            - physical port number
* @param[in]  direction          - select Egress or Ingress MACSec classifier
* @param[out] vlanParserPtr      - (pointer to) vlan parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyHdrParserVlanConfigGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PHYSICAL_PORT_NUM                       portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_HeaderParser_t      unitHeaderParserParams;
    CfyE_VLAN_Parser_t       unitVlanParams;
    GT_U32                   i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vlanParserPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitHeaderParserParams, 0, sizeof(unitHeaderParserParams));
    cpssOsMemSet(&unitVlanParams, 0, sizeof(unitVlanParams));

    /* Set pointers to structures  */
    unitTopParams.HeaderParser_p = &unitHeaderParserParams;
    unitHeaderParserParams.VLAN_Parser_p = &unitVlanParams;

    /* Call Classifier API to get port configuration parameters */
    rc = CfyE_Device_Config_Get(macSecUnitId, macSecChannelNum, &unitTopParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get per-port VLAN parsing settings */
    vlanParserPtr->cp.parseQinQ   = unitTopParams.HeaderParser_p->VLAN_Parser_p->CP.fParseQinQ;
    vlanParserPtr->cp.parseQTag   = unitTopParams.HeaderParser_p->VLAN_Parser_p->CP.fParseQTag;
    vlanParserPtr->cp.parseStag1  = unitTopParams.HeaderParser_p->VLAN_Parser_p->CP.fParseStag1;
    vlanParserPtr->cp.parseStag2  = unitTopParams.HeaderParser_p->VLAN_Parser_p->CP.fParseStag2;
    vlanParserPtr->cp.parseStag3  = unitTopParams.HeaderParser_p->VLAN_Parser_p->CP.fParseStag3;
    vlanParserPtr->scpFlag        = unitTopParams.HeaderParser_p->VLAN_Parser_p->fSCP;
    vlanParserPtr->scp.parseQinQ  = unitTopParams.HeaderParser_p->VLAN_Parser_p->SCP.fParseQinQ;
    vlanParserPtr->scp.parseQTag  = unitTopParams.HeaderParser_p->VLAN_Parser_p->SCP.fParseQTag;
    vlanParserPtr->scp.parseStag1 = unitTopParams.HeaderParser_p->VLAN_Parser_p->SCP.fParseStag1;
    vlanParserPtr->scp.parseStag2 = unitTopParams.HeaderParser_p->VLAN_Parser_p->SCP.fParseStag2;
    vlanParserPtr->scp.parseStag3 = unitTopParams.HeaderParser_p->VLAN_Parser_p->SCP.fParseStag3;

    /* Get global VLAN parsing settings */
    vlanParserPtr->defaultUp    = unitTopParams.HeaderParser_p->VLAN_Parser_p->DefaultUp;
    vlanParserPtr->qTag         = unitTopParams.HeaderParser_p->VLAN_Parser_p->QTag;
    vlanParserPtr->qTagUpEnable = unitTopParams.HeaderParser_p->VLAN_Parser_p->fQTagUpEnable;
    vlanParserPtr->sTag1        = unitTopParams.HeaderParser_p->VLAN_Parser_p->STag1;
    vlanParserPtr->sTag2        = unitTopParams.HeaderParser_p->VLAN_Parser_p->STag2;
    vlanParserPtr->sTag3        = unitTopParams.HeaderParser_p->VLAN_Parser_p->STag3;
    vlanParserPtr->sTagUpEnable = unitTopParams.HeaderParser_p->VLAN_Parser_p->fSTagUpEnable;

    /* Translation table for 1st & 2nd 802.1Q tag */
    for (i=0;i<CFYE_VLAN_UP_MAX_COUNT;i++)
    {
        vlanParserPtr->upTable1[i] = unitTopParams.HeaderParser_p->VLAN_Parser_p->UpTable1[i];
        vlanParserPtr->upTable2[i] = unitTopParams.HeaderParser_p->VLAN_Parser_p->UpTable2[i];
    }

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyHdrParserVlanConfigGet function
* @endinternal
*
* @brief   Get VLAN parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum             - device number.
* @param[in]  portNum            - physical port number
* @param[in]  direction          - select Egress or Ingress MACSec classifier
* @param[out] vlanParserPtr      - (pointer to) vlan parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserVlanConfigGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PHYSICAL_PORT_NUM                       portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyHdrParserVlanConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, vlanParserPtr));

    rc = internal_cpssDxChMacSecPortClassifyHdrParserVlanConfigGet(devNum, portNum, direction, vlanParserPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, vlanParserPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyStatisticsConfigSet function
* @endinternal
*
* @brief   Statistics control settings for all MACSec classifier units
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] statisticsCfgPtr     - (pointer to) statistics control parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyStatisticsConfigSet
(
    IN  GT_U8                                             devNum,
    IN  GT_MACSEC_UNIT_BMP                                unitBmp,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS                  rc;
    GT_U32                     macSecUnitId;
    CfyE_Device_t              unitTopParams;
    CfyE_Statistics_Control_t  unitStatisticsParams;
    GT_U32                     macSecDpId;
    GT_U32                     macSecMaxNumDp;
    GT_U32                     macSecDir;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statisticsCfgPtr);

    /* Check unitBmp is valid */
    rc = prvCpssMacSecBmpCheck(devNum, &unitBmp, GT_TRUE/*multipe units are allowed*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitStatisticsParams, 0, sizeof(unitStatisticsParams));

    /* Prepare statistics parameters */
    unitStatisticsParams.ChanCountFrameThr.Lo  = statisticsCfgPtr->portCountFrameThr.l[0];
    unitStatisticsParams.ChanCountFrameThr.Hi  = statisticsCfgPtr->portCountFrameThr.l[1];
    unitStatisticsParams.CountFrameThr.Lo      = statisticsCfgPtr->countFrameThr.l[0];
    unitStatisticsParams.CountFrameThr.Hi      = statisticsCfgPtr->countFrameThr.l[1];
    unitStatisticsParams.CountIncDisCtrl       = statisticsCfgPtr->countIncDisCtrl;
    unitStatisticsParams.fAutoStatCntrsReset   = statisticsCfgPtr->autoStatCntrsReset;

    /* Update unit statistics pointer */
    unitTopParams.StatControl_p = &unitStatisticsParams;

    /* Get maximum number of Data path units connected to MACSec */
    macSecMaxNumDp = ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp) >> 1) + 1;

    /* Loop over the maximum number of Data paths. For Hawk:2 for Phoenix:1  */
    for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
    {
        /* Check Data path bits map. Step in only if bit is on */
        if ( (unitBmp & (1 << macSecDpId)) != 0 )
        {
            /* Loop over egress and ingress directions */
            for (macSecDir = 0; (macSecDir < 2); macSecDir++)
            {
                /* Get MACSec unit ID */
                macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,macSecDpId,CPSS_DXCH_MACSEC_CLASSIFIER_E,macSecDir);

                /* Verify this specific MACSec unit was indeed initialized */
                rc = prvCpssMacSecInitCheck(CPSS_DXCH_MACSEC_CLASSIFIER_E,macSecUnitId);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* Call Classifier update API to apply these parameters
                   Note: '0' is port number which is irelevant for statistics configurations */
                rc = CfyE_Device_Update(macSecUnitId, 0, &unitTopParams);
            }
        }
    }

    return rc;
}


/**
* @internal cpssDxChMacSecClassifyStatisticsConfigSet function
* @endinternal
*
* @brief   Statistics control settings for all MACSec classifier units
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] statisticsCfgPtr     - (pointer to) statistics control parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatisticsConfigSet
(
    IN  GT_U8                                             devNum,
    IN  GT_MACSEC_UNIT_BMP                                unitBmp,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyStatisticsConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, statisticsCfgPtr));

    rc = internal_cpssDxChMacSecClassifyStatisticsConfigSet(devNum, unitBmp, statisticsCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, statisticsCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyStatisticsConfigGet function
* @endinternal
*
* @brief  Get statistics control settings from MACSec classifier units
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] statisticsCfgPtr     - (pointer to) statistics control parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyStatisticsConfigGet
(
    IN   GT_U8                                             devNum,
    IN   GT_MACSEC_UNIT_BMP                                unitBmp,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS                  rc;
    GT_U32                     macSecUnitId;
    CfyE_Device_t              unitTopParams;
    CfyE_Statistics_Control_t  unitStatisticsParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statisticsCfgPtr);

    /* Check unitBmp is valid */
    rc = prvCpssMacSecBmpCheck(devNum, &unitBmp, GT_TRUE/*multipe units are allowed*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Since statistics configurations are the same in all Classifier units we only need to read it from one of them.
       Verify which Data path is requested, select Egress and get related MACSec unit ID  */

    /* At this point unitBmp must be valid. */
    if ( (unitBmp & 1) == 1 )
    {
        /* Get MACSec unit ID connected to Data path 0 */
        macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,0,CPSS_DXCH_MACSEC_CLASSIFIER_E,CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
    }
    else
    {
        /* Get MACSec unit ID connected to Data path 1 */
        macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,1,CPSS_DXCH_MACSEC_CLASSIFIER_E,CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
    }

    /* Verify this specific MACSec unit was indeed initialized */
    rc = prvCpssMacSecInitCheck(CPSS_DXCH_MACSEC_CLASSIFIER_E,macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitStatisticsParams, 0, sizeof(unitStatisticsParams));

    /* Update unit statistics pointer */
    unitTopParams.StatControl_p = &unitStatisticsParams;

    /* Call Classifier unit API to get statistics configuration parameters */
    rc = CfyE_Device_Config_Get(macSecUnitId, 0, &unitTopParams);

    /* Get statistics settings */
    statisticsCfgPtr->countFrameThr.l[0]     = unitTopParams.StatControl_p->CountFrameThr.Lo;
    statisticsCfgPtr->countFrameThr.l[1]     = unitTopParams.StatControl_p->CountFrameThr.Hi;
    statisticsCfgPtr->portCountFrameThr.l[0] = unitTopParams.StatControl_p->ChanCountFrameThr.Lo;
    statisticsCfgPtr->portCountFrameThr.l[1] = unitTopParams.StatControl_p->ChanCountFrameThr.Hi;
    statisticsCfgPtr->countIncDisCtrl        = unitTopParams.StatControl_p->CountIncDisCtrl;
    statisticsCfgPtr->autoStatCntrsReset     = unitTopParams.StatControl_p->fAutoStatCntrsReset;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyStatisticsConfigGet function
* @endinternal
*
* @brief  Get statistics control settings from MACSec classifier units
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] statisticsCfgPtr     - (pointer to) statistics control parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatisticsConfigGet
(
    IN   GT_U8                                             devNum,
    IN   GT_MACSEC_UNIT_BMP                                unitBmp,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyStatisticsConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, statisticsCfgPtr));

    rc = internal_cpssDxChMacSecClassifyStatisticsConfigGet(devNum, unitBmp, statisticsCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, statisticsCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyControlPktDetectConfigSet function
* @endinternal
*
* @brief  Control packet detection settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ctrlPktDetectPtr     - (pointer to) control packet detection parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyControlPktDetectConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_ControlPacket_t     unitControlPacketDetectionParams;
    GT_U32                   i;
    GT_U32                   j;
    GT_U32                   maskEnableOffset = 0;
    GT_U32                   maskModeOffset = 0;
    GT_U32                   maskEnableEtStartLocation;

    /* Buffer to hold 16 MAC DAs
       etherAddr[0-7]    8 MAC DAs of MAC DA & ET type
       etherAddr[8,9]    First pair of MAC DAs range & ET type (MAC DA start & MAC DA end)
       etherAddr[10,11]  Second pair of MAC DAs range & ET type (MAC DA start & MAC DA end)
       etherAddr[12,13]  MAC DAs start and end of MAC DA range type
       etherAddr[14,15   MAC DAs of 44 and 48 bits constant  types */
    GT_U8                    etherAddr[16][6];


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ctrlPktDetectPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures and buffer before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitControlPacketDetectionParams, 0, sizeof(unitControlPacketDetectionParams));
    cpssOsMemSet(&etherAddr, 0, sizeof(etherAddr));


    /**** Prepare MAC DA & EtherType rules settings *****/

    /* Start location of EtherType mask enable bits section */
    maskEnableEtStartLocation = CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS;

    /* Go over each rule and update parameters accordingly */
    for (i=0;i<CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS;i++)
    {
        /* MAC DA Enable mask (primary & secondary). Bits 0 till 7 */
        unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macDaEtRules[i].macDaMask.primaryMask)<<(i);
        unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macDaEtRules[i].macDaMask.secondaryMask)<<(i);

        /* EtherType Enable mask (primary & secondary). Bits 8 till 15 */
        unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macDaEtRules[i].etherTypeMask.primaryMask)<<(i+maskEnableEtStartLocation);
        unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macDaEtRules[i].etherTypeMask.secondaryMask)<<(i+maskEnableEtStartLocation);

        /* EtherType match mode mask (primary & secondary). Bits 0 till 7 */
        unitControlPacketDetectionParams.CPMatchModeMask |= (ctrlPktDetectPtr->macDaEtRules[i].etherTypeMatchMode.primaryMatchMode)<<(maskModeOffset);
        unitControlPacketDetectionParams.SCPMatchModeMask |= (ctrlPktDetectPtr->macDaEtRules[i].etherTypeMatchMode.secondaryMatchMode)<<(maskModeOffset);

        /* Increment offset by 1 for next iteration */
        maskModeOffset += 1;

        /* MAC DA value (6 bytes) */
        for (j=0;j<6;j++)
        {
            etherAddr[i][j] = ctrlPktDetectPtr->macDaEtRules[i].macDa.arEther[j];
        }
        /* Update MAC DA pointers (etherAddr[0 - 7]) */
        unitControlPacketDetectionParams.MAC_DA_ET_Rules[i].MAC_DA_p = etherAddr[i];

        /* EtherType value */
        unitControlPacketDetectionParams.MAC_DA_ET_Rules[i].EtherType = ctrlPktDetectPtr->macDaEtRules[i].etherType;
    }

    /* Update offset by number of MAC DA and EtherType rules */
    maskEnableOffset = maskEnableEtStartLocation + CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS ;


    /**** Prepare combined MAC DA range & EtherType rules settings ****/

    /* Go over each rule and update parameters accordingly */
    for (i=0;i<CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULES_COUNT_CNS;i++)
    {
        /* Combined MAC DA range and Ethertype Enable mask (primary & secondary). Bits 16 & 17 */
        unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macDaEtRange[i].macDaRangeEtherTypeMask.primaryMask)<<(maskEnableOffset);
        unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macDaEtRange[i].macDaRangeEtherTypeMask.secondaryMask)<<(maskEnableOffset);

        /* Increment offset by 1 for next iteration */
        maskEnableOffset += 1;

        /* EtherType match mode mask (primary & secondary). Bits 8 & 9 */
        unitControlPacketDetectionParams.CPMatchModeMask |= (ctrlPktDetectPtr->macDaEtRange[i].etherTypeMatchMode.primaryMatchMode)<<(maskModeOffset);
        unitControlPacketDetectionParams.SCPMatchModeMask |= (ctrlPktDetectPtr->macDaEtRange[i].etherTypeMatchMode.secondaryMatchMode)<<(maskModeOffset);

        /* Increment offset by 1 for next iteration */
        maskModeOffset += 1;

        /* MAC DA range values (6 bytes) */
        for (j=0;j<6;j++)
        {
            etherAddr[8+(i*2)][j] = ctrlPktDetectPtr->macDaEtRange[i].macDaStart.arEther[j];
            etherAddr[9+(i*2)][j] = ctrlPktDetectPtr->macDaEtRange[i].macDaEnd.arEther[j];
        }
        /* Update MAC DA range pointers
           etherAddr[8,9] points to first pair of MAC DA start and MAC DA end
           etherAddr[10,11] points to second pair of MAC DA start and MAC DA end */
        unitControlPacketDetectionParams.MAC_DA_ET_Range[i].Range.MAC_DA_Start_p = etherAddr[8+(i*2)];
        unitControlPacketDetectionParams.MAC_DA_ET_Range[i].Range.MAC_DA_End_p = etherAddr[9+(i*2)];

        /* EtherType value */
        unitControlPacketDetectionParams.MAC_DA_ET_Range[i].EtherType = ctrlPktDetectPtr->macDaEtRange[i].etherType;
    }


    /**** Prepare MAC DA range rules settings ****/

    /* MAC DA range Enable mask (primary & secondary). Bit 18 */
    unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macDaRange.macRangeMask.primaryMask)<<(maskEnableOffset);
    unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macDaRange.macRangeMask.secondaryMask)<<(maskEnableOffset);

    /* Increment offset by 1 */
    maskEnableOffset += 1;


    /* MAC DA range values (6 bytes) */
    for (j=0;j<6;j++)
    {
        etherAddr[12][j] = ctrlPktDetectPtr->macDaRange.macDaStart.arEther[j];
        etherAddr[13][j] = ctrlPktDetectPtr->macDaRange.macDaEnd.arEther[j];
    }
    /* Update MAC DA range pointers
       etherAddr[12] points to MAC DA start
       etherAddr[13] points to MAC DA end */
    unitControlPacketDetectionParams.MAC_DA_Range.MAC_DA_Start_p = etherAddr[12];
    unitControlPacketDetectionParams.MAC_DA_Range.MAC_DA_End_p = etherAddr[13];


    /**** Prepare MAC DA 44 bits constant rules settings ****/

    /* MAC DA 44 bits constant Enable mask (primary & secondary). Bit 19 */
    unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macDa44BitsConstMask.primaryMask)<<(maskEnableOffset);
    unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macDa44BitsConstMask.secondaryMask)<<(maskEnableOffset);

    /* Increment offset by 1 */
    maskEnableOffset += 1 ;

    /* MAC DA 44 bits constant value (6 bytes) */
    for (j=0;j<6;j++)
    {
        etherAddr[14][j] = ctrlPktDetectPtr->macDa44BitsConst.arEther[j];
    }
    /* Update MAC DA 44 bits constant pointer (etherAddr[14]) */
    unitControlPacketDetectionParams.MAC_DA_44Bit_Const_p = etherAddr[14];


    /**** Prepare MAC DA 48 bits constant rules settings ****/

    /* MAC DA 48 bits constant Enable mask (primary & secondary). Bit 20 */
    unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macDa48BitsConstMask.primaryMask)<<(maskEnableOffset);
    unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macDa48BitsConstMask.secondaryMask)<<(maskEnableOffset);

    /* MAC DA 48 bits constant value (6 bytes) */
    for (j=0;j<6;j++)
    {
        etherAddr[15][j] = ctrlPktDetectPtr->macDa48BitsConst.arEther[j];
    }
    /* Update MAC DA 48 bits constant pointer (etherAddr[15]) */
    unitControlPacketDetectionParams.MAC_DA_48Bit_Const_p = etherAddr[15];


    /**** Prepare KaY packet rules settings ****/

    /* Update mask enable offset to point to last bit (MS bit). Bit 31 */
    maskEnableOffset = 31 ;

    /* KaY packet Enable mask (primary & secondary) */
    unitControlPacketDetectionParams.CPMatchEnableMask |= (ctrlPktDetectPtr->macSecKayPktMask.primaryKay)<<(maskEnableOffset);
    unitControlPacketDetectionParams.SCPMatchEnableMask |= (ctrlPktDetectPtr->macSecKayPktMask.secondaryKay)<<(maskEnableOffset);

    /* Update control packet pointer */
    unitTopParams.CP_p = &unitControlPacketDetectionParams;

    /* Call Classifier update API to apply these parameters */
    rc = CfyE_Device_Update(macSecUnitId, macSecChannelNum, &unitTopParams);

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyControlPktDetectConfigSet function
* @endinternal
*
* @brief  Control packet detection settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ctrlPktDetectPtr     - (pointer to) control packet detection parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyControlPktDetectConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyControlPktDetectConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, ctrlPktDetectPtr));

    rc = internal_cpssDxChMacSecPortClassifyControlPktDetectConfigSet(devNum, portNum, direction, ctrlPktDetectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, ctrlPktDetectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyControlPktDetectConfigGet function
* @endinternal
*
* @brief  Control packet detection settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[out] ctrlPktDetectPtr     - (pointer to) control packet detection parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyControlPktDetectConfigGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PHYSICAL_PORT_NUM                       portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecChannelNum;
    GT_U32                   macSecUnitId;
    CfyE_Device_t            unitTopParams;
    CfyE_ControlPacket_t     unitControlPacketDetectionParams;
    GT_U32                   i;
    GT_U32                   j;
    GT_U32                   maskEnableOffset = 0;
    GT_U32                   maskModeOffset = 0;
    GT_U32                   maskEnableEtStartLocation;

    /* Buffer to hold 16 MAC DAs
       etherAddr[0-7]    8 MAC DAs of MAC DA & ET type
       etherAddr[8,9]    First pair of MAC DAs range & ET type (MAC DA start & MAC DA end)
       etherAddr[10,11]  Second pair of MAC DAs range & ET type (MAC DA start & MAC DA end)
       etherAddr[12,13]  MAC DAs start and end of MAC DA range type
       etherAddr[14,15   MAC DAs of 44 and 48 bits constant  types */
    GT_U8                    etherAddr[16][6];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ctrlPktDetectPtr);

    /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
    rc = prvCpssMacSecPortToUnitId(devNum,
                                   portNum,
                                   direction,
                                   CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                   &macSecChannelNum,
                                   &macSecUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures and buffer before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitControlPacketDetectionParams, 0, sizeof(unitControlPacketDetectionParams));
    cpssOsMemSet(&etherAddr, 0, sizeof(etherAddr));


     /* Make all Ethernet address fields in structure point to valid buffers */
     for (j=0; j<8; j++)
         unitControlPacketDetectionParams.MAC_DA_ET_Rules[j].MAC_DA_p         = etherAddr[j];
     unitControlPacketDetectionParams.MAC_DA_ET_Range[0].Range.MAC_DA_Start_p = etherAddr[8];
     unitControlPacketDetectionParams.MAC_DA_ET_Range[0].Range.MAC_DA_End_p   = etherAddr[9];
     unitControlPacketDetectionParams.MAC_DA_ET_Range[1].Range.MAC_DA_Start_p = etherAddr[10];
     unitControlPacketDetectionParams.MAC_DA_ET_Range[1].Range.MAC_DA_End_p   = etherAddr[11];
     unitControlPacketDetectionParams.MAC_DA_Range.MAC_DA_Start_p             = etherAddr[12];
     unitControlPacketDetectionParams.MAC_DA_Range.MAC_DA_End_p               = etherAddr[13];
     unitControlPacketDetectionParams.MAC_DA_44Bit_Const_p                    = etherAddr[14];
     unitControlPacketDetectionParams.MAC_DA_48Bit_Const_p                    = etherAddr[15];

    /* Update unit statistics pointer */
    unitTopParams.CP_p = &unitControlPacketDetectionParams;

    /* Call Classifier unit API to get Control packet detection settings */
    rc = CfyE_Device_Config_Get(macSecUnitId, macSecChannelNum, &unitTopParams);

    /**** Get MAC DA & EtherType rules settings *****/

    /* Start location of EtherType mask enable bits section */
    maskEnableEtStartLocation = CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS;

    /* Go over each rule and update parameters accordingly */
    for (i=0;i<CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS;i++)
    {
        /* MAC DA Enable mask (primary & secondary). Bits 0 till 7 */
        ctrlPktDetectPtr->macDaEtRules[i].macDaMask.primaryMask = ((unitTopParams.CP_p->CPMatchEnableMask)>>(i)) & BIT_0;
        ctrlPktDetectPtr->macDaEtRules[i].macDaMask.secondaryMask = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(i)) & BIT_0;

        /* EtherType Enable mask (primary & secondary). Bits 8 till 15 */
        ctrlPktDetectPtr->macDaEtRules[i].etherTypeMask.primaryMask   = ((unitTopParams.CP_p->CPMatchEnableMask)>>(i+maskEnableEtStartLocation)) & BIT_0;
        ctrlPktDetectPtr->macDaEtRules[i].etherTypeMask.secondaryMask = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(i+maskEnableEtStartLocation)) & BIT_0;

        /* EtherType match mode mask (primary & secondary). Bits 0 till 7 */
        ctrlPktDetectPtr->macDaEtRules[i].etherTypeMatchMode.primaryMatchMode = ((unitTopParams.CP_p->CPMatchModeMask)>>(maskModeOffset)) & BIT_0;
        ctrlPktDetectPtr->macDaEtRules[i].etherTypeMatchMode.secondaryMatchMode = ((unitTopParams.CP_p->SCPMatchModeMask)>>(maskModeOffset)) & BIT_0;

        /* Increment offset by 1 for next iteration */
        maskModeOffset += 1;

        /* MAC DA value (6 bytes) */
        for (j=0;j<6;j++)
        {
            ctrlPktDetectPtr->macDaEtRules[i].macDa.arEther[j] = etherAddr[i][j] ;
        }

        /* EtherType value */
        ctrlPktDetectPtr->macDaEtRules[i].etherType = unitTopParams.CP_p->MAC_DA_ET_Rules[i].EtherType ;
    }

    /* Update offset by number of MAC DA and EtherType rules */
    maskEnableOffset = maskEnableEtStartLocation + CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS ;


    /**** Get combined MAC DA range & EtherType rules settings ****/

    /* Go over each rule and update parameters accordingly */
    for (i=0;i<CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULES_COUNT_CNS;i++)
    {
        /* Combined MAC DA range and Ethertype Enable mask (primary & secondary). Bits 16 & 17 */
        ctrlPktDetectPtr->macDaEtRange[i].macDaRangeEtherTypeMask.primaryMask = ((unitTopParams.CP_p->CPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;
        ctrlPktDetectPtr->macDaEtRange[i].macDaRangeEtherTypeMask.secondaryMask = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;

        /* Increment offset by 1 for next iteration */
        maskEnableOffset += 1;

        /* EtherType match mode mask (primary & secondary). Bits 8 & 9 */
        ctrlPktDetectPtr->macDaEtRange[i].etherTypeMatchMode.primaryMatchMode = ((unitTopParams.CP_p->CPMatchModeMask)>>(maskModeOffset)) & BIT_0;
        ctrlPktDetectPtr->macDaEtRange[i].etherTypeMatchMode.secondaryMatchMode = ((unitTopParams.CP_p->SCPMatchModeMask)>>(maskModeOffset)) & BIT_0;

        /* Increment offset by 1 for next iteration */
        maskModeOffset += 1;

        /* MAC DA range values (6 bytes) */
        for (j=0;j<6;j++)
        {
            ctrlPktDetectPtr->macDaEtRange[i].macDaStart.arEther[j] = etherAddr[8 + (i*2)][j];
            ctrlPktDetectPtr->macDaEtRange[i].macDaEnd.arEther[j] = etherAddr[9 + (i*2)][j];
        }

        /* EtherType value */
        ctrlPktDetectPtr->macDaEtRange[i].etherType = unitTopParams.CP_p->MAC_DA_ET_Range[i].EtherType;
    }


    /**** Get MAC DA range rules settings ****/

    /* MAC DA range Enable mask (primary & secondary). Bit 18 */
    ctrlPktDetectPtr->macDaRange.macRangeMask.primaryMask = ((unitTopParams.CP_p->CPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;
    ctrlPktDetectPtr->macDaRange.macRangeMask.secondaryMask = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;

    /* Increment offset by 1 */
    maskEnableOffset += 1;

    /* MAC DA range values (6 bytes) */
    for (j=0;j<6;j++)
    {
        ctrlPktDetectPtr->macDaRange.macDaStart.arEther[j] = etherAddr[12][j];
        ctrlPktDetectPtr->macDaRange.macDaEnd.arEther[j] = etherAddr[13][j];
    }


    /**** Get MAC DA 44 bits constant rules settings ****/

    /* MAC DA 44 bits constant Enable mask (primary & secondary). Bit 19 */
    ctrlPktDetectPtr->macDa44BitsConstMask.primaryMask = ((unitTopParams.CP_p->CPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;
    ctrlPktDetectPtr->macDa44BitsConstMask.secondaryMask = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;

    /* Increment offset by 1 */
    maskEnableOffset += 1 ;

    /* MAC DA 44 bits constant value (6 bytes) */
    for (j=0;j<6;j++)
    {
        ctrlPktDetectPtr->macDa44BitsConst.arEther[j] = etherAddr[14][j];
    }


    /**** Get MAC DA 48 bits constant rules settings ****/

    /* MAC DA 48 bits constant Enable mask (primary & secondary). Bit 20 */
    ctrlPktDetectPtr->macDa48BitsConstMask.primaryMask = ((unitTopParams.CP_p->CPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;
    ctrlPktDetectPtr->macDa48BitsConstMask.secondaryMask = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;

    /* MAC DA 48 bits constant value (6 bytes)*/
    for (j=0;j<6;j++)
    {
        ctrlPktDetectPtr->macDa48BitsConst.arEther[j] = etherAddr[15][j];
    }

    /**** Get KaY packet rules settings ****/

    /* Update mask enable offset to point to last bit (MS bit). Bit 31 */
    maskEnableOffset = 31 ;

    /* KaY packet Enable mask (primary & secondary) */
    ctrlPktDetectPtr->macSecKayPktMask.primaryKay = ((unitTopParams.CP_p->CPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;
    ctrlPktDetectPtr->macSecKayPktMask.secondaryKay = ((unitTopParams.CP_p->SCPMatchEnableMask)>>(maskEnableOffset)) & BIT_0;

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyControlPktDetectConfigGet function
* @endinternal
*
* @brief  Get control packet detection settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[out] ctrlPktDetectPtr     - (pointer to) control packet detection parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyControlPktDetectConfigGet
(
    IN   GT_U8                                      devNum,
    IN   GT_PHYSICAL_PORT_NUM                       portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyControlPktDetectConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, direction, ctrlPktDetectPtr));

    rc = internal_cpssDxChMacSecPortClassifyControlPktDetectConfigGet(devNum, portNum, direction, ctrlPktDetectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, direction, ctrlPktDetectPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsConfigSet function
* @endinternal
*
* @brief   Statistics control settings in MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] statisticsCtrlPtr    - (pointer to) statistic configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsConfigSet
(
    IN  GT_U8                                         devNum,
    IN  GT_MACSEC_UNIT_BMP                            unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS                  rc;
    GT_U32                     macSecUnitId;
    SecY_Device_Params_t       unitTopParams;
    SecY_Statistics_Control_t  unitStatisticsParams;
    GT_U32                     macSecDpId;
    GT_U32                     macSecMaxNumDp;
    GT_U32                     macSecDir;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statisticsCfgPtr);

    /* Check unitBmp is valid */
    rc = prvCpssMacSecBmpCheck(devNum, &unitBmp, GT_TRUE/*multipe units are allowed*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitStatisticsParams, 0, sizeof(unitStatisticsParams));

    /* Prepare statistics parameters */
    unitStatisticsParams.CountIncDisCtrl       = statisticsCfgPtr->countIncDisCtrl;
    unitStatisticsParams.fAutoStatCntrsReset   = statisticsCfgPtr->autoStatCntrsReset;
    unitStatisticsParams.IFC1CountFrameThr.Lo  = statisticsCfgPtr->ifc1CountFrameThr.l[0];
    unitStatisticsParams.IFC1CountFrameThr.Hi  = statisticsCfgPtr->ifc1CountFrameThr.l[1];
    unitStatisticsParams.IFC1CountOctetThr.Lo  = statisticsCfgPtr->ifc1CountOctetThr.l[0];
    unitStatisticsParams.IFC1CountOctetThr.Hi  = statisticsCfgPtr->ifc1CountOctetThr.l[1];
    unitStatisticsParams.IFCCountFrameThr.Lo   = statisticsCfgPtr->ifcCountFrameThr.l[0];
    unitStatisticsParams.IFCCountFrameThr.Hi   = statisticsCfgPtr->ifcCountFrameThr.l[1];
    unitStatisticsParams.IFCCountOctetThr.Lo   = statisticsCfgPtr->ifcCountOctetThr.l[0];
    unitStatisticsParams.IFCCountOctetThr.Hi   = statisticsCfgPtr->ifcCountOctetThr.l[1];
    unitStatisticsParams.RxCAMCountFrameThr.Lo = statisticsCfgPtr->rxCamCountFrameThr.l[0];
    unitStatisticsParams.RxCAMCountFrameThr.Hi = statisticsCfgPtr->rxCamCountFrameThr.l[1];
    unitStatisticsParams.SACountFrameThr.Lo    = statisticsCfgPtr->saCountFrameThr.l[0];
    unitStatisticsParams.SACountFrameThr.Hi    = statisticsCfgPtr->saCountFrameThr.l[1];
    unitStatisticsParams.SACountOctetThr.Lo    = statisticsCfgPtr->saCountOctetThr.l[0];
    unitStatisticsParams.SACountOctetThr.Hi    = statisticsCfgPtr->saCountOctetThr.l[1];
    unitStatisticsParams.SecYCountFrameThr.Lo  = statisticsCfgPtr->secyCountFrameThr.l[0];
    unitStatisticsParams.SecYCountFrameThr.Hi  = statisticsCfgPtr->secyCountFrameThr.l[1];
    unitStatisticsParams.SeqNrThreshold        = statisticsCfgPtr->seqNrThreshold;
    unitStatisticsParams.SeqNrThreshold64.Lo   = statisticsCfgPtr->seqNrThreshold64.l[0];
    unitStatisticsParams.SeqNrThreshold64.Hi   = statisticsCfgPtr->seqNrThreshold64.l[1];

    /* Update unit statistics pointer */
    unitTopParams.StatControl_p = &unitStatisticsParams;

    /* Get maximum number of Data path units connected to MACSec */
    macSecMaxNumDp = ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp) >> 1) + 1;

    /* Loop over the maximum number of Data paths. For Hawk:2 for Phoenix:1  */
    for (macSecDpId = 0; (macSecDpId < macSecMaxNumDp); macSecDpId++)
    {
        /* Check Data path bits map. Step in only if bit is on */
        if ( (unitBmp & (1 << macSecDpId)) != 0 )
        {
            /* Loop over egress and ingress directions */
            for (macSecDir = 0; (macSecDir < 2); macSecDir++)
            {
                /* Get MACSec unit ID */
                macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,macSecDpId,CPSS_DXCH_MACSEC_TRANSFORMER_E,macSecDir);

                /* Verify this specific MACSec unit was indeed initialized */
                rc = prvCpssMacSecInitCheck(CPSS_DXCH_MACSEC_TRANSFORMER_E,macSecUnitId);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* Call Transformer update API to apply these parameters */
                rc = SecY_Device_Update(macSecUnitId, &unitTopParams);
            }
        }
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsConfigSet function
* @endinternal
*
* @brief   Statistics control settings in MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] statisticsCtrlPtr    - (pointer to) statistic configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsConfigSet
(
    IN  GT_U8                                         devNum,
    IN  GT_MACSEC_UNIT_BMP                            unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, statisticsCfgPtr));

    rc = internal_cpssDxChMacSecSecyStatisticsConfigSet(devNum, unitBmp, statisticsCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, statisticsCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatisticsConfigGet function
* @endinternal
*
* @brief   Get statistics control settings from MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] statisticsCtrlPtr   - (pointer to) statistic configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatisticsConfigGet
(
    IN   GT_U8                                         devNum,
    IN   GT_MACSEC_UNIT_BMP                            unitBmp,
    OUT  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS                  rc;
    GT_U32                     macSecUnitId;
    SecY_Device_Params_t       unitTopParams;
    SecY_Statistics_Control_t  unitStatisticsParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(statisticsCfgPtr);

    /* Check unitBmp is valid */
    rc = prvCpssMacSecBmpCheck(devNum, &unitBmp, GT_TRUE/*multipe units are allowed*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Since statistics configurations are the same in all Transformer units we only need to read it from one of them.
       Verify which Data path is requested, select Ingress (to include rx cam) and get related MACSec unit ID  */

    /* At this point unitBmp must be valid. */
    if ( (unitBmp & 1) == 1 )
    {
        /* Get MACSec unit ID connected to Data path 0 */
        macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,0,CPSS_DXCH_MACSEC_TRANSFORMER_E,CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E);
    }
    else
    {
        /* Get MACSec unit ID connected to Data path 1 */
        macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,1,CPSS_DXCH_MACSEC_TRANSFORMER_E,CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E);
    }

    /* Verify this specific MACSec unit was indeed initialized */
    rc = prvCpssMacSecInitCheck(CPSS_DXCH_MACSEC_TRANSFORMER_E,macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitTopParams, 0, sizeof(unitTopParams));
    cpssOsMemSet(&unitStatisticsParams, 0, sizeof(unitStatisticsParams));

    /* Update unit statistics pointer */
    unitTopParams.StatControl_p = &unitStatisticsParams;

    /* Call Transformer unit API to get statistics configuration parameters */
    rc = SecY_Device_Config_Get(macSecUnitId, &unitTopParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get statistics settings */
    statisticsCfgPtr->autoStatCntrsReset      = unitTopParams.StatControl_p->fAutoStatCntrsReset;
    statisticsCfgPtr->countIncDisCtrl         = unitTopParams.StatControl_p->CountIncDisCtrl;
    statisticsCfgPtr->ifc1CountFrameThr.l[0]  = unitTopParams.StatControl_p->IFC1CountFrameThr.Lo;
    statisticsCfgPtr->ifc1CountFrameThr.l[1]  = unitTopParams.StatControl_p->IFC1CountFrameThr.Hi;
    statisticsCfgPtr->ifc1CountOctetThr.l[0]  = unitTopParams.StatControl_p->IFC1CountOctetThr.Lo;
    statisticsCfgPtr->ifc1CountOctetThr.l[1]  = unitTopParams.StatControl_p->IFC1CountOctetThr.Hi;
    statisticsCfgPtr->ifcCountFrameThr.l[0]   = unitTopParams.StatControl_p->IFCCountFrameThr.Lo;
    statisticsCfgPtr->ifcCountFrameThr.l[1]   = unitTopParams.StatControl_p->IFCCountFrameThr.Hi;
    statisticsCfgPtr->ifcCountOctetThr.l[0]   = unitTopParams.StatControl_p->IFCCountOctetThr.Lo;
    statisticsCfgPtr->ifcCountOctetThr.l[1]   = unitTopParams.StatControl_p->IFCCountOctetThr.Hi;
    statisticsCfgPtr->rxCamCountFrameThr.l[0] = unitTopParams.StatControl_p->RxCAMCountFrameThr.Lo;
    statisticsCfgPtr->rxCamCountFrameThr.l[1] = unitTopParams.StatControl_p->RxCAMCountFrameThr.Hi;
    statisticsCfgPtr->saCountFrameThr.l[0]    = unitTopParams.StatControl_p->SACountFrameThr.Lo;
    statisticsCfgPtr->saCountFrameThr.l[1]    = unitTopParams.StatControl_p->SACountFrameThr.Hi;
    statisticsCfgPtr->saCountOctetThr.l[0]    = unitTopParams.StatControl_p->SACountOctetThr.Lo;
    statisticsCfgPtr->saCountOctetThr.l[1]    = unitTopParams.StatControl_p->SACountOctetThr.Hi;
    statisticsCfgPtr->secyCountFrameThr.l[0]  = unitTopParams.StatControl_p->SecYCountFrameThr.Lo;
    statisticsCfgPtr->secyCountFrameThr.l[1]  = unitTopParams.StatControl_p->SecYCountFrameThr.Hi;

    /* following configurations are in Egress units only */
    if ( (unitBmp & 1) == 1 )
    {
        /* Get MACSec unit ID connected to Data path 0 */
        macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,0,CPSS_DXCH_MACSEC_TRANSFORMER_E,CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
    }
    else
    {
        /* Get MACSec unit ID connected to Data path 1 */
        macSecUnitId = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,1,CPSS_DXCH_MACSEC_TRANSFORMER_E,CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E);
    }

    /* Verify this specific MACSec unit was indeed initialized */
    rc = prvCpssMacSecInitCheck(CPSS_DXCH_MACSEC_TRANSFORMER_E,macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them  */
    cpssOsMemSet(&unitStatisticsParams, 0, sizeof(unitStatisticsParams));


    /* Call Transformer unit API to get statistics configuration parameters */
    rc = SecY_Device_Config_Get(macSecUnitId, &unitTopParams);

    statisticsCfgPtr->seqNrThreshold          = unitTopParams.StatControl_p->SeqNrThreshold;
    statisticsCfgPtr->seqNrThreshold64.l[0]   = unitTopParams.StatControl_p->SeqNrThreshold64.Lo;
    statisticsCfgPtr->seqNrThreshold64.l[1]   = unitTopParams.StatControl_p->SeqNrThreshold64.Hi;

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatisticsConfigGet function
* @endinternal
*
* @brief   Get statistics control settings from MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] statisticsCtrlPtr   - (pointer to) statistic configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsConfigGet
(
    IN   GT_U8                                         devNum,
    IN   GT_MACSEC_UNIT_BMP                            unitBmp,
    OUT  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatisticsConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, statisticsCfgPtr));

    rc = internal_cpssDxChMacSecSecyStatisticsConfigGet(devNum, unitBmp, statisticsCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, statisticsCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportSet function
* @endinternal
*
* @brief   Updates a vPort (vPort policy) for an already added vPort for a MACSec Classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - vPort handle for vPort to be updated.
* @param[in] vPortParamsPtr       - (pointer to) data for the vPort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    CfyE_vPort_t              vPortParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vPortParamsPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* Update vPort structure with vPort parameters */
    vPortParams.SecTagOffset = vPortParamsPtr->secTagOffset;
    vPortParams.PktExtension = vPortParamsPtr->pktExpansion;

    /* Call Classifier API to update vPort policy */
    rc = CfyE_vPort_Update(macSecUnitId,(CfyE_vPortHandle_t)vPortHandle, &vPortParams);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportSet function
* @endinternal
*
* @brief   Updates a vPort (vPort policy) for an already added vPort for a MACSec Classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - vPort handle for vPort to be updated.
* @param[in] vPortParamsPtr       - (pointer to) data for the vPort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortHandle, vPortParamsPtr));

    rc = internal_cpssDxChMacSecClassifyVportSet(devNum, unitBmp, direction, vPortHandle, vPortParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortHandle, vPortParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportGet function
* @endinternal
*
* @brief   Read the vPort policy of the given vPort from the required MACSec Classifer
*   NOTEs: - A vPort can be read from a MACSec classifier instance only after this vPort has been added to it
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for vPort to be updated
* @param[out] vPortDataPtr        - (pointer to) data for the vPort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortDataPtr
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    CfyE_vPort_t              vPortParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vPortDataPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&vPortParams, 0, sizeof(vPortParams));

    /* Call Classifier API to get vPort policy that related to this vPort handle */
    rc = CfyE_vPort_Read(macSecUnitId,(CfyE_vPortHandle_t)vPortHandle, &vPortParams);

    /* Update vPort structure with vPort parameters */
    vPortDataPtr->secTagOffset = vPortParams.SecTagOffset;
    vPortDataPtr->pktExpansion = vPortParams.PktExtension;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportGet function
* @endinternal
*
* @brief   Read the vPort policy of the given vPort from the required MACSec Classifer
*   NOTEs: - A vPort can be read from a MACSec classifier instance only after this vPort has been added to it
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for vPort to be updated
* @param[out] vPortDataPtr        - (pointer to) data for the vPort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortHandle, vPortDataPtr));

    rc = internal_cpssDxChMacSecClassifyVportGet(devNum, unitBmp, direction, vPortHandle, vPortDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortHandle, vPortDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportHandleGet function
* @endinternal
*
* @brief   Get the vPort handle from an existing vPort, using the vPort index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in]  vPortIndex          - vPort index for which the vPort handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[out] vPortHandlePtr      - (pointer to) vPort handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  vPortIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    CfyE_vPortHandle_t        cfyeVportHandle = CfyE_vPortHandle_NULL;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(vPortHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check vPortIndex is in valid range */
    if (vPortIndex >= PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[macSecUnitId]->vPortCount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vPortIndex(%d) is bigger than maximum allowed number of vPorts(%d)"
                                      ,vPortIndex,PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[macSecUnitId]->vPortCount);
    }


    /* Call Classifier API to get vPort handle that related to this vPort index */
    rc = CfyE_vPortHandle_Get(macSecUnitId, vPortIndex, &cfyeVportHandle);

    /* Get Classifier vPort handle */
    *vPortHandlePtr = (CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE)cfyeVportHandle;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportHandleGet function
* @endinternal
*
* @brief   Get the vPort handle from an existing vPort, using the vPort index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in]  vPortIndex          - vPort index for which the vPort handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[out] vPortHandlePtr      - (pointer to) vPort handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  vPortIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportHandleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortIndex, vPortHandlePtr));

    rc = internal_cpssDxChMacSecClassifyVportHandleGet(devNum, unitBmp, direction, vPortIndex, vPortHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortIndex, vPortHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaSet function
* @endinternal
*
* @brief   Updates SA flow parameters for an already added SA for a MACSec Transformer instance
*
*   NOTEs: This API doesn't update the SA transform record data and can't be used to update the key in the transform record
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - SA handle of the SA to be updated.
* @param[in] saParamsPtr          - (pointer to) new parameters for the SA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaSet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC      *saParamsPtr
)
{
    GT_STATUS         rc;
    GT_U32            macSecUnitId;
    SecY_SA_t         saParams;
    SecY_SAHandle_t   secySaHandle;
    GT_U32            vPortId = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(saParamsPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* If egress, check vPort parameters are aligned with current SA parameters */
    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {
        /* Get vPort ID that is associated with this SA handle */
        rc = SecY_SA_vPortIndex_Get(macSecUnitId, secySaHandle, &vPortId);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Check vPort and SA parameters are compatible */
        rc = prvCpssDxChMacSecVportVsSaCheck(devNum, unitBmp, vPortId, saParamsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&saParams, 0, sizeof(saParams));

    /* Update SA structure with SA parameters */
    saParams.ActionType        = saParamsPtr->actionType;
    saParams.DestPort          = saParamsPtr->destPort;
    saParams.DropType          = SECY_SA_DROP_INTERNAL;

    /* Keep update according to SA action type */
    switch(saParamsPtr->actionType)
    {
        /* SA action egress */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E:
            saParams.Params.Egress.ConfidentialityOffset = saParamsPtr->params.egress.confidentialityOffset;
            saParams.Params.Egress.fAllowDataPkts        = saParamsPtr->params.egress.controlledPortEnable;
            saParams.Params.Egress.fConfProtect          = saParamsPtr->params.egress.confProtect;
            saParams.Params.Egress.fIncludeSCI           = saParamsPtr->params.egress.includeSci;
            saParams.Params.Egress.fProtectFrames        = saParamsPtr->params.egress.protectFrames;
            saParams.Params.Egress.fSAInUse              = saParamsPtr->params.egress.saInUse;
            saParams.Params.Egress.fUseES                = saParamsPtr->params.egress.useEs;
            saParams.Params.Egress.fUseSCB               = saParamsPtr->params.egress.useScb;
            saParams.Params.Egress.PreSecTagAuthLength   = saParamsPtr->params.egress.preSecTagAuthLength;
            saParams.Params.Egress.PreSecTagAuthStart    = saParamsPtr->params.egress.preSecTagAuthStart;
            break;

        /* SA action ingress */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E:
            saParams.Params.Ingress.AN                    = saParamsPtr->params.ingress.an;
            saParams.Params.Ingress.ConfidentialityOffset = saParamsPtr->params.ingress.confidentialityOffset;
            saParams.Params.Ingress.fAllowTagged          = saParamsPtr->params.ingress.allowTagged;
            saParams.Params.Ingress.fAllowUntagged        = saParamsPtr->params.ingress.allowUntagged;
            saParams.Params.Ingress.fReplayProtect        = saParamsPtr->params.ingress.replayProtect;
            saParams.Params.Ingress.fRetainICV            = saParamsPtr->params.ingress.retainIcv;
            saParams.Params.Ingress.fRetainSecTAG         = saParamsPtr->params.ingress.retainSecTag;
            saParams.Params.Ingress.fSAInUse              = saParamsPtr->params.ingress.saInUse;
            saParams.Params.Ingress.fValidateUntagged     = saParamsPtr->params.ingress.validateUntagged;
            saParams.Params.Ingress.PreSecTagAuthLength   = saParamsPtr->params.ingress.preSecTagAuthLength;
            saParams.Params.Ingress.PreSecTagAuthStart    = saParamsPtr->params.ingress.preSecTagAuthStart;
            saParams.Params.Ingress.SCI_p                 = saParamsPtr->params.ingress.sciArr;
            saParams.Params.Ingress.ValidateFramesTagged  = saParamsPtr->params.ingress.validateFramesTagged;
            break;

        /* SA action bypass drop */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E:
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E:
            saParams.Params.BypassDrop.fSAInUse = saParamsPtr->params.bypassDrop.saInUse;
            break;

        /* SA action crypt-auth */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_CRYPT_AUTH_E:
            saParams.Params.CryptAuth.ConfidentialityOffset = saParamsPtr->params.cryptAuth.confidentialityOffset;
            saParams.Params.CryptAuth.fConfProtect          = saParamsPtr->params.cryptAuth.confProtect;
            saParams.Params.CryptAuth.fICVAppend            = saParamsPtr->params.cryptAuth.icvAppend;
            saParams.Params.CryptAuth.fICVVerify            = saParamsPtr->params.cryptAuth.icvVerify;
            saParams.Params.CryptAuth.fZeroLengthMessage    = saParamsPtr->params.cryptAuth.zeroLengthMessage;
            saParams.Params.CryptAuth.IVMode                = saParamsPtr->params.cryptAuth.ivMode;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid SA action type:[%d]", saParams.ActionType);
    }

    /* Call Transformer API to update SA */
    rc = SecY_SA_Update(macSecUnitId, secySaHandle, &saParams);

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaSet function
* @endinternal
*
* @brief   Updates SA flow parameters for an already added SA for a MACSec Transformer instance
*
*   NOTEs: This API doesn't update the SA transform record data and can't be used to update the key in the transform record
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - SA handle of the SA to be updated.
* @param[in] saParamsPtr          - (pointer to) new parameters for the SA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaSet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC      *saParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saHandle, saParamsPtr));

    rc = internal_cpssDxChMacSecSecySaSet(devNum, unitBmp, direction, saHandle, saParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saHandle, saParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaGet function
* @endinternal
*
* @brief   Read the SA flow parameters for an already added SA for a MACSec Transformer instance.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in]  saHandle            - SA handle of the SA to be read.
* @param[out] saParamsPtr         - (pointer to) parameters of the SA
* @param[out] vPortIdPtr          - (pointer to) variable represent the vPort associated with the SA
* @param[out] sciPtr              - (pointer to) SCI of an inbound SA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_STC       *saParamsPtr,
    OUT  GT_U32                             *vPortIdPtr,
    OUT  GT_U8                              sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS]
)
{
    GT_STATUS         rc;
    GT_U32            macSecUnitId;
    SecY_SA_t         saParams;
    SecY_SAHandle_t   secySaHandle;
    GT_U8             sci[8];
    GT_U32            i;
    GT_U32            vPortIndex;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointers are not NULL */
    CPSS_NULL_PTR_CHECK_MAC(saParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(vPortIdPtr);
    if (!sciArr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&saParams, 0, sizeof(saParams));

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Call Transformer API to get SA parameters */
    rc = SecY_SA_Params_Read(macSecUnitId, secySaHandle, &saParams, sci);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Update SA structure with SA parameters */
    saParamsPtr->actionType = saParams.ActionType;
    saParamsPtr->destPort   = saParams.DestPort;

    /* Keep update according to SA action type */
    switch(saParamsPtr->actionType)
    {
        /* SA action egress */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E:
            saParamsPtr->params.egress.confidentialityOffset = saParams.Params.Egress.ConfidentialityOffset;
            saParamsPtr->params.egress.controlledPortEnable  = saParams.Params.Egress.fAllowDataPkts;
            saParamsPtr->params.egress.confProtect           = saParams.Params.Egress.fConfProtect;
            saParamsPtr->params.egress.includeSci            = saParams.Params.Egress.fIncludeSCI;
            saParamsPtr->params.egress.protectFrames         = saParams.Params.Egress.fProtectFrames;
            saParamsPtr->params.egress.saInUse               = saParams.Params.Egress.fSAInUse;
            saParamsPtr->params.egress.useEs                 = saParams.Params.Egress.fUseES;
            saParamsPtr->params.egress.useScb                = saParams.Params.Egress.fUseSCB;
            saParamsPtr->params.egress.preSecTagAuthLength   = saParams.Params.Egress.PreSecTagAuthLength;
            saParamsPtr->params.egress.preSecTagAuthStart    = saParams.Params.Egress.PreSecTagAuthStart;
            break;

        /* SA action ingress */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E:
            saParamsPtr->params.ingress.an                    = saParams.Params.Ingress.AN;
            saParamsPtr->params.ingress.confidentialityOffset = saParams.Params.Ingress.ConfidentialityOffset;
            saParamsPtr->params.ingress.allowTagged           = saParams.Params.Ingress.fAllowTagged;
            saParamsPtr->params.ingress.allowUntagged         = saParams.Params.Ingress.fAllowUntagged;
            saParamsPtr->params.ingress.replayProtect         = saParams.Params.Ingress.fReplayProtect;
            saParamsPtr->params.ingress.retainIcv             = saParams.Params.Ingress.fRetainICV;
            saParamsPtr->params.ingress.retainSecTag          = saParams.Params.Ingress.fRetainSecTAG;
            saParamsPtr->params.ingress.saInUse               = saParams.Params.Ingress.fSAInUse;
            saParamsPtr->params.ingress.validateUntagged      = saParams.Params.Ingress.fValidateUntagged;
            saParamsPtr->params.ingress.preSecTagAuthLength   = saParams.Params.Ingress.PreSecTagAuthLength;
            saParamsPtr->params.ingress.preSecTagAuthStart    = saParams.Params.Ingress.PreSecTagAuthStart;
            saParamsPtr->params.ingress.validateFramesTagged  = saParams.Params.Ingress.ValidateFramesTagged;
            /* Get SCI */
            for (i=0;i<CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS;i++)
            {
                saParamsPtr->params.ingress.sciArr[i]         = sci[i];
                sciArr[i]                                     = sci[i];
            }
            break;

        /* SA action bypass drop */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E:
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E:
            saParamsPtr->params.bypassDrop.saInUse = saParams.Params.BypassDrop.fSAInUse;
            break;

        /* SA action crypt-auth */
        case CPSS_DXCH_MACSEC_SECY_SA_ACTION_CRYPT_AUTH_E:
             saParamsPtr->params.cryptAuth.confidentialityOffset = saParams.Params.CryptAuth.ConfidentialityOffset;
             saParamsPtr->params.cryptAuth.confProtect           = saParams.Params.CryptAuth.fConfProtect;
             saParamsPtr->params.cryptAuth.icvAppend             = saParams.Params.CryptAuth.fICVAppend;
             saParamsPtr->params.cryptAuth.icvVerify             = saParams.Params.CryptAuth.fICVVerify;
             saParamsPtr->params.cryptAuth.zeroLengthMessage     = saParams.Params.CryptAuth.fZeroLengthMessage;
             saParamsPtr->params.cryptAuth.ivMode                = saParams.Params.CryptAuth.IVMode;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid SA action type:[%d]", saParams.ActionType);
    }


    /* Get vPort ID that is associated with this SA handle */
    rc = SecY_SA_vPortIndex_Get(macSecUnitId, secySaHandle, &vPortIndex);

    /* Update vPort number */
    *vPortIdPtr = vPortIndex;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaGet function
* @endinternal
*
* @brief   Read the SA flow parameters for an already added SA for a MACSec Transformer instance.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in]  saHandle            - SA handle of the SA to be read.
* @param[out] saParamsPtr         - (pointer to) parameters of the SA
* @param[out] vPortIdPtr          - (pointer to) variable represent the vPort associated with the SA
* @param[out] sciPtr              - (pointer to) SCI of an inbound SA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_STC       *saParamsPtr,
    OUT  GT_U32                             *vPortIdPtr,
    OUT  GT_U8                              sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saHandle, saParamsPtr, vPortIdPtr, sciArr));

    rc = internal_cpssDxChMacSecSecySaGet(devNum, unitBmp, direction, saHandle, saParamsPtr, vPortIdPtr, sciArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saHandle, saParamsPtr, vPortIdPtr, sciArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaIndexGet function
* @endinternal
*
* @brief   Get the SA index and SC index from an existing SA, using the SA handle.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  saHandle             - SA handle of the SA to get the index from.
* @param[out] saIndexPtr           - (pointer to) SA index.
* @param[out] scIndexPtr           - (pointer to) SC index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaIndexGet
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  GT_U32                             *saIndexPtr,
    OUT  GT_U32                             *scIndexPtr
)
{
    GT_STATUS         rc;
    GT_U32            saIndex;
    GT_U32            scIndex;
    SecY_SAHandle_t   secySaHandle;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointers are not NULL */
    CPSS_NULL_PTR_CHECK_MAC(saIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(scIndexPtr);

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Now get SA and SC indexes */
    rc = SecY_SAIndex_Get(secySaHandle, &saIndex, &scIndex);

    /* The requested SA and SC numbers */
    *saIndexPtr = saIndex;
    *scIndexPtr = scIndex;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaIndexGet function
* @endinternal
*
* @brief   Get the SA index and SC index from an existing SA, using the SA handle.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  saHandle             - SA handle of the SA to get the index from.
* @param[out] saIndexPtr           - (pointer to) SA index.
* @param[out] scIndexPtr           - (pointer to) SC index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaIndexGet
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  GT_U32                             *saIndexPtr,
    OUT  GT_U32                             *scIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, saHandle, saIndexPtr, scIndexPtr));

    rc = internal_cpssDxChMacSecSecySaIndexGet(devNum, saHandle, saIndexPtr, scIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, saHandle, saIndexPtr, scIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaHandleGet function
* @endinternal
*
* @brief   Get the SA handle from an existing SA, using the SA index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  saIndex             - Secure Association index for which the SA handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..511)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[out] saHandlePtr         - (pointer to) SA handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaHandleGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   GT_U32                             saIndex,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_HANDLE    *saHandlePtr
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    SecY_SAHandle_t           secySaHandle = SecY_SAHandle_NULL;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(saHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check saIndex is in valid range */
    if (saIndex >= PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[macSecUnitId]->SACount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "saIndex(%d) is bigger than maximum allowed number of SAs(%d)"
                                      ,saIndex,PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device)[macSecUnitId]->SACount);
    }

    /* Call Transformer API to get SA hadle that is associated with this SA index */
    rc = SecY_SAHandle_Get(macSecUnitId, saIndex, &secySaHandle);

    /* Get Transformer SA handle */
    *saHandlePtr = (CPSS_DXCH_MACSEC_SECY_SA_HANDLE)secySaHandle.p;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaHandleGet function
* @endinternal
*
* @brief   Get the SA handle from an existing SA, using the SA index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  saIndex             - Secure Association index for which the SA handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..511)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[out] saHandlePtr         - (pointer to) SA handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaHandleGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   GT_U32                             saIndex,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_HANDLE    *saHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaHandleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saIndex, saHandlePtr));

    rc = internal_cpssDxChMacSecSecySaHandleGet(devNum, unitBmp, direction, saIndex, saHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saIndex, saHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaActiveGet function
* @endinternal
*
* @brief   For Egress return the handle of the currently active egress SA for a given vPort.
*          For Ingress return the handles of the currently active ingress SA's for a given vPort and SCI.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - device number.
* @param[in] unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                  - 1: select DP0, 2: select DP1.
*                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                - for non multi data paths device this parameter is IGNORED.
* @param[in] direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPortId            - vPort number to which the SA applies.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  sciArr             - (array of) SCI for which the SA is desired (only in Ingress).
* @param[out] activeSaHandleArr  - For Egress: (array of) one currently active egress SA for the given vPort.
*                                  For Ingress: (array of) four currently active ingress SAs for the given vPort and sci,
*                                                one for each AN from 0 to 3. Any SA that is not active is represented by null.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS internal_cpssDxChMacSecSecySaActiveGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  GT_U32                            vPortId,
    IN  GT_U8                             sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS],
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE   activeSaHandleArr[CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS]
)
{
    GT_STATUS                 rc;
    GT_U32                    macSecUnitId;
    SecY_SAHandle_t           egressSaHandle = { NULL };
    SecY_SAHandle_t           ingressSaHandles[CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS];
    GT_U32                    i;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(sciArr);
    CPSS_NULL_PTR_CHECK_MAC(activeSaHandleArr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear Ingress SA handles */
    cpssOsMemSet(&ingressSaHandles, 0, sizeof(ingressSaHandles));

    /* Get active SA handle */
    if (direction == CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E)
    {   /* For Egress return the handle of the currently active egress SA for a given vPort. */
        rc = SecY_SA_Active_E_Get(macSecUnitId, vPortId, &egressSaHandle);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Get Egress active Transformer SA handle */
        activeSaHandleArr[0] = (CPSS_DXCH_MACSEC_SECY_SA_HANDLE)egressSaHandle.p;
    }
    else
    {   /* For Ingress return the handles of the currently active ingress SA's for a given vPort and SCI. */
        rc = SecY_SA_Active_I_Get(macSecUnitId, vPortId, sciArr, &ingressSaHandles[0]);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Get Ingress array of four currently active SA for the given vPort, one for each AN from 0 to 3.
           Any SA that is not active is represented by SecY_SAHandle_NULL. */
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS;i++)
        {
            /* Get active Transformer SA handle */
            activeSaHandleArr[i] = (CPSS_DXCH_MACSEC_SECY_SA_HANDLE)ingressSaHandles[i].p;
        }
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaActiveGet function
* @endinternal
*
* @brief   For Egress return the handle of the currently active egress SA for a given vPort.
*          For Ingress return the handles of the currently active ingress SA's for a given vPort and SCI.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - device number.
* @param[in] unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                  - 1: select DP0, 2: select DP1.
*                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                - for non multi data paths device this parameter is IGNORED.
* @param[in] direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPortId            - vPort number to which the SA applies.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  sciArr             - (array of) SCI for which the SA is desired (only in Ingress).
* @param[out] activeSaHandleArr  - For Egress: (array of) one currently active egress SA for the given vPort.
*                                  For Ingress: (array of) four currently active ingress SAs for the given vPort and sci,
*                                                one for each AN from 0 to 3. Any SA that is not active is represented by null.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaActiveGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  GT_U32                            vPortId,
    IN  GT_U8                             sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS],
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE   activeSaHandleArr[CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaActiveGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortId, sciArr, activeSaHandleArr));

    rc = internal_cpssDxChMacSecSecySaActiveGet(devNum, unitBmp, direction, vPortId, sciArr, activeSaHandleArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortId, sciArr, activeSaHandleArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaNextPnUpdate function
* @endinternal
*
* @brief   Updates sequence number of nextPN for a MACSec Transformer instance
*
*   NOTEs: Applies for ingress only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] saHandle             - SA handle of the SA to be updated.
* @param[in] nextPnLo             - Least significant 32 bits of new NextPN.
*                                   For 32-bit sequence numbers it is the entire sequence number.
* @param[in] nextPnHi             - Most significant 32 bits of new NextPN in case of 64-bit sequnce numbers
*                                   Must be cleared to zero for 32-bit sequence numbers
* @param[out] nextPnWrittenPtr    - (pointer to) flag to indicate that NextPN was actually written.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaNextPnUpdate
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  GT_U32                            nextPnLo,
    IN  GT_U32                            nextPnHi,
    OUT GT_BOOL                           *nextPnWrittenPtr
)
{
    GT_STATUS         rc;
    GT_U32            macSecUnitId;
    SecY_SAHandle_t   secySaHandle;
    GT_BOOL           nextPnWritten;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(nextPnWrittenPtr);

    /* Get MACSec unit ID. Check unitBmp is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Call internal Transformer API to update nextPN */
    rc = SecY_SA_NextPN_Update(macSecUnitId,
                               secySaHandle,
                               nextPnLo,
                               nextPnHi,
                               &nextPnWritten);

    /* Update output parameter of *nextPnWrittenPtr */
    *nextPnWrittenPtr = nextPnWritten;

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaNextPnUpdate function
* @endinternal
*
* @brief   Updates sequence number of nextPN for a MACSec Transformer instance
*
*   NOTEs: Applies for ingress only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] saHandle             - SA handle of the SA to be updated.
* @param[in] nextPnLo             - Least significant 32 bits of new NextPN.
*                                   For 32-bit sequence numbers it is the entire sequence number.
* @param[in] nextPnHi             - Most significant 32 bits of new NextPN in case of 64-bit sequnce numbers
*                                   Must be cleared to zero for 32-bit sequence numbers
* @param[out] nextPnWrittenPtr    - (pointer to) flag to indicate that NextPN was actually written.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaNextPnUpdate
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  GT_U32                            nextPnLo,
    IN  GT_U32                            nextPnHi,
    OUT GT_BOOL                           *nextPnWrittenPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaNextPnUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, saHandle, nextPnLo, nextPnHi, nextPnWrittenPtr));

    rc = internal_cpssDxChMacSecSecySaNextPnUpdate(devNum, unitBmp, saHandle, nextPnLo, nextPnHi, nextPnWrittenPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, saHandle, nextPnLo, nextPnHi, nextPnWrittenPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaNextPnGet function
* @endinternal
*
* @brief   Reads the nextPN field for a specific SA
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  saHandle            - SA handle of the SA to be updated.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[out] nextPnLoPtr         - (pointer to) least significant 32 bits of nextPN.
*                                   For 32-bit sequence numbers it is the entire sequence number.
* @param[out] nextPnHiPtr         - (pointer to) most significant 32 bits of nextPN in case of 64-bit sequence numbers
*                                   Will be cleared to zero for 32-bit sequence numbers
* @param[out] extendedPnPtr       - (pointer to) flag to indicate that sequence number is 64-bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaNextPnGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    OUT GT_U32                            *nextPnLoPtr,
    OUT GT_U32                            *nextPnHiPtr,
    OUT GT_BOOL                           *extendedPnPtr
)
{
    GT_STATUS         rc;
    GT_U32            macSecUnitId;
    SecY_SAHandle_t   secySaHandle;
    GT_U32            nextPnLo = 0;
    GT_U32            nextPnHi = 0;
    GT_BOOL           extendedPn;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointers are not NULL */
    CPSS_NULL_PTR_CHECK_MAC(nextPnLoPtr);
    CPSS_NULL_PTR_CHECK_MAC(nextPnHiPtr);
    CPSS_NULL_PTR_CHECK_MAC(extendedPnPtr);

    /* Get MACSec unit ID. Check unitBmp is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Call internal Transformer API to read sequence number from transform record data */
     rc = SecY_SA_NextPN_Get(macSecUnitId,
                             secySaHandle,
                             &nextPnLo,
                             &nextPnHi,
                             &extendedPn);

    /* Update output parameters */
    *extendedPnPtr = extendedPn;
    *nextPnLoPtr = nextPnLo;
    if (extendedPn)
    {
        *nextPnHiPtr = nextPnHi;
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaNextPnGet function
* @endinternal
*
* @brief   Reads the nextPN field for a specific SA
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  saHandle            - SA handle of the SA to be updated.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[out] nextPnLoPtr         - (pointer to) least significant 32 bits of nextPN.
*                                   For 32-bit sequence numbers it is the entire sequence number.
* @param[out] nextPnHiPtr         - (pointer to) most significant 32 bits of nextPN in case of 64-bit sequence numbers
*                                   Will be cleared to zero for 32-bit sequence numbers
* @param[out] extendedPnPtr       - (pointer to) flag to indicate that sequence number is 64-bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaNextPnGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    OUT GT_U32                            *nextPnLoPtr,
    OUT GT_U32                            *nextPnHiPtr,
    OUT GT_BOOL                           *extendedPnPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaNextPnGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saHandle, nextPnLoPtr, nextPnHiPtr, extendedPnPtr));

    rc = internal_cpssDxChMacSecSecySaNextPnGet(devNum, unitBmp, direction, saHandle, nextPnLoPtr, nextPnHiPtr, extendedPnPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saHandle, nextPnLoPtr, nextPnHiPtr, extendedPnPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleSet function
* @endinternal
*
* @brief   Updates an existing packet matching rule for one MACSec classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for the vPort where the packet matching rule must be added.
* @param[in]  ruleHandle          - Rule handle for rule to be updated.
* @param[in]  ruleParamsPtr       - (pointer to) data for the rule.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr
)
{
    GT_STATUS             rc;
    GT_U32                macSecUnitId;
    CfyE_Rule_t           ruleParams;
    GT_U32                channelNum;
    GT_U32                dpId;
    GT_U8                 byteArr[4];
    GT_U32                i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ruleParamsPtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));

    /* Update rule structure with rule parameters */
    ruleParams.Policy.fControlPacket = ruleParamsPtr->policy.controlPacket;
    ruleParams.Policy.fDrop          = ruleParamsPtr->policy.drop;
    ruleParams.Policy.Priority       = ruleParamsPtr->policy.rulePriority;
    ruleParams.Policy.vPortHandle    = (CfyE_vPortHandle_t)vPortHandle;

    ruleParams.Mask.NumTags          = ruleParamsPtr->mask.numTags;
    ruleParams.Key.NumTags           = ruleParamsPtr->key.numTags;

    ruleParams.Mask.PacketType       = ruleParamsPtr->mask.packetType;
    ruleParams.Key.PacketType        = ruleParamsPtr->key.packetType;

    ruleParams.Mask.ChannelID  = ruleParamsPtr->mask.portNum;

    /* Check mask of portNum. In case it is not zero need to convert physical port number into channel number */
    if(ruleParamsPtr->mask.portNum !=0 )
    {
        /* Check portNum is valid and get local channel */
        rc = prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert(devNum, ruleParamsPtr->key.portNum, &dpId, &channelNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set channel ID */
        ruleParams.Key.ChannelID  = channelNum;
    }

    /* Need to arrange data in little endian since TCAM packet data is stored in little endian byte order */
    for(i=0;i<CFYE_RULE_NON_CTRL_WORD_COUNT;i++)
    {
        /* Extract each byte and place it in little endian byte order */
        byteArr[0] = (GT_U8)( (ruleParamsPtr->data[i]>>24) & 0xFF );
        byteArr[1] = (GT_U8)( (ruleParamsPtr->data[i]>>16) & 0xFF );
        byteArr[2] = (GT_U8)( (ruleParamsPtr->data[i]>>8)  & 0xFF );
        byteArr[3] = (GT_U8)( (ruleParamsPtr->data[i])     & 0xFF );
        ruleParams.Data[i] = (byteArr[0] | byteArr[1]<<8 | byteArr[2]<<16 | byteArr[3]<<24) ;

        /* Now do it for the data mask */
        byteArr[0] = (GT_U8)( (ruleParamsPtr->dataMask[i]>>24) & 0xFF );
        byteArr[1] = (GT_U8)( (ruleParamsPtr->dataMask[i]>>16) & 0xFF );
        byteArr[2] = (GT_U8)( (ruleParamsPtr->dataMask[i]>>8)  & 0xFF );
        byteArr[3] = (GT_U8)( (ruleParamsPtr->dataMask[i])     & 0xFF );
        ruleParams.DataMask[i] = (byteArr[0] | byteArr[1]<<8 | byteArr[2]<<16 | byteArr[3]<<24) ;
    }

    /* Call Classifier API to update rule */
    rc = CfyE_Rule_Update(macSecUnitId,(CfyE_RuleHandle_t)ruleHandle, &ruleParams);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleSet function
* @endinternal
*
* @brief   Updates an existing packet matching rule for one MACSec classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for the vPort where the packet matching rule must be added.
* @param[in]  ruleHandle          - Rule handle for rule to be updated.
* @param[in]  ruleParamsPtr       - (pointer to) data for the rule.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortHandle, ruleHandle, ruleParamsPtr));

    rc = internal_cpssDxChMacSecClassifyRuleSet(devNum, unitBmp, direction, vPortHandle, ruleHandle, ruleParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortHandle, ruleHandle, ruleParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleGet function
* @endinternal
*
* @brief   Read an existing packet matching rule from one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  ruleHandle          - rule handle for rule to be read.
* @param[out] vPortHandlePtr      - (pointer to) vPort handle that is associated with current rule.
* @param[out] ruleDataPtr         - (pointer to) data for the rule.
* @param[out] ruleEnablePtr       - (pointer to) flag indicating whether the rule was enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleDataPtr,
    OUT  GT_BOOL                                 *ruleEnablePtr
)
{
    GT_STATUS              rc;
    GT_U32                 macSecUnitId;
    CfyE_Rule_t            ruleParams;
    GT_BOOL                ruleEnable;
    GT_U32                 channelNum;
    GT_PHYSICAL_PORT_NUM   physicalPortNum;
    GT_U32                 dpId;
    GT_U8                  byteArr[4];
    GT_U32                 i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointers are not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ruleDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(vPortHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structure before filling it  */
    cpssOsMemSet(&ruleParams, 0, sizeof(ruleParams));

    /* Call Classifier API to get rule settings */
    rc = CfyE_Rule_Read(macSecUnitId,(CfyE_RuleHandle_t)ruleHandle, &ruleParams, &ruleEnable);
    if (rc != GT_OK)
    {
        return rc;
    }

    /**** Update if rule is enabled or disabled ****/
    *ruleEnablePtr = ruleEnable;


    /***** Update rule structure with rule parameters *****/

    /* Get rule policy */
    ruleDataPtr->policy.controlPacket = ruleParams.Policy.fControlPacket;
    ruleDataPtr->policy.drop          = ruleParams.Policy.fDrop;
    ruleDataPtr->policy.rulePriority  = ruleParams.Policy.Priority;
    /* Get the associated vPort handle */
    *vPortHandlePtr = (CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE)ruleParams.Policy.vPortHandle;

    /* Get number of tags */
    ruleDataPtr->mask.numTags = ruleParams.Mask.NumTags;
    ruleDataPtr->key.numTags  = ruleParams.Key.NumTags;

    ruleDataPtr->mask.packetType = ruleParams.Mask.PacketType;
    ruleDataPtr->key.packetType  = ruleParams.Key.PacketType;

    /* Get port number. Will get real value if port mask is not zero otherwise clear to zero */
    ruleDataPtr->key.portNum = 0;
    ruleDataPtr->mask.portNum = ruleParams.Mask.ChannelID;

    /* Check mask of portNum. In case it is not zero need to convert channel number into physical port number */
    if(ruleDataPtr->mask.portNum !=0 )
    {
        /* Get local channel number */
        channelNum = ruleParams.Key.ChannelID;
        /* Get DP ID (0 or 1) */
        dpId = (unitBmp>>1) & 0x1;

        /* Get physical port number */
        rc = prvCpssDxChPortPhysicalPortGet(devNum, dpId, channelNum, &physicalPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set output with port number */
        ruleDataPtr->key.portNum = physicalPortNum;
    }


    /* Since TCAM packet data is stored in little endian byte order need to arrange it in networking order
       From 44 33 22 11  to 11 22 33 44
                  66 55     55 66        */
    for(i=0;i<CFYE_RULE_NON_CTRL_WORD_COUNT;i++)
    {
        /* Extract each byte and place it in little endian byte order */
        byteArr[0] = (GT_U8)( (ruleParams.Data[i]>>24) & 0xFF );
        byteArr[1] = (GT_U8)( (ruleParams.Data[i]>>16) & 0xFF );
        byteArr[2] = (GT_U8)( (ruleParams.Data[i]>>8)  & 0xFF );
        byteArr[3] = (GT_U8)( (ruleParams.Data[i])     & 0xFF );
        ruleDataPtr->data[i] = (byteArr[0] | byteArr[1]<<8 | byteArr[2]<<16 | byteArr[3]<<24) ;

        /* Now do it for the data mask */
        byteArr[0] = (GT_U8)( (ruleParams.DataMask[i]>>24) & 0xFF );
        byteArr[1] = (GT_U8)( (ruleParams.DataMask[i]>>16) & 0xFF );
        byteArr[2] = (GT_U8)( (ruleParams.DataMask[i]>>8)  & 0xFF );
        byteArr[3] = (GT_U8)( (ruleParams.DataMask[i])     & 0xFF );
        ruleDataPtr->dataMask[i] = (byteArr[0] | byteArr[1]<<8 | byteArr[2]<<16 | byteArr[3]<<24) ;
    }

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleGet function
* @endinternal
*
* @brief   Read an existing packet matching rule from one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  ruleHandle          - rule handle for rule to be read.
* @param[out] vPortHandlePtr      - (pointer to) vPort handle that is associated with current rule.
* @param[out] ruleDataPtr         - (pointer to) data for the rule.
* @param[out] ruleEnablePtr       - (pointer to) flag indicating whether the rule was enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleDataPtr,
    OUT  GT_BOOL                                 *ruleEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleHandle, vPortHandlePtr, ruleDataPtr, ruleEnablePtr));

    rc = internal_cpssDxChMacSecClassifyRuleGet(devNum, unitBmp, direction, ruleHandle, vPortHandlePtr, ruleDataPtr, ruleEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleHandle, vPortHandlePtr, ruleDataPtr, ruleEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleHandleGet function
* @endinternal
*
* @brief   Get the Rule handle from an existing Rule, using the Rule index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleIndex            - rule index for which the Rule handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..511)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[out] ruleHandlePtr       - (pointer to) rule handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  ruleIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
)
{
    GT_STATUS             rc;
    GT_U32                macSecUnitId;
    CfyE_RuleHandle_t     cfyeRuleHandle = CfyE_RuleHandle_NULL;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                           CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(ruleHandlePtr);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check ruleIndex is valid range */
    if (ruleIndex >= PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[macSecUnitId]->RulesCount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ruleIndex(%d) is bigger than maximum allowed number of rules(%d)"
                                      ,ruleIndex,PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[macSecUnitId]->RulesCount);
    }

    /* Call Classifier API to get rule handle that is associated to this rule index */
    rc = CfyE_RuleHandle_Get(macSecUnitId, ruleIndex, &cfyeRuleHandle);

    /* Get Classifier rule handle */
    *ruleHandlePtr = (CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE)cfyeRuleHandle;

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleHandleGet function
* @endinternal
*
* @brief   Get the Rule handle from an existing Rule, using the Rule index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleIndex            - rule index for which the Rule handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..511)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[out] ruleHandlePtr       - (pointer to) rule handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  ruleIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleHandleGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleIndex, ruleHandlePtr));

    rc = internal_cpssDxChMacSecClassifyRuleHandleGet(devNum, unitBmp, direction, ruleIndex, ruleHandlePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleIndex, ruleHandlePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleEnableDisable function
* @endinternal
*
* @brief   This API offers a combined functionality to be performed with a single write action on an existing
*          rules in one MACSec Classifier device
*          - Enabling or disabling of an entry
*          - Enabling one and disabling another entry at the same time
*          - Enable or disable all entries.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandleEnable     - rule handle for rule to be enabled. Will be ignored if equal to 0.
* @param[in] ruleHandleDisable    - rule handle for rule to be disabled. Will be ignored if equal to 0.
* @param[in] enableAll            - When set to true all rules will be enabled. Takes precedence over the other parameters.
* @param[in] disableAll           - When set to true all rules will be disabled. Takes precedence over the other parameters, except enableAll.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleEnableDisable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleEnable,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleDisable,
    IN  GT_BOOL                                 enableAll,
    IN  GT_BOOL                                 disableAll
)
{
    GT_STATUS           rc;
    GT_U32              macSecUnitId;
    GT_BOOL             sync;
    CfyE_RuleHandle_t   ruleHandleForEnable = CfyE_RuleHandle_NULL;
    CfyE_RuleHandle_t   ruleHandleForDisable = CfyE_RuleHandle_NULL;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp and direction parameters are valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Option to track the number of packets in the pipeline for hardware/software synchronization purposes
       Do not synchronize with MACSec unit */
    sync = GT_FALSE;

    /* Rule handle for rule to be enabled */
    if (ruleHandleEnable)
    {
        ruleHandleForEnable = (CfyE_RuleHandle_t)ruleHandleEnable;
    }

    /* Rule handle for rule to be disabled */
    if (ruleHandleDisable)
    {
        ruleHandleForDisable = (CfyE_RuleHandle_t)ruleHandleDisable;
    }

    /* Call Classifier API with the functionality to enable or/and disable specific rules.
       Or to enable or disable all rules */
    rc = CfyE_Rule_EnableDisable(macSecUnitId,
                                 ruleHandleForEnable,
                                 ruleHandleForDisable,
                                 enableAll,
                                 disableAll,
                                 sync);
    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleEnableDisable function
* @endinternal
*
* @brief   This API offers a combined functionality to be performed with a single write action on an existing
*          rules in one MACSec Classifier device
*          - Enabling or disabling of an entry
*          - Enabling one and disabling another entry at the same time
*          - Enable or disable all entries.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandleEnable     - rule handle for rule to be enabled. Will be ignored if equal to 0.
* @param[in] ruleHandleDisable    - rule handle for rule to be disabled. Will be ignored if equal to 0.
* @param[in] enableAll            - When set to true all rules will be enabled. Takes precedence over the other parameters.
* @param[in] disableAll           - When set to true all rules will be disabled. Takes precedence over the other parameters, except enableAll.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleEnableDisable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleEnable,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleDisable,
    IN  GT_BOOL                                 enableAll,
    IN  GT_BOOL                                 disableAll
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleEnableDisable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleHandleEnable, ruleHandleDisable, enableAll, disableAll));

    rc = internal_cpssDxChMacSecClassifyRuleEnableDisable(devNum, unitBmp, direction, ruleHandleEnable, ruleHandleDisable, enableAll, disableAll);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleHandleEnable, ruleHandleDisable, enableAll, disableAll));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyLimitsGet function
* @endinternal
*
* @brief   Returns maximum number of ports,vPorts and TCAM rules for MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] maxPortNumPtr       - (pointer to) maximum number of ports. Skipped if the pointer is NULL.
* @param[out] maxVportNumPtr      - (pointer to) maximum number of vPorts. Skipped if the pointer is NULL.
* @param[out] maxRuleNumPtr       - (pointer to) maximum number of rules. Skipped if the pointer is NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyLimitsGet
(
    IN  GT_U8                 devNum,
    IN  GT_MACSEC_UNIT_BMP    unitBmp,
    OUT GT_U32                *maxPortNumPtr,
    OUT GT_U32                *maxVportNumPtr,
    OUT GT_U32                *maxRuleNumPtr
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Since all Classifier MACSec units have the same number of ports, vPorts and rules
       we will pick Classifier Egress unit to get that information */

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get maximum number of ports, vPorts and rules allowed in the device */
    rc = CfyE_Device_Limits(macSecUnitId, maxPortNumPtr, maxVportNumPtr, maxRuleNumPtr);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyLimitsGet function
* @endinternal
*
* @brief   Returns maximum number of ports,vPorts and TCAM rules for MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] maxPortNumPtr       - (pointer to) maximum number of ports. Skipped if the pointer is NULL.
* @param[out] maxVportNumPtr      - (pointer to) maximum number of vPorts. Skipped if the pointer is NULL.
* @param[out] maxRuleNumPtr       - (pointer to) maximum number of rules. Skipped if the pointer is NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyLimitsGet
(
    IN  GT_U8                 devNum,
    IN  GT_MACSEC_UNIT_BMP    unitBmp,
    OUT GT_U32                *maxPortNumPtr,
    OUT GT_U32                *maxVportNumPtr,
    OUT GT_U32                *maxRuleNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyLimitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, maxPortNumPtr, maxVportNumPtr, maxRuleNumPtr));

    rc = internal_cpssDxChMacSecClassifyLimitsGet(devNum, unitBmp, maxPortNumPtr, maxVportNumPtr, maxRuleNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, maxPortNumPtr, maxVportNumPtr, maxRuleNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyLimitsGet function
* @endinternal
*
* @brief   Returns the maximum number of ports, vPorts, SA and SC for MACSec transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] maxPortNumPtr       - (pointer to) maximum number of ports. Skipped if the pointer is NULL.
* @param[out] maxVportNumPtr      - (pointer to) maximum number of vPorts. Skipped if the pointer is NULL.
* @param[out] maxSaNumPtr         - (pointer to) maximum number of SAs. Skipped if the pointer is NULL.
* @param[out] maxScNumPtr         - (pointer to) maximum number of SCs. Skipped if the pointer is NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_MACSEC_UNIT_BMP      unitBmp,
    OUT GT_U32                  *maxPortNumPtr,
    OUT GT_U32                  *maxVportNumPtr,
    OUT GT_U32                  *maxSaNumPtr,
    OUT GT_U32                  *maxScNumPtr
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Since all Transformer MACSec units have the same number of ports, vPorts SAs and SCs
       we will pick Transformer Egress unit to get that information */

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get maximum number of ports, vPorts, SAs and SCs allowed in the device */
    rc = SecY_Device_Limits(macSecUnitId, maxPortNumPtr, maxVportNumPtr, maxSaNumPtr, maxScNumPtr);

    return rc;
}

/**
* @internal cpssDxChMacSecSecyLimitsGet function
* @endinternal
*
* @brief   Returns the maximum number of ports, vPorts, SA and SC for MACSec transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] maxPortNumPtr       - (pointer to) maximum number of ports. Skipped if the pointer is NULL.
* @param[out] maxVportNumPtr      - (pointer to) maximum number of vPorts. Skipped if the pointer is NULL.
* @param[out] maxSaNumPtr         - (pointer to) maximum number of SAs. Skipped if the pointer is NULL.
* @param[out] maxScNumPtr         - (pointer to) maximum number of SCs. Skipped if the pointer is NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecyLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_MACSEC_UNIT_BMP      unitBmp,
    OUT GT_U32                  *maxPortNumPtr,
    OUT GT_U32                  *maxVportNumPtr,
    OUT GT_U32                  *maxSaNumPtr,
    OUT GT_U32                  *maxScNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyLimitsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, maxPortNumPtr, maxVportNumPtr, maxSaNumPtr ,maxScNumPtr));

    rc = internal_cpssDxChMacSecSecyLimitsGet(devNum, unitBmp, maxPortNumPtr, maxVportNumPtr, maxSaNumPtr ,maxScNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, maxPortNumPtr, maxVportNumPtr, maxSaNumPtr ,maxScNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyGlobalCfgDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*           - Configuration information
*           - ECC setting and status (ECCCorrectable & ECCUncorrectable counters)
*           - Global packets in-flight status.
*           - Various debug header parser state registers.
*           - Statistics summary status registers.
*           - number of vPorts, SCs, SAs and ports
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of all global device configuration and status */
    rc = SecY_Diag_Device_Dump(macSecUnitId);

    return rc;
}

/**
* @internal cpssDxChMacSecSecyGlobalCfgDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*           - Configuration information
*           - ECC setting and status (ECCCorrectable & ECCUncorrectable counters)
*           - Global packets in-flight status.
*           - Various debug header parser state registers.
*           - Statistics summary status registers.
*           - number of vPorts, SCs, SAs and ports
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyGlobalCfgDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction));

    rc = internal_cpssDxChMacSecSecyGlobalCfgDump(devNum, unitBmp, direction);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortSecyDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - Port configuration
*          - Packet in-flight status of this port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] portNum              - Port physical number for which diagnostics information is desired
* @param[in] allPorts             - Dump the information of all ports instead of the one selected with portNum
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortSecyDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_BOOL                           allPorts
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;
    GT_U32          macSecChannelNum = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (allPorts)
    {   /* In case dump is required for all ports */

        /* Get MACSec unit ID. Check unitBmp parameter is valid */
        rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                              unitBmp,
                                              direction,
                                              CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                              &macSecUnitId);
    }
    else
    {   /* In case dump is required for one specific port */

        /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
        rc = prvCpssMacSecPortToUnitId(devNum,
                                       portNum,
                                       direction,
                                       CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                       &macSecChannelNum,
                                       &macSecUnitId);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of all global device configuration and status */
    rc = SecY_Diag_Channel_Dump(macSecUnitId, macSecChannelNum, allPorts);

    return rc;
}

/**
* @internal cpssDxChMacSecPortSecyDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - Port configuration
*          - Packet in-flight status of this port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] portNum              - Port physical number for which diagnostics information is desired
* @param[in] allPorts             - Dump the information of all ports instead of the one selected with portNum
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecPortSecyDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_BOOL                           allPorts
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortSecyDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, portNum, allPorts));

    rc = internal_cpssDxChMacSecPortSecyDump(devNum, unitBmp, direction, portNum, allPorts);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, portNum, allPorts));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyVportDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - Per-vPort. Parameters related to the SA which are not in the SA record,
*          - For ingress configuration: all RxCAM entries associated with the vPort and up to four SA indexes
*            associated with each SCI. RxCAM statistics for the entries involved.
*          - For egress configuration: the SA the index of the egress SA plus the index of any chained SA .
*          - Optionally all SA-related information for each of the SAs associated with the vPort.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] vPortId              - vPort identifier for which diagnostics information is desired
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] allvPorts            - Dump the information of all vPorts instead of the one selected with vPortId
* @param[in] includeSa            - Dump the information for all SAs associated with this particular vPort
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyVportDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPortId,
    IN   GT_BOOL                          allvPorts,
    IN   GT_BOOL                          includeSa
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of all vPort configuration */
    rc = SecY_Diag_vPort_Dump(macSecUnitId, vPortId, allvPorts, includeSa);

    return rc;
}

/**
* @internal cpssDxChMacSecSecyVportDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - Per-vPort. Parameters related to the SA which are not in the SA record,
*          - For ingress configuration: all RxCAM entries associated with the vPort and up to four SA indexes
*            associated with each SCI. RxCAM statistics for the entries involved.
*          - For egress configuration: the SA the index of the egress SA plus the index of any chained SA .
*          - Optionally all SA-related information for each of the SAs associated with the vPort.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] vPortId              - vPort identifier for which diagnostics information is desired
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] allvPorts            - Dump the information of all vPorts instead of the one selected with vPortId
* @param[in] includeSa            - Dump the information for all SAs associated with this particular vPort
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyVportDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPortId,
    IN   GT_BOOL                          allvPorts,
    IN   GT_BOOL                          includeSa
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyVportDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortId, allvPorts, includeSa));

    rc = internal_cpssDxChMacSecSecyVportDump(devNum, unitBmp, direction, vPortId, allvPorts, includeSa);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortId, allvPorts, includeSa));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecySaDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - The contents of the SA record.
*          - SA statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - SA handle for which diagnostics information is desired
* @param[in] allSas               - Dump the information of all SAs instead of the one selected with saHandle.
*                                   saHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecySaDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN   GT_BOOL                           allSas
)
{
    GT_STATUS        rc;
    GT_U32           macSecUnitId;
    SecY_SAHandle_t  secySaHandle;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Update SA handle */
    secySaHandle.p = (void *)saHandle;

    /* Provide a diagnostics dump of all SA configuration */
    rc = SecY_Diag_SA_Dump(macSecUnitId, secySaHandle, allSas);

    return rc;
}

/**
* @internal cpssDxChMacSecSecySaDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - The contents of the SA record.
*          - SA statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - SA handle for which diagnostics information is desired
* @param[in] allSas               - Dump the information of all SAs instead of the one selected with saHandle.
*                                   saHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN   GT_BOOL                           allSas
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecySaDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, saHandle, allSas));

    rc = internal_cpssDxChMacSecSecySaDump(devNum, unitBmp, direction, saHandle, allSas);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, saHandle, allSas));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyGlobalCfgDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - Configuration information (control packet detection, SecTag parser,vlan parser and statistics).
*          - ECC setting and status (ECCCorrectable & ECCUncorrectable counters)
*          - Various header parser state registers.
*          - Summary status registers.
*          - Number of vPorts, Rules and ports.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
)
{
    GT_STATUS        rc;
    GT_U32           macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of all unit global configuration */
    rc = CfyE_Diag_Device_Dump(macSecUnitId);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyGlobalCfgDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - Configuration information (control packet detection, SecTag parser,vlan parser and statistics).
*          - ECC setting and status (ECCCorrectable & ECCUncorrectable counters)
*          - Various header parser state registers.
*          - Summary status registers.
*          - Number of vPorts, Rules and ports.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyGlobalCfgDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction));

    rc = internal_cpssDxChMacSecClassifyGlobalCfgDump(devNum, unitBmp, direction);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecPortClassifyDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - All information that can be obtained per port.
*          - Per port statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] portNum              - Port physical number for which diagnostics information is desired
* @param[in] allPorts             - Dump the information of all channels instead of the one selected with portNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecPortClassifyDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_BOOL                          allPorts
)
{
    GT_STATUS       rc;
    GT_U32          macSecUnitId;
    GT_U32          macSecChannelNum = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    if (allPorts)
    {   /* In case dump is required for all ports */

        /* Get MACSec unit ID. Check unitBmp parameter is valid */
        rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                              unitBmp,
                                              direction,
                                              CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                              &macSecUnitId);
    }
    else
    {   /* In case dump is required for one specific port */

        /* Get MACSec unit ID and MACSec channel number. Check port number and direction parameters are valid */
        rc = prvCpssMacSecPortToUnitId(devNum,
                                       portNum,
                                       direction,
                                       CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                       &macSecChannelNum,
                                       &macSecUnitId);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of all global device configuration and status */
    rc = CfyE_Diag_Channel_Dump(macSecUnitId, macSecChannelNum, allPorts);

    return rc;
}

/**
* @internal cpssDxChMacSecPortClassifyDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - All information that can be obtained per port.
*          - Per port statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] portNum              - Port physical number for which diagnostics information is desired
* @param[in] allPorts             - Dump the information of all channels instead of the one selected with portNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_BOOL                          allPorts
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecPortClassifyDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, portNum, allPorts));

    rc = internal_cpssDxChMacSecPortClassifyDump(devNum, unitBmp, direction, portNum, allPorts);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, portNum, allPorts));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyVportDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - vPort policy.
*          - List of rules associated with this vPort.
*          - Optionally all information releated to those rules.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - vPort handle for which diagnostics information is desired
* @param[in] allvPorts            - Dump the information of all vPorts instead of the one selected with vPortHandle
* @param[in] includeRule          - Dump the information for all Rules associated with this particular vPort.
*                                   vPortHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyVportDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE   vPortHandle,
    IN   GT_BOOL                                  allvPorts,
    IN   GT_BOOL                                  includeRule
)
{
    GT_STATUS        rc;
    GT_U32           macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of vPort configuration */
    rc = CfyE_Diag_vPort_Dump(macSecUnitId, (CfyE_vPortHandle_t)vPortHandle, allvPorts, includeRule);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyVportDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - vPort policy.
*          - List of rules associated with this vPort.
*          - Optionally all information releated to those rules.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - vPort handle for which diagnostics information is desired
* @param[in] allvPorts            - Dump the information of all vPorts instead of the one selected with vPortHandle
* @param[in] includeRule          - Dump the information for all Rules associated with this particular vPort.
*                                   vPortHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE   vPortHandle,
    IN   GT_BOOL                                  allvPorts,
    IN   GT_BOOL                                  includeRule
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyVportDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, vPortHandle, allvPorts, includeRule));

    rc = internal_cpssDxChMacSecClassifyVportDump(devNum, unitBmp, direction, vPortHandle, allvPorts, includeRule);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, vPortHandle, allvPorts, includeRule));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyRuleDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - The content of the rule (TCAM contents and vPort policy).
*          - flag to indicate whether rule is enabled.
*          - TCAM statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - Rule handle for which diagnostics information is desired
* @param[in] allRules             - Dump the information of all rules instead of the one selected with ruleHandle
*                                   ruleHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyRuleDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE    ruleHandle,
    IN   GT_BOOL                                  allRules
)
{
    GT_STATUS        rc;
    GT_U32           macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Provide a diagnostics dump of rule configuration */
    rc = CfyE_Diag_Rule_Dump(macSecUnitId, (CfyE_RuleHandle_t)ruleHandle, allRules);

    return rc;
}

/**
* @internal cpssDxChMacSecClassifyRuleDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - The content of the rule (TCAM contents and vPort policy).
*          - flag to indicate whether rule is enabled.
*          - TCAM statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - Rule handle for which diagnostics information is desired
* @param[in] allRules             - Dump the information of all rules instead of the one selected with ruleHandle
*                                   ruleHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE    ruleHandle,
    IN   GT_BOOL                                  allRules
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyRuleDump);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, ruleHandle, allRules));

    rc = internal_cpssDxChMacSecClassifyRuleDump(devNum, unitBmp, direction, ruleHandle, allRules);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, ruleHandle, allRules));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyStatusGet function
* @endinternal
*
* @brief   Reads status information from Classifier unit
*          - Get ECC status
*          - Get parser debug
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  getEccStatus        - Set to GT_TRUE to get ECC status information otherwise set to GT_FALSE
* @param[in]  getPktProcessDebug  - Set to GT_TRUE to get packet processing debug information otherwise set to GT_FALSE
* @param[out] deviceStatusPtr     - (pointer to) unit's status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecClassifyStatusGet
(
    IN   GT_U8                                       devNum,
    IN   GT_MACSEC_UNIT_BMP                          unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT              direction,
    IN   GT_BOOL                                     getEccStatus,
    IN   GT_BOOL                                     getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC   *unitStatusPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;
    CfyE_DeviceStatus_t      unitStatus;
    CfyE_ECCStatus_t         eccStatus;
    CfyE_PktProcessDebug_t   pktProcessDebug;
    GT_U32                   i;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(unitStatusPtr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them */
    cpssOsMemSet(&unitStatus, 0, sizeof(unitStatus));
    cpssOsMemSet(&eccStatus, 0, sizeof(eccStatus));
    cpssOsMemSet(&pktProcessDebug, 0, sizeof(pktProcessDebug));

    /* Set unit status pointers */
    if (getEccStatus)
    {
        unitStatus.ECCStatus_p = &eccStatus;
    }

    if (getPktProcessDebug)
    {
        unitStatus.PktProcessDebug_p = &pktProcessDebug;
    }

    /* Provide a diagnostics dump of rule configuration */
    rc = CfyE_Device_Status_Get(macSecUnitId, &unitStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get ECC status counters if it is desired */
    if (getEccStatus)
    {
        for (i=0;i<CPSS_DXCH_MACSEC_CLASSIFY_ECC_NOF_STATUS_COUNTERS_CNS;i++)
        {
            unitStatusPtr->eccCountersInfo[i].correctableCount   = unitStatus.ECCStatus_p->Counters[i].CorrectableCount;
            unitStatusPtr->eccCountersInfo[i].correctableThr     = unitStatus.ECCStatus_p->Counters[i].fCorrectableThr;
            unitStatusPtr->eccCountersInfo[i].uncorrectableCount = unitStatus.ECCStatus_p->Counters[i].UncorrectableCount;
            unitStatusPtr->eccCountersInfo[i].uncorrectableThr   = unitStatus.ECCStatus_p->Counters[i].fUncorrectableThr;
        }
    }

    /* Get packet processing debug information if it is desired */
    if(getPktProcessDebug)
    {
        unitStatusPtr->pktProcessDebug.cpMatchDebug    = unitStatus.PktProcessDebug_p->CPMatchDebug;
        unitStatusPtr->pktProcessDebug.debugFlowLookup = unitStatus.PktProcessDebug_p->DebugFlowLookup;
        unitStatusPtr->pktProcessDebug.parsedDaHi      = unitStatus.PktProcessDebug_p->ParsedDAHi;
        unitStatusPtr->pktProcessDebug.parsedDaLo      = unitStatus.PktProcessDebug_p->ParsedDALo;
        unitStatusPtr->pktProcessDebug.parsedSaHi      = unitStatus.PktProcessDebug_p->ParsedSAHi;
        unitStatusPtr->pktProcessDebug.parsedSaLo      = unitStatus.PktProcessDebug_p->ParsedSALo;
        unitStatusPtr->pktProcessDebug.parsedSecTagHi  = unitStatus.PktProcessDebug_p->ParsedSecTAGHi;
        unitStatusPtr->pktProcessDebug.parsedSecTagLo  = unitStatus.PktProcessDebug_p->ParsedSecTAGLo;
        unitStatusPtr->pktProcessDebug.secTagDebug     = unitStatus.PktProcessDebug_p->SecTAGDebug;
        unitStatusPtr->pktProcessDebug.tcamDebug       = unitStatus.PktProcessDebug_p->TCAMDebug;
    }

    return rc;
}

/**
* @internal internal_cpssDxChMacSecClassifyStatusGet function
* @endinternal
*
* @brief   Reads status information from Classifier unit
*          - Get ECC status
*          - Get parser debug
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  getEccStatus        - Set to GT_TRUE to get ECC status information otherwise set to GT_FALSE
* @param[in]  getPktProcessDebug  - Set to GT_TRUE to get packet processing debug information otherwise set to GT_FALSE
* @param[out] deviceStatusPtr     - (pointer to) unit's status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatusGet
(
    IN   GT_U8                                       devNum,
    IN   GT_MACSEC_UNIT_BMP                          unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT              direction,
    IN   GT_BOOL                                     getEccStatus,
    IN   GT_BOOL                                     getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC   *unitStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecClassifyStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, getEccStatus, getPktProcessDebug, unitStatusPtr));

    rc = internal_cpssDxChMacSecClassifyStatusGet(devNum, unitBmp, direction, getEccStatus, getPktProcessDebug, unitStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, getEccStatus, getPktProcessDebug, unitStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecyStatusGet function
* @endinternal
*
* @brief   Reads status information from Transformer unit
*          - Get ECC status
*          - Get parser debug
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  getEccStatus        - Set to GT_TRUE to get ECC status information otherwise set to GT_FALSE
* @param[in]  getPktProcessDebug  - Set to GT_TRUE to get packet processing debug information otherwise set to GT_FALSE
* @param[out] deviceStatusPtr     - (pointer to) unit's status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecyStatusGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   GT_BOOL                                 getEccStatus,
    IN   GT_BOOL                                 getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC   *unitStatusPtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;
    SecY_DeviceStatus_t      unitStatus;
    SecY_ECCStatus_t         eccStatus;
    SecY_PktProcessDebug_t   pktProcessDebug;
    GT_U32                   i;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(unitStatusPtr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Clear structures before filling them */
    cpssOsMemSet(&unitStatus, 0, sizeof(unitStatus));
    cpssOsMemSet(&eccStatus, 0, sizeof(eccStatus));
    cpssOsMemSet(&pktProcessDebug, 0, sizeof(pktProcessDebug));

    /* Set unit status pointers */
    if (getEccStatus)
    {
        unitStatus.ECCStatus_p = &eccStatus;
    }

    if (getPktProcessDebug)
    {
        unitStatus.PktProcessDebug_p = &pktProcessDebug;
    }

    /* Get Transformer unit status */
    rc = SecY_Device_Status_Get(macSecUnitId, &unitStatus);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get ECC status counters if it is desired */
    if (getEccStatus)
    {
        for (i=0;i<CPSS_DXCH_MACSEC_SECY_ECC_NOF_STATUS_COUNTERS_CNS;i++)
        {
            unitStatusPtr->eccCountersInfo[i].correctableCount   = unitStatus.ECCStatus_p->Counters[i].CorrectableCount;
            unitStatusPtr->eccCountersInfo[i].correctableThr     = unitStatus.ECCStatus_p->Counters[i].fCorrectableThr;
            unitStatusPtr->eccCountersInfo[i].uncorrectableCount = unitStatus.ECCStatus_p->Counters[i].UncorrectableCount;
            unitStatusPtr->eccCountersInfo[i].uncorrectableThr   = unitStatus.ECCStatus_p->Counters[i].fUncorrectableThr;
        }
    }

    /* Get packet processing debug information if it is desired */
    if(getPktProcessDebug)
    {
        unitStatusPtr->pktProcessDebug.secTagDebug    = unitStatus.PktProcessDebug_p->SecTAGDebug;
        unitStatusPtr->pktProcessDebug.parsedDaHi     = unitStatus.PktProcessDebug_p->ParsedDAHi;
        unitStatusPtr->pktProcessDebug.parsedDaLo     = unitStatus.PktProcessDebug_p->ParsedDALo;
        unitStatusPtr->pktProcessDebug.parsedSaHi     = unitStatus.PktProcessDebug_p->ParsedSAHi;
        unitStatusPtr->pktProcessDebug.parsedSaLo     = unitStatus.PktProcessDebug_p->ParsedSALo;
        unitStatusPtr->pktProcessDebug.parsedSciHi    = unitStatus.PktProcessDebug_p->ParsedSCIHi;
        unitStatusPtr->pktProcessDebug.parsedSciLo    = unitStatus.PktProcessDebug_p->ParsedSCILo;
        unitStatusPtr->pktProcessDebug.parsedSecTagHi = unitStatus.PktProcessDebug_p->ParsedSecTAGHi;
        unitStatusPtr->pktProcessDebug.parsedSecTagLo = unitStatus.PktProcessDebug_p->ParsedSecTAGLo;
        unitStatusPtr->pktProcessDebug.parserInDebug  = unitStatus.PktProcessDebug_p->ParserInDebug;
        unitStatusPtr->pktProcessDebug.rxCamSciHi     = unitStatus.PktProcessDebug_p->RxCAMSCIHi;
        unitStatusPtr->pktProcessDebug.rxCamSciLo     = unitStatus.PktProcessDebug_p->RxCAMSCILo;
    }

    return rc;
}

/**
* @internal cpssDxChMacSecSecyStatusGet function
* @endinternal
*
* @brief   Reads status information from Transformer unit
*          - Get ECC status
*          - Get parser debug
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  getEccStatus        - Set to GT_TRUE to get ECC status information otherwise set to GT_FALSE
* @param[in]  getPktProcessDebug  - Set to GT_TRUE to get packet processing debug information otherwise set to GT_FALSE
* @param[out] deviceStatusPtr     - (pointer to) unit's status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatusGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   GT_BOOL                                 getEccStatus,
    IN   GT_BOOL                                 getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC   *unitStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecyStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, getEccStatus, getPktProcessDebug, unitStatusPtr));

    rc = internal_cpssDxChMacSecSecyStatusGet(devNum, unitBmp, direction, getEccStatus, getPktProcessDebug, unitStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, getEccStatus, getPktProcessDebug, unitStatusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSAExpiredSummaryGet function
* @endinternal
*
* @brief   Reads list of SA expired indexes.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                     - device number.
* @param[in]  unitBmp                    - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                          - 1: select DP0, 2: select DP1.
*                                          - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                          - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                  - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                 - list of SA expired summary indexes, maximum allowed indexes is
*                                          512 for AC5P; Harrier
*                                          128 for AC5X
* @param[out] indexesArrSizePtr          - (pointer to) total count of expired SA summary indexes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSAExpiredSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr); /* must always be valid and able to hold total expired SA summary indexes */

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_SA_ExpiredSummary_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;

}


/**
* @internal cpssDxChMacSecSAExpiredSummaryGet function
* @endinternal
*
* @brief   Reads list of SA expired indexes.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                     - device number.
* @param[in]  unitBmp                    - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                          - 1: select DP0, 2: select DP1.
*                                          - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                          - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                  - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                 - list of SA expired summary indexes, maximum allowed indexes is
*                                          512 for AC5P; Harrier
*                                          128 for AC5X
* @param[out] indexesArrSizePtr          - (pointer to) total count of expired SA summary indexes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSAExpiredSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSAExpiredSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecSAExpiredSummaryGet(devNum, unitBmp, direction, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSAPNThresholdSummaryGet function
* @endinternal
*
* @brief   Reads list of SA indexes where packet number processed have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                          - device number.
* @param[in]  unitBmp                         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                               - 1: select DP0, 2: select DP1.
*                                               - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                               - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                      - list of SA indexes where packet number processed is more that the
*                                               threshold, maximum allowed indexes is
*                                               512 for AC5P; Harrier
*                                               128 for AC5X
* @param[out] indexesArrSizePtr               - (pointer to) total count of SA indexes where packet number
*                                               statistics counter crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Egress MACSec transformer unit
*/
static GT_STATUS internal_cpssDxChMacSecSAPNThresholdSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_SA_PnThrSummary_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;

}

/**
* @internal cpssDxChMacSecSAPNThresholdSummaryGet function
* @endinternal
*
* @brief   Reads list of SA indexes where packet number processed have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                          - device number.
* @param[in]  unitBmp                         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                               - 1: select DP0, 2: select DP1.
*                                               - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                               - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                      - list of SA indexes where packet number processed is more that the
*                                               threshold, maximum allowed indexes is
*                                               512 for AC5P; Harrier
*                                               128 for AC5X
* @param[out] indexesArrSizePtr               - (pointer to) total count of SA indexes where packet number
*                                               statistics counter crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Egress MACSec transformer unit
*/
GT_STATUS cpssDxChMacSecSAPNThresholdSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSAPNThresholdSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecSAPNThresholdSummaryGet(devNum, unitBmp, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSACountSummaryGet function
* @endinternal
*
* @brief   Reads list of SA indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                          - device number.
* @param[in]  unitBmp                         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                               - 1: select DP0, 2: select DP1.
*                                               - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                             - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                       - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                      - list of SA indexes whose statistics count is more than the threshold,
*                                               maximum allowed indexes is
*                                               512 for AC5P; Harrier
*                                               128 for AC5X
* @param[out] indexesArrSizePtr               - (pointer to) total count of SA indexes whose statistics has crossed the thteshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSACountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_Device_CountSummary_PSA_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;
}

/**
* @internal cpssDxChMacSecSACountSummaryGet function
* @endinternal
*
* @brief   Reads list of SA indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                          - device number.
* @param[in]  unitBmp                         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                               - 1: select DP0, 2: select DP1.
*                                               - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                             - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                       - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                      - list of SA indexes whose statistics count is more than the threshold,
*                                               maximum allowed indexes is
*                                               512 for AC5P; Harrier
*                                               128 for AC5X
* @param[out] indexesArrSizePtr               - (pointer to) total count of SA indexes whose statistics has crossed the thteshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSACountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT   GT_U32                                 indexesArr[], /*maxArraySize=512*/
    OUT   GT_U32                                 *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSACountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecSACountSummaryGet(devNum, unitBmp, direction, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecSecYCountSummaryGet function
* @endinternal
*
* @brief   Reads list of SecY vPort indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                             - device number.
* @param[in]  unitBmp                            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                  - 1: select DP0, 2: select DP1.
*                                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                  - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                          - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                         - list of SecY vPort indexes whose statistics have crossed the threshold,
*                                                  maximum allowed indexes is
*                                                  256 for AC5P; Harrier
*                                                  64 for AC5X
* @param[out] indexesArrSizePtr                  - (pointer to) total count of SecY vPort indexes whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecSecYCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_Device_CountSummary_PSecY_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;
}

/**
* @internal cpssDxChMacSecSecYCountSummaryGet function
* @endinternal
*
* @brief   Reads list of SecY vPort indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                             - device number.
* @param[in]  unitBmp                            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                  - 1: select DP0, 2: select DP1.
*                                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                  - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                          - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                         - list of SecY vPort indexes whose statistics have crossed the threshold,
*                                                  maximum allowed indexes is
*                                                  256 for AC5P; Harrier
*                                                  64 for AC5X
* @param[out] indexesArrSizePtr                  - (pointer to) total count of SecY vPort indexes whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecYCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecSecYCountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecSecYCountSummaryGet(devNum, unitBmp, direction, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecIfc0CountSummaryGet function
* @endinternal
*
* @brief   Reads list of Ifc0 indexes per vPort whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                        - list of Ifc0 indexes per vPort whose statistics have crossed the threshold,
*                                                 maximum allowed indexes is
*                                                 256 for AC5P; Harrier
*                                                 64 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of Ifc0 indexes per vPort whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecIfc0CountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_Device_CountSummary_PIfc_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;
}

/**
* @internal cpssDxChMacSecIfc0CountSummaryGet function
* @endinternal
*
* @brief   Reads list of Ifc0 indexes per vPort whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                        - list of Ifc0 indexes per vPort whose statistics have crossed the threshold,
*                                                 maximum allowed indexes is
*                                                 256 for AC5P; Harrier
*                                                 64 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of Ifc0 indexes per vPort whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecIfc0CountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecIfc0CountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecIfc0CountSummaryGet(devNum, unitBmp, direction, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecIfc1CountSummaryGet function
* @endinternal
*
* @brief   Reads list of Ifc1 indexes per vPort whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                        - list of Ifc1 indexes per vPort whose statistics have crossed the threshold,
*                                                 maximum allowed indexes is
*                                                 256 for AC5P; Harrier
*                                                 64 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of Ifc1 indexes per vPort whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Ingress MACSec transformer unit
*/
static GT_STATUS internal_cpssDxChMacSecIfc1CountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_Device_CountSummary_PIfc1_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;
}

/**
* @internal cpssDxChMacSecIfc1CountSummaryGet function
* @endinternal
*
* @brief   Reads list of Ifc1 indexes per vPort whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                        - list of Ifc1 indexes per vPort whose statistics have crossed the threshold,
*                                                 maximum allowed indexes is
*                                                 256 for AC5P; Harrier
*                                                 64 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of Ifc1 indexes per vPort whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Ingress MACSec transformer unit
*/
GT_STATUS cpssDxChMacSecIfc1CountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecIfc1CountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecIfc1CountSummaryGet(devNum, unitBmp, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecRxCamCountSummaryGet function
* @endinternal
*
* @brief   Reads list of RxCAM indexes whose RxCAM hit counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                             - device number.
* @param[in]  unitBmp                            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                  - 1: select DP0, 2: select DP1.
*                                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                  - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                         - list of RxCam hit indexes whose statistics have crossed the threshold,
*                                                  maximum allowed indexes is
*                                                  256 for AC5P; Harrier
*                                                  64 for AC5X
* @param[out] indexesArrSizePtr                  - (pointer to) total count of RxCam hit counter indexes whose statistics
*                                                  counters have crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Ingress MACSec transformer unit
*/
static GT_STATUS internal_cpssDxChMacSecRxCamCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS                rc;
    GT_U32                   macSecUnitId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexesArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexesArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E,
                                          CPSS_DXCH_MACSEC_TRANSFORMER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = SecY_Device_CountSummary_PRxCAM_CheckAndClear(macSecUnitId, &indexesArr, indexesArrSizePtr);

    return rc;
}

/**
* @internal cpssDxChMacSecRxCamCountSummaryGet function
* @endinternal
*
* @brief   Reads list of RxCAM indexes whose RxCAM hit counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                             - device number.
* @param[in]  unitBmp                            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                  - 1: select DP0, 2: select DP1.
*                                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                  - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                         - list of RxCam hit indexes whose statistics have crossed the threshold,
*                                                  maximum allowed indexes is
*                                                  256 for AC5P; Harrier
*                                                  64 for AC5X
* @param[out] indexesArrSizePtr                  - (pointer to) total count of RxCam hit counter indexes whose statistics
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Ingress MACSec transformer unit
*/
GT_STATUS cpssDxChMacSecRxCamCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecRxCamCountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, indexesArr, indexesArrSizePtr));

    rc = internal_cpssDxChMacSecRxCamCountSummaryGet(devNum, unitBmp, indexesArr, indexesArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, indexesArr, indexesArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecCfyePortCountSummaryGet function
* @endinternal
*
* @brief   Reads CfyE counter summary registers and returns list of physical ports.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[out] portsArr            - list of physical ports for which summary counters is read, maximum allowed indexes
*                                   is as per the maximum number of physical ports supported in the device
* @param[out] portsArrSizePtr     - (pointer to) total count of physical ports returned
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecCfyePortCountSummaryGet
(
    IN   GT_U8                                              devNum,
    IN   GT_MACSEC_UNIT_BMP                                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                     direction,
    OUT  GT_PHYSICAL_PORT_NUM                               portsArr[], /*maxArraySize=128*/
    OUT  GT_U32                                             *portsArrSizePtr
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   macSecUnitId;
    CfyE_Ch_Mask_t           channelBmp;
    GT_U32                   portBmp;
    GT_U32                   bitPos;
    GT_U32                   index;
    GT_U32                   channelNum;
    GT_PHYSICAL_PORT_NUM     physicalPortNum;
    GT_BOOL                  secondWord;
    GT_U32                   dpId;
    GT_U32                   maxChannels;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(portsArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(portsArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = CfyE_Statistics_Summary_Channel_Read(macSecUnitId, &channelBmp);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get maximum number of channels */
    rc = SecY_Device_Limits(macSecUnitId, &maxChannels, NULL, NULL, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }
    secondWord = GT_FALSE;
    portBmp = channelBmp.ch_bitmask[0];
    index = 0; /* physical ports array indexes */
    bitPos = 0;/* bit iterator for the CfyE ports */

    /* Get DP ID (0 or 1) */
    dpId = (unitBmp>>1) & 0x1;

    while(portBmp != 0)
    {
        if(portBmp & 0x1)
        {
            /* Get local channel number */
            channelNum = bitPos;
            /* phoenix : single DP include ALL channels (54 network ports + 2 SDMA) and so to get channel
             * number based on second word need to go past 32 bits word size
             * Hawk has 26 ports per DP and Harrier 16 ports per DP */
            if((maxChannels > 32) && secondWord)
            {
                channelNum += 32;
            }
            /* Get physical port number */
            rc = prvCpssDxChPortPhysicalPortGet(devNum, dpId, channelNum, &physicalPortNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            portsArr[index] = physicalPortNum;
            index++;
        }
        portBmp >>= 1;
        bitPos++;
        /* to parse the second word */
        if(!portBmp && secondWord == GT_FALSE)
        {
            portBmp = channelBmp.ch_bitmask[1];
            bitPos = 0;
            secondWord = GT_TRUE;
        }
    }
    *portsArrSizePtr = index;

    return rc;
}

/**
* @internal cpssDxChMacSecCfyePortCountSummaryGet function
* @endinternal
*
* @brief   Reads CfyE counter summary registers and returns list of physical ports.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[out] portsArr            - list of physical ports for which summary counters is read, maximum allowed indexes
*                                   is as per the maximum number of physical ports supported in the device
* @param[out] portsArrSizePtr     - (pointer to) total count of physical ports returned
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecCfyePortCountSummaryGet
(
    IN   GT_U8                                              devNum,
    IN   GT_MACSEC_UNIT_BMP                                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                     direction,
    OUT  GT_PHYSICAL_PORT_NUM                               portsArr[], /*maxArraySize=128*/
    OUT  GT_U32                                             *portsArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecCfyePortCountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, portsArr, portsArrSizePtr));

    rc = internal_cpssDxChMacSecCfyePortCountSummaryGet(devNum, unitBmp, direction, portsArr, portsArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, portsArr, portsArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChMacSecCfyeTcamCountSummaryGet function
* @endinternal
*
* @brief   Reads TCAM counters whose rule statistics have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec classifier
* @param[out] indexArr                          - list of read TCAM counter summary registers whose statistics have crossed
*                                                 the threshold, maximum allowed indexes is
*                                                 512 for AC5P; Harrier
*                                                 128 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of tcam counter indexes whose statistics
*                                                 have crossed the threshold
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
static GT_STATUS internal_cpssDxChMacSecCfyeTcamCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexArrSizePtr
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   macSecUnitId;
    CfyE_Device_Limits_t     tcamRuleCount;
    GT_U32                   startOffset;
    GT_U32                   scanCount;
    GT_U32                   loop;
    GT_U32                   bitPos;
    GT_U32                   index;
    GT_U32                   tcamRuleBmp;
    GT_U32                   iterations;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    /* Check pointer is not NULL */
    CPSS_NULL_PTR_CHECK_MAC(indexArrSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexArr);

    /* Get MACSec unit ID. Check unitBmp parameter is valid */
    rc = prvCpssMacSecUnitBitsMapToUnitId(devNum,
                                          unitBmp,
                                          direction,
                                          CPSS_DXCH_MACSEC_CLASSIFIER_E,
                                          &macSecUnitId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get TCAM rules limit */
    rc = CfyE_Device_Limits_Get(macSecUnitId, &tcamRuleCount);
    if (rc != GT_OK)
    {
        return rc;
    }

    scanCount = 1; /* number of registers to read in every TCAM_Read call */
    iterations = (tcamRuleCount.rule_count + 31)/ 32; /* count of number of times to call TCAM_Read */
    index = 0; /* tcam rule array indexes */

    for (loop = 0; loop < iterations; loop++)
    {
        startOffset = loop; /* TCAM statistics summary register number */
        rc = CfyE_Statistics_Summary_TCAM_Read(macSecUnitId, startOffset, &tcamRuleBmp, scanCount);
        if (rc != GT_OK)
        {
            return rc;
        }

        bitPos = 0;/* bit iterator for the TCAM array indexes */
        while(tcamRuleBmp != 0)
        {
            if(tcamRuleBmp & 0x1)
            {
                /* Get set bits position where the summary count for TCAM rules has crossed the threshold */
                indexArr[index] = bitPos + (32 * loop);
                index++;
            }
            tcamRuleBmp >>= 1;
            bitPos++;
        }
    }

    *indexArrSizePtr = index;
    return rc;
}

/**
* @internal cpssDxChMacSecCfyeTcamCountSummaryGet function
* @endinternal
*
* @brief   Reads TCAM counters whose rule statistics have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec classifier
* @param[out] indexArr                          - list of read TCAM counter summary registers whose statistics have crossed
*                                                 the threshold, maximum allowed indexes is
*                                                 512 for AC5P; Harrier
*                                                 128 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of tcam counter indexes whose statistics
*                                                 have crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecCfyeTcamCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexArr[], /*maxArraySize=128*/
    OUT  GT_U32                                  *indexArrSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChMacSecCfyeTcamCountSummaryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitBmp, direction, indexArr, indexArrSizePtr));

    rc = internal_cpssDxChMacSecCfyeTcamCountSummaryGet(devNum, unitBmp, direction, indexArr, indexArrSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitBmp, direction, indexArr, indexArrSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

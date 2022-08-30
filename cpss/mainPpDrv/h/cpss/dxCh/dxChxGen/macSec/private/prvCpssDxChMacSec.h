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
* @file prvCpssDxChMacSec.h
*
* @brief CPSS DxCh MAC Security (or MACsec) private API.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChMacSech
#define __prvCpssDxChMacSech

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */






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
);


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
);


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
);


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
);


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
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChCnch */




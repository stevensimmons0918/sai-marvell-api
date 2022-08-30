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
* @file prvCpssDxChMTIMacCtrl.h
*
* @brief SIP6 MTI mac control API
*
* @version   1
********************************************************************************
*/

#ifndef PRV_CPSS_DXCH_MTI_MAC_CTRL_H
#define PRV_CPSS_DXCH_MTI_MAC_CTRL_H

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChPortMTIMacPreambleLengthSet function
* @endinternal
*
* @brief   Set Preamble length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   - preamble length
*                                       supported length values are : 1,4,8.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortMTIMacPreambleLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
);

/**
* @internal prvCpssDxChPortMTIMacPreambleLengthGet function
* @endinternal
*
* @brief   Get Preamble length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] macType               - mac type
* @param[out] lengthPtr            - (pointer to) preamble length
*                                     supported length values are : 1,4,8.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChPortMTIMacPreambleLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   macType,
    OUT GT_U32                  *lengthPtr
);

/**
* @internal prvCpssDxChPortMTIMacIPGLengthSet function
* @endinternal
*
* @brief   Set IPG length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] length                   - ipg length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortMTIMacIPGLengthSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   length
);


/**
* @internal prvCpssDxChPortMTIMacIPGLengthGet function
* @endinternal
*
* @brief   Set IPG length for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                - device number
* @param[in] portNum               - physical port number
* @param[in] macType               - mac type
* @param[out] lengthPtr            - pointer to length
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mac
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortMTIMacIPGLengthGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   macType,
    OUT GT_U32                  *lengthPtr
);

/**
* @internal prvCpssDxChPortMTIMacNumCRCBytesSet function
* @endinternal
*
* @brief   Set number of CRC bytes for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] crcBytesNum              - number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssDxChPortMTIMacNumCRCBytesSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   crcBytesNum
);

/**
* @internal prvCpssDxChPortMTIMacNumCRCBytesGet function
* @endinternal
*
* @brief   Get number of CRC bytes for MTI MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X.
*                                  Bobcat3; Aldrin2;
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] macType               - mac type
* @param[out] crcBytesNumPtr       - pointer to number of CRC bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChPortMTIMacNumCRCBytesGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN PRV_CPSS_PORT_TYPE_ENT   macType,
    OUT GT_U32                  *crcBytesNumPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


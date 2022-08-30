/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file prvCpssDxChIdebugAuto.h
*
* @brief Automatic private iDebug Types for CPSS.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChIdebugAutoh
#define __prvCpssDxChIdebugAutoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>

/**
* @internal prvCpssDxChIdebugFieldValidityLogicUpdate function
* @endinternal
*
* @brief   update field validity logic
*
* @note   APPLICABLE DEVICES:      Aldrin; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] fieldName                - field name
* @param[in] numOfFields              - number of interface
*                                       fields
* @param[in,out] fieldsValueArr       - interface fields
*                                       values array
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS prvCpssDxChIdebugFieldValidityLogicUpdate
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_CHAR_PTR                             fieldName,
    IN  GT_U32                                  numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[]
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChIdebugAutoh */

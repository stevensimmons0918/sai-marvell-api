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
* @file cpssDxChPortTxQHighSpeedPortsDrv.h
*
* @brief bobcat2 and higher TxQ high speed port support
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_DXCH_PORT_TXQ_HIGH_SPEED_PORTS_DRV_H
#define __CPSS_DXCH_PORT_TXQ_HIGH_SPEED_PORTS_DRV_H

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS        8


/**
* @internal cpssDxChPortDynamicPATxQHighSpeedPortGet function
* @endinternal
*
* @brief   get list of txQ ports that are declared as high speed
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - bad pointer
* @retval GT_FAIL                  - on error
*
* @note if highSpeedPortIdxArr is NULL, port indexes are not filled
*
*/
GT_STATUS cpssDxChPortDynamicPATxQHighSpeedPortGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *highSpeedPortNumberPtr,
    OUT GT_U32  portNumArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS],
    OUT GT_U32  highSpeedPortIdxArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);

/**
* @internal cpssDxChPortDynamicPATxQHighSpeedPortDumpGet function
* @endinternal
*
* @brief   get dump of LL and DQ unit
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] txqDqPortNumArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS] - array of TxQ ports that are declared as high speed
* @param[out] txqLLPortNumArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS] - array high speed port idx assigned to corresponded TxQ port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - bad pointer
* @retval GT_FAIL                  - on error
*
* @note if high speed port is not assigned to any port , the corresponding entry is filled by ~0
*
*/
GT_STATUS cpssDxChPortDynamicPATxQHighSpeedPortDumpGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  txqDqPortNumArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS],
    OUT GT_U32  txqLLPortNumArr[CPSS_DXCH_PORT_HIGH_SPEED_PORT_NUM_CNS]
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


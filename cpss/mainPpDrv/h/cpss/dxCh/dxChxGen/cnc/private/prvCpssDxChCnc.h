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
* @file prvCpssDxChCnc.h
*
* @brief CPSS DxCh Centralized Counters (CNC) private API.
*
* @version   10
********************************************************************************
*/

#ifndef __prvCpssDxChCnch
#define __prvCpssDxChCnch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>

/* general "less check" macro used in other macros below */
#define LESS_CHECK_MAC(_value, _pattern)     \
     if ((_value) >= (_pattern))                      \
     {                                                \
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "%s[%d] >= %s[%d]",    \
            #_value,_value,                                                 \
            #_pattern,_pattern);                                            \
     }

/* checks that the number of CNC block */
#define PRV_CPSS_DXCH_CNC_BLOCK_INDEX_VALID_CHECK_MAC(_dev, _blockIndex) \
    LESS_CHECK_MAC(_blockIndex, PRV_CPSS_DXCH_PP_MAC(_dev)->fineTuning.tableSize.cncBlocks)

/* counter index range check 0..(2K-1) */
#define PRV_CPSS_DXCH_CNC_COUNTER_INDEX_VALID_CHECK_MAC(_dev, counterIndex) \
    LESS_CHECK_MAC(counterIndex, PRV_CPSS_DXCH_PP_MAC(_dev)->fineTuning.tableSize.cncBlockNumEntries)

/* CNC clients configuration */
#define CNC_BLOCK_CLIENT_CFG_ADDR_INCREMENT_CNS  0x00000100

/**
* @internal prvCpssDxChCncBlockClientToCfgAddr function
* @endinternal
*
* @brief   The function converts the client to HW base address of configuration register.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] client                   - CNC client
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
*
* @param[out] cfgRegPtr                - (pointer to) address of the block configuration register.
* @param[out] bitOffsetPtr             - (pointer to) bit offset of the block configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
GT_STATUS prvCpssDxChCncBlockClientToCfgAddr
(
    IN   GT_U8                     devNum,
    IN   CPSS_DXCH_CNC_CLIENT_ENT  client,
    IN   GT_U32                    blockNum,
    OUT  GT_U32                    *cfgRegPtr,
    OUT  GT_U32                    *bitOffsetPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChCnch */




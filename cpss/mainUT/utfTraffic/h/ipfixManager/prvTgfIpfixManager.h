/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file prvTgfIpfixManager.h
*
* @brief IPFIX Manager functional testing private API declarations
*
* @version   1
********************************************************************************
*/

GT_VOID prvTgfIpfixManagerVlanConfigSet
(
    GT_VOID
);

/**
 * @internal prvTgfIpfixManagerDataPktsCheck function
 * @endinternal
 *
 * @brief Check data packet
 *
 * @retval GT_VOID
 */
GT_VOID prvTgfIpfixManagerDataPktsCheck
(
    GT_VOID
);

GT_VOID prvTgfIpfixManagerTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount,
    IN GT_BOOL  captureEnable
);

GT_VOID prvTgfIpfixManagerConfigRestore
(
    GT_VOID
);

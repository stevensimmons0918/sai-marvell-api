/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfMplsSrEntropyLabelUseCase.h
*
* DESCRIPTION:
*       MPLS Entropy Label use case API
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfMplsSrEntropyLabelUseCaseh
#define __prvTgfMplsSrEntropyLabelUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfMplsSrEntropyLabelConfigurationSet function
* @endinternal
*
* @brief   Set Entropy Label Configuration
*/
GT_VOID prvTgfMplsSrEntropyLabelConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfMplsSrEntropyLabelTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfMplsSrEntropyLabelTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfMplsSrEntropyLabelConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @retval GT_OK                    - on success
*/
GT_VOID prvTgfMplsSrEntropyLabelConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __prvTgfMplsSrEntropyLabelUseCaseh */

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
* @file prvTgfSgt.h
*
* @brief SGT (Security group Tag) - Test case macros & function definition
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfSgth
#define __prvTgfSgth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GT_VOID prvTgfSgtUserPort_test(GT_VOID);
GT_VOID prvTgfSgtNetworkPort_test(GT_VOID);
GT_VOID prvTgfSgtTunnelPort_test(GT_VOID);
GT_VOID prvTgfSgtCascadePort_test(GT_VOID);
GT_VOID tgfSgtUseSrcTrgMode_test1(GT_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfSgth */



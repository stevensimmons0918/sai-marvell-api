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
* @file prvCpssPxPhySmi.h
*
* @brief Private definitions for PHY SMI.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxPhySmih
#define __prvCpssPxPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvCpssPxPhySmiObjBind function
* @endinternal
*
* @brief   Binds SMI service function pointers:
*         SMI Control Register Read/Write.
*         The generic SMI functions - cpssSmiRegisterReadShort/cpssSmiRegisterWriteShort
*         use these pointers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*                                       none.
*/
GT_VOID prvCpssPxPhySmiObjBind
(
    IN  GT_SW_DEV_NUM     devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxPhySmih */



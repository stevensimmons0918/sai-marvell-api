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
* @file prvCpssBindFunc.h
*
* @brief external functions wrapper. definitions for bind OS,extDrv,trace
* dependent utilities to CPSS .
*
* private CPSS file
*
*
* @version   20
********************************************************************************
*/

#ifndef __prvCpssBindFunch
#define __prvCpssBindFunch

#ifdef __cplusplus
extern "C" {
#endif



/************* Includes *******************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/extServices/cpssExtServices.h>
#endif
/* macro to free allocation of pointer only in not NULL , and set it NULL */
#define FREE_PTR_MAC(ptr) if(ptr)cpssOsFree(ptr); ptr = NULL


/************* global *********************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* __prvCpssBindFunch */




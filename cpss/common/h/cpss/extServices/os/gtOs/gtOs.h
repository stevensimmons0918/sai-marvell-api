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
* @file gtOs.h
*
* @brief Operating System wrapper
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsh
#define __gtOsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>

#include <gtOs/cpssOsIntr.h>
#include <gtOs/cpssOsIo.h>
#include <gtOs/cpssOsMem.h>
#include <gtOs/cpssOsRand.h>
#include <gtOs/cpssOsSem.h>
#include <gtOs/cpssOsStr.h>
#include <gtOs/cpssOsTask.h>
#include <gtOs/cpssOsTimer.h>

#if (!defined __gtOsSemh) && (!defined __cmdExtServices_h_)
typedef GT_UINTPTR GT_SEM;
#endif /*!__gtOsSemh*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __gtOsh */




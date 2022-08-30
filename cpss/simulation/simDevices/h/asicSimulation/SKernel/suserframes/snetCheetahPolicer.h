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
* @file snetCheetahPolicer.h
*
* @brief (Cheetah) Policing Engine processing for frame -- simulation
*
* @version   1
********************************************************************************
*/
#ifndef __snetCheetahPolicerh
#define __snetCheetahPolicerh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
* @internal snetChtPolicer function
* @endinternal
*
* @brief   Policer Processing --- Policer Counters updates
*/
GT_VOID snetChtPolicer(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr, 
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetahPolicerh */




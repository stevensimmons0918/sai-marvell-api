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
* @file skernel.h
*
* @brief This is a external API definition for SKernel simulation.
*
* @version   3
********************************************************************************
*/
#ifndef __skernelh
#define __skernelh

#ifdef APPLICATION_SIDE_ONLY
    /* this file exists only when it is not application side only , because those
       files are on the devices side */
    #error "this file exists only when it is not application side only"
#endif /*APPLICATION_SIDE_ONLY*/

#include <asicSimulation/SInit/sinit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* typedef of Rx NIC callback function */
typedef GT_STATUS (* SKERNEL_NIC_RX_CB_FUN ) (
    IN GT_U8_PTR   segmentList[],
    IN GT_U32      segmentLen[],
    IN GT_U32      numOfSegments
);

/**
* @internal skernelInit function
* @endinternal
*
* @brief   Init SKernel library.
*/
void skernelInit
(
    void
);

/**
* @internal skernelShutDown function
* @endinternal
*
* @brief   Shut down SKernel library before reboot.
*
* @note The function unbinds ports from SLANs
*
*/
void skernelShutDown
(
    void
);
/**
* @internal skernelNicRxBind function
* @endinternal
*
* @brief   Bind Rx callback routine with skernel.
*
* @param[in] rxCbFun                  - callback function for NIC Rx.
*/
void skernelNicRxBind
(
    SKERNEL_NIC_RX_CB_FUN rxCbFun
);
/**
* @internal skernelNicRxUnBind function
* @endinternal
*
* @brief   UnBind Rx callback routine with skernel.
*
* @param[in] rxCbFun                  - callback function for unbind NIC Rx.
*/
void skernelNicRxUnBind
(
    SKERNEL_NIC_RX_CB_FUN rxCbFun
);

/**
* @internal skernelNicOutput function
* @endinternal
*
* @brief   This function transmits an Ethernet packet to the NIC
*/
GT_STATUS skernelNicOutput
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments
);

/**
* @internal skernelStatusGet function
* @endinternal
*
* @brief   Get status (Idle or Busy) of all Simulation Kernel tasks.
*
* @retval 0                        - Simulation Kernel Tasks are Idle
* @retval other                    - Simulation Kernel Tasks are busy
*/
GT_U32 skernelStatusGet
(
    void
);

#define ASSERT_PTR(ptr)   \
    if (ptr == 0) skernelFatalError("Access violation: illegal pointer\n");

#define NOT_VALID_ADDR  0xBAD0ADD0

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __skernelh */




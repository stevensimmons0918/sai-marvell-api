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
* @file cpssTmServices.h
* @version   2
********************************************************************************
*/

#ifndef __cpssTmServicesh
#define __cpssTmServicesh

#include <tm_ctl.h>
#include <tm_errcodes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

extern GT_BOOL tmDbgLogOn;

#define TM_DBG_INFO(x)              if(tmDbgLogOn) cpssOsPrintf x

#define XEL_TO_CPSS_ERR_CODE(x)     ( (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmLibIsInitilized == GT_FALSE) ? GT_NOT_INITIALIZED : ( (x) < 0) ?  tmToCpssErrCodesNeg[-(x)]: tmToCpssErrCodes[(x)])


/* TM to CPSS error codes handling */
extern GT_STATUS tmToCpssErrCodes[TM_CONF_MAX_ERROR+1];
extern GT_STATUS tmToCpssErrCodesNeg[140];

/**
* @internal cpssTmToCpssErrCodesInit function
* @endinternal
*
* @brief   The function initializes the error codes transformation arrays.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
*                                       None.
*/
GT_VOID cpssTmToCpssErrCodesInit
(
    GT_VOID
);


#if 0
/* check TM unit exists in the device */
#define PRV_CPSS_TM_IS_EXIST_IN_DEVICE(devNum)              \
        (0 != (CPSS_PORTS_BMP_IS_PORT_SET_MAC(                       \
            &(PRV_CPSS_PP_MAC(devNum)->existingPorts) , portNum)))



 /* TM support */
UTF_VERIFY_EQUAL1_STRING_MAC(ptrMatrixData->tmSupport,
//UTF_VERIFY_TM_SUPPORT_MAC(ptrMatrixData->tmSupport,
                             PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.featureInfo.tmSupported,
                             "TM support for device 0x%x",
                             deviceType);


#endif
#endif 	    /* __cpssTmServicesh */


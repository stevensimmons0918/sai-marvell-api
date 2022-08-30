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
* @file cpssTmServices.c
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <errno.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_BOOL tmDbgLogOn = GT_FALSE;

GT_STATUS tmToCpssErrCodes[TM_CONF_MAX_ERROR+1] = {GT_NOT_INITIALIZED};
GT_STATUS tmToCpssErrCodesNeg[140] = {GT_NOT_INITIALIZED};

/**
* @internal internal_cpssTmToCpssErrCodesInit function
* @endinternal
*
* @brief   The function initializes the error codes transformation arrays.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
*                                       None.
*/
static GT_VOID internal_cpssTmToCpssErrCodesInit
(
    GT_VOID
)
{
    int       i = 0;

    /* Positive errors */
    /* 0 on Success */
    tmToCpssErrCodes[0] = GT_OK;

    /* HW errors */
    for (i=1; i<=TM_HW_MAX_ERROR; i++) {
        tmToCpssErrCodes[i] = GT_FAIL;
    }

    /* SW (configuration) errors */
    for (i=TM_HW_MAX_ERROR+1; i<=TM_CONF_PER_RATE_L_K_N_NOT_FOUND; i++) {
        tmToCpssErrCodes[i] = GT_BAD_VALUE;
    }

    for (i=TM_CONF_PORT_IND_OOR; i<=TM_CONF_TM2TM_PORT_FOR_CTRL_PKT_OOR; i++) {
        tmToCpssErrCodes[i] = GT_OUT_OF_RANGE;
    }

    for (i=TM_CONF_PORT_BW_OUT_OF_SPEED; i<=TM_CONF_REORDER_NODES_NOT_ADJECENT; i++) {
        tmToCpssErrCodes[i] = GT_BAD_PARAM;
    }

    for (i=TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE; i<=TM_CONF_REORDER_CHILDREN_NOT_AVAIL; i++) {
        tmToCpssErrCodes[i] = GT_BAD_SIZE;
    }

    for (i=TM_CONF_PORT_IND_NOT_EXIST; i<=TM_CONF_C_NODE_IND_NOT_EXIST; i++) {
        tmToCpssErrCodes[i] = GT_BAD_STATE;
    }

    tmToCpssErrCodes[TM_CONF_CANNT_GET_LAD_FREQUENCY] = GT_GET_ERROR; /* Possibly not relevant for cpss */

    for (i=TM_CONF_UPD_RATE_NOT_CONF_FOR_LEVEL; i<=TM_CONF_TM2TM_CHANNEL_NOT_CONFIGURED; i++) {
        tmToCpssErrCodes[i] = GT_NOT_INITIALIZED;
    }

    tmToCpssErrCodes[TM_CONF_PORT_IND_USED] = GT_ALREADY_EXIST;
    tmToCpssErrCodes[TM_CONF_SHAPING_PROF_REF_OOR]=GT_BAD_VALUE;
    tmToCpssErrCodes[TM_WRONG_SHP_PROFILE_LEVEL]=GT_BAD_VALUE;
    /* new periodic errors */
    tmToCpssErrCodes[TM_BW_OUT_OF_RANGE]=GT_OUT_OF_RANGE;
    tmToCpssErrCodes[TM_BW_UNDERFLOW]=GT_OUT_OF_RANGE;
    /* logical layer errors */
    tmToCpssErrCodes[TM_CONF_WRONG_LOGICAL_NAME] = GT_BAD_VALUE;
    tmToCpssErrCodes[TM_CONF_NULL_LOGICAL_NAME] = GT_BAD_VALUE;


    /* Negative errors */
    tmToCpssErrCodesNeg[EINVAL]  = GT_BAD_PTR;
    tmToCpssErrCodesNeg[EBADF]   = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[ENOBUFS] = GT_NO_RESOURCE;
    tmToCpssErrCodesNeg[ENOMEM]  = GT_NO_RESOURCE;
    tmToCpssErrCodesNeg[EBADMSG] = GT_NOT_INITIALIZED;
    tmToCpssErrCodesNeg[EACCES]  = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[EADDRNOTAVAIL] = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[EDOM]    = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[EFAULT]  = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[ENODATA] = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[ENODEV]  = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[ERANGE]  = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[EPERM]   = GT_BAD_PARAM;
    tmToCpssErrCodesNeg[EBUSY]   = GT_BAD_STATE;
    tmToCpssErrCodesNeg[ENOSPC]  = GT_FULL;
}

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
)
{
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmToCpssErrCodesInit);
    /*This API must use zero level API lock*/
    CPSS_ZERO_LEVEL_API_LOCK_NO_RETURN_MAC;
    CPSS_LOG_API_ENTER_MAC((funcId));

    internal_cpssTmToCpssErrCodesInit();

    CPSS_LOG_API_EXIT_MAC(funcId, GT_OK);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, GT_OK));

    CPSS_ZERO_LEVEL_API_UNLOCK_MAC;
}


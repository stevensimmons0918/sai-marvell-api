/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* tcamUtils.h
*
* DESCRIPTION:
*  utility functions for working with TCAM
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef __tcamUtils_h
#define __tcamUtils_h

#include <cpss/generic/cpssTypes.h>
#include <cpss/extServices/os/gtOs/gtEnvDep.h>



/* get the number of floors needed for the number of rules in the tcam */
#define CPSS_DXCH_TCAM_MAX_NUM_FLOORS_MAC(_devNum)                          \
    ROUNDUP_DIV_MAC(                                                        \
        PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.tunnelTerm,     \
        CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS)

/* macro to roundup a number that is divided so :
    for example :
    3 / 2 is round up to 2 and not round down to 1
*/
#define ROUNDUP_DIV_MAC(_number , _divider)             \
    (((_number) + ((_divider)-1)) / (_divider))

/**
* @internal csRefInfraFindTCAMFirstRule function
* @endinternal
*
* @brief   find first rules for TTI, PCL, EPCL  -used for setting TCAM rule at correct index.
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           -                   device number
* @param[out] groupFirstRuleIndexIPCL      -first IPCL rule index (1-st parallel lookup).
* @param[out] groupFirstRuleIndexEPCL      -first EPCL rule index
* @param[out] groupFirstRuleIndexTTI      -first TTI rule index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS csRefInfraFindTCAMFirstRule
(
    IN GT_U8  devNum,
    OUT GT_U32 *groupFirstRuleIndexIPCL,
    OUT GT_U32 *groupFirstRuleIndexEPCL,
    OUT GT_U32 *groupFirstRuleIndexTTI
);


/**
* @internal csRefInfraTCAMCountersGet function
* @endinternal
*
* @brief   printout non-zero  CNC TCAM counters, csRefInfraSetTCAMCounters must be called before
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           -                   device number
* @param[in] ipclBlocks           -  number of CNC blocks assigned to IPCL (lookup 0)
* @param[in] epclBlocks          -  number of CNC blocks assigned to EPCL
* @param[in] ttiBlocks             -  number of CNC blocks assigned to TTI
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*     should be called after csRefInfraSetTCAMCounters, and with the same parameters for block numbers
*     use like csRefInfraGetTCAMCounters 0,8,2,6
*/
GT_STATUS csRefInfraTCAMCountersGet
(
    IN GT_U8 devNum,
    IN GT_U8 ipclBlocks,
    IN GT_U8 epclBlocks,
    IN GT_U8 ttiBlocks
);


/**
* @internal csRefInfraTCAMCountersSet function
* @endinternal
*
* @brief   set CNC counters used for debugginh the configuration - TCAM rule match indication
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           -    device number
* @param[in] ipclBlocks           -  number of CNC blocks assigned to IPCL (lookup 0)
* @param[in] epclBlocks          -  number of CNC blocks assigned to EPCL
* @param[in] ttiBlocks             -  number of CNC blocks assigned to TTI
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
* counters are set for IPCL lookup 0, EPCL and TTI
*/
GT_STATUS csRefInfraTCAMCountersSet
(
    IN GT_U8 devNum,
    IN GT_U8 ipclBlocks,
    IN GT_U8 epclBlocks,
    IN GT_U8 ttiBlocks
);

#endif


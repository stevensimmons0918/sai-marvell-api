/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file  cpssAppUtilsHa.h
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   131
********************************************************************************
*/

#ifndef __cpssAppUtilsHa_h
#define __cpssAppUtilsHa_h

#ifdef __cplusplus
extern "C" {
#endif


/**
* @internal cpssDmaFreeAll  function
* @endinternal
*
* @brief   Free DMA allocation
* @param[in] devNum         - cpss Logical Device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDmaFreeAll
(
   GT_VOID
);

/**
* @internal cpssEmulateSwCrash function
* @endinternal
*
* @brief   Simulate SW crash.Required for HA testing.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoHaEmulateSwCrash
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif

#endif  /* __cpssAppUtilsHa_h */


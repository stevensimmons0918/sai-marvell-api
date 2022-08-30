/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
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
* @file cpssPpI2c.h
*
* @brief API implementation for device I2C controller
*
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPpI2ch
#define __cpssPpI2ch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/* Max I2C buses a device can have */
#define CPSS_I2C_MAX_BUS_PER_PP_CNS     4


/**
* @internal cpssI2cHwDriverCreateDrv function
* @endinternal
*
* @brief   Initialize PP I2C master driver, or retrives an already created
*          driver for the same [dev, bus_id]
*
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum
* @retval GT_OUT_OF_CPU_MEM        - no memory/resource
* @param[in] devNum                - device number
* @param[in] bus_id                - I2C bus id
* @param[in] frequency             - requested bus freq
* @param[in] tclk                  - board's tclk (peripherals clock).
*       (APPLICABLE RANGES: 166MHZ-512MHZ)
* @param[out]drvPtr                - pointer to the created (or already
*                                    existing) driver
*/
GT_STATUS cpssI2cHwDriverCreateDrv(
    IN  GT_U8   devNum,
    IN  GT_U8   bus_id,
    IN  GT_U32  frequency,
    IN  GT_U32  tclk,
    OUT CPSS_HW_DRIVER_STC **drvPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPpSmih */



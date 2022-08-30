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
* @file mvHwsDiagnostic.h
*
* @brief
*
*/

#ifndef __mvHwsDiagnostic_H
#define __mvHwsDiagnostic_H

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/**
* @internal mvHwsDiagDeviceDbCheck function
* @endinternal
*
* @brief   This API checks the DB initialization of PCS, MAC, Serdes
*         and DDR units
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS mvHwsDiagDeviceDbCheck(GT_U8 devNum, MV_HWS_DEV_TYPE devType);

/**
* @internal mvHwsDeviceTemperatureGet function
* @endinternal
*
* @brief   Get device's temperature.
*
* @param[in] devNum                   - system device number
* @param[out] temperaturePtr           - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsDeviceTemperatureGet
(
    IN GT_U8     devNum,
    OUT GT_32    *temperaturePtr
);

/**
* @internal mvHwsTseneAuxadc12nmTempGet function
* @endinternal
*
* @brief   Gets the PP temperature for devices with 12 nm sensor (a.k.a TSENE_AUXADC).
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon AC5P;
*         AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @param[out] temperaturePtr       - (pointer to) temperature in Celsius degrees
*                                      (can be negative)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_READY             - the temperature reading is not valid
*/
GT_STATUS mvHwsTseneAuxadc12nmTempGet
(
    IN GT_U8   devNum ,
    OUT GT_32  *tempInCelsiusPtr
);

#endif /* __mvHwsDiagnostic_H */









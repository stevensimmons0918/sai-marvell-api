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
* @file prvCpssEmulatorMode.h
*
* @brief Includes definitions for running the device in EMULATOR mode.
* The EMULATOR mode allow the Design team of the PP to test the RTL before
* tape-out , and by this save time and money.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssEmulatorMode_h
#define __prvCpssEmulatorMode_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/********* Include files ******************************************************/
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/**
* @internal cpssDeviceRunSet_onEmulator function
* @endinternal
*
* @brief   State that running on EMULATOR. (all devices considered to be running on
*         EMULATOR)
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
*                                       None.
*/
void  cpssDeviceRunSet_onEmulator(void);

/**
* @internal cpssDeviceRunSet_onEmulator_internalCpu function
* @endinternal
*
* @brief   State that running on EMULATOR and the SW runs in the 'internal CPU'
*          that is in the emulator.(not in the 'HOST CPU' that is over the PEX)
*          (all devices considered to be running on EMULATOR as 'internal CPU')
*
*         (when running on EMULATOR) This function must be called before
*         'phase 1' init of the device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
*                                       None.
*/
void  cpssDeviceRunSet_onEmulator_internalCpu(void);

/**
* @internal cpssDeviceRunCheck_onEmulator function
* @endinternal
*
* @brief   Check if running on EMULATOR.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
*
* @retval 0 - not running on EMULATOR.
* @retval 1 - running on EMULATOR.
*/
GT_U32  cpssDeviceRunCheck_onEmulator(void);

/**
* @internal cpssDeviceRunCheck_onEmulator_internalCpu function
* @endinternal
*
* @brief   Check if running on EMULATOR and the SW runs in the 'internal CPU'
*          that is in the emulator.(not in the 'HOST CPU' that is over the PEX)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
*
* @retval 0 - not running on EMULATOR in the internal CPU.
* @retval 1 - running on EMULATOR in the internal CPU.
*/
GT_U32  cpssDeviceRunCheck_onEmulator_internalCpu(void);

/**
* @internal prvCpssOnEmulatorSupportedAddrCheck function
* @endinternal
*
* @brief   This function checks existences of base addresses units in the Emulator
*         of the given device.
*         since the Emulator not supports all units , the CPSS wants to avoid
*         accessing those addresses.
*         the CPSS binds the Emulator with this functions to 'cpssDriver' so the
*         driver before accessing the Emulator can know not to access the device
*         on such addresses.
*         NOTE: purpose is to remove Emulator 'ERRORS' ,and even 'crash' of
*         Emulator on some unknown addresses.
*
* @note   APPLICABLE DEVICES:      Aldrin; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - thr PP's device number to init the struct for.
* @param[in] portGroupId              - port group ID
* @param[in] regAddr                  - register address
*
* @retval GT_TRUE                  - the unit/address is   implemented in the Emulator
* @retval GT_FALSE                 - the unit/address is NOT implemented in the Emulator
*
* @note Should be called ONLY in 'EMULATOR Mode'
*
*/
GT_BOOL prvCpssOnEmulatorSupportedAddrCheck(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroupId,
    IN  GT_U32      regAddr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssEmulatorMode_h */



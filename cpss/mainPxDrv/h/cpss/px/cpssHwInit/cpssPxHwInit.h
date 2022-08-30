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
* @file cpssPxHwInit.h
*
* @brief Includes CPSS PX level basic Hw initialization functions, and data
* structures.
*
* The PX driver supports next 'families' :
* ---  PIPE
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxHwInit_h
#define __cpssPxHwInit_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
#include <cpss/px/config/cpssPxCfgInit.h>

/**
* @enum CPSS_PX_PHA_FIRMWARE_TYPE_ENT
 *
 * @brief Used to select the type of PHA firmware to be loaded to the device.
*/
typedef enum{

    /** @brief default CPSS PHA firmware will be loaded to the device
     *  during initialization.
     */
    CPSS_PX_PHA_FIRMWARE_TYPE_DEFAULT_E,

    /** @brief All default CPSS PHA firmware related initializations will be bypassed.
     *  The PHA is bypassed in HW.
     */
    CPSS_PX_PHA_FIRMWARE_TYPE_NOT_USED_E

} CPSS_PX_PHA_FIRMWARE_TYPE_ENT;

/**
* @enum CPSS_PX_POWER_SUPPLIES_NUMBER_ENT
 *
 * @brief Indicates the number of Power Supplies to the device.
*/
typedef enum{

    /** separate power supplies - for core and for SERDES. */
    CPSS_PX_POWER_SUPPLIES_DUAL_E = 0,

    /** only one power supply is in use. The AVS configuration is bypassed in HW. */
    CPSS_PX_POWER_SUPPLIES_SINGLE_E

} CPSS_PX_POWER_SUPPLIES_NUMBER_ENT;


/**
* @struct CPSS_PX_INIT_INFO_STC
 *
 * @brief This struct defines specific hardware parameters, those parameters are
 * determined according to the board specific hardware components.
*/
typedef struct{

    /** HW info (bus type, bus address, mappings, interrupts, etc) */
    CPSS_HW_INFO_STC hwInfo;

    /** Management interface type (PCI/SMI/TWSI/PEX). */
    CPSS_PP_INTERFACE_CHANNEL_ENT mngInterfaceType;

    /** @brief Bitmap of configurable address completion regions
     *  used by interrupt handling routine.
     */
    GT_U32 isrAddrCompletionRegionsBmp;

    /** @brief Bitmap of configurable address completion regions
     *  used by CPSS API.
     *  The device have 8 address completion regions.
     *  Region 0 provide access to the lower addresses that contain also
     *  configuration of address completion regions. It must never be configured.
     *  Lists (coded as ranges) of other regions can be assigned for several CPUs
     *  for interrupt handling (for such CPU that processes the PP interrupts)
     *  and for all other CPSS API using.
     *  These assigned ranges of regions must not have common members.
     *  These assigned ranges of regions must not contain zero range.
     */
    GT_U32 appAddrCompletionRegionsBmp;

    /** @brief number of elements in dataIntegrityShadowPtr
     *  NOTE :
     *  can use value CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS in this case
     *  dataIntegrityShadowPtr is ignored ! dataIntegrityShadowPtr
     *  - (pointer to) list of logical tables that needs shadow
     *    support (needed for data integrity)
     */
    GT_U32 numOfDataIntegrityElements;

    CPSS_PX_LOGICAL_TABLES_SHADOW_STC *dataIntegrityShadowPtr;

    /** @brief indication that the init is done ONLY to allow the cpssDriver
     *  to allow HW access for 'read/write registers'
     *  GT_FALSE - doing full initialization for the PX driver + cpssDriver +
     *  all needed DB and all needed HW configurations.
     *  GT_TRUE - doing minimal initialization in the PX driver + cpssDriver.
     *  to allow the application call cpssDriver for HW access
     *  for 'read/write registers'
     *  in this mode many PX driver APIs are not supported.
     *  This mode also known as 'PEX only' mode.
     */
    GT_BOOL allowHwAccessOnly;

    /** type of the PHA firmware to be loaded to the device. */
    CPSS_PX_PHA_FIRMWARE_TYPE_ENT phaFirmwareType;

    /** @brief number of power supplies to the device.
     *  Comments:
     */
    CPSS_PX_POWER_SUPPLIES_NUMBER_ENT powerSuppliesNumbers;

} CPSS_PX_INIT_INFO_STC;

/**
* @internal cpssPxHwInit function
* @endinternal
*
* @brief   This function performs basic hardware configurations on the given PX, in
*         Hw registration phase.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to assign in CPSS DB for the initialized PX device.
* @param[in] initParamsPtr            - PX device init parameters.
*
* @param[out] deviceTypePtr            - The Pp's device Id.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_IMPLEMENTED       - the CPSS was not compiled properly
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwInit
(
    IN      GT_SW_DEV_NUM                    devNum,
    IN      CPSS_PX_INIT_INFO_STC            *initParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE              *deviceTypePtr
);

/**
* @internal cpssPxHwDescSizeGet function
* @endinternal
*
* @brief   This function returns the size in bytes of a several descriptor sizes,
*         for a given device.
*         the size needed for rx descriptors for packet to the CPU.
*         the alignment needed for rx buffer for packet to the CPU.
*         the size needed for tx descriptors for packets from the CPU.
*         the size needed for CNC descriptors (pair of counters each) for CNC fast dump.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] rxDescSizePtr            - (pointer to) The Rx  descrptor's size (in bytes).
*                                      ignored if NULL
* @param[out] rxBufAlignmentPtr        - (pointer to) The Rx buffer alignment size (in bytes).
*                                      ignored if NULL
* @param[out] txDescSizePtr            - (pointer to) The Tx  descrptor's size (in bytes).
*                                      ignored if NULL
* @param[out] cncDescSizePtr           - (pointer to) The Cnc descrptor's size (in bytes).
*                                      ignored if NULL
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *rxDescSizePtr,
    OUT GT_U32                  *rxBufAlignmentPtr,
    OUT GT_U32                  *txDescSizePtr,
    OUT GT_U32                  *cncDescSizePtr
);

/**
* @internal cpssPxHwPpSoftResetTrigger function
* @endinternal
*
* @brief   This routine issue soft reset for a specific device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Soft reset is the process in which selected entities (as configured by
*       using the cpssPxHwPpSoftResetSkipParamSet API) are reset to their
*       default values.
*
*/
GT_STATUS cpssPxHwPpSoftResetTrigger
(
    IN  GT_SW_DEV_NUM               devNum
);


/**
* @internal cpssPxHwPpSoftResetSkipParamSet function
* @endinternal
*
* @brief   This routine configure skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] skipEnable               - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwPpSoftResetSkipParamSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable

);

/**
* @internal cpssPxHwPpSoftResetSkipParamGet function
* @endinternal
*
* @brief   This routine return configuration of skip parameters related to soft reset.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the skip parameter to set
*                                      see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
*
* @param[out] skipEnablePtr            - GT_FALSE: Do Not Skip
*                                      GT_TRUE:  Skip
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxHwPpSoftResetSkipParamGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr

);

/**
* @internal cpssPxHwPpInitStageGet function
* @endinternal
*
* @brief   Indicates the initialization stage of the device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to reset.
*
* @param[out] initStagePtr             - pointer to the stage of the device
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxHwPpInitStageGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT CPSS_HW_PP_INIT_STAGE_ENT  *initStagePtr
);


/**
* @internal cpssPxHwInterruptCoalescingSet function
* @endinternal
*
* @brief   Configures the interrupt coalescing parameters and enable\disable the
*         functionality.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[in] period                   - Minimal IDLE  between two consecutive interrupts.
*                                      The units of this input parameter is in nSec, the
*                                      resolution is in 320 nSec (the HW resolution for the
*                                      interrupt coalescing period is 0.32uSec). In case interrupt
*                                      coalescing is enabled (enable == GT_TRUE)
*                                      (APPLICABLE RANGES: 320..5242560).
*                                      Otherwise (enable == GT_FALSE), ignored.
* @param[in] linkChangeOverride       - GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
* @param[in] period
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwInterruptCoalescingSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN GT_BOOL  enable,
    IN GT_U32   period,
    IN GT_BOOL  linkChangeOverride
);

/**
* @internal cpssPxHwInterruptCoalescingGet function
* @endinternal
*
* @brief   Gets the interrupt coalescing configuration parameters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE: interrupt coalescing is enabled
*                                      GT_FALSE: interrupt coalescing is disabled
* @param[out] periodPtr                - (pointer to) Minimal IDLE period between two consecutive
*                                      interrupts. The units are in nSec with resolution of
*                                      320nSec (due to HW resolution) and is relevant only in
*                                      case interrupt coalescing is enabled (enablePtr == GT_TRUE).
*                                      (APPLICABLE RANGES: 320..5242560).
* @param[out] linkChangeOverridePtr    - (pointer to)
*                                      GT_TRUE: A link change in one of the ports
*                                      results interrupt regardless of the (coalescing)
*                                      period.
*                                      GT_FALSE: A link change in one of the ports
*                                      does not results interrupt immediately but
*                                      according to the (coalescing) period.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The HW resolution for the interrupt coalescing period is 0.32uSec.
*
*/
GT_STATUS cpssPxHwInterruptCoalescingGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr,
    OUT GT_U32          *periodPtr,
    OUT GT_BOOL     *linkChangeOverridePtr
);

/**
* @internal cpssPxHwCoreClockGet function
* @endinternal
*
* @brief   This function returns the core clock value from HW.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp's device number.
*
* @param[out] coreClkPtr               - Pp's core clock from HW and aligned with CPSS DB (MHz)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxHwCoreClockGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_U32  *coreClkPtr
);

/**
* @internal cpssPxMicroInitBasicCodeGenerate function
* @endinternal
*
* @brief   This function performs basic and per feature code generation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] initParamsPtr            - PX device init parameters.
* @param[in] featuresBitmap           - bitmap of cpss features participated in code generation.
* @param[in] deviceTypePtr            - (pointer to) The Pp's device type.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_ALREADY_EXIST         - the devNum already in use
* @retval GT_NOT_SUPPORTED         - the device not supported by CPSS
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
* @retval GT_BAD_VALUE             - the driver found unknown device type
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxMicroInitBasicCodeGenerate
(
    IN GT_SW_DEV_NUM              devNum,
    IN CPSS_PX_INIT_INFO_STC      *initParamsPtr,
    IN GT_U32                     featuresBitmap,
    OUT CPSS_PP_DEVICE_TYPE       *deviceTypePtr
);

/**
* @internal cpssPxCfgHwDevNumGet function
* @endinternal
*
* @brief   Reads HW device number
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] hwDevNumPtr              - Pointer to HW device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCfgHwDevNumGet
(
    IN GT_U8    devNum,
    OUT GT_HW_DEV_NUM   *hwDevNumPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxHwInit_h */


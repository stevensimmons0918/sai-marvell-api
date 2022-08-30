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
* @file prvCpssDxChDiag.h
*
* @brief Internal header with diag utilities.
*
* @version   3
********************************************************************************
*/
#ifndef __prvCpssDxChDiagh
#define __prvCpssDxChDiagh

#include <cpss/common/cpssTypes.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/generic/diag/cpssDiag.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** Value that should be treated as unknown */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS ((GT_U32)-1)

/**
* @internal prvCpssDxChDiagBistCheckSkipClient function
* @endinternal
*
* @brief   Check skip of DFX client for BIST
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pipe                     - DFX pipe number
* @param[in] client                   - DFX client number
*
* @retval GT_TRUE                  - skip BIST
* @retval GT_FALSE                 - does not skip BIST
*/
GT_BOOL prvCpssDxChDiagBistCheckSkipClient
(
    IN  GT_U8       devNum,
    IN  GT_U32      pipe,
    IN  GT_U32      client
);

/**
* @internal prvCpssDxChDiagBistAllRamSet function
* @endinternal
*
* @brief   Use BIST to set physical RAMs with specific pattern values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] pattern                  - parren to set in RAMs.
*                                      May be one of the list: 0; 0x55; 0xAA; 0xFF.
* @param[in] skipCpuMemory            - skip internal CPU related memories from MC distribution
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_TIMEOUT               - on BIST timeout
* @retval GT_BAD_STATE             - on BIST failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering the BIST will cause memory content corruption!!!
*       Executing BIST cause multiple CPSS_PP_DATA_INTEGRITY_ERROR_E events
*       which can be masked by the application using cpssEventDeviceMaskSet API.
*
*/
GT_STATUS prvCpssDxChDiagBistAllRamSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pattern,
    IN  GT_BOOL                                     skipCpuMemory
);

/**
* @internal prvCpssDxChDiagBistCaelumFixBist function
* @endinternal
*
* @brief   Function fixes BIST Max Address register default value.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_TIMEOUT               - on BIST timeout
* @retval GT_BAD_STATE             - on BIST failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagBistCaelumFixBist
(
    IN  GT_U8   devNum
);

/**
* @internal prvCpssDxChPortGroupShadowEntryWrite function
* @endinternal
*
* @brief   Writes to PP's shadow at specific 'entry index'
*         (same index as in functions of prvCpssDxChPortGroupWriteTableEntry)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] tableType                - the 'HW table'
* @param[in] entryIndex               - index in the table
*                                      (same index as in functions of prvCpssDxChPortGroupWriteTableEntry)
* @param[in] entryValuePtr            - (pointer to) An array containing the data to be
*                                      copied into the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGroupShadowEntryWrite
(
    IN  GT_U8                 devNum,
    IN  GT_U32                portGroupId,
    IN  CPSS_DXCH_TABLE_ENT   tableType,
    IN  GT_U32                entryIndex,
    IN  GT_U32               *entryValuePtr
);

/**
* @internal prvCpssDxChPortGroupShadowLineWrite function
* @endinternal
*
* @brief   Writes to PP's shadow at specific 'line index'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] tableType                - the 'HW table'
* @param[in] lineIndex                - the line index in the 'HW table'
*                                      (not the same index as the functions of 'table engine' get (like
*                                      prvCpssDxChPortGroupWriteTableEntry))
* @param[in] entryValuePtr            - (pointer to) An array containing the data to be
*                                      copied into the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGroupShadowLineWrite
(
    IN  GT_U8                 devNum,
    IN  GT_U32                portGroupId,
    IN  CPSS_DXCH_TABLE_ENT   tableType,
    IN  GT_U32                lineIndex,
    IN  GT_U32               *entryValuePtr
);

/**
* @internal prvCpssDxChPortGroupShadowLineUpdateMasked function
* @endinternal
*
* @brief   Either write a whole entry (if mask is not specified) into PP's shadow
*         DB table at specific 'line index' or update the shadow table entry
*         according to the specified mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] tableType                - the 'HW table'
* @param[in] lineIndex                - the line index in the 'HW table'
*                                      (not the same index as the functions of 'table engine'
*                                      get (like prvCpssDxChPortGroupWriteTableEntry))
* @param[in] entryValuePtr            - (pointer to) an array containing the data to be
*                                      copied into the shadow
* @param[in] entryMaskPtr             - (pointer to) array of mask.
*                                      If NULL - the entry specified by entryValuePtr will
*                                      be written to shadow.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in HW entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGroupShadowLineUpdateMasked
(
    IN GT_U8                 devNum,
    IN GT_U32                portGroupId,
    IN CPSS_DXCH_TABLE_ENT   tableType,
    IN GT_U32                lineIndex,
    IN GT_U32               *entryValuePtr,
    IN GT_U32               *entryMaskPtr
);

/**
* @internal prvCpssDxChDiagDataIntegrityObjInit function
* @endinternal
*
* @brief   Initialize diagnostic service function pointers.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[in] devNum                   - the device number
*                                       none.
*/
GT_VOID prvCpssDxChDiagDataIntegrityObjInit
(
    IN  GT_U8     devNum
);

/**
* @internal prvCpssDxChDiagFalconDataIntegrityDbPointerSet function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size for specific DFX instance.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2
*
* @param[in] devNum                - device number
* @param[in] dfxInstanceType       - DFX instance type
* @param[out] dbArrayPtrPtr        - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr   - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChDiagFalconDataIntegrityDbPointerSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT   dfxInstanceType,
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC const **dbArrayPtrPtr,
    OUT GT_U32                                      *dbArrayEntryNumPtr
);

/**
* @internal prvCpssDxChDiagFalconBistCheckSkipOptionalClient function
* @endinternal
*
* @brief  Check skip of optional DFX client for BIST for specific DFX type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;
*                                  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - PP device number
* @param[in] dfxInstanceType       - DFX instance type
* @param[in] pipe                  - DFX pipe number
* @param[in] client                - DFX client number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagFalconBistCheckSkipOptionalClient
(
    IN  GT_U8       devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType,
    IN  GT_U32      pipe,
    IN  GT_U32      client
);

/**
* @internal prvCpssDxChDiagDataIntegrityPortGroupTcamParityDaemonEnableSet function
* @endinternal
*
* @brief   Function enables/disables TCAM parity daemon.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] enable                   - GT_TRUE - enable daemon
*                                      GT_FALSE - disable daemon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChDiagDataIntegrityPortGroupTcamParityDaemonEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_BOOL             enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChDiagh */

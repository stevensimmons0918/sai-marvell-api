/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssDxChPacketAnalyzer.h
*
* @brief Packet Analyzer APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPacketAnalyzerh
#define __cpssDxChPacketAnalyzerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzerTypes.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/**
* @internal cpssDxChPacketAnalyzerManagerCreate function
* @endinternal
*
* @brief   Create Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
* @retval GT_BAD_PTR               - on NULL pointer value.
*/
GT_STATUS cpssDxChPacketAnalyzerManagerCreate
(
    IN  GT_U32                                              managerId
);

/**
* @internal cpssDxChPacketAnalyzerManagerDelete function
* @endinternal
*
* @brief   Delete Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDelete
(
    IN  GT_U32                                  managerId
);

/**
* @internal cpssDxChPacketAnalyzerManagerDeviceAdd function
* @endinternal
*
* @brief   Add device to Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_FULL                  - if array is full
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDeviceAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
);

/**
* @internal cpssDxChPacketAnalyzerManagerDeviceRemove function
* @endinternal
*
* @brief   Remove device from Packet Trace Manager.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDeviceRemove
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
);

/**
* @internal cpssDxChPacketAnalyzerManagerEnableSet function
* @endinternal
*
* @brief   Enable/Disable the packet analyzer on all devices
*          added to manager id.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] enable                   - enable/disable PA on
*                                       manager id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerManagerEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPacketAnalyzerManagerDevicesGet function
* @endinternal
*
* @brief   Get packet analyzer manager information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[out] deviceStatePtr          - (pointer to) whether PA
*                                       is enabled for manager
*                                       id's devices
* @param[in,out] numOfDevicesPtr      - in: (pointer to)
*                                       allocated number of
*                                       devices for managerId
*                                       out: (pointer to) actual
*                                       number of devices for
*                                       managerId
* @param[out] devicesArr              - devices array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerManagerDevicesGet
(
    IN    GT_U32                                            managerId,
    OUT   GT_BOOL                                           *deviceStatePtr,
    INOUT GT_U32                                            *numOfDevicesPtr,
    OUT   GT_U8                                             devicesArr[] /*arrSizeVarName=numOfDevicesPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerManagerResetToDefaults function
* @endinternal
*
* @brief   Reset Packet Trace Manager configuration to default
*          settings.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS cpssDxChPacketAnalyzerManagerResetToDefaults
(
    IN  GT_U32                                  managerId
);

/**
* @internal cpssDxChPacketAnalyzerFieldSizeGet function
* @endinternal
*
* @brief   Get packet analyzer field's size.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] fieldName                - field name
* @param[out] lengthPtr               - (pointer to) fields
*                                       length in bits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                      managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName,
    OUT GT_U32                                     *lengthPtr
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyCreate function
* @endinternal
*
* @brief   Create logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] keyAttrPtr               - (pointer to) key attributes
* @param[in] numOfStages              - number of stages
* @param[in] stagesArr                - (pointer to) applicable
*                                       stages list array
* @param[in] fieldMode                - field mode assignment
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
* @retval GT_NOT_FOUND             - not found manager
* @retval GT_FULL                  - if array is full
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyCreate
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    IN  GT_U32                                              numOfStages,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stagesArr[], /*arrSizeVarName=numOfStages*/
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT fieldMode,
    IN  GT_U32                                              numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 fieldsArr[] /*arrSizeVarName=numOfFields*/
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyFieldsAdd function
* @endinternal
*
* @brief   Add fields to existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[] /*arrSizeVarName=numOfFields*/
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyFieldsRemove function
* @endinternal
*
* @brief   Remove fields from existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields
* @param[in] fieldsArr                - (pointer to) fields list
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerLogicalKeyStagesGet,
*       cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsRemove
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[] /*arrSizeVarName=numOfFields*/
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyDelete function
* @endinternal
*
* @brief   Delete existing logical key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyInfoGet function
* @endinternal
*
* @brief   Get existing logical key configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[out] keyAttrPtr              - (pointer to) key
*                                       attributes
* @param[out] fieldModePtr            - (pointer to) field
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyInfoGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    OUT CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT *fieldModePtr
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyStagesGet function
* @endinternal
*
* @brief   Get existing logical key stages list.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in,out] numOfStagesPtr       - in: (pointer to)
*                                       allocated number of
*                                       stages
*                                       out: (pointer to) actual
*                                       number of stages
* @param[out] stagesArr               - out: (pointer to) stages
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyStagesGet
(
    IN  GT_U32                                              managerId,
    IN    GT_U32                                            keyId,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[] /*arrSizeVarName=numOfStagesPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet function
* @endinternal
*
* @brief   Get existing logical key fields list per stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
* @param[in] stagesId                 - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - out: (pointer to) field
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on wrong state
*/
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerStagesGet function
* @endinternal
*
* @brief   Get applicable stages and Valid stages - stages that are not muxed for current configuration
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in,out] numOfApplicStagesPtr - in: (pointer to)
*                                       size of allocated applicStagesListArr
*                                       out: (pointer to) actual
*                                       number of applicable stages in applicStagesListArr
* @param[out] applicStagesListArr     - (pointer to) applicable stages list array
* @param[in,out] numOfValidStagesPtr  - in: (pointer to)
*                                       size of allocated ValidStagesListArr
*                                       out: (pointer to) actual
*                                       number of valid stages in ValidStagesListArr
* @param[out] ValidStagesListArr      - (pointer to) valid stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerStagesGet
(
    IN    GT_U32                                         managerId,
    INOUT GT_U32                                         *numOfApplicStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    applicStagesListArr[], /*arrSizeVarName=numOfApplicStagesPtr*/
    INOUT GT_U32                                         *numOfValidStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    validStagesListArr[] /*arrSizeVarName=numOfValidStagesPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerMuxStagesGet function
* @endinternal
*
* @brief   Get mux stages array for specific stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in,out] numOfMuxStagesPtr    - in: (pointer to)
*                                       allocated number of
*                                       muxed stages
*                                       out: (pointer to) actual
*                                       number of muxed
*                                       stages
* @param[out] muxStagesListArr        - (pointer to) muxed
*                                       stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerMuxStagesGet
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId,
    INOUT GT_U32                                         *numOfMuxStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    muxStagesListArr[] /*arrSizeVarName=numOfMuxStagesPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerGroupCreate function
* @endinternal
*
* @brief   Create packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] groupAttrPtr             - (pointer to) group
*                                       attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS cpssDxChPacketAnalyzerGroupCreate
(
    IN GT_U32                                               managerId,
    IN GT_U32                                               groupId,
    IN CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC       *groupAttrPtr
);

/**
* @internal cpssDxChPacketAnalyzerGroupDelete function
* @endinternal
*
* @brief   Delete packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found group
*/
GT_STATUS cpssDxChPacketAnalyzerGroupDelete
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   groupId
);

/**
* @internal cpssDxChPacketAnalyzerGroupRuleAdd function
* @endinternal
*
* @brief   Add packet analyzer rule to packet analyzer group.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] ruleAttrPtr              - (pointer to) rule
*                                       attributes
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
* @retval GT_FULL                  - if array is full
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleAdd
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          groupId,
    IN  GT_U32                                          ruleId,
    IN  CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC   *ruleAttrPtr,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[], /*arrSizeVarName=numOfFields*/
    IN  GT_U32                                          actionId
);

/**
* @internal cpssDxChPacketAnalyzerGroupRuleUpdate function
* @endinternal
*
* @brief   Replace content of packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
* @param[in] actionId                 - rule action identification
*                                       (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId,
    IN  GT_U32                                      numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   fieldsValueArr[], /*arrSizeVarName=numOfFields*/
    IN  GT_U32                                      actionId
);

/**
* @internal cpssDxChPacketAnalyzerGroupRuleDelete function
* @endinternal
*
* @brief   Delete packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found group or rule
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleDelete
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
);

/**
* @internal cpssDxChPacketAnalyzerGroupRuleGet function
* @endinternal
*
* @brief   Get packet analyzer rule attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[out] keyIdPtr                - (pointer to) logical
*                                       key identification
* @param[out] ruleAttrPtr             - (pointer to) rule
*                                       attributes
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for rule
*                                       identification
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       rule identification
* @param[out] fieldsValueArr          - rule content array
*                                       (Data and Mask)
* @param[out] actionIdPtr             - (pointer to) rule action
*                                       identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or rule
*/
GT_STATUS cpssDxChPacketAnalyzerGroupRuleGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    OUT   GT_U32                                        *keyIdPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC *ruleAttrPtr,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     fieldsValueArr[], /*arrSizeVarName=numOfFieldsPtr*/
    OUT   GT_U32                                        *actionIdPtr
);

/**
* @internal cpssDxChPacketAnalyzerActionCreate function
* @endinternal
*
* @brief   Create packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[in] actionPtr             - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS cpssDxChPacketAnalyzerActionCreate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
);

/**
* @internal cpssDxChPacketAnalyzerActionUpdate function
* @endinternal
*
* @brief   Update packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[in] actionPtr             - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
* @retval GT_BAD_PTR               - on NULL pointer value
*/
GT_STATUS cpssDxChPacketAnalyzerActionUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
);

/**
* @internal cpssDxChPacketAnalyzerActionDelete function
* @endinternal
*
* @brief   Delete packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - not found action
*
*/
GT_STATUS cpssDxChPacketAnalyzerActionDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  actionId
);

/**
* @internal cpssDxChPacketAnalyzerActionGet function
* @endinternal
*
* @brief   Get packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId             - manager identification
*                                    (APPLICABLE RANGES: 1..10)
* @param[in] actionId              - action identification
*                                   (APPLICABLE RANGES: 1..128)
* @param[out] actionPtr            - (pointer to) action
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found action
*/
GT_STATUS cpssDxChPacketAnalyzerActionGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    OUT  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC       *actionPtr
);

/**
* @internal cpssDxChPacketAnalyzerSampledDataCountersClear function
* @endinternal
*
* @brief   Clear Sampling Data and Counters for packet analyzer rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - on not found group
*/
GT_STATUS cpssDxChPacketAnalyzerSampledDataCountersClear
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
);

/**
* @internal cpssDxChPacketAnalyzerActionSamplingEnableSet function
* @endinternal
*
* @brief   Enable/disable sampling for packet analyzer action.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] actionId                 - action identification
*                                       (APPLICABLE RANGES: 1..128)
* @param[in] enable                   - enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found action
*/
GT_STATUS cpssDxChPacketAnalyzerActionSamplingEnableSet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  GT_BOOL                                     enable
);

/**
* @internal cpssDxChPacketAnalyzerGroupActivateEnableSet function
* @endinternal
*
* @brief   Enable/disable packet analyzer group activation.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] enable                   - enable/disable group activation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - on not found group
* @retval GT_BAD_STATE             - on invalid parameter
*
* @note in order to see the results use APIs:
*       cpssDxChPacketAnalyzerRuleMatchStagesGet,cpssDxChPacketAnalyzerRuleMatchDataGet,
*       cpssDxChPacketAnalyzerRuleMatchDataAllFieldsGet
*/
GT_STATUS cpssDxChPacketAnalyzerGroupActivateEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  groupId,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPacketAnalyzerRuleMatchStagesGet function
* @endinternal
*
* @brief   Get stages where packet analyzer rule was matched.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in,out] numOfMatchedStagesPtr- in: (pointer to)
*                                        allocated number of
*                                        matched stages
*                                        out: (pointer to)
*                                        actual number of
*                                        matched stages
* @param[out] matchedStagesArr         - (pointer to) matched
*                                        stages list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPacketAnalyzerRuleMatchStagesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U32                                            ruleId,
    IN    GT_U32                                            groupId,
    INOUT GT_U32                                            *numOfMatchedStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       matchedStagesArr[] /*arrSizeVarName=numOfMatchedStagesPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerStageMatchDataGet function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[in,out] sampleFieldsValueArr - in: (pointer to)
*                                       requested fields for
*                                       sample data array
*                                       out: (pointer to) sample
*                                       data array values for
*                                       requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    INOUT CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
);

/**
* @internal cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          packet analyzer fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
);

/**
* @internal cpssDxChPacketAnalyzerStageFieldsGet function
* @endinternal
*
* @brief   Get stage fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - (pointer to) stage
*                                       fields list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
);

/**
* @internal
*           cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
);

/**
* @internal
*           cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet
*           function
* @endinternal
*
* @brief   Get packet analyzer stage match attributes for all
*          internal fields by specific search attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet
(
    IN    GT_U32                                                managerId,
    IN    GT_U32                                                ruleId,
    IN    GT_U32                                                groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC        *searchAttributePtr,
    OUT   GT_U32                                                *numOfHitsPtr,
    INOUT GT_U32                                                *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC    sampleFieldsValueArr[]
);

/**
* @internal cpssDxChPacketAnalyzerFieldStagesGet function
* @endinternal
*
* @brief   Get field's stage list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] fieldName                - field name
* @param[in,out] numOfStagesPtr       - in: (pointer to)
*                                       allocated number of
*                                       stages
*                                       out: (pointer to) actual
*                                       number of stages
* @param[out] stagesArr               - (pointer to)
*                                       field's stage list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - on not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerFieldStagesGet
(
    IN    GT_U32                                            managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldName,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[]/*arrSizeVarName=numOfStagesPtr*/
);

/**
* @internal cpssDxChPacketAnalyzerMuxStageBind function
* @endinternal
*
* @brief   Set stage that will be valid in group of muxed stages.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerMuxStageBind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
);

/**
* @internal cpssDxChPacketAnalyzerMuxStageUnbind function
* @endinternal
*
* @brief   Set stage that will be invalid in group of muxed
*          stages.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - not found manager
*/
GT_STATUS cpssDxChPacketAnalyzerMuxStageUnbind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldAdd function
* @endinternal
*
* @brief   Add packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
* @param[in] fieldNamePtr             - (pointer to) field name
* @param[out] udfIdPtr                - (pointer to)user defined
*                                       field identification.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldAdd
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             *udfIdPtr
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldDelete
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC     *udfAttrPtr
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldGet function
* @endinternal
*
* @brief   Get packet analyzer user defined field.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfAttrPtr               - (pointer to) udf attributes
* @param[out] udfIdPtr                - (pointer to) user
*                                       defined field identification
* @param[out] validPtr                - (pointer to) whether udf
*                                       entry is valid or not
* @param[out] fieldNamePtr            - (pointer to) field name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             *udfIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedFieldInfoGet
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined field information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udfId                    - user defined field
*                                       identification
* @param[out] validPtr                - (pointer to) whether udf
*                                       entry is valid or not
* @param[out] udfAttrPtr              - (pointer to) udf attributes.
*                                       Valid only when validPtr == GT_TRUE
* @param[out] fieldNamePtr            - (pointer to) field name.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldInfoGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             udfId,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC    *udfAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC        *fieldNamePtr
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageAdd function
* @endinternal
*
* @brief   Add packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
* @param[in] interfaceAttrPtr         - (pointer to) interface
*                                       attributes
* @param[out] udsIdPtr                - (pointer to) user
*                                       defined stage identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageAdd
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr,
    IN CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    *interfaceAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageDelete
*           function
* @endinternal
*
* @brief   Delete packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageDelete
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr
);

/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageGet function
* @endinternal
*
* @brief   Get packet analyzer user defined stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsAttrPtr               - (pointer to) uds attributes
* @param[out] udsIdPtr                - (pointer to) user
*                                       defined stage identification
* @param[out] validPtr                - (pointer to) whether uds
*                                       entry is valid or not
* @param[out] interfaceAttrPtr        - (pointer to) interface attributes.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
);


/**
* @internal cpssDxChPacketAnalyzerUserDefinedStageInfoGet
*           function
* @endinternal
*
* @brief   Get packet analyzeruser defined stages information.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsId                    - user defined stage
*                                       identification
* @param[out] validPtr                - (pointer to) whether uds
*                                       entry is valid or not
* @param[out] udsAttrPtr              - (pointer to) uds attributes.
*                                       Valid only when validPtr == GT_TRUE
* @param[out] interfaceAttrPtr        - (pointer to) interface attributes.
*                                       Valid only when validPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
*/
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageInfoGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     udsId,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
);

/**
* @internal cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet function
* @endinternal
*
* @brief   Get packet analyzer rule match attributes for all packet analyzer fields by specific search attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] ruleId                   - rule identification
*                                       (APPLICABLE RANGES: 1..2048)
* @param[in] groupId                  - group identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] stageId                  - stage identification
* @param[in] searchAttributePtr        -(pointer to) search attributes.
* @param[out] numOfHitsPtr            - (pointer to) number
*                                       of hits
* @param[in,out] numOfSampleFieldsPtr - in: (pointer to)
*                                       allocated number of
*                                       fields for sample data
*                                       array
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       sample data array
* @param[out] sampleFieldsValueArr    - out: (pointer to)
*                                       sample data array values
*                                       for requested fields
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet
(
    IN    GT_U32                                           managerId,
    IN    GT_U32                                           ruleId,
    IN    GT_U32                                           groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  *searchAttributePtr,
    OUT   GT_U32                                          *numOfHitsPtr,
    INOUT GT_U32                                          *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC        sampleFieldsValueArr[]
);

/**
* @internal
*           cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
*           function
* @endinternal
*
* @brief   Get packet analyzer overlapping fields for spesific
*          field in stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] stageId                  - stage identification
* @param[in] fieldName                - field name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - out: (pointer to) fields
*                                       list array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on not found manager
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*/
GT_STATUS cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[] /*arrSizeVarName=numOfFieldsPtr*/
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPacketAnalyzercketAnalyzerh */

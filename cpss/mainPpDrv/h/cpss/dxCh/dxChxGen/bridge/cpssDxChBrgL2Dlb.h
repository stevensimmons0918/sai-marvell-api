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
* @file cpssDxChBrgL2Dlb.h
*
* @brief L2 DLB facility CPSS DxCh implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChBrgL2Dlbh
#define __cpssDxChBrgL2Dlbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

/* Max no of path supported by DLB */
#define CPSS_DXCH_BRG_L2_DLB_MAX_PATH 16

/**
* @struct CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC
 *
 * @brief DLB Path Utilization Table Entry
*/
typedef struct{
    /** target ePort */
    GT_PORT_NUM targetEport;

    /** target HW device */
    GT_HW_DEV_NUM targetHwDevice;

    /** Weight in percentage
       (APPLICABLE RANGE:0..100)*/
    GT_U32 weight;
} CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC;

/**
* @struct CPSS_DXCH_BRG_L2_DLB_ENTRY_STC
 *
 * @brief L2 DLB Entry.
*/
typedef struct{
    /** DLB Path-ID
       (APPLICABLE RANGES: 0..15)*/
    GT_U32 pathId;

    /** DLB Last Seen TimeStamp
       (APPLICABLE RANGES: 0..0x7FFFF)*/
    GT_U32 lastSeenTimeStamp;
} CPSS_DXCH_BRG_L2_DLB_ENTRY_STC;

/**
* @struct CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT
 *
 * @brief DLB Scale mode.
*/
typedef enum{
    /** 16 Paths: 512 destinations, 16 paths each*/
    CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E,

    /** 8 Paths: 1024 destinations, 8 paths each*/
    CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E,

    /** 4 Paths: 2048 destinations, 4 paths each*/
    CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E
}CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT;

/*
* @internal cpssDxChBrgL2DlbEnableSet function
* @endinternal
*
* @brief   Set the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum              - device number
* @param[in] enable              - DLB status:
*                                  GT_TRUE : DLB is enabled
*                                  GT_FALSE: DLB is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbEnableSet
(
    IN  GT_U8       devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgL2DlbEnableGet function
* @endinternal
*
* @brief   Get the global enable/disable Dynamic Load Balancing(DLB).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum              - device number
* @param[out] enablePtr          - (pointer to)DLB status:
*                                  GT_TRUE : DLB is enabled
*                                  GT_FALSE: DLB is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgL2DlbLttIndexBaseEportSet function
* @endinternal
*
* @brief  Defines the first ePort number in the L2 DLB ePort number range.
*         The index to the L2ECMP LTT Table is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] dlbIndexBaseEport     - the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbLttIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      dlbIndexBaseEport
);

/**
* @internal cpssDxChBrgL2DlbLttIndexBaseEportGet function
* @endinternal
*
* @brief  Get the first ePort number in the L2 DLB ePort number range.
*         The index to the L2 DLB LTT is <Target ePort>-<DLB LTT Base ePort>.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out]dlbIndexBaseEportPtr     - (pointer to)the ePort number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbLttIndexBaseEportGet
(
    IN GT_U8            devNum,
    OUT GT_PORT_NUM     *dlbIndexBaseEportPtr
);

/**
* @internal cpssDxChBrgL2DlbTableEntrySet function
* @endinternal
*
* @brief   Set L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry index
*
* @param[in] dlbEntryPtr           - (pointer to) L2 DLB entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Both ECMP and DLB feature use same ECMP Table
*       2. Index according to index from L2 ECMP LTT
*       3. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
GT_STATUS cpssDxChBrgL2DlbTableEntrySet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     *dlbEntryPtr
);

/**
* @internal cpssDxChBrgL2DlbTableEntryGet function
* @endinternal
*
* @brief   Get L2 DLB entry info.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry index
*
* @param[out] dlbEntryPtr          - (pointer to) L2 DLB entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Index according to index from L2 ECMP LTT
*       2. When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
*       access to all table entries.
*       Otherwise the CPSS allows access to ONLY to lower half of table entries.
*
*/
GT_STATUS cpssDxChBrgL2DlbTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     *dlbEntryPtr
);

/**
* @internal cpssDxChBrgL2DlbPathUtilizationScaleModeSet function
* @endinternal
*
* @brief  Configure the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] scaleMode                - path utilization scaling mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationScaleModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  scaleMode
);

/**
* @internal cpssDxChBrgL2DlbPathUtilizationScaleModeGet function
* @endinternal
*
* @brief  Get the path utilization scale mode for DLB.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] scaleModePtr            - (pointer to)path utilization scaling mode.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationScaleModeGet
(
    IN GT_U8                                 devNum,
    OUT CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  *scaleModePtr
);

/**
* @internal cpssDxChBrgL2DlbMinimumFlowletIpgSet function
* @endinternal
*
* @brief  Configure the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] minimumIpg            - Minimum Inter-packet Gap in micro-second.
*                                    APPLICABLE RANGES:0..0x7FFFF.

* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_OUT_OF_RANGE          -  parameter out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbMinimumFlowletIpgSet
(
    IN GT_U8    devNum,
    IN GT_U32   minimumIpg
);

/**
* @internal cpssDxChBrgL2DlbMinimumFlowletIpgGet function
* @endinternal
*
* @brief  Get the minimum inter-packet gap(IPG) for flowlet boundary identification.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[out] minimumIpgPtr        - Inter-packet Gap.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbMinimumFlowletIpgGet
(
    IN GT_U8                          devNum,
    OUT GT_U32                        *minimumIpgPtr
);

/**
* @internal cpssDxChBrgL2DlbPathUtilizationTableEntrySet function
* @endinternal
*
* @brief  Configure the Path Utilization Table entry(Target ePort/Device, Weight).
*         Path weight is assigned in proportion to the utilizable percentage of bandwidth
*         i.e 100% utilization percentage.
*                * Weight 100 equals full BW available
*                * Weight 50 is assigned for 50% available BM
*                * Weight 25 is assigned for available 25% BW
*                * Weight 0 is assigned for path which has no BW.
*         Relative weight is normalized to span the range [0,1024]
*         such that the sum of all the path weights is equal '1024'
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
* @param[in] index                 - the DLB entry Index.
* @param[in] dlbEntryArr           - DLB path utilization table entry array.
*                                    Array indexes are used based on path utilization scale mode as below.
*                                       512 x 16 path  - 16 indexes are used.
*                                       1K x 8 path    - Only first 8 indexes are used.
*                                       2K x 4 path    - Only first 4 indexes are used
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - parameter out of range.
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationTableEntrySet
(
    IN GT_U8                               devNum,
    IN GT_U32                              index,
    IN CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
);

/**
* @internal cpssDxChBrgL2DlbPathUtilizationTableEntryGet function
* @endinternal
*
* @brief  Get the Path Utilization Table entry(Target ePort/Device, Weight).
*         Path weight is assigned in proportion to the utilizable percentage of bandwidth
*         i.e 100% utilization percentage.
*                * Weight 100 equals full BW available
*                * Weight 50 is assigned for 50% available BM
*                * Weight 25 is assigned for available 25% BW
*                * Weight 0 is assigned for path which has no BW.
*         Relative weight is normalized to span the range [0,1024]
*         such that the sum of all the path weights is equal '1024'
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                 xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the DLB entry Index.
* @param[out] dlbEntryArr             - (pointer to) DLB path utilization table entry array.
*                                       Array indexes are used based on path utilization scale mode as below.
*                                       512 x 16 path  - 16 indexes are used.
*                                       1K x 8 path    - Only first 8 indexes are used.
*                                       2K x 4 path    - Only first 4 indexes are used
*
* @retval GT_OK                       - on success.
* @retval GT_BAD_PARAM                - on wrong input parameters
* @retval GT_HW_ERROR                 - failed to write to hardware
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbPathUtilizationTableEntryGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          index,
    OUT CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]/*maxArraySize=16*/
);

/**
* @internal cpssDxChBrgL2DlbWaLttInfoSet function
* @endinternal
*
* @brief  Updates WA logic with new or changed LTT entry information.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] lttIndex              - LTT Index.
* @param[in] secondaryDlbBaseIndex - Base index to the secondary region of DLB table.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbWaLttInfoSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               lttIndex,
    IN GT_U32                               secondaryDlbBaseIndex
);

/**
* @internal cpssDxChBrgL2DlbWaExecute function
* @endinternal
*
* @brief  Performs DLB WA.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgL2DlbWaExecute
(
    IN GT_U8                                devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgL2Dlbh */

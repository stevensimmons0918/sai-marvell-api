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
* @file prvCpssDxChVirtualTcamDb.h
*
* @brief The CPSS DXCH Virtual TCAM Manager internal batabase
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChVirtualTcamDb_h
#define __prvCpssDxChVirtualTcamDb_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpssCommon/private/prvCpssAvlTree.h>

extern void prvCpssDxChVirtualTcamDbSegmentTableException(void);

typedef enum PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_ENT
{
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_1_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_2_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_3_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_0_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_1_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_2_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_3_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_0_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_1_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_2_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_3_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_0_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_1_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_2_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_3_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_0_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_1_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_2_E,
    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_3_E,

    PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E
} PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_ENT;

/* Converts actual lookupId to lookupId Index for vTcamHaDb */
#define PRV_CPSS_DXCH_VTCAM_LOOKUPID_IDX(_lookupId)                              \
    switch (_lookupId)                                                           \
    {                                                                            \
        case ((0 << 8) | 0):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E;                \
            break;                                                               \
        case ((0 << 8) | 1):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_1_E;                \
            break;                                                               \
        case ((0 << 8) | 2):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_2_E;                \
            break;                                                               \
        case ((0 << 8) | 3):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_3_E;                \
            break;                                                               \
        case ((1 << 8) | 0):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_0_E;                \
            break;                                                               \
        case ((1 << 8) | 1):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_1_E;                \
            break;                                                               \
        case ((1 << 8) | 2):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_2_E;                \
            break;                                                               \
        case ((1 << 8) | 3):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_1_3_E;                \
            break;                                                               \
        case ((2 << 8) | 0):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_0_E;                \
            break;                                                               \
        case ((2 << 8) | 1):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_1_E;                \
            break;                                                               \
        case ((2 << 8) | 2):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_2_E;                \
            break;                                                               \
        case ((2 << 8) | 3):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_2_3_E;                \
            break;                                                               \
        case ((3 << 8) | 0):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_0_E;                \
            break;                                                               \
        case ((3 << 8) | 1):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_1_E;                \
            break;                                                               \
        case ((3 << 8) | 2):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_2_E;                \
            break;                                                               \
        case ((3 << 8) | 3):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_3_3_E;                \
            break;                                                               \
        case ((4 << 8) | 0):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_0_E;                \
            break;                                                               \
        case ((4 << 8) | 1):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_1_E;                \
            break;                                                               \
        case ((4 << 8) | 2):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_2_E;                \
            break;                                                               \
        case ((4 << 8) | 3):                                                     \
            _lookupId = PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_4_3_E;                \
            break;                                                               \
        default:                                                                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);       \
    }


/* Converts rule type to rule size value */
#define PRV1_CPSS_DXCH_TTI_CONVERT_RULE_TYPE_TO_TCAM_RULE_SIZE_VAL_MAC(_ruleSize, _ruleType)     \
    switch (_ruleType)                                                                      \
    {                                                                                       \
        case CPSS_DXCH_TTI_RULE_UDB_10_E:                                                   \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;                                    \
            break;                                                                          \
        case CPSS_DXCH_TTI_RULE_UDB_20_E:                                                   \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_20_B_E;                                    \
            break;                                                                          \
        case CPSS_DXCH_TTI_RULE_UDB_30_E:                                                   \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                    \
            break;                                                                          \
        default:                                                                            \
            _ruleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;                                    \
            break;                                                                          \
    }

/* HW index indicating 'not used' */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS   0xFFFFFFFF

typedef struct{
    GT_U32  oldLogicalIndex;
    GT_U32  newLogicalIndex;
}PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC;

/**********************************************************************************/
/* Common                                                                         */
/**********************************************************************************/

/**********************************************************************************/
/* Virtual TCAMs Segment Data and structures                                                        */
/**********************************************************************************/

/* This library should use lookup concept for pair of client group and hit number */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(client_group, hit_number) \
    ((client_group << 8) | hit_number)
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_CLIENT_GROUP_MAC(lookup_id) \
    (lookup_id >> 8)
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_HIT_NUMBER_MAC(lookup_id) \
    (lookup_id & 0xFF)

/* value for free space in segments table */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_VTAM_ID_FREE_SPACE_CNS 0xFFFFFFFF
/* value for free block in TCAM blocks table */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS 0xFFFFFFFF

/* amount of rules added / removed by autoresize */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DEFAULT_AUTO_RESIZE_GRANULARITY_CNS 12

/**
* @enum PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT
 *
 * @brief Column status of horizontal block row status index.
 * All supported states of part0 and part1.
*/
typedef enum
{
    /* may be named as _OOOOOO_E for SIP5, used also for legacy as _OOOO_*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_BUSY_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOOF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDFFFF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDOOOO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFOOOO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOODD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOOFF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDFF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDOO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFOO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFDDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OODDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOFFFF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFFFF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_OOOF_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDF_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDO_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFO_E,
    /*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E*/
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_DDDD_E,
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_FFFF_E,
    /* enum range values */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_LAST_E
} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT;

/**
* @struct PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC
 *
 * @brief This structure of horizontal block row DRAG status changing.
*/
typedef struct{

    /** @brief amount of such horizontal block rows.
     *  used as repeater when this structure is a part of Dragging Rule.
     */
    GT_U32 repeater;

    /** bitmask of allowed horizontal block indexes                                       */
    /** valid set of bitmaps requires that each bitmap is "a part of" or "not intersects" */
    /** with any of the next bitmaps  (bmp_i & bmp_j) == 0 || (bmp_i & bmp_j) == bmp_i    */
    GT_U32 horzBlockIdxMask;

    /** source status index. */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT source;

    /** @brief target status index.
     *  Comments:
     */
    PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_INDEX_ENT target;

} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC;

/**
* @struct PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC
 *
 * @brief This structure of horizontal block row DRAG status changing.
 * Dragging rule represents moving of some space used by dragged vTcam to
 * another place as changing status
 * status of an old place from "used by dragged vTcam" to "free" and
 * status of a new space from "used by dragged vTcam" to "free"
 * The amount of old space and new space should be the same.
*/
typedef struct{

    /** amount of change status elements. */
    GT_U32 changeArrSize;

    /** @brief (pointer to)array of change status elements.
     *  Comments:
     */
    const PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_STC* changeArr;

} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC;

typedef const PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_STC
    *PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR;

/**
* @struct PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_LIST_STC
 *
 * @brief List of rules.
*/
typedef struct
{
    /** pointer to array of rules */
    const PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_PTR *rulesArrPtr;
    /** amount of rules in array */
    GT_U32                                             rulesArrSize;
} PRV_SEG_DRAG_HORZ_BLOCK_ROW_STATUS_CHANGE_RULE_LIST_STC;

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_MAP_FUNC
 *
 * Description:
 *     This Function maps TCAM segment to lookup (i.e. pair of client_group and hit_number).
 *
 * INPUTS:
 *       cookiePtr      - device info passed to callback function.
 *       baseRow        - base row of mapped segment.
 *       baseColumn     - base column of mapped segment.
 *       lookupId       - id of lookup for mapping.
 *
 *  Comments:
 *
 */
typedef GT_STATUS PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_MAP_FUNC
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      baseRow,
    IN  GT_U32      baseColumn,
    IN  GT_U32      lookupId
);

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_UNMAP_FUNC
 *
 * Description:
 *     This Function unmaps TCAM segment from all lookups.
 *     Called when block becomes free. May be used for power reducing.
 *
 * INPUTS:
 *       cookiePtr      - device info passed to callback function.
 *       baseRow        - base row of mapped segment.
 *       baseColumn     - base column of mapped segment.
 *
 *  Comments:
 *
 */
typedef GT_STATUS PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_UNMAP_FUNC
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      baseRow,
    IN  GT_U32      baseColumn
);

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_RULE_INVALIDATE_FUNC
 *
 * Description:
 *     This Function invalidates rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr            - device info passed to callback function.
 *       rulePhysicalIndex    - rule physical index.
 *       ruleSize             - rule size in TCAM minimal rules (10 bytes)
 *
 *  Comments:
 *
 */
typedef GT_STATUS PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_RULE_INVALIDATE_FUNC
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      rulePhysicalIndex,
    IN  GT_U32      ruleSize
);

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_RULE_MOVE_FUNC
 *
 * Description:
 *     This Function moves rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr               - device info passed to callback function.
 *       srcRulePhysicalIndex    - source rule physical index.
 *       dstRulePhysicalIndex    - destination rule physical index.
 *       ruleSize                - rule size in TCAM minimal rules (10 bytes)
 *
 *  Comments:
 *
 */
typedef GT_STATUS PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_RULE_MOVE_FUNC
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      srcRulePhysicalIndex,
    IN  GT_U32      dstRulePhysicalIndex,
    IN  GT_U32      ruleSize
);

/**
* @enum PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT
 *
 * @brief Class of device.
*/
typedef enum{

    /** PCL TCAM on legacy device */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E,

    /** SIP5 device (shared PCL/TTI TCAM) */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E,

    /** SIP6_10 device (shared PCL/TTI TCAM), different from SIP5 only     */
    /** that does not support 10-byte rules with even indexed TCAM entries */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E,

    /** TTI TCAM on legacy device */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E,

    /** PCL TCAM1 on AC5 device */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E,

    /** device has no TCAM represented by structure containing this value */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_INVALID_E
} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT;

/**
* @enum PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT
 *
 * @brief Columns covered by TCAM memory segment (in block scope).
*/
typedef enum{

    /** @brief no columns used (for parallel horizontal blocks)
     *  values for SIP5 devices
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E  = 0,

    /** column 5, remainder after 50-byte rule */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E,

    /** columns 0..1 high part of 80-byte rule
     *  for SIP6_10 also reminder before 40-byte rule aligned to bank-8 (odd block)
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E,

    /** columns 4..5, remainder after 40-byte rule
     * for SIP6_10 only remainder after 40-byte rule aligned to bank-0 (even block)
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E,

    /** columns 0..3, 40-byte rule
     * for SIP6_10 only 40-byte rule aligned to bank-0 (even block)
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E,

    /** columns 2..5 remainder after 80-byte rule
     * for SIP6_10 also 40-byte rule aligned to bank-8 (odd block)
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E,

    /** columns 0..4, 50-byte rule */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E,

    /** @brief all columns (0..5), 10/20/30/60-byte rule
     *  values for legacy devices, all must be recognized as legacy
     *  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E,  - column 3, remainder after 80-byte rule
     *  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E, - columns 0..2 80-byte rule
     *  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E, - all columns (0..3), 30/60-byte rule
     *  values for TTI TCAM on legacy devices, all must be recognized as legacy
     *  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E, - only column, 30-byte rule
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E  ,

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E,

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E,

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E,

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E,

    /** amount of members in this ENUM */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E,

    /* shifted values freqiently used as expressions */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_5_E      = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E     << 4),
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_01_E     = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E    << 4),
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_45_E     = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E    << 4),
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_0123_E   = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E  << 4),
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_2345_E   = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E  << 4),
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_01234_E  = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E << 4),
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E    = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E   << 4)
                                                       
} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT;       

/* bitmaps of 10-byte columns in one horizontal block */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_BITMAPS_MAC                 \
    {                                                                         \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E        */ 0,      \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E           */ 0x20,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E          */ 0x03,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E          */ 0x30,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E        */ 0x0F,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E        */ 0x3C,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E       */ 0x1F,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E         */ 0x3F,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_3_E    */ 8,      \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_012_E  */ 7,      \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E  */ 0x0F,   \
        /* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E  */ 0x01    \
    }

/* Forward declaration */
struct  tag_PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC;

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC
 *
 * @brief This structure of DB segments block state
*/
typedef struct{

    /** base column number. */
    GT_U32 columnsBase;

    /** base row number. */
    GT_U32 rowsBase;

    /** amount of columns. */
    GT_U32 columnsAmount;

    /** amount of rows. */
    GT_U32 rowsAmount;

    /** @brief lookup Id. (or constant for free block)
     *  Comments:
     */
    GT_U32 lookupId;

    /** For each type of free part of block row - amount of such free places in the block */
    GT_U32 freeSpace[PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E];

    /** amount of columns before SIP6_10 10-byte rules. */
    GT_U32 reservedColumnsSpace;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC
    *PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_PTR;

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC
 *
 * @brief This structure of DB segments table configuration
*/
typedef struct
{
    struct tag_PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *pVTcamMngPtr;
    GT_U32                                                      tcamColumnsAmount;
    GT_U32                                                      tcamRowsAmount;
    GT_U32                                                      lookupColumnAlignment;
    GT_U32                                                      lookupRowAlignment;
    GT_U32                                                      blocksAmount;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC        *blockInfArr;
    PRV_CPSS_AVL_TREE_ID                                        segFreePrefTree;
    PRV_CPSS_AVL_TREE_ID                                        segFreeLuTree;
    GT_VOID                                                     *cookiePtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_MAP_FUNC       *lookupMapFuncPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_UNMAP_FUNC     *lookupUnmapFuncPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_RULE_INVALIDATE_FUNC  *ruleInvalidateFuncPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_RULE_MOVE_FUNC        *ruleMoveFuncPtr;
    CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC                  *hwDbWriteFuncPtr; 
    CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC                   *hwDbReadFuncPtr; 
    GT_U32                                                      maxSegmentsInDb;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT             deviceClass;
} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC;

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC
 *
 * @brief This structure of DB segments table allocation request
*/
typedef struct{

    /** lookup Id (combined from client group and hit index). */
    GT_U32 lookupId;

    /** vTCAM Id (for vTCAM that will contain memory). */
    GT_U32 vTcamId;

    /** Amount of rules in vTCAM. */
    GT_U32 rulesAmount;

    /** rule start columns bitmap. */
    GT_U32 ruleStartColumnsBitmap;

    /** @brief rule Size.
     *  (APPLICABLE RANGES: 1..6)
     *  Comments:
     */
    GT_U32 ruleSize;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC;

/* width of bitmaps of 10-byte columns in one horizontal block */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_BITBAPS_WIDTH_CNS 6

/* map of horizontal blocks using in segment */
typedef GT_U32 PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP;

/* row bitmap empty for all horizontal blocks */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_EMPTY_CNS 0

/* Max supported amount of block columns */
/* this value can be increased up to 8, but yet not needed */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_MAX_SUPPORTED_CNS 2

/* row bitmap complement */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_COMPLEMENT_MAC(_x) \
    prvCpssDxChVirtualTcamDbSegmentTableSegColComplementGet(_x)

/* detect full row in block */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_IS_FULL_MAC(_x) \
    ((_x == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E)          \
    || (_x == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_ALL_E)          \
    || (_x == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LEGACY_TTI_ALL_E))

/* row bitmap of 10-byte columns by enum value */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_BITMAP_OF_COLUMNS_MAC(_enum, _bmp) \
    {static GT_U8 const __arr[] = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_BITMAPS_MAC; \
    _bmp = 0; if (_enum < sizeof(__arr)) {_bmp = __arr[_enum];} }

/* set and get SEG_COLUMNS to the ROW MAP */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(_rowMap,_horzBlock,_columns) \
    _rowMap = (_rowMap & (~ (0xF << (4 * (_horzBlock))))) | (_columns << (4 * (_horzBlock)))
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(_rowMap,_horzBlock) \
    ((_rowMap >> (4 * (_horzBlock)))& 0xF)
/* Get all SEG_COLUMNS starting from a certain hor. block */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SHIFT_MAC(_rowMap,_horzBlock) \
        (_rowMap >> (4 * (_horzBlock)))

/* find first Horizontal block in columns using map */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_FIRST_MAC(_rowMap,_horzBlock,_columns) \
    _columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;                                      \
    for (_horzBlock = 0;                                                                              \
        (_horzBlock < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_MAX_SUPPORTED_CNS);                   \
        _horzBlock++)                                                                                 \
    {                                                                                                 \
        _columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(                         \
            _rowMap, _horzBlock);                                                                     \
        if (_columns  != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)                            \
        {                                                                                             \
            break;                                                                                    \
        }                                                                                             \
    }

/* find last Horizontal block in columns using map */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_FIND_LAST_MAC(_rowMap,_horzBlock,_columns)  \
    _columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;                                      \
    for (_horzBlock = 0;                                                                              \
        (_horzBlock < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_MAX_SUPPORTED_CNS);                   \
        _horzBlock++)                                                                                 \
    {                                                                                                 \
        if (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SHIFT_MAC(_rowMap,(_horzBlock+1)) == 0)\
        {                                                                                             \
            _columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(                     \
                _rowMap, _horzBlock);                                                                 \
            break;                                                                                    \
        }                                                                                             \
    }

/* names for dump */
typedef char* PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_CHAR_PTR;

extern  const PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_CHAR_PTR
    prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E];

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC
 *
 * @brief This structure of DB segments table entry
*/
typedef struct{

    /** id of owning vTcam (or constant for free space) */
    GT_U32 vTcamId;

    /** first logical index of the segment */
    GT_U32 baseLogicalIndex;

    /** rules amount */
    GT_U32 rulesAmount;

    /** size of rule in columns */
    GT_U32 ruleSize;

    /** lookup Id. (Used to search block where to move) */
    GT_U32 lookupId;

    /** base row number. */
    GT_U32 rowsBase;

    /** amount of rows. */
    GT_U32 rowsAmount;

    /** @brief compressed array of enum values per horizontal block
     *  describing used space.
     *  Comments:
     */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC;

typedef PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC
    *PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR;

GT_BOOL prvCpssDxChVirtualTcamSegmentTableIsBlockFull
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfoPtr
);

GT_U32 prvCpssDxChVirtualTcamSegmentTableBlockFreeSpaceCapacityGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT       deviceClass,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC  *blockInfoPtr,
    IN  GT_U32                                                ruleSizeIn10byteUnits
);

/* whether a TCAM block is full or has some free space */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_MAC(_blockInfo)               \
    (prvCpssDxChVirtualTcamSegmentTableIsBlockFull(_blockInfo) != GT_FALSE)

/* whether a TCAM block has some free space suitable for a rule size */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_IS_FULL_BLOCK_FOR_RULE_SIZE_MAC(_deiceClass, _blockInfo,_ruleSize) \
    (prvCpssDxChVirtualTcamSegmentTableBlockFreeSpaceCapacityGet(_deiceClass, _blockInfo, _ruleSize) == 0)

/**********************************************************************************/
/* Virtual TCAMs CFG Table                                                        */
/**********************************************************************************/

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC
 *
 * @brief This structure of DB of vTCAMs.
*/
typedef struct{

    /** @brief structure with data for vTCAM create.
     *  tcamSegCfgPtr  - (pointer to) segment configuration information.
     */
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC tcamInfo;

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr;

    /** @brief last physicalIndex during config replay (HA)
     *  haLastPhysicalIndex
     */
    GT_U32 haLastConfigReplayPhysicalIndex;

    /** amount of rules */
    GT_U32 rulesAmount;

    /** @brief amount of used rules
     *  usedRulesBitmapArr  - (pointer to)array of 32-bit words containing
     *  1 (used) or 0 (unused) rules state per logical
     *  index.
     *  rulePhysicalIndexArr - (pointer to)array of physical indexes of rules.
     *  Used to fast conversion logical index to physical.
     *  Recalculated at any segment move.
     *  Reallocated on Resize.
     */
    GT_U32 usedRulesAmount;

    GT_U32 *usedRulesBitmapArr;

    /* size of usedRulesBitmapArr(number of 32-bit words) - will be used in HA reconstruction to track Auto Resize  */
    GT_U32 usedRulesBitmapArrSizeHa;

    GT_U16 *rulePhysicalIndexArr;

    /** @brief tree of segments of space in TCAM
     *  using PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC
     *  start of priority derive vTCAMs only
     */
    PRV_CPSS_AVL_TREE_ID segmentsTree;

    /** @brief head of Tree with key vTcamId+RuleId
     *  uses shared nodes pool ruleIdIdNodesPool
     *  used to convert ruleId to logical index
     *  using PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC
     */
    PRV_CPSS_AVL_TREE_ID ruleIdIdTree;

    /** @brief head of Tree with key vTcamId+logicalIndex
     *  uses shared nodes pool ruleIdIndexNodesPool
     *  used to convert logical index to ruleId
     *  using PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC
     */
    PRV_CPSS_AVL_TREE_ID ruleIdIndexTree;

    /** @brief head of Tree with key vTcamId+priority
     *  uses shared nodes pool priorityPriNodesPool
     *  used to convert priority to logical index
     *  using PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC
     */
    PRV_CPSS_AVL_TREE_ID priorityPriTree;

    /** @brief head of Tree with key vTcamId+priority
     *  used to convert logical index to priority
     *  using PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC
     */
    PRV_CPSS_AVL_TREE_ID priorityIndexTree;

    /** @brief For auto
     *  resizing is temporarily disabled.
     *  Used for bulk delete operations.
     *  end of priority derive vTCAMs only
     *  Comments:
     */
    GT_BOOL autoResizeDisable;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC
    *PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR;

/* bitmap of devices used by Vitrtual TCAM Manarer */
typedef GT_U32 PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP[PRV_CPSS_MAX_PP_DEVICES_CNS/32];

/* macro to return error when the tcam manager hold no devices */
#define  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(_vTcamMngId)  \
    if(0 == prvCpssDxChVirtualTcamDbVTcamMngGet(_vTcamMngId)->numOfDevices)    \
    {                                                                          \
       /* the vTcamMngDB[_vTcamMngId]->devsBitmap[] is all zero's */           \
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,                 \
        "The tcam manager hold no devices");                                   \
    }

/*
 * @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_STC
 *
 * @brief structure for storing rule info in HA DB.
 */
typedef struct PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_STC
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID   ruleId;                                            /* 0xFFFFF by default - will be filled when rule found */
    GT_U32                           priority;                                          /* Applicable for priority mode vTCAMs */
    GT_U32                           priority_logicalIndex;                             /* Applicable for priority mode vTCAMs */
    GT_U32                           physicalIndex;                                     /* physical index is the key for search */
    GT_U32                           action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS];    /* action */
    GT_U32                           mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];        /* mask in H/w format */
    GT_U32                           pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];     /* pattern in H/w format */
    GT_U32                           vTcamId;                                           /* 0xFFFFF by default - will be filled when rule found */
    GT_U32                           found;                                             /* default: 0(not found), 1(found) in some previous search */
} PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_STC*   PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR;

/*
 * @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_STC
 *
 * @brief Structure to store multiple rules' info corres. to a {rulesize, lookupId}.
 */
typedef struct PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_STC
{
    GT_U32                                            numRules;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR   rulesArr;
} PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_STC*   PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_PTR;

/*
 * @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC
 *
 * @brief Structure with buffers used to row format rule read/write.
 */
typedef struct
{
    /** Size of action in Bobcat2; Caelum; Bobcat3 is 8 words for TTI and PCL */
    GT_U32       rawEntryFormatOperation_action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS];
    /** Rule entry size in words - defined by Bobcat2; Caelum; Bobcat3 size 7 x 3 words for PCL. for TTI it's 8 words */
    GT_U32       rawEntryFormatOperation_mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    /** Rule entry size in words - defined by Bobcat2; Caelum; Bobcat3 size 7 x 3 words for PCL. for TTI it's 8 words */
    GT_U32       rawEntryFormatOperation_pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    /**  Rule entry is valid */
    GT_BOOL      rawEntryFormatOperation_valid;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC;

/*
 * Typedef: struct PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC
 *
 * Description:
 *     This structure describes Virtual TCAM Manager.
 *
 * Fields:
 *     vTcamMngId              - VTCAM Manager Id.
 *     devsBitmap              - bitmap of used devices
 *     numOfDevices            - number of devices in devsBitmap
 *     portGroupBmpArr         - array of bitmaps of port groups representing TCAMs in each device
 *     devFamily               - device family served by this vTcam manager instance
 *     tcamSegCfg              - configuration of segment manager
 *     pclTcam1SegCfg          - AC5 PCL TCAM1 configuration of segment manager
 *     tcamTtiSegCfg           - configuration of segment manager for TTI TCAM on legacy devices
 *     segmentsPool            - pool for segment structures
 *     segNodesPoolFreePref    - pool for nodes of segment allocation preferences tree
 *     segNodesPoolFreeLu      - pool for nodes of segment lookup ordered tree
 *     segNodesPool            - pool for nodes of segment access trees
 *                               (vTcam layout and temporary trees)
 *     -- rule Id table for priority driven vTCAMs only --
 *     ruleIdPool             - pool of Rule Id Table Entries
 *     ruleIdIdNodesPool      - pool of Nodes of Tree with key RuleId (Rule Id Table)
 *     ruleIdIndexNodesPool   - pool of Nodes of Tree with key logicalIndex (Rule Id Table)
 *     -- Priority table for priority driven vTCAMs only --
 *     priorityPool           - pool of Priority Table Entries
 *     priorityNodesPool      - pool of Nodes of Tree with key Priority (Priority Table)
 *     priorityIndexNodesPool - pool of Nodes of Tree with key logicalIndex (Priority Table)
 *       autoResizeGranularity  - Granularity (in rules) to be applied at auto-resize
 *                               events on vTCAMs belonging to this vTCAM manager.
 *                           (Rounded up to a whole TCAM row, default: 12 rules)
 *     ------------------------------------------------------------
 *     vTcamCfgArrSize - size of array of vTCAMs Infos (maximal amount of vTCAMs).
 *     vTcamCfgArr     - (pointer to)array of pointers to vTCAMs Infos.
 *     --------
 *      tcamNumEntriesInSmallestRuleSize - the number of rules that the tcam hold.
 *                          the 'units' are of 'rules of smallest size' that supported by the TCAM.
 *                          for example : sip5 : 10B rules
 *                                      xCat3 TTI/PCL : 24B rules
 *                          NOTE: this parameter allow BC2 device to be compatible
 *                                 with BOBK device with 1/2 the tcam size
 *  Comments:
 *
 */
typedef struct tag_PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC
{
    GT_U16                                                 poweredOnBlocksBitMap[CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS];
    GT_U32                                                 vTcamMngId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP             devsBitmap;
    GT_PORT_GROUPS_BMP                                     portGroupBmpArr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                                 numOfDevices;
    CPSS_PP_FAMILY_TYPE_ENT                                devFamily;
    /* segments related */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC          tcamSegCfg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC          pclTcam1SegCfg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC          tcamTtiSegCfg;
    CPSS_BM_POOL_ID                                        segmentsPool;
    CPSS_BM_POOL_ID                                        segNodesPoolFreePref;
    CPSS_BM_POOL_ID                                        segNodesPoolFreeLu;
    CPSS_BM_POOL_ID                                        segNodesPool;
    /* rule Id table */
    CPSS_BM_POOL_ID                                        ruleIdPool;
    CPSS_BM_POOL_ID                                        ruleIdIdNodesPool;
    CPSS_BM_POOL_ID                                        ruleIdIndexNodesPool;
    /* priority table */
    CPSS_BM_POOL_ID                                        priorityPool;
    CPSS_BM_POOL_ID                                        priorityPriNodesPool;
    CPSS_BM_POOL_ID                                        priorityIndexNodesPool;
    GT_U32                                                 autoResizeGranularity;
    /* vTCAMs related */
    GT_U32                                                 vTcamCfgPtrArrSize;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR            *vTcamCfgPtrArr;
    /* tcam num rules info */
    GT_U32                                                 tcamNumEntriesInSmallestRuleSize;
    /* Indicates if HA needs to be supported for this manager */
    GT_BOOL                                                haSupported;
    GT_U32                                                 haFeaturesSupportedBmp;
    /* TCAM Valid Rules HA DB - used for recreation of vTCAMs*/
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_STC             vTcamHaDb[CPSS_DXCH_TCAM_RULE_SIZE_80_B_E+1][PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E];
    /* debug statistics data */
    GT_U32                                                 ruleInvalidateCounter;
    GT_U32                                                 ruleMoveCounter;
    /* start time for time measuring */
    GT_U32                                                 startSeconds;
    GT_U32                                                 startNanoSeconds;
    /** HW access buffers */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC       hwBuffers;

    /* handles of dynamic created objects will be added */
} PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC;

/* typedef for pointer to structure */
typedef PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC
    *PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_PTR;

/* macro PRV_CPSS_VIRTUAL_TCAM_MNG_DEV_BMP_SET_MAC
    to set a device in bitmap of devices in the TCAM manager database

  devsBitmap - of type PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP
                device bitmap

  devNum - the device num to set in the device bitmap
*/
#define PRV_CPSS_VIRTUAL_TCAM_MNG_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    (devsBitmap)[(devNum)>>5] |= 1 << ((devNum) & 0x1f)

/* macro PRV_CPSS_VIRTUAL_TCAM_MNG_DEV_BMP_CLEAR_MAC
    to clear a device from the bitmap of devices in the TCAM manager database

  devsBitmap - of type PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEV_BITMAP
                device bitmap

  devNum - the device num to set in the device bitmap
*/
#define PRV_CPSS_VIRTUAL_TCAM_MNG_DEV_BMP_CLEAR_MAC(devsBitmap, devNum)   \
    (devsBitmap)[(devNum)>>5] &= ~(1 << ((devNum) & 0x1f))

/* macro PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC
    to check if device is set in the bitmap of devices in the TCAM manager database

  devsBitmap - of type PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEV_BITMAP
                device bitmap

  devNum - the device num to set in the device bitmap

  return 0 -- device not set in bmp
  return 1 -- device set in bmp
*/
#define PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    (((devsBitmap)[(devNum)>>5] & (1 << ((devNum) & 0x1f))) ? 1 : 0)

#define PRV_VTCAM_HA_MAX_ERR_CNS 20

typedef struct PRV_VTCAM_HA_ERR_DB_ENTRY_STC
{
    GT_U32           line;
    const GT_CHAR   *func;
    GT_STATUS        rc;
    const GT_CHAR   *info;

} PRV_VTCAM_HA_ERR_DB_ENTRY_STC;

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet function
* @endinternal
*
* @brief   Get the resize granularity - according to rule size and vTcam Mng. configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] enumRuleSize             - Enum Rule Size
*                                       Resize granularity in rules, 0 on wrong parameters
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT    enumRuleSize
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet function
* @endinternal
*
* @brief   Full rows element of columns enum depends on device type.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                       Full rows element of columns enum.
*/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvCpssDxChVirtualTcamDbSegmentTableFullRowColumnsGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableRuleAlignmentBitmapGet function
* @endinternal
*
* @brief  returns bitmap of columns that can contain start entry of rule with given size.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] deviceClass            - class of device.
* @param[in] ruleSize               - rule size measured in 10-byte units
*                                     amount of rules that can be placed in segment row.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableRuleAlignmentBitmapGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass,
    IN  GT_U32                                            ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableRuleEnumAlignmentBitmapGet function
* @endinternal
*
* @brief  returns bitmap of columns that can contain start entry of rule with given size.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] deviceClass            - class of device.
* @param[in] ruleSizeEnt            - rule size enum element.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableRuleEnumAlignmentBitmapGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT              ruleSizeEnt
);

/**
* @internal prvCpssDxChVirtualTcamSegmentRuleSizeToUnits function
* @endinternal
*
* @brief   Convert Rule Size from enum to units.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] deviceClass            - device class.
* @param[in] ruleSize               - enum ruleSize
* @retval -  rule size in the smallest device rule units, 0 at error.
*/
GT_U32 prvCpssDxChVirtualTcamSegmentRuleSizeToUnits
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT deviceClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits function
* @endinternal
*
* @brief   Convert Rule Size from enum to units.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - enum ruleSize
*                                       rule size in the smallest device rule units, 0 at error.
*/
GT_U32 prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamSegmentTableRuleSizeToEnum function
* @endinternal
*
* @brief   Convert Rule Size from units to enum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - units measured ruleSize
*                                       rule size in the enum values, 0 at error.
*/
CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT prvCpssDxChVirtualTcamSegmentTableRuleSizeToEnum
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN  GT_U32                                      ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment function
* @endinternal
*
* @brief   Convert Rule Size from enum to units.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - enum ruleSize
*                                       rule alignment in the smallest device rule units, 0 at error.
*/
GT_U32 prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC          *tcamSegCfgPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity function
* @endinternal
*
* @brief   Calculate rule amount granularity.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] enumRuleSize             - Enum Rule Size
*                                       Rules amount granularity, 0 on wrong parameters
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC  *tcamSegCfgPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT    enumRuleSize
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet function
* @endinternal
*
* @brief   Calculate one row capacity for horizontal block part of vTcam segment.
*         Not usable and not empty columns cause error tracing.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] deviceClass            - class of device.
* @param[in] horzBlockIdx           - horizontan index of block
* @param[in] segmentColumns         - segment columns in one horizontal block
* @param[in] size                   - rule  measured in minimal rule units
*                                       amount of rules that can be placed in segment columns.
* @retval - maximal amount of rules of given size that can be placed in given columns ot the block row
*
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableMaxRulesAmountPerColumnsTypeGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass,
    IN  GT_U32                                            horzBlockIdx,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     segmentColumns,
    IN  GT_U32                                            size
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableColumnsToUnitsAmount function
* @endinternal
*
* @brief   Get amount of units in row columns ENUM element.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] segmentColumns           - segment columns in one horizontal block
*                                       amount of units in segment columns.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableColumnsToUnitsAmount
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCountFreeSpaceInUnits function
* @endinternal
*
* @brief   Count free space for in minimal record units.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id
*                                       used to fast estimate possibility of allocations.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableCountFreeSpaceInUnits
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                             lookupId
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree function
* @endinternal
*
* @brief   Remove memory from free space DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] lookupId                 - lookup Id (needed when marking busy parts of free block)
* @param[in] rowsBase                 - rows base of memory removed from free DB
* @param[in] rowsAmount               - amount of rows of memory removed from free DB
* @param[in] segmentColumnsMap        - columns map used by memory removed from free DB
*
* @retval GT_OK                    - on success
* @retval other                    - on fail
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     GT_U32                                                     lookupId,
    IN     GT_U32                                                     rowsBase,
    IN     GT_U32                                                     rowsAmount,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP          segmentColumnsMap
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableSegColComplementGet function
* @endinternal
*
* @brief   returns comlement of segment colunm.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] segmentColumns           - segment columns
*                                       comlement of segment colunm.
*/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT prvCpssDxChVirtualTcamDbSegmentTableSegColComplementGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap function
* @endinternal
*
* @brief   Convert segment columns map to bitmap of minimal rule places.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamWidth                - number of columns in TCAM
* @param[in] horzBlockWidth           - amount of columns in horizontal block
* @param[in] segmentColumnsMap        - segment columns map
*                                       Calcuilated bitmap.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap
(
    IN  GT_U32                                            tcamWidth,
    IN  GT_U32                                            horzBlockWidth,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableRowCapacity function
* @endinternal
*
* @brief   Calculate one row capacity for horizontal block part of vTcam segment.
*         Not usable and not empty columns are valid, 0 returned.
*         80-byte rules supported for legacy devices only: size = 3.
*
* @param[in] deviceClass            - class of device.
* @param[in] horzBlockIdx           - Index of Horizontal Block (0..1), 0xFF - unknown.
* @param[in] rowType                - Segment columns set enum element.
* @param[in] ruleSize               - rule size in minimal entries.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       amount of rules that can be placed in segment columns.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableRowCapacity
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass,
    IN  GT_U32                                            horzBlockIdx,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT     rowType,
    IN  GT_U32                                            ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace function
* @endinternal
*
* @brief   Decrease or increase per block counters for segment removed from free space.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr                   - (pointer to) segment removed from free space.
* @param[in] decrease                 - GT_TRUE - decrease, GT_FALSE - increase
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr,
    IN     GT_BOOL                                                    decrease
);


/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment function
* @endinternal
*
* @brief   Chain free segment to trees.
*         Segment data in data pool, but not in both trees - will be added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr                   - (pointer to) segment being added to trees.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegment
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit function
* @endinternal
*
* @brief   Chain free segment to trees with horizontal splitting.
*         Segment data in data pool, but not in both trees - will be added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] segPtr                   - (pointer to) segment being added to trees.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableChainFreeSegmentWithSplit
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC            *segPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamCfgTableCreate function
* @endinternal
*
* @brief   Create vTcam Configuration Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] maxEntriesInDb           - maximal amount entries in DB.
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Allocation result stored to it.
*                                      .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamCfgTableCreate
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC       *pVTcamMngPtr,
    IN     GT_U32                                   maxEntriesInDb
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamCfgTableDelete function
* @endinternal
*
* @brief   Delete all vTcams and vTcam Configuration Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Freed meory Pointer set to NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamCfgTableDelete
(
    INOUT   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *pVTcamMngPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamCfgTableVTcamAdd function
* @endinternal
*
* @brief   Add new vTcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamInfoPtr              - (pointer to) vTCAM Info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamCfgTableVTcamAdd
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN  GT_U32                                              vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC                     *tcamInfoPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamCfgTableVTcamRemove function
* @endinternal
*
* @brief   Remove vTcam.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamCfgTableVTcamRemove
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr,
    IN  GT_U32                                              vTcamId
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamCfgTableVTcamGet function
* @endinternal
*
* @brief   Get vTcam Configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      tcamInfoPtr     - (pointer to) vTCAM Configuration
*
* @param[out] tcamCfgPtrPtr            - (pointer to)(pointer to) vTCAM Configuration
*                                      None.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamCfgTableVTcamGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr,
    IN   GT_U32                                              vTcamId,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         **tcamCfgPtrPtr
);

/**********************************************************************************/
/* segments Table (the alone feature for physical layout)                      */
/**********************************************************************************/

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCreate function
* @endinternal
*
* @brief   Create Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr         - (pointer to) VTCAM Manager structure
* @param[in] segTabCfgPtr             - (pointer to) segments table configuration for TCAM.
* @param[in] segTabTtiCfgPtr          - (pointer to) segments table configuration for TTI TCAM on legacy devices.
* @param[in] pclTcam1SegCfgPtr        - (pointer to) segments table configuration for PCL TCAM1 on AC5 devices.
* @param[in] fillSegFreeTrees         - skip the filling of segFreePrefTree,segFreeLuTree with buffers
*                                      from segmentsPool
* @param[in,out] pVTcamMngPtr         - (pointer to) VTCAM Manager structure
*                                      Keeps created objects handles.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCreate
(
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr,
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabCfgPtr,
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabTtiCfgPtr,
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *pclTcam1SegCfgPtr,
    IN    GT_BOOL                                              fillSegFreeTrees
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDelete function
* @endinternal
*
* @brief   Delete Segment Table for TCAM. (Free used memory)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Deleted objects handles set to NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDelete
(
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableCopy function
* @endinternal
*
* @brief   Copy Segment Table from source TCAM to target TCAM.
*         also copy ALL source vtcam segments in source TCAM manager to
*         target vtcam segments in target TCAM manager
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] srcVTcamMngPtr           - (pointer to) source VTCAM Manager structure
* @param[in,out] trgVTcamMngPtr           - (pointer to) target VTCAM Manager structure
* @param[in] allocSkeletonOnly        - indication to allocate only skeleton
* @param[in,out] trgVTcamMngPtr           - (pointer to) updated target VTCAM Manager structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - not enough resources
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableCopy
(
    IN    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *srcVTcamMngPtr,
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *trgVTcamMngPtr,
    IN    GT_BOOL                                             allocSkeletonOnly
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors function
* @endinternal
*
* @brief   Merge neighbor (horizontal and vertical) free space segments
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableFreeSpaceMergeNeighbors
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC    *tcamSegCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge function
* @endinternal
*
* @brief   Splits or merges lookup ordered segments to be vTCAM space.
*         Parts containing same rows (but different columns) merged to one segment.
*         Parts remaining after merge splitted to new segments.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - primary tree for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts function
* @endinternal
*
* @brief   Update ruleAmount field in all segments of the tree.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - segments tree.
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC             *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
);

/**
* @internal segmentTableVTcamBaseLogicalIndexUpdate function
* @endinternal
*
* @brief   update the baseLogicalIndex in the segments of the vtcam
*         and the rulesAmount of the vtcam
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                       None.
*/
void segmentTableVTcamBaseLogicalIndexUpdate
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate function
* @endinternal
*
* @brief   Generate table for conversion logical indexes to physical indexes of rules.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] treeId                   - tree of vTCAM segments.
*
* @param[out] rulePhysicalIndexArr     - buffer to fulfill by physical indexes
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     PRV_CPSS_AVL_TREE_ID                        treeId,
    OUT    GT_U16                                      *rulePhysicalIndexArr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableMoveRulesToNewLayout function
* @endinternal
*
* @brief   Generate table for conversion logical indexes to physical indexes of rules.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] rulesAmount              - amount of rules.
* @param[in] ruleSize                 - rule size measured in 10-byte rules
* @param[in] ruleValidBmpPtr          - (pointer to) bitmap of valid rules.
* @param[in] srcRulePhysicalIndexArr  - (pointer to) array of source rules physical indexes
* @param[in] dstRulePhysicalIndexArr  - (pointer to) array of destination rules physical indexes
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableMoveRulesToNewLayout
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                      rulesAmount,
    IN     GT_U32                                      ruleSize,
    IN     GT_U32                                      *ruleValidBmpPtr,
    IN     GT_U16                                      *srcRulePhysicalIndexArr,
    IN     GT_U16                                      *dstRulePhysicalIndexArr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDeleteWithVtcamsSegments function
* @endinternal
*
* @brief   1. delete TCAM manager that hold only 'segments table' .
*         2. delete all the vTCAMs of this TCAM manager (their 'segments table')
*         NOTE : DO NOT free the CPU memory of vTcamMngPtr ,
*         vTcamMngPtr->vTcamCfgPtrArr
*         vTcamCfgPtrArr[ii]
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngPtr              - (pointer to) the TCAM manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDeleteWithVtcamsSegments
(
    OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC   *vTcamMngPtr
);

/*******************************************************************************
* prvCpssDxChVirtualTcamDbAllocWithOnlySegmentTable
*
* DESCRIPTION:
*       allocate vtcam with segments tree
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngPtr  - (pointer to) the TCAM manager
*       vTcamId      - the vtcam ID to allocate for
*       tcamInfoPtr  - (pointer to) the vtcam info
*       allocSkeletonOnly   - indication to allocate only skeleton
* OUTPUTS:
*       vTcamMngPtr  - (pointer to) the TCAM manager , with the added vtcam
*                       allocation at index vTcamId.
*
* RETURNS :
*       pointer to allocated vtcam memory (on NULL)
*
* COMMENTS:
*
*******************************************************************************/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* prvCpssDxChVirtualTcamDbAllocWithOnlySegmentTable
(
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC      *vTcamMngPtr,
    IN GT_U32                                   vTcamId,
    IN CPSS_DXCH_VIRTUAL_TCAM_INFO_STC         *tcamInfoPtr,
    IN    GT_BOOL                               allocSkeletonOnly
);

/**
* @internal prvCpssDxChVirtualTcamDbBlocksMakeSpace function
* @endinternal
*
* @brief   Compresses vTcams of other lookups (i.e. different than allocRequestPtr->lookupId)
*         into fewer blocks, in order to free whole blocks so that allocRequestPtr will succeed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] requestRemainder         - requested amount of space (in rules of size allocRequestPtr->ruleSize)
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
GT_STATUS prvCpssDxChVirtualTcamDbBlocksMakeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     GT_U32                                                   requestRemainder
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate function
* @endinternal
*
* @brief   Allocate memory for Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC   *allocRequestPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamFree function
* @endinternal
*
* @brief   Free memory of Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      freed objects handlers set to NULL
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamFree
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC                *tcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine function
* @endinternal
*
* @brief   Combine segmentsTree of secondary Virtual TCAM into primary Virtual TCAM.
*         the function free any secondary resource related to segmentsTree that
*         not needed any more.
*         the function fully align the info in the tree of primaryTcamCfgPtr->segmentsTree
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] primaryTcamCfgPtr        - (pointer to)primary virtual TCAM structure
* @param[in] secondaryTcamCfgPtr      - (pointer to)secondary virtual TCAM structure
* @param[in,out] primaryTcamCfgPtr        - (pointer to)updated primary virtual TCAM structure
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *primaryTcamCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *secondaryTcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableDetachSpace function
* @endinternal
*
* @brief   the function detach the last rules from the vtcam and restore them into
*         'free' segments tree
*         the function may remove one or more segments that hold the rules range.
*         the function may split one more segment to reduce number of rules in it.
*         NOTE: updates the vTcamInfoPtr->rulesAmount according to actual number of
*         rules that can be used by the vtcam after the operation.
*         this value may be more than numOfRulesToKeep due to rules granularity
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in,out] vTcamCfgPtr              - (pointer to) virtual TCAM structure
* @param[in] fromEndOrStart           - the type of direction
*                                      GT_TRUE  - hole from end   of range
*                                      GT_FALSE - hole from start of range
* @param[in] numOfRulesToKeep         - number of rules that need to keep , so may release
*                                      what ever is more than that.
*                                      NOTE: there may be 'align' issues that may release
*                                      a little less rules.
* @param[in,out] vTcamCfgPtr         - (pointer to)updated virtual TCAM structure
* @param[out] numOfDetachedRulesPtr  - (pointer to)number of detached rules
*                                      pointer may be NULL (if result not needed)
*                                      number of detached rules can be less than expected
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - the given tree has not been initialized.
* @retval GT_BAD_STATE             - In case the tree is not empty.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableDetachSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *vTcamCfgPtr,
    IN     GT_BOOL                                      fromEndOrStart,
    IN     GT_U32                                       numOfRulesToKeep,
    OUT    GT_U32                                       *numOfDetachedRulesPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableAllTcamCapacity function
* @endinternal
*
* @brief   Get Maximal amount of rules that can be placed in all TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - rule size enum element.
*                                       amount of rules that can be placed in all TCAM.
*/
GT_U32 prvCpssDxChVirtualTcamDbSegmentTableAllTcamCapacity
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize
);

/**
* @internal prvCpssDxChVirtualTcamDbBlocksReservedSpaceRecalculate function
* @endinternal
*
* @brief   Recalculates spaces reserved as even columns before SIP6_10 10-byte rules.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] clientGroup              - clientGroup of blocks to update
* @param[in] hitNumber                - hitNumber of blocks to update
*/
GT_VOID prvCpssDxChVirtualTcamDbBlocksReservedSpaceRecalculate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    IN     GT_U32                                        clientGroup,
    IN     GT_U32                                        hitNumber
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamSizeGet function
* @endinternal
*
* @brief   Get Size of Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
*
* @param[out] sizePtr                  - (pointer to) size of vTCAM in rules
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamSizeGet
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC             *pVTcamMngPtr,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC    *tcamCfgPtr,
    OUT GT_U32                                         *sizePtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableVTcamResize function
* @endinternal
*
* @brief   Resise Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] toInsert                 - GT_TRUE - insert rules, GT_FALSE - delete rules
*                                      logicalIndex            - logical index of rule.
* @param[in] baseRuleLogicalIndex     - logical index of base rule.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTableVTcamResize
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC             *pVTcamMngPtr,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC    *tcamCfgPtr,
    IN  GT_BOOL                                        toInsert,
    IN  GT_U32                                         baseRuleLogicalIndex,
    IN  GT_U32                                         rulesAmount
);

/**********************************************************************************/
/* Rule Id Table                                                                  */
/**********************************************************************************/

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC
 *
 * @brief This structure of DB Rule Id Entry
*/
typedef struct{

    /** rule id, */
    GT_U32 ruleId;

    /** @brief logical Index.
     *  applicationDataPtr  - application data pointer.
     *  Comments:
     */
    GT_U32 logicalIndex;

    GT_VOID *applicationDataPtr;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC;

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableCreate function
* @endinternal
*
* @brief   Create Rule Id Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] maxEntriesInDb           - maximal amount entries in DB.
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableCreate
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *pVTcamMngPtr,
    IN     GT_U32                                maxEntriesInDb
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableDelete function
* @endinternal
*
* @brief   Delete Rule Id Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      freed objects handlers set to NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableDelete
(
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *pVTcamMngPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesCreate function
* @endinternal
*
* @brief   Create Local per-vTcam Access Trees for Rule Id Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesCreate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete function
* @endinternal
*
* @brief   Delete Local per-vTcam Access Trees for Rule Id Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      freed objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableVTcamResizeCommit function
* @endinternal
*
* @brief   Commit Resise of Virtual TCAM.
*         Rules supposed aready moved - just update DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] toInsert                 - GT_TRUE - insert rules, GT_FALSE - delete rules
*                                      logicalIndex            - logical index of rule.
* @param[in] baseRuleLogicalIndex     - logical index of base rule.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableVTcamResizeCommit
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC             *pVTcamMngPtr,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC    *tcamCfgPtr,
    IN  GT_BOOL                                        toInsert,
    IN  GT_U32                                         baseRuleLogicalIndex,
    IN  GT_U32                                         rulesAmount
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryAdd function
* @endinternal
*
* @brief   Add rule Id entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] entryPtr                 - (pointer to) rule Id entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*
* @note treePathArr        - (pointer to) tree path array.
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryAdd
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC      *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC  *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryGet function
* @endinternal
*
* @brief   Get rule Id entry by rule Id.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] ruleId                   -  to find.
*
* @param[out] entryPtr                 - (pointer to) find rule entry.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - the logical index was not found.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                     ruleId,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryFind function
* @endinternal
*
* @brief   Find entry Id rule by logical index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] logicalIndex             - logical index to search.
* @param[in] findType                 - find type: see enum.
*
* @param[out] treePathArr              - (pointer to) tree path array.
* @param[out] entryPtr                 - (pointer to) find rule entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryFind
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN   GT_U32                                            logicalIndex,
    IN   PRV_CPSS_AVL_TREE_FIND_ENT                        findType,
    OUT  PRV_CPSS_AVL_TREE_PATH                            treePathArr,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntrySeek function
* @endinternal
*
* @brief   Seek rule entry by logical Id order.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] seekType                 - seek type: see enum.
* @param[in,out] treePathArr              - (pointer to) tree path array.
* @param[in,out] treePathArr              - (pointer to) tree path array.
*
* @param[out] entryPtr                 - (pointer to) find rule entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntrySeek
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *pVTcamMngPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN     PRV_CPSS_AVL_TREE_SEEK_ENT                        seekType,
    INOUT  PRV_CPSS_AVL_TREE_PATH                            treePathArr,
    OUT    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryRemove function
* @endinternal
*
* @brief   Remove rule Id entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] ruleId                   -  to remove.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryRemove
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN   GT_U32                                             ruleId
);

/**********************************************************************************/
/* Rule Id Table rule ranges utilities                                                                 */
/**********************************************************************************/

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_INVALIDATE_FUNC
 *
 * Description:
 *     This Function invalidates rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr            - device info passed to callback function.
 *       ruleLogicalIndex     - rule logial index.
 *       ruleSize             - size of rules used in virtual TCAM.
 *
 *  Comments:
 *
 */
typedef GT_STATUS PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_INVALIDATE_FUNC
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      ruleLogicalIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT      ruleSize
);

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_MOVE_FUNC
 *
 * Description:
 *     This Function move/copy rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr               - device info passed to callback function.
 *       srcRuleLogicalIndex     - source rule logical index.
 *       dstRuleLogicalIndex     - destination rule logical index.
 *       ruleSize                - size of rules used in virtual TCAM.
 *       moveOrCopy              - indication that we move or Copy
 *                                  GT_TRUE - move.
 *                                  GT_FALSE - copy.
 *  Comments:
 *
 */
typedef GT_STATUS PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_MOVE_FUNC
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      srcRuleLogicalIndex,
    IN  GT_U32      dstRuleLogicalIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT      ruleSize,
    IN  GT_BOOL     moveOrCopy
);

/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC
 *
 * @brief This structure of DB Rule Id table HW context
*/
typedef struct{

    GT_VOID *cookiePtr;

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_INVALIDATE_FUNC *ruleInvalidateFuncPtr;

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_MOVE_FUNC *ruleMoveFuncPtr;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC;

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryRangeGetRulesAmount function
* @endinternal
*
* @brief   Gets amount of rules in ginen range,.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] baseLogicalIndex         - base Logical Index of the range.
* @param[in] rangeSize                - size of the range in Logical Indexes
*
* @param[out] rulesAmountPtr           - (pointer to) amount of rules in the range
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryRangeGetRulesAmount
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN   GT_U32                                            baseLogicalIndex,
    IN   GT_U32                                            rangeSize,
    OUT  GT_U32                                            *rulesAmountPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryRangeDrag function
* @endinternal
*
* @brief   Move range of rules to a new range keeping logical index order of rules in all vTcam.
*         Used only to reorganize free space for future using.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] hwContextPtr             - (pointer to) HW Context
* @param[in] srcLogicalIndex          - base Logical Index of the source range.
* @param[in] srcRangeSize             - size of source range in Logical Indexes
* @param[in] dstLogicalIndex          - base Logical Index of the destination range.
* @param[in] dstRangeSize             - size of destination range in Logical Indexes
* @param[in] nearHighBound            - GT_TRUE - gather moved records near High Bound of new range
*                                      GT_FALSE - gather moved records near Low Bound of new range
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryRangeDrag
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                        *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC  *hwContextPtr,
    IN   GT_U32                                                    srcLogicalIndex,
    IN   GT_U32                                                    srcRangeSize,
    IN   GT_U32                                                    dstLogicalIndex,
    IN   GT_U32                                                    dstRangeSize,
    IN   GT_BOOL                                                   nearHighBound
);

/**********************************************************************************/
/* Priority Table                                                                 */
/**********************************************************************************/
/**
* @struct PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC
 *
 * @brief This structure of DB priority Entry
*/
typedef struct{

    /** priority. */
    GT_U32 priority;

    /** base Logical Index of range */
    GT_U32 baseLogIndex;

    /** @brief size of range, can be 0
     *  (to reserve space for yet not created ranges)
     *  Comments:
     */
    GT_U32 rangeSize;

} PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC;

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableCreate function
* @endinternal
*
* @brief   Create Priority Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] maxEntriesInDb           - maximal amount entries in DB.
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Handle to allocted table updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableCreate
(
    INOUT   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *pVTcamMngPtr,
    IN      GT_U32                              maxEntriesInDb
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableDelete function
* @endinternal
*
* @brief   Delete Priority Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
*                                      Handle to freed table set to NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableDelete
(
    INOUT   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *pVTcamMngPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesCreate function
* @endinternal
*
* @brief   Create Local per-vTcam Access Trees for Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      allocated objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesCreate
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete function
* @endinternal
*
* @brief   Delete Local per-vTcam Access Trees for Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in,out] tcamCfgPtr               - (pointer to)virtual TCAM structure
*                                      freed objects handlers updated
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntrySet function
* @endinternal
*
* @brief   Add or Update Logical Index Range to Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*
* @note 1. The given range should not overlap any existing range.
*       2. if priority0 >= priority1 then baseLogIndex0 >= baseLogIndex1
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntrySet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryGet function
* @endinternal
*
* @brief   Get Logical Index Range from Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
* @note For not existing range returned rangeSizePtr == 0 and
*       baseLogIndexPtr points to the end of the last range
*       that should be before the required range, i.e. the range with
*       minimal priority greater the given (if not exist - baseLogIndexPtr == 0)
*       This value can be used as base of this not existing range adding it.
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   GT_U32                                       priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityRemoveRuleFromRangeById function
* @endinternal
*
* @brief   Removes rule from priority entry if first or last.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  ruleId                   - rile id.
* @param[in]  reservedLogIndex         - reserved Logical Index for yet not copied rule - do not remove.
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityRemoveRuleFromRangeById
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   GT_U32                                       ruleId,
    IN   GT_U32                                       reservedLogIndex
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet function
* @endinternal
*
* @brief   Get Maximal Logical Index Range gor the given Priority (range between neighbour priorityes).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  priority                 - priority as key to find entry.
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   GT_U32                                       priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet function
* @endinternal
*
* @brief   Get Minimal Logical Index Range for the given Priority.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  priority                 - priority as key to find entry.
* @param[in]  findType                 - type of find entry by priority (EQ, LE, GE)
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   PRV_CPSS_AVL_TREE_FIND_ENT                   findType,
    IN   GT_U32                                       priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableMoveNeighbourEntriesBound function
* @endinternal
*
* @brief   Update neighbour priority entries moving the low or High bound of one of them to the neigbour.
* @note    the function does not check validity of the given pair of neigbour entries - can corrupt the DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  lowEntryPtr              - (pointer to) entry with info
* @param[in]  highEntryPtr             - (pointer to) entry with info
* @param[in]  moveLowToHigh            - priority GT_TRUE - low to high, GT_FALSE high to low.
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableMoveNeighbourEntriesBound
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *lowEntryPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *highEntryPtr,
    IN   GT_BOOL                                            moveLowToHigh
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableIndexRangeClear function
* @endinternal
*
* @brief   Clear priority entries covering the given index range not containing rules.
*          The range can remain covered if all of it at the middle of one priority entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - priority
* @param[in] placeFromHigh            - GT_TRUE for high neighbour, GT_FALSE for low neighbour
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableIndexRangeClear
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN       GT_U32                                             baseLogIndex,
    IN       GT_U32                                             numOfRules
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryNeighbourAdjust function
* @endinternal
*
* @brief   Adjust bound of neighbour priority entry before adding rule.
*          Decreases space covered by priorty entry not containing rules.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - priority
* @param[in] placeFromHigh            - GT_TRUE for high neighbour, GT_FALSE for low neighbour
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryNeighbourAdjust
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN       GT_U32                                             priority,
    IN       GT_BOOL                                            placeFromHigh
);

/**
* @internal prvCpssDxChVirtualTcamSplitHoleLowPartSizeGet function
* @endinternal
*
* @brief   Get size of left part of the splitted hole.
* @note    Two cases for splitting hole:
* @note    1. write new rule into the hole, 2. shift range of rules from the hole side into it.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in]  holeBaseLogIndex         - Hole Base Logical Index
* @param[in]  holeSize                 - Hole Size in rules
* @param[in]  requiredPriority         - priority of the new rule
* @param[in]  anchorLogicalIndex       - logical index of busy position precalculated to insert the new rule
* @param[out] lowPartSizePtr           - (pointer to) size of low part of the hole
* @param[out] newRuleCloseToLowPtr     - (pointer to) position of the new rule in a new hole:
*                                        0 - low side, 100 - high side, 50 - middle
*
* @retval                              - GT_OK - on success, GT_FAIL - on wrong parameters
*
*/
GT_STATUS prvCpssDxChVirtualTcamSplitHoleLowPartSizeGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN   GT_U32                                             holeBaseLogIndex,
    IN   GT_U32                                             holeSize,
    IN   GT_U32                                             requiredPriority,
    IN   GT_U32                                             anchorLogicalIndex,
    OUT  GT_U32                                             *lowPartSizePtr,
    OUT  GT_U32                                            *newRuleCloseToLowPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryBallancedGet function
* @endinternal
*
* @brief   Get Logical Index Range from Priority Table with memory ballance for a new priority.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - priority
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
* @note For not existing range returned rangeSizePtr == 0 and
*       baseLogIndexPtr points to the middle of the nearest less priority range (or 0 if not)
*       and nearest more priority range (or tcamCfgPtr->rulesAmount if not).
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryBallancedGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN   GT_U32                                             priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryFind function
* @endinternal
*
* @brief   Find entry Id rule by logical index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] treePathArr              - (pointer to) tree path array.
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryFind
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC            *tcamCfgPtr,
    IN   GT_U32                                                  logicalIndex,
    IN   PRV_CPSS_AVL_TREE_FIND_ENT                              findType,
    OUT  PRV_CPSS_AVL_TREE_PATH                                  treePathArr,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC       *entryPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntrySeek function
* @endinternal
*
* @brief   Seek rule entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] seekType                 - seek type: see enum.
* @param[in,out] treePathArr              - (pointer to) tree path array.
* @param[in,out] treePathArr              - (pointer to) tree path array.
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntrySeek
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *pVTcamMngPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC      *tcamCfgPtr,
    IN     PRV_CPSS_AVL_TREE_SEEK_ENT                       seekType,
    INOUT  PRV_CPSS_AVL_TREE_PATH                           treePathArr,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC       *entryPtr
);

/**********************************************************************************/
/* Utility                                                                */
/**********************************************************************************/

/**
* @internal prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet function
* @endinternal
*
* @brief   Get place for new rule.
*         If needed move existing rules.
*         If needed resize vTCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - vTCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - virtual TCAM Id
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] origIndexForPriorityUpdate - the original index of the entry that doing
* @param[in] priority                 update'
*                                      value ORIG_INDEX_NOT_USED_CNS meaning that it is
*                                      not relevant.
* @param[in] vTcamMaxSize             - maximal size of vTCAM. (no resize to greater).
*                                      The current size retrieved from Segment Table.
* @param[in] hwContextPtr             - (pointer to)HW context for moving existing rules.
* @param[in] priority                 - new rule priority
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
* @param[in] sizeIncrement            - amount of rules to be added if resize will needed.
* @param[in] recalled                 -  due to autoresize
*
* @param[out] logIndexPtr              - (pointer to) Logical Index of new rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_FULL                  - on table full (no free logical index)
*/
GT_STATUS prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   GT_U32                                                    origIndexForPriorityUpdate ,
    IN   GT_U32                                                    vTcamMaxSize,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC  *hwContextPtr,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position,
    IN   GT_U32                                                    sizeIncrement,
    IN   GT_BOOL                                                   recalled,
    OUT  GT_U32                                                    *logIndexPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1Get function
* @endinternal
*
* @brief   Get place for new rule.
*         If needed move existing rules.
*         If needed resize vTCAM.
*         Optimized version of prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - vTCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - virtual TCAM Id
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] origIndexForPriorityUpdate - the original index of the entry that doing
* @param[in] priority                 update'
*                                      value ORIG_INDEX_NOT_USED_CNS meaning that it is
*                                      not relevant.
* @param[in] vTcamMaxSize             - maximal size of vTCAM. (no resize to greater).
*                                      The current size retrieved from Segment Table.
* @param[in] hwContextPtr             - (pointer to)HW context for moving existing rules.
* @param[in] priority                 - new rule priority
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
* @param[in] sizeIncrement            - amount of rules to be added if resize will needed.
* @param[in] recalled                 -  due to autoresize
*
* @param[out] logIndexPtr              - (pointer to) Logical Index of new rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_FULL                  - on table full (no free logical index)
*/
GT_STATUS prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1Get
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   GT_U32                                                    origIndexForPriorityUpdate ,
    IN   GT_U32                                                    vTcamMaxSize,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC  *hwContextPtr,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position,
    IN   GT_U32                                                    sizeIncrement,
    IN   GT_BOOL                                                   recalled,
    OUT  GT_U32                                                    *logIndexPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbUtilVTcamReduce function
* @endinternal
*
* @brief   Compress vTCAM and free part of it.
*         If needed move existing rules.
*         Resize vTCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] hwContextPtr             - (pointer to)HW context for moving existing rules.
* @param[in] priority                 -  used to seek the prefered place for removing
*                                      free space.
* @param[in] sizeDecrement            - amount of rules free.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbUtilVTcamReduce
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                        *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *tcamCfgPtr,
    IN   GT_U32                                                    vTcamMaxSize,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC  *hwContextPtr,
    IN   GT_U32                                                    priority,
    IN   GT_U32                                                    sizeDecrement
);


/*******************************************************************************
* prvCpssDxChVirtualTcamDbVTcamMngGet
*
* DESCRIPTION:
*       Get DB info about a vTcam manager.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngId        - Virtual TCAM Manager Id
*                           (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the DB of the vTcam.
*       NULL on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* prvCpssDxChVirtualTcamDbVTcamMngGet
(
    IN  GT_U32                                      vTcamMngId
);

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVTcamGet
*
* DESCRIPTION:
*       Get DB info about a vTcam in vTcam manager.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngId        - Virtual TCAM Manager Id
*                           (APPLICABLE RANGES: 0..31)
*       vTcamId           - unique Id of  Virtual TCAM
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the DB of the vTcam.
*       NULL on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* prvCpssDxChVirtualTcamDbVTcamGet
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
);

/**
* @internal prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst function
* @endinternal
*
* @brief   The function returns the first devNum that is bound to the tcam manager.
*         it must be called before any iteration on the device list of the vTcam.
* @param[in] vTcamMngDBPtr            - (pointer to)Virtual TCAM Manager structure
*
* @param[out] devNumPtr                - (pointer to) hold 'first' devNum .
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_NO_MORE               - no devices attached to the vTcamMngId
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    OUT GT_U8                               *devNumPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbDeviceIteratorGetNext function
* @endinternal
*
* @brief   The function returns the next devNum that is bound to the tcam manager.
*
* @param[in] vTcamMngDBPtr            - (pointer to)Virtual TCAM Manager structure
* @param[in,out] devNumPtr                - (pointer to) hold 'current' devNum .
* @param[in,out] devNumPtr                - (pointer to) hold 'next' devNum .
*
* @retval 0                        - no more devices. ('current' device is the last one)
* @retval 1                        - devNumPtr hold 'next' devNum
*/
GT_U32  prvCpssDxChVirtualTcamDbDeviceIteratorGetNext
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    INOUT GT_U8                             *devNumPtr
);

/**
* @internal prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet function
* @endinternal
*
* @brief   The function gets bitmap of all port groups representing TCAMs.
*
* @param[in]  devNum            - device number
* @param[out] portGroupsBmpPtr  - (pointer to) bitmap of all port groups representing TCAMs.
*
* @retval GT_BAD_PARAM          - wrong device number.
* @retval GT_OK                 - correct result
*/
GT_STATUS  prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet
(
    IN  GT_U8                             devNum,
    OUT GT_PORT_GROUPS_BMP                *portGroupsBmpPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbManagerDump function
* @endinternal
*
* @brief   Dump tcam info without ‘vtcam info’
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*                                      perLookupOnly     - GT_TRUE  - amounts per lookup only
*                                      - GT_FALSE - amounts per block and per lookup
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbManagerDump
(
    IN  GT_U32                                      vTcamMngId
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDump function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDump
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
);

/* several partial dums: subsets of prvCpssDxChVirtualTcamDbVTcamDump */
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintPriorityTree
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr
);
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRuleIdTree
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr
);
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRulesLogIndexesBitmap
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr
);
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintPriorityTreeById
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
);
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRuleIdTreeById
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
);
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRulesLogIndexesBitmapById
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr function
* @endinternal
*
* @brief   Dump block free space.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                      (usefull for dummy vTCAM Manager that has no Id)
* @param[in] perLookupOnly            - GT_TRUE  - amounts per lookup only
*                                      - GT_FALSE - amounts per block and per lookup
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* tcamSegCfgPtr,
    IN  GT_BOOL                                     perLookupOnly
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace function
* @endinternal
*
* @brief   Dump block free space.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     perLookupOnly
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager of specific field.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngPtr              - (pointer to)Virtual TCAM Manager structure
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] fieldNamePtr             - name of the required field
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*         vTcamMngPtr,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpByField function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager of specific field.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] fieldNamePtr             - name of the required field
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByField
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr function
* @endinternal
*
* @brief   Dump DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC*    tcamSegCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout function
* @endinternal
*
* @brief   Dump DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout
(
    IN  GT_U32                                      vTcamMngId
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify function
* @endinternal
*
* @brief   Verify DB info about Virtual TCAM Rules layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
);

/**
* @internal prvCpssDxChVirtualTcamDbVerifyFullTcamLayout function
* @endinternal
*
* @brief   Verify DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVerifyFullTcamLayout
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
);

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree function
* @endinternal
*
* @brief   print the avl segments tree..
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] treeIdPtr                - a pointer to the id of the segments tree
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbVerifyAllTrees function
* @endinternal
*
* @brief   Verify trees in DB - matching data to compaire functions.
*         Should detect corruption caused by data updated without removing and inserting
*         related tree items.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVerifyAllTrees
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc function
* @endinternal
*
* @brief   AVL Tree compare function for segments in lookup order.
*         Used only internal.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] data_ptr1                - (pointer to) first segment DB entry
* @param[in] data_ptr2                - (pointer to) second segment DB entry
*                                       GT_EQUAL, GT_GREATER or GT_SMALLER.
*/
GT_COMP_RES prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
);

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceEstimateFreeSpace function
* @endinternal
*
* @brief   Estimate free space in all TCAM related to request.
*         Free space in other lookups truncated to size of whole blocks
*         that possible can be freed for use by given lookup.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @retval GT_OK                    - enough free space (maybe can be moved by dragging to be usable).
* @retval GT_FULL                  - not enough free space.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceEstimateFreeSpace
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId function
* @endinternal
*
* @brief   Drag vTcams in blocks mapped to required lookup id.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] checkWithoutUpdate       - GT_TRUE  - vTcams dragging checked using work DB,
*                                      permanent DB and HW not updated
*                                      GT_FALSE - vTcams dragged in permanent DB and HW
*
* @param[out] actualCapacityPtr        - (pointer to) actual TCAM Rules amount..
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpaceDragVTcamsInLookupId
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     GT_BOOL                                                    checkWithoutUpdate,
    OUT    GT_U32                                                     *actualCapacityPtr
);

/**
* @internal prvCpssDxChVirtualTcamDbSegSpace80bytesSpaceMake function
* @endinternal
*
* @brief   Make free space ro 80-byte rules.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
*
* @retval GT_OK                    - success.
* @retval other                    - fail.
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegSpace80bytesSpaceMake
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr
);

/**
* @internal prvCpssDxChVirtualTcamRemove function
* @endinternal
*
* @brief   Remove Virtual TCAM - with option to remove not empty vTCAM from DB
*         without cleaning HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] failOnHavingRules        - GT_TRUE  - to fail if vTcam contains valid rules
*                                      - GT_FALSE - to remove vTcam anyway
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - vTcam contains rules (either valid or invalid rules)
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS prvCpssDxChVirtualTcamRemove
(
    IN  GT_U32                             vTcamMngId,
    IN  GT_U32                             vTcamId,
    IN  GT_BOOL                            failOnHavingRules
);

/**
* @internal prvCpssDxChVirtualTcamLibraryShutdown function
* @endinternal
*
* @brief   Shutdown Virtual TCAM Library.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on TCAM manager not found
*/
GT_STATUS prvCpssDxChVirtualTcamLibraryShutdown
(
    GT_VOID
);

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVTcamIsUnmovable
*
* DESCRIPTION:
*       Checks whether VTCAM is unmovable.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngDBPtr        - (pointer to) VTCAM Manager DB structure
*       vTcamId              - unique Id of  Virtual TCAM
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE for unmovable VTCAM, GT_FALSE for all other cases including wrong parameters
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL prvCpssDxChVirtualTcamDbVTcamIsUnmovable
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    IN  GT_U32                              vTcamId
);

/**
* @internal prvCpssDxChVirtualTcamDbSegSpaceBlockUnmovableBlocksBitmapGet function
* @endinternal
*
* @brief   Get bitmap of indexes of unmovable blocks.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*
* @param[out] unmovableBlocksBmpArr[] - (pointer to)bitmap array of indexes of unmovable blocks
* RETURN
* None.
* COMMENTS:
*/
GT_VOID prvCpssDxChVirtualTcamDbSegSpaceBlockUnmovableBlocksBitmapGet
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr,
    OUT    GT_U32                                        unmovableBlocksBmpArr[]
);

/**
* @internal prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns function
* @endinternal
*
* @brief   Horizontal split and free not usable space of segments.
*         Segments retieved from free space tree covered oly one horizontal block,
*         but already merged with horizontal neighbors (if were).
*         The needed split relevant only for 40 and 50 byte rules that
*         got full block row segments.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] allocRequestPtr          - (pointer to) allocation request structure
* @param[in] treeId                   - primary tree for allocation.
*
* @retval GT_OK                    - on success
* @retval GT_NO_RESOURCE           - on no TCAM space
*/
GT_STATUS prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC              *tcamSegCfgPtr,
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    *allocRequestPtr,
    IN     PRV_CPSS_AVL_TREE_ID                                       treeId
);

/* amount of words in bitmap of TCAM block 256 / 32 */
#define PRV_WORDS_IN_BLOCK_ROWS_BITMAP_CNS 8

#define VTCAM_GLOVAR(_x) PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChVTcamDir._x)

#define PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DBG_PRINT(_x)                                   \
    {                                                                                 \
        if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDebugTraceEnable) != GT_FALSE)         \
        {                                                                             \
            cpssOsPrintf("line %4.4d: ",__LINE__);                                    \
            cpssOsPrintf _x;                                                          \
            cpssOsPrintf("\n");                                                       \
        }                                                                             \
    }

/* values marking deleted rule */
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_VTCAM_ID 0x0FFF
#define PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_RULE_ID 0xFFFFFFFF

/**
* @internal prvCpssDxChVirtualTcamHaHwDbWrite function
* @endinternal
*
* @brief   Writes to HW DB pair of vTcamId and ruleId for HW rule index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
* @param[in] ruleHwIndex             - rule physical index.
* @param[in] vTcamId                 - vTCAM Id.
* @param[in] ruleId                  - rule Id
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbWrite
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr,
    IN  GT_U32                                       ruleHwIndex,
    IN  GT_U32                                       vTcamId,
    IN  GT_U32                                       ruleId
);

/**
* @internal prvCpssDxChVirtualTcamHaHwDbRead function
* @endinternal
*
* @brief   Reads to HW DB pair of vTcamId and ruleId by HW rule index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
* @param[in] ruleHwIndex             - rule physical index.
* @param[out] vTcamIdPtr             - (pointer to)vTCAM Id.
* @param[out] ruleIdPtr              - (pointer to)rule Id
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbRead
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr,
    IN  GT_U32                                       ruleHwIndex,
    OUT GT_U32                                       *vTcamIdPtr,
    OUT GT_U32                                       *ruleIdPtr
);

/**
* @internal prvCpssDxChVirtualTcamHaHwDbClearAll function
* @endinternal
*
* @brief   Clear all HW DB of pairs of vTcamId and ruleId.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbClearAll
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr
);

/**
* @internal prvCpssDxChVirtualTcamHaHwDbCopyAll function
* @endinternal
*
* @brief   Copy all HW DB of pairs of vTcamId and ruleId to a new device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbCopyAll
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr,
    IN GT_U8                                         devNum,
    IN GT_U32                                        portGroupsBmp
);

/**
* @internal prvCpssDxChVirtualTcamSegmentDbByDevClassGet function
* @endinternal
*
* @brief   Get Segment Cfg DB by device class.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngDBPtr         - (pointer to)VTCAM Manager structure.
* @param[in] deviceClass        - device TCAM class: SIP5(and above), LEGACY_PCL, LEGACY_TTI, AC5_PCL_TCAM1.
*
* @retval GT_OK                    - on success, others on fail.
*/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* prvCpssDxChVirtualTcamSegmentDbByDevClassGet
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngDBPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass
);

/**
* @internal prvCpssDxChVirtualTcamHaHwDbAccessBind function
* @endinternal
*
* @brief   Bind HW DB access functions to Segment Cfg DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
* @param[in] deviceClass        - device TCAM class: SIP5(and above), LEGACY_PCL, LEGACY_TTI, AC5_PCL_TCAM1.
* @param[in] hwDbWriteFuncPtr   - (pointer to)write function.
* @param[in] hwDbReadFuncPtr    - (pointer to)read function
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbAccessBind
(
    IN GT_U32                                            vTcamMngId,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass,
    IN CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC    *hwDbWriteFuncPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC     *hwDbReadFuncPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChVirtualTcamDb_h */


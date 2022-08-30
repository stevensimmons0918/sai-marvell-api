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
* @file cpssDxChDiagDataIntegrity.c
*
* @brief CPSS DXCH Diagnostic Data Integrity API
*
* @version   15
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagLog.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsBobcat2.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsBobK.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAldrin.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsBobcat3.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAldrin2.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsFalcon.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAlleyCat5.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAlleyCat5X.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAlleyCat5P.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsHarrier.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* the number of entries allocated for IPLR0 to use */
#define IPLR0_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]

/* the number of entries allocated for IPLR1 to use */
#define IPLR1_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]

/* the number of counting entries allocated for IPLR0 to use */
#define IPLR0_COUNTING_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.countingMemSize[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]

/* the number of counting entries allocated for IPLR1 to use */
#define IPLR1_COUNTING_SIZE_MAC(devNum) \
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.countingMemSize[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E]

#define PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChDiagDir.dataIntegrityDb._var,_value)

#define PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChDiagDir.dataIntegrityDb._var)

/**
* @internal specialTableIdToBase function
* @endinternal
*
* @brief   Function converts some special table Ids from ranges to the bases of these ranges.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] srcId                   - special table Id to be converted
* @param[in] forEventInfo            - GT_FALSE - function is called for configurations
*                                      GT_TRUE - function is called for HW index calculation for event
*
* @retval GT_OK                    - result of conversion
*/
static PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT specialTableIdToBase
(
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT srcId,
    GT_BOOL   forEventInfo
)
{
    /* such LPM not releant for SIP6 devices*/
    if ((srcId >= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E)
        && (srcId <= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_39_E))
    {
        return PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E;
    }
    /* Shared memory LPM, FDB, EM, ARP of SIP6 devices */
    if ((srcId >= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E)
        && (srcId <= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_LAST_E))
    {
        return PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E;
    }
    if ((srcId >= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE1_SBM_0_E)
        && (srcId <= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE1_SBM_LAST_E))
    {
        /* use TYPE2 for configurations to configure all shared memory RAMs
           use TYPE1 for HW index calculations    */
        return (forEventInfo == GT_FALSE) ? PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E :
                                            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE1_SBM_0_E;
    }
    return srcId;
}

/* convert HW Table number for special cases:
   LPM - DB hold 20 (sip5.20 - 40) different IDs belong to same HW Table. Use
   first one for algorithm to manage features on all LPM RAM by same way.
   SHM_TYPE2 and SHM_TYPE1 of SIP6 ranges added.
   */
#define PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(__hwTbl) \
    {__hwTbl = specialTableIdToBase(__hwTbl, GT_FALSE);}

#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_BASE_ADDR_CNS                       0x0   /* base address */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_CAUSE_REG_CNS            0x1C  /* memory interrupt cause register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS             0x18  /* memory interrupt mask register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS              0x14  /* memory control register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRONEOUS_ADDR_REG_CNS       0x8   /* memory erroneous address register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRONEOUS_SEGMENT_REG_CNS    0x10  /* memory erroneous segment register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ECC_SYNDROME_REG_CNS         0xC   /* memory ECC syndrome register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRORS_COUNTER_REG_CNS       0x4   /* memory error counter register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS          0x50  /* client status 0 register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS1_REG_CNS          0x54  /* client status 1 register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS2_REG_CNS          0x58  /* client status 2 register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS3_REG_CNS          0x5C  /* client status 3 register */

#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_CAUSE_CNS                0x88  /* client interrupt cause register */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_MASK_CNS                 0x8C  /* client interrupt mask register */

/* macros to extract DFX pipe, client, memory values from key field of DataIntegrity DB
 * item (DB item is PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC) */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(key)   U32_GET_FIELD_MAC((key), 12, 3)
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(key) U32_GET_FIELD_MAC((key), 7,  5)
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(key) U32_GET_FIELD_MAC((key), 0,  7)

#define PRV_CPSS_DXH_DIAG_DATA_INTEGRITY_CLIENTS_COUNT_CNS 28
/* macro to construct DataIntegrity DB item's key value from DFX pipe, clien, memory
 * values. DB item is PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_GENERATE_KEY_MAC(pipeId, clientId, memId) \
          ((pipeId) << 12) | ((clientId) << 7) | (memId)

/* constant not matching any existent hw table (CPSS_DXCH_TABLE_ENT or PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT)*/
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS LAST_VALID_TABLE_CNS

/* constant not matching any existent memory type */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS ((GT_U32)-1)


/* Falcon: Eagle Tiles 0,2 - pipes 0..4 - #pipes = 5;
 *         Eagle Tiles 1,3 - pipes 0..3 - #pipes = 4;
 */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_NUM_GET(__tile) (((__tile) & 1) ? 4 : 5)

/* __result = 1 =>  pipe index is not valid for the given tile index
 * __result = 0 => pipe index is valid for the given tile index
 */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_INDEX_CHECK(__tile, __pipe, __result) \
   __result = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_NUM_GET(__tile);                \
   if (__pipe >= __result)                                                                       \
   {                                                                                             \
       __result = 1;                                                                             \
   }                                                                                             \
   else                                                                                          \
   {                                                                                             \
       __result = 0;                                                                             \
   }

/* Falcon: Eagle Tiles 0,2 - pipes 0..4 - bitmap = 0x1F;
 *         Eagle Tiles 1,3 - pipes 0..3 - bitmap = 0xF;
 */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_BMP_GET(__tile) (((__tile) & 1) ? 0xF : 0x1F)

/* Maximal number of DFX tiles in multicore system */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_TILE_MAX_CNS 4

/* Maximal number of DFX chiplets in multicore system */
#define PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_CHIPLETS_MAX_CNS 16

/* Interrupt index assigned for the unused interrupt */
#define PRV_CPSS_LAST_INT_INDEX_E 0xFFFFFFFF

/**
* @struct PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC
*
* @brief Bitmap of memories errors in DFX Client
*/
typedef struct{

    GT_U32 bitmap[4];

} PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC;


static GT_STATUS prvCpssDxChDiagDataIntegrityPortGroupMemoryIndexesGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          portGroupId,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocation
);

static GT_STATUS    plrStartRamToIndexInTableConvert
(
    IN  GT_U8                                           devNum,
    IN GT_U32                                           ramIndex,
    OUT GT_U32                                          *plrIndexPtr,
    OUT GT_U32                                          *indexPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityErrorInfoGetLion2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityErrorInfoGetSip5
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSegmentPtr,
    OUT GT_U32                                          *failedSyndromePtr
);

static GT_STATUS plrMemTypeFirstIndexInHwTableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    OUT CPSS_DXCH_TABLE_ENT                        *hwTablePtr,
    OUT GT_U32                                     *hwIndexPtr
);

static GT_STATUS plrHwTableIndexToLogicalTableIndex
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_TABLE_ENT         hwTable,
    IN  GT_U32                      hwIndex,
    IN  CPSS_DXCH_LOGICAL_TABLE_ENT logicalTable,
    OUT GT_U32                      *logicalIndexPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat3
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetFalcon
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5x
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5p
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetHarrier
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat3
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskFalcon
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5x
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5p
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskHarrier
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvMakeHwTableSuitableForApi
(
    IN    GT_U8                devNum,
    INOUT CPSS_DXCH_TABLE_ENT *tableTypePtr,
    INOUT GT_U32              *entryIndexPtr,
    IN    GT_U32               startBit
);

static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGet
(
    IN  GT_U8                                       devNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pipeId,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT   dfxInstanceType,
    IN  GT_U32                                      tileIndex,
    IN  GT_U32                                      chipletIndex,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDfxMemoryEventFromDbGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pipeId,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      memId,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);

static GT_STATUS prvCpssDxChDiagDfxMemoryEventFromDbGetFalcon
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pipeId,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      memId,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT   dfxInstanceType,
    IN  GT_U32                                      tileIndex,
    IN  GT_U32                                      chipletIndex,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat3
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin2
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetFalcon
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5x
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5p
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetHarrier
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_VOID prvPhyToTxqRange
(
    IN  GT_U32 devNum,
    IN  GT_U32 phyStart,
    IN  GT_U32 phyRange,
    OUT GT_U32 *txqStartPtr,
    OUT GT_U32 *txqRangePtr
);

static GT_VOID prvTxqToPhyRange
(
    IN  GT_U32 devNum,
    IN  GT_U32 txqStart,
    IN  GT_U32 txqRange,
    OUT GT_U32 *phyStartPtr,
    OUT GT_U32 *phyRangePtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobk
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
);

static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobk
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);

static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobk
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

static GT_BOOL prvIsMemProtectedButNotSupportedSip5
(
    IN  GT_U8                                      devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType
);

static GT_BOOL prvIsMemProtectedButNotSupportedCnmAc5_Ac5x
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType
);

static GT_BOOL prvIsMemProtectedButNotSupportedSip6_10
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType
);

/* array of bitmaps specifies pipes and clients relevant for eagle.
 * pipe i, client j exists if eagleDfxPipeClientsBmpArr[i] has bit #j raised. */
static GT_U32 eagleDfxPipeClientsBmpArr[] = {
    0x0003FFFF,   /* pipe 0, clients 0..17  */
    0x03FFFFFF,   /* pipe 1, clients 0..25 */
    0x0003FFFF,   /* pipe 2, clients 0..17 */
    0x00FFFFFF,   /* pipe 3, clients 0..23 */
    0x00000007    /* pipe 4, clients 0..2 */
};

/* array of bitmaps specifies pipes and clients relevant for raven.
 * pipe i, client j exists if ravenDfxPipeClientsBmpArr[i] has bit #j raised. */
static GT_U32 ravenDfxPipeClientsBmpArr[] = {
    0x00000003,   /* pipe 0, clients 0..1  */
    0x0000001F,   /* pipe 1, clients 0..4 */
    0x00000001    /* pipe 2, client 0 */
};

/**
* @internal prvCpssDxChDiagDataIntegrityDbPointerSet function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
*
* @param[out] dbArrayPtrPtr            - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr       - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_VOID prvCpssDxChDiagDataIntegrityDbPointerSet
(
    IN  GT_U8                                               devNum,
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC  const **dbArrayPtrPtr,
    OUT GT_U32                                        *dbArrayEntryNumPtr
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devType)
            {
                case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
                    *dbArrayPtrPtr = hooperDataIntegrityDbArray;
                    *dbArrayEntryNumPtr = hooperDataIntegrityDbArrayEntryNum;
                    break;
                default:
                    *dbArrayPtrPtr = lion2DataIntegrityDbArray;
                    *dbArrayEntryNumPtr = lion2DataIntegrityDbArrayEntryNum;
                    break;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {/* Caelum based devices */
                *dbArrayPtrPtr = caelumDataIntegrityDbArray;

                if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
                {
                    /* Cetus doesn't support DFX pipe #3 placed at the end of the DB.
                     * So return a reduced  (compared with Caelum) DB size -
                     * without pipe 3 memories */

                    if (PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS == PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_GET(cetusDbSize))
                    {
                        GT_U32 pipe3Key = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_GENERATE_KEY_MAC(3,0,0);

                        for (PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_SET(cetusDbSize, 0);
                            (PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_GET(cetusDbSize) < caelumDataIntegrityDbArrayEntryNum &&
                              caelumDataIntegrityDbArray[PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_GET(cetusDbSize)].key < pipe3Key);
                             PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_GET(cetusDbSize)++);

                    }
                    *dbArrayEntryNumPtr = PRV_SHARED_DIAG_DIR_DATA_INTEGRITY_SRC_GLOBAL_VAR_GET(cetusDbSize);
                }
                else
                {
                    /* caelum */
                    *dbArrayEntryNumPtr = caelumDataIntegrityDbArrayEntryNum;
                }
            }
            else
            {/* Bobcat2 devices */
                if (!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
                {
                    *dbArrayPtrPtr = bobcat2DataIntegrityDbArray;
                    *dbArrayEntryNumPtr = bobcat2DataIntegrityDbArrayEntryNum;
                }
                else
                {
                    *dbArrayPtrPtr = bobcat2B0DataIntegrityDbArray;
                    *dbArrayEntryNumPtr = bobcat2B0DataIntegrityDbArrayEntryNum;
                }
            }
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            *dbArrayPtrPtr = aldrinDataIntegrityDbArray;
            *dbArrayEntryNumPtr = aldrinDataIntegrityDbArrayEntryNum;
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            *dbArrayPtrPtr = bobcat3DataIntegrityDbArray;
            *dbArrayEntryNumPtr = bobcat3DataIntegrityDbArrayEntryNum;
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            *dbArrayPtrPtr = aldrin2DataIntegrityDbArray;
            *dbArrayEntryNumPtr = aldrin2DataIntegrityDbArrayEntryNum;
            break;

        case CPSS_PP_FAMILY_DXCH_AC5_E:
            *dbArrayPtrPtr = ac5DataIntegrityDbArray;
            *dbArrayEntryNumPtr = ac5DataIntegrityDbArrayEntryNum;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            *dbArrayPtrPtr = ac5xDataIntegrityDbArray;
            *dbArrayEntryNumPtr = ac5xDataIntegrityDbArrayEntryNum;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            *dbArrayPtrPtr = ac5pDataIntegrityDbArray;
            *dbArrayEntryNumPtr = ac5pDataIntegrityDbArrayEntryNum;
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            *dbArrayPtrPtr = harrierDataIntegrityDbArray;
            *dbArrayEntryNumPtr = harrierDataIntegrityDbArrayEntryNum;
            break;
        default:
             *dbArrayEntryNumPtr = 0;
             *dbArrayPtrPtr = NULL;
             break;
    }

    return;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDbPointerSetFalcon function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
*
* @param[out] dbArrayPtrPtr            - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr       - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_VOID prvCpssDxChDiagDataIntegrityDbPointerSetFalcon
(
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType,
    OUT const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    **dbArrayPtrPtr,
    OUT GT_U32                                          *dbArrayEntryNumPtr
)
{

    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
    {
        *dbArrayPtrPtr = eagleDataIntegrityDbArray;
        *dbArrayEntryNumPtr = eagleDataIntegrityDbArrayEntryNum;
    }
    else
    {
        *dbArrayPtrPtr = ravenDataIntegrityDbArray;
        *dbArrayEntryNumPtr = ravenDataIntegrityDbArrayEntryNum;
    }
    return;
}

/**
* @internal plrStartPlrRamToOffsetInDbGet function
* @endinternal
*
* @brief   Function converts PLR RAM index to index in DB.
*
* @note   APPLICABLE DEVICES:      AC5; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] hwTableType              - HW table type
*
* @param[out] ramOffsetPtr             - (pointer to) PLR offset in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong table type
*/
static GT_STATUS plrStartPlrRamToOffsetInDbGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_TABLE_ENT             hwTableType,
    OUT GT_U32                          *ramOffsetPtr
)
{
    GT_STATUS   rc;
    GT_U32      index;
    GT_U32      ramIndex;
    GT_U32      plrIndex;
    GT_U32      tablePlrIndex; /* ingress policer number (0,1) the hw table belongs to */

    tablePlrIndex = 1;
    switch (hwTableType)
    {
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E:
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                *ramOffsetPtr = 0;
                return GT_OK;
            }
            tablePlrIndex = 0;
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E:
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                *ramOffsetPtr = 0;
                return GT_OK;
            }
            tablePlrIndex = 1;
            break;
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E:
            tablePlrIndex = 0;
            break;
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E:
            tablePlrIndex = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (ramIndex = 0; ramIndex < 3; ramIndex++)
    {
        rc = plrStartRamToIndexInTableConvert(devNum, ramIndex, &plrIndex, &index);
        if(GT_OK != rc)
        {
            return rc;
        }
        if (plrIndex == tablePlrIndex)
        {
            *ramOffsetPtr = ramIndex * 2;
            break;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityHwTableGet function
* @endinternal
*
* @brief   Function gets 'HW_table + offset' from DB
*
* @note   APPLICABLE DEVICES:      AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in,out] hwTableTypePtr       - in : HW table type.
*                                       out: HW table type as it is in DB.
*
* @param[out] offsetPtr                - (pointer to) HW table offset. Can be NULL.
* @param[out] maxEntriesPtr            - (pointer to) HW table actual number of entries.
*                                      Can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityHwTableGet
(
    IN  GT_U8                                           devNum,
    INOUT GT_U32                                      * hwTableTypePtr,
    OUT GT_U32                                        * offsetPtr,
    OUT GT_U32                                        * maxEntriesPtr
)
{
    CPSS_DXCH_TABLE_ENT            hwTableType;
    GT_STATUS                      rc         = GT_OK;
    GT_U32                         offset     = 0;
    GT_U32                         maxEntries = 0;

    hwTableType = *hwTableTypePtr;

    rc = prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet(devNum, hwTableType, &maxEntries, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch (hwTableType)
    {
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E:
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E:
            if (! PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_IPLR_METERING_E;
                rc = plrStartPlrRamToOffsetInDbGet(devNum, hwTableType, &offset);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E:
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E:
            if (! PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_IPLR_COUNTING_E;
                rc = plrStartPlrRamToOffsetInDbGet(devNum, hwTableType, &offset);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E:
        case CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E:
            *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ERMRK_QOS_MAP_E;
            break;

        case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E:
        case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E:
            *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP_E;
            break;

        case CPSS_DXCH_SIP5_TABLE_L2_MLL_E:
            *hwTableTypePtr = CPSS_DXCH_SIP5_TABLE_IP_MLL_E;
            break;

        case CPSS_DXCH_SIP5_TABLE_LPM_MEM_E:
            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum) || PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E;
            }
            else
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E;
            }
            break;
        case CPSS_DXCH_TABLE_FDB_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
                !PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E;
            }
            break;
        case CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
                !PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E;
            }
            break;

        case CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
                !PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E;
            }
            break;

        case CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E:
            *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E;
            break;

        case CPSS_DXCH_SIP6_TABLE_PBR_E:
            if (!PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E;
            }
            else
            {
                *hwTableTypePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E;
            }
            break;

        default:
            /* special cases for LMU tables those have instance per chiplet die
               and differs from port group model. These HW tables are mapped to one RAM.
               DI APIs use first HW table for mapping so far. */
            if ((hwTableType > CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E) &&
                (hwTableType <= CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_LAST_E))
            {
                *hwTableTypePtr = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? hwTableType : CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E;
            }
            else if ((hwTableType > CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E) &&
                     (hwTableType <= CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_LAST_E))
            {
                *hwTableTypePtr = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? hwTableType : CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E;
            }

            break;
    }

    if (GT_OK == rc)
    {
        if (offsetPtr)
        {
            *offsetPtr = offset;
        }

        if (maxEntriesPtr)
        {
            *maxEntriesPtr = maxEntries;
        }

    }
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityHwTableCheck function
* @endinternal
*
* @brief   Function checks table avalability for test based on DFX injection error.
*
* @note   APPLICABLE DEVICES:      AC5; Bobcat2; Aldrin; AC3X; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] tableType                - table type
*
* @param[out] memTypePtr               - (pointer to) memory type
* @param[out] numOfEntriesPtr          - (pointer to) number of table entries
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported tableType
* @retval GT_NOT_FOUND             - on not found tableType in RAM DB
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on other error
*/
GT_STATUS prvCpssDxChDiagDataIntegrityHwTableCheck
(
    IN GT_U8   devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      *memTypePtr,
    OUT GT_U32                                          *numOfEntriesPtr
)
{
    GT_U32                                         i, offset;
    GT_BOOL                                        retVal;
    GT_STATUS                                      rc;
    GT_U32                                         tableType_U32;
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr;
    GT_U32                                         dbArrayEntryNum;
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT      dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(memTypePtr);

    /* LPM supported in SIP_6, but entry#0 may be not valid. Skip check for LPM. */
    if (!((CPSS_DXCH_SIP5_TABLE_LPM_MEM_E == tableType) && PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        retVal = prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum, tableType, 0);
        if (retVal == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    tableType_U32 = tableType;
    /* Check special cases */
    rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &tableType_U32, &offset, numOfEntriesPtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    tableType = tableType_U32;

    /* assign DB pointer and size */
    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
            {
                /* should not happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                if(dbArrayPtr[i].hwTableName == (GT_U32)tableType)
                {
                    *memTypePtr = dbArrayPtr[i+offset].memType;
                    return GT_OK;
                }
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);
        if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
        {
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* loop over all memories in DB */
        for(i = 0; i < dbArrayEntryNum; i++)
        {
            if(dbArrayPtr[i].hwTableName == (GT_U32)tableType)
            {
                *memTypePtr = dbArrayPtr[i+offset].memType;
                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChDiagDataIntegrityEccInterruptsMask function
* @endinternal
*
* @brief   Mask/Unmask (i.e disable/enable) interrupt(s) responsible
*         for specified ECC error types signalling.
*         IMPORTANT!
*         There is no protection against multiple simultaneous interrupt registers
*         access. Calling code is responsible for locking/unlocking!
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] errorType                - ECC error type: <single> / <multiple> / <single+multiple>
* @param[in] maskEn                   - mask (GT_TRUE) or unmask (GT_FALSE) interrupt(s)
* @param[in] intEccSingle             - interrupt number raised in case of single ECC error
* @param[in] intEccDouble             - interrupt number raised in case of double ECC error
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad param
*
* @note Calling code is responsible to locking/unlocking!
*
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEccInterruptsMask
(
    IN GT_U8                                         devNum,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT errorType,
    IN GT_BOOL                                       maskEn,
    IN GT_U32                                        intEccSingle,
    IN GT_U32                                        intEccDouble
)
{
    GT_STATUS rc = GT_OK;
    switch (errorType) {
        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
            rc = prvCpssDrvEventsMask(devNum, intEccSingle, maskEn);
            if (GT_OK == rc)
            {
                rc = prvCpssDrvEventsMask(devNum, intEccDouble, maskEn);
            }
            break;
        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
            rc = prvCpssDrvEventsMask(devNum, intEccDouble, maskEn);
            break;
        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
            rc = prvCpssDrvEventsMask(devNum, intEccSingle, maskEn);
            break;
        default:
            rc = GT_BAD_PARAM;
    }
    return rc;
}



/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat2 function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */
    GT_U32      upperIntIndex;  /* upper HW interrupt index (for the loop) */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
            {
                intIndex      = PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                upperIntIndex = PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
            }
            else
            {
                /* bobcat2 b0 */
                intIndex      = PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                upperIntIndex = PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E;
            };

            while (intIndex <= upperIntIndex && rc == GT_OK)
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex++, intMaskEn);
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,
                             PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                         PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_SINGLE_ERROR_E,
                         PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_DOUBLE_ERROR_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
            if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
            {
                /* memory is not protected in bobcat2 a0 revision */
                rc = GT_BAD_PARAM;
            }
            else
            {
                /* bobcat2 b0 */
                PRV_CPSS_INT_SCAN_LOCK();
                rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                         PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E,
                         PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E);
                if (rc == GT_OK)
                {
                    rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E,
                             PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E);
                }
                PRV_CPSS_INT_SCAN_UNLOCK();
            }
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_BOBCAT2_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            /* PARITY */
            intIndex = PRV_CPSS_BOBCAT2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        default:
            rc = prvIsMemProtectedButNotSupportedSip5(devNum, memType) ?
                GT_NOT_SUPPORTED : GT_BAD_PARAM;
            break;
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat2 function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_BOBCAT2_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_BOBCAT2_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            /* read for BM core 0 only. cores 0..5 (bobcat a0) or 0..3 (bobcat b0)
             * are expected to store the same values */
            intIndex = PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum) ?
                PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E :
                PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            intIndex      = PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
            intIndex2     = PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
            intIndex      = PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_SINGLE_ERROR_E;
            intIndex2     = PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_DOUBLE_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
            /* read ECC_0 only. ECC_1 is expected to store the same value */
            intIndex  = PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E;
            intIndex2 = PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_BOBCAT2_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_BOBCAT2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            break;
        default:
            return (prvIsMemProtectedButNotSupportedSip5(devNum, memType) ?
                    GT_NOT_SUPPORTED : GT_BAD_PARAM);
            break;
    }

    if (intIndex == PRV_CPSS_BOBCAT2_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_BOBCAT2_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskLion2 function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskLion2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
            intIndex = PRV_CPSS_LION2_TCC_UPPER_GEN_TCAM_ERROR_DETECTED_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
            intIndex = PRV_CPSS_LION2_TCC_LOWER_GEN_TCAM_ERROR_DETECTED_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E:
            if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E)
            {
                intIndex = PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_ONE_ERROR_E;
            }
            else if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E)
            {
                intIndex = PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_TWO_OR_MORE_ERRORS_E;
            }
            else if (errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)
            {
                intIndex = PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_ONE_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_TWO_OR_MORE_ERRORS_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E:
            /* TxQ EGR 0 */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_0_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* TxQ EGR 1 */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_1_GEN_MC_FIFO_0_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E:
            /* TxQ EGR 0 */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_1_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* TxQ EGR 1 */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_1_GEN_MC_FIFO_1_PARITY_ERROR_E;

            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:
            /* TxQ EGR 0 */

            /* false alarm for IPMC routed packets */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_3_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, GT_TRUE);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* TxQ EGR 1 */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_1_GEN_MC_FIFO_3_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_EGR_VLAN_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_ING_VLAN_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_L2_PORT_ISOLATION_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_L3_PORT_ISOLATION_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_VIDX_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E:
            /* ingress PLR0 */
            intIndex = PRV_CPSS_LION2_IPLR0_DATA_ERROR_E;

            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* ingress PLR1 */
            intIndex = PRV_CPSS_LION2_IPLR1_DATA_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E:
            intIndex = PRV_CPSS_LION2_IP_ROUTER_NHE_DIP_DATA_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            intIndex = PRV_CPSS_LION2_IP_ROUTER_NHE_SIP_DATA_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E:
            intIndex = PRV_CPSS_LION2_IP_ROUTER_STG2_DATA_FIFO_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E:
            intIndex = PRV_CPSS_LION2_IP_ROUTER_STG4_DATA_FIFO_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E:
            intIndex = PRV_CPSS_LION2_IP_ROUTER_UNUSED_DATA_FIFO_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E:
            if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E)
            {
                intIndex = PRV_CPSS_LION2_MPPM_0_BK0_ECC_1_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_MPPM_0_BK1_ECC_1_ERROR_E;
                rc  = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_MPPM_1_BK0_ECC_1_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_MPPM_1_BK1_ECC_1_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);

                return rc;
            }
            else if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E)
            {
                intIndex = PRV_CPSS_LION2_MPPM_0_BK0_ECC_2_OR_MORE_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_MPPM_0_BK1_ECC_2_OR_MORE_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_MPPM_1_BK0_ECC_2_OR_MORE_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                if(GT_OK != rc)
                {
                    return rc;
                }

                intIndex = PRV_CPSS_LION2_MPPM_1_BK1_ECC_2_OR_MORE_ERROR_E;
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                return rc;
            }
            else if (errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)
            {
                rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskLion2(devNum, memType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E, operation);
                if(GT_OK != rc)
                {
                    return rc;
                }

                return prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskLion2(devNum, memType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E, operation);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:

            /* MASK false interrupt */
            intMaskEn = GT_TRUE;
            intIndex = PRV_CPSS_LION2_TXQ_DQ_MEMORY_ERROR_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E:
            intIndex = PRV_CPSS_LION2_TXQ_DQ_MEMORY_ERROR_STATE_VARIABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E:
            intIndex = PRV_CPSS_LION2_TXQ_LL_GEN_FBUF_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_QCN_CN_BUFFER_FIFO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP0_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP1_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP2_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP3_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP4_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP5_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP6_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP7_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_BMA_CORE0_MCCNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            intIndex = PRV_CPSS_LION2_BMA_CORE1_MCCNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            intIndex = PRV_CPSS_LION2_BMA_CORE2_MCCNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            if(GT_OK != rc)
            {
                return rc;
            }

            intIndex = PRV_CPSS_LION2_BMA_CORE3_MCCNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);

            return rc;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetLion2 function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetLion2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_BOOL     intEnable; /* interrupt enable/disable */
    GT_BOOL     intEnable1; /* interrupt enable/disable */
    GT_U32      intIndex; /* HW interrupt index */

    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
            intIndex = PRV_CPSS_LION2_TCC_UPPER_GEN_TCAM_ERROR_DETECTED_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
            intIndex = PRV_CPSS_LION2_TCC_LOWER_GEN_TCAM_ERROR_DETECTED_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E:

            intIndex = PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_ONE_ERROR_E;
            prvCpssDrvEventsMaskGet(devNum, intIndex, &intEnable);

            intIndex = PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_TWO_OR_MORE_ERRORS_E;
            prvCpssDrvEventsMaskGet(devNum, intIndex, &intEnable1);

            /* reversed logic from MaskSet */
            if(intEnable == GT_TRUE)
            {
                if(intEnable1 == GT_TRUE)
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                    *operationPtr = CPSS_EVENT_UNMASK_E;
                }
                else
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                    *operationPtr = CPSS_EVENT_UNMASK_E;
                }
            }
            else
            {
                if(intEnable1 == GT_TRUE)
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                    *operationPtr = CPSS_EVENT_UNMASK_E;
                }
                else
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                    *operationPtr = CPSS_EVENT_MASK_E;
                }
            }
            return GT_OK;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E:
            /* TxQ EGR 0 and TxQ EGR 1 - the same value */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_0_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E:
            /* TxQ EGR 0 and TxQ EGR 1 - the same value */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_1_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:
            /* TxQ EGR 0 and TxQ EGR 1 - the same value */
            intIndex = PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_3_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_EGR_VLAN_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_ING_VLAN_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_L2_PORT_ISOLATION_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_L3_PORT_ISOLATION_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E:
            intIndex = PRV_CPSS_LION2_TXQ_SHT_GEN_VIDX_TBL_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E:
            intIndex = PRV_CPSS_LION2_IPLR0_DATA_ERROR_E;
            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E:
            /* NHE_DIP_DATA and NHE_SIP_DATA - the same value */
            intIndex = PRV_CPSS_LION2_IP_ROUTER_NHE_DIP_DATA_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E:
            intIndex = PRV_CPSS_LION2_IP_ROUTER_STG2_DATA_FIFO_ECC_ERROR_E;
            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E:
            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
            intIndex = PRV_CPSS_LION2_IP_ROUTER_STG4_DATA_FIFO_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E:
            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
            intIndex = PRV_CPSS_LION2_IP_ROUTER_UNUSED_DATA_FIFO_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E:

            intIndex = PRV_CPSS_LION2_MPPM_0_BK0_ECC_1_ERROR_E;
            prvCpssDrvEventsMaskGet(devNum, intIndex, &intEnable);

            intIndex = PRV_CPSS_LION2_MPPM_0_BK0_ECC_2_OR_MORE_ERROR_E;
            prvCpssDrvEventsMaskGet(devNum, intIndex, &intEnable1);

            /* reversed logic from MaskSet */
            if(intEnable == GT_TRUE)
            {
                if(intEnable1 == GT_TRUE)
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                    *operationPtr = CPSS_EVENT_UNMASK_E;
                }
                else
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                    *operationPtr = CPSS_EVENT_UNMASK_E;
                }
            }
            else
            {
                if(intEnable1 == GT_TRUE)
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                    *operationPtr = CPSS_EVENT_UNMASK_E;
                }
                else
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                    *operationPtr = CPSS_EVENT_MASK_E;
                }
            }
            return GT_OK;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_LION2_TXQ_DQ_MEMORY_ERROR_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E:
            intIndex = PRV_CPSS_LION2_TXQ_DQ_MEMORY_ERROR_STATE_VARIABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E:
            intIndex = PRV_CPSS_LION2_TXQ_LL_GEN_FBUF_ECC_ERROR_E;
            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_QCN_CN_BUFFER_FIFO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP0_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP1_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP2_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP3_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP4_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP5_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP6_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP7_COUNTERS_ERR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E:
            intIndex = PRV_CPSS_LION2_BMA_CORE0_MCCNT_PARITY_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E:
            /* there are no interrupts for MLL memories */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);


        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEnable);

    *operationPtr = (intEnable == GT_TRUE) ? CPSS_EVENT_UNMASK_E : CPSS_EVENT_MASK_E;

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5 function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_FAIL; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E:
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_AC5_BM_1_BANK0_ONE_ECC_ERROR_E,
                             PRV_CPSS_AC5_BM_1_BANK0_TWO_OR_MORE_ECC_ERRORS_E);
            if(GT_OK != rc)
            {
                return rc;
            }

            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_AC5_BM_1_BANK1_ONE_ECC_ERROR_E,
                             PRV_CPSS_AC5_BM_1_BANK1_TWO_OR_MORE_ECC_ERRORS_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E:
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_FIXED_ECC_ERROR_E,
                             PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_UNFIXED_ECC_ERROR_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E:
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_AC5_TXQ_GENERAL_FLL_FIXED_ECC_ERROR_E,
                             PRV_CPSS_AC5_TXQ_GENERAL_FLL_UNFIXED_ECC_ERROR_E);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;

}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */
    GT_U32      upperIntIndex;  /* upper HW interrupt index (for the loop) */

    /* Unused parameter.
     * errorType is used only with memories with not-DFX ECC protection . */
    (void)errorType;


    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
                intIndex      = PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                upperIntIndex = PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
            while (intIndex <= upperIntIndex && rc == GT_OK)
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex++, intMaskEn);
            }
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,
                             PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_ALDRIN_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            /* PARITY */
            intIndex = PRV_CPSS_ALDRIN_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        default:
            rc = GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_ALDRIN_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_ALDRIN_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            /* read for BM core 0 only. cores 0..5 are expected to store the same values */
            intIndex = PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            intIndex      = PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
            intIndex2     = PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_ALDRIN_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_ALDRIN_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    if (intIndex == PRV_CPSS_ALDRIN_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_ALDRIN_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5 function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Aldrin; AC3X; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_AC5_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_AC5_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E:
            intIndex      = PRV_CPSS_AC5_BM_1_BANK1_ONE_ECC_ERROR_E;
            intIndex2     = PRV_CPSS_AC5_BM_1_BANK1_TWO_OR_MORE_ECC_ERRORS_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E:
            intIndex      = PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_FIXED_ECC_ERROR_E;
            intIndex2     = PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_UNFIXED_ECC_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E:
            intIndex      = PRV_CPSS_AC5_TXQ_GENERAL_FLL_FIXED_ECC_ERROR_E;
            intIndex2     = PRV_CPSS_AC5_TXQ_GENERAL_FLL_UNFIXED_ECC_ERROR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (intIndex == PRV_CPSS_AC5_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_AC5_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxInterruptPipeClientGet function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - portGroup number. Ignored in single-core devices
* @param[in] dfxInt                   - DFX interrupt number
*                                      eventsNumPtr  - (pointer to) max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsTypeArr[] and
*                                      memTypeArr[] can retrieve.
*
* @param[out] pipePtr                  -   pipe number
* @param[out] clientPtr                - client number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown device family or wrong dfxInt number
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxInterruptPipeClientGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 dfxInt,
    OUT GT_U32 *pipePtr,
    OUT GT_U32 *clientPtr
)
{
    GT_STATUS rc = GT_OK;
    const PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC *mapArr;
    GT_U32 ix;

    /* unused parameter */
    (void)portGroupId;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily) {

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
            {
                /* bobcat2 */
                if (dfxInt >  PRV_CPSS_BOBCAT2_DFX_SUM_DFX_INTERRUPT_SUMMARY_E &&
                    dfxInt <= PRV_CPSS_BOBCAT2_DFX_SUM_DFX_INTERRUPT_SUM_31_E)
                {
                    ix     = dfxInt - PRV_CPSS_BOBCAT2_DFX_SUM_DFX_INTERRUPT_SUMMARY_E;
                    mapArr = bobcat2DfxIntToPipeClientMapArr;
                }
                else if (dfxInt >  PRV_CPSS_BOBCAT2_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E &&
                         dfxInt <= PRV_CPSS_BOBCAT2_DFX1_SUM_DFX_1_INTERRUPT_SUM_31_E)
                {
                    ix     = dfxInt - PRV_CPSS_BOBCAT2_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E;
                    mapArr = bobcat2Dfx1IntToPipeClientMapArr;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *pipePtr   = mapArr[ix].dfxPipeIndex;
                *clientPtr = mapArr[ix].dfxClientIndex;

            }
            else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                /* bobk */
                if (dfxInt >  PRV_CPSS_BOBK_DFX_SUM_DFX_INTERRUPT_SUMMARY_E &&
                    dfxInt <= PRV_CPSS_BOBK_DFX_SUM_DFX_INTERRUPT_SUM_31_E)
                {
                    ix     = dfxInt - PRV_CPSS_BOBK_DFX_SUM_DFX_INTERRUPT_SUMMARY_E;
                    mapArr = bobkDfxIntToPipeClientMapArr;
                }
                else if (dfxInt >  PRV_CPSS_BOBK_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E &&
                         dfxInt <= PRV_CPSS_BOBK_DFX1_SUM_DFX_1_INTERRUPT_SUM_31_E)
                {
                    ix     = dfxInt - PRV_CPSS_BOBK_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E;
                    mapArr = bobkDfx1IntToPipeClientMapArr;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *pipePtr   = mapArr[ix].dfxPipeIndex;
                *clientPtr = mapArr[ix].dfxClientIndex;
            }
            else
            {
                rc = GT_NOT_IMPLEMENTED;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            if (dfxInt >  PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUMMARY_E &&
                dfxInt <= PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUM_31_E)
            {
                ix     = dfxInt - PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUMMARY_E;
                mapArr = aldrinDfxIntToPipeClientMapArr;
            }
            else if (dfxInt >  PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E &&
                     dfxInt <= PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUM_31_E)
            {
                ix     = dfxInt - PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E;
                mapArr = aldrinDfx1IntToPipeClientMapArr;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            *pipePtr   = mapArr[ix].dfxPipeIndex;
            *clientPtr = mapArr[ix].dfxClientIndex;
            break;

        default:
            rc = GT_NOT_IMPLEMENTED;
            break;
    }

    return rc;
}



/**
* @internal prvCpssDxChDiagDataIntegrityDfxInterruptCheckAndRegGet function
* @endinternal
*
* @brief   Check if interrupt is DFX interrupt and (optionally) return appropriate
*         DFX Interrupt Cause Register and/or DFX Interrupt Mask Register
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
*                                      portGroupId   - portGroup number
* @param[in] intNum                   - interrupt number
*
* @param[out] causeRegAddrPtr          - (pointer to) DFX/DFX1 Interrupt Cause register address.
*                                      Can be null.
* @param[out] maskRegAddrPtr           - (array of) Interrupt Mask address
*                                      Can be null.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_STATE             - illegal HW state
* @retval GT_OUT_OF_RANGE          - if intNum is not DFX interrupt number
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxInterruptCheckAndRegGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    OUT GT_U32                                      *causeRegAddrPtr,
    OUT GT_U32                                      *maskRegAddrPtr

)
{
    GT_U32 dfxStart;
    GT_U32 dfx1Start;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily) {

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
            {
                /* bobcat2 */
                dfxStart  = PRV_CPSS_BOBCAT2_DFX_SUM_DFX_INTERRUPT_SUMMARY_E;
                dfx1Start = PRV_CPSS_BOBCAT2_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E;
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily ==
                     CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                /* bobk */
                dfxStart  = PRV_CPSS_BOBK_DFX_SUM_DFX_INTERRUPT_SUMMARY_E;
                dfx1Start = PRV_CPSS_BOBK_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            dfxStart  = PRV_CPSS_BOBCAT3_DFX_SUM_DFX_DFX_INTERRUPT_SUMMARY_E;
            dfx1Start = PRV_CPSS_BOBCAT3_DFX1_DFX_1_INTERRUPT_SUMMARY_E;
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            dfxStart  = PRV_CPSS_ALDRIN2_DFX_SUM_DFX_DFX_INTERRUPT_SUMMARY_E;
            dfx1Start = PRV_CPSS_ALDRIN2_DFX1_DFX_1_INTERRUPT_SUMMARY_E;
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            dfxStart  = PRV_CPSS_ALDRIN_DFX_SUM_DFX_INTERRUPT_SUMMARY_E;
            dfx1Start = PRV_CPSS_ALDRIN_DFX1_SUM_DFX_1_INTERRUPT_SUMMARY_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    if (intNum > dfxStart && intNum <= dfxStart+32)
    {
        if (causeRegAddrPtr != NULL)
        {
            *causeRegAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptCause;
        }

        if (maskRegAddrPtr != NULL)
        {
            *maskRegAddrPtr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptMask;
        }
    }
    else if (intNum > dfx1Start && intNum <= dfx1Start+32)
    {

        if (causeRegAddrPtr != NULL)
        {
            *causeRegAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfx1InterruptCause;
        }
        if (maskRegAddrPtr != NULL)
        {
            *maskRegAddrPtr  = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfx1InterruptMask;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMask function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_IMPLEMENTED       - if feature is not implemented for the device yet
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMask
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc; /* return code */

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_LION2_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskLion2(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
            {
                /* bobcat2 */
                rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat2(
                    devNum, memType, errorType, operation);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily ==
                     CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                /* bobk */
                rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobk(
                    devNum, memType, errorType, operation);
            }
            else
            {
                rc = GT_NOT_SUPPORTED;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat3(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin2(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskFalcon(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5x(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5p(
                    devNum, memType, errorType, operation);
            break;

        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskHarrier(
                    devNum, memType, errorType, operation);
            break;

        default:
            rc = GT_NOT_IMPLEMENTED;
            break;
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMask function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity DFX interrupt.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Falcon.
*
* @param[in] devNum                   - PP device number
* @param[in] memInfoPtr               - (pointer to) item of Device Data Integrity DB(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC).
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMask
(
    IN GT_U8                                            devNum,
    IN const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC     *memInfoPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_U32 pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC   (memInfoPtr->key);
    GT_U32 clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC (memInfoPtr->key);
    GT_U32 memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC (memInfoPtr->key);
    GT_STATUS rc;       /* return code          */
    GT_U32 fieldOffset; /* field offset in bits */
    GT_U32 fieldLength; /* field length in bits */
    GT_U32 regAddr;     /* address of register  */
    GT_U32 regData;     /* data of register     */

    /* prepare register Data */
    if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
        switch(errorType)
        {
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                fieldOffset = 2;
                fieldLength = 1;
                regData = operation;
                break;

            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                fieldOffset = 3;
                fieldLength = 1;
                regData = operation;
                break;

            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
                fieldOffset = 2;
                fieldLength = 2;
                regData = (operation == CPSS_EVENT_MASK_E) ? 0 : 3;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
        fieldOffset = 1;
        fieldLength = 1;
        regData = operation;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

     /* AC5 use SIP_6 based management system */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ||
        (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* use Tile #0 for SIP_6 devices */
        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[0].memoryInterruptMask;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS;
    }

    /* write to Memory Interrupt Mask Register */
    rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
               regAddr, fieldOffset, fieldLength, regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* it's take some time in DFX bus to configure register.
       Add this Read to guaranty that write done before exit from this function. */
    rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
               regAddr, fieldOffset, fieldLength, &regData);

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMaskFalcon function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity DFX interrupt.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin;
*
* @param[in] devNum                   - PP device number
* @param[in] memInfoPtr               - (pointer to) item of Device Data Integrity DB(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC).
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - Tile index
* @param[in] chipletIndex             - Chiplet index
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMaskFalcon
(
    IN  GT_U8                                           devNum,
    IN  const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *memInfoPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType,
    IN  GT_U32                                          tileIndex,
    IN  GT_U32                                          chipletIndex
)
{
    GT_U32      regAddr;    /* register address */

    GT_U32 pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC   (memInfoPtr->key);
    GT_U32 clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC (memInfoPtr->key);
    GT_U32 memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC (memInfoPtr->key);
    GT_STATUS rc;
    GT_U32 fieldOffset;
    GT_U32 fieldLength;
    GT_U32 regData;

    /* prepare register Data */
    if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
        switch(errorType)
        {
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                fieldOffset = 2;
                fieldLength = 1;
                regData = operation;
                break;

            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                fieldOffset = 3;
                fieldLength = 1;
                regData = operation;
                break;

            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
                fieldOffset = 2;
                fieldLength = 2;
                regData = (operation == CPSS_EVENT_MASK_E) ? 0 : 3;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
        fieldOffset = 1;
        fieldLength = 1;
        regData = operation;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (dfxInstanceType)
    {
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
            sip6_tile_DFXRam[tileIndex].memoryInterruptMask;
            break;
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
            sip6_chiplet_DFXRam[tileIndex*4 + chipletIndex].memoryInterruptMask;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* write to Memory Interrupt Mask Register */
    rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
        regAddr, fieldOffset, fieldLength, regData, dfxInstanceType, tileIndex, chipletIndex);
    if(GT_OK != rc)
    {
      return rc;
    }

    /* it's take some time in DFX bus to configure register.
       Add this Read to guaranty that write done before exit from this function. */
    rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
              regAddr, fieldOffset, fieldLength, dfxInstanceType, tileIndex, chipletIndex,  &regData);

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGet function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS rc;

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5(
                    devNum, memType, errorTypePtr, operationPtr);
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetLion2(
                    devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
            {
                /* bobcat2 */
                rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat2(
                    devNum, memType, errorTypePtr, operationPtr);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily ==
                     CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                /* bobk */
                rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobk(
                    devNum, memType, errorTypePtr, operationPtr);
            }
            else
            {
                rc = GT_NOT_SUPPORTED;
            }

            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin(
                    devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat3(
                devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin2(
                devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetFalcon(
                devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5x(
                devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5p(
                devNum, memType, errorTypePtr, operationPtr);
            break;

        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetHarrier(
                devNum, memType, errorTypePtr, operationPtr);
            break;

        default:
            rc = GT_NOT_IMPLEMENTED;
            break;

    }

    return rc;
}


/**
* @internal dataIntegrityDbCmp function
* @endinternal
*
* @brief   Function for compare two DB entries for binary search algorithm.
*
* @param[in] aPtr                     - (pointer to) key
* @param[in] bPtr                     - (pointer to) DB item key
*
* @retval 0                        - equal
* @retval else                     - not equal
*/
static int dataIntegrityDbCmp
(
    IN const GT_VOID *aPtr,
    IN const GT_VOID *bPtr
)
{
    int temp1 = *((int*)aPtr);
    int temp2 = (int)(((PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC*)bPtr)->key);

    return temp1 -  temp2;
}

/**
* @internal searchMemType function
* @endinternal
*
* @brief   Function performs binary search in map DB.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] dfxPipeIndex             - DFX pipe index
* @param[in] dfxClientIndex           - DFX client index
* @param[in] dfxMemoryIndex           - DFX memory index
*
* @param[out] dbItemPtrPtr             - (pointer to) DB entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS searchMemType
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          dfxPipeIndex,
    IN  GT_U32                                          dfxClientIndex,
    IN  GT_U32                                          dfxMemoryIndex,
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC  const **dbItemPtrPtr
)
{
    GT_STATUS                           rc;    /* return code */
    GT_U32                              key;   /* compare key */
    GT_U32                              itemSize; /* size of compared item */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC   *dbItemTmpPtr; /* pointer to found item */
    CPSS_OS_COMPARE_ITEMS_FUNC_PTR      cmpFuncPtr; /* pointer to compare function */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */

    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

    cmpFuncPtr = dataIntegrityDbCmp;
    key = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_GENERATE_KEY_MAC(
        dfxPipeIndex, dfxClientIndex, dfxMemoryIndex);

    itemSize = sizeof(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC);

    rc = cpssOsBsearch((void *)&key, (void *)dbArrayPtr, dbArrayEntryNum, itemSize, cmpFuncPtr, (void*)&dbItemTmpPtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The dfx memory was not found");
    }

    *dbItemPtrPtr = dbItemTmpPtr;

    return GT_OK;
}

/**
* @internal searchMemTypeFalcon function
* @endinternal
*
* @brief   Function performs binary search in map DB for falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] dfxPipeIndex             - DFX pipe index
* @param[in] dfxClientIndex           - DFX client index
* @param[in] dfxMemoryIndex           - DFX memory index
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
*
* @param[out] dbItemPtrPtr             - (pointer to) DB entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS searchMemTypeFalcon
(
    IN  GT_U32                                          dfxPipeIndex,
    IN  GT_U32                                          dfxClientIndex,
    IN  GT_U32                                          dfxMemoryIndex,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType,
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC   const **dbItemPtrPtr
)
{
    GT_STATUS                           rc;    /* return code */
    GT_U32                              key;   /* compare key */
    GT_U32                              itemSize; /* size of compared item */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC   *dbItemTmpPtr; /* pointer to found item */
    CPSS_OS_COMPARE_ITEMS_FUNC_PTR      cmpFuncPtr; /* pointer to compare function */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */

    prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);

    cmpFuncPtr = dataIntegrityDbCmp;
    key = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_GENERATE_KEY_MAC(
        dfxPipeIndex, dfxClientIndex, dfxMemoryIndex);

    itemSize = sizeof(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC);

    rc = cpssOsBsearch((void *)&key, (void *)dbArrayPtr, dbArrayEntryNum, itemSize, cmpFuncPtr, (void*)&dbItemTmpPtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The dfx memory was not found");
    }

    *dbItemPtrPtr = dbItemTmpPtr;

    return GT_OK;
}

/**
* @internal prvCpssDxChLion2DiagDataIntegrityDfxMemoryPipeIdSet function
* @endinternal
*
* @brief   Function sets DFX pipe select register.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman, Pipe.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
static GT_STATUS prvCpssDxChLion2DiagDataIntegrityDfxMemoryPipeIdSet
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId
)
{
    GT_U32 regAddr; /* register address */

    /* Set Pipe Select Register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->dfxUnits.server.pipeSelect;

    return prvCpssHwPpWriteRegister(devNum, regAddr, (1 << pipeId));
}

/**
* @internal prvCpssDxChLion2DiagDataIntegrityDfxMemoryRegWrite function
* @endinternal
*
* @brief   Function writes DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
* @param[in] regData                  - register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static GT_STATUS prvCpssDxChLion2DiagDataIntegrityDfxMemoryRegWrite
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    IN GT_U32 regData
)
{
    GT_STATUS rc;                                           /* return code */
    GT_U32 regAddr;                                         /* register address */
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC  addrStr;     /* DFX address struct */

    /* unused parameter */
    (void)pipeId;
    /* Read Memory ID STATUS 0x34 */
    addrStr.dfxUnitClientIdx = clientId;
    addrStr.dfxXsbSelect = 0;
    addrStr.dfxClientSelect = 0;
    addrStr.dfxClientMemoryNumber = memNumber;
    addrStr.dfxClientMemoryRegisterOffset = memReg;

    rc = prvCpssDfxBuildAddr(devNum, &addrStr, &regAddr);
    if(GT_OK != rc)
    {
        return rc;
    }

    return prvCpssHwPpWriteRegister(devNum, regAddr, regData);
}

/**
* @internal prvCpssDxChLion2DiagDataIntegrityDfxClientRegRead function
* @endinternal
*
* @brief   Function reads DFX client registers.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] clientReg                - DFX client register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static GT_STATUS prvCpssDxChLion2DiagDataIntegrityDfxClientRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 clientReg,
    OUT GT_U32 *regDataPtr
)
{
    GT_STATUS rc;                                           /* return code */
    GT_U32 regAddr;                                         /* register address */
    GT_U32 regData;                                         /* register data */
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC   addrStr;    /* DFX address struct */

    /* unused parameter */
    (void)pipeId;

    addrStr.dfxUnitClientIdx = clientId;
    addrStr.dfxXsbSelect = 0;
    addrStr.dfxClientSelect = 1;
    addrStr.dfxClientRegisterOffset = clientReg;

    rc = prvCpssDfxBuildAddr(devNum, &addrStr, &regAddr);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    *regDataPtr = regData;

    return GT_OK;
}

/**
* @internal prvCpssDxChLion2DiagDataIntegrityDfxMemoryRegRead function
* @endinternal
*
* @brief   Function reads DFX memory registers.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - DFX pipe ID
* @param[in] clientId                 - DFX client ID
* @param[in] memNumber                - DFX memory ID
* @param[in] memReg                   - DFX memory register address
*
* @param[out] regDataPtr               - (pointer to) register data
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static GT_STATUS prvCpssDxChLion2DiagDataIntegrityDfxMemoryRegRead
(
    IN GT_U8 devNum,
    IN GT_U32 pipeId,
    IN GT_U32 clientId,
    IN GT_U32 memNumber,
    IN GT_U32 memReg,
    OUT GT_U32 *regDataPtr
)
{
    GT_STATUS rc;                                           /* return code */
    GT_U32 regAddr;                                         /* register address */
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC  addrStr;     /* DFX address struct */

    CPSS_NULL_PTR_CHECK_MAC(regDataPtr);

    /* unused parameter */
    (void)pipeId;

    /* Read Memory ID STATUS 0x34 */
    addrStr.dfxUnitClientIdx = clientId;
    addrStr.dfxXsbSelect = 0;
    addrStr.dfxClientSelect = 0;
    addrStr.dfxClientMemoryNumber = memNumber;
    addrStr.dfxClientMemoryRegisterOffset = memReg;

    rc = prvCpssDfxBuildAddr(devNum, &addrStr, &regAddr);
    if(GT_OK != rc)
    {
        return rc;
    }

    return prvCpssHwPpReadRegister(devNum, regAddr, regDataPtr);
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxEventsViaMgUnitGet function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*         Unlike prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGet
*         the function uses MG unit registers "DFX/DFX1 Interrupts Summary
*         Cause Register" to get pipe/client indexes of occured event.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - portGroup number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaMgUnitGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      portGroupId,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;
    GT_U32    dfxIntCauseReg;   /* DFX/DFX1 Interrupt Cause register address */
    GT_U32    dfxIntMaskReg;    /* DFX/DFX1 Interrupt Mask Register address */
    GT_U32    regData;          /* register data */
    GT_U32    regData2;         /* mask register data */
    GT_U32    pipeIndex;        /* DFX pipe number */
    GT_U32    clientIndex;      /* DFX client number */
    GT_U32    memoryNumber;     /* DFX memory number */
    GT_U32    i;                /* cycle counter */
    GT_U32    j;                /* cycle counter */
    GT_U32    eventCounter = 0; /* events counter */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC clientBmp = {{0,0,0,0}};  /* client status struct */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;

    /* - For DFX interrupt find DFX Pipe index and Client index.
       - Set "Pipe Select" register according to DFX pipe index
       - Read 4 memory bitmap registers according to Client index in current DFX pipe.
       - For every bit which is set (memory number) calculate interrupt cause register address:
             pipe index + client index + memory number => cause/mask register address.
    */

    rc = prvCpssDxChDiagDataIntegrityDfxInterruptPipeClientGet(devNum, portGroupId, intNum,
                                                               &pipeIndex, &clientIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(pipeIndex   == DATA_INTEGRITY_ENTRY_NOT_USED_CNS ||
       clientIndex == DATA_INTEGRITY_ENTRY_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < 4; i++)
    {
        rc = prvCpssDfxClientRegRead(
            devNum, pipeIndex, clientIndex,
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS + i * 4,
            &(clientBmp.bitmap[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChDiagDataIntegrityDfxInterruptCheckAndRegGet(devNum, intNum,
                                               &dfxIntCauseReg, &dfxIntMaskReg);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 32; j++)
        {
            if(U32_GET_FIELD_MAC(clientBmp.bitmap[i], j, 1))
            {
                memoryNumber = i*32 + j;

                rc = prvCpssDxChDiagDfxMemoryEventFromDbGet(
                    devNum, pipeIndex, clientIndex, memoryNumber, &eventsArr[eventCounter]);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if(eventCounter == (*eventsNumPtr - 1))
                {
                    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, dfxIntCauseReg, 31, 1, &regData);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }

                    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, dfxIntMaskReg, 31, 1, &regData2);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }


                    if((regData & regData2) != 0)
                    {
                        *isNoMoreEventsPtr = GT_FALSE;
                    }

                    return GT_OK;
                }

                eventCounter++;
            }
        }
    }
    *eventsNumPtr = eventCounter;

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat2 function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat2
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_BOOL isLocationAssigned; /* if DFX coordinates of memory already calculated */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    isLocationAssigned = GT_FALSE;
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        /* ------ handle interrupts common for all devices revisions------ */

        case PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_SINGLE_ERROR_E:
            /* signals about error in either header_reorder_mem_iddb_mem or payload_reorder_mem_iddb_mem memories.
             * But there is no way to determine which one exactly. Return header_reorder_mem_iddb_mem. */
            eventsArr[0].location.ramEntryInfo.memType   = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_BOBCAT2_TXDMA_GENERAL_SUM_ECC_DOUBLE_ERROR_E:
            /* signals about error in either header_reorder_mem_iddb_mem or payload_reorder_mem_iddb_mem memories.
             * But there is no way to determine which one exactly. Return header_reorder_mem_iddb_mem. */
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        case PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_BOBCAT2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        /* -- _PARITY_ -- */

        case PRV_CPSS_BOBCAT2_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    =  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.tcamParityErrorAddr;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }
            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 0, 1) == 0) ?
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E :
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

            eventsArr[0].location.tcamMemLocation.ruleIndex =
                U32_GET_FIELD_MAC(regData, 13, 4) /* floor */  * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
                U32_GET_FIELD_MAC(regData, 5, 8)  /* row */    * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +
                U32_GET_FIELD_MAC(regData, 1, 4)  /* bank */;

            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;


        case PRV_CPSS_BOBCAT2_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupt is out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBCAT2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        default:

            /* -------------- handle device revision specific interrupts ------------- */

            if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
            {
                switch(intNum)
                {
                /* bobcat2 a0 */
                    case PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E:
                        eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
                        eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                        break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                /* bobcat2 b0 */
                switch(intNum)
                {
                    case PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E:
                    case PRV_CPSS_BOBCAT2_B0_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E:
                        eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
                        eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                        break;

                    case PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_SINGLE_ERROR_E:
                    case PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_SINGLE_ERROR_E:
                        /* signals about error in either payload_iddb_mem or header_iddb_mem memories.
                         * But there is no way to determine which one exactly. Return header_iddb_mem. */
                        eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E;
                        eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                        break;

                    case PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_0_DOUBLE_ERROR_E:
                    case PRV_CPSS_BOBCAT2_TX_FIFO_GENERAL1_SUM_ECC_1_DOUBLE_ERROR_E:
                        /* signals about error in either payload_iddb_mem or header_iddb_mem memories.
                         * But there is no way to determine which one exactly. Return header_iddb_mem. */
                        eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E;
                        eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                        break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */

    if (GT_TRUE == isLocationAssigned)
    {
        /* look for the DB item with specified DFX coordinates */
        rc = searchMemType(devNum,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The Data Integrity DB item not found");
        }
    }
    else
    {
        /* look for the first DB item with specified memory type */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
        if (NULL == dbPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                  "Data Integrity DB not found for the device");
        }

        memType = eventsArr[0].location.ramEntryInfo.memType;
        for (i = 0; i < dbSize && dbPtr[i].memType != (GT_U32)memType; i++ );
        if (i == dbSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                     "The Data Integrity DB item is not found");
        }

        dbItemPtr = &dbPtr[i];

        /* assign DFX coordinates */
        eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    }

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_BOOL isLocationAssigned; /* if DFX coordinates of memory already calculated */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    isLocationAssigned = GT_FALSE;
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_ALDRIN_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        /* -- _PARITY_ -- */

        case PRV_CPSS_ALDRIN_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    =  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.tcamParityErrorAddr;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }
            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 0, 1) == 0) ?
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E :
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

            eventsArr[0].location.tcamMemLocation.ruleIndex =
                U32_GET_FIELD_MAC(regData, 13, 4) /* floor */  * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
                U32_GET_FIELD_MAC(regData, 5, 8)  /* row */    * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +
                U32_GET_FIELD_MAC(regData, 1, 4)  /* bank */;

            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;

        case PRV_CPSS_ALDRIN_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupt is out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_ALDRIN_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */

    if (GT_TRUE == isLocationAssigned)
    {
        /* look for the DB item with specified DFX coordinates */
        rc = searchMemType(devNum,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The Data Integrity DB item not found");
        }
    }
    else
    {
        /* look for the first DB item with specified memory type */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
        if (NULL == dbPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                  "Data Integrity DB not found for the device");
        }

        memType = eventsArr[0].location.ramEntryInfo.memType;
        for (i = 0; i < dbSize && dbPtr[i].memType != (GT_U32)memType; i++ );
        if (i == dbSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                     "The Data Integrity DB item is not found");
        }

        dbItemPtr = &dbPtr[i];

        /* assign DFX coordinates */
        eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    }

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5 function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_U32 ruleOffset;
    GT_BOOL isLocationAssigned; /* if DFX coordinates of memory already calculated */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    isLocationAssigned = GT_FALSE;
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;
    eventsArr[0].location.portGroupsBmp = 0;

    switch(intNum)
    {
        case PRV_CPSS_AC5_BM_1_BANK0_TWO_OR_MORE_ECC_ERRORS_E:
        case PRV_CPSS_AC5_BM_1_BANK1_TWO_OR_MORE_ECC_ERRORS_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            eventsArr[0].location.mppmMemLocation.bankId = (intNum == PRV_CPSS_AC5_BM_1_BANK0_TWO_OR_MORE_ECC_ERRORS_E) ? 0 : 1;
            eventsArr[0].location.mppmMemLocation.mppmId = 0;
            eventsArr[0].location.mppmMemLocation.portGroupId = 0;
            break;

        case PRV_CPSS_AC5_BM_1_BANK0_ONE_ECC_ERROR_E:
        case PRV_CPSS_AC5_BM_1_BANK1_ONE_ECC_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            eventsArr[0].location.mppmMemLocation.bankId = (intNum == PRV_CPSS_AC5_BM_1_BANK0_ONE_ECC_ERROR_E) ? 0 : 1;
            eventsArr[0].location.mppmMemLocation.mppmId = 0;
            eventsArr[0].location.mppmMemLocation.portGroupId = 0;
            break;

        case PRV_CPSS_AC5_TXQ_GENERAL_FLL_UNFIXED_ECC_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        case PRV_CPSS_AC5_TXQ_GENERAL_FLL_FIXED_ECC_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_UNFIXED_ECC_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        case PRV_CPSS_AC5_TXQ_GENERAL_DESC_LL_FIXED_ECC_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_AC5_TCC_LOWER_TCAM_ERROR_DETECTED_E:
        case PRV_CPSS_AC5_TCC1_LOWER_TCAM_ERROR_DETECTED_E:
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            if (intNum == PRV_CPSS_AC5_TCC_LOWER_TCAM_ERROR_DETECTED_E)
            {
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E;
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tccErrorInformation[0];
            }
            else
            {
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_1_E;
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tccErrorInformation[1];
            }
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 30, 1) == 0) ? CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E : CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

            if(U32_GET_FIELD_MAC(regData, 12, 4) != 0)
            {
                ruleOffset = 3;
            }
            else if(U32_GET_FIELD_MAC(regData, 8, 4) != 0)
            {
                ruleOffset = 2;
            }
            else if(U32_GET_FIELD_MAC(regData, 4, 4) != 0)
            {
                ruleOffset = 1;
            }
            else
            {
                ruleOffset = 0;
            }
            eventsArr[0].location.tcamMemLocation.ruleIndex = (U32_GET_FIELD_MAC(regData, 16, 14)) * 4 + ruleOffset;
            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;

        case PRV_CPSS_AC5_TCC_UPPER_TCAM_ERROR_DETECTED_E:
            eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTccErrorInformation;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 30, 1) == 0) ? CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E : CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;
            eventsArr[0].location.tcamMemLocation.ruleIndex = U32_GET_FIELD_MAC(regData, 16, 14);
            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (eventsArr[0].location.isTcamInfoValid == GT_TRUE)
    {
        /* there is no more info for TCAM */
        return GT_OK;
    }

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */

    if (GT_TRUE == isLocationAssigned)
    {
        /* look for the DB item with specified DFX coordinates */
        rc = searchMemType(devNum,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The Data Integrity DB item not found");
        }
    }
    else
    {
        /* look for the first DB item with specified memory type */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
        if (NULL == dbPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                  "Data Integrity DB not found for the device");
        }

        memType = eventsArr[0].location.ramEntryInfo.memType;
        for (i = 0; i < dbSize && dbPtr[i].memType != (GT_U32)memType; i++ );
        if (i == dbSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                     "The Data Integrity DB item is not found");
        }

        dbItemPtr = &dbPtr[i];

        /* assign DFX coordinates */
        eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    }

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetSip5 function
* @endinternal
*
* @brief   Function performs interrupt tree scan for DataIntegrity event details
*         for SIP5 devices.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetSip5
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;   /* return code */
    GT_U32 portGroupId;     /* port group id */
    GT_U32 intNum;          /* interrupt number */

    portGroupId = evExtData >> 16;
    intNum      = evExtData & 0xFFFF;

    if ((intNum == PRV_CPSS_BOBCAT3_FUNCTIONAL1_SERVER_INT_E &&
         PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
        (intNum == PRV_CPSS_ALDRIN2_FUNCTIONAL1_SERVER_INT_E &&
         PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        /* handle DFX events starting with reading DFX register
         * "Server Interrupt Summary Cause register" */
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGet(
            devNum, eventsNumPtr, eventsArr, isNoMoreEventsPtr);
    }
    else if (GT_OK == prvCpssDxChDiagDataIntegrityDfxInterruptCheckAndRegGet(
                                       devNum, intNum, NULL, NULL))
    {
        /* handle DFX events starting with reading MG unit registers
         * "DFX/DFX1 Interrupts Summary Cause Register" */
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaMgUnitGet(devNum, portGroupId,
                                                         intNum,
                                                         eventsNumPtr,
                                                         eventsArr,
                                                         isNoMoreEventsPtr);
    }
    else
    {
        /* handle non-DFX interrupts. Device specific.  */
        switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
                {
                    /* bobcat2 */
                    rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat2(
                                                    devNum, intNum,
                                                    eventsNumPtr, eventsArr,
                                                    isNoMoreEventsPtr);
                }
                else if (PRV_CPSS_PP_MAC(devNum)->devSubFamily ==
                         CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
                {
                    /* bobk */
                    rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobk(
                        devNum, intNum,
                        eventsNumPtr, eventsArr,
                        isNoMoreEventsPtr);
                }
                else
                {
                    rc = GT_NOT_IMPLEMENTED;
                }
                break;

            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
                rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin(
                                                    devNum, intNum,
                                                    eventsNumPtr, eventsArr,
                                                    isNoMoreEventsPtr);
                break;

            case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat3(
                                                    devNum, intNum,
                                                    eventsNumPtr, eventsArr,
                                                    isNoMoreEventsPtr);
                break;

            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin2(
                                                    devNum, intNum,
                                                    eventsNumPtr, eventsArr,
                                                    isNoMoreEventsPtr);
                break;

            default:
                rc = GT_BAD_PARAM;
                break;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetAc5 function
* @endinternal
*
* @brief   Function performs interrupt tree scan for DataIntegrity event details
*         for AC5 devices.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetAc5
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;   /* return code */
    GT_U32 intNum;          /* interrupt number */
    GT_U32 pipeNum;         /* DFX pipe number  */

    intNum      = evExtData & 0xFFFF;

    if ((intNum == PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E) ||
        (intNum == PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E))
    {
        /* handle DFX events for specific DFX Pipe */
        pipeNum = intNum - PRV_CPSS_AC5_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E;
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10(
                devNum, pipeNum, eventsNumPtr, eventsArr, isNoMoreEventsPtr);

    }
    else
    {
        /* handle non-DFX interrupts */
        rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5(
                                            devNum, intNum,
                                            eventsNumPtr, eventsArr,
                                            isNoMoreEventsPtr);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetAc5x function
* @endinternal
*
* @brief   Function performs interrupt tree scan for DataIntegrity event details
*         for AC5X devices.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetAc5x
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;   /* return code */
    GT_U32 intNum;          /* interrupt number */
    GT_U32 pipeNum;         /* DFX pipe number  */

    intNum      = evExtData & 0xFFFF;

    if ((intNum >= PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E) &&
        (intNum <= PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E))
    {
        /* handle DFX events for specific DFX Pipe */
        pipeNum = intNum - PRV_CPSS_AC5X_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E;
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10(
                devNum, pipeNum, eventsNumPtr, eventsArr, isNoMoreEventsPtr);

    }
    else
    {
        /* handle non-DFX interrupts */
        rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5x(
                                            devNum, intNum,
                                            eventsNumPtr, eventsArr,
                                            isNoMoreEventsPtr);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetAc5p function
* @endinternal
*
* @brief   Function performs interrupt tree scan for DataIntegrity event details
*         for AC5P devices.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetAc5p
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;   /* return code */
    GT_U32 intNum;          /* interrupt number */
    GT_U32 pipeNum;         /* DFX pipe number  */

    intNum      = evExtData & 0xFFFF;

    if ((intNum >= PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E) &&
        (intNum <= PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E))
    {
        /* handle DFX events for specific DFX Pipe */
        pipeNum = intNum - PRV_CPSS_AC5P_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E;
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10(
                devNum, pipeNum, eventsNumPtr, eventsArr, isNoMoreEventsPtr);

    }
    else
    {
        /* handle non-DFX interrupts */
        rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5p(
                                            devNum, intNum,
                                            eventsNumPtr, eventsArr,
                                            isNoMoreEventsPtr);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetHarrier function
* @endinternal
*
* @brief   Function performs interrupt tree scan for DataIntegrity event details
*         for Harrier devices.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; AC5P; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetHarrier
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;   /* return code */
    GT_U32 intNum;          /* interrupt number */
    GT_U32 pipeNum;         /* DFX pipe number  */

    intNum      = evExtData & 0xFFFF;

    if ((intNum >= PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E) &&
        (intNum <= PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E))
    {
        /* handle DFX events for specific DFX Pipe */
        pipeNum = intNum - PRV_CPSS_HARRIER_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E;
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10(
                devNum, pipeNum, eventsNumPtr, eventsArr, isNoMoreEventsPtr);

    }
    else
    {
        /* handle non-DFX interrupts */
        rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetHarrier(
                                            devNum, intNum,
                                            eventsNumPtr, eventsArr,
                                            isNoMoreEventsPtr);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetSip6 function
* @endinternal
*
* @brief   Function performs interrupt tree scan for DataIntegrity event details
*         for SIP6 devices.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetSip6
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS                                 rc = GT_OK;      /* return code */
    GT_U32                                    intNum;          /* interrupt number */
    GT_U32                                    dfxInstanceIndex;/* DFX Instance Index: Tiles 0-3, Chiplets 0-15 */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                    tileIndex = 0; /* falcon tile index 0:3 */
    GT_U32                                    chipletIndex = 0; /* falcon raven index/tile 0:3 */

    intNum      = evExtData & 0x3FFFF; /* falcon interrupt enumerator last index 2D001 */

    rc = prvCpssDxChDiagDataIntegrityEventsDfxInstanceGet(intNum, &dfxInstanceIndex, &dfxInstanceType);

    if(GT_OK == rc)
    {
        dfxType = dfxInstanceType;
        if(dfxType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
        {
            tileIndex = dfxInstanceIndex;
            chipletIndex = 0;
        }
        else if(dfxType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
        {
            tileIndex = dfxInstanceIndex/4;
            chipletIndex = dfxInstanceIndex%4;
        }
        else
        {
            /* dfx instance type not valid */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        /* handle DFX events starting with reading DFX register
         * "Server Interrupt Summary Cause register" */
        rc = prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6(
            devNum, dfxType, tileIndex, chipletIndex, eventsNumPtr, eventsArr, isNoMoreEventsPtr);
    }
    else
    {
        /* handle non-DFX interrupts. Device specific.  */
        rc = prvCpssDxChDiagDataIntegrityNonDfxEventsGetFalcon(
            devNum, intNum,
            eventsNumPtr, eventsArr,
            isNoMoreEventsPtr);
    }
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsDfxInstanceGet function
* @endinternal
*
* @brief   Function gets DFX instance type and index based on interrupt cause.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in]  intNum                - HW interrupt index.
* @param[out] dfxInstanceIndex      - (pointer to) dfx instance index.
*                                       TILE    : 0 - 3
*                                       CHIPLET : 0 - 15
* @param[out] dfxInstanceType       - (pointer to) dfx instance type.
*                                       TILE/CHIPLET
*                                       out: HW table type as it is in DB.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
*/

GT_STATUS prvCpssDxChDiagDataIntegrityEventsDfxInstanceGet
(
    IN  GT_U32                                    intNum,
    OUT GT_U32                                    *dfxInstanceIndex,
    OUT CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT *dfxInstanceType
)
{
    GT_STATUS rc = GT_OK;

    switch(intNum)
    {
        case PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            *dfxInstanceIndex = 0;
            break;

        case PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            *dfxInstanceIndex = 1;
            break;

        case PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            *dfxInstanceIndex = 2;
            break;

        case PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            *dfxInstanceIndex = 3;
            break;

        case PRV_CPSS_FALCON_TILE_0_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 0;
            break;

        case PRV_CPSS_FALCON_TILE_0_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 1;
            break;

        case PRV_CPSS_FALCON_TILE_0_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 2;
            break;

        case PRV_CPSS_FALCON_TILE_0_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_0_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 3;
            break;

        case PRV_CPSS_FALCON_TILE_1_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 4;
            break;

        case PRV_CPSS_FALCON_TILE_1_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 5;
            break;

        case PRV_CPSS_FALCON_TILE_1_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 6;
            break;

        case PRV_CPSS_FALCON_TILE_1_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_1_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 7;
            break;

        case PRV_CPSS_FALCON_TILE_2_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 8;
            break;

        case PRV_CPSS_FALCON_TILE_2_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 9;
            break;

        case PRV_CPSS_FALCON_TILE_2_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 10;
            break;

        case PRV_CPSS_FALCON_TILE_2_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_2_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 11;
            break;

        case PRV_CPSS_FALCON_TILE_3_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_0_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 12;
            break;

        case PRV_CPSS_FALCON_TILE_3_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_1_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 13;
            break;

        case PRV_CPSS_FALCON_TILE_3_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_2_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 14;
            break;

        case PRV_CPSS_FALCON_TILE_3_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E:
        case PRV_CPSS_FALCON_TILE_3_RAVEN_3_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E:
            *dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            *dfxInstanceIndex = 15;
            break;

        default:
            return /* it's not error to get this line */ GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityEventsGetLion2 function
* @endinternal
*
* @brief   Function performs DFX interrupt tree scan for Lion2.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityEventsGetLion2
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;    /* return code */
    GT_U32 regAddr;     /* register address */
    GT_U32 regData;     /* register data */
    GT_U32 regData1;    /* register data */
    GT_U32 ruleOffset;  /* variable for policy rule offset calculations */
    GT_U32 i, j, k;     /* loop iterators */
    GT_U32 pipeIndex;   /* pipe index */
    GT_U32 clientIndex; /* client index */
    GT_U32 memoryNumber;/* memory index */
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_ADDR_STC      addrStr;    /* DFX address struct */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC clientBmp = {{0,0,0,0}};  /* client status struct */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC        *dbItemPtr; /* pointer to map DB item */
    GT_U32 eventCounter = 0; /* found event counter */
    GT_U32 portGroupId; /* port group id */
    GT_U32 eventAdditionInfo; /* event additional info - HW interrupt code */
    const PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC (*firstStageMappingArrayPtr)[MAX_DFX_INT_CAUSE_NUM_CNS];

    portGroupId = evExtData >> 16;
    eventAdditionInfo = evExtData & 0xFFFF;

    switch (PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
            firstStageMappingArrayPtr = hooperDataIntegrityFirstStageMappingArray;
            break;
        default:
            firstStageMappingArrayPtr = lion2DataIntegrityFirstStageMappingArray;
            break;
    }

    /* NON DFX memories */
    if(eventAdditionInfo != 0)
    {
        *isNoMoreEventsPtr = GT_TRUE;
        eventsArr[0].location.isTcamInfoValid =
        eventsArr[0].location.isMppmInfoValid = GT_FALSE;
        eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
        eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;
        switch(eventAdditionInfo)
        {
            case PRV_CPSS_LION2_TCC_LOWER_GEN_TCAM_ERROR_DETECTED_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E;
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.pclTccRegs.policyTccErrorInformation;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 30, 1) == 0) ? CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E : CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

                if(U32_GET_FIELD_MAC(regData, 12, 4) != 0)
                {
                    ruleOffset = 3;
                }
                else if(U32_GET_FIELD_MAC(regData, 8, 4) != 0)
                {
                    ruleOffset = 2;
                }
                else if(U32_GET_FIELD_MAC(regData, 4, 4) != 0)
                {
                    ruleOffset = 1;
                }
                else
                {
                    ruleOffset = 0;
                }
                eventsArr[0].location.tcamMemLocation.ruleIndex = (U32_GET_FIELD_MAC(regData, 16, 14)) * 4 + ruleOffset;
                eventsArr[0].location.isTcamInfoValid = GT_TRUE;

                return GT_OK;

            case PRV_CPSS_LION2_TCC_UPPER_GEN_TCAM_ERROR_DETECTED_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E;
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTccErrorInformation;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 30, 1) == 0) ? CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E : CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;
                eventsArr[0].location.tcamMemLocation.ruleIndex = U32_GET_FIELD_MAC(regData, 16, 14);
                eventsArr[0].location.isTcamInfoValid = GT_TRUE;

                return GT_OK;

            case PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_ONE_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E;

                break;

            case PRV_CPSS_LION2_TXDMA_ERROR_HA_INFO_FIFO_ECC_TWO_OR_MORE_ERRORS_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E;
                break;

            case PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_0_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E;
                break;

            case PRV_CPSS_LION2_TXQ_EGRESS_1_GEN_MC_FIFO_0_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E;
                break;

            case PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_1_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E;

                break;

            case PRV_CPSS_LION2_TXQ_EGRESS_1_GEN_MC_FIFO_1_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E;
                break;

            case PRV_CPSS_LION2_TXQ_EGRESS_0_GEN_MC_FIFO_3_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E;
                break;

            case PRV_CPSS_LION2_TXQ_EGRESS_1_GEN_MC_FIFO_3_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E;
                break;

            case PRV_CPSS_LION2_TXQ_SHT_GEN_EGR_VLAN_TBL_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E;
                break;

            case PRV_CPSS_LION2_TXQ_SHT_GEN_ING_VLAN_TBL_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E;
                break;

            case PRV_CPSS_LION2_TXQ_SHT_GEN_L2_PORT_ISOLATION_TBL_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E;
                break;

            case PRV_CPSS_LION2_TXQ_SHT_GEN_L3_PORT_ISOLATION_TBL_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E;
                break;

            case PRV_CPSS_LION2_TXQ_SHT_GEN_VIDX_TBL_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E;
                break;

            case PRV_CPSS_LION2_IPLR0_DATA_ERROR_E:
            case PRV_CPSS_LION2_IPLR1_DATA_ERROR_E:

                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);

                if(eventAdditionInfo == PRV_CPSS_LION2_IPLR0_DATA_ERROR_E)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerErrorReg;
                }
                else
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[1].policerErrorReg;
                }

                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 24, 2, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(regData == 1)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                }
                else if(regData == 2)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E;

                if(eventAdditionInfo == PRV_CPSS_LION2_IPLR0_DATA_ERROR_E)
                {
                    eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId = portGroupId; /* por PLR PipeId = PortGroupId */
                    eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId = 8;
                    eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId = 34;
                }
                else
                {
                    eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId = portGroupId; /* por PLR PipeId = PortGroupId */
                    eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId = 8;
                    eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId = 32;
                }

                return GT_OK;

            case PRV_CPSS_LION2_IP_ROUTER_NHE_DIP_DATA_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E;
                break;

            case PRV_CPSS_LION2_IP_ROUTER_NHE_SIP_DATA_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E;
                break;

            case PRV_CPSS_LION2_IP_ROUTER_STG2_DATA_FIFO_ECC_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 18, 2, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(regData == 1)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                }
                else if(regData == 2)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E;
                break;

            case PRV_CPSS_LION2_IP_ROUTER_STG4_DATA_FIFO_ECC_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 20, 2, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(regData == 1)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                }
                else if(regData == 2)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E;
                break;

            case PRV_CPSS_LION2_IP_ROUTER_UNUSED_DATA_FIFO_ECC_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 16, 2, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(regData == 1)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                }
                else if(regData == 2)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E;
                break;

            /* all MPPM cases should return here because of different location structure */
            case PRV_CPSS_LION2_MPPM_0_BK0_ECC_1_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 0;
                eventsArr[0].location.mppmMemLocation.bankId = 0;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_0_BK1_ECC_1_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 0;
                eventsArr[0].location.mppmMemLocation.bankId = 1;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_1_BK0_ECC_1_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 1;
                eventsArr[0].location.mppmMemLocation.bankId = 0;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_1_BK1_ECC_1_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 1;
                eventsArr[0].location.mppmMemLocation.bankId = 1;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_0_BK0_ECC_2_OR_MORE_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 0;
                eventsArr[0].location.mppmMemLocation.bankId = 0;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_0_BK1_ECC_2_OR_MORE_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 0;
                eventsArr[0].location.mppmMemLocation.bankId = 1;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_1_BK0_ECC_2_OR_MORE_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 1;
                eventsArr[0].location.mppmMemLocation.bankId = 0;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_MPPM_1_BK1_ECC_2_OR_MORE_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                eventsArr[0].location.mppmMemLocation.portGroupId = portGroupId;
                eventsArr[0].location.mppmMemLocation.mppmId = 1;
                eventsArr[0].location.mppmMemLocation.bankId = 1;
                eventsArr[0].location.isMppmInfoValid = GT_TRUE;
                return GT_OK;

            case PRV_CPSS_LION2_TXQ_DQ_MEMORY_ERROR_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
                break;

            case PRV_CPSS_LION2_TXQ_DQ_MEMORY_ERROR_STATE_VARIABLE_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E;
                break;

            case PRV_CPSS_LION2_TXQ_LL_GEN_FBUF_ECC_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.ll.linkList.linkListEccControl.fbufRamEccStatus;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 12, 2, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(regData == 1)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                }
                else if(regData == 2)
                {
                    eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_QCN_CN_BUFFER_FIFO_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP0_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP1_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP2_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP3_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP4_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP5_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP6_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_TXQ_QUEUE_PFC_PARITY_PORT_GROUP7_COUNTERS_ERR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E;
                break;

            case PRV_CPSS_LION2_BMA_CORE0_MCCNT_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId = 4;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId = 1;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId = 51;
                return GT_OK;

            case PRV_CPSS_LION2_BMA_CORE1_MCCNT_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId = 4;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId = 1;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId = 43;
                return GT_OK;

            case PRV_CPSS_LION2_BMA_CORE2_MCCNT_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId = 4;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId = 1;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId = 35;
                return GT_OK;

            case PRV_CPSS_LION2_BMA_CORE3_MCCNT_PARITY_ERROR_E:
                *eventsNumPtr = 1;
                eventsArr[0].location.portGroupsBmp = (1 << portGroupId);
                eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId = 4;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId = 1;
                eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId = 27;
                return GT_OK;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChDiagDataIntegrityPortGroupMemoryIndexesGet(devNum,
                                                                   portGroupId,
                                                                   eventsArr[0].location.ramEntryInfo.memType,
                                                                   &(eventsArr[0].location.ramEntryInfo.memLocation));
        return rc;
    }


    /* DFX Interrupts Summary Cause */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptCause;

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 27, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* Ignore bits [1:0] */
    regData &= ~0x3;

    *isNoMoreEventsPtr = GT_TRUE;

    for(i = 2; i <= 26; i++)
    {
        /* ECC/Parity Interrupt Cause Register scanning algorithm */
        /*
             - Read DFX Interrupt cause register
             - For every bit which is set - find in first stage map array DFX pipe index and Client index.
             - Set "Pipe Select" register according to DFX pipe index
             - Read 4 memory bitmap registers according to Client index in current DFX pipe.
             - For every bit which is set (memory number) calculate interrupt cause register address:
                    pipe index + client index + memory number => cause/mask register address.
             - In order to return CPSS Table + entry number date go to second stage map array to make conversion.
        */

        if(U32_GET_FIELD_MAC(regData, i, 1))
        {
            pipeIndex = firstStageMappingArrayPtr[portGroupId][i].dfxPipeIndex;
            clientIndex = firstStageMappingArrayPtr[portGroupId][i].dfxClientIndex;

            if((pipeIndex == DATA_INTEGRITY_ENTRY_NOT_USED_CNS) || (clientIndex == DATA_INTEGRITY_ENTRY_NOT_USED_CNS))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            /* Set Pipe Select Register */
            rc = prvCpssDfxMemoryPipeIdSet(devNum, pipeIndex);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Read DFX Client Memory BitMap 0-3 */
            addrStr.dfxUnitClientIdx = clientIndex;
            addrStr.dfxXsbSelect = 0;
            addrStr.dfxClientSelect = 1;

            for(k = 0; k < 4; k++)
            {
                addrStr.dfxClientRegisterOffset = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS + k * 4;

                rc = prvCpssDfxBuildAddr(devNum, &addrStr, &regAddr);
                if(GT_OK != rc)
                {
                    return rc;
                }
                rc = prvCpssHwPpReadRegister(devNum, regAddr, &(clientBmp.bitmap[k]));
                if(GT_OK != rc)
                {
                    return rc;
                }
            }

            for(k = 0; k < 4; k++)
            {
                for(j = 0; j < 32; j++)
                {
                    if(U32_GET_FIELD_MAC(clientBmp.bitmap[k], j, 1))
                    {
                        memoryNumber = k*32 + j;

                        /* Read Memory Interrupt Cause Register */
                        addrStr.dfxUnitClientIdx = clientIndex;
                        addrStr.dfxXsbSelect = 0;
                        addrStr.dfxClientSelect = 0;
                        addrStr.dfxClientMemoryNumber = memoryNumber;
                        addrStr.dfxClientMemoryRegisterOffset = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_CAUSE_REG_CNS;

                        rc = prvCpssDfxBuildAddr(devNum, &addrStr, &regAddr);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, &regData);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        rc = searchMemType(devNum, pipeIndex, clientIndex, memoryNumber, &dbItemPtr);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }

                        /* update output parameters */
                        eventsArr[eventCounter].location.ramEntryInfo.memType = dbItemPtr->memType;
                        eventsArr[eventCounter].location.portGroupsBmp = 1 << dbItemPtr->causePortGroupId;
                        /* Othe locations except ramEnryInfo are not assigned */
                        eventsArr[eventCounter].location.ramEntryInfo.memLocation.dfxPipeId = pipeIndex;
                        eventsArr[eventCounter].location.ramEntryInfo.memLocation.dfxClientId = clientIndex;
                        eventsArr[eventCounter].location.ramEntryInfo.memLocation.dfxMemoryId = memoryNumber;
                        eventsArr[eventCounter].location.isTcamInfoValid =
                        eventsArr[eventCounter].location.isMppmInfoValid = GT_FALSE;


                        if(U32_GET_FIELD_MAC(regData, 1, 1))
                        {
                            eventsArr[eventCounter].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                        }
                        else if(U32_GET_FIELD_MAC(regData, 2, 1))
                        {
                            eventsArr[eventCounter].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                        }
                        else if(U32_GET_FIELD_MAC(regData, 3, 1))
                        {
                            eventsArr[eventCounter].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                        }

                        if(eventCounter == (*eventsNumPtr - 1))
                        {
                            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptCause;

                            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 31, 1, &regData);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }

                            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalInterrupt.dfxInterruptMask;

                            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 31, 1, &regData1);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }


                            if((regData & regData1) != 0)
                            {
                                *isNoMoreEventsPtr = GT_FALSE;
                            }

                            return GT_OK;
                        }

                        eventCounter++;
                    }
                }
            }

        }
    }
    *eventsNumPtr = eventCounter;

    return GT_OK;
}


/**
* @internal prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam function
* @endinternal
*
* @brief   Using memory location (it is assumed filled) fill appropriately
*         HW and logical locations of CPSS_DXCH_LOCATION_FULL_INFO_STC struct.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in,out] locationPtr          - (pointer to) location. HW- and logical-
*                                       related parts will be modified.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam
(
    IN    GT_U8 devNum,
    INOUT CPSS_DXCH_LOCATION_FULL_INFO_STC *locationPtr
)
{
    CPSS_DXCH_LOGICAL_TABLE_ENT      logicalTableName;
    CPSS_DXCH_LOGICAL_TABLE_INFO_STC *logicalInfoPtr;
    CPSS_DXCH_RAM_INDEX_INFO_STC     *ramInfoPtr;
    GT_U32                           marker; /* used to get multiple logical tables with
                                                prvCpssDxChDiagDataIntegrityTableHwToLogicalGet */
    GT_U32 cnt;
    GT_U32 entryLen;            /*number of bits in the entry*/
    GT_U32 logicalIndex = 0;    /* logical table entry index */
    GT_U32 defaultLogicalIndex; /* logical table entry index */
    GT_U32 arpLogicalIndex      = 0;     /* logical index of ARP table */
    GT_U32 arpLogicalNumEntries = 0;    /* number of ARP logical table entries */
    GT_U32 entriesPerHwLine     = 0;    /* Number of ARP entries per HW Line */
    CPSS_DXCH_TABLE_ENT         hwTable; /* HW table type */
    GT_U32                      hwIndex; /* HW table item index */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* pointer to Data Integrity DB entry */
    GT_BOOL                     skipTable;
    GT_U32                      logicalNumEntries; /* number of entries in the logical table */
    GT_U32                      portGroupId;       /* port group id */
    GT_STATUS                   rc;
    GT_U32                      dqIndex;
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */

    ramInfoPtr = &locationPtr->ramEntryInfo;

    if (locationPtr->isMppmInfoValid)
    {
        /* MPPM memory could be the special case because of unusual
           locationPtr->ramEntryInfo.ramRow content:
           ramRow[19:18] = group number,
           ramRow[17:16] = bank number,
           ramRow[15:0]  = buffer number
           But there are no HW table relevant to MPPM so it will be handled
           like usual case when a RAM->HW mapping is absent */
        locationPtr->mppmMemLocation.bankId = (ramInfoPtr->ramRow >> 16) & 0x3;
        locationPtr->mppmMemLocation.mppmId = (ramInfoPtr->ramRow >> 18) & 0x3;
        locationPtr->mppmMemLocation.portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        return GT_OK;
    }

    /* fill hw table info */
    if (locationPtr->isTcamInfoValid)
    {
        locationPtr->hwEntryInfo.hwTableType = CPSS_DXCH_SIP5_TABLE_TCAM_E;
        rc = prvCpssDxChTcamRuleIndexToSip5HwEntryNumber(
                                         locationPtr->tcamMemLocation.ruleIndex,
                                         &locationPtr->hwEntryInfo.hwTableEntryIndex);

        if(rc != GT_OK)
        {
            return rc;
        }

        if (CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E == locationPtr->tcamMemLocation.arrayType)
        {
            ++locationPtr->hwEntryInfo.hwTableEntryIndex;
        }
        defaultLogicalIndex = locationPtr->tcamMemLocation.ruleIndex;
    }
    else
    {
        rc = prvCpssDxChDiagDataIntegrityDfxErrorConvert(devNum,
                                                     &ramInfoPtr->memLocation,
                                                     ramInfoPtr->ramRow,
                                                     &portGroupId,
                                                     &locationPtr->hwEntryInfo);
        locationPtr->portGroupsBmp = 1 << (portGroupId & 0x1f);

        if (rc == GT_EMPTY)
        {
            /* RAM does not have HW Table */
            locationPtr->hwEntryInfo.hwTableType = CPSS_DXCH_TABLE_LAST_E;
            locationPtr->logicalEntryInfo.numOfLogicalTables = 0;
            return GT_OK;
        }

        if (rc != GT_OK)
        {
            return rc;
        }

        if (CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E == locationPtr->hwEntryInfo.hwTableType)
        {
            /* there can be 1 <IPv6 TS> or 2 <non-IPv6 TS> or 8 <ARP> enrties
               in single HW table entry. But the API doesn't have info about type
               of TS entry so treat all of them as non-IPv6 TS enrtries
               - i.e. 2 TS entries per one HW entry */

            if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
            {
                if (ramInfoPtr->memLocation.dfxInstance.dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                {
                    dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
                }
                else if(ramInfoPtr->memLocation.dfxInstance.dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                {
                    dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
                }
                else
                {
                    /* dfx instance type is invalid */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
                rc = searchMemTypeFalcon(ramInfoPtr->memLocation.dfxPipeId,
                    ramInfoPtr->memLocation.dfxClientId,
                    ramInfoPtr->memLocation.dfxMemoryId,
                    dfxInstanceType,
                    &dbItemPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (dbItemPtr == NULL)
                {
                    /* shouldn't happen */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                rc = searchMemType(devNum,
                    ramInfoPtr->memLocation.dfxPipeId,
                    ramInfoPtr->memLocation.dfxClientId,
                    ramInfoPtr->memLocation.dfxMemoryId,
                    &dbItemPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (dbItemPtr == NULL)
                {
                    /* shouldn't happen */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }

            if ((dbItemPtr->hwTableName >= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E) &&
                (dbItemPtr->hwTableName <= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_LAST_E))
            {
                /* shared memory case */

                GT_U32 arpSbmIndex; /* serial number of SBM block */
                GT_U32 arpInTsLine; /* index of ARP entry in logical TS line */

                arpSbmIndex = prvCpssDxChCfgSharedTablesArpSmbIndexGet(devNum, (dbItemPtr->hwTableName - PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E));
                if (arpSbmIndex == 0xFFFFFFFF)
                {
                    /* shouldn't happen */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* only 96 bits are used in SBMs for ARP/TS. Each SBM holds 2 full ARP entries. */
                arpLogicalNumEntries = 2;

                /* TS logical entry is in four sequential SBMs:
                   arpSbmIndex  data bits  ARP   IPv4 TS
                      0         288..383 - 6..7    1
                      1         192..287 - 4..5    1
                      2          96..191 - 2..3    0
                      3           0..95  - 0..1    0 */
                arpInTsLine = (3 - (arpSbmIndex % 4)) * arpLogicalNumEntries;

                /* ARP entry */
                arpLogicalIndex = 8 * locationPtr->hwEntryInfo.hwTableEntryIndex + arpInTsLine;

                /* IPv4 TS entry */
                defaultLogicalIndex = 2 * locationPtr->hwEntryInfo.hwTableEntryIndex + arpInTsLine / 4;
            }
            else
            {
                /* not-IPv6 entry logical index. 2 entries per HW line
                 * AC5 - 1 entry per HW line */
                entryLen = 192;
                entriesPerHwLine = PRV_CPSS_DXCH_AC5_CHECK_MAC(devNum)?1:2;
                defaultLogicalIndex = entriesPerHwLine * locationPtr->hwEntryInfo.hwTableEntryIndex +
                    (dbItemPtr->firstTableDataBit / entryLen); /* 0 or 1 */

                /* ARP entry logical index. 8 entries per HW line
                 * Note - AC5 - 4 entries per HW line */
                entryLen = 48;
                entriesPerHwLine = PRV_CPSS_DXCH_AC5_CHECK_MAC(devNum)?4:8;
                arpLogicalIndex  = entriesPerHwLine * locationPtr->hwEntryInfo.hwTableEntryIndex +
                    (dbItemPtr->firstTableDataBit / entryLen); /* 0 .. 7 */

                /* how many ARP enrties fit into single memory */
                arpLogicalNumEntries = (dbItemPtr->lastTableDataBit - dbItemPtr->firstTableDataBit) / entryLen + 1;
            }
        }
        else
        {
            defaultLogicalIndex = locationPtr->hwEntryInfo.hwTableEntryIndex;
        }
    }


    /* fill logical tables info */
    marker = 0;
    cnt    = 0;
    hwTable = locationPtr->hwEntryInfo.hwTableType;
    hwIndex = locationPtr->hwEntryInfo.hwTableEntryIndex;
    /* get all logical tables (until GT_NOT_FOUND)  related to HW table */
    while (GT_OK ==   (rc = prvCpssDxChDiagDataIntegrityTableHwToLogicalGet(
                           devNum, hwTable, &marker, &logicalTableName)))
    {
        if (cnt >=  CPSS_DXCH_LOGICAL_IN_LOCATION_MAX_CNS)
        {
            /* Something wrong. Logical tables number exeeds maximum */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        logicalNumEntries = 1;
        skipTable = GT_FALSE;

        switch (logicalTableName)
        {
            case CPSS_DXCH_LOGICAL_TABLE_ARP_E:
                logicalNumEntries = arpLogicalNumEntries;
                logicalIndex      = arpLogicalIndex;
                break;

            case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E:
            case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E:
            case CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E:
            case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_COUNTING_E:
            case CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E:
            case CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E:
                rc = plrHwTableIndexToLogicalTableIndex(
                    devNum, hwTable, hwIndex, logicalTableName, &logicalIndex);
                if (rc == GT_BAD_VALUE)
                {
                    skipTable = GT_TRUE;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                }

                break;

            case CPSS_DXCH_LOGICAL_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E:
                switch (hwTable)
                {
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_2_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_3_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_4_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_5_E:
                        /* tables are the same size */
                        dqIndex = hwTable + 1 -
                            CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E;
                        logicalIndex = hwIndex +
                            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp * dqIndex;
                        break;
                    default:
                        break;
                }
                break;

            case CPSS_DXCH_LOGICAL_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E:
                switch (hwTable)
                {
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_2_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_3_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E:
                        /* tables are the same size */
                        dqIndex = hwTable + 1 -
                            CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E;
                        logicalIndex = hwIndex +
                            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp * dqIndex;
                        break;
                    default:
                        break;
                }
                break;

            case CPSS_DXCH_LOGICAL_TABLE_PHYSICAL_PORT_E:
                switch (hwTable)
                {
                    case CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_2_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_3_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_4_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_5_E:
                        /* hw tables are indexed with txq port number whereas logical
                         * table - with physical port. Convertation is required. */
                        dqIndex  = (hwTable == CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E) ? 0 :
                            hwTable + 1 - CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E;

                        /* convert local txq -> global txq */
                        hwIndex += dqIndex * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                        /* convert txq port to physical port number */
                        prvTxqToPhyRange(devNum, hwIndex, 1, &logicalIndex, &logicalNumEntries);
                        if (dqIndex > PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp
                           || logicalNumEntries == 0)
                        {
                            skipTable = GT_TRUE;
                        }
                        break;

                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_2_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_3_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_4_E:
                    case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_5_E:
                        dqIndex = hwTable -
                            CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E;
                        if (dqIndex > PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)
                        {
                            skipTable = GT_TRUE;
                        }
                        else
                        {
                            logicalIndex = hwIndex +
                                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseFirstPortInRange;
                        }
                        break;
                    default:
                        break;
                }
                break;

            default:
                logicalIndex = defaultLogicalIndex;
                break;
        }
        if (skipTable != GT_TRUE)
        {
            logicalInfoPtr = &locationPtr->logicalEntryInfo.logicaTableInfo[cnt++];
            logicalInfoPtr->logicalTableType       = logicalTableName;
            logicalInfoPtr->logicalTableEntryIndex = logicalIndex;
            logicalInfoPtr->numEntries             = logicalNumEntries;
        }
    }
    if (rc == GT_NOT_FOUND)
    {
        locationPtr->logicalEntryInfo.numOfLogicalTables = cnt;
    }
    else if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagDataIntegrityEventsGet function
* @endinternal
*
* @brief   Function returns array of data integrity events.
*
* @note   APPLICABLE DEVICES:      Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - array of ECC/parity events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*                                      GT_TRUE - no more events found
*                                      GT_FALSE - there are more events found
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityEventsGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT location;
    GT_U32 failedRow = 0;
    GT_U32 i;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(eventsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(eventsArr);
    CPSS_NULL_PTR_CHECK_MAC(isNoMoreEventsPtr);

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            rc = prvCpssDxChDiagDataIntegrityEventsGetLion2(devNum, evExtData,
                                                            eventsNumPtr, eventsArr,
                                                            isNoMoreEventsPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* get failed row info */
            for (i=0; i < *eventsNumPtr; i++)
            {
                if (eventsArr[i].location.isMppmInfoValid == GT_TRUE)
                {
                    location.mppmMemLocation = eventsArr[i].location.mppmMemLocation;
                }
                else if (eventsArr[i].location.isTcamInfoValid == GT_TRUE)
                {
                    location.tcamMemLocation = eventsArr[i].location.tcamMemLocation;
                }
                else
                {
                    location.memLocation = eventsArr[i].location.ramEntryInfo.memLocation;
                }
                rc = prvCpssDxChDiagDataIntegrityErrorInfoGetLion2(
                    devNum, eventsArr[i].location.ramEntryInfo.memType, &location,
                    NULL, &failedRow, NULL);
                if (rc != GT_OK)
                {
                    return rc;
                }
                eventsArr[i].location.ramEntryInfo.ramRow = failedRow;
            }
            break;

        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            /* sip6 */
            rc = prvCpssDxChDiagDataIntegrityEventsGetSip6(devNum, evExtData,
                                                           eventsNumPtr, eventsArr,
                                                           isNoMoreEventsPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* get failed row info */
            for (i=0; i < *eventsNumPtr; i++)
            {
                /* TCAM error location is already set */
                if (eventsArr[i].location.isTcamInfoValid == GT_FALSE)
                {
                    location.memLocation = eventsArr[i].location.ramEntryInfo.memLocation;

                    rc = prvCpssDxChDiagDataIntegrityErrorInfoGetSip5(
                        devNum, eventsArr[i].location.ramEntryInfo.memType, &location,
                        NULL, &failedRow, NULL, NULL);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }

                eventsArr[i].location.ramEntryInfo.ramRow = failedRow;

                /* RAM coordiates are filled. Fill HW, Logical coordinates */
                rc = prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam(
                                                devNum, &eventsArr[i].location);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            break;
        default:
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
            {
                /* AC5 */
                rc = prvCpssDxChDiagDataIntegrityEventsGetAc5(devNum, evExtData,
                                                               eventsNumPtr, eventsArr,
                                                               isNoMoreEventsPtr);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
            {
                /* AC5X */
                rc = prvCpssDxChDiagDataIntegrityEventsGetAc5x(devNum, evExtData,
                                                               eventsNumPtr, eventsArr,
                                                               isNoMoreEventsPtr);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                /* AC5P */
                rc = prvCpssDxChDiagDataIntegrityEventsGetAc5p(devNum, evExtData,
                                                               eventsNumPtr, eventsArr,
                                                               isNoMoreEventsPtr);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
            {
                /* Harrier */
                rc = prvCpssDxChDiagDataIntegrityEventsGetHarrier(devNum, evExtData,
                                                               eventsNumPtr, eventsArr,
                                                               isNoMoreEventsPtr);
            }
            else
            {
                /* sip5 */
                rc = prvCpssDxChDiagDataIntegrityEventsGetSip5(devNum, evExtData,
                                                               eventsNumPtr, eventsArr,
                                                               isNoMoreEventsPtr);
            }
            if (rc != GT_OK)
            {
                return rc;
            }
            /* get failed row info */
            for (i=0; i < *eventsNumPtr; i++)
            {
                if (eventsArr[i].location.isMppmInfoValid == GT_TRUE)
                {
                    location.mppmMemLocation = eventsArr[i].location.mppmMemLocation;
                }
                else
                {
                    location.memLocation = eventsArr[i].location.ramEntryInfo.memLocation;
                }

                /* TCAM error location is already set */
                if (eventsArr[i].location.isTcamInfoValid == GT_FALSE)
                {
                    rc = prvCpssDxChDiagDataIntegrityErrorInfoGetSip5(
                        devNum, eventsArr[i].location.ramEntryInfo.memType, &location,
                        NULL, &failedRow, NULL, NULL);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }

                eventsArr[i].location.ramEntryInfo.ramRow = failedRow;

                /* RAM coordiates are filled. Fill HW, Logical coordinates */
                rc = prvCpssDxChDiagDataIntegrityFillHwLogicalFromRam(
                                                devNum, &eventsArr[i].location);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
    }
    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityEventsGet function
* @endinternal
*
* @brief   Function returns array of data integrity events.
*
* @note   APPLICABLE DEVICES:      Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - array of ECC/parity events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*                                      GT_TRUE - no more events found
*                                      GT_FALSE - there are more events found
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChDiagDataIntegrityEventsGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityEventsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, evExtData, eventsNumPtr, eventsArr, isNoMoreEventsPtr));

    rc = internal_cpssDxChDiagDataIntegrityEventsGet(devNum, evExtData, eventsNumPtr, eventsArr, isNoMoreEventsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, evExtData, eventsNumPtr, eventsArr, isNoMoreEventsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagDataIntegrityEventMaskSet function
* @endinternal
*
* @brief   Function sets mask/unmask for ECC/Parity event.
*         Event is masked/unmasked for whole hw/logical table or for all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] errorType                - type of error interrupt
*                                      relevant only for ECC protected memories
* @param[in] operation                - mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - if memEntry exists but doesn't support
*                                       event masking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityEventMaskSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS                   rc = GT_OK;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_U32                      memoryFound = 0; /* memory found counter */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex; /* falcon raven index 0:15, per tile 0:3 */
    GT_U32                                          pipeIndex; /* DFX pipe index */
    GT_U32                                          pipeIndexCheck; /* DFX pipe for tile index */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);

    if(operation != CPSS_EVENT_MASK_E &&
       operation != CPSS_EVENT_UNMASK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* call cpssDxChDiagDataIntegrityEventMaskSet for every hw table
               relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

            for (i = 0; i< memEntryPtr->info.logicalEntryInfo.numOfLogicalTables; i++)
            {
                rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                    devNum,
                    memEntryPtr->info.logicalEntryInfo.logicaTableInfo[i].logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* if some table doesn't have protection (so can't be masked)
                   or is absent in the DB skip it. But if ALL tables are
                   skipped return error */
                rc1 = GT_NOT_FOUND;
                for (j=0; hwTablesPtr[j] != LAST_VALID_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                    rc = internal_cpssDxChDiagDataIntegrityEventMaskSet(
                        devNum, &hwLocation, errorType, operation);
                    if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                    {
                       /* ignore tables either not found in DB  or not supporting the feature */
                        continue;
                    }
                    else if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc1 = rc;
                }
                /* neither of tables supports masking */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            }

            /* done */
            return GT_OK;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;

        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                hwTableInDb = dbArrayPtr[i].hwTableName;
                PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);

                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
                {
                    memoryFound += 1;
                    pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                    if (dbArrayPtr[i].protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
                        {
                            if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                            {
                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_INDEX_CHECK(tileIndex, pipeIndex, pipeIndexCheck)
                                if(pipeIndexCheck)
                                {
                                    continue;
                                }
                                /* DFX memory mask operation */
                                rc = prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMaskFalcon(devNum, &dbArrayPtr[i], errorType, operation,
                                    dfxInstanceType, tileIndex, 0);
                                if(GT_OK != rc)
                                {
                                    return rc;
                                }
                            }
                            else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                            {
                                for (chipletIndex = 0; chipletIndex < FALCON_RAVENS_PER_TILE; chipletIndex++)
                                {
                                    PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, chipletIndex);
                                    /* DFX memory mask operation */
                                    rc = prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMaskFalcon(devNum, &dbArrayPtr[i], errorType, operation,
                                        dfxInstanceType, tileIndex, chipletIndex);
                                    if(GT_OK != rc)
                                    {
                                        return rc;
                                    }
                                }
                            }
                            else
                            {
                                /* dfx instance type is invalid */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                            }
                        }
                    }
                    else if (dbArrayPtr[i].externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        /* non-DFX memory mask operation */
                        rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMask(devNum, dbArrayPtr[i].memType, errorType, operation);
                        if((rc != GT_OK) && (rc != GT_NOT_SUPPORTED))
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* the memory is not protected at all */
                        rc = GT_NOT_SUPPORTED;
                    }
                }
                if(rc == GT_NOT_SUPPORTED)
                {
                    if(memEntryPtr->type == CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE)
                    {
                        return rc;
                    }
                    memoryFound -= 1;
                    rc = GT_OK;
                }
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            hwTableInDb = dbArrayPtr[i].hwTableName;
            PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);

            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
            {
                memoryFound += 1;

                if (dbArrayPtr[i].protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* DFX memory mask operation */
                    rc = prvCpssDxChDiagDataIntegrityDfxMemoryProtectionEventMask(devNum, &dbArrayPtr[i], errorType, operation);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
                else if (dbArrayPtr[i].externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* non-DFX memory mask operation */
                    rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMask(devNum, dbArrayPtr[i].memType, errorType, operation);
                    if(rc == GT_NOT_SUPPORTED)
                    {
                        if(memEntryPtr->type == CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE)
                        {
                            return rc;
                        }
                        /* some none DFX protected RAMs are not supported
                           by CPSS but share same HW/Logical Table with protected RAMs.
                           skip such cases */
                        memoryFound -= 1;
                        rc = GT_OK;
                    }
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
                else
                {
                    /* the memory is not protected at all */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    if (memoryFound == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityEventMaskSet function
* @endinternal
*
* @brief   Function sets mask/unmask for ECC/Parity event.
*         Event is masked/unmasked for whole hw/logical table or for all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] errorType                - type of error interrupt
*                                      relevant only for ECC protected memories
* @param[in] operation                - mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - if memEntry exists but doesn't support
*                                       event masking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssDxChDiagDataIntegrityEventMaskSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityEventMaskSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorType, operation));

    rc = internal_cpssDxChDiagDataIntegrityEventMaskSet(devNum, memEntryPtr, errorType, operation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorType, operation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGet function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity DFX interrupt.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Falcon.
*
* @param[in] devNum                   - PP device number
* @param[in] memInfoPtr               - (pointer to) item of Device Data Integrity DB(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC).
*
* @param[out] errorTypePtr             - (pointer to) type of error interrupt
* @param[out] operationPtr             - (pointer to) mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGet
(
    IN GT_U8                                            devNum,
    IN const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC  *memInfoPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr

)
{
    GT_STATUS rc;
    GT_U32 regAddr;     /* address of register  */
    GT_U32 regData;     /* data of register     */
    GT_U32 pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC   (memInfoPtr->key);
    GT_U32 clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC (memInfoPtr->key);
    GT_U32 memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC (memInfoPtr->key);

     /* AC5 use SIP_6 based management system */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ||
        (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* use Tile #0 for SIP_6 devices */
        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[0].memoryInterruptMask;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS;
    }

    /* read the Interrupt Mask Register */
    rc = prvCpssDfxMemoryRegRead (devNum, pipeIndex, clientIndex, memoryIndex,
                                  regAddr, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
        regData = U32_GET_FIELD_MAC(regData, 2, 2);
        switch(regData)
        {
            case 0:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_MASK_E;
                break;
            case 1:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
            case 2:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
            default: /* 3 */
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
        }
    }
    else if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
        *operationPtr = U32_GET_FIELD_MAC(regData, 1, 1);

        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGetFalcon function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity DFX interrupt for falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] memInfoPtr               - (pointer to) item of Device Data Integrity DB(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC).
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - Tile index
* @param[in] chipletIndex             - Chiplet index
*
* @param[out] errorTypePtr             - (pointer to) type of error interrupt
* @param[out] operationPtr             - (pointer to) mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGetFalcon
(
    IN GT_U8                                            devNum,
    IN const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *memInfoPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType,
    IN  GT_U32                                          tileIndex,
    IN  GT_U32                                          chipletIndex,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr

)
{
    GT_STATUS rc;
    GT_U32 regData;     /* register data */
    GT_U32 regAddr;
    GT_U32 pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC   (memInfoPtr->key);
    GT_U32 clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC (memInfoPtr->key);
    GT_U32 memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC (memInfoPtr->key);

    switch (dfxInstanceType)
    {
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
            sip6_tile_DFXRam[tileIndex].memoryInterruptMask;
            break;
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
            sip6_chiplet_DFXRam[tileIndex*4 + chipletIndex].memoryInterruptMask;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* read the Interrupt Mask Register */
    rc = prvCpssDfxMemoryRegReadFalcon (devNum, pipeIndex, clientIndex, memoryIndex,
                                        regAddr,
                                        dfxInstanceType, tileIndex, chipletIndex, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
        regData = U32_GET_FIELD_MAC(regData, 2, 2);
        switch(regData)
        {
            case 0:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_MASK_E;
                break;
            case 1:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
            case 2:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
            default: /* 3 */
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
        }
    }
    else if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
        *operationPtr = U32_GET_FIELD_MAC(regData, 1, 1);

        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagDataIntegrityEventMaskGet function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity interrupt.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] errorTypePtr             - (pointer to) type of error interrupt
* @param[out] operationPtr             - (pointer to) mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry, errorType
* @retval GT_NOT_SUPPORTED         - if memEntry doesn't support masking/unmasking
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityEventMaskGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_U32                      i;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex = 0; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex = 0; /* falcon raven index 0:15, per tile 0:3 */
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(operationPtr);

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* get list of HW tables from first logical */
            rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                devNum,
                memEntryPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* return as result the status of first HW table supporting masking */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

            for (i=0; hwTablesPtr[i] != LAST_VALID_TABLE_CNS; i++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[i];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */

                rc = internal_cpssDxChDiagDataIntegrityEventMaskGet (
                    devNum, &hwLocation, errorTypePtr, operationPtr);

                /* ignore tables either not found in DB  or not supporting the feature */
                if (rc != GT_NOT_SUPPORTED && rc != GT_NOT_FOUND)
                {
                    return rc;
                }
            }
            /* done  */
            return rc;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                hwTableInDb = dbArrayPtr[i].hwTableName;
                PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);

                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
                {
                    memoryFound = GT_TRUE;
                    if (dbArrayPtr[i].protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                        {
                            /* DFX memory mask operation */
                            rc = prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGetFalcon(devNum, &dbArrayPtr[i], dfxInstanceType,
                                tileIndex, chipletIndex, errorTypePtr, operationPtr);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                        }
                        else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                        {
                            /* DFX memory mask operation */
                            rc = prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGetFalcon(devNum, &dbArrayPtr[i], dfxInstanceType,
                                tileIndex, chipletIndex, errorTypePtr, operationPtr);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                        }
                        else
                        {
                            /* dfx instance type is invalid */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                    }
                    else if (dbArrayPtr[i].externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        /* non-DFX memory mask operation */
                        rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGet(devNum, dbArrayPtr[i].memType, errorTypePtr, operationPtr);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* the memory is not protected at all */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }

                    /* it's enough to read the first instance of certain memType */
                    break;
                }
            }
            if (memoryFound == GT_TRUE)
            {
                /* it's enough to read the first instance of certain memType */
                break;
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            /* find first entry of given memType/hwTable */
            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (dbArrayPtr[i].hwTableName == hwTable))
            {
                memoryFound = GT_TRUE;

                if (dbArrayPtr[i].protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* DFX memory mask operation */
                    rc  = prvCpssDxChDiagDataIntegrityDfxMemoryProtectEventsMaskGet(
                        devNum, &dbArrayPtr[i], errorTypePtr, operationPtr);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
                else if (dbArrayPtr[i].externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* non-DFX memory mask operation */
                    rc = prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGet(
                        devNum, dbArrayPtr[i].memType,
                        errorTypePtr, operationPtr);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
                else
                {
                    /* memory is not protected at all */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }

                /* it's enough to read the first instance of certain memType */
                break;
            }
        }
    }
    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityEventMaskGet function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity interrupt.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] errorTypePtr             - (pointer to) type of error interrupt
* @param[out] operationPtr             - (pointer to) mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry, errorType
* @retval GT_NOT_SUPPORTED         - if memEntry doesn't support masking/unmasking
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssDxChDiagDataIntegrityEventMaskGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityEventMaskGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorTypePtr, operationPtr));

    rc = internal_cpssDxChDiagDataIntegrityEventMaskGet(devNum, memEntryPtr, errorTypePtr, operationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorTypePtr, operationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error info from DFX unit.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Falcon.
*
* @param[in] devNum                   - PP device number
* @param[in] locationPtr              - (pointer to) memory location indexes
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxErrorInfoGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
{
    GT_U32    pipeIndex;
    GT_U32    clientIndex;
    GT_U32    memoryIndex;
    GT_BOOL   sip6Logic; /* GT_TRUE - SIP6 and above DFX logic, GT_FALSE - SIP5 logic */
    GT_U32    regAddr;
    GT_STATUS rc;

    pipeIndex   = locationPtr->memLocation.dfxPipeId;
    clientIndex = locationPtr->memLocation.dfxClientId;
    memoryIndex = locationPtr->memLocation.dfxMemoryId;

    /* SIP_6 devices and AC5 use DFX unit baseaddress as part of register offset.
       skip related checks. */
    sip6Logic = (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
                 PRV_CPSS_SIP_6_CHECK_MAC(devNum);

    /* read Error Counter Register. SIP6 based devices does not have counters */
    if (errorCounterPtr && (GT_FALSE == sip6Logic))
    {
        rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                         PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRORS_COUNTER_REG_CNS,
                                         0, 5, errorCounterPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* read Erroneous Address Register */
    if (failedRowPtr)
    {
        regAddr = sip6Logic ? PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[0].memoryErroneousAddress :
                    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRONEOUS_ADDR_REG_CNS;
        rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                         regAddr, 0, 16, failedRowPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* read ECC Syndrome Register */
    if (failedSyndromePtr)
    {
        regAddr = sip6Logic ? PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[0].memoryEccSyndrome :
                     PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ECC_SYNDROME_REG_CNS;

        rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                         regAddr, 0, 8, failedSyndromePtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxErrorInfoGetFalcon function
* @endinternal
*
* @brief   Function gets ECC/Parity error info from DFX unit for falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] locationPtr              - (pointer to) memory location indexes
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - Tile index
* @param[in] chipletIndex             - Chiplet index
*
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxErrorInfoGetFalcon
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType,
    IN  GT_U32                                          tileIndex,
    IN  GT_U32                                          chipletIndex,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
{
    GT_U32    pipeIndex;
    GT_U32    clientIndex;
    GT_U32    memoryIndex;
    GT_STATUS rc;
    GT_U32    regAddr;

    pipeIndex   = locationPtr->memLocation.dfxPipeId;
    clientIndex = locationPtr->memLocation.dfxClientId;
    memoryIndex = locationPtr->memLocation.dfxMemoryId;

    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
    {
        /* read Erroneous Address Register */
        if (failedRowPtr)
        {
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_tile_DFXRam[tileIndex].memoryErroneousAddress;
            rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                regAddr,
                0, 16, dfxInstanceType, tileIndex, 0, failedRowPtr);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        /* read ECC Syndrome Register */
        if (failedSyndromePtr)
        {
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_tile_DFXRam[tileIndex].memoryEccSyndrome;
            rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                regAddr,
                0, 8, dfxInstanceType, tileIndex, 0, failedSyndromePtr);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }
    else if(dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
    {
        /* read Erroneous Address Register */
        if (failedRowPtr)
        {
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_chiplet_DFXRam[tileIndex*4 + chipletIndex].memoryErroneousAddress;
            rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                regAddr,
                0, 16, dfxInstanceType, tileIndex, chipletIndex, failedRowPtr);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        /* read ECC Syndrome Register */
        if (failedSyndromePtr)
        {
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_chiplet_DFXRam[tileIndex*4 + chipletIndex].memoryEccSyndrome;
            rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                regAddr,
                0, 8, dfxInstanceType, tileIndex, chipletIndex, failedSyndromePtr);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }
    else
    {
        /* dfx instance type is invalid */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityErrorInfoGetLion2 function
* @endinternal
*
* @brief   Function gets ECC/Parity error info.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
* @param[in] locationPtr              - (pointer to) memory location indexes
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Error status or counter that isn't supported returns 0xFFFFFFFF
*
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityErrorInfoGetLion2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_U32                      regAddr;     /* register address */
    GT_U32                      regAddr1;    /* register address */
    GT_U32                      regData;     /* register data */
    GT_U32                      regOffset;   /* register offset */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */

    if(memType >= CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* init counter/status by default values*/
    if (errorCounterPtr)
    {
        *errorCounterPtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }
    if (failedRowPtr)
    {
        *failedRowPtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }
    if (failedSyndromePtr)
    {
        *failedSyndromePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }


    if((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E) ||
       (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E))
    {
        if (failedRowPtr)
        {
            if(locationPtr->mppmMemLocation.mppmId == 0)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->mppm[0].ecc.eccLastFailedAddress;
            }
            else
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->mppm[1].ecc.eccLastFailedAddress;
            }
            regOffset = (locationPtr->mppmMemLocation.bankId == 0) ? 0 : 12;

            return prvCpssHwPpPortGroupGetRegField(devNum,
                                                   locationPtr->mppmMemLocation.portGroupId,
                                                   regAddr, regOffset, 12, failedRowPtr);
        }
        return GT_OK;
    }
    else
    {
        /* Get portgroup for non-DFX memories */
        rc = searchMemType(devNum,
                           locationPtr->memLocation.dfxPipeId,
                           locationPtr->memLocation.dfxClientId,
                           locationPtr->memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    rc = GT_OK;
    switch(memType)
    {
        /* there are no counters or row statuses for these memories */
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_INFO_DESC_PREFETCH_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
            return GT_OK;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_EGR_VLAN_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.egrVlanParityErrorCounter;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, errorCounterPtr);
            }

            if (GT_OK == rc && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.vlanParityErrorLastEntries;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 12, 12, failedRowPtr);
            }

            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_ING_VLAN_E:

            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.ingVlanParityErrorCounter;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, errorCounterPtr);
            }

            if (GT_OK == rc && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.vlanParityErrorLastEntries;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 12, failedRowPtr);
            }
            return rc;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L2_E:

            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.l2PortIsolationParityErrorCounter;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, errorCounterPtr);

            }
            if(GT_OK == rc && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.portIsolationParityErrorLastEntries;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 12, failedRowPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_PORT_ISO_L3_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.l3PortIsolationParityErrorCounter;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, errorCounterPtr);
            }
            if(GT_OK == rc && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.portIsolationParityErrorLastEntries;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 12, 12, failedRowPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_SHT_VIDX_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.vidxParityErrorCounter;
                rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, errorCounterPtr);
            }
            if(GT_OK == rc && failedRowPtr)
            {

                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.sht.global.vidxParityErrorLastEntry;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 12, failedRowPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_NHE_TABLE_E:

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
            rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }
            if (errorCounterPtr)
            {
                *errorCounterPtr = U32_GET_FIELD_MAC(regData, 24, 8);
            }

            /* NHE Data Error Address Valid */
            /* This bit indicates whether the 'NHE Data Error Address' field
               is valid. This bit is set to one by the design upon every data
               error that occurs in the NHE table. This bit should be reset by
               software after reading the register content. */
            if((regData & 0x1) != 0)
            {
                if (failedRowPtr)
                {
                    *failedRowPtr = U32_GET_FIELD_MAC(regData, 1, 15);
                }

                rc = prvCpssHwPpPortGroupSetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 1, 0);
            }
            return rc;


        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG2_DESC_RETURN_TWO_FIFO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
                rc =  prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 24, 8, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_DESCRIPTOR_FIFO_E:

            if(locationPtr->memLocation.dfxMemoryId == 34)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerErrorReg;
                regAddr1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[0].policerErrorCntrReg;
            }
            else if(locationPtr->memLocation.dfxMemoryId == 32)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[1].policerErrorReg;
                regAddr1 = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->PLR[1].policerErrorCntrReg;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssHwPpPortGroupReadRegister(devNum, dbItemPtr->causePortGroupId, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            if(U32_GET_FIELD_MAC(regData, 31, 1) != 0)
            {
                if (failedRowPtr)
                {
                    *failedRowPtr = U32_GET_FIELD_MAC(regData, 0, 24);
                }

                /* get error counter */
                if (errorCounterPtr)
                {
                    rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr1, 0, 8, errorCounterPtr);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }
            }

            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_STG4_DESC_RETURN_TWO_FIFO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 24, 8, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IP_UNUSED_DATA_FIFO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerDataError;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 24, 8, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.global.memoryParityError.tokenBucketPriorityParityErrorCounter;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 16, errorCounterPtr);
            }

            if(rc == GT_OK && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.global.memoryParityError.parityErrorBadAddress;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 5, failedRowPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_WRR_STATE_VARIABLES_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.global.memoryParityError.stateVariablesParityErrorCounter;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 16, errorCounterPtr);
            }

            if(GT_OK == rc && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.dq.global.memoryParityError.parityErrorBadAddress;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 5, 5, failedRowPtr);
            }
            return rc;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_FREE_BUFS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.ll.linkList.linkListEccControl.fbufRamEccErrorCounter;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 16, errorCounterPtr);
            }

            if (GT_OK == rc && failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.ll.linkList.linkListEccControl.fbufRamEccStatus;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 12, failedRowPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_BUFFER_FIFO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.congestNotification.cnBufferFifoParityErrorsCnt;
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[0];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[1];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[2];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[3];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[4];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[5];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[6];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->txqVer1.queue.llfcRegs.pfcPipeCountersParityErrorsCounter[7];
                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 31, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E:
            if (errorCounterPtr)
            {
                if(locationPtr->memLocation.dfxMemoryId == 51)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bma.bmaCore[0].mcCntParityErrorCounter;
                }
                else if(locationPtr->memLocation.dfxMemoryId == 43)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bma.bmaCore[1].mcCntParityErrorCounter;
                }
                else if(locationPtr->memLocation.dfxMemoryId == 35)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bma.bmaCore[2].mcCntParityErrorCounter;
                }
                else if(locationPtr->memLocation.dfxMemoryId == 27)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bma.bmaCore[3].mcCntParityErrorCounter;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssHwPpPortGroupGetRegField(devNum, dbItemPtr->causePortGroupId, regAddr, 0, 8, errorCounterPtr);
            }
            return rc;

        default:
            /* read DFX interrupts info */
            rc = prvCpssDxChDiagDataIntegrityDfxErrorInfoGet(devNum, locationPtr,
                                                             errorCounterPtr, failedRowPtr, failedSyndromePtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityErrorInfoGetSip5 function
* @endinternal
*
* @brief   Function gets ECC/Parity error info.
*
* @note   APPLICABLE DEVICES:      AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
* @param[in] locationPtr              - (pointer to) memory location indexes
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed row
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Error status or counter that isn't supported returns 0xFFFFFFFF
*
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityErrorInfoGetSip5
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSegmentPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
{
    GT_STATUS                   rc = GT_OK;  /* return code */
    GT_U32                      regAddr;     /* register address */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr; /* pointer to map DB item */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex = 0; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex = 0; /* falcon raven index 0:15, per tile 0:3 */

    /* init counter/status */
    if (errorCounterPtr)
    {
        *errorCounterPtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }
    if (failedRowPtr)
    {
        *failedRowPtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }
    if (failedSegmentPtr)
    {
        *failedSegmentPtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }

    if (failedSyndromePtr)
    {
        *failedSyndromePtr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            return GT_OK;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            /* read last failing group, bank, buffer fields ase one field  */
            if (failedRowPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MPPM.dataIntegrity.lastFailingBuffer;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, failedRowPtr);
                /* note that *failedRowPtr is composed from group[19:18], bank[17:16], buffer[15:0] */
            }
            /* read failed segment */
            if (rc == GT_OK && failedSegmentPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MPPM.dataIntegrity.lastFailingSegment;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, failedSegmentPtr);
            }
            /* read failed syndrome */
            if (rc == GT_OK && failedSyndromePtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MPPM.dataIntegrity.statusFailedSyndrome;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 10, failedSyndromePtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
            if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
            {
                /* maybe return "Single ECC failed column" in failedSyndromePtr ? */
                return GT_OK;
            }
            /* Starting from bobcat2 b0 (Caelum, Aldrin, AC3X, Bobcat3 etc)
               the memory is protected by DFX so will be handled below. */
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
            /* The memory is protected by DFX so will be handled below. */
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            /* the memory interrupt
             * (PRV_CPSS_[BOBCAT2|ALDRIN]_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E)
             * is out of interrupt tree  because of erratum. See WA description:
             * PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */

            return GT_OK;
            /* if (errorCounterPtr) */
            /* { */
            /*     regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.dq[0].global.memoryParityError.tokenBucketPriorityParityErrorCntr; */
            /*     rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 15, errorCounterPtr); */
            /* } */
            /* if (GT_OK == rc && failedRowPtr) */
            /* { */
            /*     regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.dq[0].global.memoryParityError.parityErrorBadAddr; */
            /*     rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, failedRowPtr); */
            /* } */
            /* return rc; */

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TXQ.qcn.CNBufferFIFOParityErrorsCntr;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, errorCounterPtr);
            }
            return rc;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->BMA.mcCntParityErrorCounter1;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, errorCounterPtr);
            }

            break;
        default:
            break;
    }
    /* handle DFX memories */

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        dfxInstanceType = locationPtr->memLocation.dfxInstance.dfxInstanceType;

        /* check correctness of memType input parameter */
        rc = searchMemTypeFalcon(locationPtr->memLocation.dfxPipeId,
            locationPtr->memLocation.dfxClientId,
            locationPtr->memLocation.dfxMemoryId,
            dfxInstanceType,
            &dbItemPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
        if ((GT_U32)memType != dbItemPtr->memType)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
        {
            if(locationPtr->memLocation.dfxInstance.dfxInstanceIndex >= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_TILE_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "%d - table dfx instance type %d instance index %d",
                    memType, dfxInstanceType, locationPtr->memLocation.dfxInstance.dfxInstanceIndex);
            }
            tileIndex = locationPtr->memLocation.dfxInstance.dfxInstanceIndex;
            chipletIndex = 0; /* chipletIndex not used in this case */
        }
        else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
        {
            if(locationPtr->memLocation.dfxInstance.dfxInstanceIndex >= PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_CHIPLETS_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "%d - table dfx instance type %d instance index %d",
                    memType, dfxInstanceType, locationPtr->memLocation.dfxInstance.dfxInstanceIndex);
            }
            tileIndex = locationPtr->memLocation.dfxInstance.dfxInstanceIndex / 4;
            chipletIndex = locationPtr->memLocation.dfxInstance.dfxInstanceIndex % 4;
        }
        else
        {
            /* dfx instance type is invalid */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* get error details */
        rc = prvCpssDxChDiagDataIntegrityDfxErrorInfoGetFalcon(devNum, locationPtr,
            dfxInstanceType,
            tileIndex,
            chipletIndex,
            failedRowPtr,
            failedSyndromePtr);
    }
    else
    {
        /* check correctness of memType input parameter */
        rc = searchMemType(devNum,
            locationPtr->memLocation.dfxPipeId,
            locationPtr->memLocation.dfxClientId,
            locationPtr->memLocation.dfxMemoryId,
            &dbItemPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
        if ((GT_U32)memType != dbItemPtr->memType)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChDiagDataIntegrityDfxErrorInfoGet(devNum, locationPtr,
            errorCounterPtr,
            failedRowPtr,
            failedSyndromePtr);
    }
    return rc;
}

/**
* @internal internal_cpssDxChDiagDataIntegrityErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error details. An error counter info getting
*         is performed per whole hw/logical table or memory. An entry line number
*         information - ramRow, logicalTableEntryIndex, hwTableEntryIndex - is
*         ignored. A failed row/segment/syndrome info is available for RAM
*         location type only.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*                                      locationPtr        - (pointer to) memory location indexes
* @param[in] mppmMemLocationPtr       - MPPM memory coordinates. For memories other
*                                      than MPPM should be null!
*                                      (APPLICABLE DEVICES Lion2)
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSegmentPtr         - (pointer to) failed segment
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Error status or counter that isn't supported returns 0xFFFFFFFF
*
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityErrorInfoGet
(
    IN  GT_U8                                              devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmMemLocationPtr,
    OUT GT_U32                                            *errorCounterPtr,
    OUT GT_U32                                            *failedRowPtr,
    OUT GT_U32                                            *failedSegmentPtr,
    OUT GT_U32                                            *failedSyndromePtr
)
{
    GT_STATUS                                       rc; /* return code */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    location;
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    GT_BOOL isSingleMem = GT_FALSE;
    GT_U32  errorSum = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    GT_U32  errorCounter;
    GT_U32  failedRow=0;
    GT_U32  failedSegment=0;
    GT_U32  failedSyndrome=0;
    GT_U32  i;
    GT_U32  j;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorCounterPtr);
    CPSS_NULL_PTR_CHECK_MAC(failedRowPtr);
    CPSS_NULL_PTR_CHECK_MAC(failedSegmentPtr);
    CPSS_NULL_PTR_CHECK_MAC(failedSyndromePtr);

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* call API for every hw table relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
            for (i = 0; i < memEntryPtr->info.logicalEntryInfo.numOfLogicalTables; i++)
            {
                /* get HW tables  relevant to logical table */
                rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                    devNum,
                    memEntryPtr->info.logicalEntryInfo.logicaTableInfo[i].logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                for (j = 0; hwTablesPtr[j] != LAST_VALID_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                    rc = internal_cpssDxChDiagDataIntegrityErrorInfoGet(
                        devNum, &hwLocation,  NULL,
                        &errorCounter, &failedRow, &failedSegment, &failedSyndrome);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    /* accumulate sum */
                    if (errorCounter != PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
                    {
                        if (errorSum !=  PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
                        {
                            errorSum +=  errorCounter;
                        }
                        else
                        {
                            errorSum = errorCounter;
                        }
                    }
                }
            }
            *errorCounterPtr =  errorSum;
            return GT_OK;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            isSingleMem =  GT_FALSE;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }

            break;

        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            /* make compilers happy */
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;

            isSingleMem =  GT_TRUE;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            if (mppmMemLocationPtr && PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(devNum))
            {
                location.mppmMemLocation.portGroupId = mppmMemLocationPtr->portGroupId;
                location.mppmMemLocation.mppmId      = mppmMemLocationPtr->mppmId;
                location.mppmMemLocation.bankId      = mppmMemLocationPtr->bankId;
            }
            else
            {
                location.memLocation   = memEntryPtr->info.ramEntryInfo.memLocation;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* handle either RAM location or HW Table location */
    if (isSingleMem == GT_TRUE)
    {
        dbArrayEntryNum = 1; /* the cycle below will be iterated once */
        dbArrayPtr = NULL;
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);
    }

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        errorCounter = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS; /* initialization */
        if (isSingleMem == GT_FALSE && dbArrayPtr)
        {
            if  (dbArrayPtr[i].hwTableName == hwTable)
            {
                memType = dbArrayPtr[i].memType;
                location.memLocation.dfxPipeId   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                location.memLocation.dfxClientId = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                location.memLocation.dfxMemoryId = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);
            }
            else
            {
                continue;
            }
        }

        /* the main part: read Memory error status */

        switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_LION2_E:
                /* failedSegment is not relevant for Lion2 */
                failedSegment = 0;
                rc =  prvCpssDxChDiagDataIntegrityErrorInfoGetLion2(
                    devNum, memType, &location,
                    &errorCounter, &failedRow, &failedSyndrome);
                break;

            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E: /* bobcat2, bobk */
            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
            case CPSS_PP_FAMILY_DXCH_AC5_E:
            case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
            case CPSS_PP_FAMILY_DXCH_AC5X_E:
            case CPSS_PP_FAMILY_DXCH_AC5P_E:
            case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                rc =  prvCpssDxChDiagDataIntegrityErrorInfoGetSip5(
                    devNum, memType, &location,
                    &errorCounter, &failedRow, &failedSegment, &failedSyndrome);
                break;

            default:
                rc = GT_NOT_IMPLEMENTED;
                break;
        }

        if (rc != GT_OK)
        {
            return rc;
        }

        /* accumulate errors sum */
        if (errorCounter != PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
        {
            if (errorSum !=  PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
            {
                errorSum +=  errorCounter;
            }
            else
            {
                errorSum = errorCounter;
            }
        }
    }


    *errorCounterPtr = errorSum;
    if (isSingleMem ==  GT_TRUE)
    {
        *failedRowPtr      = failedRow;
        *failedSegmentPtr  = failedSegment;
        *failedSyndromePtr = failedSyndrome;
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error details. An error counter info getting
*         is performed per whole hw/logical table or memory. An entry line number
*         information - ramRow, logicalTableEntryIndex, hwTableEntryIndex - is
*         ignored. A failed row/segment/syndrome info is available for RAM
*         location type only.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*                                      locationPtr        - (pointer to) memory location indexes
* @param[in] mppmMemLocationPtr       - MPPM memory coordinates. For memories other
*                                      than MPPM should be null!
*                                      (APPLICABLE DEVICES Lion2)
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSegmentPtr         - (pointer to) failed segment
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Error status or counter that isn't supported returns 0xFFFFFFFF
*
*/
GT_STATUS cpssDxChDiagDataIntegrityErrorInfoGet
(
    IN  GT_U8                                              devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmMemLocationPtr,
    OUT GT_U32                                            *errorCounterPtr,
    OUT GT_U32                                            *failedRowPtr,
    OUT GT_U32                                            *failedSegmentPtr,
    OUT GT_U32                                            *failedSyndromePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityErrorInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr,mppmMemLocationPtr,
                            errorCounterPtr, failedRowPtr, failedSegmentPtr,
                            failedSyndromePtr));

    rc = internal_cpssDxChDiagDataIntegrityErrorInfoGet(devNum, memEntryPtr, mppmMemLocationPtr,
            errorCounterPtr, failedRowPtr, failedSegmentPtr, failedSyndromePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr,
                              errorCounterPtr, failedRowPtr, failedSegmentPtr,
                              failedSyndromePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionLion2 function
* @endinternal
*
* @brief   Check if memory error injection status can be read/write for
*         the specified memory item of device's Data Integrity DB.
*         Additionally if a writing procedure is unusual for this memory
*         the function can do it by itself.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - PP device number
* @param[in] memInfoPtr               - (pointer to) an item of Device Data Integrity DB.
* @param[in] isReadOp                 - GT_TRUE:  only check if error injection status can be read.
*                                      GT_FALSE: check if error injection status can be written.
*                                      Do writing if the procedure is unusual.
* @param[in] injectEnable             - enable/disable error injection. Ignored if isReadOp == true.
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @param[out] continuePtr              - if calling code can continue with read/write error injection.
*                                      GT_FALSE means the function either made the
*                                      some nonstandard handling by itself or the handling
*                                      must not be done for some reasons for this item.
*
* @retval GT_OK                    - on success. Means a memory injections is supported.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memType, injectMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if operation is not supported for this memory type
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionLion2
(
    IN  GT_U8                                           devNum,
    IN  const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *memInfoPtr,
    IN  GT_BOOL                                         isReadOp,
    IN  GT_BOOL                                         injectEnable,
    OUT GT_BOOL                                         *continuePtr
)
{
    GT_STATUS                                  rc;          /* return code */
    GT_U32                                     regAddr;     /* register address */
    GT_U32                                     regData;     /* register data */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType = memInfoPtr->memType;

    *continuePtr = GT_TRUE;

    if((memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E) &&
       (memInfoPtr->externalProtectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(memType)
    {
        /* TCAM memories */
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:

            if (isReadOp == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            *continuePtr = GT_FALSE;
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable)
            {
                if(injectEnable == GT_FALSE)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }

                if(memInfoPtr->memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E)
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.tcamReadMaskParityBase;
                }
                else
                {
                    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamReadMaskParityBase;
                }

                /* Read Parity_Y table */
                rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, (regAddr), &regData);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* invert first bit in Y parity */
                regData ^= 1;

                /* Write Parity_Y table */
                rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, (regAddr), regData);
                return rc;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

        /* there are no interrupts for MLL memories */
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* BMA: special memory vendor (MISL_SR512) - no error injection via DFX
           TXQ Q Counters: special memory vendor (MISL_SR512) - no error injection via DFX
           TXQ Q Index FIFO: useless(spare) memory - no access with traffic and/or MG
           TXQ: false interrupts that cause storming */
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MULTICAST_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_0_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_1_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_2_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_3_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_4_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_5_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_6_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_SRC_PIPE_7_COUNTERS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_2_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_3_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_4_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_5_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_6_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_PFC_IND_FIFO_PIPE_7_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_EGR_MC_FIFO_3_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        /* MPPM memories */
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E:
            /* There is no data integity error per specific RAM.
               Only BANK0/1 indication supported. Bank reflects state 8 data
               and 1 ecc memories.
               Therefore in order to simulate sigle ECC error injection it is
               needed to skip 7 memories and ECC memory block. */
            if( PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(memInfoPtr->key) % 8 != 7)
            {
                *continuePtr = GT_FALSE;
            }
            GT_ATTR_FALLTHROUGH;

        default:
            /* All other Non-DFX memories:
               In order to simulate and catch data integrity event for the
               memories with more than one RAM in table entry, but one interrupt
               per table(not RAM) line, it is needed to skip all RAMs in line
               except the first one. */
            if (memInfoPtr->externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                *continuePtr = (memInfoPtr->firstTableDataBit == 0);
            }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5 function
* @endinternal
*
* @brief   Check if memory error injection status can be read/write for
*         the specified memory item of device's Data Integrity DB.
*         Additionally if a writing procedure is unusual for this memory
*         the function can do it by itself.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] memInfoPtr               -  (pointer to) an item of Device Data Integrity DB.
*
* @param[out] continuePtr              - if calling code can continue with read/write error injection.
*                                      GT_FALSE means the function either made the
*                                      some nonstandard handling by itself or the handling
*                                      must not be done for some reasons for this item.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memType, injectMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - if operation is not supported for this memory type
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5
(
    IN  const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *memInfoPtr,
    OUT GT_BOOL                                         *continuePtr
)
{
    (void)memInfoPtr;/* prevent warnings */

    *continuePtr = GT_TRUE;

    /* all memories support error injection */
    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Function enables/disables injection of error during next write operation.
*         Injection is applied to whole hw/logical table or to all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location.
* @param[in] injectMode               - error injection mode
*                                      relevant only for ECC protected memories
* @param[in] injectEnable             - enable/disable error injection
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, injectMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*
* @note Memory with parity can not be configured with multiple error injection
*
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* pipe index */
    GT_U32                      clientIndex; /* client index */
    GT_U32                      memoryIndex; /* memory index */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                     dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                     hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                           PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    GT_BOOL                              doIt = GT_TRUE;
    const CPSS_DXCH_TABLE_ENT            *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC hwLocation;   /* hw location */
    GT_U32                               hwTableInDb;  /* value of HW Table in DB entry */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex; /* falcon raven index 0:15, per tile 0:3 */
    GT_U32                                          regAddr;    /* register address */
    GT_U32                                          pipeIndexCheck; /* DFX pipe for tile index check */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);

    if (injectMode != CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E &&
        injectMode != CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* call cpssDxChDiagDataIntegrityErrorInjectionConfigSet for every
               hw table relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

            for (i = 0; i < memEntryPtr->info.logicalEntryInfo.numOfLogicalTables; i++)
            {
                rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                    devNum,
                    memEntryPtr->info.logicalEntryInfo.logicaTableInfo[i].logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                /* if some table doesn't support the feature or is absent in
                   the DB skip it. But if ALL tables are skipped return error */
                rc1 = GT_NOT_FOUND;
                for (j=0; hwTablesPtr[j] != LAST_VALID_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                    rc = internal_cpssDxChDiagDataIntegrityErrorInjectionConfigSet(
                        devNum, &hwLocation, injectMode, injectEnable);
                    if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                    {
                       /* ignore tables either not found in DB  or not supporting the feature */
                        continue;
                    }
                    else if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc1 = rc;
                }
                /* neither of tables handled successfully */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            }

            return GT_OK;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;

        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                hwTableInDb = dbArrayPtr[i].hwTableName;
                PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);
                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
                {
                    memoryFound = GT_TRUE;
                    pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                    clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                    memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);

                    for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
                    {
                        if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                        {
                            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_INDEX_CHECK(tileIndex, pipeIndex, pipeIndexCheck)
                            if(pipeIndexCheck)
                            {
                                continue;
                            }
                            rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5(
                                &dbArrayPtr[i], &doIt);
                            if (rc != GT_OK)
                            {
                              return rc;
                            }

                            if (doIt == GT_FALSE)
                            {
                              continue;
                            }

                            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                      sip6_tile_DFXRam[tileIndex].memoryControl;
                            /* NOTE: In order to switch error injection modes
                               <Error Injection Enable> must be set to Disable */

                            /* write to Memory Control Register - disable error injection */
                            rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                regAddr,
                                1, 1, 0, dfxInstanceType, tileIndex, 0);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }

                            if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                            {
                                regData = 0;
                            }
                            else
                            {
                                if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E)
                                {
                                    regData = 0;
                                }
                                else if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E)
                                {
                                    regData = 1;
                                }
                                else
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                                }
                            }

                            /* write to Memory Control Register - error injection mode configuration */
                            rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                regAddr,
                                2,1, regData, dfxInstanceType, tileIndex, 0);
                            if(GT_OK != rc)
                            {
                              return rc;
                            }

                            if(injectEnable == GT_TRUE)
                            {
                                /* write to Memory Control Register - enable error injection */
                                rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    1, 1, 1, dfxInstanceType, tileIndex, 0);

                                if(GT_OK != rc)
                                {
                                    return rc;
                                }
                            }

                            /* it's take some time in DFX bus to configure register.
                               Add this Read to guaranty that write done. */
                            rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                regAddr,
                                1, 1, dfxInstanceType, tileIndex, 0, &regData);

                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                        }
                        else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                        {
                            for (chipletIndex = 0; chipletIndex < FALCON_RAVENS_PER_TILE; chipletIndex++)
                            {
                                PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, chipletIndex);

                                rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5(
                                    &dbArrayPtr[i], &doIt);
                                if (rc != GT_OK)
                                {
                                    return rc;
                                }

                                if (doIt == GT_FALSE)
                                {
                                    continue;
                                }

                                regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                          sip6_chiplet_DFXRam[tileIndex*4 + chipletIndex].memoryControl;
                                /* NOTE: In order to switch error injection modes
                                   <Error Injection Enable> must be set to Disable */

                                /* write to Memory Control Register - disable error injection */
                                rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    1, 1, 0, dfxInstanceType, tileIndex, chipletIndex);
                                if(GT_OK != rc)
                                {
                                    return rc;
                                }

                                if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                                {
                                    regData = 0;
                                }
                                else
                                {
                                    if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E)
                                    {
                                        regData = 0;
                                    }
                                    else if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E)
                                    {
                                        regData = 1;
                                    }
                                    else
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                                    }
                                }

                                /* write to Memory Control Register - error injection mode configuration */
                                rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    2,1, regData, dfxInstanceType, tileIndex, chipletIndex);
                                if(GT_OK != rc)
                                {
                                    return rc;
                                }

                                if(injectEnable == GT_TRUE)
                                {
                                    /* write to Memory Control Register - enable error injection */
                                    rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                        regAddr,
                                        1, 1, 1, dfxInstanceType, tileIndex, chipletIndex);

                                    if(GT_OK != rc)
                                    {
                                        return rc;
                                    }
                                }

                                /* it's take some time in DFX bus to configure register.
                                    Add this Read to guaranty that write done. */
                                rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    1, 1, dfxInstanceType, tileIndex, chipletIndex, &regData);

                                if(GT_OK != rc)
                                {
                                    return rc;
                                }
                            }
                        }
                        else
                        {
                            /* dfx instance type is invalid */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                    }
                }
            }
        }
    }
    else
    {
        /* the code below is relevant to HW or RAM locations. */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            hwTableInDb = dbArrayPtr[i].hwTableName;
            PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);

            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
            {
                memoryFound = GT_TRUE;

                switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    case CPSS_PP_FAMILY_DXCH_LION2_E:
                        rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionLion2(
                            devNum, &dbArrayPtr[i], GT_FALSE, injectEnable, &doIt);
                        break;
                    case CPSS_PP_FAMILY_DXCH_BOBCAT2_E: /* bobcat2, bobk */
                    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                    case CPSS_PP_FAMILY_DXCH_AC3X_E:
                    case CPSS_PP_FAMILY_DXCH_AC5_E:
                    case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                    case CPSS_PP_FAMILY_DXCH_AC5X_E:
                    case CPSS_PP_FAMILY_DXCH_AC5P_E:
                    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                        rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5(
                            &dbArrayPtr[i], &doIt);
                        break;
                    default:
                        /* shouldn't happen */
                        rc = GT_NOT_IMPLEMENTED;
                }
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (doIt == GT_FALSE)
                {
                    continue;
                }

                pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);


                /* NOTE: In order to switch error injection modes
                    <Error Injection Enable> must be set to Disable */

                /* write to Memory Control Register - disable error injection */
                rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXRam.memoryControl,
                    1, 1, 0);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                {
                    regData = 0;
                }
                else
                {
                    if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E)
                    {
                        regData = 0;
                    }
                    else if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E)
                    {
                        regData = 1;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }

                /* write to Memory Control Register - error injection mode configuration */
                rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXRam.memoryControl,
                    2,1, regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(injectEnable == GT_TRUE)
                {
                    /* write to Memory Control Register - enable error injection */
                    rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXRam.memoryControl,
                        1, 1, 1);

                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }

                /* it's take some time in DFX bus to configure register.
                   Add this Read to guaranty that write done. */
                rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXRam.memoryControl,
                    1, 1, &regData);

                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }
    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Function enables/disables injection of error during next write operation.
*         Injection is applied to whole hw/logical table or to all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location.
* @param[in] injectMode               - error injection mode
*                                      relevant only for ECC protected memories
* @param[in] injectEnable             - enable/disable error injection
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, injectMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*
* @note Memory with parity can not be configured with multiple error injection
*
*/
GT_STATUS cpssDxChDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityErrorInjectionConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, injectMode, injectEnable));

    rc = internal_cpssDxChDiagDataIntegrityErrorInjectionConfigSet(devNum,
                                    memEntryPtr, injectMode, injectEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, injectMode, injectEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChDiagDataIntegrityErrorInjectionConfigGet function
* @endinternal
*
* @brief   Function gets status of error injection.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] injectModePtr            - (pointer to) error injection mode
* @param[out] injectEnablePtr          - (pointer to) enable/disable error injection
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memEntry
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* pipe index */
    GT_U32                      clientIndex; /* client index */
    GT_U32                      memoryIndex; /* memory index */
    GT_U32                      i;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    GT_BOOL                     canRead;     /* if can read injection config */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex = 0; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex = 0; /* falcon raven index 0:15, per tile 0:3 */
    GT_U32                                          regAddr;
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(injectModePtr);
    CPSS_NULL_PTR_CHECK_MAC(injectEnablePtr);

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:

            /* get list of HW tables from first logical */

            rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                devNum,
                memEntryPtr->info.logicalEntryInfo.logicaTableInfo[0].logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* return as result the status of first HW table supporting the error injection */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;
            for (i=0; hwTablesPtr[i] != LAST_VALID_TABLE_CNS; i++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[i];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */

                rc = internal_cpssDxChDiagDataIntegrityErrorInjectionConfigGet (
                    devNum, &hwLocation, injectModePtr, injectEnablePtr);
                /* ignore tables either not found in DB  or not supporting the feature */
                if (rc != GT_NOT_SUPPORTED && rc != GT_NOT_FOUND)
                {
                    return rc;
                }
            }
            /* done  */
            return rc;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                hwTableInDb = dbArrayPtr[i].hwTableName;
                PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);
                canRead = GT_FALSE; /* initialization */
                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
                {
                    memoryFound = GT_TRUE;
                    rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5(
                        &dbArrayPtr[i], &canRead);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    if (canRead == GT_FALSE)
                    {
                        continue;
                    }

                    pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                    clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                    memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);

                    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                    {
                        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                            sip6_tile_DFXRam[tileIndex].memoryControl;
                        /* read Memory Control Register, */
                        rc = prvCpssDfxMemoryRegReadFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                            regAddr,
                            dfxInstanceType, tileIndex, chipletIndex, &regData);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                    {
                        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                            sip6_chiplet_DFXRam[tileIndex*4 + chipletIndex].memoryControl;
                        /* read Memory Control Register, */
                        rc = prvCpssDfxMemoryRegReadFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                            regAddr,
                            dfxInstanceType, tileIndex, chipletIndex, &regData);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* dfx instance type is invalid */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }

                    *injectModePtr = (U32_GET_FIELD_MAC(regData, 2, 1) == 1) ?
                        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E :
                        CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;

                    *injectEnablePtr = (U32_GET_FIELD_MAC(regData, 1, 1) == 1) ? GT_TRUE : GT_FALSE;
                    break;
                }
            }
        }
    }
    else
    {
        /* the code below is relevant to HW or RAM locations. */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            hwTableInDb = dbArrayPtr[i].hwTableName;
            PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);

            canRead = GT_FALSE; /* initialization */
            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
            {
                memoryFound = GT_TRUE;

                switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    case CPSS_PP_FAMILY_DXCH_LION2_E:
                        rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionLion2(
                            devNum, &dbArrayPtr[i], GT_TRUE, GT_FALSE, &canRead);
                        break;
                    case CPSS_PP_FAMILY_DXCH_BOBCAT2_E: /* bobcat2, bobk */
                    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                    case CPSS_PP_FAMILY_DXCH_AC3X_E:
                    case CPSS_PP_FAMILY_DXCH_AC5_E:
                    case CPSS_PP_FAMILY_DXCH_AC5X_E:
                    case CPSS_PP_FAMILY_DXCH_AC5P_E:
                    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                    case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                        rc = prvCpssDxChDiagDataIntegrityCheckMemForErrorInjectionSip5(
                            &dbArrayPtr[i], &canRead);
                        break;
                    default:
                        /* shouldn't happen */
                        rc = GT_NOT_IMPLEMENTED;
                }
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (canRead == GT_FALSE)
                {
                    continue;
                }

                pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);


                /* read Memory Control Register, */
                rc = prvCpssDfxMemoryRegRead(devNum, pipeIndex, clientIndex, memoryIndex,
                    PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXRam.memoryControl,
                    &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                *injectModePtr = (U32_GET_FIELD_MAC(regData, 2, 1) == 1) ?
                    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E :
                    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;

                *injectEnablePtr = (U32_GET_FIELD_MAC(regData, 1, 1) == 1) ? GT_TRUE : GT_FALSE;

                break;
            }
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityErrorInjectionConfigGet function
* @endinternal
*
* @brief   Function gets status of error injection.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] injectModePtr            - (pointer to) error injection mode
* @param[out] injectEnablePtr          - (pointer to) enable/disable error injection
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memEntry
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssDxChDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityErrorInjectionConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, injectModePtr, injectEnablePtr));

    rc = internal_cpssDxChDiagDataIntegrityErrorInjectionConfigGet(devNum,
                             memEntryPtr, injectModePtr, injectEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, injectModePtr, injectEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagDataIntegrityErrorCountEnableSet function
* @endinternal
*
* @brief   Function enables/disable error counter. Enabling/disabling is performed
*         for whole hw/logical table or for all memories of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] errorType                - error type
* @param[in] countEnable              - enable/disable error counter
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not suppoted memory type
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* DFX pipe number */
    GT_U32                      clientIndex; /* DFX client number */
    GT_U32                      memoryIndex; /* DFX memory number */
    GT_U32                      fieldOffset; /* register field offset */
    GT_U32                      fieldLength; /* register field length */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      dbMemType; /* memory type of current DB entry */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex; /* falcon raven index 0:15, per tile 0:3 */
    GT_U32                                          regAddr;
    GT_U32                                          pipeIndexCheck; /* DFX pipe for tile index check */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E | CPSS_AC5_E
                                          | CPSS_XCAT2_E |CPSS_LION_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);

    if (errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E &&
        errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E &&
        errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E &&
        errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:

            /* call API for every hw table relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

            for (i = 0; i< memEntryPtr->info.logicalEntryInfo.numOfLogicalTables; i++)
            {
                rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                    devNum,
                    memEntryPtr->info.logicalEntryInfo.logicaTableInfo[i].logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                /* if some table doesn't support the feature or is absent in
                   the DB skip it. But if ALL tables are skipped return error */
                rc1 = GT_NOT_FOUND;
                for (j=0; hwTablesPtr[j] != LAST_VALID_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                    rc = internal_cpssDxChDiagDataIntegrityErrorCountEnableSet(
                        devNum, &hwLocation, errorType, countEnable);
                    if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                    {
                        continue;
                    }
                    else if (rc != GT_OK)
                    {
                        return rc;
                    }
                    rc1 = rc;
                }
                /* neither of tables supports the feature */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            }

            /* done */
            return GT_OK;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;

            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }

            break;

        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    };

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                hwTableInDb = dbArrayPtr[i].hwTableName;
                PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);
                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
                {
                    memoryFound = GT_TRUE;
                    dbMemType = dbArrayPtr[i].memType;

                    if (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        /* device specific handling of some special types of non-DFX memories */
                        if (prvIsMemProtectedButNotSupportedSip5(devNum, dbMemType))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }

                        /* device independent handling of rest of non-DFX memory types */
                        if (dbArrayPtr[i].externalProtectionType ==
                            CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                        {
                            /* the memory is not protected at all (neither DFX nor non-DFX). */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                        else if (countEnable == GT_TRUE)
                        {
                            return GT_OK;
                        }
                        else
                        {
                            /* non-DFX memories doesn't support counter disable */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                    }

                    pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                    clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                    memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);

                    if(countEnable == GT_TRUE)
                    {
                        for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
                        {
                            if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                            {
                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_INDEX_CHECK(tileIndex, pipeIndex, pipeIndexCheck)
                                if(pipeIndexCheck)
                                {
                                    continue;
                                }
                                regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[tileIndex].memoryControl;
                                /* NOTE: In order to set configuration to other value then Disable,
                                    the follow configuration bits must be set to Disable in
                                    advanced: <Address Monitoring Int Select> */
                                    /* write to Memory Control Register - disable address monitoring mechanism */
                                rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    12, 2, 0, dfxInstanceType, tileIndex, 0);
                                if(GT_OK != rc)
                                {
                                    return rc;
                                }
                            }
                            else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                            {
                                for (chipletIndex = 0; chipletIndex < FALCON_RAVENS_PER_TILE; chipletIndex++)
                                {
                                    PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, chipletIndex);

                                    regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                        sip6_tile_DFXRam[tileIndex*4 + chipletIndex].memoryControl;
                                    /* NOTE: In order to set configuration to other value then Disable,
                                        the follow configuration bits must be set to Disable in
                                        advanced: <Address Monitoring Int Select> */
                                    /* write to Memory Control Register - disable address monitoring mechanism */
                                    rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                        regAddr,
                                        12, 2, 0, dfxInstanceType, tileIndex, chipletIndex);
                                    if(GT_OK != rc)
                                    {
                                        return rc;
                                    }
                                }
                            }
                            else
                            {
                                /* dfx instance type is invalid */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                            }
                        }
                    }
                    if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                    {
                        fieldOffset = 5;
                        fieldLength = 2;

                        switch(errorType)
                        {
                            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                                regData = BOOL2BIT_MAC(countEnable);
                                break;

                            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                                regData = BOOL2BIT_MAC(countEnable) ? 2 : 0;
                                break;

                            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
                                regData = BOOL2BIT_MAC(countEnable) ? 3 : 0;
                                break;

                            default:
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }
                    }
                    else if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                    {
                        if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)
                        {
                            fieldOffset = 6;
                            fieldLength = 1;
                            regData = BOOL2BIT_MAC(countEnable);
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }

                    for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
                    {
                        if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                        {
                            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_INDEX_CHECK(tileIndex, pipeIndex, pipeIndexCheck)
                            if(pipeIndexCheck)
                            {
                                continue;
                            }
                            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                sip6_tile_DFXRam[tileIndex].memoryControl;
                            /* write to Memory Control Register - enable/disable error counter */
                            rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                regAddr,
                                fieldOffset, fieldLength, regData, dfxInstanceType, tileIndex, 0);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }

                            /* it's take some time in DFX bus to configure register.
                                Add this Read to guaranty that write done before exit from this function. */
                            rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                regAddr,
                                fieldOffset, fieldLength, dfxInstanceType, tileIndex, 0, &regData);
                            if(GT_OK != rc)
                            {
                                return rc;
                            }
                        }
                        else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                        {
                            for (chipletIndex = 0; chipletIndex < FALCON_RAVENS_PER_TILE; chipletIndex++)
                            {
                                PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, chipletIndex);

                                regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[tileIndex*4 + chipletIndex].memoryControl;
                                /* write to Memory Control Register - enable/disable error counter */
                                rc = prvCpssDfxMemorySetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    fieldOffset, fieldLength, regData, dfxInstanceType, tileIndex, chipletIndex);
                                if(GT_OK != rc)
                                {
                                    return rc;
                                }

                                /* it's take some time in DFX bus to configure register.
                                    Add this Read to guaranty that write done before exit from this function. */
                                rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                                    regAddr,
                                    fieldOffset, fieldLength, dfxInstanceType, tileIndex, chipletIndex, &regData);
                                if(GT_OK != rc)
                                {
                                    return rc;
                                }
                            }
                        }
                        else
                        {
                            /* dfx instance type is invalid */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                    }
                }
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            hwTableInDb = dbArrayPtr[i].hwTableName;
            PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);

            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
            {
                memoryFound = GT_TRUE;
                dbMemType = dbArrayPtr[i].memType;

                if (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* device specific handling of some special types of non-DFX memories */

                    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        case CPSS_PP_FAMILY_DXCH_LION2_E:
                        switch (dbMemType)
                        {
                            /* there are no interrupts for MLL memories */
                            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E:
                            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E:
                            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E:
                            /* no counters for TCAM memories */
                            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
                            case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                                break;
                            default:
                                break;
                        }
                            break;

                        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E: /* bobcat2, bobk */
                        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                        case CPSS_PP_FAMILY_DXCH_AC3X_E:
                        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                        if (prvIsMemProtectedButNotSupportedSip5(devNum, dbMemType))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                            break;

                        default:
                            /* other devices */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
                    }

                    /* device independent handling of rest of non-DFX memory types */
                    if (dbArrayPtr[i].externalProtectionType ==
                        CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        /* the memory is not protected at all (neither DFX nor non-DFX). */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                    else if (countEnable == GT_TRUE)
                    {
                        return GT_OK;
                    }
                    else
                    {
                        /* non-DFX memories doesn't support counter disable */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                }

                pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);

                /* NOTE: In order to set configuration to other value then Disable,
                   the follow configuration bits must be set to Disable in
                   advanced: <Address Monitoring Int Select> */
                if(countEnable == GT_TRUE)
                {
                    /* write to Memory Control Register - disable address monitoring mechanism */
                    rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                        PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                        12, 2,0);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }
                }

                if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                {
                    fieldOffset = 5;
                    fieldLength = 2;

                    switch(errorType)
                    {
                        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                            regData = BOOL2BIT_MAC(countEnable);
                            break;

                        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                            regData = BOOL2BIT_MAC(countEnable) ? 2 : 0;
                            break;

                        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
                            regData = BOOL2BIT_MAC(countEnable) ? 3 : 0;
                            break;

                        default:
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
                else if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                {
                    if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)
                    {
                        fieldOffset = 6;
                        fieldLength = 1;
                        regData = BOOL2BIT_MAC(countEnable);
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                /* write to Memory Control Register - enable/disable error counter */
                rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                    fieldOffset, fieldLength, regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* it's take some time in DFX bus to configure register.
                    Add this Read to guaranty that write done before exit from this function. */
                rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                    fieldOffset, fieldLength, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }
    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityErrorCountEnableSet function
* @endinternal
*
* @brief   Function enables/disable error counter. Enabling/disabling is performed
*         for whole hw/logical table or for all memories of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] errorType                - error type
* @param[in] countEnable              - enable/disable error counter
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not suppoted memory type
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssDxChDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityErrorCountEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorType, countEnable));

    rc = internal_cpssDxChDiagDataIntegrityErrorCountEnableSet(devNum,
                                           memEntryPtr, errorType, countEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorType, countEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagDataIntegrityErrorCountEnableGet function
* @endinternal
*
* @brief   Function gets status of error counter. Status getting is performed
*         to per whole hw/logical table or memory. An entry line number -
*         ramRow, logicalTableEntryIndex, hwTableEntryIndex - is ignored.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] countEnablePtr           - (pointer to) status of error counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_VALUE             - if HW tables composing logical table
*                                       have different error enabling status
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT GT_BOOL                                         *countEnablePtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* DFX pipe number */
    GT_U32                      clientIndex; /* DFX client number */
    GT_U32                      memoryIndex; /* DFX memory number */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType;
    GT_BOOL                                         countEnable;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      dbMemType; /* memory type of current DB entry */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32                                          tileIndex = 0; /* falcon tile index 0:3 */
    GT_U32                                          chipletIndex = 0; /* falcon raven index 0:15, per tile 0:3 */
    GT_U32                                          regAddr;
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                          | CPSS_CH2_E   | CPSS_CH3_E  | CPSS_XCAT_E   | CPSS_XCAT3_E | CPSS_AC5_E
                                          | CPSS_XCAT2_E | CPSS_LION_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(countEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(errorTypePtr);

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

            for (i = 0; i < memEntryPtr->info.logicalEntryInfo.numOfLogicalTables; i++)
            {
                rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                    devNum,
                    memEntryPtr->info.logicalEntryInfo.logicaTableInfo[i].logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                rc1 = GT_NOT_FOUND;
                for (j = 0; hwTablesPtr[j] != LAST_VALID_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */

                    rc = internal_cpssDxChDiagDataIntegrityErrorCountEnableGet(
                        devNum, &hwLocation, &errorType, &countEnable);
                    /* ignore tables not supporting error counting */
                    if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                    {
                        continue;
                    }
                    else if (rc != GT_OK)
                    {
                        return rc;
                    }

                    if (rc1 != GT_OK)
                    {
                        /* this is first table supporting the feature */
                        rc1 = rc;
                        *errorTypePtr = errorType;
                        *countEnablePtr = countEnable;
                    }
                    else if (*countEnablePtr != countEnable)
                    {
                        /* logical table has HW tables with different counter status */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                    }
                    else if (*errorTypePtr != errorType)
                    {

                        /* don't treat the case when some table(s) has errorType SINGLE_AND_MULTIPLE_ECC
                         * and rest of tables have errorType any kind of ECC - SINGLE or MULTIPLE - as error.
                         * The logical table is treated as SINGLE_AND_MULTIPLE_ECC errorType in such cases */
                        if (
                            (*errorTypePtr == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E &&
                             errorType     != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E /* any kind of ECC */)
                            ||
                            (errorType     == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E &&
                             *errorTypePtr != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E /* any kind of ECC */))
                        {
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                        }
                        else
                        {
                            /* logical table has HW tables with different errorType */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                        }
                    }
                }
                /* neither of tables supports masking */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            }

            /* done */
            return GT_OK;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;

            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }

            break;

        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    };

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                hwTableInDb = dbArrayPtr[i].hwTableName;
                PRV_DATA_INTEGRITY_SPECIAL_HW_TBL_CONVERT_MAC(hwTableInDb);
                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (hwTableInDb == hwTable))
                {
                    memoryFound = GT_TRUE;
                    dbMemType = dbArrayPtr[i].memType;
                    if (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                    {
                        if (prvIsMemProtectedButNotSupportedSip5(devNum, dbMemType))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                        }
                        /* device independent handling of rest of non-DFX memory types */
                        switch (dbArrayPtr[i].externalProtectionType)
                        {
                            case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E:
                                /* the memory is not protected at all (neither DFX nor non-DFX). */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

                            case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E:
                                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                                break;
                            default:
                                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                                break;
                        }
                        /* non-DFX memories doesn't support counter disable */
                        *countEnablePtr = GT_TRUE;
                        return GT_OK;
                    }
                    pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                    clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                    memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);

                    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                    {
                        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                            sip6_tile_DFXRam[tileIndex].memoryControl;
                            /* read from Memory Control Register - enable/disable error counter */
                        rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                            regAddr,
                            5, 2, dfxInstanceType, tileIndex, chipletIndex, &regData);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                    {
                        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                            sip6_tile_DFXRam[tileIndex*4 + chipletIndex].memoryControl;
                        /* read from Memory Control Register - enable/disable error counter */
                        rc = prvCpssDfxMemoryGetRegFieldFalcon(devNum, pipeIndex, clientIndex, memoryIndex,
                            regAddr,
                            5, 2, dfxInstanceType, tileIndex, chipletIndex, &regData);
                        if(GT_OK != rc)
                        {
                            return rc;
                        }
                    }
                    else
                    {
                        /* dfx instance type is invalid */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }

                    if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                    {
                        switch(regData)
                        {
                            case 0:
                                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                                *countEnablePtr = GT_FALSE;
                                break;

                            case 1:
                                *countEnablePtr = GT_TRUE;
                                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                                break;

                            case 2:
                                *countEnablePtr = GT_TRUE;
                                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                                break;

                            default:
                                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                                *countEnablePtr = GT_TRUE;
                                break;
                        }
                    }
                    else if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                    {
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                        *countEnablePtr = BIT2BOOL_MAC((regData >> 1) & 0x1);
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                    break;
                }
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            /* find first entry of given memType/hwTable */
            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (dbArrayPtr[i].hwTableName == hwTable))
            {
                memoryFound = GT_TRUE;
                dbMemType = dbArrayPtr[i].memType;

                if (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* device specific handling of some special types of non-DFX memories */
                    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        case CPSS_PP_FAMILY_DXCH_LION2_E:
                            switch (dbMemType)
                            {
                                /* - there are no interrupts for MLL memories
                                   - no counters for TCAM memories */
                                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E:
                                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E:
                                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E:
                                    /* no counters for TCAM memories */
                                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
                                case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                                    break;
                                default:
                                    break;
                            }
                            break;

                        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E: /* bobcat2, bobk */
                        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                        case CPSS_PP_FAMILY_DXCH_AC3X_E:
                        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                            if (prvIsMemProtectedButNotSupportedSip5(devNum, dbMemType))
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                            }
                            break;

                        default:
                            /* other devices */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
                    }

                    /* device independent handling of rest of non-DFX memory types */
                    switch (dbArrayPtr[i].externalProtectionType)
                    {
                        case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E:
                            /* the memory is not protected at all (neither DFX nor non-DFX). */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

                        case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E:
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                            break;
                        default:
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                            break;
                    }
                    /* non-DFX memories doesn't support counter disable */
                    *countEnablePtr = GT_TRUE;
                    return GT_OK;
                }

                pipeIndex   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                clientIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                memoryIndex = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);

                /* read from Memory Control Register - enable/disable error counter */
                rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                          PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                          5, 2, &regData);
                if(GT_OK != rc)
                {
                    return rc;
                }

                if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
                {
                    switch(regData)
                    {
                        case 0:
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                            *countEnablePtr = GT_FALSE;
                            break;

                        case 1:
                            *countEnablePtr = GT_TRUE;
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                            break;

                        case 2:
                            *countEnablePtr = GT_TRUE;
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                            break;

                        default:
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                            *countEnablePtr = GT_TRUE;
                            break;
                    }
                }
                else if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
                {
                    *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                    *countEnablePtr = BIT2BOOL_MAC((regData >> 1) & 0x1);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                break;
            }
        }
    }
    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityErrorCountEnableGet function
* @endinternal
*
* @brief   Function gets status of error counter. Status getting is performed
*         to per whole hw/logical table or memory. An entry line number -
*         ramRow, logicalTableEntryIndex, hwTableEntryIndex - is ignored.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] countEnablePtr           - (pointer to) status of error counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_VALUE             - if HW tables composing logical table
*                                       have different error enabling status
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssDxChDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT GT_BOOL                                         *countEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityErrorCountEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorTypePtr, countEnablePtr));

    rc = internal_cpssDxChDiagDataIntegrityErrorCountEnableGet(devNum,
                                     memEntryPtr, errorTypePtr, countEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorTypePtr, countEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagDataIntegrityProtectionTypeGet function
* @endinternal
*
* @brief   Function gets memory protection type.
*         In case of "logical table":
*         if some of HW tables composing logical table are not protected
*         and some of HW tables are protected (with the same protection type)
*         - return protection type of protected tables
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] protectionTypePtr        - (pointer to) memory protection type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
* @retval GT_BAD_VALUE             - if logical table consists of HW tables
*                                       with different protection types (parity or
*                                       ECC). So protectionTypePtr can't be
*                                       assigned uniquely.
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityProtectionTypeGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                    *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
{
    GT_U32                                          i=0, j=0;    /* loop iterators */
    GT_STATUS                                       rc;      /* return code    */
    GT_STATUS                                       rc1;     /* return code    */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type    */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum=0; /* size of data integrity DB */
    GT_U32                                          hwTable; /* CPSS_DXCH_TABLE_ENT or
                                                                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    const CPSS_DXCH_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      dbMemType; /* memory type of current DB entry */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_BOOL                                         memoryFound = GT_FALSE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(protectionTypePtr);

    switch (memEntryPtr->type) {
        case CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.portGroupsBmp = memEntryPtr->portGroupsBmp;
            hwLocation.type = CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE;

            for (i = 0; i < memEntryPtr->info.logicalEntryInfo.numOfLogicalTables; i++)
            {
                rc = prvCpssDxChDiagDataIntegrityTableLogicalToHwListGet(
                    devNum,
                    memEntryPtr->info.logicalEntryInfo.logicaTableInfo[i].logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* if some table doesn't support the feature or is absent in
                   the DB skip it. But if ALL tables are skipped return error */
                rc1 = GT_NOT_FOUND;
                for (j = 0; hwTablesPtr[j] != LAST_VALID_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                    rc = internal_cpssDxChDiagDataIntegrityProtectionTypeGet(
                        devNum, &hwLocation, &protectionType);
                    if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                    {
                       /* ignore tables either not found in DB  or not supporting the feature */
                        continue;
                    }
                    else if (rc != GT_OK)
                    {
                        return rc;
                    };

                    if (rc1 != GT_OK)
                    {
                        /* this is first table supporting the feature */
                        rc1 = rc;
                        *protectionTypePtr = protectionType;
                    }
                    else if ((*protectionTypePtr) != protectionType)
                    {
                        /* logical table have HW tables with different protection type */
                        if (CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E
                            == *protectionTypePtr)
                        {
                            *protectionTypePtr = protectionType;
                        }
                        else if (CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E
                                 != protectionType)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                        }

                    }
                }
                /* neither of tables supports the feature */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            }

            /* done */
            return GT_OK;

        case CPSS_DXCH_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            /* Check special cases */
            rc = prvCpssDxChDiagDataIntegrityHwTableGet(devNum, &hwTable, NULL, NULL);
            if(GT_OK != rc)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                /* find first entry of given memType/hwTable */
                if ((dbArrayPtr[i].memType == (GT_U32)memType) || (dbArrayPtr[i].hwTableName == hwTable))
                {
                    dbMemType =dbArrayPtr[i].memType;
                    memoryFound = GT_TRUE;
                    if (prvIsMemProtectedButNotSupportedSip5(devNum, memType))
                    {
                        *protectionTypePtr = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E;
                    }
                    else
                    {
                        *protectionTypePtr = (dbArrayPtr[i].protectionType ==
                            CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                            dbArrayPtr[i].externalProtectionType :
                            dbArrayPtr[i].protectionType);
                    }
                    break;
                }
            }
            if(memoryFound == GT_TRUE)
            {
                break;
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            /* find first entry of given memType/hwTable */
            if ((dbArrayPtr[i].memType == (GT_U32)memType) || (dbArrayPtr[i].hwTableName == hwTable))
            {
                dbMemType =dbArrayPtr[i].memType;

                switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
                {
                    case CPSS_PP_FAMILY_DXCH_LION2_E:
                    switch(dbMemType)
                    {
                        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_TABLE_FIFO_E:
                        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_REPLICATIONS_IN_USE_FIFO_E:
                        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E:
                            /* there are no interrupts for MLL memories */
                            *protectionTypePtr = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E;
                            break;

                        default:
                            *protectionTypePtr = (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                                dbArrayPtr[i].externalProtectionType :
                                dbArrayPtr[i].protectionType);
                    }
                    break;

                    case CPSS_PP_FAMILY_DXCH_AC5_E:
                        if (prvIsMemProtectedButNotSupportedCnmAc5_Ac5x(memType))
                        {
                            *protectionTypePtr = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E;
                        }
                        else
                        {
                            *protectionTypePtr = (dbArrayPtr[i].protectionType ==
                                CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                                dbArrayPtr[i].externalProtectionType :
                                dbArrayPtr[i].protectionType);
                        }
                        break;

                    case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                    case CPSS_PP_FAMILY_DXCH_AC3X_E:
                    case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                        if (prvIsMemProtectedButNotSupportedSip5(devNum, memType))
                        {
                            *protectionTypePtr = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E;
                        }
                        else
                        {
                            *protectionTypePtr = (dbArrayPtr[i].protectionType ==
                                CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                                dbArrayPtr[i].externalProtectionType :
                                dbArrayPtr[i].protectionType);
                        }
                        break;

                    case CPSS_PP_FAMILY_DXCH_AC5X_E:
                        if (prvIsMemProtectedButNotSupportedCnmAc5_Ac5x(memType) ||
                            prvIsMemProtectedButNotSupportedSip6_10(memType))
                        {
                            *protectionTypePtr = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E;
                        }
                        else
                        {
                            *protectionTypePtr = (dbArrayPtr[i].protectionType ==
                                CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                                dbArrayPtr[i].externalProtectionType :
                                dbArrayPtr[i].protectionType);
                        }
                        break;
                    case CPSS_PP_FAMILY_DXCH_AC5P_E:
                    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                        if (prvIsMemProtectedButNotSupportedSip6_10(memType))
                        {
                            *protectionTypePtr = CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E;
                        }
                        else
                        {
                            *protectionTypePtr = (dbArrayPtr[i].protectionType ==
                                CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                                dbArrayPtr[i].externalProtectionType :
                                dbArrayPtr[i].protectionType);
                        }
                        break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
                }
                break;
            }
        }
    }

    if(i == dbArrayEntryNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else
    {
        return GT_OK;
    }
}

/**
* @internal cpssDxChDiagDataIntegrityProtectionTypeGet function
* @endinternal
*
* @brief   Function gets memory protection type.
*         In case of "logical table":
*         if some of HW tables composing logical table are not protected
*         and some of HW tables are protected (with the same protection type)
*         - return protection type of protected tables
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] protectionTypePtr        - (pointer to) memory protection type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
* @retval GT_BAD_VALUE             - if logical table consists of HW tables
*                                       with different protection types (parity or
*                                       ECC). So protectionTypePtr can't be
*                                       assigned uniquely.
*/
GT_STATUS cpssDxChDiagDataIntegrityProtectionTypeGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC                    *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityProtectionTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, protectionTypePtr));

    rc = internal_cpssDxChDiagDataIntegrityProtectionTypeGet(devNum, memEntryPtr, protectionTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, protectionTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

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
)
{
    GT_U32  regAddr; /* register address */
    GT_U32  hwValue; /* value of field */
    GT_U32  portGroupId; /* the port group Id */
    GT_STATUS rc;        /* return code       */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(
        devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    /* 0 - enable, 1- disable */
    hwValue = (enable == GT_FALSE)? 1 : 0;
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.parityDaemonRegs.parityDaemonCtrl;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 0, 1, hwValue);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    return GT_OK;
}

/**
* @internal internal_cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet function
* @endinternal
*
* @brief   Function enables/disables TCAM parity daemon.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
*                                      only Router TCAM and Policy TCAM supported
*                                      (APPLICABLE DEVICES Lion2)
* @param[in] enable                   - GT_TRUE - enable daemon
*                                      GT_FALSE - disable daemon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memType
* @retval GT_BAD_STATE             - on tcamParityCalcEnable is disabled
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 only: API is supported only if tcamParityCalcEnable is enabled by cpssDxChPpPhase1Init
*
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    IN  GT_BOOL                                     enable
)
{
    GT_U32  regAddr; /* register address */
    GT_STATUS rc;    /* return code       */
    GT_PORT_GROUPS_BMP   activeTcamBmp; /* bitmap of Port Groups with Active TCAM. */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E );

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if (enable == GT_FALSE)
        {
            /* disable daemon on all TCAM instances */
            activeTcamBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }
        else
        {
            /* TCAM parity daemon may generate false interrupts when TCAM is in shutdown state.
               enable daemon only on active (powered UP) TCAMs */
            rc = prvCpssDxChTcamActiveBmpGet(devNum, &activeTcamBmp);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = prvCpssDxChDiagDataIntegrityPortGroupTcamParityDaemonEnableSet(
                             devNum, activeTcamBmp, enable);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityDaemonEnable = enable;

        return GT_OK;
    }

    /* validity check */
    if((PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.tcamParityCalcEnable == GT_FALSE) && (enable == GT_TRUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamControl;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.pclTccRegs.policyTcamControl;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, 28, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet function
* @endinternal
*
* @brief   Function enables/disables TCAM parity daemon.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
*                                      only Router TCAM and Policy TCAM supported
*                                      (APPLICABLE DEVICES Lion2)
* @param[in] enable                   - GT_TRUE - enable daemon
*                                      GT_FALSE - disable daemon
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memType
* @retval GT_BAD_STATE             - on tcamParityCalcEnable is disabled
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Lion2 only: API is supported only if tcamParityCalcEnable is enabled by cpssDxChPpPhase1Init
*
*/
GT_STATUS cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    IN  GT_BOOL                                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, enable));

    rc = internal_cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet(devNum, memType, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet function
* @endinternal
*
* @brief   Function gets status of TCAM parity daemon.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
*                                      only Router TCAM and Policy TCAM supported
*                                      (APPLICABLE DEVICES Lion2)
*
* @param[out] enablePtr                - (pointer to) daemon status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static GT_STATUS internal_cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    OUT GT_BOOL                                     *enablePtr
)
{
    GT_STATUS rc; /* return code */
    GT_U32  regAddr; /* register address */
    GT_U32  regData; /* register data */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E );

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.parityDaemonRegs.parityDaemonCtrl;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &regData);
        *enablePtr = (regData == 0) ? GT_TRUE : GT_FALSE;
        return rc;
    }

    switch(memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.ipTccRegs.ipTcamControl;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->pclRegs.pclTccRegs.policyTcamControl;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 28, 1, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return GT_OK;
}

/**
* @internal cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet function
* @endinternal
*
* @brief   Function gets status of TCAM parity daemon.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
*                                      only Router TCAM and Policy TCAM supported
*                                      (APPLICABLE DEVICES Lion2)
*
* @param[out] enablePtr                - (pointer to) daemon status
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    OUT GT_BOOL                                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, enablePtr));

    rc = internal_cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet(devNum, memType, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChDfxPipesBmpGet function
* @endinternal
*
* @brief   Return bitmap of DFX pipes relevant for the device .
*         If bit #N is raised pipeId #N exists in the device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - PP device number
*
* @param[out] pipesBmpPtr              - (pointer to) pipes bitmap
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChDfxPipesBmpGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *pipesBmpPtr
)
{
    GT_U32 pipesBmp;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            /* Harrier: pipes 0..5 */
            pipesBmp = 0x3F;
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            /* AC5X: pipes 0, 1, 2, 3, 4 */
            pipesBmp = 0x1F;
        }
        else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            /* AC5P: pipes 0, 1, 2, 3, 4, 5, 6, 7 */
            pipesBmp = 0xFF;
        }
        else
        {
            /* Falcon: Eagle Tiles 0,2 - pipes 0..4;
                       Eagle Tiles 1,3 - pipes 0..3;
                       Raven 0..2 */
            if (PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMultiInstanceData.dfxInstanceType ==
                 CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
            {
                pipesBmp = (PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMultiInstanceData.dfxInstanceIndex & 1) ? 0xF : 0x1F;
            }
            else
            {
                pipesBmp = 0x7;
            }
        }
    }
    else if (PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        /* Aldrin2: pipes 0..2 */
        pipesBmp = 0x7;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        /* Bobcat3: pipes 0..5 */
        pipesBmp = 0x3f;
    }
    else if (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        /* Caelum: pipes 0, 1, 2, 3 */
        pipesBmp = 0xF;

    }
    else if (PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(devNum) ||
             PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        /* Bobcat2 B0, Cetus: pipes 0, 1, 2 */
        pipesBmp = 7;
    }
    else
    {
        /* Bobcat2 A0, Aldrin, AC3X, xCat3, AC5: pipes 0, 1 */
        pipesBmp = 3;
    }

    *pipesBmpPtr = pipesBmp;
    return GT_OK;
}

/**
* @internal prvCpssDxChDfxPipesBmpGetFalcon function
* @endinternal
*
* @brief   Return bitmap of DFX pipes relevant for the falcon device.
*         If bit #N is raised pipeId #N exists in the device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - Tile index
*
* @param[out] pipesBmpPtr              - (pointer to) pipes bitmap
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChDfxPipesBmpGetFalcon
(
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType,
    IN  GT_U32                                    tileIndex,
    OUT GT_U32                                    *pipesBmpPtr
)
{
    GT_U32 pipesBmp;
    /* Falcon: Eagle Tiles 0,2 - pipes 0..4;
       Eagle Tiles 1,3 - pipes 0..3;
       Raven 0..2 */
    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
    {
        pipesBmp = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_BMP_GET(tileIndex);
    }
    else
    {
        pipesBmp = 0x7;
    }

    *pipesBmpPtr = pipesBmp;
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityMemoryIndexesGet function
* @endinternal
*
* @brief   Function gets all memory location coordinates for given memory type.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - memory type
* @param[in] isPerPortGroup           - filtering flag
*                                      GT_TRUE - only per given portgroup id
*                                      GT_FALSE - all
* @param[in] portGroupId              - portgroup id
*                                      relevant only if isPerPortGroup = GT_TRUE
* @param[in,out] arraySizePtr        - in : size of input array
*                                      out: actual size of array
*
* @param[out] memLocationArr           - array of memory locations
* @param[out] protectionTypePtr        - (pointer to) protection memory type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_SIZE              - if size of input array is not enough to store
*                                       coordinates of all found items
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS prvCpssDxChDiagDataIntegrityMemoryIndexesGet
(
    IN  GT_U8                                                   devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT              memType,
    IN  GT_BOOL                                                 isPerPortGroup,
    IN  GT_U32                                                  portGroupId,
    INOUT GT_U32                                                *arraySizePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC            *memLocationArr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
{
    GT_U32                      i;
    GT_U32                      moduleCount = 0;
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT       dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E);

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        if(isPerPortGroup == GT_TRUE)
        {
            /* per port group request is not supported for multi tile device by the function */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        for (dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; dfxInstanceType <= CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E; dfxInstanceType++)
        {
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(dfxInstanceType, &dbArrayPtr, &dbArrayEntryNum);
            for(i = 0; i < dbArrayEntryNum; i++)
            {
                if(dbArrayPtr[i].memType == (GT_U32)memType)
                {
                    memoryFound = GT_TRUE;
                    if(moduleCount >= *arraySizePtr)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
                    }

                    *protectionTypePtr = dbArrayPtr[i].protectionType;
                    memLocationArr[moduleCount].dfxPipeId   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                    memLocationArr[moduleCount].dfxClientId = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                    memLocationArr[moduleCount].dfxMemoryId = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);
                    if(dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
                    {
                        memLocationArr[moduleCount].dfxInstance.dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
                    }
                    else if(dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
                    {
                        memLocationArr[moduleCount].dfxInstance.dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
                    }
                    else
                    {
                        /* dfx instance type is invalid */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                    }
                    moduleCount++;
                }
            }
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

        for(i = 0; i < dbArrayEntryNum; i++)
        {
            if(dbArrayPtr[i].memType == (GT_U32)memType)
            {
                if(isPerPortGroup == GT_TRUE &&
                    (portGroupId != dbArrayPtr[i].causePortGroupId))
                {
                    continue;
                }

                if( dbArrayPtr[i].causePortGroupId != CPSS_PORT_GROUP_UNAWARE_MODE_CNS &&
                    0 == (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp
                    & (1<<(dbArrayPtr[i].causePortGroupId))))
                {
                    continue;
                }

                memoryFound = GT_TRUE;
                if(moduleCount >= *arraySizePtr)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
                }

                *protectionTypePtr = dbArrayPtr[i].protectionType;
                memLocationArr[moduleCount].dfxPipeId   = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbArrayPtr[i].key);
                memLocationArr[moduleCount].dfxClientId = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbArrayPtr[i].key);
                memLocationArr[moduleCount].dfxMemoryId = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbArrayPtr[i].key);
                moduleCount++;
            }
        }
    }
    *arraySizePtr = moduleCount;

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChDiagDataIntegrityMemoryInstancesCountGet function
* @endinternal
*
* @brief   Function returns number of memory instances in DB.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - memory type
*
* @param[out] counterPtr               - (pointer to) memory counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityMemoryInstancesCountGet
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT GT_U32                                          *counterPtr
)
{
    GT_U32                      i;
    GT_U32                      moduleCount = 0;
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E );

    prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        if(dbArrayPtr[i].memType == (GT_U32)memType)
        {
            memoryFound = GT_TRUE;
            moduleCount++;
        }
    }

    *counterPtr = moduleCount;

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityPortGroupMemoryIndexesGet function
* @endinternal
*
* @brief   Function returns first instance of specific memory type from DB per
*         given port group.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - port group id
* @param[in] memType                  - memory type
*
* @param[out] memLocationPtr           - (pointer to) location
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityPortGroupMemoryIndexesGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          portGroupId,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr
)
{
    GT_STATUS rc; /* return code */
    GT_U32 arraySize;
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationArr;

    rc = prvCpssDxChDiagDataIntegrityMemoryInstancesCountGet(devNum, memType, &arraySize);
    if(GT_OK != rc)
    {
        return rc;
    }

    memLocationArr = (CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC*)cpssOsMalloc(arraySize*sizeof(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC));
    cpssOsMemSet(memLocationArr, 0, arraySize*sizeof(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC));

    rc = prvCpssDxChDiagDataIntegrityMemoryIndexesGet(devNum,
                                                      memType,
                                                      GT_TRUE, portGroupId,
                                                      &arraySize, memLocationArr, &protectionType);
    if(GT_OK != rc)
    {
        cpssOsFree(memLocationArr);
        return rc;
    }

    memLocationPtr->dfxPipeId   = memLocationArr[0].dfxPipeId;
    memLocationPtr->dfxClientId = memLocationArr[0].dfxClientId;
    memLocationPtr->dfxMemoryId = memLocationArr[0].dfxMemoryId;

    cpssOsFree(memLocationArr);

    return GT_OK;
}

extern GT_STATUS prvCpssPolicerIngressRamIndexInfoGet
(
    IN  GT_U8                               devNum,
    OUT  GT_U32                             *numEntriesRam0Ptr,
    OUT  GT_U32                             *numEntriesRam1Ptr,
    OUT  GT_U32                             *numEntriesRam2Ptr,
    OUT  GT_U32                             *iplr0StartRamPtr,
    OUT  GT_U32                             *iplr0NumRamsUsedPtr,/*0..3*/
    OUT  GT_U32                             *iplr1StartRamPtr,
    OUT  GT_U32                             *iplr1NumRamsUsedPtr/*3-iplr0*/
);

#define PLR_0   0
#define PLR_1   1
/**
* @internal plrStartRamToIndexInTableConvert function
* @endinternal
*
* @brief   Function converts from 'IPLR RAM 0/1/2' to 'IPLR_stage + index'
*
* @note   APPLICABLE DEVICES:      AC5; Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] ramIndex                 - 0 or 1 or 2 : 'IPLR RAM 0/1/2'
*
* @param[out] plrIndexPtr              - (pointer to) 0 or 1 : the IPLR_stage_0 or IPLR_stage_1
*                                      using input RAM ram.
* @param[out] indexPtr                 - (pointer to) the offset of the start of the input RAM in
*                                      the IPLR_stage
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS plrStartRamToIndexInTableConvert(
    IN  GT_U8                           devNum,
    IN  GT_U32                          ramIndex,
    OUT GT_U32                          *plrIndexPtr,
    OUT GT_U32                          *indexPtr
)
{
    GT_STATUS   rc;
    GT_U32  numEntriesRam0;
    GT_U32  numEntriesRam1;
    GT_U32  numEntriesRam2;
    GT_U32  iplr0StartRam;
    GT_U32  iplr0NumRamsUsed;
    GT_U32  iplr1StartRam;
    GT_U32  iplr1NumRamsUsed;

    rc = prvCpssPolicerIngressRamIndexInfoGet(devNum,
        &numEntriesRam0,&numEntriesRam1,&numEntriesRam2,
        &iplr0StartRam,&iplr0NumRamsUsed,
        &iplr1StartRam,&iplr1NumRamsUsed);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(ramIndex == 0)
    {
        /* hold by RAM 0 */
        *indexPtr = 0;

        if(iplr0NumRamsUsed && iplr0StartRam == 0)
        {
            *plrIndexPtr = PLR_0;
            return GT_OK;
        }
        else
        {
            *plrIndexPtr = PLR_1;
            return GT_OK;
        }
    }
    else
    if(ramIndex == 1)
    {
        /* hold by RAM 1 */
        if(iplr0NumRamsUsed > 1 && iplr0StartRam == 0)
        {
            *plrIndexPtr = PLR_0;
            *indexPtr = 0 + numEntriesRam0;
            return GT_OK;
        }
        else
        if(iplr0NumRamsUsed && iplr0StartRam == 1)
        {
            *plrIndexPtr = PLR_0;
            *indexPtr = 0;
            return GT_OK;
        }
        if(iplr1NumRamsUsed > 1 && iplr1StartRam == 0)
        {
            *plrIndexPtr = PLR_1;
            *indexPtr = 0 + numEntriesRam0;
            return GT_OK;
        }
        else/*(iplr1NumRamsUsed && iplr1StartRam == 1)*/
        {
            *plrIndexPtr = PLR_1;
            *indexPtr = 0;
            return GT_OK;
        }
    }
    else
    {
        /* hold by RAM 2 */
        if(iplr0NumRamsUsed && iplr0StartRam == 2)
        {
            *plrIndexPtr = PLR_0;
            *indexPtr = 0;
            return GT_OK;
        }
        else
        if(iplr0NumRamsUsed > 1 && iplr0StartRam == 1)
        {
            *plrIndexPtr = PLR_0;
            *indexPtr = numEntriesRam1;
            return GT_OK;
        }
        else
        if(iplr0NumRamsUsed > 2 && iplr0StartRam == 0)
        {
            *plrIndexPtr = PLR_0;
            *indexPtr = numEntriesRam0 + numEntriesRam1;
            return GT_OK;
        }
        else
        if(iplr1NumRamsUsed && iplr1StartRam == 2)
        {
            *plrIndexPtr = PLR_1;
            *indexPtr = 0;
            return GT_OK;
        }
        else
        if(iplr1NumRamsUsed > 1 && iplr1StartRam == 1)
        {
            *plrIndexPtr = PLR_1;
            *indexPtr = numEntriesRam1;
            return GT_OK;
        }
        else/*(iplr1NumRamsUsed > 2 && iplr1StartRam == 0)*/
        {
            *plrIndexPtr = PLR_1;
            *indexPtr = numEntriesRam0 + numEntriesRam1;
            return GT_OK;
        }
    }
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxErrorConvert function
* @endinternal
*
* @brief   Function converts from 'Specific RAM + failedRow(index)' to 'HW_table + index'
*         HW index means a value passed as IN-parameter "entryIndex" in API
*         prvCpssDxChReadTableEntry/prvCpssDxChWriteTableEntry.
*         I.e. it can differ from actual hardware geometry. See using of
*         VERTICAL_INDICATION_CNS, FRACTION_INDICATION_CNS, FRACTION_HALF_TABLE_INDICATION_CNS
*         for details
*
* @note   APPLICABLE DEVICES:      Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3.
*
* @param[in] devNum                   - PP device number
* @param[in] memLocationPtr           - (pointer to) DFX memory info
* @param[in] failedRow                - the failed row in the DFX memory.
*
* @param[out] hwErrorInfoPtr           - (pointer to) the converted 'HW_table + index'
* @param[out] portGroupIdPtr           - (pointer to) port group id
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the DFX location was not found in DB.
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChDiagDataIntegrityDfxErrorConvert
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT GT_U32                                          *portGroupIdPtr,
    OUT CPSS_DXCH_HW_INDEX_INFO_STC                     *hwErrorInfoPtr
)
{
    GT_STATUS   rc;
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */
    CPSS_DXCH_TABLE_ENT  hwTableType;   /* HW table type */
    GT_U32  tmpIndex;                   /* temp variable */
    GT_U32  hwTableEntryIndex;          /* HW table entry index */
    GT_U32  ramHwTableOffset;   /* first HW entry index for first line of RAM */
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT lpmMode; /* LPM mode (bobcat3 only) */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */
    GT_U32  tile = 0; /* index of tile, used for multi tile devices like Falcon */
    GT_U32  portGroup; /* port group */
    PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT baseTableId; /* base of Ids range */
    GT_U32  shmBlockIndex = 0;     /* index of block of SIP6 shared memory */
    GT_U32  clientOffset = 0;  /* offset from the base client mapped to block of SIP6 shared memory */
    GT_U32  logicalBlockSize;  /* logical table block size */
    GT_U32  numOfBanks;        /* number of memory banks */
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr; /* module configuration */
    GT_U32  blockStartLpmEntry;  /* start entry in small LPM block */
    GT_U32  smbBlockPort;  /* shared block connection port */
    GT_U32  rowInSharedBlock;  /* row in shared block */
    GT_U32  totalNumOfLinesInBlockIncludingGap; /* line indexes between 2 LPM blocks starts */
    GT_U32  chipletIdx; /* index of chiplet */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(memLocationPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwErrorInfoPtr);

    if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
    {
        /* DB storing info related to this feature is actual for bobcat2 b0, not a0 */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        if(memLocationPtr->dfxInstance.dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
        {
            dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            tile = memLocationPtr->dfxInstance.dfxInstanceIndex;
        }
        else if(memLocationPtr->dfxInstance.dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
        {
            dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E;
            tile = memLocationPtr->dfxInstance.dfxInstanceIndex / 4;
        }
        else
        {
            /* dfx instance type is invalid */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        /* Get entry index for DFX memories */
        rc = searchMemTypeFalcon(memLocationPtr->dfxPipeId,
            memLocationPtr->dfxClientId,
            memLocationPtr->dfxMemoryId,
            dfxInstanceType,
            &dbItemPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "The dfx memory was not found");
        }
        if(dbItemPtr->hwTableName == CPSS_DXCH_TABLE_LAST_E)
        {
            /*the memory hold no CPSS accessing support*/
            return /* this is not ERROR for the LOG !!! */ GT_EMPTY;
        }
    }
    else
    {
        /* Get entry index for DFX memories */
        rc = searchMemType(devNum,
            memLocationPtr->dfxPipeId,
            memLocationPtr->dfxClientId,
            memLocationPtr->dfxMemoryId,
            &dbItemPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "The dfx memory was not found");
        }
        if(dbItemPtr->hwTableName == CPSS_DXCH_TABLE_LAST_E)
        {
            /*the memory hold no CPSS accessing support*/
            return /* this is not ERROR for the LOG !!! */ GT_EMPTY;
        }
    }

    if ((dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_ECC_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_PARITY_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_PACKET_DATA_ECC_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_PACKET_DATA_PARITY_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_PHA_RAM_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_CM3_RAM_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_INTERNAL_TABLE_MANAGEMENT_E) ||
        (dbItemPtr->hwTableName == CPSS_DXCH_SIP6_TABLE_MTIP_MAC_STAT_E))
    {
        /*CPSS does not manage these RAMs and HW index is not used  */
        hwErrorInfoPtr->hwTableType = dbItemPtr->hwTableName;
        hwErrorInfoPtr->hwTableEntryIndex = 0;
        return GT_OK;
    }

    cpssOsMemSet(hwErrorInfoPtr,0,sizeof(*hwErrorInfoPtr));

    ramHwTableOffset = 0;
    moduleCfgPtr = NULL;
    blockStartLpmEntry         = 0;
    rowInSharedBlock           = 0;
    smbBlockPort               = 0;
    logicalBlockSize           = dbItemPtr->logicalBlockSize;

    if(dbItemPtr->hwTableName < CPSS_DXCH_TABLE_LAST_E)
    {
        hwTableType = dbItemPtr->hwTableName;
        if ((hwTableType == CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E) ||
            (hwTableType == CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E))
        {
            chipletIdx = PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) ? memLocationPtr->dfxInstance.dfxInstanceIndex : 0;

            /* there are up to 32 instances of such tables. */
            hwTableType += dbItemPtr->causePortGroupId + 2 * chipletIdx;
        }
    }
    else
    {
        baseTableId = specialTableIdToBase(dbItemPtr->hwTableName, GT_TRUE);
        switch(baseTableId)
        {
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_IPLR_METERING_E:
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_IPLR_COUNTING_E:
                rc = plrMemTypeFirstIndexInHwTableGet(devNum, dbItemPtr->memType,
                                                      &hwTableType, &ramHwTableOffset);
                if(GT_OK != rc)
                {
                    return rc;
                }
                break;

            /*case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_GOP_PR_MIB_COUNTERS_E: -- PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PR.PRMIB.MIBCountersBase
                break;*/
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ERMRK_QOS_MAP_E:
                /*PRV_CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E or
                  PRV_CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E*/
                if(failedRow < 192) /*0..191*/
                {
                    hwTableType = CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E;
                }
                else   /* >= 192 */
                {
                    hwTableType = CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E;
                    failedRow -= 192;
                }
                break;
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP_E:
                if(failedRow < 128) /*0..128*/
                {
                    hwTableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E;
                }
                else   /* >= 128 */
                {
                    hwTableType = CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E;
                    failedRow -= 128;
                }
                break;
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E:
                /* not related to SIP6 devices or SIP6 without shared memory */
                /* treated after the 'hwTableEntryIndex' calculations */
                hwTableType = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
                break;
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E:
                shmBlockIndex = dbItemPtr->hwTableName - baseTableId;
                smbBlockPort  = 0;
                rc = prvCpssDxChCfgSharedTablesType2BlockTableIdGet(
                    devNum, shmBlockIndex, smbBlockPort, &hwTableType, &clientOffset);
                if (GT_OK != rc)
                {
                    return rc;
                }

                if ((hwTableType == CPSS_DXCH_SIP5_TABLE_LPM_MEM_E) ||
                    (hwTableType == CPSS_DXCH_SIP6_TABLE_PBR_E))
                {
                    /* half block mode */
                    if (PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lpmBankSize
                        == (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock / 2))
                    {
                        /* correct both for SHM_TYPE2_SBM_DATA and SHM_TYPE2_SBM_XOR */
                        /* typem  size, firstTableLine port */
                        /* DATA , 2048, 0               1   */
                        /* DATA , 2048, 2048            1   */
                        /* DATA , 2048, 4096            1   */
                        /* DATA , 2048, 6144            0   */
                        /* DATA , 2048, 8192            0   */
                        /* DATA , 2048, 10240           0   */
                        /* XOR  , 1024, 12288           1   */
                        /* XOR  , 1024, 13312           0   */
                        if ((dbItemPtr->firstTableLine <= (PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? 2048 : 4096)) ||
                            (dbItemPtr->firstTableLine == (PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? 8192 : 12288)))
                        {
                            /* odd */
                            smbBlockPort = 1;
                        }
                        else
                        {
                            /*even*/
                            smbBlockPort = 0;
                        }

                        if (smbBlockPort != 0)
                        {
                            rc = prvCpssDxChCfgSharedTablesType2BlockTableIdGet(
                                devNum, shmBlockIndex, smbBlockPort, &hwTableType, &clientOffset);
                            if (GT_OK != rc)
                            {
                                return rc;
                            }
                        }
                    }
                }
                if ((hwTableType == CPSS_DXCH_TABLE_FDB_E) ||
                         (hwTableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E))
                {
                    logicalBlockSize = _8K;
                }
                else if (hwTableType == CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E)
                {
                    /* ARP/TS memory */
                    logicalBlockSize = PRV_CPSS_DXCH_AC5_CHECK_MAC(devNum) ?
                        _1K : PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? _8K : 12 * _1K;
                }
                break;
            case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE1_SBM_0_E:

                moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);

                /* each block lines accessed with indexes in totalNumOfLinesInBlockIncludingGap */
                /* based range regardless of real size of the block                             */
                totalNumOfLinesInBlockIncludingGap =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;

                /* TYPE1 RAMs are hardwared to upper LPM banks 29..10, TYPE1 bank0 is LPM bank 29
                   and so on. */
                blockStartLpmEntry =
                    (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared - 1
                     - (dbItemPtr->hwTableName - baseTableId))
                       * totalNumOfLinesInBlockIncludingGap;

                if ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr
                        <= blockStartLpmEntry)
                    && ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr
                            + moduleCfgPtr->ip.maxNumOfPbrEntries)
                        > blockStartLpmEntry))
                {
                    hwTableType = CPSS_DXCH_SIP6_TABLE_PBR_E;
                }
                else
                {
                    hwTableType = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
                }
                break;


            default:/* should not happen */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        }
    }

    /*calculate the entry index in the HW table*/
    /*
     *  tmpIndex = firstTableLine + failedRow
     *
     *  hwTableEntryIndex = startAddress +
     *      (tmpIndex / logicalBlockSize) * logicalBlockAddressSteps +
     *      (tmpIndex % logicalBlockSize) * addressIncrement
    */
    tmpIndex = dbItemPtr->firstTableLine + failedRow;

    hwTableEntryIndex =
        dbItemPtr->startAddress +
        (tmpIndex / logicalBlockSize) * dbItemPtr->logicalBlockAddressSteps +
        (tmpIndex % logicalBlockSize) * dbItemPtr->addressIncrement;

    baseTableId = specialTableIdToBase(dbItemPtr->hwTableName, GT_TRUE);
    switch(baseTableId)
    {
        case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E:
            /* not related to sip6 devices or SIP6 without shared memory */
            tmpIndex = dbItemPtr->hwTableName - PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_LPM_0_E;
            /* add the relative memory in the 'LPM table' */
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
                totalNumOfLinesInBlockIncludingGap =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
                hwTableEntryIndex = tmpIndex * totalNumOfLinesInBlockIncludingGap + failedRow;
                if ((hwTableEntryIndex >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr) &&
                    (hwTableEntryIndex <= (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr
                                           + moduleCfgPtr->ip.maxNumOfPbrEntries)))
                {
                    hwTableType = CPSS_DXCH_SIP6_TABLE_PBR_E;
                    hwTableEntryIndex *= PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS;
                }
            }
            else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                rc = cpssDxChLpmMemoryModeGet(devNum, &lpmMode);
                if(GT_OK != rc)
                {
                    return rc;
                }
                if (lpmMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
                {
                    /* even 8K blocks belongs to CPSS_DXCH_SIP5_TABLE_LPM_MEM_E
                     * odd  8K blocks belongs to CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E */
                    if (tmpIndex % 2)
                    {
                        hwTableType = CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E;
                    }
                    tmpIndex /= 2;
                }

                hwTableEntryIndex += (tmpIndex) * (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap / 2);
                /* In Aldrin2 numOfLinesInBlock is 8k and
                 * totalNumOfLinesInBlockIncludingGap is 16k so for every odd
                 * index LPM table ie 1,3,5.. (each RAM bank has 8k acceptable
                 * RAM lines comprising of 2 logical LPM table) the
                 * hwTableEntryIndex exceeds the allowed limit.
                 */
                if((dbItemPtr->logicalBlockSize != (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap / 2)) && tmpIndex % 2)
                {
                    hwTableEntryIndex -= (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap / 2   -  dbItemPtr->logicalBlockSize);
                }
            }
            else
            {
                hwTableEntryIndex += (tmpIndex * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap);
            }
            break;

        case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE2_SBM_0_E:

            if ((hwTableType == CPSS_DXCH_TABLE_FDB_E) ||
                (hwTableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E))
            {
                numOfBanks = (hwTableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E) ?
                                    PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks:
                                    (GT_U32)(1 << PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank);

                hwTableEntryIndex = (hwTableEntryIndex * numOfBanks) + clientOffset;
            }
            else if (hwTableType == CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E)
            {
                /* ARP/TS - entry crosses 4 blocks, each line in SBM holds 2 ARP entries */
                hwTableEntryIndex += (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm / 2) * (clientOffset / 4);
            }
            else if ((hwTableType == CPSS_DXCH_SIP5_TABLE_LPM_MEM_E) ||
                    (hwTableType == CPSS_DXCH_SIP6_TABLE_PBR_E))
            {
                /* LPM and PBR */
                totalNumOfLinesInBlockIncludingGap =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;

                rowInSharedBlock = failedRow + dbItemPtr->firstTableLine;
                /* half block mode */
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lpmBankSize
                    == (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock / 2))
                {
                    /* correct both for SHM_TYPE2_SBM_DATA and SHM_TYPE2_SBM_XOR */
                    /* typem  size, firstTableLine  port  new_base  */
                    /* DATA , 2048, 0                1    0         */
                    /* DATA , 2048, 2048             1    2048      */
                    /* DATA , 2048, 4096             1    4096      */
                    /* DATA , 2048, 6144             0    0         */
                    /* DATA , 2048, 8192             0    2048      */
                    /* DATA , 2048, 10240            0    4096      */
                    /* XOR  , 1024, 12288            1    6144      */
                    /* XOR  , 1024, 13312            0    6144      */
                    GT_U32 firstTableLine = dbItemPtr->firstTableLine;

                    if (firstTableLine >= (PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? 8192 : 12288))
                    {
                        /* XOR RAMs */
                        firstTableLine = PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? 4096 : 6144;
                    }
                    else if (firstTableLine >= (PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? 4096 : 6144))
                    {
                        firstTableLine -= (PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum) ? 4096 : 6144);
                    }

                    rowInSharedBlock = failedRow + firstTableLine;
                }

                blockStartLpmEntry = (clientOffset * totalNumOfLinesInBlockIncludingGap);
                hwTableEntryIndex = blockStartLpmEntry + rowInSharedBlock;
                if (hwTableType == CPSS_DXCH_SIP6_TABLE_PBR_E)
                {
                    hwTableEntryIndex *= PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS;
                }
            }
            else
            {
                /* should never occur */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Unknown HW Table Id");
            }
            break;
        case PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_SHM_TYPE1_SBM_0_E:
            /* LPM and PBR */
            /* PBR table is a part of LPM table, the same indexes used for access */
            /* Line indexes relevant for PBR-lines-range invalid for LPM table    */
            /* Line indexes relevant for LPM-lines-range invalid for PBR table    */
            /* LPM table has 1 entry per line, PBR table has 5 entries per line   */
            hwTableEntryIndex = blockStartLpmEntry + failedRow;
            if (hwTableType == CPSS_DXCH_SIP6_TABLE_PBR_E)
            {
                hwTableEntryIndex *= PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS;
            }
            break;

        default:
            hwTableEntryIndex += ramHwTableOffset;
            rc = prvMakeHwTableSuitableForApi(devNum, &hwTableType, &hwTableEntryIndex, dbItemPtr->firstTableDataBit);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;
    }

    if(PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (dbItemPtr->causePortGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        {
            /* all per tile units use local port group 0 for CPSS management */
            portGroup = 0;
        }
        else if (tile & 1)
        {
            /* tiles 1,3 has port group mirroring */
            portGroup = (dbItemPtr->causePortGroupId == 0) ? 1 : 0;
        }
        else
        {
            portGroup = dbItemPtr->causePortGroupId;
        }

        *portGroupIdPtr = portGroup + (2 * tile);
    }
    else
    {
        *portGroupIdPtr = dbItemPtr->causePortGroupId;
    }

    hwErrorInfoPtr->hwTableType = hwTableType;
    hwErrorInfoPtr->hwTableEntryIndex = hwTableEntryIndex;

    return GT_OK;
}

/**
* @internal plrMemTypeFirstIndexInHwTableGet function
* @endinternal
*
* @brief   Function converts Ingress Policer DFX memory into HW table + index
*         of HW table item corresponding to the beginning of the memory.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - DFX memory type
*
* @param[out] hwTablePtr               - (pointer to) HW table
* @param[out] hwIndexPtr               - (pointer to) HW table item index corresponding to the
*                                      beginning of the memType.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or memType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS plrMemTypeFirstIndexInHwTableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType,
    OUT CPSS_DXCH_TABLE_ENT                        *hwTablePtr,
    OUT GT_U32                                     *hwIndexPtr
)
{
    GT_U32              itemsCount;
    GT_U32              i;
    GT_STATUS           rc;
    CPSS_DXCH_TABLE_ENT hwTable;
    GT_U32              hwIndex;
    GT_U32              iplr0StartRam;
    GT_U32              iplr0NumRamsUsed;
    GT_U32              iplr1StartRam;
    GT_U32              iplr1NumRamsUsed;
    GT_U32              ramIx;
    struct {
        GT_U32 entriesNum;
        GT_U32 stage;            /* IPLR stage: 0,1 */
        GT_U32 startIx;          /* start index of RAM in IPLR items range*/
        GT_U32 startIxInStage; /* start index of RAM in IPLR items range
                                * relative to IPLR stage(0,1) it belongs to. */
    } ramInfoArr[3]; /* IPLR RAMs info */


    cpssOsMemSet(ramInfoArr, 0, sizeof(ramInfoArr));
    rc = prvCpssPolicerIngressRamIndexInfoGet(devNum,
                                              &ramInfoArr[0].entriesNum,
                                              &ramInfoArr[1].entriesNum,
                                              &ramInfoArr[2].entriesNum,
                                              &iplr0StartRam, &iplr0NumRamsUsed,
                                              &iplr1StartRam, &iplr1NumRamsUsed);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check ram number is in range 0..2 */
    if (iplr0StartRam + iplr0NumRamsUsed -1 > 2 || iplr1StartRam + iplr1NumRamsUsed -1 > 2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fill startIx in ramInfoArr for all RAMs */
    for (itemsCount = 0, i = 0; i < 3; i++)
    {
        ramInfoArr[i].startIx = itemsCount;
        itemsCount += ramInfoArr[i].entriesNum;
    }

    /* fill stage, startIxInStage in ramInfoArr for Policer 0 RAMs */
    for (itemsCount = 0, i = iplr0StartRam; i < iplr0StartRam + iplr0NumRamsUsed; i++)
    {
        ramInfoArr[i].stage = 0;
        ramInfoArr[i].startIxInStage = itemsCount;
        itemsCount += ramInfoArr[i].entriesNum;
    }
    /* fill stage, startIxInStage in ramInfoArr for Policer 1 RAMs */
    for (itemsCount = 0, i = iplr1StartRam; i < iplr1StartRam + iplr1NumRamsUsed; i++)
    {
        ramInfoArr[i].stage = 1;
        ramInfoArr[i].startIxInStage = itemsCount;
        itemsCount += ramInfoArr[i].entriesNum;
    }

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR2_E:
            ramIx =
                (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR0_E ? 0 :
                 memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_METERING_INGRESS_PLR1_E ? 1 :
                 2);

            if (PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum) || PRV_CPSS_DXCH_AC5_CHECK_MAC(devNum)) /* bobcat2, not bobk! */
            {
                hwTable = (ramInfoArr[ramIx].stage == 0) ?
                    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E :
                    CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E;
                hwIndex = ramInfoArr[ramIx].startIxInStage;
            }
            else
            {
                /* sip5.15 and above (bobk, aldrin etc) */
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                hwTable = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E;
                hwIndex = ramInfoArr[ramIx].startIx;
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR2_E:
            ramIx =
                (memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR0_E ? 0 :
                 memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_PLR_BILLING_INGRESS_PLR1_E ? 1 :
                 2);

            hwTable = (ramInfoArr[ramIx].stage == 0 ?
                       CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E :
                       CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E);
            hwIndex = ramInfoArr[ramIx].startIxInStage;
            break;

        default:
            /* wrong memType */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *hwIndexPtr = hwIndex;
    *hwTablePtr = hwTable;

    return GT_OK;
}


/**
* @internal plrHwTableIndexToLogicalTableIndex function
* @endinternal
*
* @brief   Function converts Policer Metering/Counting HW table+index into
*         the appropriate index inside the specified logical table.
*         if logical table is not connected with HW table+index return
*         GT_BAD_VALUE
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number
* @param[in] hwTable                  - HW table type
* @param[in] hwIndex                  - HW table item index
* @param[in] logicalTable             - (pointer to) logical table
*
* @param[out] logicalTable             - (pointer to) logical table
* @param[out] logicalIndexPtr          - (pointer to) item index in the logical table
*                                      beginning of the memType.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or if hwTable is not
*                                       Ingress Policer table
* @retval GT_BAD_VALUE             - if logicalTable is not connected with
*                                       hwTable+hwIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS plrHwTableIndexToLogicalTableIndex
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_TABLE_ENT         hwTable,
    IN  GT_U32                      hwIndex,
    IN  CPSS_DXCH_LOGICAL_TABLE_ENT logicalTable,
    OUT GT_U32                      *logicalIndexPtr
)
{
    GT_U32  p0Size = (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && (hwTable == CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E)) ? IPLR0_COUNTING_SIZE_MAC(devNum) :
                                                                                                                                 IPLR0_SIZE_MAC(devNum);
    GT_U32  p1Size = (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && (hwTable == CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E)) ? IPLR1_COUNTING_SIZE_MAC(devNum) :
                                                                                                                                 IPLR1_SIZE_MAC(devNum);
    CPSS_DXCH_LOGICAL_TABLE_ENT ltable = CPSS_DXCH_LOGICAL_TABLE_LAST_E;
    GT_U32 lindex = 0;


    lindex = hwIndex;
    switch (hwTable)
    {
        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E:
        case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E;
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) && hwIndex >= p0Size + p1Size)
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                ltable = CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E;
                lindex -= p0Size+p1Size;

            }
            else if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum) && hwIndex >= p0Size)
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E;
                lindex -= p0Size;
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E;
            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E;
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                lindex -= p0Size;
            }
            else if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E;
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E;
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                lindex -= p0Size+p1Size;
            }
            else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E;
            break;


        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_COUNTING_E;
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                if (hwIndex >= p0Size + p1Size)
                {
                    ltable = CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E;
                    lindex -= p0Size + p1Size;
                }
                else if (hwIndex >= p0Size)
                {
                    ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E;
                    lindex -= p0Size;
                }
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E;
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E;
            if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E;
            break;

        case CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E:
            ltable = CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (ltable != logicalTable)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }
    *logicalIndexPtr = lindex;
    return GT_OK;
}


/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat3 function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobcat3
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_BOBCAT3_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_BOBCAT3_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_BOBCAT3_BMA_SUM_MC_CNT_PARITY_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            /* read for BM core 0 only. cores 0..5 are expected to store the same values */
            intIndex = PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            intIndex      = PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
            intIndex2     = PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            /* there are DQ0..DQ5. Read DQ0, rest are expected to be the same */
            intIndex = PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_BOBCAT3_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            break;
    }

    if (intIndex == PRV_CPSS_BOBCAT3_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_BOBCAT3_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat3 function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobcat3
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */
    GT_U32      upperIntIndex;  /* upper HW interrupt index (for the loop) */
    GT_U32      i;

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex  = PRV_CPSS_BOBCAT3_BMA_SUM_MC_CNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            intIndex      = PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            upperIntIndex = PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;

            while (intIndex <= upperIntIndex && rc == GT_OK)
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex++, intMaskEn);
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,
                             PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            /* iterate through DQ0..DQ5. */
            i = 0;
            intIndex = PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            do
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                switch (++i) {
                    case 1:
                        intIndex = PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    case 2:
                        intIndex = PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    case 3:
                        intIndex = PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    case 4:
                        intIndex = PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    case 5:
                        intIndex = PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    default:
                        /* stop cycle */
                        intIndex = PRV_CPSS_BOBCAT3_LAST_INT_E;
                        break;
                }
            }
            while (rc == GT_OK && intIndex != PRV_CPSS_BOBCAT3_LAST_INT_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_BOBCAT3_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin2 function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAldrin2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */
    GT_U32      upperIntIndex;  /* upper HW interrupt index (for the loop) */
    GT_U32      i;

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex  = PRV_CPSS_ALDRIN2_BMA_SUM_MC_CNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            intIndex      = PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            upperIntIndex = PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E;

            while (intIndex <= upperIntIndex && rc == GT_OK)
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex++, intMaskEn);
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,
                             PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            /* iterate through DQ0..DQ3. */
            i = 0;
            intIndex = PRV_CPSS_ALDRIN2_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            do
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                switch (++i) {
                    case 1:
                        intIndex = PRV_CPSS_ALDRIN2_TXQ_DQ1_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    case 2:
                        intIndex = PRV_CPSS_ALDRIN2_TXQ_DQ2_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    case 3:
                        intIndex = PRV_CPSS_ALDRIN2_TXQ_DQ3_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
                        break;
                    default:
                        /* stop cycle */
                        intIndex = PRV_CPSS_ALDRIN2_LAST_INT_E;
                        break;
                }
            }
            while (rc == GT_OK && intIndex != PRV_CPSS_ALDRIN2_LAST_INT_E);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_ALDRIN2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin2 function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAldrin2
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_ALDRIN2_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_ALDRIN2_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_ALDRIN2_BMA_SUM_MC_CNT_PARITY_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            /* read for BM core 0 only. cores 0..3 are expected to store the same values */
            intIndex = PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            intIndex      = PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
            intIndex2     = PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            /* there are DQ0..DQ3. Read DQ0, rest are expected to be the same */
            intIndex = PRV_CPSS_ALDRIN2_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_ALDRIN2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            break;
    }

    if (intIndex == PRV_CPSS_ALDRIN2_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_ALDRIN2_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskFalcon function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskFalcon
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt indexes */
    GT_U32      intIndex2; /* HW interrupt indexes */
    GT_U32      i=0; /* interrupt iterator */
    GT_U32      numOfTiles; /* number of tiles */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;
    numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            /* iterate through per tile and per pipe interrupts */
            intIndex = PRV_CPSS_FALCON_TILE_0_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E;
            do
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
                switch(++i) {
                    case 1:
                        intIndex = PRV_CPSS_FALCON_TILE_0_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    case 2:
                        intIndex = PRV_CPSS_FALCON_TILE_1_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    case 3:
                        intIndex = PRV_CPSS_FALCON_TILE_1_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    case 4:
                        intIndex = PRV_CPSS_FALCON_TILE_2_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    case 5:
                        intIndex = PRV_CPSS_FALCON_TILE_2_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    case 6:
                        intIndex = PRV_CPSS_FALCON_TILE_3_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    case 7:
                        intIndex = PRV_CPSS_FALCON_TILE_3_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E;
                        break;
                    default:
                        /* stop cycle */
                        intIndex = PRV_CPSS_FALCON_LAST_INT_E;
                        break;
                }
            }
            while (rc == GT_OK && intIndex != PRV_CPSS_FALCON_LAST_INT_E && i < (2*numOfTiles) /* per tile per pipe interrupts */);
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            /* iterate through per tile and per pipe interrupts */
            intIndex = PRV_CPSS_FALCON_TILE_0_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_FALCON_TILE_0_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E;
            do
            {
                rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask(devNum, errorType, intMaskEn,
                                                                   intIndex, intIndex2);
                switch(++i) {
                    case 1:
                        intIndex = PRV_CPSS_FALCON_TILE_0_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_0_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    case 2:
                        intIndex = PRV_CPSS_FALCON_TILE_1_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_1_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    case 3:
                        intIndex = PRV_CPSS_FALCON_TILE_1_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_1_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    case 4:
                        intIndex = PRV_CPSS_FALCON_TILE_2_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_2_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    case 5:
                        intIndex = PRV_CPSS_FALCON_TILE_2_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_2_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    case 6:
                        intIndex = PRV_CPSS_FALCON_TILE_3_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_3_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    case 7:
                        intIndex = PRV_CPSS_FALCON_TILE_3_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E;
                        intIndex2 = PRV_CPSS_FALCON_TILE_3_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E;
                        break;
                    default:
                        /* stop cycle */
                        intIndex = PRV_CPSS_FALCON_LAST_INT_E;
                        break;
                }
            }
            while (rc == GT_OK && intIndex != PRV_CPSS_FALCON_LAST_INT_E && i < (2*numOfTiles) /* per tile per pipe interrupts */);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_GRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CLAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CMYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLQ2AMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLGRNDPRNT_E:

            /* not implemented */
            rc = GT_NOT_SUPPORTED;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5x function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5x
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt indexes */
    GT_U32      intIndex2; /* HW interrupt indexes */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_AC5X_BMA_MC_CNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E;
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask(devNum, errorType, intMaskEn,
                                                               intIndex, intIndex2);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PFUNC_E:

            /* not implemented */
            rc = GT_NOT_SUPPORTED;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5p function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskAc5p
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt indexes */
    GT_U32      intIndex2; /* HW interrupt indexes */
    GT_U32      ii; /* iterator */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E:
            {
                GT_U32 singleEccIntArr[] = { /* list of SERDES SRAM ECC interrupts */
                    PRV_CPSS_AC5P_SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_5_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_6_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_AC5P_SDW_INST_7_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E};

                for (ii=0; ii < sizeof(singleEccIntArr)/sizeof(singleEccIntArr[0]);ii++)
                {
                    intIndex = singleEccIntArr[ii];
                    /* double ECC is right after single ECC interrupt */
                    intIndex2 = intIndex + 1;
                    rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask(devNum, errorType, intMaskEn,
                                                                       intIndex, intIndex2);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }

            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_AC5P_BMA_MC_CNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E;
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask(devNum, errorType, intMaskEn,
                                                               intIndex, intIndex2);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_CLS0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_GRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CLAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CMYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PTBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLQ2AMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLGRNDPRNT_E:
            /* not implemented */
            rc = GT_NOT_SUPPORTED;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskHarrier function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; AC5P ; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskHarrier
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt indexes */
    GT_U32      intIndex2; /* HW interrupt indexes */
    GT_U32      ii; /* iterator */

    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E:
            {
                GT_U32 singleEccIntArr[] = { /* list of SERDES SRAM ECC interrupts */
                    PRV_CPSS_HARRIER_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_HARRIER_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_HARRIER_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_HARRIER_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E,
                    PRV_CPSS_HARRIER_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E};

                for (ii=0; ii < sizeof(singleEccIntArr)/sizeof(singleEccIntArr[0]);ii++)
                {
                    intIndex = singleEccIntArr[ii];
                    /* double ECC is right after single ECC interrupt */
                    intIndex2 = intIndex + 1;
                    rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask(devNum, errorType, intMaskEn,
                                                                       intIndex, intIndex2);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }

            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E;
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask(devNum, errorType, intMaskEn,
                                                               intIndex, intIndex2);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PFUNC_E:
            /* not implemented */
            rc = GT_NOT_SUPPORTED;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsMaskStateGet function
* @endinternal
*
* @brief   Function fills mask state for non-DFX DataIntegrity event.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    - PP device number
* @param[in] intIndex                  - HW interrupt index for parity/single ECC error
* @param[in] intIndex2                 - HW interrupt index for double ECC error
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsMaskStateGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          intIndex,
    IN  GT_U32                                          intIndex2,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    if (intIndex == PRV_CPSS_LAST_INT_INDEX_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_LAST_INT_INDEX_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetFalcon function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetFalcon
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      intIndex  = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for  <double ECC> error */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            /* there are per tile and per pipe, reading one and expecting
             * others to be same.
             */
            intIndex = PRV_CPSS_FALCON_TILE_0_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_FALCON_TILE_0_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_FALCON_TILE_0_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChDiagDataIntegrityNonDfxEventsMaskStateGet(devNum, intIndex, intIndex2, errorTypePtr, operationPtr);
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5x function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5x
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      intIndex  = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for  <double ECC> error */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_AC5X_BMA_MC_CNT_PARITY_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChDiagDataIntegrityNonDfxEventsMaskStateGet(devNum, intIndex, intIndex2, errorTypePtr, operationPtr);
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5p function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetAc5p
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      intIndex  = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for  <double ECC> error */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_AC5P_BMA_MC_CNT_PARITY_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E:
            intIndex = PRV_CPSS_AC5P_SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E;
            intIndex2 = PRV_CPSS_AC5P_SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChDiagDataIntegrityNonDfxEventsMaskStateGet(devNum, intIndex, intIndex2, errorTypePtr, operationPtr);
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetHarrier function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetHarrier
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_U32      intIndex  = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_LAST_INT_INDEX_E; /* HW interrupt index for  <double ECC> error */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E:
            intIndex = PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E;
            break;
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E:
            intIndex = PRV_CPSS_HARRIER_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E;
            intIndex2 = PRV_CPSS_HARRIER_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChDiagDataIntegrityNonDfxEventsMaskStateGet(devNum, intIndex, intIndex2, errorTypePtr, operationPtr);
    return rc;
}

/**
* @internal prvMakeHwTableSuitableForApi function
* @endinternal
*
* @brief   convert 'real' HW <table>/<entry index> - as they are in hardware -
*         to 'api compatable' HW <table>/<entry index>
*         suitable to be passed into such API like
*         prvCpssDxChWriteTableEntry, prvCpssDxChReadTableEntry.
*         Don't mess with DataIntegrity 'logical table'
*         (CPSS_DXCH_LOGICAL_TABLE_ENT).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum            - the device number
* @param[in,out] tableTypePtr  - (pointer to) the HW table name
* @param[in,out] entryIndexPtr - in : HW table entry index
*                                out: entry index applicable to be passed
*                                     into APIs
* @param[in] startBit          - start bit of HW table entry's bits range we're
*                                interested in. This information can be useful
*                                in case when single 'real' HW index appropriates
*                                to several 'api compatable' indexes to clarify
*                                which one should be chosen.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if tableType is out of range.
* @retval GT_BAD_PARAM             - if tableTypePtr or entryIndexPtr is null
* @retval GT_FAIL                  - on impossible entryIndex value
*/
static GT_STATUS prvMakeHwTableSuitableForApi
(
    IN    GT_U8                devNum,
    INOUT CPSS_DXCH_TABLE_ENT *tableTypePtr,
    INOUT GT_U32              *entryIndexPtr,
    IN    GT_U32               startBit
)
{
    GT_U32              startBitTmp   = 0;
    GT_U32              entryIndex;
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32              tabDepth;
    GT_U32              numBitsPerEntry;
    GT_U32              numEntriesPerLine;
    GT_U32              entryIndexTmp = 0;

    CPSS_NULL_PTR_CHECK_MAC(tableTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(entryIndexPtr);

    tableType  = *tableTypePtr;
    entryIndex = *entryIndexPtr;

    /* No HW table or shadow table read/write count */
    if((CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily) &&
       (((tableType >= CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E) && (tableType <= CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E+3)) ||
        ((tableType >= CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E) && (tableType <= CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E))))
    {
        return GT_OK;
    }

    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    tabDepth = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType)->maxNumOfEntries;

    if(GT_TRUE == prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(
           devNum, tableType,
           INOUT &entryIndexTmp /* ignored */, INOUT &startBitTmp /* ignored */,
           OUT &numBitsPerEntry , OUT &numEntriesPerLine))
    {

        if (numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
        {
            entryIndex %= tabDepth / (numEntriesPerLine - FRACTION_HALF_TABLE_INDICATION_CNS);
        }
        else if (numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            entryIndex /= numEntriesPerLine - FRACTION_INDICATION_CNS;
        }
        else if (numEntriesPerLine & VERTICAL_INDICATION_CNS)
        {
            entryIndex +=  tabDepth * (startBit / numBitsPerEntry); /* brackets are important! */
        }
        else
        {
            entryIndex *= numEntriesPerLine;

            /* several entries per single RAM line. We can't determine exactly
               which one is failed . Return first one. */
            entryIndex += startBit / numBitsPerEntry;
        }

    }

    *entryIndexPtr = entryIndex;
    *tableTypePtr  = tableType;
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGet function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity events details.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*         Unlike prvCpssDxChDiagDataIntegrityDfxEventsViaMgUnitGet it doesn't
*         read MG unit registers "DFX/DFX1 Interrupts Summary Cause Register".
*         It reads DFX unit register "Server Interrupt Summary Cause Register"
*         instead.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGet
(
    IN  GT_U8                                       devNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    clientRegData;
    GT_U32    eventCounter = 0; /* events counter */
    GT_U32    pipesBmp;
    GT_U32    pipeId;
    GT_U32    clientsBmp;
    GT_U32    clientId;
    GT_U32    memRangeIter;
    GT_U32    memBmp;
    GT_U32    memId;

    GT_STATUS rc;

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }


    /* read Server Interrupt Summary Cause Register */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXServerUnits.DFXServerRegs.serverInterruptSummaryCause;

    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChDfxPipesBmpGet(devNum, &pipesBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if (!(pipesBmp & 1))
        {
            continue;
        }

        /* bits 3..10 signals about interrupts in pipes 0..7. */
        if  (! (regData & (BIT_3 << pipeId)))
        {
            continue;
        }

        rc = prvCpssDxChDfxClientsBmpGet(devNum, pipeId, &clientsBmp);
        if (rc != GT_OK)
        {
            return rc;
        }


        /* iterate all client: from 27 down to 0 */
        for (clientId = 27 + 1; clientId-- > 0;)
        {
            if (! ((clientsBmp >> clientId) & 1))
            {
                /* this client is not valid */
                continue;
            }

            rc = prvCpssDfxClientRegRead(
                devNum, pipeId, clientId,
                PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_CAUSE_CNS,
                &clientRegData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* check register bits 1..4 responsible for memories 0..127 interrupts
             * bit 1 - memories  0..31  (range 0)
             * bit 2 - memories 32..63  (range 1)
             * bit 3 - memories 64..95  (range 2)
             * bit 4 - memories 96..127 (range 3) */
            for (memRangeIter = 0; memRangeIter < 4; memRangeIter++)
            {
                if (clientRegData & (BIT_1 << memRangeIter))
                {
                    rc = prvCpssDfxClientRegRead(
                        devNum, pipeId, clientId,
                        PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS + memRangeIter * 4,
                        &memBmp);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    for (memId = memRangeIter * 32; memBmp; memId++, memBmp >>= 1)
                    {
                        if (memBmp & 0x1)
                        {
                            rc = prvCpssDxChDiagDfxMemoryEventFromDbGet(
                                devNum, pipeId, clientId, memId, &eventsArr[eventCounter++]);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                            if (eventCounter == *eventsNumPtr)
                            {
                                *isNoMoreEventsPtr = GT_FALSE;
                                return GT_OK;
                            }
                        }
                    }
                }
            }

            /* if bit #5 - "Previous Clients Interrupts Summary" - is not raised we can
             * skip rest of clients */
            if (! (clientRegData & BIT_5))
            {
                break;
            }
        }
    }

    *eventsNumPtr      = eventCounter;
    *isNoMoreEventsPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10 function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity events details.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*
* @note   APPLICABLE DEVICES:      AC5; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Falcon; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - PP device number
* @param[in] pipeId                  - DFX Pipe where interrupt came from
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6_10
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pipeId,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_U32    clientInterruptCauseRegAddr;     /* DFX Client Interrupt Cause register address */
    GT_U32    clientMemInterruptStatusRegAddr; /* DFX Client Memories Interrupt Cause register address */
    GT_U32    clientRegData;    /* value of DFX Client Interrupt Cause register */
    GT_U32    eventCounter = 0; /* events counter */
    GT_U32    clientsBmp;       /* bitmap of all available DFX clients */
    GT_U32    clientId;         /* iterator for DFX clients */
    GT_U32    memRangeIter;     /* memories ranges iterator */
    GT_U32    memBmp;           /* bitmap of memories       */
    GT_U32    memId;            /* memory ID                */
    GT_STATUS rc;               /* return status            */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    rc = prvCpssDxChDfxClientsBmpGet(devNum, pipeId, &clientsBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    clientInterruptCauseRegAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                    sip6_tile_DFXClientUnits[0].clientInterruptCause;
    clientMemInterruptStatusRegAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                    sip6_tile_DFXClientUnits[0].clientMemoriesInterruptStatus[0];


    /* iterate all client: from 27 down to 0 */
    for (clientId = 27 + 1; clientId-- > 0;)
    {
        if (! ((clientsBmp >> clientId) & 1))
        {
            /* this client is not valid */
            continue;
        }

        rc = prvCpssDfxClientRegRead(
            devNum, pipeId, clientId, clientInterruptCauseRegAddr, &clientRegData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* check register bits 1..4 responsible for memories 0..127 interrupts
         * bit 1 - memories  0..31  (range 0)
         * bit 2 - memories 32..63  (range 1)
         * bit 3 - memories 64..95  (range 2)
         * bit 4 - memories 96..127 (range 3) */
        for (memRangeIter = 0; memRangeIter < 4; memRangeIter++)
        {
            if (clientRegData & (BIT_1 << memRangeIter))
            {
                rc = prvCpssDfxClientRegRead(
                    devNum, pipeId, clientId,
                    clientMemInterruptStatusRegAddr + memRangeIter * 4,
                    &memBmp);
                if (rc != GT_OK)
                {
                    return rc;
                }

                for (memId = memRangeIter * 32; memBmp; memId++, memBmp >>= 1)
                {
                    if (memBmp & 0x1)
                    {
                        rc = prvCpssDxChDiagDfxMemoryEventFromDbGet(
                            devNum, pipeId, clientId, memId, &eventsArr[eventCounter++]);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        if (eventCounter == *eventsNumPtr)
                        {
                            *isNoMoreEventsPtr = GT_FALSE;
                            return GT_OK;
                        }
                    }
                }
            }
        }

        /* if bit #5 - "Previous Clients Interrupts Summary" - is not raised we can
         * skip rest of clients */
        if (! (clientRegData & BIT_5))
        {
            break;
        }
    }

    *eventsNumPtr      = eventCounter;
    *isNoMoreEventsPtr = GT_TRUE;
    return GT_OK;
}


/**
* @internal prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6 function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity events details.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*         Unlike prvCpssDxChDiagDataIntegrityDfxEventsViaMgUnitGet it doesn't
*         read MG unit registers "DFX/DFX1 Interrupts Summary Cause Register".
*         It reads DFX unit register "Server Interrupt Summary Cause Register"
*         instead.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - PP device number
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - Tile index
* @param[in] chipletIndex             - Chiplet index
*
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityDfxEventsViaDfxServerGetSip6
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT   dfxInstanceType,
    IN  GT_U32                                      tileIndex,
    IN  GT_U32                                      chipletIndex,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    clientRegData;
    GT_U32    eventCounter = 0; /* events counter */
    GT_U32    pipesBmp;
    GT_U32    pipeId;
    GT_U32    clientsBmp;
    GT_U32    clientId;
    GT_U32    memRangeIter;
    GT_U32    memBmp;
    GT_U32    memId;

    GT_STATUS rc;

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    /* read Server Interrupt Summary Cause Register */
    switch (dfxInstanceType)
    {
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_tile_DFXServerUnits[tileIndex].DFXServerRegs.serverInterruptSummaryCause;
            break;
      case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_chiplet_DFXServerUnits[tileIndex*4 + chipletIndex].DFXServerRegs.serverInterruptSummaryCause;
            break;
      default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    /* SIP6 devices use usual address space for DFX registers. */
    rc = prvCpssDrvHwPpReadRegister (devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set DFX multiinstance data  */
    PRV_CPSS_DFX_MULTI_INSTANCE_DATA_SET_MAC(devNum, dfxInstanceType, tileIndex, chipletIndex);
    rc = prvCpssDxChDfxPipesBmpGetFalcon(dfxInstanceType, tileIndex, &pipesBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if (!(pipesBmp & 1))
        {
            continue;
        }

        /* bits 3..10 signals about interrupts in pipes 0..7. */
        if  (! (regData & (BIT_3 << pipeId)))
        {
            continue;
        }

        rc = prvCpssDxChMultiInstanceDfxClientsBmpGet(devNum, pipeId, dfxInstanceType, tileIndex, &clientsBmp);
        if (rc != GT_OK)
        {
            return rc;
        }


        /* iterate all client: from 27 down to 0 */
        for (clientId = 27 + 1; clientId-- > 0;)
        {
            if (! ((clientsBmp >> clientId) & 1))
            {
                /* this client is not valid */
                continue;
            }

            /* read Client Interrupt Cause Register */
            switch (dfxInstanceType)
            {
                case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
                    regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                    sip6_tile_DFXClientUnits[tileIndex].clientInterruptCause;
                    break;
                case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
                    regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                    sip6_chiplet_DFXClientUnits[tileIndex*4 + chipletIndex].clientInterruptCause;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    break;
            }
            rc = prvCpssDfxClientRegReadFalcon(
                devNum, pipeId, clientId,
                regAddr,
                dfxInstanceType, tileIndex, chipletIndex,
                &clientRegData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* read Client Interrupt Status Register */
            switch (dfxInstanceType)
            {
                case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
                    regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                    sip6_tile_DFXClientUnits[tileIndex].clientMemoriesInterruptStatus[0];
                    break;
                case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
                    regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                    sip6_chiplet_DFXClientUnits[tileIndex*4 + chipletIndex].clientMemoriesInterruptStatus[0];
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    break;
            }
            /* check register bits 1..4 responsible for memories 0..127 interrupts
             * bit 1 - memories  0..31  (range 0)
             * bit 2 - memories 32..63  (range 1)
             * bit 3 - memories 64..95  (range 2)
             * bit 4 - memories 96..127 (range 3) */
            for (memRangeIter = 0; memRangeIter < 4; memRangeIter++)
            {
                if (clientRegData & (BIT_1 << memRangeIter))
                {
                    rc = prvCpssDfxClientRegReadFalcon(
                        devNum, pipeId, clientId,
                        regAddr + memRangeIter * 4,
                        dfxInstanceType, tileIndex, chipletIndex,
                        &memBmp);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    for (memId = memRangeIter * 32; memBmp; memId++, memBmp >>= 1)
                    {
                        if (memBmp & 0x1)
                        {
                            rc = prvCpssDxChDiagDfxMemoryEventFromDbGetFalcon(devNum, pipeId, clientId, memId,
                                dfxInstanceType, tileIndex, chipletIndex, &eventsArr[eventCounter++]);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                            if (eventCounter == *eventsNumPtr)
                            {
                                *isNoMoreEventsPtr = GT_FALSE;
                                return GT_OK;
                            }
                        }
                    }
                }
            }

            /* if bit #5 - "Previous Clients Interrupts Summary" - is not raised we can
             * skip rest of clients */
            if (! (clientRegData & BIT_5))
            {
                break;
            }
        }
    }

    *eventsNumPtr      = eventCounter;
    *isNoMoreEventsPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDfxMemoryEventFromDbGet function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] pipeId                   - DFX pipe index
* @param[in] clientId                 - DFX client index
* @param[in] memId                    - DFX memory index
*
* @param[out] eventPtr                 - (pointer to) occured event info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDfxMemoryEventFromDbGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pipeId,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      memId,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
)
{

    GT_U32 regData; /* data of register    */
    GT_U32 regAddr; /* address of register */
    GT_U32 rc;      /* return code         */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */

     /* AC5 use SIP_6 based management system */
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ||
        (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* use Tile #0 for SIP_6 devices */
        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXRam[0].memoryInterruptCause;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_CAUSE_REG_CNS;
    }

    /* Read Memory Interrupt Cause Register */
    rc = prvCpssDfxMemoryRegRead(devNum, pipeId, clientId, memId, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = searchMemType(devNum, pipeId, clientId, memId, &dbItemPtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* update output parameters */
    eventPtr->location.ramEntryInfo.memType                 = dbItemPtr->memType;
    eventPtr->location.portGroupsBmp                        = 1 << dbItemPtr->causePortGroupId;
    eventPtr->location.ramEntryInfo.memLocation.dfxPipeId   = pipeId;
    eventPtr->location.ramEntryInfo.memLocation.dfxClientId = clientId;
    eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId = memId;
    eventPtr->memoryUseType                                 = dbItemPtr->memUsageType;
    eventPtr->correctionMethod                              = dbItemPtr->correctionMethod;

    eventPtr->location.isMppmInfoValid =
        eventPtr->location.isTcamInfoValid = GT_FALSE;

    if(U32_GET_FIELD_MAC(regData, 1, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else if(U32_GET_FIELD_MAC(regData, 2, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
    }
    else if(U32_GET_FIELD_MAC(regData, 3, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDfxMemoryEventFromDbGetFalcon function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details for falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] pipeId                   - DFX pipe index
* @param[in] clientId                 - DFX client index
* @param[in] memId                    - DFX memory index
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - Tile index
* @param[in] chipletIndex             - Chiplet index
*
* @param[out] eventPtr                 - (pointer to) occured event info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDfxMemoryEventFromDbGetFalcon
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      pipeId,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      memId,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT   dfxInstanceType,
    IN  GT_U32                                      tileIndex,
    IN  GT_U32                                      chipletIndex,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
)
{

    GT_U32 regData;
    GT_U32 rc;
    GT_U32 regAddr;
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */
    GT_U32  dfxInstanceIndex;
    GT_U32  portGroup;

    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
    {
        dfxInstanceIndex = tileIndex;
        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                  sip6_tile_DFXRam[dfxInstanceIndex].memoryInterruptCause;
    }
    else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
    {
        dfxInstanceIndex = tileIndex*4 + chipletIndex;
        regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                  sip6_chiplet_DFXRam[dfxInstanceIndex].memoryInterruptCause;
    }
    else
    {
        /* dfx instance type is invalid */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* Read Memory Interrupt Cause Register */
    rc = prvCpssDfxMemoryRegReadFalcon(
        devNum, pipeId, clientId, memId,
        regAddr, dfxInstanceType, tileIndex, chipletIndex,
        &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = searchMemTypeFalcon(pipeId, clientId, memId, dfxInstanceType, &dbItemPtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* update output parameters */
    eventPtr->location.ramEntryInfo.memType                 = dbItemPtr->memType;

    if (dbItemPtr->causePortGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        /* all per tile units use local port group 0 for CPSS management */
        portGroup = 0;
    }
    else if (tileIndex & 1)
    {
        /* tiles 1,3 has port group mirroring */
        portGroup = (dbItemPtr->causePortGroupId == 0) ? 1 : 0;
    }
    else
    {
        portGroup = dbItemPtr->causePortGroupId;
    }

    eventPtr->location.portGroupsBmp                        = 1 << (portGroup + 2*tileIndex);
    eventPtr->location.ramEntryInfo.memLocation.dfxPipeId   = pipeId;
    eventPtr->location.ramEntryInfo.memLocation.dfxClientId = clientId;
    eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId = memId;
    eventPtr->memoryUseType                                 = dbItemPtr->memUsageType;
    eventPtr->correctionMethod                              = dbItemPtr->correctionMethod;
    eventPtr->location.ramEntryInfo.memLocation.dfxInstance.dfxInstanceType = dfxInstanceType;
    eventPtr->location.ramEntryInfo.memLocation.dfxInstance.dfxInstanceIndex = dfxInstanceIndex;

    eventPtr->location.isMppmInfoValid =
        eventPtr->location.isTcamInfoValid = GT_FALSE;

    if(U32_GET_FIELD_MAC(regData, 1, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else if(U32_GET_FIELD_MAC(regData, 2, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
    }
    else if(U32_GET_FIELD_MAC(regData, 3, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}



/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat3 function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobcat3
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_BOOL isLocationAssigned; /* if DFX coordinates of memory already calculated */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }
    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    isLocationAssigned = GT_FALSE;
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_BOBCAT3_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        /* -- _PARITY_ -- */

        case PRV_CPSS_BOBCAT3_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    =  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.tcamParityErrorAddr;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }
            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 0, 1) == 0) ?
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E :
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

            eventsArr[0].location.tcamMemLocation.ruleIndex =
                U32_GET_FIELD_MAC(regData, 13, 4) /* floor */  * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
                U32_GET_FIELD_MAC(regData, 5, 8)  /* row */    * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +
                U32_GET_FIELD_MAC(regData, 1, 4)  /* bank */;

            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;

        case PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ1_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_PIPE0_TXQ_DQ2_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupts are out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            portGroupsBmp = BIT_0;
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ3_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ4_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_PIPE1_TXQ_DQ5_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupts are out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            portGroupsBmp = BIT_1;
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBCAT3_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBCAT3_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBCAT3_BMA_SUM_MC_CNT_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = portGroupsBmp;

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */

    if (GT_TRUE == isLocationAssigned)
    {
        /* look for the DB item with specified DFX coordinates */
        rc = searchMemType(devNum,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The Data Integrity DB item not found");
        }
    }
    else
    {
        /* look for the first DB item with specified memory type */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
        if (NULL == dbPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                  "Data Integrity DB not found for the device");
        }

        memType = eventsArr[0].location.ramEntryInfo.memType;
        for (i = 0; i < dbSize && dbPtr[i].memType != (GT_U32)memType; i++ );
        if (i == dbSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                     "The Data Integrity DB item is not found");
        }

        dbItemPtr = &dbPtr[i];

        /* assign DFX coordinates */
        eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    }

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}


/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin2 function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAldrin2
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_BOOL isLocationAssigned; /* if DFX coordinates of memory already calculated */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }
    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    isLocationAssigned = GT_FALSE;
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_ALDRIN2_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        /* -- _PARITY_ -- */

        case PRV_CPSS_ALDRIN2_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    =  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;

            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.tcamParityErrorAddr;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }
            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 0, 1) == 0) ?
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E :
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

            eventsArr[0].location.tcamMemLocation.ruleIndex =
                U32_GET_FIELD_MAC(regData, 13, 4) /* floor */  * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
                U32_GET_FIELD_MAC(regData, 5, 8)  /* row */    * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +
                U32_GET_FIELD_MAC(regData, 1, 4)  /* bank */;

            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;

        case PRV_CPSS_ALDRIN2_TXQ_DQ0_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN2_TXQ_DQ1_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN2_TXQ_DQ2_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupts are out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            portGroupsBmp = BIT_0;
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_ALDRIN2_TXQ_DQ3_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupts are out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            portGroupsBmp = BIT_1;
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_ALDRIN2_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_ALDRIN2_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_ALDRIN2_BMA_SUM_MC_CNT_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = portGroupsBmp;

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */

    if (GT_TRUE == isLocationAssigned)
    {
        /* look for the DB item with specified DFX coordinates */
        rc = searchMemType(devNum,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The Data Integrity DB item not found");
        }
    }
    else
    {
        /* look for the first DB item with specified memory type */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
        if (NULL == dbPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                  "Data Integrity DB not found for the device");
        }

        memType = eventsArr[0].location.ramEntryInfo.memType;
        for (i = 0; i < dbSize && dbPtr[i].memType != memType; i++ );
        if (i == dbSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                     "The Data Integrity DB item is not found");
        }

        dbItemPtr = &dbPtr[i];

        /* assign DFX coordinates */
        eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    }

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsDataGet function
* @endinternal
*
* @brief   Function fills event details  for non-DFX DataIntegrity event.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - PP device number
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex                - event tile index
* @param[in] isTcam                   - is TCAM event
                                        GT_TRUE - DFX event is TCAM related
                                        GT_FALSE - DFX event is not TCAM related
* @param[out] eventsArr[]             - (array of) events
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsDataGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT   dfxInstanceType,
    IN  GT_U32                                      tileIndex,
    IN  GT_BOOL                                     isTcam,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[]
)
{
    GT_STATUS rc = GT_OK;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_U32 portGroupId;         /* port group id to read from specific tile */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC const *dbPtr = NULL;  /* Data Integrity DB */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC const *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */
    GT_PORT_GROUPS_BMP portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT    dfxInstance; /* DFX Instance Type: TILE/CHIPLET */

    dfxInstance = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; /* compiler warning */

    if(isTcam)
    {
        eventsArr[0].location.ramEntryInfo.memType    =  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;
        eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.tcamParityErrorAddr;
        portGroupId = PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE ? tileIndex * 2 :  CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &regData);
        if(GT_OK != rc)
        {
            return rc;
        }
        eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 0, 1) == 0) ?
            CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E :
            CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

        eventsArr[0].location.tcamMemLocation.ruleIndex =
            U32_GET_FIELD_MAC(regData, 13, 4) /* floor */  * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
            U32_GET_FIELD_MAC(regData, 5, 8)  /* row */    * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +
            U32_GET_FIELD_MAC(regData, 1, 4)  /* bank */;

        eventsArr[0].location.isTcamInfoValid = GT_TRUE;
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = portGroupsBmp;

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */
    if (PRV_CPSS_DEV_DFX_MULTI_INSTANCE_CHECK_MAC(devNum) == GT_TRUE)
    {
        dfxInstance = (isTcam == GT_TRUE) ? CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E : dfxInstanceType;
        if (dfxInstance == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
        {
            /* look for the first DB item with specified memory type */
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E, &dbPtr, &dbSize);
        }
        else
        {
            /* currently per Raven DB no non-dfx interrupt handling exists */
            /* look for the first DB item with specified memory type */
            prvCpssDxChDiagDataIntegrityDbPointerSetFalcon(CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E, &dbPtr, &dbSize);
        }
    }
    else
    {
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
    }

    if (NULL == dbPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                              "Data Integrity DB not found for the device");
    }

    memType = eventsArr[0].location.ramEntryInfo.memType;
    for (i = 0; i < dbSize && dbPtr[i].memType != memType; i++ );
    if (i == dbSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                 "The Data Integrity DB item is not found");
    }

    dbItemPtr = &dbPtr[i];

    /* assign DFX coordinates */
    eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
        PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

    eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
        PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

    eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
        PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetFalcon function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetFalcon
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;               /* return code */
    GT_U32 tileIndex = 0;       /* tile index */
    GT_BOOL isTcam = GT_FALSE;  /* if tcam event occus */
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT    dfxInstanceType; /* DFX Instance Type: TILE/CHIPLET */

    dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E; /* compiler warning */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }
    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        /* -- _PARITY_ -- */
        case PRV_CPSS_FALCON_TILE_0_TCAM_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            tileIndex = 0;
            break;
        case PRV_CPSS_FALCON_TILE_1_TCAM_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            tileIndex = 1;
            break;
        case PRV_CPSS_FALCON_TILE_2_TCAM_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            tileIndex = 2;
            break;
        case PRV_CPSS_FALCON_TILE_3_TCAM_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            tileIndex = 3;
            break;

        case PRV_CPSS_FALCON_TILE_0_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_1_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_2_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_3_PIPE0_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_0_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_1_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_2_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E:
        case PRV_CPSS_FALCON_TILE_3_PIPE1_BMA_SUM_MC_CNT_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            break;

        case PRV_CPSS_FALCON_TILE_0_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_1_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_2_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_3_PIPE0_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_0_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_1_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_2_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_3_PIPE1_HA_SUM_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            break;

        case PRV_CPSS_FALCON_TILE_0_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_1_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_2_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_3_PIPE0_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_0_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_1_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_2_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E:
        case PRV_CPSS_FALCON_TILE_3_PIPE1_HA_SUM_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            dfxInstanceType = CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc  = prvCpssDxChDiagDataIntegrityNonDfxEventsDataGet(devNum, dfxInstanceType, tileIndex, isTcam, eventsArr);
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5x function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      AC5X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5x
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;               /* return code */
    GT_BOOL isTcam = GT_FALSE;  /* if tcam event occus */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }
    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_AC5X_TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            break;

        case PRV_CPSS_AC5X_BMA_MC_CNT_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_AC5X_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc  = prvCpssDxChDiagDataIntegrityNonDfxEventsDataGet(devNum, 0 /* don't care */, 0 /* don't care */, isTcam, eventsArr);
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5p function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetAc5p
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;               /* return code */
    GT_BOOL isTcam = GT_FALSE;  /* if tcam event occus */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }
    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_AC5P_TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            break;

        case PRV_CPSS_AC5P_BMA_MC_CNT_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_AC5P_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        case PRV_CPSS_AC5P_SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_5_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_6_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_7_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_AC5P_SDW_INST_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_1_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_2_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_3_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_4_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_5_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_6_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_AC5P_SDW_INST_7_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc  = prvCpssDxChDiagDataIntegrityNonDfxEventsDataGet(devNum, 0 /* don't care */, 0 /* don't  care */, isTcam, eventsArr);
    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetHarrier function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; AC5P; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetHarrier
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc = GT_OK;               /* return code */
    GT_BOOL isTcam = GT_FALSE;  /* if tcam event occus */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }
    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_HARRIER_TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E:
            isTcam = GT_TRUE;
            break;

        case PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_HARRIER_HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_HBU_HEADER_BUFFER_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        case PRV_CPSS_HARRIER_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_HARRIER_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
        case PRV_CPSS_HARRIER_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_SDW_SRAM64K_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc  = prvCpssDxChDiagDataIntegrityNonDfxEventsDataGet(devNum, 0 /* don't care */, 0 /* don't  care */, isTcam, eventsArr);
    return rc;
}


/**
* @internal prvCpssDxChMultiInstanceDfxClientsBmpGet function
* @endinternal
*
* @brief   Function returns bitmap of DFX clients relevant for specified pipe index and dfx instance type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Lion2;
*                                  Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X
*
* @param[in] devNum                - PP device number
* @param[in] pipeId                - Pipe index
* @param[in] dfxInstanceType       - DFX Instance type: TILE/CHIPLET
* @param[in] tileIndex             - Tile index
* @param[out] clientsBmpPtr        - (pinter to) clients bitmap
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail of algorithm
*/
GT_STATUS prvCpssDxChMultiInstanceDfxClientsBmpGet
(
    IN  GT_U8                                     devNum,
    IN  GT_U32                                    pipeId,
    IN  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType,
    IN  GT_U32                                    tileIndex,
    OUT GT_U32                                    *clientsBmpPtr
)
{
    GT_U32 result;

    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }


    switch (dfxInstanceType)
    {
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_FALCON_DFX_PIPES_INDEX_CHECK(tileIndex, pipeId, result);
            /* Falcon Tile 1,3 has DFX Pipes 0..3 and Tile 0,2 has 0..4 so
             * substracting one for odd tile indexes.
             */
            if (!result)
            {
                *clientsBmpPtr = eagleDfxPipeClientsBmpArr[pipeId];
            }
            else
            {
                *clientsBmpPtr = 0;
            }
            break;
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
            if (pipeId < sizeof(ravenDfxPipeClientsBmpArr)/sizeof(ravenDfxPipeClientsBmpArr[0]))
            {
                *clientsBmpPtr = ravenDfxPipeClientsBmpArr[pipeId];
            }
            else
            {
                *clientsBmpPtr = 0;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSet function
* @endinternal
*
* @brief   Mask/unmask bits indicating Data Integrity ECC/Parity errors in the
*         DFX registers "Server Interrupt Summary Mask Register",
*         "Client Interrupt Summary Mask Register".
*         It makes possible manage DFX memories events appearance inside this
*         registers via signle register - "Memory Interrupt Mask Resigter"
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if pipe number is out of range.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_IMPLEMENTED       - if feature is not implemented for the device yet
*/
GT_STATUS prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSet
(
    IN GT_U8                   devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_U32 pipesBmp;   /* bitmap of DFX pipes in device      */
    GT_U32 pipeId;     /* DFX pipe ID                        */
    GT_U32 clientsBmp; /* bitmap of all valid pipe's clients */
    GT_U32 clientId;   /* DFX client ID                 */
    GT_U32 regAddr;    /* address of register           */
    GT_U32 data;       /* data                          */
    GT_U32 sip6Logic;  /* SIP_6 managment logic is used */
    GT_STATUS rc;      /* return code                   */

    rc = prvCpssDxChDfxPipesBmpGet(devNum, &pipesBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(pipesBmp >= BIT_8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* AC5 use SIP_6 based management system */
    sip6Logic = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ||
                (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily);

    /* usual DATA_INTEGRITY event logic manages "pipe_n" bits in
       serverInterruptSummaryMask for SIP_6 devices. Need to skip
       configuration for SIP_6.*/
    if (!sip6Logic)
    {
        /* update Server Interrupt Summary Mask Register, bits 3..10
           which are responsible for masking pipes 0..7. */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnits.DFXServerRegs.serverInterruptSummaryMask;

        data = (CPSS_EVENT_UNMASK_E == operation) ? pipesBmp : 0;

        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField (devNum, regAddr, 3, 8, data);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* update Client Interrupt Summary Mask Register bits 0..5 for all clients:
     * Bits 1..4 summarize RAM interrupts for memories:
     *     0..31, 32..63, 64..95, 96..124
     * Bit 5 summarizes interrupts from previous client in the chain. */
    data =  (CPSS_EVENT_UNMASK_E == operation) ? 0x1f : 0;

    /* use Tile #0 for SIP_6 devices */
    regAddr = sip6Logic ? PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                                    sip6_tile_DFXClientUnits[0].clientInterruptMask :
                          PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_MASK_CNS;

    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if (pipesBmp & 1)
        {
            rc = prvCpssDxChDfxClientsBmpGet(devNum, pipeId, &clientsBmp);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* iterate all clients 0..27 */
            for (clientId = 0; clientsBmp; clientId++, clientsBmp >>= 1)
            {
                if (clientsBmp & 1)
                {
                    rc = prvCpssDfxClientSetRegField(
                        devNum, pipeId, clientId, regAddr, 1, 5, data);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSetSip6 function
* @endinternal
*
* @brief   Mask/unmask bits indicating Data Integrity ECC/Parity errors in the
*         "Client Interrupt Summary Mask Register".
*         It makes possible manage DFX memories events appearance inside this
*         registers via single register - "Memory Interrupt Mask Resigter"
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] operation                - mask/unmask operation
* @param[in] dfxInstanceIndex         - DFX Instance index
*                                      TILE    : 0 - 3
*                                      CHIPLET : 0 - 15
* @param[in] dfxInstanceType          - DFX Instance type: TILE/CHIPLET
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if pipe number is out of range.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_IMPLEMENTED       - if feature is not implemented for the device yet
*/
GT_STATUS prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSetSip6
(
    IN GT_U8                                  devNum,
    IN CPSS_EVENT_MASK_SET_ENT                operation,
    GT_U32                                    dfxInstanceIndex,
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT dfxInstanceType
)
{
    GT_U32 pipesBmp;
    GT_U32 pipeId;
    GT_U32 clientsBmp; /* bitmap of all valid pipe's clients */
    GT_U32 clientId;
    GT_U32 regAddr;
    GT_U32 data;
    GT_STATUS rc;
    GT_U32 tileIndex;
    GT_U32 chipletIndex;

    if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E)
    {
        tileIndex = dfxInstanceIndex;
        chipletIndex = 0;
    }
    else if (dfxInstanceType == CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E)
    {
        tileIndex = dfxInstanceIndex/4;
        chipletIndex = dfxInstanceIndex%4;
    }
    else
    {
        /* dfx instance type is invalid */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* Set DFX multiinstance data  */
    PRV_CPSS_DFX_MULTI_INSTANCE_DATA_SET_MAC(devNum, dfxInstanceType, dfxInstanceIndex, chipletIndex);
    rc = prvCpssDxChDfxPipesBmpGetFalcon(dfxInstanceType, dfxInstanceIndex, &pipesBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update Client Interrupt Summary Mask Register bits 0..5 for all clients:
     * Bits 1..4 summarize RAM interrupts for memories:
     *     0..31, 32..63, 64..95, 96..124
     * Bit 5 summarizes interrupts from previous client in the chain. */
    data =  (CPSS_EVENT_UNMASK_E == operation) ? 0x1f : 0;

    /* read Client Interrupt Mask Register */
    switch (dfxInstanceType)
    {
        case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_tile_DFXClientUnits[tileIndex].clientInterruptMask;
            break;
      case CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E:
            regAddr = PRV_CPSS_PP_MAC(devNum)->resetAndInitControllerRegsAddr.
                sip6_chiplet_DFXClientUnits[tileIndex*4 + chipletIndex].clientInterruptMask;
            break;
      default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if (pipesBmp & 1)
        {
            rc = prvCpssDxChMultiInstanceDfxClientsBmpGet(devNum, pipeId, dfxInstanceType, tileIndex, &clientsBmp);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* iterate all clients 0..27 */
            for (clientId = 0; clientsBmp; clientId++, clientsBmp >>= 1)
            {
                if (clientsBmp & 1)
                {
                    rc = prvCpssDfxClientSetRegFieldFalcon(
                        devNum, pipeId, clientId,
                        regAddr,
                        1, 5, data,
                        dfxInstanceType, tileIndex, chipletIndex);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChDiagDataIntegrityAllDfxServersClientsIntUnMask function
* @endinternal
*
* @brief   Unmask bits indicating Data Integrity ECC/Parity errors in all
*         "Client Interrupt Summary Mask Register".
*         It makes possible manage DFX memories events appearance inside this
*         registers via single register - "Memory Interrupt Mask Resigter"
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if pipe number is out of range.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_IMPLEMENTED       - if feature is not implemented for the device yet
*/
GT_STATUS prvCpssDxChDiagDataIntegrityAllDfxServersClientsIntUnMask
(
    IN GT_U8                                        devNum
)
{
    GT_U32          tileIndex;
    GT_U32          chipletIndex;
    CPSS_EVENT_MASK_SET_ENT    operation = CPSS_EVENT_UNMASK_E;
    GT_STATUS   rc;

    for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        rc = prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSetSip6(devNum,
                                                                        operation, tileIndex,
                                                                        CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        for (chipletIndex = 0; chipletIndex < FALCON_RAVENS_PER_TILE; chipletIndex++)
        {
            PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, chipletIndex);
            rc = prvCpssDxChDiagDataIntegrityDfxServerClientsIntMaskSetSip6(devNum,
                                                                            operation,
                                                                            (PRV_CPSS_DFX_CHIPLET_MAC(tileIndex,chipletIndex)),
                                                                            CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_CHIPLET_E);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvPhyToTxqRange function
* @endinternal
*
* @brief   convert physical ports range to the closest valid txq ports ranges
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number
* @param[in] phyStart                 - physical port - first port of the range
* @param[in] phyRange                 - range size
*
* @param[out] txqStartPtr              - (pointer to) txq port appropriate to phyStart.
*                                      if port phyStart doesn't mappped to txq port, scan
*                                      the specified range up from phyStart for
*                                      the first physical port having txq port.
* @param[out] txqRangePtr              - (pointer to) txq ports range size appropriate to
*                                      phyRange.
*                                      If upper bound port of physical ports
*                                      range isn't mapped to txq port scan range down
*                                      from the upper bound for the first physical
*                                      port having txq port.
*/
static GT_VOID prvPhyToTxqRange
(
    IN  GT_U32 devNum,
    IN  GT_U32 phyStart,
    IN  GT_U32 phyRange,
    OUT GT_U32 *txqStartPtr,
    OUT GT_U32 *txqRangePtr
)
{
    GT_STATUS rc;
    GT_U32 phyPort;
    GT_U32 txqPort;
    GT_U32 txqStart = 0;
    GT_U32 txqRange = 0;
    GT_U32 limit = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);


    if (limit > (phyStart + phyRange))
    {
        limit = phyStart + phyRange;
    }
    /* convert phyStart to the closest valid txq port */
    for (phyPort = phyStart; phyPort < limit; phyPort++)
    {

        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            (GT_U8)devNum, phyPort, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &txqPort);
        if (rc == GT_OK)
        {
            txqStart = txqPort;
            break;
        }
    }
    if (phyPort != limit)       /* txq mapping was found */
    {
        /* determine txqRange */
        if (limit)
        {
            /* convert upper phy port to valid txq Port */
            ;
            for (phyPort = limit - 1; phyPort+1 > phyStart; phyPort--)
            {
                rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
                    (GT_U8)devNum, phyPort, PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &txqPort);
                if (GT_OK == rc)
                {
                    txqRange = txqPort + 1 - txqStart;
                    break;
                }

            }
        }
    }
    *txqStartPtr = txqStart;
    *txqRangePtr = txqRange;
}

/**
* @internal prvTxqToPhyRange function
* @endinternal
*
* @brief   convert physical ports range to the closest valid txq ports ranges
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number
* @param[in] txqStart                 - TXQ port - first port of the range
* @param[in] txqRange                 - TXQ ports range size
*
* @param[out] phyStartPtr              - (pointer to) txq port appropriate to phyStart.
*                                      if port phyStart doesn't mappped to txq port, scan
*                                      the specified range up from phyStart for
*                                      the first physical port having txq port.
* @param[out] phyRangePtr              - (pointer to) txq ports range size appropriate to
*                                      phyRange. If upper bound port of physical ports
*                                      range isn't mapped to txq port scan range down
*                                      from the upper bound for the first physical
*                                      port having txq port.
*/
static GT_VOID prvTxqToPhyRange
(
    IN  GT_U32 devNum,
    IN  GT_U32 txqStart,
    IN  GT_U32 txqRange,
    OUT GT_U32 *phyStartPtr,
    OUT GT_U32 *phyRangePtr
)
{
    GT_U32 phyPort;
    GT_U32 txqPort = 0;
    GT_U32 phyStart = 0;
    GT_U32 phyRange = 0;
    GT_U32 limit = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    CPSS_DXCH_DETAILED_PORT_MAP_STC *mapInfoPtr;

    mapInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr;

    /* get physical port for TXQ port txqStart */
    for (phyStart = phyPort = 0; phyPort < limit; phyPort++)
    {

        txqPort = mapInfoPtr[phyPort].portMap.txqNum;
        if (txqPort == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
        {
            continue;
        }
        phyStart = phyPort;
        if (txqPort >= txqStart)
        {
            break;
        }
    }

    if (phyPort != limit) /* phyStart contains valid port. Calculate phyRange */
    {
        phyRange = phyStart;
        for (phyPort = phyStart + 1 ; phyPort < limit; phyPort++)
        {
            if (txqPort == CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS)
            {
                continue;
            }
            txqPort = mapInfoPtr[phyPort].portMap.txqNum;
            if (txqPort >= txqStart + txqRange)
            {
                /* this phy port is out of our txq ports range */
                break;
            }
            phyRange = phyPort;
        }
        phyRange += 1 - phyStart;
    }

    /* fill OUT paramteters */
    *phyStartPtr = phyStart;
    *phyRangePtr = phyRange;
}

/**
* @internal prvCpssDxChDiagDataIntegrityLogicalToHwTableMap function
* @endinternal
*
* @brief   Convert logical table entries range to an appropriate HW table entries
*         range. If logical range is too big the HW range relevant to sub-range
*         applicable for the HW table will be returned instead.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number
* @param[in] logicalTable             - logical table
* @param[in] logicalStart             - logical item index starting the range
* @param[in] logicalRange             - range size
* @param[in] hwTable                  - HW table
*
* @param[out] hwStartPtr               - (pointer to) HW entry index appropriate to logicalStart.
*                                      Can be NULL.
* @param[out] hwRangePtr               - (pointer to) HW indexes range size.
*                                      Can be NULL.
* @param[out] hwLogicalFirstPtr        - (pointer to) the logical index appropriate
*                                      to the HW table first entry.
*                                      Can be NULL.
* @param[out] hwLogicalMaxPtr          - (pointer to) index of last+1 logical entry
*                                      appropriate to the full HW table size.
*                                      Can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on impossible logicalRange
* @retval GT_BAD_PARAM             - on unknown logical table or HW table.
* @retval GT_BAD_VALUE             - if HW table belongs to logical table not covered by the
*                                       function
*
* @note Use logicalStart=0, logicalRange = 0xFFFFFFFF to get full HW range
*       relevant to the logical table
*
*/
GT_STATUS prvCpssDxChDiagDataIntegrityLogicalToHwTableMap
(
    IN  GT_U32                       devNum,
    IN  CPSS_DXCH_LOGICAL_TABLE_ENT  logicalTable,
    IN  GT_U32                       logicalStart,
    IN  GT_U32                       logicalRange,
    IN  CPSS_DXCH_TABLE_ENT          hwTable,
    OUT GT_U32                       *hwStartPtr,
    OUT GT_U32                       *hwRangePtr,
    OUT GT_U32                       *hwLogicalFirstPtr,
    OUT GT_U32                       *hwLogicalMaxPtr
)
{
    GT_STATUS rc;
    GT_U32 tableSize;
    GT_U32 dqIndex;
    GT_U32 entryOffset;
    GT_U32 isLimitedRange = (logicalRange != (GT_U32)-1); /* true if logical range is not 0xFFFFFFFF */
    GT_U32 hwMin;        /* index of first hw entry relevant to logical table */
    GT_U32 hwMax;        /* index of last+1 hw entry relevant to logical table */
    GT_U32 hwRangeStart; /* hw entry index appropriate to logicalStart */
    GT_U32 hwRangeMax;   /* hw entry index appropriate to (logicalStart + logicalRange) */
    GT_U32 hwLogicalFirst; /* index of first logical entry relevant to HW table */
    GT_U32 hwLogicalMax;   /* index of last+1 logical entry relevant to HW table */
    GT_U32 exactMatchNumOfBanks; /* number of banks for exact match in shared mode */

    rc = prvCpssDxChDiagDataIntegrityTableHwMaxIndexGet((GT_U8)devNum, hwTable, &tableSize, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* init default values */
    hwMin   = 0;
    hwMax   = tableSize;
    hwRangeStart = logicalStart;
    hwRangeMax   =  isLimitedRange ? (logicalStart + logicalRange) : tableSize;
    hwLogicalFirst = hwMin;
    hwLogicalMax = hwMax;

    switch (hwTable)
    {
        case CPSS_DXCH_SIP5_TABLE_TCAM_E:
            /* get TCAM entry index per rule without rule check */
            rc = prvCpssDxChTcamRuleIndexToSip5HwEntryNumber(logicalStart, &hwRangeStart);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* get TCAM entry index per rule without rule check */
            if (isLimitedRange)
            {
                /* get TCAM entry index per rule without rule check */
                rc = prvCpssDxChTcamRuleIndexToSip5HwEntryNumber(logicalStart + logicalRange,
                                                                 &hwRangeMax);
                if(rc != GT_OK)
                {
                    return rc;
                }
                break;
            }
            hwLogicalMax = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;
            GT_ATTR_FALLTHROUGH;
        case CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E:
            /* 8 ARP entries in one HW entry */
            hwRangeStart = logicalStart / 8;
            if (isLimitedRange)
            {
                hwRangeMax = (logicalStart + logicalRange) / 8;
            }

            hwLogicalMax *= 8;

            break;

        case CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E:
            /* all tables are the same size */
            if (CPSS_DXCH_LOGICAL_TABLE_PHYSICAL_PORT_E == logicalTable)
            {
                /* Convert physical ports range to the TXQ ports range */
                prvPhyToTxqRange(
                    devNum, logicalStart, logicalRange,
                    &logicalStart, &logicalRange);

                hwRangeStart = logicalStart;
                if (isLimitedRange)
                {
                    hwRangeMax = hwRangeStart + logicalRange;
                }
                prvTxqToPhyRange(devNum, hwMin, hwMax - hwMin,
                                 &hwLogicalFirst, &hwLogicalMax);
                hwLogicalMax += hwLogicalFirst;
            }
            break;

        case CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* for indirect accsess the index For all MHT modes: 4/8/16 bank design, entry bits are as follows:
                 * [3:0] - Bank number
                 * [19:4] - Address inside bank
                 * No need conversion for MHT16 - all 4 bits are used for bank ID
                 * MHT 8 uses 3 bits for bank ID and MHT 4 uses 2 bits for bank ID and hence needs
                 * conversion of entry indexes to specified position in tables this ignoring the
                 * not required hash location.
                 */
                PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(devNum,exactMatchNumOfBanks);
                switch(exactMatchNumOfBanks)
                {
                    case 4:/* Four Multiple Hash Tables */
                        /* 4 banks are used : bank=0..3
                            number of entries is banks * _8K */
                        hwLogicalMax = _32K;
                        break;
                    case 8:/* Eight Multiple Hash Tables */
                        /* 8 banks are used: bank=0..7
                            number of entries is banks * _8K */
                        hwLogicalMax = _64K;
                        break;
                    case 16:/* Sixteen Multiple Hash Tables  */
                        /* 16 banks are used: bank=0..15
                             number of entries is banks * _8K */
                        hwLogicalMax = _128K;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                }
            }
            break;

        default:
            break;
    }


    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        switch (hwTable)
        {
            case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E:
            case CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E:

                /* PLR_METERING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL */
                if (CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_METERING_E == logicalTable)
                {
                    hwMax = IPLR0_SIZE_MAC(devNum);
                }
                else if (CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_METERING_E == logicalTable)
                {
                    hwMin = IPLR0_SIZE_MAC(devNum);
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        /* egress metering items are stored here too, so upper bound for IPLR1
                         * still need to be precised  */
                        hwMax = IPLR0_SIZE_MAC(devNum) + IPLR1_SIZE_MAC(devNum);
                    }
                }
                else if (CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_METERING_E == logicalTable)
                {
                    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        hwMin = IPLR0_SIZE_MAC(devNum) + IPLR1_SIZE_MAC(devNum);
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                }

                hwRangeStart = hwMin + logicalStart;
                if (isLimitedRange)
                {
                    hwRangeMax = hwRangeStart + logicalRange;
                }
                hwLogicalMax = hwMax - hwMin;
                break;

            default:
                break;
        }
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        switch (hwTable)
        {
            case CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E:
                /* PLR_COUNTING_SHARED_USE_PLR0_AND_BASE_ADDR_LABEL */
                if (CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_0_COUNTING_E == logicalTable)
                {
                    hwMax = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? IPLR0_COUNTING_SIZE_MAC(devNum) : IPLR0_SIZE_MAC(devNum);
                }
                else if (CPSS_DXCH_LOGICAL_TABLE_INGRESS_POLICER_1_COUNTING_E == logicalTable)
                {
                    hwMin = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? IPLR0_COUNTING_SIZE_MAC(devNum) : IPLR0_SIZE_MAC(devNum);
                    hwMax = hwMin + (PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? IPLR1_COUNTING_SIZE_MAC(devNum) : IPLR1_SIZE_MAC(devNum));
                }
                else if (CPSS_DXCH_LOGICAL_TABLE_EGRESS_POLICER_COUNTING_E == logicalTable)
                {
                    hwMin = PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? IPLR0_COUNTING_SIZE_MAC(devNum) + IPLR1_COUNTING_SIZE_MAC(devNum)
                                                                 : IPLR0_SIZE_MAC(devNum) + IPLR1_SIZE_MAC(devNum);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                }
                hwRangeStart = hwMin + logicalStart;
                if (isLimitedRange)
                {
                    hwRangeMax = hwRangeStart + logicalRange;
                }
                hwLogicalMax = hwMax - hwMin;
                break;

            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_2_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_3_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_4_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_5_E:
                if (CPSS_DXCH_LOGICAL_TABLE_PHYSICAL_PORT_E == logicalTable)
                {
                    /* all tables are the same size */
                    dqIndex = hwTable - CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E;
                    hwLogicalFirst = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[dqIndex].pfcResponseFirstPortInRange;
                    hwLogicalMax   = hwLogicalFirst + tableSize;
                    hwRangeStart   = (logicalStart >= hwLogicalFirst) ?
                        (logicalStart - hwLogicalFirst) : 0;
                    if (isLimitedRange)
                    {
                        hwRangeMax   = hwRangeStart + logicalRange;
                    }
                }
                break;

            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_2_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_3_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_4_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_5_E:
                /* all tables are the same size */
                if (CPSS_DXCH_LOGICAL_TABLE_PHYSICAL_PORT_E == logicalTable)
                {
                    dqIndex  = hwTable + 1 -
                        CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E;

                    /* Convert physical ports range to the TXQ ports range */
                    prvPhyToTxqRange(
                        devNum, logicalStart, logicalRange,
                        &logicalStart, &logicalRange);

                    entryOffset = tableSize * dqIndex;
                    hwRangeStart =  (logicalStart >= entryOffset) ?
                        (logicalStart - entryOffset) : 0;
                    if (isLimitedRange)
                    {
                        hwRangeMax = hwRangeStart + logicalRange;
                    }
                    prvTxqToPhyRange(devNum, hwMin, hwMax - hwMin,
                                     &hwLogicalFirst, &hwLogicalMax);
                    hwLogicalMax += hwLogicalFirst;
                }
                break;

            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_2_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_3_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_4_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_5_E:
                /* all tables are the same size */
                dqIndex  = hwTable + 1 -
                    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E;
                goto lbl_multiDqTablesHandle;
                break;

            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_2_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_3_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E:
            case CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E:
                /* all tables are the same size */
                dqIndex  = hwTable + 1 -
                    CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E;
            lbl_multiDqTablesHandle:
                hwLogicalFirst = tableSize * dqIndex;
                hwLogicalMax = hwLogicalFirst + tableSize;
                hwRangeStart =  (logicalStart >= hwLogicalFirst) ?
                    (logicalStart - hwLogicalFirst) : 0;
                if (isLimitedRange)
                {
                    hwRangeMax = hwRangeStart + logicalRange;
                }
                break;

            default:
                break;
        }
    }

    hwRangeStart = MIN(hwRangeStart, hwMax);
    hwRangeMax   = MIN(hwRangeMax, hwMax);

    /* assign OUT parameters */
    if (hwStartPtr)
    {
        *hwStartPtr = hwRangeStart;
    }

    if (hwRangePtr)
    {
        *hwRangePtr = (hwRangeMax < hwRangeStart) ? 0 : hwRangeMax - hwRangeStart;
    }

    if (hwLogicalFirstPtr)
    {
        *hwLogicalFirstPtr =hwLogicalFirst;
    }

    if (hwLogicalMaxPtr)
    {
        *hwLogicalMaxPtr =hwLogicalMax;
    }

    return GT_OK;
}

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
)
{
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryPipeIdSetFunc =
            prvCpssDxChLion2DiagDataIntegrityDfxMemoryPipeIdSet;
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryWriteFunc =
            prvCpssDxChLion2DiagDataIntegrityDfxMemoryRegWrite;
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryRegReadFunc =
            prvCpssDxChLion2DiagDataIntegrityDfxMemoryRegRead;
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxClientReadRegFunc =
            prvCpssDxChLion2DiagDataIntegrityDfxClientRegRead;
    }
    else
    {
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryPipeIdSetFunc = NULL;
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryWriteFunc     = NULL;
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxMemoryRegReadFunc   = NULL;
        PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxClientReadRegFunc   = NULL;
    }

    PRV_CPSS_PP_MAC(devNum)->diagInfo.dfxPipesBmpGetFunc = prvCpssDxChDfxPipesBmpGet;

    return;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobk function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskBobk
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */
    GT_U32      upperIntIndex;  /* upper HW interrupt index (for the loop) */

    /* Unused parameter.
     * errorType is used only with memories with not-DFX ECC protection . */
    (void)errorType;


    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
                intIndex      = PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
                upperIntIndex = PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E;
            while (intIndex <= upperIntIndex && rc == GT_OK)
            {
                rc = prvCpssDrvEventsMask(devNum, intIndex++, intMaskEn);
            }
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssDxChDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                             PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E,
                             PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_BOBK_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            /* PARITY */
            intIndex = PRV_CPSS_BOBK_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        default:
            rc = prvIsMemProtectedButNotSupportedSip5(devNum, memType) ?
                GT_NOT_SUPPORTED : GT_BAD_PARAM;
            break;
    }

    return rc;
}

/**
* @internal prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobk function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityExtMemoryProtectionEventMaskGetBobk
(
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_BOBK_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_BOBK_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            /* read for BM core 0 only. cores 0..5 are expected to store the same values */
            intIndex = PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            intIndex      = PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E;
            intIndex2     = PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E:
            intIndex = PRV_CPSS_BOBK_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E;
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_BOBK_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E;
            break;

        default:
            return (prvIsMemProtectedButNotSupportedSip5(devNum, memType) ?
                    GT_NOT_SUPPORTED : GT_BAD_PARAM);
            break;

    }

    if (intIndex == PRV_CPSS_BOBK_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_BOBK_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobk function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssDxChDiagDataIntegrityNonDfxEventsGetBobk
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regData;             /* register data */
    GT_U32 i;                   /* loop iterators */
    GT_BOOL isLocationAssigned; /* if DFX coordinates of memory already calculated */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    const PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    isLocationAssigned = GT_FALSE;
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].location.isTcamInfoValid = GT_FALSE;
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;

    switch(intNum)
    {
        case PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_BOBK_MPPM_ECC_SUM_MPPM_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        /* -- _PARITY_ -- */

        case PRV_CPSS_BOBK_TCAM_LOGIC_SUM_TCAM_ARRAY_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    =  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TCAM_PARITY_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.tcamInterrupts.tcamParityErrorAddr;
            rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }
            eventsArr[0].location.tcamMemLocation.arrayType = (U32_GET_FIELD_MAC(regData, 0, 1) == 0) ?
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E :
                CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_Y_E;

            eventsArr[0].location.tcamMemLocation.ruleIndex =
                U32_GET_FIELD_MAC(regData, 13, 4) /* floor */  * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
                U32_GET_FIELD_MAC(regData, 5, 8)  /* row */    * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS +
                U32_GET_FIELD_MAC(regData, 1, 4)  /* bank */;

            eventsArr[0].location.isTcamInfoValid = GT_TRUE;

            break;

        case PRV_CPSS_BOBK_TXQ_DQ_MEM_ERR_SUM_TOKEN_BUCKET_PRIO_PARITY_ERROR_E:
            /* This interrupt is out of interrupts tree because of erratum.
               See WA description: PRV_CPSS_DXCH_BOBCAT2_RM_SHAPER_PARITY_ERROR_INTERRUPT_WA_E */
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_DQ_TB_GIGA_PRIO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBK_QCN_SUM_CN_BUFFER_FIFO_PARITY_ERR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_0_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_1_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_2_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_3_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_4_VALID_TABLE_PARITY_ERROR_E:
        case PRV_CPSS_BOBK_BM_GEN1_SUM_CORE_5_VALID_TABLE_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType = CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* fill causePortGroupId of found memories */
    eventsArr[0].location.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* assign DFX coordinates (if not assigned before), usage type
       and correction method  of the specified memory */

    if (GT_TRUE == isLocationAssigned)
    {
        /* look for the DB item with specified DFX coordinates */
        rc = searchMemType(devNum,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId,
                           eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId,
                           &dbItemPtr);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The Data Integrity DB item not found");
        }
    }
    else
    {
        /* look for the first DB item with specified memory type */
        prvCpssDxChDiagDataIntegrityDbPointerSet(devNum, &dbPtr, &dbSize);
        if (NULL == dbPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                  "Data Integrity DB not found for the device");
        }

        memType = eventsArr[0].location.ramEntryInfo.memType;
        for (i = 0; i < dbSize && dbPtr[i].memType != (GT_U32)memType; i++ );
        if (i == dbSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                     "The Data Integrity DB item is not found");
        }

        dbItemPtr = &dbPtr[i];

        /* assign DFX coordinates */
        eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_PIPE_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_CLIENT_MAC(dbItemPtr->key);

        eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
            PRV_CPSS_DXCH_DIAG_DATA_INTEGRITY_KEY_MEMORY_MAC(dbItemPtr->key);

    }

    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}

/**
* @internal prvIsMemProtectedButNotSupportedSip5 function
* @endinternal
*
* @brief Check special cases when memory is protected in the device,
*        but CPSS is not able get and handle DataIntegrity events
*        for this memory somewhy.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @retval GT_TRUE              - the memory is protected but CPSS doesn't support memory's
*                                DataIntegrity actions.
* @retval GT_FALSE             - otherwise.
*/
static GT_BOOL prvIsMemProtectedButNotSupportedSip5
(
    IN  GT_U8                                      devNum,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType
)
{
    GT_BOOL intResult = GT_FALSE;
    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MLL_CONF_TABLE_E:
            /* there are no interrupts for non-DFX protected
               MLL memories in bobcat2 a0.
               The memories are not protected at all starting
               from bobcat2 b0 */
            intResult = PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L0_DATA_BANK_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L0_DPARITY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L0_TAG_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_ATTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_DATA_BANK_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_DPARITY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_STATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DCACHE_L1_TAG_WAY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L0_DATA_BANK_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L0_TAG_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L1_DATA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ICACHE_L1_TAG_WAY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_IF_ID_IS_BPM_BANK_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_DATA_RAM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_ECC_RAM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_L2_TDV_RAM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MMU_TLB_DATA_WAY_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MMU_TLB_TAG_WAY_E:
            /* non-DFX protected memories inaccessable from PP in bobcat2, bobk devices */
            intResult = (CPSS_PP_FAMILY_DXCH_BOBCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily);
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_LL_ATTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_LL_BAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_LL_BVAL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_LL_PTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_RD_CTRL_ADDR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_RD_DATA_PARAM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_SERIAL_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_WR_CTRL_ADDR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_WR_CTRL_DATA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_BAP_CHR_WR_DATA_BURST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_AAQL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_ACURVE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_ADP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_APROF_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_APROFPNTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_BAQL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_BDP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_BPROF_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_BPROFPNTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_CAQL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_CDP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_CPROF_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_CPROFPNTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_PAQLCOS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_PAQL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_PDP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_PGDP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_PPROF_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_QAQL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_QCOS_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_QCURVE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_QDP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_QPROF_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMDRP_QPROFPNTR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMMSG_PNODE_DP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMMSG_QCLS_0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMMSG_QNODE_DP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMMSG_QNODE_STATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_FREE_RD_HEAD_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_FREE_WR_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_FREE_WR_TAIL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_PAGE_DEQ_CACHE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_PAGE_DEQ_CURR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_PAGE_DEQ_NEXT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_PAGE_ENQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_PKG_FREE_WR_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PAGE_PKG_FREE_WR_TAIL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_FREE_RD_HEAD_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PAYLOAD_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PKG_BOTH_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PKG_HEAD_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PKG_TAIL_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PKT_ENQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PKT_ENQ_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_PKT_ENQ_MSG_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TM_QMR_PKTPKG_SYNC_DATA_FIFO_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMRCB_CACHE_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMRCB_FIFO_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMRCB_PORT_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMRCB_TX_SYNC_MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_CLS0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_GRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_TBNEG2MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_TBNEGMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_WFSMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_CLS0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_GRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBNEG2MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBNEGMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_WFSMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBNEG2MEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBNEGMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_WFSMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_CLS0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_GRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CLAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CMYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CNODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLGRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLQ2AMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PTBMEM_E:
            /* TM memories interrupts are not supported in bobcat2, bobk.
               May be will be supported later. */
            intResult = ((CPSS_PP_FAMILY_DXCH_BOBCAT2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)||
                         (CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily));
            break;

        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMMSG_QCLS_1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_CLS1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_CLS1_E:
            /* TM memories interrupts are not supported in bobcat2, bobk.
               May be will be supported later.
               Here are bobcat2-specific memories.  */
            intResult = PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum);
            break;

        default:
            break;
    }
    return (intResult ? GT_TRUE : GT_FALSE);
}

/**
* @internal prvIsMemProtectedButNotSupportedCnmAc5_Ac5x function
* @endinternal
*
* @brief Check special cases when memory is protected in the device,
*        but CPSS is not able get and handle DataIntegrity events
*        for this memory somewhy.
*
* @note   APPLICABLE DEVICES:      AC5X; Ironman; AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Harrier.
*
* @param[in] memType                  - memory type
*
* @retval GT_TRUE              - the memory is protected but CPSS doesn't support memory's
*                                DataIntegrity actions.
* @retval GT_FALSE             - otherwise.
*/
static GT_BOOL prvIsMemProtectedButNotSupportedCnmAc5_Ac5x
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType
)
{
    GT_BOOL intResult = GT_FALSE;
    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CORE_CLUSTER_ANANKE_BTAC_STAGE_0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CORE_CLUSTER_ANANKE_BTAC_STAGE_1_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CORE_CLUSTER_ANANKE_L1D_DATA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CORE_CLUSTER_ANANKE_L1I_DATA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CORE_CLUSTER_ANANKE_TLB_DATA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_CORE_CLUSTER_ANANKE_TLB_TAG_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DSU_CORINTH_SCU_SF_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DSU_CORINTH_L3_DATA_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DSU_CORINTH_L3D_TAG_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_DSU_CORINTH_L3_VICTIM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_GIC_GICD_LPI_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_GIC_ITS_V_CACHE_E:
            /* these are internal CPU related RAMs with external protection only.
               CPSS does not cares such RAMs and interrupt managment is not
               supported by CPSS for them.*/
            intResult = GT_TRUE;
            break;

        default:
            break;
    }
    return (intResult ? GT_TRUE : GT_FALSE);
}

/**
* @internal prvIsMemProtectedButNotSupportedSip6_10 function
* @endinternal
*
* @brief Check special cases when memory is protected in the device,
*        but CPSS is not able get and handle DataIntegrity events
*        for this memory somewhy.
*
* @note   APPLICABLE DEVICES:      AC5X; AC5P; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] memType                  - memory type
*
* @retval GT_TRUE              - the memory is protected but CPSS doesn't support memory's
*                                DataIntegrity actions.
* @retval GT_FALSE             - otherwise.
*/
static GT_BOOL prvIsMemProtectedButNotSupportedSip6_10
(
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT  memType
)
{
    GT_BOOL intResult = GT_FALSE;
    switch (memType)
    {
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBNK_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_CLS0_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_GRNDPRNT_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHCMDPIPE_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_NODESTATE_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHALVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_TBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_NODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_PRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_LAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_FUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHBLVL_MYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CPRNTMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CLAST_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CMYQ_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PPERCONFMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PTBMEM_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PNODEDWRR_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_PFUNC_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLQ2AMAP_E:
        case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TMSCHPCCTRL_CTRLGRNDPRNT_E:
            /* these are TxQ (TM based units) related RAMs with external protection only.
               Interrupt managment is not supported by CPSS for them.*/
            intResult = GT_TRUE;
            break;

        default:
            break;
    }
    return (intResult ? GT_TRUE : GT_FALSE);
}
/**
*
* @internal internal_cpssDxChDiagDataIntegritySerConfigSet function
*
* @brief Set the CPU code and packet command for the SER error detection.
*        Applicable for both to packet's header ECC and Tables ECC errors
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number
* @param[in] cpuCode               - Packet CPU/Drop code
* @param[in] pktCmd                - Packet command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device, packet command or CPU code
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChDiagDataIntegritySerConfigSet
(
    IN GT_U8                     devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode,
    IN CPSS_PACKET_CMD_ENT       pktCmd
)
{
    GT_STATUS                       rc;
    GT_U32                          regAddr, data, pktCmdInHwFormat=0;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  cpuCodeInHwFormat;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &cpuCodeInHwFormat);
    if (rc != GT_OK)
    {
        return rc;
    }
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(pktCmdInHwFormat, pktCmd);

    /*Data
    3:10 SER_CPU_CODE
    0:2 SER_PKT_CMD
    */
    data = (cpuCodeInHwFormat << 3) | pktCmdInHwFormat ;
    /*Config the ERMRK SER_CONFIG_REG*/
    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKSerConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 11, data);
    if(rc != GT_OK)
    {
        return rc;
    }
    /*Config the HA SER_CONFIG_REG*/
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).HASerConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 2, 11, data);
    return rc;
}

/**
*
* @internal cpssDxChDiagDataIntegritySerConfigSet function
*
* @brief Set the CPU code  and packet command for the SER error detection.
*        Applicable for both to packet's header ECC and Tables ECC errors
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum         - device Number
* @param[in] cpuCode        - Packet CPU code
* @param[in] pktCmd         - Packet command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device, packet command or CPU code
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChDiagDataIntegritySerConfigSet
(
    IN GT_U8                     devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT  cpuCode,
    IN CPSS_PACKET_CMD_ENT       pktCmd
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,cpssDxChDiagDataIntegritySerConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCode, pktCmd));

    rc = internal_cpssDxChDiagDataIntegritySerConfigSet(devNum, cpuCode, pktCmd);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
*
* @internal internal_cpssDxChDiagDataIntegritySerConfigGet function
*
* @brief Get the CPU code and packet command for the SER error detection.
*        Applicable for both to packet's header ECC and Tables ECC errors
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number
* @param[out] cpuCodePtr           - (pointer to) Packet CPU/Drop code
* @param[out] pktCmdPtr            - (pointer to) Packet command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_STATE             - inconsistent values in HA and ERMRK
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChDiagDataIntegritySerConfigGet
(
    IN GT_U8                     devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr,
    OUT CPSS_PACKET_CMD_ENT      *pktCmdPtr
)
{
    GT_STATUS                       rc ;
    GT_U32                          regAddr, data, data1;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  cpuCodeInHwFormat;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);
    CPSS_NULL_PTR_CHECK_MAC(pktCmdPtr);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    /*Get from the ERMRK SER_CONFIG_REG*/
    regAddr = PRV_DXCH_REG1_UNIT_ERMRK_MAC(devNum).ERMRKSerConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 11, &data);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*Get from the HA SER_CONFIG_REG*/
    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(devNum).HASerConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 2, 11, &data1);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(data != data1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "SER config ERMRK[%x] != HA[%x] ",data, data1);
    }
    /*Data
    3:10 SER_CPU_CODE
    0:2 SER_PKT_CMD
    */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(*pktCmdPtr, (data & 0x7));
    cpuCodeInHwFormat = (data >> 3) & 0xFF;

    rc = prvCpssDxChNetIfDsaToCpuCode(cpuCodeInHwFormat, cpuCodePtr);
    return rc;
}

/**
*
* @internal cpssDxChDiagDataIntegritySerConfigGet function
*
* @brief Get the CPU code and packet command for the SER error detection.
*        Applicable for both to packet's header ECC and Tables ECC errors
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number
* @param[out] cpuCodePtr           - (pointer to) Packet CPU/Drop code
* @param[out] pktCmdPtr            - (pointer to) Packet command
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_STATE             - inconsistent values in HA and ERMRK
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChDiagDataIntegritySerConfigGet
(
    IN GT_U8                      devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr,
    OUT CPSS_PACKET_CMD_ENT      *pktCmdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,cpssDxChDiagDataIntegritySerConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cpuCodePtr));

    rc = internal_cpssDxChDiagDataIntegritySerConfigGet(devNum, cpuCodePtr, pktCmdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cpuCodePtr, pktCmdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

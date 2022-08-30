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
* @file gtDbDxHawkScreening.c
*
* @brief Utilities for screening Hawk boards.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <gtOs/gtOsExc.h>

#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*============================================================================================*/

/**
* @internal hawk_ro_chk function
* @endinternal
*
* @brief   Read register by address
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] regAddr               - The register address.
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @retval GT_OK                    - on success,
* @retval other                    - otherwise.
*
*/
static GT_STATUS hawk_ro_chk
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
)
{
    GT_STATUS rc;
    GT_U32    data = 0;

    if (afterPrintSleep != 0) cpssOsPrintf("hawk_ro_chk regAddr 0x%08X \n", regAddr);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 0 /*portGroupId*/, regAddr, &data);
    if (afterPrintSleep != 0) cpssOsPrintf("read done data 0x%08X rc %d \n", data, rc);
    return rc;
}

/**
* @internal hawk_rw_chk function
* @endinternal
*
* @brief   Read and write back register by address
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] regAddr               - The register address.
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @retval GT_OK                    - on success,
* @retval other                    - otherwise.
*
*/
static GT_STATUS hawk_rw_chk
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U32    data = 0;

    if (afterPrintSleep != 0) cpssOsPrintf("hawk_rw_chk regAddr 0x%08X \n", regAddr);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 0 /*portGroupId*/, regAddr, &data);
    if (rc != GT_OK) {rc1 = rc;}
    if (afterPrintSleep != 0) cpssOsPrintf("read done data 0x%08X rc %d - start write back \n", data, rc);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, 0 /*portGroupId*/, regAddr, data);
    if (rc != GT_OK) {rc1 = rc;}
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    if (afterPrintSleep != 0) cpssOsPrintf("data written back done rc %d \n", rc);
    return rc1;
}

/**
* @internal hawk_rw_pattern function
* @endinternal
*
* @brief   Read, write patterns, check, restore original register data
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] regAddr               - The register address.
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @retval GT_OK                    - on success,
* @retval other                    - otherwise.
*
*/
static GT_STATUS hawk_rw_pattern
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U32    originalData = 0;
    GT_U32    data = 0;
    GT_U32    pattern[] = {0, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA};
    GT_U32    patternSize = sizeof(pattern) / sizeof(pattern[0]);
    GT_U32    i;

    if (afterPrintSleep != 0) cpssOsPrintf("hawk_rw_pattern regAddr 0x%08X \n", regAddr);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 0 /*portGroupId*/, regAddr, &originalData);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Read regAddr 0x%08X failed rc %d\n", regAddr, rc);
        rc1 = rc;
    }
    if (afterPrintSleep != 0) cpssOsPrintf("read done originalData 0x%08X rc %d \n", originalData, rc);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    for (i = 0; (i < patternSize); i++)
    {
        if (afterPrintSleep != 0) cpssOsPrintf("start pattern write 0x%08X  rc %d \n", pattern[i], rc);
        if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
        rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, 0 /*portGroupId*/, regAddr, pattern[i]);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Write regAddr 0x%08X failed rc %d\n", regAddr, rc);
            rc1 = rc;
        }
        if (afterPrintSleep != 0) cpssOsPrintf("start pattern read \n");
        if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
        rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 0 /*portGroupId*/, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Read regAddr 0x%08X failed rc %d\n", regAddr, rc);
            rc1 = rc;
        }
        if (afterPrintSleep != 0) cpssOsPrintf("read data 0x%08X rc %d \n", data, rc);
        if (data != pattern[i])
        {
            cpssOsPrintf(
                "regAddr 0x%08X written 0x%08X read  0x%08X\n",
                regAddr, pattern[i], data);
            rc1 = GT_FAIL;
        }
    }
    if (afterPrintSleep != 0) cpssOsPrintf("start originalData write,  previous rc %d \n", rc);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, 0 /*portGroupId*/, regAddr, originalData);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Write regAddr 0x%08X failed rc %d\n", regAddr, rc);
        rc1 = rc;
    }
    if (afterPrintSleep != 0) cpssOsPrintf("originalData written back rc %d \n", rc);
    return rc1;
}

/**
* @internal hawk_rw_chk hawk_rd_verify
* @endinternal
*
* @brief   Read and veriyy register by address and pattern
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] regAddr               - The register address.
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @param[in] pattern               - pattern to compare
* @retval GT_OK                    - on success,
* @retval other                    - otherwise.
*
*/
static GT_STATUS hawk_rd_verify
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep,
    IN  GT_U32           pattern
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U32    data = 0;

    if (afterPrintSleep != 0) cpssOsPrintf("hawk_rd_verify regAddr 0x%08X \n", regAddr);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 0 /*portGroupId*/, regAddr, &data);
    if (rc != GT_OK) {rc1 = rc;}
    if (afterPrintSleep != 0) cpssOsPrintf("read done data 0x%08X rc %d - start write back \n", data, rc);
    if (data != pattern)
    {
        cpssOsPrintf(
            "hawk_rd_verify regAddr 0x%08X expected  0x%08X found 0x%08X \n",
            regAddr, pattern, data);
        rc1 = GT_BAD_STATE;
    }
    return rc1;
}

/* arrays for one tile */

typedef GT_STATUS (*HAWK_CHK_FUNC_PTR)
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
);

/* to avoid compiler warning on yet unused functions upper */
HAWK_CHK_FUNC_PTR hawk_screening_registers_checking_functions_arr[] =
{
    &hawk_ro_chk,
    &hawk_rw_chk,
    &hawk_rw_pattern
};

GT_VOID *hawk_screening_registers_verify_functions_arr[] =
{
    (GT_VOID *)&hawk_rd_verify
};

typedef enum
{
    OFFSET_TYPE_NUMBER_E,
    OFFSET_TYPE_UNIT_BASE_ID_E
} OFFSET_TYPE_ENT;
typedef struct
{
    OFFSET_TYPE_ENT offsetType;
    GT_U32 *offsetsArrPtr;
    GT_U32 offsetsNum;
} OFFSETS_ARR_STC;
typedef OFFSETS_ARR_STC *OFFSETS_ARR_PTR;

typedef struct
{
    OFFSETS_ARR_PTR  *offsetsArrsPtrArrPtr;
    GT_U32           offsetsArrsNum;
} OFFSETS_TREE_STC;

/**
* @internal hawk_tree_check function
* @endinternal
*
* @brief   check set of the same register instances using the given function
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] offsetInSubunut       - The register address offset inside subunit.
* @param[in] treePtr               - Pointer to the tree of subunit bases.
* @param[in] funcPtr               - Pointer to the checking function.
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @retval GT_OK                    - on success,
* @retval other                    - otherwise.
*
*/
static GT_STATUS hawk_tree_check
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              offsetInSubunut,
    IN  OFFSETS_TREE_STC    *treePtr,
    IN  HAWK_CHK_FUNC_PTR   funcPtr,
    IN  GT_U32              afterPrintSleep
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U16    index[32];
    GT_U16    i;
    GT_U16    j;
    GT_U16    increasedIndex;
    GT_U16    offArrsInTree;
    GT_U32    regAddr;
    GT_U32    offset;
    GT_U32    value;
    GT_BOOL   error;

    offArrsInTree = (GT_U16)treePtr->offsetsArrsNum;

    if (offArrsInTree > 32)
    {
        cpssOsPrintf("Tree trunkated to 32 offset arrays\n");
        offArrsInTree = 32;
    }
    for (i = 0; (i < offArrsInTree); i++)
    {
        index[i] = 0;
    }

    while (1)
    {
        regAddr = offsetInSubunut;
        for (i = 0; (i < offArrsInTree); i++)
        {
            value = treePtr->offsetsArrsPtrArrPtr[i]->offsetsArrPtr[index[i]];
            switch (treePtr->offsetsArrsPtrArrPtr[i]->offsetType)
            {
                case OFFSET_TYPE_NUMBER_E:
                    offset = value;
                    break;
                case OFFSET_TYPE_UNIT_BASE_ID_E:
                    error = GT_FALSE;
                    offset = prvCpssDxChHwUnitBaseAddrGet(
                        devNum, (PRV_CPSS_DXCH_UNIT_ENT)value, &error);
                    if (error != GT_FALSE)
                    {
                        cpssOsPrintf("Tree: BASE UNIT ID error\n");
                        offset = 0;
                    }
                    break;
                default:
                    cpssOsPrintf("Tree: offset type error\n");
                    offset = 0;
                    break;
            }
            regAddr += offset;
        }

        rc = (*funcPtr)(devNum, regAddr, afterPrintSleep);
        if (rc != GT_OK) {rc1 = rc;}

        /* next instance - look for index that can be increased */
        increasedIndex = 0xFFFF; /* not found */
        for (i = 0; (i < offArrsInTree); i++)
        {
            j = offArrsInTree - i - 1;
            if (((GT_U32)index[j] + 1) < treePtr->offsetsArrsPtrArrPtr[j]->offsetsNum)
            {
                increasedIndex = j;
                break;
            }
        }
        if (increasedIndex == 0xFFFF) break; /* increased index not found */
        index[increasedIndex] ++;
        for (i = (increasedIndex + 1); (i < offArrsInTree); i++)
        {
            index[i] = 0;
        }
    }

    if (afterPrintSleep) cpssOsPrintf("\n");
    return rc1;
}

#define OFFSETS_ARR_NAME(_name) _OFFSETS_ARR_##_name
#define OFFSETS_ARR_DEF_BEGIN(_name)                                  \
    static GT_U32 _OFFSETS_ARR_arr_##_name[] = {
#define OFFSETS_ARR_DEF_END(_name) };                                 \
    static OFFSETS_ARR_STC OFFSETS_ARR_NAME(_name) =                  \
    {                                                                 \
        OFFSET_TYPE_NUMBER_E,                                         \
        &(_OFFSETS_ARR_arr_##_name[0]),                               \
        (sizeof(_OFFSETS_ARR_arr_##_name) / sizeof(GT_U32))           \
    };

#define BASE_UNIT_ID_ARR_NAME(_name) _BASE_UNIT_ID_ARR_##_name
#define BASE_UNIT_ID_ARR_DEF_BEGIN(_name)                             \
    static GT_U32 _BASE_UNIT_ID_ARR_arr_##_name[] = {
#define BASE_UNIT_ID_ARR_DEF_END(_name) };                            \
    static OFFSETS_ARR_STC BASE_UNIT_ID_ARR_NAME(_name) =             \
    {                                                                 \
        OFFSET_TYPE_UNIT_BASE_ID_E,                                   \
        &(_BASE_UNIT_ID_ARR_arr_##_name[0]),                          \
        (sizeof(_BASE_UNIT_ID_ARR_arr_##_name) / sizeof(GT_U32))      \
    };

#define OFFSETS_TREE_NAME(_name) _OFFSETS_TREE_##_name
#define OFFSETS_TREE_DEF_BEGIN(_name)                                      \
    static OFFSETS_ARR_PTR _OFFSETS_TREE_ptr_arr_##_name[] = {
#define OFFSETS_TREE_DEF_END(_name) };                                     \
    static OFFSETS_TREE_STC OFFSETS_TREE_NAME(_name) =                     \
    {                                                                      \
        &(_OFFSETS_TREE_ptr_arr_##_name[0]),                               \
        (sizeof(_OFFSETS_TREE_ptr_arr_##_name) / sizeof(OFFSETS_ARR_PTR))  \
    };

#define BASE_UNIT_ID_ONLY_TREE_BEGIN(_name) BASE_UNIT_ID_ARR_DEF_BEGIN(_name)
#define BASE_UNIT_ID_ONLY_TREE_END(_name)                                  \
    BASE_UNIT_ID_ARR_DEF_END(_name)                                        \
    OFFSETS_TREE_DEF_BEGIN(_name)                                          \
        &BASE_UNIT_ID_ARR_NAME(_name)                                      \
    OFFSETS_TREE_DEF_END(_name)

/* all arrays for one tile */

#if 0
OFFSETS_ARR_DEF_BEGIN(null)
    0
OFFSETS_ARR_DEF_END(null)

OFFSETS_TREE_DEF_BEGIN(null)
    &OFFSETS_ARR_NAME(null)
OFFSETS_TREE_DEF_END(null)
#endif

BASE_UNIT_ID_ONLY_TREE_BEGIN(LPM)
    PRV_CPSS_DXCH_UNIT_LPM_E
BASE_UNIT_ID_ONLY_TREE_END(LPM)

BASE_UNIT_ID_ONLY_TREE_BEGIN(CNC)
    PRV_CPSS_DXCH_UNIT_CNC_0_E,
    PRV_CPSS_DXCH_UNIT_CNC_1_E,
    PRV_CPSS_DXCH_UNIT_CNC_2_E,
    PRV_CPSS_DXCH_UNIT_CNC_3_E
BASE_UNIT_ID_ONLY_TREE_END(CNC)

BASE_UNIT_ID_ONLY_TREE_BEGIN(IPVX)
    PRV_CPSS_DXCH_UNIT_IPVX_E
BASE_UNIT_ID_ONLY_TREE_END(IPVX)

BASE_UNIT_ID_ONLY_TREE_BEGIN(L2I)
    PRV_CPSS_DXCH_UNIT_L2I_E
BASE_UNIT_ID_ONLY_TREE_END(L2I)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCL)
    PRV_CPSS_DXCH_UNIT_PCL_E
BASE_UNIT_ID_ONLY_TREE_END(PCL)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EPLR)
    PRV_CPSS_DXCH_UNIT_EPLR_E
BASE_UNIT_ID_ONLY_TREE_END(EPLR)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EOAM)
    PRV_CPSS_DXCH_UNIT_EOAM_E
BASE_UNIT_ID_ONLY_TREE_END(EOAM)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EPCL)
    PRV_CPSS_DXCH_UNIT_EPCL_E
BASE_UNIT_ID_ONLY_TREE_END(EPCL)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PREQ)
    PRV_CPSS_DXCH_UNIT_PREQ_E
BASE_UNIT_ID_ONLY_TREE_END(PREQ)

BASE_UNIT_ID_ONLY_TREE_BEGIN(ERMRK)
    PRV_CPSS_DXCH_UNIT_ERMRK_E
BASE_UNIT_ID_ONLY_TREE_END(ERMRK)

BASE_UNIT_ID_ONLY_TREE_BEGIN(IOAM)
    PRV_CPSS_DXCH_UNIT_IOAM_E
BASE_UNIT_ID_ONLY_TREE_END(IOAM)

BASE_UNIT_ID_ONLY_TREE_BEGIN(MLL)
    PRV_CPSS_DXCH_UNIT_MLL_E
BASE_UNIT_ID_ONLY_TREE_END(MLL)

BASE_UNIT_ID_ONLY_TREE_BEGIN(IPLR)
    PRV_CPSS_DXCH_UNIT_IPLR_E,
    PRV_CPSS_DXCH_UNIT_IPLR_1_E
BASE_UNIT_ID_ONLY_TREE_END(IPLR)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EQ)
    PRV_CPSS_DXCH_UNIT_EQ_E
BASE_UNIT_ID_ONLY_TREE_END(EQ)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EGF_QAG)
    PRV_CPSS_DXCH_UNIT_EGF_QAG_E
BASE_UNIT_ID_ONLY_TREE_END(EGF_QAG)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EGF_SHT)
    PRV_CPSS_DXCH_UNIT_EGF_SHT_E
BASE_UNIT_ID_ONLY_TREE_END(EGF_SHT)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EGF_EFT)
    PRV_CPSS_DXCH_UNIT_EGF_EFT_E
BASE_UNIT_ID_ONLY_TREE_END(EGF_EFT)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TTI)
    PRV_CPSS_DXCH_UNIT_TTI_E
BASE_UNIT_ID_ONLY_TREE_END(TTI)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PPU)
    PRV_CPSS_DXCH_UNIT_PPU_E
BASE_UNIT_ID_ONLY_TREE_END(PPU)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EREP)
    PRV_CPSS_DXCH_UNIT_EREP_E
BASE_UNIT_ID_ONLY_TREE_END(EREP)

BASE_UNIT_ID_ONLY_TREE_BEGIN(BMA)
    PRV_CPSS_DXCH_UNIT_BMA_E
BASE_UNIT_ID_ONLY_TREE_END(BMA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(IA)
    PRV_CPSS_DXCH_UNIT_IA_E
BASE_UNIT_ID_ONLY_TREE_END(IA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(RXDMA)
     PRV_CPSS_DXCH_UNIT_RXDMA_E,
     PRV_CPSS_DXCH_UNIT_RXDMA1_E,
     PRV_CPSS_DXCH_UNIT_RXDMA2_E,
     PRV_CPSS_DXCH_UNIT_RXDMA3_E
BASE_UNIT_ID_ONLY_TREE_END(RXDMA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXDMA)
     PRV_CPSS_DXCH_UNIT_TXDMA_E,
     PRV_CPSS_DXCH_UNIT_TXDMA1_E,
     PRV_CPSS_DXCH_UNIT_TXDMA2_E,
     PRV_CPSS_DXCH_UNIT_TXDMA3_E
BASE_UNIT_ID_ONLY_TREE_END(TXDMA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXFIFO)
     PRV_CPSS_DXCH_UNIT_TX_FIFO_E,
     PRV_CPSS_DXCH_UNIT_TX_FIFO1_E,
     PRV_CPSS_DXCH_UNIT_TX_FIFO2_E,
     PRV_CPSS_DXCH_UNIT_TX_FIFO3_E
BASE_UNIT_ID_ONLY_TREE_END(TXFIFO)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXQ_SDQ)
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E
BASE_UNIT_ID_ONLY_TREE_END(TXQ_SDQ)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXQ_PDS)
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E
BASE_UNIT_ID_ONLY_TREE_END(TXQ_PDS)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXQ_QFC)
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E,
     PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E
BASE_UNIT_ID_ONLY_TREE_END(TXQ_QFC)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXQ_PDX)
     PRV_CPSS_DXCH_UNIT_TXQ_PDX_E
BASE_UNIT_ID_ONLY_TREE_END(TXQ_PDX)

#ifndef ASIC_SIMULATION

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXQ_PFCC)
     PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E
BASE_UNIT_ID_ONLY_TREE_END(TXQ_PFCC)

#endif /*ASIC_SIMULATION*/

BASE_UNIT_ID_ONLY_TREE_BEGIN(TXQ_PSI)
     PRV_CPSS_DXCH_UNIT_TXQ_PSI_E
BASE_UNIT_ID_ONLY_TREE_END(TXQ_PSI)

BASE_UNIT_ID_ONLY_TREE_BEGIN(HA)
     PRV_CPSS_DXCH_UNIT_HA_E
BASE_UNIT_ID_ONLY_TREE_END(HA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PHA)
     PRV_CPSS_DXCH_UNIT_PHA_E
BASE_UNIT_ID_ONLY_TREE_END(PHA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(SHM)
     PRV_CPSS_DXCH_UNIT_SHM_E
BASE_UNIT_ID_ONLY_TREE_END(SHM)

BASE_UNIT_ID_ONLY_TREE_BEGIN(EM)
     PRV_CPSS_DXCH_UNIT_EM_E
BASE_UNIT_ID_ONLY_TREE_END(EM)

BASE_UNIT_ID_ONLY_TREE_BEGIN(FDB)
     PRV_CPSS_DXCH_UNIT_FDB_E
BASE_UNIT_ID_ONLY_TREE_END(FDB)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_CENTER)
     PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E
BASE_UNIT_ID_ONLY_TREE_END(PB_CENTER)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_COUNTER)
     PRV_CPSS_DXCH_UNIT_PB_COUNTER_E
BASE_UNIT_ID_ONLY_TREE_END(PB_COUNTER)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_SMB_WA)
    PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E,
    PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_1_E,
    PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_2_E,
    PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_3_E
BASE_UNIT_ID_ONLY_TREE_END(PB_SMB_WA)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_PKT_WR)
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_2_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_3_E
BASE_UNIT_ID_ONLY_TREE_END(PB_PKT_WR)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_CELL_RD)
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E
BASE_UNIT_ID_ONLY_TREE_END(PB_CELL_RD)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_PKT_RD)
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E,
    PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E
BASE_UNIT_ID_ONLY_TREE_END(PB_PKT_RD)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_NEXT_PTR)
    PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E,
    PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E
BASE_UNIT_ID_ONLY_TREE_END(PB_NEXT_PTR)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PB_SH_MEMO)
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E,
    PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E
BASE_UNIT_ID_ONLY_TREE_END(PB_SH_MEMO)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_SFF)
    PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_SFF)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_LMU)
    PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_LMU)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_CTSU)
    PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_CTSU_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_CTSU_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_CTSU_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_CTSU)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_BRG)
    PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_BRG_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_BRG)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_PZ_ARB)
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E,
    PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_PZ_ARB)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_MACSEC_EXT)
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_MACSEC_EXT)

/* these are MG_TAI unitis other TAI units yet not supported by CPSS */
BASE_UNIT_ID_ONLY_TREE_BEGIN(TAI)
    PRV_CPSS_DXCH_UNIT_TAI_E,
    PRV_CPSS_DXCH_UNIT_TAI1_E
BASE_UNIT_ID_ONLY_TREE_END(TAI)

BASE_UNIT_ID_ONLY_TREE_BEGIN(MG)
    PRV_CPSS_DXCH_UNIT_MG_E,
    PRV_CPSS_DXCH_UNIT_MG_0_1_E,
    PRV_CPSS_DXCH_UNIT_MG_0_2_E,
    PRV_CPSS_DXCH_UNIT_MG_0_3_E
BASE_UNIT_ID_ONLY_TREE_END(MG)

BASE_UNIT_ID_ONLY_TREE_BEGIN(CNM_RFU)
    PRV_CPSS_DXCH_UNIT_CNM_RFU_E
BASE_UNIT_ID_ONLY_TREE_END(CNM_RFU)

BASE_UNIT_ID_ONLY_TREE_BEGIN(CNM_SMI)
    PRV_CPSS_DXCH_UNIT_SMI_0_E,
    PRV_CPSS_DXCH_UNIT_SMI_1_E
BASE_UNIT_ID_ONLY_TREE_END(CNM_SMI)

BASE_UNIT_ID_ONLY_TREE_BEGIN(CNM_AAC)
    PRV_CPSS_DXCH_UNIT_CNM_AAC_E
BASE_UNIT_ID_ONLY_TREE_END(CNM_AAC)

BASE_UNIT_ID_ONLY_TREE_BEGIN(LED)
    PRV_CPSS_DXCH_UNIT_LED_0_E,
    PRV_CPSS_DXCH_UNIT_LED_1_E,
    PRV_CPSS_DXCH_UNIT_LED_2_E,
    PRV_CPSS_DXCH_UNIT_LED_3_E
BASE_UNIT_ID_ONLY_TREE_END(LED)

BASE_UNIT_ID_ONLY_TREE_BEGIN(MAC_400G)
    PRV_CPSS_DXCH_UNIT_MAC_400G_0_E,
    PRV_CPSS_DXCH_UNIT_MAC_400G_1_E,
    PRV_CPSS_DXCH_UNIT_MAC_400G_2_E,
    PRV_CPSS_DXCH_UNIT_MAC_400G_3_E
BASE_UNIT_ID_ONLY_TREE_END(MAC_400G)

BASE_UNIT_ID_ONLY_TREE_BEGIN(CPU_MAC)
    PRV_CPSS_DXCH_UNIT_MAC_CPU_0_E
BASE_UNIT_ID_ONLY_TREE_END(CPU_MAC)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCS_400G)
    PRV_CPSS_DXCH_UNIT_PCS_400G_0_E,
    PRV_CPSS_DXCH_UNIT_PCS_400G_1_E,
    PRV_CPSS_DXCH_UNIT_PCS_400G_2_E,
    PRV_CPSS_DXCH_UNIT_PCS_400G_3_E
BASE_UNIT_ID_ONLY_TREE_END(PCS_400G)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCS_CPU)
    PRV_CPSS_DXCH_UNIT_PCS_CPU_0_E
BASE_UNIT_ID_ONLY_TREE_END(PCS_CPU)

BASE_UNIT_ID_ONLY_TREE_BEGIN(MIF_400G)
    PRV_CPSS_DXCH_UNIT_MIF_400G_0_E,
    PRV_CPSS_DXCH_UNIT_MIF_400G_1_E,
    PRV_CPSS_DXCH_UNIT_MIF_400G_2_E,
    PRV_CPSS_DXCH_UNIT_MIF_400G_3_E,
    PRV_CPSS_DXCH_UNIT_MIF_USX_0_E,
    PRV_CPSS_DXCH_UNIT_MIF_USX_1_E,
    PRV_CPSS_DXCH_UNIT_MIF_CPU_0_E
BASE_UNIT_ID_ONLY_TREE_END(MIF_400G)

BASE_UNIT_ID_ONLY_TREE_BEGIN(ANP_400G)
    PRV_CPSS_DXCH_UNIT_ANP_400G_0_E,
    PRV_CPSS_DXCH_UNIT_ANP_400G_1_E,
    PRV_CPSS_DXCH_UNIT_ANP_400G_2_E,
    PRV_CPSS_DXCH_UNIT_ANP_400G_3_E,
    PRV_CPSS_DXCH_UNIT_ANP_USX_0_E,
    PRV_CPSS_DXCH_UNIT_ANP_USX_1_E,
    PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E
BASE_UNIT_ID_ONLY_TREE_END(ANP_400G)

BASE_UNIT_ID_ONLY_TREE_BEGIN(PCA_MACSEC_EIP)
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E,
    PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E
BASE_UNIT_ID_ONLY_TREE_END(PCA_MACSEC_EIP)

BASE_UNIT_ID_ONLY_TREE_BEGIN(TCAM)
    PRV_CPSS_DXCH_UNIT_TCAM_E
BASE_UNIT_ID_ONLY_TREE_END(TCAM)

BASE_UNIT_ID_ONLY_TREE_BEGIN(DFX_SERVER)
    PRV_CPSS_DXCH_UNIT_DFX_SERVER_E
BASE_UNIT_ID_ONLY_TREE_END(DFX_SERVER)

BASE_UNIT_ID_ONLY_TREE_BEGIN(SERDES)
    PRV_CPSS_DXCH_UNIT_SERDES_E
BASE_UNIT_ID_ONLY_TREE_END(SERDES)

static struct
{
    const char          *name;
    GT_U32              offsetInSubunut;
    OFFSETS_TREE_STC    *treePtr;
    HAWK_CHK_FUNC_PTR   funcPtr;
} screenig_registers_arr[] =
{
     {"LPM General interrupt mask RW",    0x00000130, &OFFSETS_TREE_NAME(LPM),      &hawk_rw_chk}
    ,{"CNC interrupt summary mask RW",    0x00000104, &OFFSETS_TREE_NAME(CNC),      &hawk_rw_chk}
    ,{"IPVX router interrupt mask RW",    0x00000974, &OFFSETS_TREE_NAME(IPVX),     &hawk_rw_chk}
    ,{"L2I bridge interrupt mask RW",     0x00002104, &OFFSETS_TREE_NAME(L2I),      &hawk_rw_chk}
    ,{"PCL unit interrupt mask RW",       0x00000008, &OFFSETS_TREE_NAME(PCL),      &hawk_rw_chk}
    ,{"EPRL policer interrupt mask RW",   0x00000204, &OFFSETS_TREE_NAME(EPLR),     &hawk_rw_chk}
    ,{"EOAM unit interrupt mask RW",      0x000000F4, &OFFSETS_TREE_NAME(EOAM),     &hawk_rw_chk}
    ,{"EPCL interrupt mask RW",           0x00000014, &OFFSETS_TREE_NAME(EPCL),     &hawk_rw_chk}
    ,{"PREQ interrupt mask RW",           0x00000604, &OFFSETS_TREE_NAME(PREQ),     &hawk_rw_chk}
    ,{"ERMRK interrupt mask RW",          0x00000008, &OFFSETS_TREE_NAME(ERMRK),    &hawk_rw_chk}
    ,{"IOAM unit interrupt mask RW",      0x000000F4, &OFFSETS_TREE_NAME(IOAM),     &hawk_rw_chk}
    ,{"MLL interrupt mask RW",            0x00000034, &OFFSETS_TREE_NAME(MLL),      &hawk_rw_chk}
    ,{"IPRL policer interrupt mask RW",   0x00000204, &OFFSETS_TREE_NAME(IPLR),     &hawk_rw_chk}
    ,{"EQ pre-egress interrupt mask RW",  0x0000005C, &OFFSETS_TREE_NAME(EQ),       &hawk_rw_chk}
    ,{"EGF_QAG interrupt mask RW",        0x00000010, &OFFSETS_TREE_NAME(EGF_QAG),  &hawk_rw_chk}
    ,{"EGF_SHT interrupt mask RW",        0x00000020, &OFFSETS_TREE_NAME(EGF_SHT),  &hawk_rw_chk}
    ,{"EGF_EFT interrupt mask RW",        0x000000B0, &OFFSETS_TREE_NAME(EGF_EFT),  &hawk_rw_chk}
    ,{"TTI Engine interrupt mask RW",     0x00000008, &OFFSETS_TREE_NAME(TTI),      &hawk_rw_chk}
    ,{"PPU interrupt mask RW",            0x00000008, &OFFSETS_TREE_NAME(PPU),      &hawk_rw_chk}
    ,{"EREP interrupt mask RW",           0x00000004, &OFFSETS_TREE_NAME(EREP),     &hawk_rw_chk}
    ,{"BMA interrupt mask RW",            0x00059004, &OFFSETS_TREE_NAME(BMA),      &hawk_rw_chk}
    ,{"IA interrupt0 mask RW",            0x00000504, &OFFSETS_TREE_NAME(IA),       &hawk_rw_chk}
    ,{"RXDMA interrupt0 mask RW",         0x00001C84, &OFFSETS_TREE_NAME(RXDMA),    &hawk_rw_chk}
    ,{"TXDMA interrupt mask RW",          0x00006004, &OFFSETS_TREE_NAME(TXDMA),    &hawk_rw_chk}
    ,{"TXFIFO interrupt mask RW",         0x00004004, &OFFSETS_TREE_NAME(TXFIFO),   &hawk_rw_chk}
    ,{"TXQ_SDQ interrupt functional mask RW", 0x0000020C, &OFFSETS_TREE_NAME(TXQ_SDQ),  &hawk_rw_chk}
    ,{"TXQ_PDS interrupt functional mask RW", 0x0004200C, &OFFSETS_TREE_NAME(TXQ_PDS),  &hawk_rw_chk}
    ,{"TXQ_QFC interrupt functional mask RW", 0x00000204, &OFFSETS_TREE_NAME(TXQ_QFC),  &hawk_rw_chk}
    ,{"TXQ_PDX interrupt functional mask RW", 0x0000140C, &OFFSETS_TREE_NAME(TXQ_PDX),  &hawk_rw_chk}
#ifndef ASIC_SIMULATION
    ,{"TXQ_PFCC interrupt mask RW",       0x0000010C, &OFFSETS_TREE_NAME(TXQ_PFCC), &hawk_rw_chk}
#endif /*ASIC_SIMULATION*/
    ,{"TXQ_PSI interrupt mask RW",        0x0000010C, &OFFSETS_TREE_NAME(TXQ_PSI),  &hawk_rw_chk}
    ,{"HA interrupt mask RW",             0x00000304, &OFFSETS_TREE_NAME(HA),       &hawk_rw_chk}
    ,{"PHA interrupt sum mask RW",        0x007FFF8C, &OFFSETS_TREE_NAME(PHA),      &hawk_rw_chk}
    ,{"SHM interrupt mask RW",            0x000000E4, &OFFSETS_TREE_NAME(SHM),      &hawk_rw_chk}
    ,{"EM interrupt mask RW",             0x00004004, &OFFSETS_TREE_NAME(EM),       &hawk_rw_chk}
    ,{"FDB interrupt mask RW",            0x000001B4, &OFFSETS_TREE_NAME(FDB),      &hawk_rw_chk}
    ,{"PB_CENTER interrupt mask RW",      0x00001104, &OFFSETS_TREE_NAME(PB_CENTER),   &hawk_rw_chk}
    ,{"PB_COUNTER interrupt mask RW",     0x00000194, &OFFSETS_TREE_NAME(PB_COUNTER),  &hawk_rw_chk}
    ,{"PB_SMB_WA interrupt sum mask RW",  0x00000114, &OFFSETS_TREE_NAME(PB_SMB_WA),   &hawk_rw_chk}
    ,{"PB_PKT_WR interrupt mask RW",      0x00000204, &OFFSETS_TREE_NAME(PB_PKT_WR),   &hawk_rw_chk}
    ,{"PB_CELL_RD interrupt mask RW",     0x000000A4, &OFFSETS_TREE_NAME(PB_CELL_RD),  &hawk_rw_chk}
    ,{"PB_PKT_RD interrupt misc mask RW", 0x0000150C, &OFFSETS_TREE_NAME(PB_PKT_RD),   &hawk_rw_chk}
    ,{"PB_NEXT_PTR gen interrupt mask RW",0x0000010C, &OFFSETS_TREE_NAME(PB_NEXT_PTR), &hawk_rw_chk}
    ,{"PB_SH_MEMO interrupt mask RW",     0x00000104, &OFFSETS_TREE_NAME(PB_SH_MEMO),  &hawk_rw_chk}
    ,{"PCA_SFF global interrupt mask RW", 0x00000014, &OFFSETS_TREE_NAME(PCA_SFF),  &hawk_rw_chk}
    ,{"PCA_LMU global interrupt mask RW", 0x00008224, &OFFSETS_TREE_NAME(PCA_LMU),  &hawk_rw_chk}
    ,{"PCA_CTSU global interrupt mask RW",0x00000084, &OFFSETS_TREE_NAME(PCA_CTSU), &hawk_rw_chk}
    ,{"PCA_BRG interrupt mask RW",        0x00000008, &OFFSETS_TREE_NAME(PCA_BRG),  &hawk_rw_chk}
    ,{"PCA_PZ_ARB BadAddr interrupt mask RW", 0x00000B08, &OFFSETS_TREE_NAME(PCA_PZ_ARB), &hawk_rw_chk}
    ,{"PCA_MACSEC_EXT interrupt mask RW", 0x00000334, &OFFSETS_TREE_NAME(PCA_MACSEC_EXT), &hawk_rw_chk}
    ,{"PCA_MACSEC_EIP Tcaqm Key0[0] RW",  0x00000000, &OFFSETS_TREE_NAME(PCA_MACSEC_EIP), &hawk_rw_chk}
    ,{"TAI interrupt mask RW",            0x00000004, &OFFSETS_TREE_NAME(TAI),      &hawk_rw_chk}
    ,{"MG global interrupt Sum mask RW",  0x00000034, &OFFSETS_TREE_NAME(MG),       &hawk_rw_chk}
    ,{"CNM_RFU interrupt mask RW",        0x00000018, &OFFSETS_TREE_NAME(CNM_RFU),  &hawk_rw_chk}
    ,{"CNM_SMI Misc Configurations RW",   0x00000004, &OFFSETS_TREE_NAME(CNM_SMI),  &hawk_rw_chk}
    ,{"CNM_AAC interrupt mask RW",        0x00000014, &OFFSETS_TREE_NAME(CNM_AAC),  &hawk_rw_chk}
    ,{"LED interrupt mask RW",            0x00000204, &OFFSETS_TREE_NAME(LED),      &hawk_rw_chk}
    ,{"MAC_400G interrupt mask RW",       0x00000038, &OFFSETS_TREE_NAME(MAC_400G), &hawk_rw_chk}
    ,{"CPU_MAC interrupt mask RW",        0x00000010, &OFFSETS_TREE_NAME(CPU_MAC),  &hawk_rw_chk}
    ,{"PCS_400G LPCS GSTATUS RW",         0x0000B3E4, &OFFSETS_TREE_NAME(PCS_400G), &hawk_rw_chk}
    ,{"PCS_CPU PORT STATUS RW",           0x00000004, &OFFSETS_TREE_NAME(PCS_CPU),  &hawk_rw_chk}
    ,{"MIF_400G global interrupt Sum mask RW",  0x00000124, &OFFSETS_TREE_NAME(MIF_400G), &hawk_rw_chk}
    ,{"ANP_400G interrupt Sum mask RW",   0x00000028, &OFFSETS_TREE_NAME(ANP_400G), &hawk_rw_chk}
    ,{"TCAM interrupt mask RW",           0x00001008, &OFFSETS_TREE_NAME(TCAM),     &hawk_rw_chk}
    ,{"DFX_SERVER interrupt mask RW",     0x000F810C, &OFFSETS_TREE_NAME(DFX_SERVER), &hawk_rw_chk}
    ,{"SERDES interrupt mask RW",         0x00000304, &OFFSETS_TREE_NAME(SERDES),     &hawk_rw_chk}
    ,{NULL, 0, NULL, NULL}
};

/**
* @internal hawk_screening_registers function
* @endinternal
*
* @brief  Tests several registers in Hawk.
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @param[in] skipRegisters         - amount of registers to skip - needed to find the next stuck
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS hawk_screening_registers
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           afterPrintSleep,
    IN  GT_U32           skipRegisters
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U32    i;

    for (i = 0; (screenig_registers_arr[i].funcPtr != NULL); i++)
    {
        if (i < skipRegisters) continue;
        if (afterPrintSleep) cpssOsPrintf("============================= ");
        cpssOsPrintf("[%d] Testing %s", i, screenig_registers_arr[i].name);
        if (afterPrintSleep) cpssOsPrintf(" =============================");
        cpssOsPrintf("\n");
        rc = hawk_tree_check(
            devNum,
            (screenig_registers_arr[i].offsetInSubunut),
            screenig_registers_arr[i].treePtr,
            screenig_registers_arr[i].funcPtr,
            afterPrintSleep);
        if (rc != GT_OK)
        {
            cpssOsPrintf("=============== Test failed =============\n");
            rc1 = GT_FAIL;
        }
    }

    return rc1;
}
#if 0
static struct
{
    const char          *name;
    GT_U32              offsetInSubunut;
    OFFSETS_TREE_STC    *treePtr;
    GT_U32              pattern;
} screenig_registers_patterns_arr[] =
{
     {"SERDES interrupt mask RW",         0x00000304, &OFFSETS_TREE_NAME(SERDES),     0x00000000}
    ,{NULL, 0, NULL, NULL}
};
#endif

/*===============================================================*/

#define MAX_TABLE_ENTRY_WORDS_CNS 80

/**
* @internal hawk_table_entry0_pattern_check function
* @endinternal
*
* @brief   check entry0 of the given table
*
* @param[in] devNum                - CPSS devNum.
* @param[in] portGroupId           - Port Group Id.
* @param[in] tableId               - Table Id.
* @param[in] wordsInEntry          - Amount of 32-bit words in table entry.
* @param[in] checkedBitsBitmaskPtr - pointer to bitmask of checked bits, can be NULL.
*
* @retval GT_OK                    - on success,
* @retval other                    - otherwise.
*
*/
static GT_STATUS hawk_table_entry0_pattern_check
(
    IN  GT_SW_DEV_NUM        devNum,
    IN GT_U32                portGroupId,
    IN  CPSS_DXCH_TABLE_ENT  tableId,
    IN  GT_U32               wordsInEntry,
    IN  GT_U32               *checkedBitsBitmaskPtr
)
{
    GT_STATUS rc;
    static GT_U32  entry[MAX_TABLE_ENTRY_WORDS_CNS];
    static GT_U32  pattern[] = {0xFFFFFFFF, 0, 0x55555555, 0xAAAAAAAA};
    static GT_U32  patternSize = (sizeof(pattern) / sizeof(pattern[0]));
    GT_U32    patternNdx;
    GT_U32    i;
    GT_U32    bitsInLastWordByOnes;
    GT_U32    bitsInLastWordByZeros;
    GT_U32    bitsInLastWord;
    GT_U32    lastWordMask;
    GT_U32    entryIndex, startBit ,numBitsPerEntry, numEntriesPerLine;

    entryIndex         = 0;
    startBit           = 0;
    numBitsPerEntry    = 0;
    numEntriesPerLine  = 0;
    if (GT_FALSE != prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(
        devNum, tableId, &entryIndex, &startBit, &numBitsPerEntry, &numEntriesPerLine))
    {
        #if 0
        cpssOsPrintf(
            "entryIndex %d startBit %d numBitsPerEntry %d numEntriesPerLine 0x%08X \n",
            entryIndex, startBit ,numBitsPerEntry, numEntriesPerLine);
        #endif
        /* recalculate wordsInEntry */
        wordsInEntry = ((31 + numBitsPerEntry) / 32);
    }

    lastWordMask          = 0;
    bitsInLastWordByOnes  = 0;
    bitsInLastWordByZeros = 0;
    for (patternNdx = 0; (patternNdx < patternSize); patternNdx++)
    {
        /* write pattern */
        for (i = 0; (i < wordsInEntry); i++)
        {
            entry[i] = pattern[patternNdx];
        }
        rc = prvCpssDxChPortGroupWriteTableEntry(
            devNum, portGroupId, tableId, 0/*entryIndex*/, entry);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Write failed, rc %d \n", rc);
            return rc;
        }
        /* clear buffer and read pattern */
        for (i = 0; (i < wordsInEntry); i++)
        {
            entry[i] = 0;
        }
        rc = prvCpssDxChPortGroupReadTableEntry(
            devNum, portGroupId, tableId, 0/*entryIndex*/, entry);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Read failed, rc %d \n", rc);
            return rc;
        }
        /* when checkedBitsBitmaskPtr given just use it */
        if (checkedBitsBitmaskPtr)
        {
            for (i = 0; (i < wordsInEntry); i++)
            {
                if ((entry[i] & checkedBitsBitmaskPtr[i])
                    != (pattern[patternNdx] & checkedBitsBitmaskPtr[i]))
                {
                    cpssOsPrintf(
                        "Entry word %d written 0x%08X read 0x%08X checkedBitsBitmask 0x%08X\n",
                        i, pattern[patternNdx], entry[i], checkedBitsBitmaskPtr[i]);
                    return GT_FAIL;
                }
            }
            continue;
        }
        /* when checkedBitsBitmaskPtr == NULL guess the amount bits in the last word */

        /* check patterns in not last words */
        for (i = 0; ((i + 1) < wordsInEntry); i++)
        {
            if (entry[i] != pattern[patternNdx])
            {
                cpssOsPrintf(
                    "Entry word %d written 0x%08X read 0x%08X\n",
                    i, pattern[patternNdx], entry[i]);
                return GT_FAIL;
            }
        }
        if ((lastWordMask & pattern[patternNdx]) != (lastWordMask & entry[wordsInEntry - 1]))
        {
            /* initially lastWordMask == 0, set after patterns 0xFFFFFFFF and 0 */
            cpssOsPrintf(
                "Entry last word mask 0x%08X written 0x%08X read 0x%08X\n",
                lastWordMask, pattern[patternNdx], entry[wordsInEntry - 1]);
            return GT_FAIL;
        }
        if (pattern[patternNdx] == 0xFFFFFFFF)
        {
            /* calculate bitsInLastWordByOnes */
            for (i = 0; ((i < 32) && ((entry[wordsInEntry - 1] & (1 << i)) != 0)); i++) {}
            bitsInLastWordByOnes = i;
        }
        if (pattern[patternNdx] == 0)
        {
            /* calculate bitsInLastWordByZeros */
            for (i = 0; ((i < 32) && ((entry[wordsInEntry - 1] & (1 << i)) == 0)); i++) {}
            bitsInLastWordByZeros = i;
        }
        if ((lastWordMask == 0) && (bitsInLastWordByOnes != 0) && (bitsInLastWordByZeros != 0))
        {
            bitsInLastWord =
                (bitsInLastWordByOnes <= bitsInLastWordByZeros)
                ? bitsInLastWordByOnes : bitsInLastWordByZeros;
            if (bitsInLastWord >= 32)
                lastWordMask = 0xFFFFFFFF;
            else
                lastWordMask = (1 << bitsInLastWord) - 1;
        }
    }
    return GT_OK;
}


#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32 prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
static const char* getTableNamebyEnum
(
    IN   CPSS_DXCH_TABLE_ENT tableId,
    OUT  GT_U32              *intancePtr
)
{
    GT_U32              ii;
    CPSS_DXCH_TABLE_ENT currentTableId = 0;
    GT_U32              maxLessFoundIndex = 0;

    for (ii = 0 ; ii < prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT; ii++)
    {
        currentTableId = prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[ii].enumValue;
        if (currentTableId == tableId)
        {
            *intancePtr = 0;
            return prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[ii].namePtr;
        }
        if ((currentTableId < tableId)
            && (prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[maxLessFoundIndex].enumValue < (GT_32)currentTableId))
        {
            maxLessFoundIndex = ii;
        }
    }
    *intancePtr = (tableId - prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[maxLessFoundIndex].enumValue);
    return prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[maxLessFoundIndex].namePtr;
}
#else
static const char* getTableNamebyEnum
(
    IN   CPSS_DXCH_TABLE_ENT tableId,
    OUT  GT_U32              *intancePtr
)
{
    GT_UNUSED_PARAM(tableId);
    *intancePtr = 0;
    return "unknown";
}
#endif /* CPSS_LOG_ENABLE */

/*
CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E
CPSS_DXCH3_TABLE_MAC2ME_E

*/

typedef struct
{
    CPSS_DXCH_TABLE_ENT           startTableId;
    GT_U32                        rangeSize;
} TABLE_ID_RANGE_STC;

typedef struct
{
    CPSS_DXCH_TABLE_ENT           startTableId;
    GT_U32                        rangeSize;
    GT_U32                        twoWordBitmask[2];
    GT_U32                        *bitmaskPtr; /* used only if twoWordBitmask == {0,0} */
} TABLE_ID_RANGE_RW_BITMASK_STC;

/* table entry is full or partially updated not by CPU */
static TABLE_ID_RANGE_STC bypassed_not_cpu_updated_tables[] =
{
     { CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E, 32} /* read only */
     ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E, 32} /* read only */
     ,{CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E, 32}
     ,{CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E, 32}
     ,{CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E, 32}
     ,{CPSS_DXCH_TABLE_LAST_E, 0/*end*/}
};

/* tables with access configured only on full System initialization */
/* shared memory TS, FDB, ExactMatch, LPM */
static TABLE_ID_RANGE_STC bypassed_full_system_init_tables[] =
{
    {CPSS_DXCH_TABLE_FDB_E, 1}
    ,{CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E, 1}
    ,{CPSS_DXCH_SIP5_TABLE_LPM_MEM_E, 1}
    ,{CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E, 1}
    ,{CPSS_DXCH_TABLE_LAST_E, 0/*end*/}
};

/* tables with yet not explained problems  */
static TABLE_ID_RANGE_STC bypassed_problem_tables[] =
{
    {CPSS_DXCH_TABLE_LAST_E, 0/*end*/}
};

static GT_U32 isTableFound(TABLE_ID_RANGE_STC *listPtr, CPSS_DXCH_TABLE_ENT tableId)
{
    GT_U32              i;
    CPSS_DXCH_TABLE_ENT low;
    CPSS_DXCH_TABLE_ENT high;

    for (i = 0; (listPtr[i].rangeSize != 0); i++)
    {
        low   = listPtr[i].startTableId;
        high  = low + listPtr[i].rangeSize - 1;
        if (tableId < low) continue;
        if (tableId > high) continue;
        return 1;
    }
    return 0;
}

static GT_U32* tableBitmaskGet(
     TABLE_ID_RANGE_RW_BITMASK_STC *listPtr, CPSS_DXCH_TABLE_ENT tableId)
{
    GT_U32              i;
    CPSS_DXCH_TABLE_ENT low;
    CPSS_DXCH_TABLE_ENT high;

    for (i = 0; (listPtr[i].rangeSize != 0); i++)
    {
        low   = listPtr[i].startTableId;
        high  = low + listPtr[i].rangeSize - 1;
        if (tableId < low) continue;
        if (tableId > high) continue;
        if (listPtr[i].twoWordBitmask[0] != 0) return &(listPtr[i].twoWordBitmask[0]);
        if (listPtr[i].twoWordBitmask[1] != 0) return &(listPtr[i].twoWordBitmask[0]);
        return listPtr[i].bitmaskPtr;
    }
    return NULL;
}

/* table has reseved bits */
static GT_U32 mac2meTableMaskArr[] =
    {0xFFFF0FFF, 0xFFFFFFFF, 0xFFFF0FFF, 0xFFFFFFFF, 0x03FFFFFF, 0x3};
static GT_U32 preqQueDpCfgTableMaskArr[] = {0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF, 0};
static GT_U32 preqQueCfgTableMaskArr[] = {0xFFFFFFFF, 0xFFFFFFFF, 0x3, 0};
static GT_U32 preqPortProfileTableMaskArr[] = {0xFFFFFFFF, 0xFFFFFFFF, 0x0FFFFFFF, 0};

static TABLE_ID_RANGE_RW_BITMASK_STC masked_tables[] =
{
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E, 1,  {0x3FFFFFFF, 0}, NULL}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E, 1,  {0x3FFFFFFF, 0}, NULL}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E, 1,  {0x1FFF, 0}, NULL}
    ,{CPSS_DXCH3_TABLE_MAC2ME_E, 1, {0, 0}, mac2meTableMaskArr}
    ,{CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E, 1, {0, 0}, preqQueDpCfgTableMaskArr}
    ,{CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E, 1, {0, 0}, preqQueCfgTableMaskArr}
    ,{CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E, 1, {0, 0}, preqPortProfileTableMaskArr}
    ,{CPSS_DXCH_TABLE_LAST_E, 0/*end*/, {0, 0}, NULL}
};

#if 0
static TABLE_ID_RANGE_STC absolute_addressed_tables[] =
{
    /* all LMU tables */
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E, CPSS_DXCH_SIP6_MAX_GOP_LMU_STATISTICS_MAC}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E, CPSS_DXCH_SIP6_MAX_GOP_LMU_CONFIGURATION_MAC}
    /* all TXQ tables */
    ,{CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PORT_DESC_COUNTER_E,
        (CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_LAST_E + 1
         - CPSS_DXCH_SIP6_TXQ_PDX_PAC_0_PORT_DESC_COUNTER_E)}
    ,{CPSS_DXCH_TABLE_LAST_E, 0/*end*/}
};

static TABLE_ID_RANGE_STC per_tile_tables[] =
{
    {CPSS_DXCH_TABLE_FDB_E, 1}
    ,{CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E, 1}
    ,{CPSS_DXCH_SIP5_TABLE_LPM_MEM_E, 1}
    ,{CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E, 1}
    ,{CPSS_DXCH_SIP5_TABLE_TCAM_E, 1}
    ,{CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E, 1}
    ,{CPSS_DXCH_TABLE_LAST_E, 0/*end*/}
};
#endif
/**
* @internal hawk_screening_tables function
* @endinternal
*
* @brief  Tests several tables in Hawk.
*
* @param[in] devNum                   - The CPSS devNum.
* @param[in] afterPrintSleep          - Time to sleep after printing to see last printed line
*                                       before stucking.
* @param[in] includeBypassedTableBmp  - test special tables
*                                       1 - tables with yet not explaned behavior.
*                                       2 - tables in shared memory.
*                                       4 - tables updated not by CPU (having read only fields).
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS hawk_screening_tables
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           afterPrintSleep,
    IN  GT_U32           includeBypassedTableBmp
)
{
    GT_STATUS                     rc;
    GT_STATUS                     rc1 = GT_OK;
    CPSS_DXCH_TABLE_ENT           tableId;
    CPSS_DXCH_TABLE_ENT           maxTableId;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr;
    const char*                   tableName;
    GT_U32                        tableInstance;
    GT_U32                        portGroupId;
    GT_U32                        portGroupIdStart;
    GT_U32                        portGroupIdStep;
    GT_U32                        portGroupIdMax;
    static GT_U32                 saveEntry[MAX_TABLE_ENTRY_WORDS_CNS];
    GT_U32*                       tableBitmaskPtr;

    maxTableId = PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum);
    for (tableId = 0; (tableId < maxTableId); tableId ++)
    {
        if ((includeBypassedTableBmp & 1) == 0)
        {
            if (isTableFound(bypassed_problem_tables, tableId)) continue;
        }
        if ((includeBypassedTableBmp & 2) == 0)
        {
            if (isTableFound(bypassed_full_system_init_tables, tableId)) continue;
        }
        if ((includeBypassedTableBmp & 4) == 0)
        {
            if (isTableFound(bypassed_not_cpu_updated_tables, tableId)) continue;
        }

        tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableId);
        if (tableInfoPtr->maxNumOfEntries == 0) continue;
        if (tableInfoPtr->readTablePtr == 0) continue;
        if (tableInfoPtr->readAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
        {
            PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr =
                (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);
            /* wrong base address */
            if (tablePtr->baseAddress & 3) continue;
        }

        tableName    = getTableNamebyEnum(tableId, &tableInstance);
        if (tableInfoPtr->entrySize > MAX_TABLE_ENTRY_WORDS_CNS)
        {
            cpssOsPrintf(
                "Table entry size %d words too big %s(%d)",
                tableInfoPtr->entrySize, tableName, tableInstance);
            continue;
        }

        if (tableId == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E)
        {
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum == 0)
            {
                continue;
            }
        }

        cpssOsPrintf(
            "id %d entries %d wordsInEntry %d instance %d name %s\n",
            tableId, tableInfoPtr->maxNumOfEntries,
            tableInfoPtr->entrySize, tableInstance, tableName);
        /*default*/
        portGroupIdStart = 0;
        portGroupIdMax   = 1;
        portGroupIdStep  = 1; /* unit per pipe */
        for (portGroupId = portGroupIdStart;
              (portGroupId < portGroupIdMax); portGroupId += portGroupIdStep)
        {
            if (afterPrintSleep != 0) cpssOsPrintf("portGroup %d\n", portGroupId);
            if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
            rc = prvCpssDxChPortGroupReadTableEntry(
                devNum, portGroupId, tableId, 0/*entryIndex*/, saveEntry);
            if (rc != GT_OK)
            {
                cpssOsPrintf("Read failed, rc %d \n", rc);
                rc1 = rc;
                continue;
            }

            /* check if write applicable */
            if (tableInfoPtr->writeTablePtr == NULL) continue;
            if (tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
            {
                PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr =
                    (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);
                /* wrong base address */
                if (tablePtr->baseAddress & 3) continue;
            }

            tableBitmaskPtr = tableBitmaskGet(masked_tables, tableId);

            /* call testing function */
            rc = hawk_table_entry0_pattern_check(
                devNum, portGroupId, tableId, tableInfoPtr->entrySize, tableBitmaskPtr);
            if (rc != GT_OK) {rc1 = rc;} /* called function prints errors */

            /* write saved data back */
            rc = prvCpssDxChPortGroupWriteTableEntry(
                devNum, portGroupId, tableId, 0/*entryIndex*/, saveEntry);
            if (rc != GT_OK)
            {
                cpssOsPrintf("Write failed, rc %d \n", rc);
                rc1 = rc;
                continue;
            }
        }
    }

    return rc1;
}

/**
* @internal hawk_all_screening function
* @endinternal
*
* @brief  Tests several registers and tables in Hawk.
*
* @param[in] devNum                   - The CPSS devNum.
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS hawk_all_screening
(
    IN  GT_SW_DEV_NUM    devNum
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;

    rc = hawk_screening_registers(
        devNum, 0/*afterPrintSleep*/, 0/*skipRegisters*/);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Hawk Registers OK\n");
    }
    else
    {
        cpssOsPrintf("Hawk Registers test FAILED\n");
        rc1 = GT_FAIL;
    }
#if 0 /* hawk_screening_tables yet not adapted */
    rc = hawk_screening_tables(
        devNum, 0/*afterPrintSleep*/, 0/*includeBypassedTableBmp*/);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Hawk Tables OK\n");
    }
    else
    {
        cpssOsPrintf("Hawk Tables test FAILED\n");
        rc1 = GT_FAIL;
    }
#endif /*0*/
    return rc1;
}

/**
* @internal hawk_screening function
* @endinternal
*
* @brief  Tests several registers and tables in all Hawk devices.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS hawk_screening
(
    GT_VOID
)
{
    GT_SW_DEV_NUM    devNum;
    GT_STATUS        rc;
    GT_STATUS        rc1 = GT_OK;

    for (devNum = 0; (devNum < PRV_CPSS_MAX_PP_DEVICES_CNS); devNum++)
    {
        if (PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0) continue;
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5P_E) continue;
        cpssOsPrintf("Hawk devNum %d test STARTED\n", devNum);

        rc = hawk_all_screening(devNum);
        if (rc == GT_OK)
        {
            cpssOsPrintf("Hawk devNum %d test OK\n", devNum);
        }
        else
        {
            cpssOsPrintf("Hawk devNum %d test FAILED\n", devNum);
            rc1 = rc;
        }
    }
    return rc1;
}

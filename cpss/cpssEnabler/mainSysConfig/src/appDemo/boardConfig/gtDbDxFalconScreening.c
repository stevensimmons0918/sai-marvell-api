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
* @file gtDbDxFalconScreening.c
*
* @brief Utilities for screening Falcon boards.
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
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*============================================================================================*/

/**
* @internal falcon_ro_chk function
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
static GT_STATUS falcon_ro_chk
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
)
{
    GT_STATUS rc;
    GT_U32    data = 0;

    if (afterPrintSleep != 0) cpssOsPrintf("falcon_ro_chk regAddr 0x%08X \n", regAddr);
    if (afterPrintSleep != 0) osTimerWkAfter(afterPrintSleep);
    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, 0 /*portGroupId*/, regAddr, &data);
    if (afterPrintSleep != 0) cpssOsPrintf("read done data 0x%08X rc %d \n", data, rc);
    return rc;
}

/**
* @internal falcon_rw_chk function
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
static GT_STATUS falcon_rw_chk
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U32    data = 0;

    if (afterPrintSleep != 0) cpssOsPrintf("falcon_rw_chk regAddr 0x%08X \n", regAddr);
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
* @internal falcon_rw_pattern function
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
static GT_STATUS falcon_rw_pattern
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

    if (afterPrintSleep != 0) cpssOsPrintf("falcon_rw_pattern regAddr 0x%08X \n", regAddr);
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

/* arrays for one tile */

typedef GT_STATUS (*FALCON_CHK_FUNC_PTR)
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           regAddr,
    IN  GT_U32           afterPrintSleep
);

typedef struct
{
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
* @internal falcon_tree_check function
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
static GT_STATUS falcon_tree_check
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              offsetInSubunut,
    IN  OFFSETS_TREE_STC    *treePtr,
    IN  FALCON_CHK_FUNC_PTR funcPtr,
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
            regAddr += treePtr->offsetsArrsPtrArrPtr[i]->offsetsArrPtr[index[i]];
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
        &(_OFFSETS_ARR_arr_##_name[0]),                               \
        (sizeof(_OFFSETS_ARR_arr_##_name) / sizeof(GT_U32))           \
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

/* all arrays for one tile */

OFFSETS_ARR_DEF_BEGIN(null)
    0
OFFSETS_ARR_DEF_END(null)

OFFSETS_ARR_DEF_BEGIN(raven)
    0, 0x01000000, 0x02000000, 0x03000000
OFFSETS_ARR_DEF_END(raven)

OFFSETS_ARR_DEF_BEGIN(raven_tsu0)
    0x00405000, 0x00485000
OFFSETS_ARR_DEF_END(raven_tsu0)

OFFSETS_ARR_DEF_BEGIN(raven_10_steps)
    0, 0x4000, 0x8000, 0xC000, 0x10000, 0x14000, 0x18000, 0x1C000, 0x20000, 0x24000
OFFSETS_ARR_DEF_END(raven_10_steps)

OFFSETS_ARR_DEF_BEGIN(raven_lmu)
    0x00430000, 0x004B0000
OFFSETS_ARR_DEF_END(raven_lmu)

OFFSETS_ARR_DEF_BEGIN(raven_mpf)
    0x00406000, 0x00486000
OFFSETS_ARR_DEF_END(raven_mpf)

OFFSETS_ARR_DEF_BEGIN(raven_mtip_ext)
    0x00478000, 0x004F8000
OFFSETS_ARR_DEF_END(raven_mtip_ext)

OFFSETS_TREE_DEF_BEGIN(null)
    &OFFSETS_ARR_NAME(null)
OFFSETS_TREE_DEF_END(null)

OFFSETS_TREE_DEF_BEGIN(raven)
    &OFFSETS_ARR_NAME(raven)
OFFSETS_TREE_DEF_END(raven)

OFFSETS_TREE_DEF_BEGIN(raven_tsu)
    &OFFSETS_ARR_NAME(raven), &OFFSETS_ARR_NAME(raven_tsu0), &OFFSETS_ARR_NAME(raven_10_steps)
OFFSETS_TREE_DEF_END(raven_tsu)

OFFSETS_TREE_DEF_BEGIN(raven_lmu)
    &OFFSETS_ARR_NAME(raven), &OFFSETS_ARR_NAME(raven_lmu)
OFFSETS_TREE_DEF_END(raven_lmu)

OFFSETS_TREE_DEF_BEGIN(raven_mpf)
    &OFFSETS_ARR_NAME(raven), &OFFSETS_ARR_NAME(raven_mpf), &OFFSETS_ARR_NAME(raven_10_steps)
OFFSETS_TREE_DEF_END(raven_mpf)

OFFSETS_TREE_DEF_BEGIN(raven_mtip_ext)
    &OFFSETS_ARR_NAME(raven), &OFFSETS_ARR_NAME(raven_mtip_ext)
OFFSETS_TREE_DEF_END(raven_mtip_ext)

OFFSETS_ARR_DEF_BEGIN(control_pipe)
    /*0x08000000, 0x10000000 real bases */
    0, 0x08000000 /* bases decreased by 0x08000000 included in addresses in CIDER */
OFFSETS_ARR_DEF_END(control_pipe)

OFFSETS_TREE_DEF_BEGIN(control_pipe)
    &OFFSETS_ARR_NAME(control_pipe)
OFFSETS_TREE_DEF_END(control_pipe)

OFFSETS_ARR_DEF_BEGIN(cnc_subunits)
    0, 0x40000
OFFSETS_ARR_DEF_END(cnc_subunits)

OFFSETS_TREE_DEF_BEGIN(cnc)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(cnc_subunits)
OFFSETS_TREE_DEF_END(cnc)

#ifndef ASIC_SIMULATION
OFFSETS_ARR_DEF_BEGIN(cp_tai_subunits)
    0, 0x10000
OFFSETS_ARR_DEF_END(cp_tai_subunits)

OFFSETS_TREE_DEF_BEGIN(cp_tai)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(cp_tai_subunits)
OFFSETS_TREE_DEF_END(cp_tai)
#endif /*ASIC_SIMULATION*/

OFFSETS_ARR_DEF_BEGIN(rx_dma_subunits)
    0, 0x10000, 0x20000, 0x30000
OFFSETS_ARR_DEF_END(rx_dma_subunits)

OFFSETS_TREE_DEF_BEGIN(rx_dma)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(rx_dma_subunits)
OFFSETS_TREE_DEF_END(rx_dma)

OFFSETS_ARR_DEF_BEGIN(tx_dma_subunits)
    0, 0x20000, 0x40000, 0x60000
OFFSETS_ARR_DEF_END(tx_dma_subunits)

OFFSETS_TREE_DEF_BEGIN(tx_dma)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(tx_dma_subunits)
OFFSETS_TREE_DEF_END(tx_dma)

OFFSETS_ARR_DEF_BEGIN(tx_fifo_subunits)
    0, 0x20000, 0x40000, 0x60000
OFFSETS_ARR_DEF_END(tx_fifo_subunits)

OFFSETS_TREE_DEF_BEGIN(tx_fifo)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(tx_fifo_subunits)
OFFSETS_TREE_DEF_END(tx_fifo)

#ifndef ASIC_SIMULATION
OFFSETS_ARR_DEF_BEGIN(txq_tai_subunits)
    0, 0x100000
OFFSETS_ARR_DEF_END(txq_tai_subunits)

OFFSETS_TREE_DEF_BEGIN(txq_tai)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(txq_tai_subunits)
OFFSETS_TREE_DEF_END(txq_tai)
#endif /*ASIC_SIMULATION*/

OFFSETS_ARR_DEF_BEGIN(txq_pds_subunits)
    0, 0x040000, 0x100000, 0x140000
OFFSETS_ARR_DEF_END(txq_pds_subunits)

OFFSETS_TREE_DEF_BEGIN(txq_pds)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(txq_pds_subunits)
OFFSETS_TREE_DEF_END(txq_pds)

OFFSETS_ARR_DEF_BEGIN(txq_qfc_subunits)
    0, 0x010000, 0x100000, 0x110000
OFFSETS_ARR_DEF_END(txq_qfc_subunits)

OFFSETS_TREE_DEF_BEGIN(txq_qfc)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(txq_qfc_subunits)
OFFSETS_TREE_DEF_END(txq_qfc)

OFFSETS_ARR_DEF_BEGIN(txq_sdq_subunits)
    0, 0x020000, 0x100000, 0x120000
OFFSETS_ARR_DEF_END(txq_sdq_subunits)

OFFSETS_TREE_DEF_BEGIN(txq_sdq)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(txq_sdq_subunits)
OFFSETS_TREE_DEF_END(txq_sdq)

OFFSETS_ARR_DEF_BEGIN(d2d_pcs_subunits)
    0, 0x10000, 0x20000, 0x30000
OFFSETS_ARR_DEF_END(d2d_pcs_subunits)

OFFSETS_TREE_DEF_BEGIN(d2d_pcs)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(d2d_pcs_subunits)
OFFSETS_TREE_DEF_END(d2d_pcs)

OFFSETS_ARR_DEF_BEGIN(d2d_pma_subunits)
    0, 0x10000, 0x20000, 0x30000
OFFSETS_ARR_DEF_END(d2d_pma_subunits)

OFFSETS_TREE_DEF_BEGIN(d2d_pma)
    &OFFSETS_ARR_NAME(control_pipe), &OFFSETS_ARR_NAME(d2d_pma_subunits)
OFFSETS_TREE_DEF_END(d2d_pma)

#ifndef ASIC_SIMULATION
OFFSETS_ARR_DEF_BEGIN(pb_gpc_grp_cell_read_subunits)
    0, 0x10000, 0x60000, 0x70000
OFFSETS_ARR_DEF_END(pb_gpc_grp_cell_read_subunits)

OFFSETS_TREE_DEF_BEGIN(pb_gpc_grp_cell_read)
    &OFFSETS_ARR_NAME(pb_gpc_grp_cell_read_subunits)
OFFSETS_TREE_DEF_END(pb_gpc_grp_cell_read)

OFFSETS_ARR_DEF_BEGIN(pb_gpc_grp_packet_read_subunits)
    0, 0x10000, 0x60000, 0x70000
OFFSETS_ARR_DEF_END(pb_gpc_grp_packet_read_subunits)

OFFSETS_TREE_DEF_BEGIN(pb_gpc_grp_packet_read)
    &OFFSETS_ARR_NAME(pb_gpc_grp_packet_read_subunits)
OFFSETS_TREE_DEF_END(pb_gpc_grp_packet_read)

OFFSETS_ARR_DEF_BEGIN(pb_gpc_grp_packet_write_subunits)
    0, 0x60000
OFFSETS_ARR_DEF_END(pb_gpc_grp_packet_write_subunits)

OFFSETS_TREE_DEF_BEGIN(pb_gpc_grp_packet_write)
    &OFFSETS_ARR_NAME(pb_gpc_grp_packet_write_subunits)
OFFSETS_TREE_DEF_END(pb_gpc_grp_packet_write)

OFFSETS_ARR_DEF_BEGIN(pb_npm_mc_subunits)
    0, 0x10000, 0x20000
OFFSETS_ARR_DEF_END(pb_npm_mc_subunits)

OFFSETS_TREE_DEF_BEGIN(pb_npm_mc)
    &OFFSETS_ARR_NAME(pb_npm_mc_subunits)
OFFSETS_TREE_DEF_END(pb_npm_mc)

OFFSETS_ARR_DEF_BEGIN(pb_smb_mc_subunits)
    0, 0x10000, 0x20000, 0x30000, 0x40000, 0x50000
OFFSETS_ARR_DEF_END(pb_smb_mc_subunits)

OFFSETS_TREE_DEF_BEGIN(pb_smb_mc)
    &OFFSETS_ARR_NAME(pb_smb_mc_subunits)
OFFSETS_TREE_DEF_END(pb_smb_mc)
#endif /*ASIC_SIMULATION*/

OFFSETS_ARR_DEF_BEGIN(txq_psi_pdx_pac_subunits)
    0, 0x10000
OFFSETS_ARR_DEF_END(txq_psi_pdx_pac_subunits)

OFFSETS_TREE_DEF_BEGIN(txq_psi_pdx_pac)
    &OFFSETS_ARR_NAME(txq_psi_pdx_pac_subunits)
OFFSETS_TREE_DEF_END(txq_psi_pdx_pac)

static struct
{
    const char          *name;
    GT_U32              tileFilter; /* bitmap of not relevant tiles */
    GT_U32              offsetInSubunut;
    OFFSETS_TREE_STC    *treePtr;
    FALCON_CHK_FUNC_PTR funcPtr;
} screenig_registers_arr[] =
{
     {"Raven D2D_CP Metal Fix RO",0,      0x2F0070, &OFFSETS_TREE_NAME(raven),      &falcon_ro_chk}
    ,{"Raven D2D_CP Metal Fix RW",0,      0x2F0070, &OFFSETS_TREE_NAME(raven),      &falcon_rw_chk}
    ,{"Raven D2D_CP Metal Fix PATTERN",0, 0x2F0070, &OFFSETS_TREE_NAME(raven),      &falcon_rw_pattern}
    ,{"Raven TSU Metal Fix",0,              0x0104, &OFFSETS_TREE_NAME(raven_tsu),  &falcon_rw_pattern}
    ,{"Raven LMU Metal Fix",0,              0x8024, &OFFSETS_TREE_NAME(raven_lmu),  &falcon_rw_pattern}
    ,{"Raven MPF Metal Fix",0,              0x0314, &OFFSETS_TREE_NAME(raven_mpf),  &falcon_rw_pattern}
    ,{"Raven MTIP_EXT Metal Fix",0,         0x0058, &OFFSETS_TREE_NAME(raven_mtip_ext), &falcon_rw_pattern}
    ,{"Raven DFX Server Metal Fix",0,     0x7F8248, &OFFSETS_TREE_NAME(raven),      &falcon_rw_pattern}
#ifndef ASIC_SIMULATION
    ,{"Eagle MNG_TAI Metal Fix",0xA,    0x1BF00204, &OFFSETS_TREE_NAME(null),       &falcon_rw_pattern}
    ,{"Eagle MNG_D2D_CP Metal Fix",0,     0x2E0070, &OFFSETS_TREE_NAME(null),       &falcon_rw_pattern}
    ,{"Eagle DFX_SRV Metal Fix",0,      0x1BEF8248, &OFFSETS_TREE_NAME(null),       &falcon_rw_pattern}
    ,{"Eagle BMA Metal Fix",0,          0x0D459034, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
#endif /*ASIC_SIMULATION*/
    ,{"Eagle CNC Metal Fix",0,          0x0A000180, &OFFSETS_TREE_NAME(cnc), &falcon_rw_pattern}
#ifndef ASIC_SIMULATION
    ,{"Eagle CP_TAI Metal Fix",0,       0x0A800204, &OFFSETS_TREE_NAME(cp_tai), &falcon_rw_pattern}
#endif /*ASIC_SIMULATION*/
    ,{"Eagle EOAM Metal Fix",0,         0x0A2000FC, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle EPCL Metal Fix",0,         0x0A3000C0, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle EPLR Metal Fix",0,         0x0A100220, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle EQ Metal Fix",0,           0x0B00AF30, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle EREP Metal Fix",0,         0x0D3F3104, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle ERMRK Metal Fix",0,        0x0A400108, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle HA Metal Fix",0,           0x0E000710, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle IA Metal Fix",0,           0x0D510300, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle IOAM Metal Fix",0,         0x0AB000FC, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle IPLR0 Metal Fix",0,        0x0AE00220, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle IPLR1 Metal Fix",0,        0x0AF00220, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle IPVX Metal Fix",0,         0x09000968, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle L2I Metal Fix",0,          0x09802010, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle LPM Metal Fix",0,          0x08F000F0, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle MLL Metal Fix",0,          0x0AC00050, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle IPCL Metal Fix",0,         0x0A080048, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle PREQ Metal Fix",0,         0x0A380054, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle RX_DMA Metal Fix",0,       0x0D521D00, &OFFSETS_TREE_NAME(rx_dma), &falcon_rw_pattern}
    ,{"Eagle TTI Metal Fix",0,          0x0CC013F8, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle TX_DMA Metal Fix",0,       0x0D565000, &OFFSETS_TREE_NAME(tx_dma), &falcon_rw_pattern}
    ,{"Eagle TX_FIFO Metal Fix",0,      0x0D573000, &OFFSETS_TREE_NAME(tx_fifo), &falcon_rw_pattern}
#ifndef ASIC_SIMULATION
    ,{"Eagle TXQ_TAI Metal Fix",0,      0x0D6E0204, &OFFSETS_TREE_NAME(txq_tai), &falcon_rw_pattern}
#endif /*ASIC_SIMULATION*/
    ,{"Eagle EGF_EFT Metal Fix",0,      0x0CB01110, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle EGF_QAG Metal Fix",0,      0x0C9B0020, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle EGF_SHT Metal Fix",0,      0x0CAB0030, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}
    ,{"Eagle TXQ_PDS Metal Fix",0,      0x0D671000, &OFFSETS_TREE_NAME(txq_pds), &falcon_rw_pattern}
    ,{"Eagle TXQ_QFX Metal Fix",0,      0x0D6C0000, &OFFSETS_TREE_NAME(txq_qfc), &falcon_rw_pattern}
    ,{"Eagle TXQ_SDQ Metal Fix",0,      0x0D600000, &OFFSETS_TREE_NAME(txq_sdq), &falcon_rw_pattern}
    /* PHA unit not initialyzed by pexOnly initSystem */
    /*,{"Eagle PHA_PPA Metal Fix",0,      0x0C7EFF40, &OFFSETS_TREE_NAME(control_pipe), &falcon_rw_pattern}*/
    ,{"Eagle D2D_PCS Metal Fix",0,      0x0D7F7078, &OFFSETS_TREE_NAME(d2d_pcs), &falcon_rw_pattern}
    ,{"Eagle D2D_PMA Metal Fix",0,      0x0D7F803C, &OFFSETS_TREE_NAME(d2d_pma), &falcon_rw_pattern}
    ,{"Eagle EM_FDB Metal Fix",0,       0x188A0170, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle FDB_MT Metal Fix",0,       0x188B0170, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
#ifndef ASIC_SIMULATION
    ,{"Eagle PB_SMB_WA Metal Fix",0,    0x19030164, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle SHM Metal Fix",0,          0x18890104, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle TCAM Metal Fix",0,         0x18507010, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle PB_GPC_GRP_CELL_READ Metal Fix",0,    0x19060210,
        &OFFSETS_TREE_NAME(pb_gpc_grp_cell_read), &falcon_rw_pattern}
    ,{"Eagle PB_GPC_GRP_PACKET_READ Metal Fix",0,  0x19082010,
        &OFFSETS_TREE_NAME(pb_gpc_grp_packet_read), &falcon_rw_pattern}
    ,{"Eagle PB_CENTER Metal Fix",0,    0x19082010, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle PB_GPC_GRP_PACKET_WRITE Metal Fix",0,  0x19082010,
        &OFFSETS_TREE_NAME(pb_gpc_grp_packet_write), &falcon_rw_pattern}
    ,{"Eagle PB_NPM_MC Metal Fix",0,    0x1910000C, &OFFSETS_TREE_NAME(pb_npm_mc), &falcon_rw_pattern}
    ,{"Eagle PB_SMB_MC Metal Fix",0,    0x19130164, &OFFSETS_TREE_NAME(pb_smb_mc), &falcon_rw_pattern}
#endif /*ASIC_SIMULATION*/
    ,{"Eagle TXQ_PSI_PDX Metal Fix",0,  0x1885F004, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle TXQ_PSI_PDX_PAC Metal Fix",0,  0x18801F04,
        &OFFSETS_TREE_NAME(txq_psi_pdx_pac), &falcon_rw_pattern}
    ,{"Eagle TXQ_PSI_PFCC Metal Fix",0, 0x1888000C, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{"Eagle TXQ_PSI_REGS Metal Fix",0, 0x18900000, &OFFSETS_TREE_NAME(null), &falcon_rw_pattern}
    ,{NULL, 0, 0, NULL, NULL}
};

/**
* @internal falcon_tile_screening_registers function
* @endinternal
*
* @brief  Tests several registers in falcon.
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] tileIndex             - Tile index
* @param[in] afterPrintSleep       - Time to sleep after printing to see last printed line
*                                    before stucking.
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_tile_screening_registers
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           tileIndex,
    IN  GT_U32           afterPrintSleep
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;
    GT_U32    i;

    if (tileIndex >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        cpssOsPrintf(
            "tileIndex exceedes amount of tiles %d\n",
            PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        return GT_FAIL;
    }

    for (i = 0; (screenig_registers_arr[i].funcPtr != NULL); i++)
    {
        if ((1 << tileIndex) & screenig_registers_arr[i].tileFilter)
            continue;
        if (afterPrintSleep) cpssOsPrintf("============================= ");
        cpssOsPrintf("Testing %s", screenig_registers_arr[i].name);
        if (afterPrintSleep) cpssOsPrintf(" =============================");
        cpssOsPrintf("\n");
        rc = falcon_tree_check(
            devNum,
            (screenig_registers_arr[i].offsetInSubunut + (tileIndex * 0x20000000)),
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

#define MAX_TABLE_ENTRY_WORDS_CNS 80

/**
* @internal falcon_table_entry0_pattern_check function
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
static GT_STATUS falcon_table_entry0_pattern_check
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

static TABLE_ID_RANGE_STC absolue_addressed_tables[] =
{
    /* all LMU tables */
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E, CPSS_DXCH_SIP6_MAX_GOP_LMU_STATISTICS_MAC}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E, CPSS_DXCH_SIP6_MAX_GOP_LMU_CONFIGURATION_MAC}
    /* all TXQ tables */
    ,{CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
        (CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_LAST_E + 1
         - CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E)}
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

/**
* @internal falcon_tile_screening_tables function
* @endinternal
*
* @brief  Tests several tables in falcon.
*
* @param[in] devNum                   - The CPSS devNum.
* @param[in] tileIndex                - Tile index
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
GT_STATUS falcon_tile_screening_tables
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           tileIndex,
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
    const GT_U32                  tileWindow = 0x20000000;
    GT_U32                        tileLow, tileHigh;
    static GT_U32                 saveEntry[MAX_TABLE_ENTRY_WORDS_CNS];
    GT_U32*                       tableBitmaskPtr;

    if (tileIndex >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        cpssOsPrintf(
            "tileIndex exceedes amount of tiles %d\n",
            PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        return GT_FAIL;
    }

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

        /*default*/
        portGroupIdStart = (tileIndex * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile);
        portGroupIdMax   = (portGroupIdStart + PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile);
        portGroupIdStep  = 1; /* unit per pipe */
        if (isTableFound(absolue_addressed_tables, tableId))
        {
            tileLow = (tileWindow * tileIndex);
            tileHigh = (tileLow + tileWindow) - 1;
            if (tableInfoPtr->readAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
            {
                PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr =
                    (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);
                /* out of tile base address */
                if ((tablePtr->baseAddress < tileLow) || (tablePtr->baseAddress > tileHigh))
                {
                    continue;
                }
            }
            /* address is absolute, use portGroupId == 0 only */
            portGroupIdStart = 0;
            portGroupIdMax   = 1;
            portGroupIdStep  = 1;
        }
        if (isTableFound(per_tile_tables, tableId))
        {
            portGroupIdStart = (tileIndex * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile);
            portGroupIdMax   = (portGroupIdStart + PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile);
            portGroupIdStep  = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile; /* unit per tile */
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
            rc = falcon_table_entry0_pattern_check(
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
* @internal falcon_tile_screening function
* @endinternal
*
* @brief  Tests several registers and tables in falcon tile.
*
* @param[in] devNum                   - The CPSS devNum.
* @param[in] tileIndex                - Tile index
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_tile_screening
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           tileIndex
)
{
    GT_STATUS rc;
    GT_STATUS rc1 = GT_OK;

    rc = falcon_tile_screening_registers(
        devNum, tileIndex, 0/*afterPrintSleep*/);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Falcon Tile %d Registers OK\n", tileIndex);
    }
    else
    {
        cpssOsPrintf("Falcon Tile %d Registers test FAILED\n", tileIndex);
        rc1 = GT_FAIL;
    }
    rc = falcon_tile_screening_tables(
        devNum, tileIndex, 0/*afterPrintSleep*/, 0/*includeBypassedTableBmp*/);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Falcon Tile %d Tables OK\n", tileIndex);
    }
    else
    {
        cpssOsPrintf("Falcon Tile %d Tables test FAILED\n", tileIndex);
        rc1 = GT_FAIL;
    }
    return rc1;
}

/**
* @internal falcon_screening function
* @endinternal
*
* @brief  Tests several registers and tables in all falcon devices.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_screening
(
    GT_VOID
)
{
    GT_SW_DEV_NUM    devNum;
    GT_STATUS        rc;
    GT_STATUS        rc1 = GT_OK;
    GT_U32           tileIndex;

    for (devNum = 0; (devNum < PRV_CPSS_MAX_PP_DEVICES_CNS); devNum++)
    {
        if (PRV_CPSS_IS_DEV_EXISTS_MAC(devNum) == 0) continue;
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E) continue;
        cpssOsPrintf("Falcon devNum %d test STARTED\n", devNum);

        for (tileIndex = 0; (tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
        {
            rc = falcon_tile_screening(devNum, tileIndex);
            if (rc != GT_OK)
            {
                rc1 = rc;
            }
        }
        if (rc1 == GT_OK)
        {
            cpssOsPrintf("Falcon devNum %d test OK\n", devNum);
        }
        else
        {
            cpssOsPrintf("Falcon devNum %d test FAILED\n", devNum);
        }
    }
    return rc1;
}

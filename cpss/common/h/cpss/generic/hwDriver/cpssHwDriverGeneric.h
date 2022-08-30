/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssHwDriverAPI.h
*
* @brief generic HW driver implementations
*
* @version   1
********************************************************************************
*/
#ifndef __cpssHwDriverGeneric_h__
#define __cpssHwDriverGeneric_h__
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/generic/cpssHwInfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SSMI_FALCON_ADDRESS_SPACE   0xFFFFFFFD

/**
 * @struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC
 *
 * @brief CPSS HW driver info to support split address space ranges , needed by AC5/AC5X devices.
 *        The CPSS uses this internally to create 'driver' to access the memories of the switch using
 *       'sip6 mbus' (internal CPU bus to the switch)
 *       The 'split' needed since the 'mmap' function is limited to 2G memory space allocation ,
 *       and when using sip6 mbus the memory space is more than 2G.
 *
 *   start        - virtual address for the start of the address space (as given by 'mmap' for the physical address)
 *
 *   size         - the size of the address space. in bytes as used for allocating the 'mmap'.
 *
 *  relativeOffset - for the case of AC5 when working with the internal CPU ,
 *                  for the 'resetAndInitController' the relativeOffset should be 0x84000000 (size 1M)
 *                  for the 'cnm' the relativeOffset should be 0x80000000 (size (5M))
 *                  for the 'switch' the relativeOffset should be 0x00000000 (zero) (size (2G - 4))
 *                  NOTE:
 *                     1. currently implemented only for the case of AC5 when working with the internal CPU
 *                     otherwise should be 0x00000000
 *                     2. this relativeOffset is the same value that given to 'mmap' as the 'physical address'
 *
 *  nextSplitResourcePtr - (pointer to) structure that hold the next 'split' resource
 *                  this allows for example a resource to have huge hole in it can not
 *                  be covered by single 'start' + 'size' as the 'mmap' function is limited to '2G-1'
 *                  size of mapping.
 *                  NOTE:
 *                  1. currently implemented only for the case of AC5 when working with the internal CPU
 *                     otherwise should be NULL
 *                  2. for AC5 when working with the internal CPU the 'switching' hold addresses from 0x00000000 to 0x80000000-1  (2G)
 *                     but the CPSS code also access to 'cnm','resetAndInitController' also as 'switching' address space
 *                     so those 2 resources should be given for the 'switching' as 'next split resource'
 *                  example :
 *                      'switching'.nextSplitResourcePtr = &'cnm';
 *                      'cnm'.nextSplitResourcePtr       = &'resetAndInitController';
 *                  3. An element will point to 'next' element that the 'next' relativeOffset is higher than
 *                     the relativeOffset of 'current' element.
*/
typedef struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT{
    GT_UINTPTR              start;
    GT_UINTPTR              size;
    GT_U32                  relativeOffset;
    struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT*  nextSplitResourcePtr;

}CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC;

/**
* @internal cpssHwDriverGenMmapCreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*
* @param[in] base   - resource virtual address
* @param[in] size   - mapped resource size
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapCreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size
);

GT_VOID cpssHwDriverGenMmapReloadDrv
(
    CPSS_HW_DRIVER_STC *drv
);


GT_VOID cpssHwDriverGenMmapRelativeAddrDrvReload
(
    CPSS_HW_DRIVER_STC *drv
);

/**
* @internal cpssHwDriverGenMmapCreateRelativeAddrDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*
* @param[in] base   - resource virtual address
* @param[in] size   - mapped resource size
* @param[in] relativeAddr   - the relative address that should be
*                   reduced from the 'regAddr' that used for read/write
* @param[in] nextSplitResourcePtr - allow to support split range or very large range
*            that it's actual size is more than 'size' parameter.
*
*   NOTE: compatible to cpssHwDriverGenMmapCreateDrv if using relativeAddr = 0
*         and nextSplitResourcePtr = NULL
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapCreateRelativeAddrDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size,
    IN  GT_UINTPTR  relativeAddr,
    IN  struct CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STCT*  nextSplitResourcePtr
);

/**
* @internal cpssHwDriverGenMmapAc8CreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*        with 8-region address completion
*
* @param[in] base           - resource virtual address
* @param[in] size           - mapped resource size
* @param[in] compRegionMask - the bitmap of address completion regions to use
*                             If the bit N is set to 1 then region N can be used
*                             by driver
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size must be >= 0x80000
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapAc8CreateDrv(
    IN  GT_UINTPTR  base,
    IN  GT_UINTPTR  size,
    IN  GT_U32      compRegionMask
);

/**
* @internal cpssHwDriverGenMmapAc4CreateDrv function
* @endinternal
*
* @brief Create driver instance for mmap'ed resource
*        with 4-region address completion
*
* @param[in] base           - resource virtual address
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*
* @note  size must be 64M
*/
CPSS_HW_DRIVER_STC *cpssHwDriverGenMmapAc4CreateDrv(
    IN  GT_UINTPTR  base
);

/**
* @internal cpssHwDriverSip5SlaveSMICreateDrv function
* @endinternal
*
* @brief Create driver instance for Sip5 Slave SMI
*
* @param[in] smi        - pointer to SMI driver
* @param[in] phy        - PP phy address
* @param[in] haSkipInit - skip Init if true
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSip5SlaveSMICreateDrv(
    IN  CPSS_HW_DRIVER_STC *smi,
    IN  GT_U32              phy,
    IN  GT_BOOL             haSkipInit
);

/**
* @internal cpssHwDriverSlaveSmiConfigure function
* @endinternal
*
* @brief   Configure Slave SMI device driver
*
* @param[in] smiMasterPath            - driver pointer
* @param[in] slaveSmiPhyId            - SMI Id of slave device
*
* @param[out] hwInfoPtr               - The pointer to HW info, will be used
*                                       for cpssDxChHwPpPhase1Init()
*                                       NULL to not fill HW info
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PARAM                - on wrong smiMasterPath
* @retval GT_NOT_INITIALIZED          - CPSS was not initialized yet by cpssPpInit
* @retval GT_OUT_OF_CPU_MEM           - on failed malloc
*/
GT_STATUS cpssHwDriverSlaveSmiConfigure(
    IN  const char         *smiMasterPath,
    IN  GT_U32              slaveSmiPhyId,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
);

#ifdef ASIC_SIMULATION

/* check if WM supports working in PEX mode (using BAR0,BAR2) */
GT_BOOL cpssHwDriverGenWmInPexModeGet(void);
/* set WM to supports working in PEX mode (using BAR0,BAR2) */
void cpssHwDriverGenWmInPexModeSet(IN GT_BOOL isPexMode);

/**
* @internal ASIC_SIMULATION_ReadMemory function
* @endinternal
*
* @brief   Read memory from ASIC_SIMULATION device.
*
* @param[in] drvBaseAddr              - the base that the driver hold for the WM device.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] pexFinalAddr             - address on the PEX to access (used in 'pex mode')
*
* @param[out] dataPtr                  - pointer to copy read data.
*/
void ASIC_SIMULATION_ReadMemory
(
    IN  GT_U32        drvBaseAddr,  /* used by default mode */
    IN  GT_U32        memAddr,      /* used by default mode */
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr,

    IN GT_UINTPTR     pexFinalAddr
);

/**
* @internal ASIC_SIMULATION_WriteMemory function
* @endinternal
*
* @brief   Write to memory of a ASIC_SIMULATION device.
*
* @param[in] drvBaseAddr              - the base that the driver hold for the WM device.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] pexFinalAddr             - address on the PEX to access (used in 'pex mode')
* @param[in] dataPtr                  - pointer to copy read data.
*
*
*/
void ASIC_SIMULATION_WriteMemory
(
    IN  GT_U32        drvBaseAddr,  /* used by default mode */
    IN  GT_U32        memAddr,      /* used by default mode */
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr,

    IN GT_UINTPTR     pexFinalAddr
);

#endif/*ASIC_SIMULATION*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(__cpssHwDriverGeneric_h__) */


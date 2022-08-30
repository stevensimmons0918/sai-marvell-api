/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file gtOsGlobalDb.h
*
* @brief This file provides  defenitions of global variables structures(both shared and non shared)
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsGlobalDb
#define __gtOsGlobalDb

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <gtOs/globalDb/gtOsEnablerModGlobalSharedDb.h>
#include <gtOs/globalDb/gtOsEnablerModGlobalNonSharedDb.h>

#define SHARED_DB_MAGIC     0xCAFED00D
#define NON_SHARED_DB_MAGIC 0xD00DCAFE
#define MAX_PIDS_SIZE  256

#define FIRST_CLIENT_DISCOVERY_SEM_NAME   "FIRST_CLIENT_SEM"

#define FIRST_CLIENT_DISCOVERY_SEM_OWNER_BIT   0x1
#define DB_INIT_SEM_OWNER_BIT                  0x2




/**
* @struct PRV_OS_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are ment to be shared
 * between processes (when CPSS is linked as shared object)
*/
typedef struct{

  /** @brief process id of the process that created shared memory used for this structure
    */
  GT_U32 initializerPid;

  /** @brief GT_TRUE if DB should be initialized,GT_FALSE otherwise
    */
  GT_BOOL dbInitialized;
  /** @brief unique number in order to identify shared variables data base.
    */
  GT_U32 magic;
  /** @brief number of processes that are using shared objest
  *          Note if not used as shared object then should be equal to 1
    */
  GT_U32 clientNum;
  /** @brief equal to GT_TRUE in case running as shared object ,GT_FALSE otherwise
    */
  GT_BOOL sharedMemoryUsed;

  /** @brief process id of the processes  that using  shared memory
    */
  GT_U32 pids[MAX_PIDS_SIZE];

  /** @brief Global shared variables used in cpssEnabler module
    */

  PRV_CPSS_ENABLER_MOD_SHARED_GLOBAL_DB     cpssEnablerMod;
} PRV_OS_SHARED_GLOBAL_DB;


/**
* @struct PRV_OS_NON_SHARED_NON_VOLATILE_DB
 *
 * @brief  Structure contain resources that were taken by the process.
 *  Should not be cleared on initialization.
*/

typedef struct{


    GT_BOOL ppMappingStageDone;
 /** @brief Bmp that mark the ownership of semaphores.
         Semaphores : OS_SHM_INIT_SEM and FIRST_CLIENT_SEM
      */
  GT_U32  semOwnershipBmp;

  /** @brief : the indication that multi-window supported .
   *    the first time that we need to use this info is when need to mmap DMA from
   *    the mvDmaDrv.ko in function try_map_mvDmaDrv() (called from check_dma()
   *    that called from extDrvGetDmaVirtBase(),extDrvGetDmaBase())
   *
   *    if the flag is GT_FALSE , we will use only index 0
   */
  GT_BOOL dmaConfigMultiWindowsSupported;

  /* @brief : cpssEnabler/mainExtDrv/src/gtExtDrv/linuxNoKernelModule/noKmDrvCacheMng.c
   *   NOTE : this size hold max devices that the sysfs_pci_configure_pex() can support.
   *   the function sysfs_pci_configure_pex() will fill per new device the info in
   *   new index in : dmaConfig[dmaConfigCurrentWindow] and will update dmaConfigNumOfActiveWindows.
   *
   *   NOTE: a system that not need multi windows , will work with index 0
   *    after setting : dmaConfigNumOfActiveWindows = 1.
   *
   *     NOTE: the function prvExtDrvSysfsInitDevices expected to reset the
   *        dmaConfigNumOfActiveWindows = 0 and the dmaConfigCurrentWindow = 0;
   */
  GT_MEMORY_DMA_CONFIG dmaConfig[GT_MEMORY_DMA_CONFIG_WINDOWS_CNS];
  /** @brief : number of active windows in dmaConfig[]
   */
  GT_U32               dmaConfigNumOfActiveWindows;
  /** @brief : the current index in dmaConfig[] , that will be accessed.
   */
  GT_U32               dmaConfigCurrentWindow;

  GT_BOOL               aslrSupport;

  GT_BOOL               verboseMode;

  GT_BOOL               isFirst;

      /** @brief  GT_TRUE if shared library client already passed initalization phase.
                GT_FALSE otherwise
      */
  GT_BOOL               clientIsInitialised;

} PRV_OS_NON_SHARED_NON_VOLATILE_DB;

/**
* @struct PRV_OS_NON_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are not ment to be shared
 * between processes (when CPSS is linked as shared object).
 *Each process will have an unique copy of the structure.
*/

typedef struct{
    /** @brief unique number in order to identify shared variables data base.
      */

  GT_U32 magic;

  PRV_CPSS_ENABLER_MOD_NON_SHARED_GLOBAL_DB     cpssEnablerMod;

  /*the variables that are not initialized at DB initialization phase nor erased*/
  PRV_OS_NON_SHARED_NON_VOLATILE_DB         osNonVolatileDb;
} PRV_OS_NON_SHARED_GLOBAL_DB;


/*OS layer global variables that are ment to be shared  between processes in shared lib mode*/
extern PRV_OS_SHARED_GLOBAL_DB     *osSharedGlobalVarsPtr;
/*OS layer global variables that are not ment to be shared  between processes in shared lib mode*/
extern PRV_OS_NON_SHARED_GLOBAL_DB *osNonSharedGlobalVarsPtr;
extern PRV_OS_NON_SHARED_GLOBAL_DB  osNonSharedGlobalVars;

/**
* @internal osGlobalDbDmaActiveWindowByPcieParamsSet function
* @endinternal
*
* @brief   function to set the current window Id for DMA chunk allocation to be
*          the one that should be associated with the PCIe device.
*          the function sets a new current window Id and returns this window to application.
*
* INPUTS:
*        pciDomain  - the PCI domain
*        pciBus     - the PCI bus
*        pciDev     - the PCI device
*        pciFunc    - the PCI function
*
* OUTPUTS:
*       windowPtr   - (pointer to) the window Id that associated with the PCIe device
*                   ignored if NULL
*
* @retval  GT_OK                    - if the PCIe parameters found to be for device
*                                   that was seen during PCIe scan of the extDrv
*          GT_NOT_FOUND             - if not found
*/
GT_STATUS osGlobalDbDmaActiveWindowByPcieParamsSet(
    IN GT_U32  pciDomain,
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc,
    OUT GT_U32  *windowPtr/* ignored if NULL*/
);

/**
* @internal osGlobalDbDmaActiveWindowSet function
* @endinternal
*
* @brief   function to set the current window for DMA chunk allocation.
*
* INPUTS:
*       window      - the window Id to used as active.
*
* @retval  GT_OK                    - if window in valid range
*          GT_BAD_PARAM             - if window not in valid range
*/
GT_STATUS osGlobalDbDmaActiveWindowSet(
    IN GT_U32  window
);

/**
* @internal osGlobalDbDmaActiveWindowGet function
* @endinternal
*
* @brief   function to get the current window used for DMA chunk allocation.
*
* OUTPUTS:
*       windowPtr   - (pointer to) the window Id
*
*
* @retval  GT_OK                  - on success
*          GT_BAD_PTR             - if windowPtr is NULL pointer
*/
GT_STATUS osGlobalDbDmaActiveWindowGet(
    OUT GT_U32  *windowPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtOsGlobalDb */


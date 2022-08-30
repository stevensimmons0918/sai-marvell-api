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
* @file prvCpssExtDrvFunctionsGlobalNonSharedDb.h
*
* @brief This file external driver function pointers  non shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssExtDrvFunctionsGlobalNonSharedDb
#define __prvCpssExtDrvFunctionsGlobalNonSharedDb

#include <cpss/extServices/extDrv/drivers/cpssEthPortCtrl.h>
#include <cpss/extServices/extDrv/drivers/cpssCacheMng.h>
#include <cpss/extServices/extDrv/drivers/cpssDmaDrv.h>



#define PRV_CPSS_DECLARE_FUNC(_name,_type) _type _name

/**
* @struct PRV_CPSS_COMMON_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain function pointers  that are not meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in common module
*/
typedef struct
{
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPrePendTwoBytesHeaderSet, CPSS_EXTDRV_ETH_CPU_PREPEND_TWO_BYTES_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvMgmtCacheFlush, CPSS_EXT_DRV_MGMT_CACHE_FLUSH_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvMgmtCacheInvalidate, CPSS_EXT_DRV_MGMT_CACHE_INVALIDATE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvDmaWrite, CPSS_EXTDRV_DMA_WRITE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvDmaRead, CPSS_EXTDRV_DMA_READ_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvInboundSdmaEnable, CPSS_EXTDRV_HSU_INBOUND_SDMA_ENABLE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvInboundSdmaDisable, CPSS_EXTDRV_HSU_INBOUND_SDMA_DISABLE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvInboundSdmaStateGet, CPSS_EXTDRV_HSU_INBOUND_SDMA_STATE_GET_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiInitDriver, CPSS_EXTDRV_HW_IF_SMI_INIT_DRIVER_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiWriteReg, CPSS_EXTDRV_HW_IF_SMI_WRITE_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiReadReg, CPSS_EXTDRV_HW_IF_SMI_READ_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTskRegRamRead, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_RAM_READ_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTskRegRamWrite, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_RAM_WRITE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTskRegVecRead, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_VEC_READ_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTskRegVecWrite, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_VEC_WRITE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTaskWriteReg, CPSS_EXTDRV_HW_IF_SMI_TASK_WRITE_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTaskReadReg, CPSS_EXTDRV_HW_IF_SMI_TASK_READ_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiInterruptWriteReg, CPSS_EXTDRV_HW_IF_SMI_INTERRUPT_WRITE_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiInterruptReadReg, CPSS_EXTDRV_HW_IF_SMI_INTERRUPT_READ_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvSmiDevVendorIdGet, CPSS_EXTDRV_HW_IF_SMI_DEV_VENDOR_ID_GET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfSmiTaskWriteRegField, CPSS_EXTDRV_HW_IF_SMI_TASK_WRITE_REG_FIELD_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfTwsiInitDriver, CPSS_EXTDRV_HW_IF_TWSI_INIT_DRIVER_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfTwsiWriteReg, CPSS_EXTDRV_HW_IF_TWSI_WRITE_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHwIfTwsiReadReg, CPSS_EXTDRV_HW_IF_TWSI_READ_REG_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvIntConnect, CPSS_EXTDRV_INT_CONNECT_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvIntDisconnect, CPSS_EXTDRV_INT_DISCONNECT_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvIntEnable, CPSS_EXTDRV_INT_ENABLE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvIntDisable, CPSS_EXTDRV_INT_DISABLE_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPortRxInit, CPSS_EXTDRV_ETH_PORT_RX_INIT_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthRawSocketModeSet, CPSS_EXTDRV_ETH_RAW_SOCKET_MODE_SET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthRawSocketModeGet, CPSS_EXTDRV_ETH_RAW_SOCKET_MODE_GET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvLinuxModeSet, CPSS_EXTDRV_LINUX_MODE_SET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvLinuxModeGet, CPSS_EXTDRV_LINUX_MODE_GET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHsuMemBaseAddrGet, CPSS_EXTDRV_HSU_MEM_BASE_ADDR_GET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvHsuWarmRestart, CPSS_EXTDRV_HSU_WARM_RESTART_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPortTxInit, CPSS_EXTDRV_ETH_PORT_TX_INIT_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPortEnable, CPSS_EXTDRV_ETH_PORT_ENABLE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPortDisable, CPSS_EXTDRV_ETH_PORT_DISABLE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPortTx, CPSS_EXTDRV_ETH_PORT_TX_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthInputHookAdd, CPSS_EXTDRV_ETH_PORT_INPUT_HOOK_ADD_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthRawSocketRxHookAdd, CPSS_EXTDRV_ETH_RAW_SOCKET_RX_HOOK_ADD_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthTxCompleteHookAdd, CPSS_EXTDRV_ETH_PORT_TX_COMPLETE_HOOK_ADD_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthRxPacketFree, CPSS_EXTDRV_ETH_PORT_RX_PACKET_FREE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthPortTxModeSet, CPSS_EXTDRV_ETH_PORT_TX_MODE_SET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEthCpuCodeToQueue, CPSS_EXTDRV_ETH_CPU_CODE_TO_QUEUE);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvI2cMgmtMasterInit, CPSS_EXTDRV_I2C_MGMT_MASTER_INIT_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvMgmtReadRegister, CPSS_EXTDRV_MGMT_READ_REGISTER_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvMgmtWriteRegister, CPSS_EXTDRV_MGMT_WRITE_REGISTER_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvMgmtIsrReadRegister, CPSS_EXTDRV_MGMT_ISR_READ_REGISTER_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvMgmtIsrWriteRegister, CPSS_EXTDRV_MGMT_ISR_WRITE_REGISTER_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvPciConfigWriteReg, CPSS_EXTDRV_PCI_CONFIG_WRITE_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvPciConfigReadReg, CPSS_EXTDRV_PCI_CONFIG_READ_REG_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvPciFindDev, CPSS_EXTDRV_PCI_DEV_FIND_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvGetPciIntVec, CPSS_EXTDRV_PCI_INT_VEC_GET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvGetIntMask, CPSS_EXTDRV_INT_MASK_GET_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvEnableCombinedPciAccess, CPSS_EXTDRV_PCI_COMBINED_ACCESS_ENABLE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvPciDoubleWrite, CPSS_EXTDRV_PCI_DOUBLE_WRITE_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvPciDoubleRead, CPSS_EXTDRV_PCI_DOUBLE_READ_FUNC);
   PRV_CPSS_DECLARE_FUNC(cpssExtDrvPciSetDevId, CPSS_EXTDRV_PCI_SET_DEV_ID_FUNC);

   PRV_CPSS_DECLARE_FUNC(cpssExtDrvDragoniteShMemBaseAddrGet, CPSS_EXTDRV_DRAGONITE_GET_BASE_ADDRESS_FUNC);

} PRV_CPSS_EXT_DRV_FUNC_PTR_NON_SHARED_GLOBAL_DB;


#define cpssExtDrvEthPrePendTwoBytesHeaderSet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPrePendTwoBytesHeaderSet)
#define cpssExtDrvMgmtCacheFlush PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvMgmtCacheFlush)
#define cpssExtDrvMgmtCacheInvalidate PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvMgmtCacheInvalidate)
#define cpssExtDrvDmaWrite PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvDmaWrite)
#define cpssExtDrvDmaRead PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvDmaRead)

#define cpssExtDrvInboundSdmaEnable PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvInboundSdmaEnable)
#define cpssExtDrvInboundSdmaDisable PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvInboundSdmaDisable)
#define cpssExtDrvInboundSdmaStateGet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvInboundSdmaStateGet)

#define cpssExtDrvHwIfSmiInitDriver PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiInitDriver)
#define cpssExtDrvHwIfSmiWriteReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiWriteReg)
#define cpssExtDrvHwIfSmiReadReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiReadReg)
#define cpssExtDrvHwIfSmiTskRegRamRead PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTskRegRamRead)
#define cpssExtDrvHwIfSmiTskRegRamWrite PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTskRegRamWrite)
#define cpssExtDrvHwIfSmiTskRegVecRead PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTskRegVecRead)
#define cpssExtDrvHwIfSmiTskRegVecWrite PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTskRegVecWrite)
#define cpssExtDrvHwIfSmiTaskWriteReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTaskWriteReg)
#define cpssExtDrvHwIfSmiTaskReadReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTaskReadReg)
#define cpssExtDrvHwIfSmiInterruptWriteReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiInterruptWriteReg)
#define cpssExtDrvHwIfSmiInterruptReadReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiInterruptReadReg)
#define cpssExtDrvSmiDevVendorIdGet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvSmiDevVendorIdGet)
#define cpssExtDrvHwIfSmiTaskWriteRegField PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfSmiTaskWriteRegField)

#define cpssExtDrvHwIfTwsiInitDriver PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfTwsiInitDriver)
#define cpssExtDrvHwIfTwsiWriteReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfTwsiWriteReg)
#define cpssExtDrvHwIfTwsiReadReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHwIfTwsiReadReg)

#define cpssExtDrvIntConnect PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvIntConnect)
#define cpssExtDrvIntDisconnect PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvIntDisconnect)
#define cpssExtDrvIntEnable PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvIntEnable)
#define cpssExtDrvIntDisable PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvIntDisable)

#define cpssExtDrvEthPortRxInit PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPortRxInit)
#define cpssExtDrvEthRawSocketModeSet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthRawSocketModeSet)
#define cpssExtDrvEthRawSocketModeGet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthRawSocketModeGet)
#define cpssExtDrvLinuxModeSet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvLinuxModeSet)
#define cpssExtDrvLinuxModeGet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvLinuxModeGet)
#define cpssExtDrvHsuMemBaseAddrGet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHsuMemBaseAddrGet)
#define cpssExtDrvHsuWarmRestart PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvHsuWarmRestart)
#define cpssExtDrvEthPortTxInit PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPortTxInit)
#define cpssExtDrvEthPortEnable PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPortEnable)
#define cpssExtDrvEthPortDisable PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPortDisable)
#define cpssExtDrvEthPortTx PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPortTx)
#define cpssExtDrvEthInputHookAdd PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthInputHookAdd)
#define cpssExtDrvEthRawSocketRxHookAdd PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthRawSocketRxHookAdd)
#define cpssExtDrvEthTxCompleteHookAdd PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthTxCompleteHookAdd)
#define cpssExtDrvEthRxPacketFree PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthRxPacketFree)
#define cpssExtDrvEthPortTxModeSet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthPortTxModeSet)
#define cpssExtDrvEthCpuCodeToQueue PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEthCpuCodeToQueue)

#define cpssExtDrvI2cMgmtMasterInit PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvI2cMgmtMasterInit)
#define cpssExtDrvMgmtReadRegister PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvMgmtReadRegister)
#define cpssExtDrvMgmtWriteRegister PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvMgmtWriteRegister)
#define cpssExtDrvMgmtIsrReadRegister PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvMgmtIsrReadRegister)
#define cpssExtDrvMgmtIsrWriteRegister PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvMgmtIsrWriteRegister)

#define cpssExtDrvPciConfigWriteReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvPciConfigWriteReg)
#define cpssExtDrvPciConfigReadReg PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvPciConfigReadReg)
#define cpssExtDrvPciFindDev PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvPciFindDev)
#define cpssExtDrvGetPciIntVec PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvGetPciIntVec)
#define cpssExtDrvGetIntMask PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvGetIntMask)
#define cpssExtDrvEnableCombinedPciAccess PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvEnableCombinedPciAccess)
#define cpssExtDrvPciDoubleWrite PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvPciDoubleWrite)
#define cpssExtDrvPciDoubleRead PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvPciDoubleRead)
#define cpssExtDrvPciSetDevId PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvPciSetDevId)

#define cpssExtDrvDragoniteShMemBaseAddrGet PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.extDrvFuncDb.cpssExtDrvDragoniteShMemBaseAddrGet)


#endif /* __prvCpssExtDrvFunctionsGlobalNonSharedDb */


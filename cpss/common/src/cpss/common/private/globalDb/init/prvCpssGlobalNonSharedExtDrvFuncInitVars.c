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
* @file prvCpssGlobalNonSharedExtDrvFuncInitVars.c
*
* @brief This file handle global non shared function pointers initialization
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG

#ifndef CPSS_NOT_IMPLEMENTED_EXTENDED_DEBUG

#define STR_NOT_IMPLEMENTED_CNS " extServiceFuncNotImplementedCalled \n"


#define PRV_CPSS_INIT_EXT_DRV_FUNC(_name,_type) \
     _name = (_type)extServiceFuncNotImplementedCalled;

static GT_STATUS extServiceFuncNotImplementedCalled
(
    void
)
{
    if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)extServiceFuncNotImplementedCalled)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}



#else
static void extServiceFuncNotImplementedCalledPrint(const char *name);

static GT_STATUS extServiceFuncNotImplementedCalled_cpssOsPrintf(void)
{
    extServiceFuncNotImplementedCalledPrint("cpssOsPrintf");
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}


#define STR_NOT_IMPLEMENTED_CNS " extServiceFuncNotImplementedCalled %s\n"

static void extServiceFuncNotImplementedCalledPrint(const char *name)
{
    if(cpssOsPrintf != (CPSS_OS_IO_PRINTF_FUNC)extServiceFuncNotImplementedCalled_cpssOsPrintf)
    {
        /* we already have "printf" from the application
          but current pointer of a function was not initialized yet */
        cpssOsPrintf(STR_NOT_IMPLEMENTED_CNS, name);
    }
}


#define PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(_name)\
\
static GT_STATUS extServiceFuncNotImplementedCalled_ ## _name(void) \
{ \
    extServiceFuncNotImplementedCalledPrint(#_name); \
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG); \
} \

#define PRV_CPSS_INIT_EXT_DRV_FUNC(_name,_type) \
    _name = (NULL == _name)?(_type)extServiceFuncNotImplementedCalled_ ## _name:_name;

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPrePendTwoBytesHeaderSet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvMgmtCacheFlush)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvMgmtCacheInvalidate)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvDmaWrite)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvDmaRead)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvInboundSdmaEnable)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvInboundSdmaDisable)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvInboundSdmaStateGet)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiInitDriver)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiWriteReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiReadReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegRamRead)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegRamWrite)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegVecRead)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegVecWrite)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTaskWriteReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTaskReadReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiInterruptWriteReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiInterruptReadReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvSmiDevVendorIdGet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTaskWriteRegField)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfTwsiInitDriver)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfTwsiWriteReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHwIfTwsiReadReg)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvIntConnect)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvIntDisconnect)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvIntEnable)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvIntDisable)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPortRxInit)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthRawSocketModeSet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthRawSocketModeGet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvLinuxModeSet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvLinuxModeGet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHsuMemBaseAddrGet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvHsuWarmRestart)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPortTxInit)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPortEnable)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPortDisable)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPortTx)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthInputHookAdd)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthRawSocketRxHookAdd)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthTxCompleteHookAdd)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthRxPacketFree)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthPortTxModeSet)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEthCpuCodeToQueue)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvI2cMgmtMasterInit)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvMgmtReadRegister)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvMgmtWriteRegister)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvMgmtIsrReadRegister)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvMgmtIsrWriteRegister)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvPciConfigWriteReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvPciConfigReadReg)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvPciFindDev)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvGetPciIntVec)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvGetIntMask)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvEnableCombinedPciAccess)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvPciDoubleWrite)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvPciDoubleRead)
PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvPciSetDevId)

PRV_CPSS_DECLARE_DBG_EXT_DRV_FUNC(cpssExtDrvDragoniteShMemBaseAddrGet)

#endif

/**
 * @internal cpssGlobalNonSharedDbExtDrvFuncInit function,
 *           CPSS_EXTDRV_DRAGONITE_GET_BASE_ADDRESS_FUNC);
 * @endinternal
 *
 * @brief  Initialize global non shared function pointers
 *
 */
GT_VOID cpssGlobalNonSharedDbExtDrvFuncInit
(
    GT_VOID
)
{
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPrePendTwoBytesHeaderSet, CPSS_EXTDRV_ETH_CPU_PREPEND_TWO_BYTES_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvMgmtCacheFlush, CPSS_EXT_DRV_MGMT_CACHE_FLUSH_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvMgmtCacheInvalidate, CPSS_EXT_DRV_MGMT_CACHE_INVALIDATE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvDmaWrite, CPSS_EXTDRV_DMA_WRITE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvDmaRead, CPSS_EXTDRV_DMA_READ_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvInboundSdmaEnable, CPSS_EXTDRV_HSU_INBOUND_SDMA_ENABLE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvInboundSdmaDisable, CPSS_EXTDRV_HSU_INBOUND_SDMA_DISABLE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvInboundSdmaStateGet, CPSS_EXTDRV_HSU_INBOUND_SDMA_STATE_GET_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiInitDriver, CPSS_EXTDRV_HW_IF_SMI_INIT_DRIVER_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiWriteReg, CPSS_EXTDRV_HW_IF_SMI_WRITE_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiReadReg, CPSS_EXTDRV_HW_IF_SMI_READ_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegRamRead, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_RAM_READ_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegRamWrite, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_RAM_WRITE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegVecRead, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_VEC_READ_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTskRegVecWrite, CPSS_EXTDRV_HW_IF_SMI_TASK_REG_VEC_WRITE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTaskWriteReg, CPSS_EXTDRV_HW_IF_SMI_TASK_WRITE_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTaskReadReg, CPSS_EXTDRV_HW_IF_SMI_TASK_READ_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiInterruptWriteReg, CPSS_EXTDRV_HW_IF_SMI_INTERRUPT_WRITE_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiInterruptReadReg, CPSS_EXTDRV_HW_IF_SMI_INTERRUPT_READ_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvSmiDevVendorIdGet, CPSS_EXTDRV_HW_IF_SMI_DEV_VENDOR_ID_GET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfSmiTaskWriteRegField, CPSS_EXTDRV_HW_IF_SMI_TASK_WRITE_REG_FIELD_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfTwsiInitDriver, CPSS_EXTDRV_HW_IF_TWSI_INIT_DRIVER_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfTwsiWriteReg, CPSS_EXTDRV_HW_IF_TWSI_WRITE_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHwIfTwsiReadReg, CPSS_EXTDRV_HW_IF_TWSI_READ_REG_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvIntConnect, CPSS_EXTDRV_INT_CONNECT_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvIntDisconnect, CPSS_EXTDRV_INT_DISCONNECT_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvIntEnable, CPSS_EXTDRV_INT_ENABLE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvIntDisable, CPSS_EXTDRV_INT_DISABLE_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPortRxInit, CPSS_EXTDRV_ETH_PORT_RX_INIT_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthRawSocketModeSet, CPSS_EXTDRV_ETH_RAW_SOCKET_MODE_SET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthRawSocketModeGet, CPSS_EXTDRV_ETH_RAW_SOCKET_MODE_GET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvLinuxModeSet, CPSS_EXTDRV_LINUX_MODE_SET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvLinuxModeGet, CPSS_EXTDRV_LINUX_MODE_GET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHsuMemBaseAddrGet, CPSS_EXTDRV_HSU_MEM_BASE_ADDR_GET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvHsuWarmRestart, CPSS_EXTDRV_HSU_WARM_RESTART_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPortTxInit, CPSS_EXTDRV_ETH_PORT_TX_INIT_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPortEnable, CPSS_EXTDRV_ETH_PORT_ENABLE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPortDisable, CPSS_EXTDRV_ETH_PORT_DISABLE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPortTx, CPSS_EXTDRV_ETH_PORT_TX_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthInputHookAdd, CPSS_EXTDRV_ETH_PORT_INPUT_HOOK_ADD_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthRawSocketRxHookAdd, CPSS_EXTDRV_ETH_RAW_SOCKET_RX_HOOK_ADD_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthTxCompleteHookAdd, CPSS_EXTDRV_ETH_PORT_TX_COMPLETE_HOOK_ADD_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthRxPacketFree, CPSS_EXTDRV_ETH_PORT_RX_PACKET_FREE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthPortTxModeSet, CPSS_EXTDRV_ETH_PORT_TX_MODE_SET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEthCpuCodeToQueue, CPSS_EXTDRV_ETH_CPU_CODE_TO_QUEUE);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvI2cMgmtMasterInit, CPSS_EXTDRV_I2C_MGMT_MASTER_INIT_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvMgmtReadRegister, CPSS_EXTDRV_MGMT_READ_REGISTER_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvMgmtWriteRegister, CPSS_EXTDRV_MGMT_WRITE_REGISTER_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvMgmtIsrReadRegister, CPSS_EXTDRV_MGMT_ISR_READ_REGISTER_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvMgmtIsrWriteRegister, CPSS_EXTDRV_MGMT_ISR_WRITE_REGISTER_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvPciConfigWriteReg, CPSS_EXTDRV_PCI_CONFIG_WRITE_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvPciConfigReadReg, CPSS_EXTDRV_PCI_CONFIG_READ_REG_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvPciFindDev, CPSS_EXTDRV_PCI_DEV_FIND_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvGetPciIntVec, CPSS_EXTDRV_PCI_INT_VEC_GET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvGetIntMask, CPSS_EXTDRV_INT_MASK_GET_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvEnableCombinedPciAccess, CPSS_EXTDRV_PCI_COMBINED_ACCESS_ENABLE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvPciDoubleWrite, CPSS_EXTDRV_PCI_DOUBLE_WRITE_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvPciDoubleRead, CPSS_EXTDRV_PCI_DOUBLE_READ_FUNC);
    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvPciSetDevId, CPSS_EXTDRV_PCI_SET_DEV_ID_FUNC);

    PRV_CPSS_INIT_EXT_DRV_FUNC(cpssExtDrvDragoniteShMemBaseAddrGet, CPSS_EXTDRV_DRAGONITE_GET_BASE_ADDRESS_FUNC);


}





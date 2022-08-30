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
* @file scib.h
*
* @brief This is a API definition for CPU interface block of the Simulation.
*
* @version   15
********************************************************************************
*/
#ifndef __scibh
#define __scibh

#include <os/simTypes.h>
#include <os/simTypesBind.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SCIB_NOT_EXISTED_DEVICE_ID_CNS 0xffffffff

#define NUM_BYTES_TO_WORDS(bytes) (((bytes)+3) / 4)

/**
* @enum SCIB_MEM_ACCESS_CLIENT
 *
 * @brief list of memory access clients with separate memories.
*/
typedef enum{

    /** PCI/PEX/MBUS external memory access */
    SCIB_MEM_ACCESS_PCI_E = 0,

    /** DFX server external memory access */
    SCIB_MEM_ACCESS_DFX_E,

    /** PP internal memory */
    SCIB_MEM_ACCESS_CORE_E,

    /** BAR0 1M window to some place in the CnM memory */
    SCIB_MEM_ACCESS_BAR0_E,

    /** BAR2 4M/8M window to see the switch , together with the CnM 'iATU' 64 windows , there is
        'address completion' that allow to access the 32bits address of the device  */
    SCIB_MEM_ACCESS_BAR2_E,

    SCIB_MEM_ACCESS_LAST_E

} SCIB_MEM_ACCESS_CLIENT;

/* Each bit in the values of enum SCIB_MEMORY_ACCESS_TYPE */
#define OPERATION_BIT       (1<<0) /* bit 0 */  /* 0 - read , 1 - write   */
#define LAYER_BIT           (1<<1) /* bit 1 */  /* 0 - scib , 1 - skernel */
#define PCI_BIT             (1<<2) /* bit 2 */  /* 0 - non PCI, 1 - PCI */
#define DFX_BIT             (1<<3) /* bit 3 */  /* 0 - non DFX, 1 - DFX */
#define BAR0_BIT            (1<<4) /* bit 4 */  /* 0 - non BAR0, 1 - BAR0 */
#define BAR2_BIT            (1<<5) /* bit 5 */  /* 0 - non BAR2, 1 - BAR2 */

/* Check if operation type is 'write' (or 'read')*/
#define IS_WRITE_OPERATION_MAC(type)    \
    (((type) & OPERATION_BIT) ? 1 : 0)

/* Check if operation type is 'skernel' (or 'scib')*/
#define IS_SKERNEL_OPERATION_MAC(type)  \
    (((type) & LAYER_BIT) ? 1 : 0)

/* Check if operation type is 'PCI' */
#define IS_PCI_OPERATION_MAC(type)  \
    (((type) & PCI_BIT) ? 1 : 0)

/* Check if operation type is 'DFX' */
#define IS_DFX_OPERATION_MAC(type)  \
    (((type) & DFX_BIT) ? 1 : 0)

/* Check if operation type is 'BAR0' */
#define IS_BAR0_OPERATION_MAC(type)  \
    (((type) & BAR0_BIT) ? 1 : 0)

/* Check if operation type is 'BAR2' */
#define IS_BAR2_OPERATION_MAC(type)  \
    (((type) & BAR2_BIT) ? 1 : 0)


typedef enum
{
                            /* PCI/DFX/CORE  skernel     write */
    SCIB_MEMORY_READ_E         = 0                                      ,/*0*/
    SCIB_MEMORY_WRITE_E        =                        OPERATION_BIT   ,/*1*/
    SKERNEL_MEMORY_READ_E      =            LAYER_BIT                   ,/*2*/
    SKERNEL_MEMORY_WRITE_E     =            LAYER_BIT | OPERATION_BIT   ,/*3*/
    SCIB_MEMORY_READ_PCI_E     = PCI_BIT                                ,/*4*/
    SCIB_MEMORY_WRITE_PCI_E    = PCI_BIT |              OPERATION_BIT   ,/*5*/
    SKERNEL_MEMORY_READ_PCI_E  = PCI_BIT |  LAYER_BIT                   ,/*6*/
    SKERNEL_MEMORY_WRITE_PCI_E = PCI_BIT |  LAYER_BIT | OPERATION_BIT   ,/*7*/
    SCIB_MEMORY_READ_DFX_E     = DFX_BIT                                ,/*8*/
    SCIB_MEMORY_WRITE_DFX_E    = DFX_BIT |              OPERATION_BIT   ,/*9*/
    SKERNEL_MEMORY_READ_DFX_E  = DFX_BIT |  LAYER_BIT                   ,/*10*/
    SKERNEL_MEMORY_WRITE_DFX_E = DFX_BIT |  LAYER_BIT | OPERATION_BIT   ,/*11*/

    SCIB_MEMORY_READ_BAR0_E     = BAR0_BIT                                ,/*12*/
    SCIB_MEMORY_WRITE_BAR0_E    = BAR0_BIT |              OPERATION_BIT   ,/*13*/
    SKERNEL_MEMORY_READ_BAR0_E  = BAR0_BIT |  LAYER_BIT                   ,/*14*/
    SKERNEL_MEMORY_WRITE_BAR0_E = BAR0_BIT |  LAYER_BIT | OPERATION_BIT   ,/*15*/

    SCIB_MEMORY_READ_BAR2_E     = BAR2_BIT                                ,/*16*/
    SCIB_MEMORY_WRITE_BAR2_E    = BAR2_BIT |              OPERATION_BIT   ,/*17*/
    SKERNEL_MEMORY_READ_BAR2_E  = BAR2_BIT |  LAYER_BIT                   ,/*18*/
    SKERNEL_MEMORY_WRITE_BAR2_E = BAR2_BIT |  LAYER_BIT | OPERATION_BIT   ,/*19*/

    SCIB_MEMORY_LAST_E = 0xFFFF
}SCIB_MEMORY_ACCESS_TYPE;

/* value for interrupt line that is not used */
#define SCIB_INTERRUPT_LINE_NOT_USED_CNS    0xFFFFFFFF


/*
    problem was seen only in WIN32 , due to OS implementation (and not needed in Linux):

    the 'lock'/'unlock' (scibAccessLock,scibAccessUnlock)was created for next purpose:
    the application don't want to 'lock task' that is currently take the one of
    the simulation semaphore  (like during sbufAlloc) , so we give the
    application a semaphore that it application can use , so it can take this
    semaphore when 'lock tasks/interrupts' so application will know that no task
    is currently inside the simulation when locking it.

    (see in CPSS the use :
        <gtOs\win32\osWin32IntrSim.c>
        <gtOs\win32\osWin32Task.c> )

    NOTE: the protector needed only on NON-distributed simulation.
         because on distributed simulation , on PP side , all actions are
         serialized
*/

#define SCIB_SEM_TAKE scibAccessLock()
#define SCIB_SEM_SIGNAL scibAccessUnlock()
/* get the counter to indicate the locking level of simulationProtectorMtx */
GT_U32  scibAccessMutexCountGet(void);

/*******************************************************************************
*  SCIB_RW_MEMORY_FUN
*
* DESCRIPTION:
*      Definition of R/W Skernel memory function.
* INPUTS:
*       accessType  - Define access operation Read or Write.
*       devObjPtr   - Opaque for SCIB device id.
*       address     - Address for ASIC memory.
*       memSize     - Size of ASIC memory to read or write.
*       memPtr      - For Write this pointer to application memory,which
*                     will be copied to the ASIC memory .
*
* OUTPUTS:
*       memPtr     - For Read this pointer to application memory in which
*                     ASIC memory will be copied.
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
typedef void (* SCIB_RW_MEMORY_FUN) (
                                   IN SCIB_MEMORY_ACCESS_TYPE accessType,
                                   IN void * deviceObjPtr,
                                   IN GT_U32 address,
                                   IN GT_U32 memSize,
                                   INOUT GT_U32 * memPtr );

/*******************************************************************************
*   SCIB_INTERRUPT_SET_FUN
*
* DESCRIPTION:
*       Generate interrupt for SKernel device.
*
* INPUTS:
*       deviceId  - ID of device, which is equal to PSS Core API device ID.
*
* OUTPUTS:
*       none.
*
* RETURNS:
*
*
* COMMENTS:
*       because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*******************************************************************************/
typedef void (*SCIB_INTERRUPT_SET_FUN)
(
    IN  GT_U32        deviceId
);

/**
* @internal scibInit0 function
* @endinternal
*
* @brief   Init SCIB mutex for scibAccessLock(), scibAccessUnlock
*/
void scibInit0(void);

/**
* @internal scibInit function
* @endinternal
*
* @brief   Init SCIB library.
*
* @param[in] maxDevNumber             - maximal number of SKernel devices in the Simulation.
*/
void scibInit
(
    IN GT_U32 maxDevNumber
);

/**
* @internal scibBindRWMemory function
* @endinternal
*
* @brief   Bind callbacks of SKernel for R/W memory requests.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] deviceHwId               - physical device Id.
*                                      devObjPtr  - pointer to the opaque for SCIB device object.
* @param[in] rwFun                    - pointer to the R/W SKernel memory CallBack function.
* @param[in] isPP                     - bind to pp object or to fa.
* @param[in] addressCompletionStatus  - device enable/disable address completion
*
* @note device object can be fa also.
*
*/
void scibBindRWMemory
(
    IN GT_U32           deviceId,
    IN GT_U32           deviceHwId,
    IN void         *       deviceObjPtr,
    IN SCIB_RW_MEMORY_FUN   rwFun,
    IN GT_U8                isPP,
    IN GT_BOOL              addressCompletionStatus
);

/**
* @internal scibSetIntLine function
* @endinternal
*
* @brief   Set interrupt line for SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] intrline                 - number of interrupt line.
*
* @note device object can be fa also.
*
*/
void scibSetIntLine
(
    IN GT_U32               deviceId,
    IN GT_U32               intrline
);
/**
* @internal scibReadMemory function
* @endinternal
*
* @brief   Read memory from SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*/
void scibReadMemory
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr
 );
/**
* @internal scibWriteMemory function
* @endinternal
*
* @brief   Write to memory of a SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*
* @note device object can be fa also.
*
*/
void scibWriteMemory
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr
 );
/**
* @internal scibPciRegRead function
* @endinternal
*
* @brief   Read PCI registers memory from SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*
* @note device object can not be fa.
*
*/
void scibPciRegRead
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    OUT GT_U32 *      dataPtr
);

/**
* @internal scibPciRegWrite function
* @endinternal
*
* @brief   Write to PCI memory of a SKernel device.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*
* @note device object can not be fa.
*
*/
void scibPciRegWrite
(
    IN  GT_U32        deviceId,
    IN  GT_U32        memAddr,
    IN  GT_U32        length,
    IN  GT_U32 *      dataPtr
);


/**
* @internal scibSetInterrupt function
* @endinternal
*
* @brief   Generate interrupt for SKernel device.
*
* @param[in] deviceId                 - ID of device
*/
void scibSetInterrupt
(
    IN  GT_U32        deviceId
);

/**
* @internal scibUnSetInterrupt function
* @endinternal
*
* @brief   give indication that the interrupt reason has ended.
*
* @param[in] deviceId                 - ID of device.
*
* @note because the fa and the pp has the same interrupt link i chosen to use
*       only the interrupt line of pp .
*
*/
void scibUnSetInterrupt
(
    IN  GT_U32        deviceId
);
/**
* @internal scibSmiRegRead function
* @endinternal
*
* @brief   This function reads a switch's port register.
*
* @param[in] deviceId                 - Device object Id.
* @param[in] smiDev                   - Smi device to read the register for
* @param[in] regAddr                  - The register's address.
*
* @param[out] data                     - The read register's data.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
void scibSmiRegRead
(
    IN  GT_U32      deviceId,
    IN  GT_U32      smiDev,
    IN  GT_U32      regAddr,
    OUT GT_U32      *data
);

/**
* @internal scibSmiRegWrite function
* @endinternal
*
* @brief   This function writes to a switch's port register.
*
* @param[in] deviceId                 - Device object Id
* @param[in] smiDev                   - Smi device number to read the register for.
* @param[in] regAddr                  - The register's address.
* @param[in] data                     - The  to be written.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
void scibSmiRegWrite
(
    IN  GT_U32      deviceId,
    IN  GT_U32      smiDev,
    IN  GT_U32      regAddr,
    IN  GT_U32      data
);

/**
* @internal scibGetDeviceId function
* @endinternal
*
* @brief   scans for index of the entry with given hwId
*
* @param[in] deviceHwId               - maximal number of SKernel devices in the Simulation.
*/
GT_U32     scibGetDeviceId
(
    IN  GT_U32        deviceHwId
 );

/**
* @internal scibAddressCompletionStatusGet function
* @endinternal
*
* @brief   Get address completion for given hwId
*
* @param[in] devNum                   - device ID.
*/
GT_BOOL  scibAddressCompletionStatusGet
(
    IN  GT_U32        devNum
);

/* defines to be used for value dataIsWords in APIs of : scibDmaRead,scibDmaWrite*/
#define SCIB_DMA_BYTE_STREAM    0
#define SCIB_DMA_WORDS          1

/* define to be used for value deviceId in APIs of : scibDmaRead,scibDmaWrite
   can use it in:
   1. non-distributed
   2. on application side (when distributed)

   on the Asic side (when distributed) must use the real deviceId of the device
*/
#define SCIB_DMA_ACCESS_DUMMY_DEV_NUM_CNS 0xee

/**
* @internal scibDmaRead function
* @endinternal
*
* @brief   read HOST CPU DMA memory function.
*         Asic is calling this function to read DMA.
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to read .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*/
void scibDmaRead
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal scibDmaWrite function
* @endinternal
*
* @brief   write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void scibDmaWrite
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal scibRemoteInit function
* @endinternal
*
* @brief   init a device info in Scib , when working in distributed architecture and
*         this is the application side .
*         Asic send message to application side , and on application side this
*         function is called.
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID
* @param[in] deviceHwId               - Physical device Id.
* @param[in] interruptLine            - interrupt line of the device.
* @param[in] isPp                     - (GT_BOOL) is PP of FA
* @param[in] addressCompletionStatus  - device enable/disable address completion
*/
void scibRemoteInit
(
    IN GT_U32  deviceId,
    IN GT_U32  deviceHwId,
    IN GT_U32  interruptLine,
    IN GT_U32  isPp,
    IN GT_U32  addressCompletionStatus
);

/**
* @internal scibAccessUnlock function
* @endinternal
*
* @brief   function to protect the accessing to the SCIB layer .
*         the function UN-LOCK the access.
*         The mutex implementations allow reentrant of the locking task.
*/
void scibAccessUnlock(void);

/**
* @internal scibAccessLock function
* @endinternal
*
* @brief   function to protect the accessing to the SCIB layer .
*         the function LOCK the access.
*         The mutex implementations allow reentrant of the locking task.
*/
void scibAccessLock(void);


/**
* @internal scibUnBindDevice function
* @endinternal
*
* @brief   unBind the BUS from read/write register functions
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*
* @note accessing to the 'read'/'write' registers will cause fatal error.
*
*/
void scibUnBindDevice
(
    IN GT_U32               deviceId
);

/**
* @internal scibReBindDevice function
* @endinternal
*
* @brief   re-bind the BUS to read/write register functions
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*/
void scibReBindDevice
(
    IN GT_U32               deviceId
);

/**
* @internal scibPortLoopbackForceModeSet function
* @endinternal
*
* @brief   the function set the 'loopback force mode' on a port of device.
*         this function needed for devices that not support loopback on the ports
*         and need 'external support'
* @param[in] deviceId                 - the simulation device Id .
* @param[in] portNum                  - the physical port number .
* @param[in] mode                     - the loopback force mode.
*                                      0 - SKERNEL_PORT_LOOPBACK_NOT_FORCED_E,
*                                      1 - SKERNEL_PORT_LOOPBACK_FORCE_ENABLE_E,
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad portNum or mode
*
* @note function do fatal error on non-exists device or out of range device.
*
*/
GT_STATUS scibPortLoopbackForceModeSet
(
    IN  GT_U32                      deviceId,
    IN  GT_U32                      portNum,
    IN  GT_U32                      mode
);

/**
* @internal scibMemoryClientRegRead function
* @endinternal
*
* @brief   Generic read of SCIB client registers from SKernel device.
*
* @param[in] deviceId                 - ID of SKErnel device, which is equal to PSS Core API device ID.
* @param[in] scibClient               - memory access client: PCI/Core/DFX
* @param[in] memAddr                  - address of first word to read.
* @param[in] length                   - number of words to read.
*
* @param[out] dataPtr                  - pointer to copy read data.
*                                       None
*/
void scibMemoryClientRegRead
(
    IN  GT_U32                  deviceId,
    IN  SCIB_MEM_ACCESS_CLIENT  scibClient,
    IN  GT_U32                  memAddr,
    IN  GT_U32                  length,
    OUT GT_U32 *                dataPtr
);

/**
* @internal scibMemoryClientRegWrite function
* @endinternal
*
* @brief   Generic write to registers of SKernel device according to SCIB client.
*
* @param[in] deviceId                 - ID of SKernel device, which is equal to PSS Core API device ID.
* @param[in] memAddr                  - address of first word to read.
* @param[in] scibClient               - memory access client: PCI/Core/DFX
* @param[in] length                   - number of words to read.
* @param[in] dataPtr                  - pointer to copy read data.
*                                       None
*/
void scibMemoryClientRegWrite
(
    IN  GT_U32                  deviceId,
    IN  SCIB_MEM_ACCESS_CLIENT  scibClient,
    IN  GT_U32                  memAddr,
    IN  GT_U32                  length,
    IN  GT_U32 *                dataPtr
);

/**
* @internal scibBindInterruptMppTrigger function
* @endinternal
*
* @brief   Bind callback of device to be called in interrupt instead of to trigger the CPU.
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
* @param[in] interruptTriggerFun      - the CallBack function.
*/
void scibBindInterruptMppTrigger
(
    IN GT_U32               deviceId,
    IN SCIB_INTERRUPT_SET_FUN interruptTriggerFun
);

#define   SCIB_BUS_INTERFACE_PEX            (1<<1)
#define   SCIB_BUS_INTERFACE_SMI            (1<<2)
#define   SCIB_BUS_INTERFACE_SMI_INDIRECT   (1<<3)
/**
* @internal scibBusInterfaceGet function
* @endinternal
*
* @brief   get the bus interface(s) that the device is connected to.
*
* @param[in] deviceId                 - the device id as appear in the INI file
* @param[out] interfaceBmpPtr         - (pointer to)the BMP of interfaces that
*                                        the device connected to
*                                       (combination of :SCIB_BUS_INTERFACE_PEX ,
*                                       SCIB_BUS_INTERFACE_SMI,
*                                       SCIB_BUS_INTERFACE_SMI_INDIRECT)
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS scibBusInterfaceGet
(
    IN GT_U32                  deviceId,
    OUT GT_U32                *interfaceBmpPtr
);

/**
* @internal scibBusInterfaceAllowedSet function
* @endinternal
*
* @brief   state the which interface(s) will be able to show in scibBusInterfaceGet(...)
*          (meaning that the device maybe connected to the SMI , but we want to
*          'hide' the device from the 'SMI scan')
*
* @param[in] deviceId                 - the device id as appear in the INI file
* @param[out] allowedInterfaceBmp     - the BMP of interfaces that the device
*                                       allow in scibBusInterfaceGet(...).
*                                       (combination of :SCIB_BUS_INTERFACE_PEX ,
*                                       SCIB_BUS_INTERFACE_SMI,
*                                       SCIB_BUS_INTERFACE_SMI_INDIRECT,
*                                       Value of 0xFFFFFFFF - means 'no limitations'
*                                           --> default behavior
*                                       Value of 0 - means the 'device hide' will
*                                           no be seen by scibBusInterfaceGet(...)
*                                           that used by 'SMI scan')
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS scibBusInterfaceAllowedSet
(
    IN GT_U32                  deviceId,
    IN GT_U32                  allowedInterfaceBmp
);

/**
* @internal scibDmaUpper32BitsSet function
* @endinternal
*
* @brief   set the WM for 64bits CPU , that allocated 'malloc' for DMA in
*           address >= 4G
*
* @param[in] addr_high                - the high 32bits of the DMA address.
*
*/
void scibDmaUpper32BitsSet(
    IN GT_U32   addr_high
);


typedef struct{
    GT_BOOL     update_deviceForceBar0Bar2;
    GT_BOOL     deviceForceBar0Bar2;

    GT_BOOL update_pciInfo;
    GT_U32 pciBus;
    GT_U32 pciDev;
    GT_U32 pciFunc;

    GT_BOOL update_bar0_base;
    GT_U32  bar0_base;
    GT_BOOL update_bar0_size;
    GT_U32  bar0_size;
    GT_BOOL update_bar0_base_high;
    GT_U32  bar0_base_high;

    GT_BOOL update_bar2_base;
    GT_U32  bar2_base;
    GT_BOOL update_bar2_size;
    GT_U32  bar2_size;
    GT_BOOL update_bar2_base_high;
    GT_U32  bar2_base_high;
}SCIB_BIND_EXT_INFO_STC;

/**
* @internal scibBindExt function
* @endinternal
*
* @brief   extra Bind info of SKernel for the device.
*
* @param[in] deviceId                 - ID of device (as appears in the INI file).
* @param[in] bindExtInfoPtr           - pointer to more info.
*
* @note
*
*/
void scibBindExt
(
    IN GT_U32                   deviceId,
    IN SCIB_BIND_EXT_INFO_STC   *bindExtInfoPtr
);

/**
* @internal scibDmaRead64BitAddr function
* @endinternal
*
* @brief   prototype for function that allow the device to read DMA (DRAM or other memory) via PCIe 'bus'
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to read from the address
* @param[out] wordsArray[]            - array of words fill with the read values
*
*/
void scibDmaRead64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    OUT GT_U32  wordsArray[]/* according to numOfWords */
);

/**
* @internal scibDmaWrite64BitAddr function
* @endinternal
*
* @brief   prototype for function that allow the device to write DMA (DRAM or other memory) via PCIe 'bus'
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to write from the address
* @param[in] wordsArray[]             - array of words fill with the values to write
*
*/
void scibDmaWrite64BitAddr(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32  wordsArray[]/* according to numOfWords */
);

/**
* @internal scibRemoteTraceInfo function
* @endinternal
*
* @brief   Function to get trace strings that WM wants to log.
*
* @param[in] str                   - string to log
*
* @retval WM_OK                    - on success
* @retval WM_FAIL                  - on error
*/
void scibRemoteTraceInfo
(
    IN const char *str
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __scibh */




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
* @file sdistributed.h
*
* @brief This module is the distribution manager of simulation.
*
*
* @version   4
********************************************************************************
*/
#ifndef __sdistributedh
#define __sdistributedh

#include <os/simTypes.h>
#include <asicSimulation/SCIB/scib.h>
/* next needed for H file <asicSimulation/SDistributed/new_message.h> */
#define USE_GT_TYPES
#include <asicSimulation/SDistributed/new_message.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************/
/* next relevant only to broker and to application connected to it */
/*******************************************************************/
#define BROKER_USE_SOCKET       1
#define BROKER_NOT_USE_SOCKET   2
/* simulation flag indicating that we need to treat DMA memory as shared memory */
extern GT_U32   brokerDmaMode;
/* simulation flag indicating that we need to treat interrupt with signal instead send to socket */
extern GT_U32   brokerInterruptMode;

/* the mode of interrupt mask for application that connected to broker. */
extern BROKER_INTERRUPT_MASK_MODE   sdistAppViaBrokerInterruptMaskMode;
/****************************************************/
/* end of broker and to application connected to it */
/****************************************************/


/**
* @struct SIM_DISTRIBUTED_INIT_DEVICE_STC
 *
 * @brief Describe the parameters for the initialization of device
*/
typedef struct{

    /** ID of device, which is equal to PSS Core API device ID */
    GT_U32 deviceId;

    /** Physical device Id. */
    GT_U32 deviceHwId;

    /** interrupt line of the device. */
    GT_U32 interruptLine;

    /** (GT_BOOL) is PP of FA */
    GT_U32 isPp;

    /** device enable/disable address completion */
    GT_U32 addressCompletionStatus;

    /** @brief is nic device 0
     *  Comments:
     */
    GT_U32 nicDevice;

} SIM_DISTRIBUTED_INIT_DEVICE_STC;


/**
* @enum DEBUG_SIM_DISTRIBUTED_PERFORMANCE_CHECK_ENT
 *
 * @brief list the different type of performance tests. --- debug utility
*/
typedef enum{

    /** @brief :
     *  test the performance of read register from application to asic.
     */
    DEBUG_SIM_DISTRIBUTED_PERFORMANCE_CHECK_REGISTER_READ_E,

    /** @brief :
     *  test the performance of write register from application to asic.
     */
    DEBUG_SIM_DISTRIBUTED_PERFORMANCE_CHECK_REGISTER_WRITE_E,

    /** @brief :
     *  test the performance of write register from application to asic.
     *  then read the register that was just written, and check that read
     *  value match the write value
     */
    DEBUG_SIM_DISTRIBUTED_PERFORMANCE_CHECK_REGISTER_WRITE_AND_READ_E,

} DEBUG_SIM_DISTRIBUTED_PERFORMANCE_CHECK_ENT;

/**
* @enum DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_ENT
 *
 * @brief list the different type of TRACE actions. --- debug utility
*/
typedef enum{

    /** @brief :
     *  action is to add a flag to TRACE
     */
    DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_ADD_E,

    /** @brief :
     *  action is to remove a flag from TRACE
     */
    DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_REMOVE_E,

    /** @brief :
     *  action is to add all flags to TRACE
     *  DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_REMOVE_ALL_E:
     *  action is to remove all flags from TRACE
     */
    DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_ADD_ALL_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_REMOVE_ALL_E,

} DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_ENT;

/**
* @enum DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_ENT
 *
 * @brief list the different type of TRACE flags. --- debug utility
*/
typedef enum{

    /** @brief :
     *  trace flag for read/write registers actions
     */
    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_REGISTER_E,

    /** @brief :
     *  trace flag for read/write dma actions
     */
    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_DMA_E,

    /** @brief :
     *  trace flag for interrupt set action
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_NIC_RX_E:
     *  trace flag for nic rx frame send action
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_NIC_TX_E:
     *  trace flag for nic tx frame send action
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_SEQUENCE_NUM_E:
     *  trace flag for sequence number
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_CONNECTION_INIT_E:
     *  trace flag for connection init action
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_REMOTE_INIT_E:
     *  trace flag for remote init action
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_SOCKET_ID_E:
     *  trace flag for socketId on which we send/receive
     *  DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_DATA_READ_WRITE_E:
     *  trace flag for the data the read/write register/dma did
     */
    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_INTERRUPT_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_NIC_RX_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_NIC_TX_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_SEQUENCE_NUM_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_CONNECTION_INIT_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_REMOTE_INIT_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_SOCKET_ID_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_DATA_READ_WRITE_E,

    DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_FULL_MSG_E,

} DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_ENT;

/**
* @internal simDistributedInit function
* @endinternal
*
* @brief   Init the simulation distribution functionality.
*         initialization is done according to global parameter sasicgSimulationRole
*/
void simDistributedInit
(
    void
);

/**
* @internal simDistributedExit function
* @endinternal
*
* @brief   Exit (Kill) the simulation distribution functionality.
*/
void simDistributedExit
(
    void
);

/**
* @internal simDistributedRemoteInit function
* @endinternal
*
* @brief   do remote Asic init function. --> direction is asic to application
*         function should be called only on the Asic side
* @param[in] numOfDevices             - number of device to initialize
* @param[in] devicesArray             - devices array , and info.
* @param[in] clientIndex             - application client index
*/
void simDistributedRemoteInit
(
    IN GT_U32   numOfDevices,
    IN SIM_DISTRIBUTED_INIT_DEVICE_STC *devicesArray,
    IN GT_U32   clientIndex
);

/**
* @internal simDistributedRegisterRead function
* @endinternal
*
* @brief   Read Skernel memory function. --> direction is cpu from asic
*         function should be called only on the cpu side
* @param[in] accessType               - Define access operation Read or Write.
* @param[in] deviceId                 - device id.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - Size of ASIC memory to read or write.
*
* @param[out] memPtr                   - pointer to application memory in which
*                                      ASIC memory will be copied.
*/
void simDistributedRegisterRead
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr
);

/**
* @internal simDistributedRegisterWrite function
* @endinternal
*
* @brief   write Skernel memory function. --> direction is cpu to asic
*         function should be called only on the cpu side
* @param[in] accessType               - Define access operation Read or Write.
* @param[in] deviceId                 - device id.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - Size of ASIC memory to read or write.
* @param[in] memPtr                   - For Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
*/
void simDistributedRegisterWrite
(
    IN SCIB_MEMORY_ACCESS_TYPE accessType,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr
);


/**
* @internal simDistributedDmaRead function
* @endinternal
*
* @brief   Read HOST CPU DMA memory function. --> direction is asic from cpu
*         function should be called only on the asic side
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - Address in HOST DMA memory.
* @param[in] memSize                  - Size of DMA memory to read .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - pointer to application memory in which
*                                      ASIC memory will be copied.
*/
void simDistributedDmaRead
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal simDistributedDmaWrite function
* @endinternal
*
* @brief   write HOST CPU DMA memory function. --> direction is asic to cpu
*         function should be called only on the asic side
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - Address in HOST DMA memory.
* @param[in] memSize                  - Size of ASIC memory to read or write.
* @param[in,out] memPtr                   - For Write this pointer to application memory,which
*                                      will be copied to the ASIC memory .
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void simDistributedDmaWrite
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    INOUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);


/**
* @internal simDistributedNicTxFrame function
* @endinternal
*
* @brief   This function transmits an Ethernet packet from the NIC of CPU.
*         function should be called only on the cpu side
* @param[in] frameLength              - length of frame.
* @param[in] framePtr                 - pointer the frame (array of bytes).
*                                       none
*/
void simDistributedNicTxFrame
(
    IN GT_U32       frameLength,
    IN GT_U8        *framePtr
);

/**
* @internal simDistributedNicRxFrame function
* @endinternal
*
* @brief   This function transmit an Ethernet packet to the NIC of CPU.
*         function should be called only on the asic side
* @param[in] frameLength              - length of frame.
* @param[in] framePtr                 - pointer the frame (array of bytes).
*                                       none
*/
void simDistributedNicRxFrame
(
    IN GT_U32       frameLength,
    IN GT_U8        *framePtr
);

/**
* @internal simDistributedInterruptSet function
* @endinternal
*
* @brief   Set interrupt line for a device function. --> direction is cpu from asic
*         function should be called only on the application side
* @param[in] deviceId                 - the device id that set the interrupt
*/
void simDistributedInterruptSet
(
    IN  GT_U32        deviceId
);

/**
* @internal simDistributedRegisterDma function
* @endinternal
*
* @brief   register DMA info to broker --> direction is application to broker
*
* @param[in] startAddress             - DMA start address
* @param[in] size                     -  of DMA in bytes
* @param[in] key                      - the  of shared memory that represent the DMA
* @param[in] dmaMode                  - the DMA mode (socket/shared memory)
*/
void simDistributedRegisterDma
(
    IN GT_U32  startAddress,
    IN GT_U32  size,
    IN GT_SH_MEM_KEY  key,
    IN BROKER_DMA_MODE dmaMode
);

/**
* @internal simDistributedRegisterInterrupt function
* @endinternal
*
* @brief   register Interrupt info to broker --> direction is application to broker
*
* @param[in] interruptLineId          - interrupt line ID that when device triggers it ,
*                                      the broker will signal the application with the signalId
* @param[in] signalId                 - the signal ID to send on 'Interrupt set'
*                                      mode          - broker interrupt mode , one of BROKER_INTERRUPT_MASK_MODE
*/
void simDistributedRegisterInterrupt
(
    IN GT_U32  interruptLineId,
    IN GT_U32  signalId,
    IN BROKER_INTERRUPT_MASK_MODE  maskMode
);

/**
* @internal simDistributedInterruptUnmask function
* @endinternal
*
* @brief   send "unmask interrupt" message : application send to the
*         broker request that broker will be able to signal
*         application on interrupt.
*         NOTE : the issue is that once the broker signal the application
*         about interrupt, the broker will not signal it again even if
*         received another interrupt from device , until application
*         will send MSG_TYPE_UNMASK_INTERRUPT
*         -- see modes of BROKER_INTERRUPT_MASK_MODE --
* @param[in] interruptLineId          - interrupt line ID that when device triggers it ,
*                                      the broker will signal the application with the signalId
*                                      may use value ALL_INTERRUPT_LINES_ID
*/
void simDistributedInterruptUnmask
(
    IN GT_U32 interruptLineId
);

/**
* @internal simDistributedInterruptMask function
* @endinternal
*
* @brief   send "mask interrupt" message : application send to the
*         broker request that broker will NOT be able to signal
*         application on interrupt.
*         NOTE : see modes of BROKER_INTERRUPT_MASK_MODE
* @param[in] interruptLineId          - interrupt line ID that when device triggers it ,
*                                      the broker will NOT signal the application with the
*                                      signalId
*                                      may use value ALL_INTERRUPT_LINES_ID
*/
void simDistributedInterruptMask
(
    IN GT_U32 interruptLineId
);


/**
* @internal debugSimDistributedPerformanceCheck function
* @endinternal
*
* @brief   debug function to test performances . debug utility
*
* @param[in] type                     - one of DEBUG_SIM_DISTRIBUTED_PERFORMANCE_CHECK_ENT
* @param[in] loopSize                 - number of loops to read/write
* @param[in] address                  - the  of the register to read/write
*
* @retval GT_OK                    - check was done , successfully
* @retval GT_BAD_STATE             - function called not on application side
* @retval GT_GET_ERROR             - read after write failed , the read value was different
*                                       then the write value
* @retval GT_BAD_PARAM             - bad type value
*/
GT_STATUS debugSimDistributedPerformanceCheck
(
    IN GT_U32   type,
    IN GT_U32   loopSize,
    IN GT_U32   address
);

/**
* @internal debugSimDistributedTraceSet function
* @endinternal
*
* @brief   debug function to set the flags of the trace printings . debug utility
*
* @param[in] actionType               - the action type : one of DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_ENT
* @param[in] flag                     - the trace  to set : one of DEBUG_SIM_DISTRIBUTED_TRACE_FLAG_ENT
*                                      relevant when actionType is:
*                                      DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_ADD_E or
*                                      DEBUG_SIM_DISTRIBUTED_TRACE_ACTION_REMOVE_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad actionType or flag
*/
GT_STATUS debugSimDistributedTraceSet
(
    IN GT_U32   actionType,
    IN GT_U32   flag
);

/**
* @internal debugSimDistributedSet function
* @endinternal
*
* @brief   debug function to set/unset the option to have debug printings . debug utility
*
* @param[in] usedDebug                - 0 - don't use debug printings (trace)
*                                      otherwise - use debug printings (trace)
*
* @retval GT_OK                    - on success
*/
GT_STATUS debugSimDistributedSet
(
    IN GT_U32   usedDebug
);

/**
* @internal simDistributedRemoteDebugLevelSend function
* @endinternal
*
* @brief   application send to single/all the distributed part(s) on the system
*         the 'Debug level set' message
* @param[in] mainTarget               - the target of the message (broker/bus/device(s)/all)
*                                      on of TARGET_OF_MESSAGE_ENT
* @param[in] secondaryTarget          - the more specific target with in the main target (mainTarget)
*                                      may be 0..0xfffffffe for specific ID of the secondary target
*                                      may be ALL_SECONDARY_TARGETS means 'all' secondary targets
*                                      for example when there are several devices processes,
*                                      we can distinguish between devices by the board part
* @param[in] secondaryTarget          or set secondaryTarget to
*                                      ALL_SECONDARY_TARGETS to apply to all board parts
* @param[in] debugLevel               - the debug level bmp
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when parameter not valid to this part of simulation
*/
GT_STATUS simDistributedRemoteDebugLevelSend
(
    IN TARGET_OF_MESSAGE_ENT  mainTarget,
    IN GT_U32                 secondaryTarget,
    IN GT_U32                 debugLevel
);

/**
* @internal simDistributedRemoteDebugLevelSendAll function
* @endinternal
*
* @brief   application send to all the distributed parts on the system
*         the FULL bmp of 'Debug level set' message
* @param[in] doDebug                  - open or close debug level
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when parameter not valid to this part of simulation
*/
GT_STATUS simDistributedRemoteDebugLevelSendAll
(
    IN GT_U32   doDebug
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sdistributedh */




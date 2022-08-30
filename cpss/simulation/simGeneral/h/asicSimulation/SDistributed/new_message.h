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
* @file new_message.h
*
* @brief This file defines the messages format send between the two sides of
* distributed simulation.
*
* @version   2
********************************************************************************
*/
#ifndef _NEW_MESSAGE_H_
#define _NEW_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* check if we can use GT_ types */
#ifdef USE_GT_TYPES
    #define UINT32  GT_U32
#endif /*USE_GT_U32*/


/* define variable type .
all variables sent on messages must has same size */
#define VARIABLE_TYPE       UINT32
#define PLACE_HOLDER_TYPE   VARIABLE_TYPE*

/**
* @enum MSG_TYPE
 *
 * @brief list the different type of messages
*/
typedef enum{

    /** : read register request , sent from Application to Device */
    MSG_TYPE_READ,

    /** : write register request , sent from Application to Device */
    MSG_TYPE_WRITE,

    /** : read register response, sent from Device to Application */
    MSG_TYPE_READ_RESPONSE,

    /** : read DMA request , sent from Device to Application */
    MSG_TYPE_DMA_READ,

    /** : write DMA request , sent from Device to Application */
    MSG_TYPE_DMA_WRITE,

    /** : read DMA response, sent from Application to Device */
    MSG_TYPE_DMA_READ_RESPONSE,

    MSG_TYPE_INTERRUPT_SET,

    /** @brief : send Ethernet frame from CPU's Nic to ingress the Device,
     *  sent from Application to Device
     */
    MSG_TYPE_NIC_TX_FRAME,

    /** @brief : send Ethernet frame to CPU's Nic to egress the Device'
     *  sent from Device to Application
     */
    MSG_TYPE_NIC_RX_FRAME,

    /** : initialization parameters , sent from Device to Application. */
    MSG_TYPE_INIT_PARAMS,

    /** @brief : Reset message from the application to all
     *  distributed processes in the system
     */
    MSG_TYPE_INIT_RESET,

    /** @brief : set debug level of the distributed mechanism
     *  the following messages are exchanged only between application(s) and broker
     */
    MSG_TYPE_DEBUG_LEVEL_SET,

    /** @brief application send to the broker it's
     *  processId , so broker can signal application on interrupt
     */
    MSG_TYPE_APP_PID = 50,

    /** @brief application send to the broker
     *  info about a DMA shared memory. So broker can read/write from
     *  this shared memory without the application knowledge.
     */
    MSG_TYPE_REGISTER_DMA,

    /** @brief application send to the
     *  broker request to register itself as one that want to be
     *  signaled when interrupt arrive.
     *  signaled when interrupt arrive.
     */
    MSG_TYPE_REGISTER_INTERRUPT,

    /** @brief application send to the
     *  broker request that broker will be able to signal
     *  application on interrupt.
     *  NOTE : the issue is that once the broker signal the application
     *  about interrupt, the broker will not signal it again even if
     *  received another interrupt from device , until application
     *  will send MSG_TYPE_UNMASK_INTERRUPT
     *  NOTE : see mode of BROKER_INTERRUPT_MASK_MODE
     */
    MSG_TYPE_UNMASK_INTERRUPT,

    /** @brief application send to the
     *  broker request that broker will NOT be able to signal
     *  application on interrupt.
     *  NOTE : the issue is that once the broker signal the application
     *  about interrupt, the application wants to silent the broker.
     *  application will 'unmask' the broker when application ready
     *  for new signal.
     *  NOTE : see mode of BROKER_INTERRUPT_MASK_MODE
     */
    MSG_TYPE_MASK_INTERRUPT,

    MSG_TYPE_LAST

} MSG_TYPE;

/**
* @enum READ_WRITE_ACCESS_TYPE
 *
 * @brief list the different register accessing
*/
typedef enum{

    /** Device's registers (via SMI/PCI/PEX/TWSI...) */
    REGISTER_MEMORY_ACCESS,

    /** PCI/PEX configuration registers */
    PCI_REGISTERS_ACCESS

} READ_WRITE_ACCESS_TYPE;

/**
* @struct MSG_HDR
 *
 * @brief Describe the header of the messages send via sockets
*/
typedef struct{

    /** : the message , one of the : MSG_TYPE */
    VARIABLE_TYPE type;

    /** @brief : length of message excluding Hdr in Bytes
     *  Comments:
     */
    VARIABLE_TYPE msgLen;

} MSG_HDR;

/**
* @struct WRITE_MSG
 *
 * @brief Describe the "write register" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** device id. (of the device in the simulation) */
    VARIABLE_TYPE deviceId;

    /** @brief Define access operation Read or Write.
     *  one of READ_WRITE_ACCESS_TYPE
     */
    VARIABLE_TYPE accessType;

    /** number of registers to write. */
    VARIABLE_TYPE writeLen;

    /** Address of first register to write. */
    VARIABLE_TYPE address;

    /** @brief (place holder) start here to place the data to write
     *  Comments:
     *  sent from Application side to Device side.
     */
    PLACE_HOLDER_TYPE dataPtr;

} WRITE_MSG;

/**
* @struct READ_MSG
 *
 * @brief Describe the "read register" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** device id. (of the device in the simulation) */
    VARIABLE_TYPE deviceId;

    /** @brief Define access operation Read or Write.
     *  one of READ_WRITE_ACCESS_TYPE
     */
    VARIABLE_TYPE accessType;

    /** number of registers to read. */
    VARIABLE_TYPE readLen;

    /** @brief Address of first register to read.
     *  Comments:
     *  sent from Application side to Device side.
     *  the reply to this message will be in the format of READ_RESPONSE_MSG
     *  (from Device to application)
     */
    VARIABLE_TYPE address;

} READ_MSG;

/**
* @struct READ_RESPONSE_MSG
 *
 * @brief Describe the "reply to read register" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** number of registers read. */
    VARIABLE_TYPE readLen;

    /** @brief (place holder) start here to place the data that read
     *  Comments:
     *  sent from Device side to Application side.(as response to message
     *  READ_MSG)
     */
    PLACE_HOLDER_TYPE dataPtr;

} READ_RESPONSE_MSG;

/**
* @struct DMA_WRITE_MSG
 *
 * @brief Describe the "write DMA" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** number of "words" (4 bytes) to write. */
    VARIABLE_TYPE writeLen;

    /** Address of first "word" to write. */
    VARIABLE_TYPE address;

    /** @brief (place holder) start here to place the data to write
     *  Comments:
     *  sent from Device side to Application side.
     */
    PLACE_HOLDER_TYPE dataPtr;

} DMA_WRITE_MSG;

/**
* @struct DMA_READ_MSG
 *
 * @brief Describe the "read DMA" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** number of "words" (4 bytes) to read. */
    VARIABLE_TYPE readLen;

    /** @brief Address of first "word" to read.
     *  Comments:
     *  sent from Device side to Application side.
     *  the reply to this message will be in the format of DMA_READ_RESPONSE_MSG
     *  (from Application to Device)
     */
    VARIABLE_TYPE address;

} DMA_READ_MSG;

/**
* @struct DMA_READ_RESPONSE_MSG
 *
 * @brief Describe the "reply to Read DMA" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** number of "words" (4 bytes) read. */
    VARIABLE_TYPE readLen;

    /** @brief (place holder) start here to place the data that read
     *  Comments:
     *  sent from Application side to Device side.(as response to message
     *  DMA_READ_MSG)
     */
    PLACE_HOLDER_TYPE dataPtr;

} DMA_READ_RESPONSE_MSG;

/**
* @struct INTERRUPT_SET_MSG
 *
 * @brief Describe the "set interrupt" message
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief device id. (of the device in the simulation)
     *  Comments:
     *  Sent from Device Side to Application side
     */
    VARIABLE_TYPE deviceId;

} INTERRUPT_SET_MSG;

/**
* @struct INIT_PARAM_SINGLE_DEVICE_INFO
 *
 * @brief Describe the Device init parameters (per device)
*/
typedef struct{

    /** device id. (of the device in the simulation) */
    VARIABLE_TYPE deviceId;

    /** Physical device Id. */
    VARIABLE_TYPE deviceHwId;

    /** interrupt line of the device. */
    VARIABLE_TYPE interruptLine;

    /** 1 is PP , 0 */
    VARIABLE_TYPE isPp;

    /** device enable/disable address completion */
    VARIABLE_TYPE addressCompletionStatus;

    /** @brief is nic device 0
     *  Comments:
     */
    VARIABLE_TYPE nicDevice;

} INIT_PARAM_SINGLE_DEVICE_INFO;

/**
* @struct INIT_PARAM_MSG
 *
 * @brief Describe the header of the initialization parameters of a device.
 * message sent from Device to Application
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief the info about the device
     *  Comments:
     */
    INIT_PARAM_SINGLE_DEVICE_INFO deviceInfo;

} INIT_PARAM_MSG;

/**
* @struct RESET_PARAM_MSG
 *
 * @brief Describe the header of the Reset message.
 * message sent from Application to all distributed part of system
*/
typedef struct{

    /** @brief the header that all messages start with
     *  deviceInfo  - the info about the device
     *  Comments:
     */
    MSG_HDR hdr;

} RESET_PARAM_MSG;

/**
* @enum TARGET_OF_MESSAGE_ENT
 *
 * @brief define all types of target parts in the system
*/
typedef enum{

    /** application part */
    TARGET_OF_MESSAGE_APPLICATION_E,

    /** broker part */
    TARGET_OF_MESSAGE_BROKER_E,

    /** bus part */
    TARGET_OF_MESSAGE_BUS_E,

    /** device part */
    TARGET_OF_MESSAGE_DEVICE_E,

    /** all parts */
    TARGET_OF_MESSAGE_ALL_E,

} TARGET_OF_MESSAGE_ENT;

/* value for the field of secondaryTarget that indicate ALL secondary targets */
#define ALL_SECONDARY_TARGETS   0xffffffff

/**
* @struct DEBUG_LEVEL_MSG
 *
 * @brief Describe the "debug level " message : application send to the
 * message sent from Application to all distributed part of system
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief the target of the message (broker/bus/device(s)/all)
     *  on of TARGET_OF_MESSAGE_ENT
     */
    VARIABLE_TYPE mainTarget;

    /** @brief the more specific target with in the main target (mainTarget)
     *  may be 0..0xfffffffe for specific ID of the secondary target
     *  may be ALL_SECONDARY_TARGETS means 'all' secondary targets
     *  for example when there are several devices processes,
     *  we can distinguish between devices by the board part
     *  (secondaryTarget) or set secondaryTarget to
     *  ALL_SECONDARY_TARGETS to apply to all board parts
     */
    VARIABLE_TYPE secondaryTarget;

    /** @brief the debug level bmp
     *  Comments:
     */
    VARIABLE_TYPE debugLevel;

} DEBUG_LEVEL_MSG;

/**
* @struct APP_PID_MSG
 *
 * @brief Describe the "application process ID" message : application send to the
 * broker it's processId , so broker can signal application on interrupt
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief application's process Id
     *  Comments:
     *  Sent from Application Side to broker side
     */
    VARIABLE_TYPE processId;

} APP_PID_MSG;

/**
* @enum BROKER_DMA_MODE
 *
 * @brief Describe the "DMA mode" : how broker handle read/write DMA messages
*/
typedef enum{

    /** the broker will send the message to the application */
    BROKER_DMA_MODE_SOCKET,

    /** @brief the broker will do read/write DMA from
     *  the shared memory
     */
    BROKER_DMA_MODE_SHARED_MEMORY

} BROKER_DMA_MODE;

/**
* @struct REGISTER_DMA_MSG
 *
 * @brief Describe the "register DMA" message : application send to the
 * broker info about a DMA shared memory. So broker can read/write from
 * this shared memory without the application knowledge.
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** the start address of the DMA memory */
    VARIABLE_TYPE startAddress;

    /** number of bytes the DMA uses */
    VARIABLE_TYPE size;

    /** the key that represents the DMA memory (shared memory) */
    VARIABLE_TYPE key;

    /** @brief the DMA mode : broker send message via socket to
     *  application or broker do read/write DMA from shared
     *  memory (one of BROKER_DMA_MODE)
     *  Comments:
     *  Sent from Application Side to broker side
     */
    VARIABLE_TYPE dmaMode;

} REGISTER_DMA_MSG;

/**
* @enum BROKER_INTERRUPT_MASK_MODE
 *
 * @brief Describe the "Interrupt mask mode" : how broker handle interrupt messages
*/
typedef enum{

    /** @brief the broker
     *  will signal application on interrupt , and then will state itself
     *  into 'Masked state' , meaning that it can't signal the application
     *  until receiving 'Unmask interrupt' message
     */
    INTERRUPT_MODE_BROKER_AUTOMATICALLY_MASK_INTERRUPT_LINE,

    /** @brief the broker
     *  will signal application on interrupt , the broker will NOT state
     *  itself into 'Masked state'
     *  in order for the broker to be in 'Masked state' , application
     *  need to send message 'Maks interrupt'
     */
    INTERRUPT_MODE_BROKER_USE_MASK_INTERRUPT_LINE_MSG

} BROKER_INTERRUPT_MASK_MODE;

/* value to treat all interrupt lines regardless of their value */
#define ALL_INTERRUPT_LINES_ID      0xffeeddcc

/**
* @struct REGISTER_INTERRUPT_MSG
 *
 * @brief Describe the "register interrupt" message : application send to the
 * broker request to register itself as one that want to be
 * signaled when interrupt arrive.
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief interrupt line ID that when device triggers it ,
     *  the broker will signal the application with the signalId
     *  may use value ALL_INTERRUPT_LINES_ID
     */
    VARIABLE_TYPE interruptLineId;

    /** the signal ID to send on 'Interrupt set' */
    VARIABLE_TYPE signalId;

    /** @brief broker interrupt mask mode , one of BROKER_INTERRUPT_MASK_MODE
     *  Comments:
     *  Sent from Application Side to broker side
     */
    VARIABLE_TYPE maskMode;

} REGISTER_INTERRUPT_MSG;

/**
* @struct UNMASK_INTERRUPT_MSG
 *
 * @brief Describe the "unmask interrupt" message : application send to the
 * broker request that broker will be able to signal
 * application on interrupt.
 * NOTE : the issue is that once the broker signal the application
 * about interrupt, the broker will not signal it again even if
 * received another interrupt from device , until application
 * will send MSG_TYPE_UNMASK_INTERRUPT
 * -- see modes of BROKER_INTERRUPT_MASK_MODE --
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief interrupt line ID that when device triggers it ,
     *  the broker will signal the application with the signalId
     *  may use value ALL_INTERRUPT_LINES_ID
     *  Comments:
     *  Sent from Application Side to broker side
     */
    VARIABLE_TYPE interruptLineId;

} UNMASK_INTERRUPT_MSG;

/**
* @struct MASK_INTERRUPT_MSG
 *
 * @brief Describe the "mask interrupt" message : application send to the
 * broker request that broker will NOT be able to signal
 * application on interrupt.
 * NOTE : see modes of BROKER_INTERRUPT_MASK_MODE
*/
typedef struct{

    /** the header that all messages start with */
    MSG_HDR hdr;

    /** @brief interrupt line ID that when device triggers it ,
     *  the broker will NOT signal the application with the
     *  signalId
     *  may use value ALL_INTERRUPT_LINES_ID
     *  Comments:
     *  Sent from Application Side to broker side
     */
    VARIABLE_TYPE interruptLineId;

} MASK_INTERRUPT_MSG;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NEW_MESSAGE_H_ */



/*******************************************************************************
*              (c), Copyright 2020, Marvell International Ltd.                 *
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
* @file wmApi.h
*
* @brief This is a API definition for WM external access.
*
* @version   1
********************************************************************************
*/
#ifndef __wmAPi_h
#define __wmAPi_h

#include <os/simTypes.h>
#include <os/simTypesBind.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef GT_U32 WM_STATUS; /* see applicable values below */
#define WM_OK       0
#define WM_ERROR    1

/**
* @internal wmMemPciConfigSpaceRead function
* @endinternal
*
* @brief   WM memory function to read register(s) from the PCIe configuration space.
*
* @param[in] pciBus      - the bus ID.
* @param[in] pciDev      - the device ID on the bus.
* @param[in] pciFunc     - the function ID in the device.
* @param[in] addr        - the address in the configuration space.
* @param[out] valuePtr   - (pointer to) the read value
*
*/
WM_STATUS wmMemPciConfigSpaceRead(
    IN  GT_U32 pciBus,
    IN  GT_U32 pciDev,
    IN  GT_U32 pciFunc,
    IN  GT_U32 addr,
    OUT GT_U32 *valuePtr
);

/**
* @internal wmMemPciConfigSpaceWrite function
* @endinternal
*
* @brief   WM memory function to write register(s) to the PCIe configuration space.
*
* @param[in] pciBus      - the bus ID.
* @param[in] pciDev      - the device ID on the bus.
* @param[in] pciFunc     - the function ID in the device.
* @param[in] addr        - the address in the configuration space.
* @param[in] value       - the value to write
*
*/
WM_STATUS wmMemPciConfigSpaceWrite(
    IN GT_U32 pciBus,
    IN GT_U32 pciDev,
    IN GT_U32 pciFunc,
    IN GT_U32 addr,
    IN GT_U32 value
);

/**
* @internal wmMemPciRead function
* @endinternal
*
* @brief   WM memory function to read register(s) from the device via PCIe 'bus' on address that
*           match one of the 'BARs'.
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to read from the address
* @param[out] wordsArray[]            - array of words fill with the read values
*
*/
WM_STATUS wmMemPciRead(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    OUT GT_U32  wordsArray[]/* according to numOfWords */
);

/**
* @internal wmMemPciWrite function
* @endinternal
*
* @brief   WM memory function to write register(s) to the device via PCIe 'bus' on address that
*           match one of the 'BARs'
*
* @param[in] addr_high                - the high 32bits of the address.
* @param[in] addr_low                 - the low  32bits of the address.
* @param[in] numOfWords               - number of consecutive 32bits words to write from the address
* @param[in] wordsArray[]             - array of words fill with the values to write
*
*/
WM_STATUS wmMemPciWrite(
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32   wordsArray[]/* according to numOfWords */
);


/**
* @internal wmMemSmiRead function
* @endinternal
*
* @brief   WM memory function to read register from the device via SMI bus.
*
* @param[in] smiBusId                - the SMI bus ID.
* @param[in] smiAddr                 - the address on the SMI bus.
* @param[out] valuePtr               - (pointer to) the value that was read
*
*/
WM_STATUS wmMemSmiRead(
    IN GT_U32   smiBusId,
    IN GT_U32   smiAddr,
    OUT GT_U16  *valuePtr
);

/**
* @internal wmMemSmiWrite function
* @endinternal
*
* @brief   WM memory function to write register to the device via SMI bus.
*
* @param[in] smiBusId                - the SMI bus ID.
* @param[in] smiAddr                 - the address on the SMI bus.
* @param[in] value                   - the value to write
*
*/
WM_STATUS wmMemSmiWrite(
    IN GT_U32   smiBusId,
    IN GT_U32   smiAddr,
    IN GT_U16   value
);

/**
* @internal wmTrafficIngressPacket function
* @endinternal
*
* @brief   WM traffic function to ingress packet into a port in a device for processing.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] numOfBytes              - the munber of bytes in the packet.
* @param[in] packetPtr               - (pointer to) array of bytes of the packet
*
*   NOTE:
*       0. if the port hold SLAN , this function should NOT be called (but the WM will process it !)
*       1. the caller is responsible to free the packet memory.
*       2. the caller to this function may free the memory of the packet as soon
*       as the function ends.
*       3. the function responsible to 'copy' the packet for it's own usage , to it's own memory.
*/
WM_STATUS wmTrafficIngressPacket(
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
);

/*==================*/
/**
* @internal WM_DMA_PCI_READ_CB_FUNC function
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
typedef WM_STATUS (* WM_DMA_PCI_READ_CB_FUNC) (
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    OUT GT_U32  wordsArray[]/* according to numOfWords */
);

/**
* @internal WM_DMA_PCI_WRITE_CB_FUNC function
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
typedef WM_STATUS (* WM_DMA_PCI_WRITE_CB_FUNC) (
    IN GT_U32   addr_high,
    IN GT_U32   addr_low,
    IN GT_U32   numOfWords,
    IN GT_U32  wordsArray[]/* according to numOfWords */
);


/**
* @internal WM_INTERRUPT_TRIGGER_CB_FUNC function
* @endinternal
*
* @brief   prototype for function that send indication about interrupt
*
* @param[in] doAssert      - indication if to assert or de-assert the interrupt.
*                            1 - to assert the interrupt
*                            0 - to de-assert the interrupt
* @param[in] line          - The interrupt line being asserted.
* @param[in] source        - The source core when line < 32
* @param[in] flags         - Sideband signals from the device to the interrupt controller
*
*   NOTE: those parameters match the info given by 'wmDeviceIrqParamsSet' for the WM device
*/
typedef WM_STATUS (* WM_INTERRUPT_TRIGGER_CB_FUNC) (
    IN  GT_U32 doAssert,
    IN  GT_U32 line,
    IN  GT_U32 source,
    IN  GT_U32 flags
);

/**
* @internal WM_TRAFFIC_EGRESS_PACKET_CB_FUNC function
* @endinternal
*
* @brief   prototype for function to egress packet from a port in a device.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device egress port number (MAC number).
* @param[in] numOfBytes              - the munber of bytes in the packet.
* @param[in] packetPtr               - (pointer to) array of bytes of the packet
*
*   NOTE:
*       0. if the port hold SLAN , this CB will NOT be called , and the SLAN will get the packet !
*       1. the caller is responsible to free the packet memory.
*       2. the caller to this function may free the memory of the packet as soon
*       as the function ends.
*       3. the function responsible to 'copy' the packet for it's own usage , to it's own memory.
*/
typedef WM_STATUS (* WM_TRAFFIC_EGRESS_PACKET_CB_FUNC) (
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN GT_U32   numOfBytes,
    IN char*    packetPtr /* pointer to start of packet (network order) according to 'numOfBytes' */
);

/**
* @internal WM_TRACE_INFO_CB_FUNC function
* @endinternal
*
* @brief   Function to get trace strings that WM wants to log.
*
* @param[in] str                   - string to log
*
* @retval WM_OK                    - on success
* @retval WM_FAIL                  - on error
*/
typedef WM_STATUS (*WM_TRACE_INFO_CB_FUNC)(
    IN const char *str
);

/*==================*/
/*
 * Typedef: struct WM_BIND_CB_FUNC_STC
 *
 * Description:
 *      Hold the call back (CB) functions that the WM need to access the outer world:
 *      read/write DMA/DRAM , trigger interrupt , egress packet from a port
 *
 * Fields:
 *      dmaReadFunc          : a CB function for DMA read
 *      dmaWriteFunc         : a CB function for DMA write
 *      interruptTriggerFunc : a CB function for interrupt triggering
 *      egressPacketFunc     : a CB function for egress packet from a port in device.
 *      traceLogFunc         : a CB function for trace strings that WM wants to log.
 *                             can be NULL function if the LOG should be in the 'file system' (legacy mode)
 *
 * Comments:
 */
typedef struct{
    WM_DMA_PCI_READ_CB_FUNC             dmaReadFunc;
    WM_DMA_PCI_WRITE_CB_FUNC            dmaWriteFunc;
    WM_INTERRUPT_TRIGGER_CB_FUNC        interruptTriggerFunc;
    WM_TRAFFIC_EGRESS_PACKET_CB_FUNC    egressPacketFunc;
    WM_TRACE_INFO_CB_FUNC               traceLogFunc;

}WM_BIND_CB_FUNC_STC;

/**
* @internal wmBindCbFunctions function
* @endinternal
*
* @brief   function to bind the WM with call back (CB) functions.
*
* @param[in] cbFuncInfoPtr            - (pointer to) info about the CB functions needed by the device.
*
*/
WM_STATUS wmBindCbFunctions(
    IN  WM_BIND_CB_FUNC_STC *cbFuncInfoPtr
);

/**
* @internal wmDeviceOnPciBusGet function
* @endinternal
*
* @brief   The function check if the {pciBus,pciDev,pciFunc} are associated with
*          any of the WM devices.
*          by default this association done by the parameters in the INI file,
*          but can be set in runtime by 'wmDeviceOfPciBusSet(...)'
*
* @param[in] pciBus         - the bus ID.
* @param[in] pciDev         - the device ID on the bus.
* @param[in] pciFunc        - the function ID in the device.
* @param[out] wmDeviceIdPtr - (pointer to) the WM deviceId if found.
*                             if not found the value is 0xFFFFFFFF
*
*/
WM_STATUS wmDeviceOnPciBusGet(
    IN  GT_U32 pciBus,
    IN  GT_U32 pciDev,
    IN  GT_U32 pciFunc,
    OUT GT_U32 *wmDeviceIdPtr
);

/**
* @internal wmDeviceOnPciBusSet function
* @endinternal
*
* @brief   The function associate te WM device with the {pciBus,pciDev,pciFunc} .
*          by default this association done by the parameters in the INI file.
*
* @param[in] wmDeviceId     - the WM deviceId
* @param[in] pciBus         - the bus ID.
* @param[in] pciDev         - the device ID on the bus.
* @param[in] pciFunc        - the function ID in the device.
*
*/
WM_STATUS wmDeviceOnPciBusSet(
    IN  GT_U32 wmDeviceId,
    IN  GT_U32 pciBus,
    IN  GT_U32 pciDev,
    IN  GT_U32 pciFunc
);

/**
* @internal wmDeviceIrqParamsSet function
* @endinternal
*
* @brief   The function associate the WM device with the IRQ info {line,source,flags} ,
*          by default 'line' comes from the INI file. (others are ZERO)
*          those params will be called by this device on CB function of 'interruptTriggerFunc'
*
* @param[in] wmDeviceId    - the WM deviceId
* @param[in] line          - The interrupt line being asserted.
* @param[in] source        - The source core when line < 32
* @param[in] flags         - Sideband signals from the device to the interrupt controller
*
*/
WM_STATUS wmDeviceIrqParamsSet(
    IN  GT_U32 wmDeviceId,
    IN  GT_U32 line,
    IN  GT_U32 source,
    IN  GT_U32 flags
);

/*
  enum : WM_PORT_CONNECTION_TYPE_ENT

  WM_PORT_CONNECTION_TYPE_NONE_E - this port considered 'not connected'. (disconnected)
      so port will be 'link down' (unless forced to link up by configuration)
  WM_PORT_CONNECTION_TYPE_CALLBACK_E - this port considered 'connected' and
      the CB of 'egressPacketFunc' will apply on egress traffic.
      port will be 'link up' (if allowed by configuration).
  WM_PORT_CONNECTION_TYPE_PEER_PORT_E - this port considered 'connected' but it's
    egress traffic redirected to a 'peer' WM port (without calling to the CB of 'egressPacketFunc')
    (the 2 ports are considered on egress as connected to ingress of the other)
    the peer port info is in struct : peerInfo{wmDeviceId,portNum}
    NOTE: to 'break' this peer connection , disconnect one of the ports (the peer
        will also become disconnected).
*/
typedef enum{
    WM_PORT_CONNECTION_TYPE_NONE_E,
    WM_PORT_CONNECTION_TYPE_CALLBACK_E,
    WM_PORT_CONNECTION_TYPE_PEER_PORT_E,

    WM_PORT_CONNECTION_TYPE__LAST__E/*must not be used*/
}WM_PORT_CONNECTION_TYPE_ENT;
/* struct : WM_PORT_CONNECTION_INFO_STC
    hold info about the connection types of a port.
*/
typedef struct{
    WM_PORT_CONNECTION_TYPE_ENT connectionType;
    struct{/* relevant only to connectionType == WM_PORT_CONNECTION_TYPE_PEER_PORT_E */
        GT_U32  wmDeviceId;
        GT_U32  portNum;
    }peerInfo;

    /* indication that the port allow to have SLAN in addition to it's connectionType
       GT_FALSE means that even if a port hold SLAN , the SLAN will not be used
       GT_TRUE  means that if a port hold SLAN , the SLAN can ingress/egress packets regardless to 'connectionType'
    */
    GT_BOOL allowSlan;
}WM_PORT_CONNECTION_INFO_STC;

/**
* @internal wmPortConnectionInfo function
* @endinternal
*
* @brief   WM function to state the port connection info.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
* @param[in] portNum                 - the WM device ingress port number (MAC number).
* @param[in] infoPtr                 - (pointer to) info about the port connection
*
*/
WM_STATUS wmPortConnectionInfo(
    IN GT_U32   wmDeviceId,
    IN GT_U32   portNum,
    IN WM_PORT_CONNECTION_INFO_STC  *infoPtr
);


/**
* @internal wmHardResetTrigger function
* @endinternal
*
* @brief   WM function to indicate the device to do 'HARD reset'.
*          NOTEs:
*          1. the WM will forget ALL the settings that previously configured on this device.
*            including :
*                1. the PCIe config space (all registers return to 'default')
*                2. all the registers in BAR0,BAR2 (all registers/memories return to 'default')
*                3. info set by wmDeviceOnPciBusSet
*                4. info set by wmDeviceIrqParamsSet
*                5. info set by wmPortConnectionInfo about all the ports.
*          2. no need to call again to 'global functions' (that are not 'per device') :
*                1. simOsFuncBindOwnSimOs / simOsFuncBind
*                2. simulationLibInit
*                3. wmBindCbFunctions
*          3. after the function returns to the caller , the caller can re-configure the WM device.
*             meaning that function not end till the WM device finished the Hard reset.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmHardResetTrigger(
    IN GT_U32   wmDeviceId
);

/**
* @internal wmSoftResetTrigger function
* @endinternal
*
* @brief   WM function to indicate the device to do 'SOFT reset' without loosing PCIe config.
*          This API needed by the Simics although no such 'API' in the HW !
*
*          NOTEs:
*          1. the WM will forget almost ALL the settings that previously configured on this device.
*            but will NOT forget about :
*                1. the PCIe config space (all registers return to 'default')
*                2. all the registers in BAR0,BAR2 (all registers/memories return to 'default')
*                3. info set by wmDeviceOnPciBusSet
*                4. info set by wmDeviceIrqParamsSet
*                5. info set by wmPortConnectionInfo about all the ports.
*          2. no need to call again to 'global functions' (that are not 'per device') :
*                1. simOsFuncBindOwnSimOs / simOsFuncBind
*                2. simulationLibInit
*                3. wmBindCbFunctions
*          3. after the function returns to the caller , the caller can re-configure the WM device.
*             meaning that function not end till the WM device finished the Soft reset.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmSoftResetTrigger(
    IN GT_U32   wmDeviceId
);

/**
* @internal wmHarrierAllowTraffic_forDebugOnly function
* @endinternal
*
* @brief   WM 'debug only' function to allow traffic to pass in the device , before CPSS initialization.
*          this function is for debug instead of the 'micro-init : supper image'
*          the function is for Harrier device.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmAllowTraffic_forDebugOnly_harrier(
    IN GT_U32   wmDeviceId
);

/**
* @internal wmAllowTraffic_forDebugOnly_aldrin3m function
* @endinternal
*
* @brief   WM 'debug only' function to allow traffic to pass in the device , before CPSS initialization.
*          this function is for debug instead of the 'micro-init : supper image'
*          the function is for Aldrin3-M device.
*
* @param[in] wmDeviceId              - the WM device ID. (as appears in the INI file)
*
*/
WM_STATUS wmAllowTraffic_forDebugOnly_aldrin3m(
    IN GT_U32   wmDeviceId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __wmAPi_h */


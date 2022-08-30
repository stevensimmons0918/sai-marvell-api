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
* @file simSip6PhaFirmwareUtil.c
*
* @brief SIP 6 common PHA processing (programmable header modifications)
*
*   APPLICABLE DEVICES:      Falcon; AC5P; AC5X.
*
* @version   1
********************************************************************************
*/

#ifdef _VISUALC
    #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
#endif

#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
/* Needed for PPA_FW_SP_WRITE, PPA_FW_SP_SHORT_WRITE etc. which are identical to Falcon and Hawk devices */
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/simSip6Pha.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/wm_asic_sim_defs.h"



/* Number of bytes in PHA for packet header */
#define SIP6_PACKET_HEADER_MAX_COPY_SIZE   (128+32)

/* Number of WORDS in PHA for descriptor */
#define SIP6_DESC_NUM_WORDS           16  /*64 bytes*/

/* Number of WORDS in PHA for configurations */
#define SIP6_CONFIG_NUM_WORDS         (8+1)  /* 9 words*/

/* ALL addresses of instruction pointers need to be with prefix 0x00400000 */
/* the PHA table need to hold only lower 16 bits (the prefix is added internally by the HW) */
#define SIP6_FW_INSTRUCTION_DOMAIN_ADDR_CNS      0x00400000

/* Indication to read 'ZERO' from the TOD , to allow comparing with expected results*/
GT_U32  pha_fw_activeMem_read_TOD_always_ZERO = 0;






/**
* @internal invalidFirmwareThread function
* @endinternal
*
* @brief  Error indication for trying to call unregistered firmware thread
*
*/
void invalidFirmwareThread()
{
    /* Call unregistered firmware thread */
    skernelFatalError("PHA failure; try to call unregistered firmware thread \n");
}


/*******************************************************************************
*   FIRMWARE_THREAD_FUNC
*
* DESCRIPTION:
*       prototype for emulated 'firmware' function
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       descrPtr    - descriptor for the packet
*******************************************************************************/
typedef void (*FIRMWARE_THREAD_FUNC)
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32       egressPort
);


/**
* @internal copyPHaConfigToMemory function
* @endinternal
*
* @brief   put the 'PHA config'   into the PPN 'config memory'.
*/
void copyPHaConfigToMemory
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STCT_PTR  descrPtr
)
{
    /*128 bits*/
    PPA_FW_SP_WRITE(devObjPtr->PHA_FW_support.addr_CFG_REGs_lo + 0x00,descrPtr->pha.pha_template[3]);
    PPA_FW_SP_WRITE(devObjPtr->PHA_FW_support.addr_CFG_REGs_lo + 0x04,descrPtr->pha.pha_template[2]);
    PPA_FW_SP_WRITE(devObjPtr->PHA_FW_support.addr_CFG_REGs_lo + 0x08,descrPtr->pha.pha_template[1]);
    PPA_FW_SP_WRITE(devObjPtr->PHA_FW_support.addr_CFG_REGs_lo + 0x0c,descrPtr->pha.pha_template[0]);

    /* 16 bits */
    PPA_FW_SP_SHORT_WRITE(devObjPtr->PHA_FW_support.addr_CFG_REGs_lo + 0x10,(uint16_t)descrPtr->pha.pha_srcPhyPortData[0]);
    /* 16 bits */
    PPA_FW_SP_SHORT_WRITE(devObjPtr->PHA_FW_support.addr_CFG_REGs_lo + 0x12,(uint16_t)descrPtr->pha.pha_trgPhyPortData[0]);
}


/**
* @internal copyPacketHeaderToMemory function
* @endinternal
*
* @brief   put first 128 bytes of ingress packet into 'header memory' (clear to ZERO first 20 bytes)
*/
void copyPacketHeaderToMemory
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STCT_PTR  descrPtr
)
{
    DECLARE_FUNC_NAME(copyPacketHeaderToMemory);

    GT_U32  ii,iiMax;
    GT_U32  address;
    /*0x0 = EXPAND 32B; EXPAND_32B; 128B header with maximum expansion 32 bytes; Maximum expansion 32 bytes;
      0x1 = EXPAND 48B; EXPAND_48B; 112B header with maximum expansion 48 bytes; Maximum expansion 48 bytes;
      0x2 = EXPAND 64B; EXPAND_64B; 96B header with maximum expansion 64 bytes; Maximum expansion 64 bytes; */
    GT_U32  expansionSize = 32 + 16 * descrPtr->pha.pha_HeaderWindowSize;
    GT_U8   *headerPtr;

    __LOG(("expansion allow [%d] bytes (header is [%d] bytes)\n",
        expansionSize ,
        SIP6_PACKET_HEADER_MAX_COPY_SIZE - expansionSize));

    /*
    Anchor type of the PPA accessible header window
        0x0 = Outer L2 Start; Outer_L2_Start; Start of packet - offset 0; ;
        0x1 = Outer L3 Start; Outer_L3_Start; L3 start byte without tunnel start. Tunnel L3 start with tunnel start; ;
        0x2 = Inner L2 Start; Inner_L2_Start; Passenger L2 start byte. Relevant only for tunnel start packets with passenger having L2 layer; ;
        0x3 = Inner L3 Start; Inner_L3_Start; Passenger L3 start byte. Relevant only for tunnel start packets.; ;
    */
    switch(descrPtr->pha.pha_HeadeWindowAnchor)
    {
        default:
        case 0:
            __LOG(("<pha_HeadeWindowAnchor>: Outer_L2_Start; Start of packet - offset 0 \n"));
            headerPtr = devObjPtr->egressBuffer;
            break;
        case 1:
            __LOG(("<pha_HeadeWindowAnchor>: Outer_L3_Start; L3 start byte without tunnel start \n"));
            headerPtr = descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr ?
                        descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr :
                        descrPtr->haToEpclInfo.l3StartOffsetPtr;
            break;
        case 2:
            __LOG(("<pha_HeadeWindowAnchor>: Inner_L2_Start; Passenger L2 start byte. Relevant only for tunnel start packets with passenger having L2 layer \n"));
            headerPtr = descrPtr->haToEpclInfo.macDaSaPtr;
            if(headerPtr == NULL)
            {
                __LOG(("WARNING : The Inner_L2_Start Offset is not valid , use the Outer L2 offset (offset 0) \n"));
                headerPtr = devObjPtr->egressBuffer;
            }
            break;
        case 3:
            __LOG(("<pha_HeadeWindowAnchor>: Inner_L3_Start; Passenger L3 start byte. Relevant only for tunnel start packets \n"));
            headerPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
            break;
    }

    if(headerPtr == NULL)
    {
        /* bug in simulation : need to understand why happened */
        skernelFatalError("copyPacketHeaderToMemory : NULL header pointer \n");
        return;
    }

    address = devObjPtr->PHA_FW_support.addr_PKT_REGs_lo;
    /* reset the place for expansion */
    for(ii = 0 ; ii < (expansionSize/4) ; ii++ , address+=4)
    {
        PPA_FW_SP_WRITE(address, 0);
    }

    iiMax = descrPtr->egressByteCount - (headerPtr - devObjPtr->egressBuffer);
    if(iiMax > (SIP6_PACKET_HEADER_MAX_COPY_SIZE - expansionSize))
    {
        iiMax = (SIP6_PACKET_HEADER_MAX_COPY_SIZE - expansionSize);
    }

    /* here address is : (FALCON_PKT_REGs_lo + expensionSize) */
    for(ii = 0 ; ii < iiMax ; ii++ , address++)
    {
        PPA_FW_SP_BYTE_WRITE(address, headerPtr[ii]);
    }

    /* continue to reset the rest of the bytes (just in case the thread try to copy from there) */
    for(/* ii continue */ ; ii < (SIP6_PACKET_HEADER_MAX_COPY_SIZE - expansionSize) ; ii++ , address++)
    {
        PPA_FW_SP_BYTE_WRITE(address,0);
    }

    /* save the pointer so we can restore changes into the egress packet */
    descrPtr->pha.pha_startHeaderPtr = headerPtr;
    descrPtr->pha.pha_numBytesFromStartHeader = iiMax;

}


/**
* @internal ppnProcessing function
* @endinternal
*
* @brief   let the PPN firmware to process the info
*/
void ppnProcessing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32       egressPort
)
{
    GT_U32  threadType;
    PHA_THREAD_INFO *phaThreadNames;

    threadType = descrPtr->pha.pha_threadType;
    phaThreadNames = devObjPtr->PHA_FW_support.phaThreadsInfoPtr;

   /* run the thread to process the data , and then we can check egress packet */
    __LOG_NO_LOCATION_META_DATA(("Firmware Start : %s (for egress port[%d])\n",
        phaThreadNames[threadType].threadDescription,
        egressPort));

    phaThreadNames[threadType].threadType();

    __LOG_NO_LOCATION_META_DATA(("Firmware Ended : %s (for egress port[%d])\n",
        phaThreadNames[threadType].threadDescription,
        egressPort));
}


/**
* @internal takeEgressPacketHeaderFromMemory function
* @endinternal
*
* @brief   get up to PACKET_HEADER_MAX_COPY_SIZE bytes of egress packet
*   from 'header memory'.
*/
void takeEgressPacketHeaderFromMemory
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STCT_PTR  descrPtr
)
{
    GT_U32  ii,iiMax;
    GT_U32  numBytesTillStartHeader;
    GT_U32  numBytesFromStartHeader;
    GT_U32  numBytesTillEndOfPacket;
    GT_U32  address;
    GT_U8   *headerPtr;
    GT_U8   *restOfPacketPtr;
    GT_U32  expansionSize = 32 + 16 * descrPtr->pha.pha_HeaderWindowSize;
    GT_U32  offset;

    headerPtr = descrPtr->pha.pha_startHeaderPtr;
    numBytesFromStartHeader = descrPtr->pha.pha_numBytesFromStartHeader;

    numBytesTillStartHeader = (headerPtr - devObjPtr->egressBuffer);

    restOfPacketPtr = descrPtr->pha.pha_startHeaderPtr + numBytesFromStartHeader;

    /* number of bytes from the end of content in the PPN till the end of the packet */
    numBytesTillEndOfPacket =  descrPtr->egressByteCount -
        (restOfPacketPtr - devObjPtr->egressBuffer);

    offset = numBytesFromStartHeader +
            (descrPtr->pha.pha_startHeaderPtr - devObjPtr->egressBuffer);

    address = devObjPtr->PHA_FW_support.addr_PKT_REGs_lo;

    if(descrPtr->pha.pha_fw_bc_modification < 0)
    {
        GT_U32  reducedNum = 0 - descrPtr->pha.pha_fw_bc_modification;

        /* means negative value --> so number of bytes 'reduced' */
        address += expansionSize + reducedNum;

        iiMax = numBytesFromStartHeader - reducedNum;

        /* we need to compress devObjPtr->egressBuffer for the removed bytes */
        for(ii = 0 ; ii < numBytesTillEndOfPacket ; ii++)
        {
            devObjPtr->egressBuffer[offset + ii - reducedNum] =
            devObjPtr->egressBuffer[offset + ii];
        }

        descrPtr->egressByteCount -= reducedNum;
    }
    else
    {
        GT_U32  enlargedNum = descrPtr->pha.pha_fw_bc_modification;

        if(enlargedNum > expansionSize)
        {
            skernelFatalError("takeEgressPacketHeaderFromMemory : can not move start of packet beyond [%d] bytes , but got [%d] \n",
                expansionSize , expansionSize);
        }

        /* means positive value --> so number of bytes 'enlarged' */
        address += expansionSize  - enlargedNum;
        iiMax = numBytesFromStartHeader + enlargedNum;

        /* we need to make room in devObjPtr->egressBuffer for those bytes */
        for(ii = numBytesTillEndOfPacket ; ii ; ii--)
        {
            devObjPtr->egressBuffer[offset + ii-1 + enlargedNum] =
            devObjPtr->egressBuffer[offset + ii-1];
        }

        descrPtr->egressByteCount += enlargedNum;
    }

    offset = numBytesTillStartHeader;
    for(ii = 0 ; ii < iiMax ; ii++ , address++)
    {
        devObjPtr->egressBuffer[ii + offset] = PPA_FW_SP_BYTE_READ(address);
    }

}


/**
* @internal copySingletonMemoryToSimulationPpnMemory function
* @endinternal
*
* @brief   copy from this singleton memory manager , the memories of : descriptor,config,header
*         to the memory of the PPN in the proper device in the proper memory space
*         this to allow the CPU to be able to look at the processing that was done.
*/
void copySingletonMemoryToSimulationPpnMemory
(
    IN SKERNEL_DEVICE_OBJECT_T_PTR devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STCT_PTR  descrPtr
)
{
    GT_U32  srcAddress,trgAddress,memNumWords,value,ii;
    GT_U32  simulationRelativeAddress;

    simulationRelativeAddress =
        UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA);

    /********copy config***************/
    srcAddress = devObjPtr->PHA_FW_support.addr_CFG_REGs_lo;
    trgAddress = srcAddress + simulationRelativeAddress;
    memNumWords = SIP6_CONFIG_NUM_WORDS;

    for(ii = 0 ; ii < memNumWords; ii++ , srcAddress+=4)
    {
        value = PPA_FW_SP_READ(srcAddress);
        smemRegSet(devObjPtr, trgAddress , value);
    }
    /********copy descriptor***************/
    srcAddress = devObjPtr->PHA_FW_support.addr_DESC_REGs_lo;
    trgAddress = srcAddress + simulationRelativeAddress;
    memNumWords = SIP6_DESC_NUM_WORDS;

    for(ii = 0 ; ii < memNumWords; ii++ , srcAddress+=4)
    {
        value = PPA_FW_SP_READ(srcAddress);
        smemRegSet(devObjPtr, trgAddress , value);
    }
    /********copy packet header***************/
    srcAddress = devObjPtr->PHA_FW_support.addr_PKT_REGs_lo;
    trgAddress = srcAddress + simulationRelativeAddress;
    memNumWords = SIP6_PACKET_HEADER_MAX_COPY_SIZE / 4;

    for(ii = 0 ; ii < memNumWords; ii++ , srcAddress+=4)
    {
        value = PPA_FW_SP_READ(srcAddress);
        smemRegSet(devObjPtr, trgAddress , value);
    }
    /***********************/
}


/**
* @internal actual_firmware_processing function
* @endinternal
*
* @brief    actual 'firmware' processing.  (according to descrPtr->pha.pha_threadType)
*/
extern void actual_firmware_processing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32       egressPort
)
{
    DECLARE_FUNC_NAME(falcon_actual_firmware_processing);

    /************************************************/
    /* the code is written as 'single instance' !!! */
    /* so we must lock it from other simulation tasks*/
    /************************************************/
    SCIB_SEM_TAKE;

    __LOG(("The PPN started processing (for egress port[%d]) \n",
        egressPort));

    current_fw_devObjPtr = devObjPtr;
    current_fw_ThreadId  = descrPtr->pha.pha_threadType;

    /********* PRE-processing *************/
    /* 1. convert simulation 'descrPtr' to 'PHA descriptor' */
    /* 2. put the 'PHA descriptor' into the PPN 'desc memory' */
    devObjPtr->PHA_FW_support.convertSimulationDescToPHaDesc(devObjPtr,descrPtr,egressPort);
    /* 3. put the 'PHA config'     into the PPN 'config memory' */
    copyPHaConfigToMemory(devObjPtr,descrPtr);
    /* 4. put first 128 bytes of ingress packet into 'header memory' (clear to ZERO first 20 bytes) */
    copyPacketHeaderToMemory(devObjPtr,descrPtr);

    /********* The processing *************/
    /* 5. let the PPN firmware to process the info */
    ppnProcessing(devObjPtr,descrPtr,egressPort);

    /********* POST-processing *************/
    /* 6. convert the 'PHA descriptor' (from memory) back to fields in simulation 'descrPtr' */
    devObjPtr->PHA_FW_support.convertPHaDescToSimulationDesc(devObjPtr,descrPtr);
    /* 7. put the packet in the device egress buffer */
    takeEgressPacketHeaderFromMemory(devObjPtr,descrPtr);

    /* 8. copy from this singleton memory manager , the memories of : descriptor,config,header
       to the memory of the PPN in the proper device in the proper memory space

       this to allow the CPU to be able to look at the processing that was done.
    */
    copySingletonMemoryToSimulationPpnMemory(devObjPtr,descrPtr);

    __LOG(("The PPN ended processing (for egress port[%d]) \n",
        egressPort));

    SCIB_SEM_SIGNAL;

}


/**
* @internal pha_fw_activeMem_write_toPpnMem function
* @endinternal
*
* @brief   active memory to reflect the write to PPN , to the memory of PPN 0 in the device
*/
uint32_t pha_fw_activeMem_write_toPpnMem(uint32_t addr, uint32_t *memPtr , uint32_t writeValue)
{
    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;
    GT_U32  phaBaseAddr = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA); /* Cider of the PHA unit */
    GT_U32  addressInPHA_ppn = 0x00000000;/*Cider offset PPN 0 in PPG 0 */

    /* write the address in the PPN 0 of the current device */
    smemRegSet(devObjPtr,
        phaBaseAddr + addressInPHA_ppn + addr,
        writeValue);

    return 0;/* dummy --> return value is only for 'read' active memories */
}


/**
* @internal fw_activeMem_read_DRAM_BASE_ADDR function
* @endinternal
*
* @brief   active memory to reflect the read from data shared memory
*/
uint32_t fw_activeMem_read_DRAM_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue/*relevant for write only*/)
{
    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;
    GT_U32  phaBaseAddr = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA); /* Cider of the PHA unit */
    GT_U32  addressInPHA_DRAM_INIT_DONE_ADDR = 0x007C0000;/*Cider offset for the shared DRAM in the PHA unit */
    GT_U32  relativeAddrInDram = addr - DRAM_BASE_ADDR;
    GT_U32  regValue;

    /* get the address from the DRAM of the current device */
    smemRegGet(devObjPtr,
        phaBaseAddr + addressInPHA_DRAM_INIT_DONE_ADDR + relativeAddrInDram,
        &regValue);

    return regValue;
}

/**
* @internal fw_activeMem_write_DRAM_BASE_ADDR function
* @endinternal
*
* @brief   active memory to reflect the write to data shared memory
*/
uint32_t fw_activeMem_write_DRAM_BASE_ADDR(uint32_t addr, uint32_t *memPtr , uint32_t writeValue)
{
    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;
    GT_U32  phaBaseAddr = UNIT_BASE_ADDR_GET_MAC(devObjPtr,UNIT_PHA); /* Cider of the PHA unit */
    GT_U32  addressInPHA_DRAM_INIT_DONE_ADDR = 0x007C0000;/*Cider offset for the shared DRAM in the PHA unit */
    GT_U32  relativeAddrInDram = addr - DRAM_BASE_ADDR;

    /* write the address in the DRAM of the current device */
    smemRegSet(devObjPtr,
        phaBaseAddr + addressInPHA_DRAM_INIT_DONE_ADDR + relativeAddrInDram,
        writeValue);

    return 0;/* dummy --> return value is only for 'read' active memories */
}

/**
* @internal pha_fw_activeMem_read_TOD function
* @endinternal
*
* @brief   active memory to reflect the read from TOD interface
*/
uint32_t pha_fw_activeMem_read_TOD(uint32_t addr, uint32_t *memPtr , uint32_t writeValue/*relevant for write only*/)
{
    DECLARE_FUNC_NAME(pha_fw_activeMem_read_TOD);

    SKERNEL_DEVICE_OBJECT *devObjPtr = current_fw_devObjPtr;
    GT_U32  taiGroup = 0;
    GT_U32  taiId =  (addr >= PPN_IO_REG_TOD_1_WORD_0_ADDR) ? 1 : 0;
    GT_U32  wordIndex = ((addr - PPN_IO_REG_TOD_0_WORD_0_ADDR) & 0xf) / 4;
    SNET_TOD_TIMER_STC  tod;
    GT_U32  regValue;

    if(pha_fw_activeMem_read_TOD_always_ZERO)
    {
        return 0;
    }

    /* read the current TOD  */
    /* NOTE: those register ARE NOT implemented as 'atomic read' !!! (like in HW) */
    /* the FW need to be aware to it !!! */
    snetLion3PtpTodGetTimeCounter(devObjPtr,taiGroup,taiId,&tod);

    switch(wordIndex)
    {
        default:
        case 0: /* TOD %t nanoseconds */
            regValue = tod.nanoSecondTimer;
            __LOG(("read TOD [%d] nanoseconds value [0x%x] \n",taiId,tod.nanoSecondTimer));
            break;
        case 1:/* TOD %tt seconds field bits [31:0] */
            regValue = tod.secondTimer.l[0];
            __LOG(("read TOD [%d]seconds field bits [31:0] value [0x%x] \n",taiId,tod.secondTimer.l[0]));
            break;
        case 2:/* bits 16:31 - TOD %ttt seconds field bits [47:32]. Big Endian field placement. */
            regValue = tod.secondTimer.l[1] << 16;
            __LOG(("read TOD [%d]seconds field bits [47:32] value [0x%x] \n",taiId,tod.secondTimer.l[1]));
            break;
    }

    return regValue;
}


/**
* @internal testEndianCasting function
* @endinternal
*
* @brief   test endian format is as expected
*/
void testEndianCasting(IN SKERNEL_DEVICE_OBJECT * devObjPtr)
{
    struct IPv4_Header*        ipv4_hdr_ptr;
    GT_U32      IPv4_Header_words[5] = {0};
    GT_U32      ii,address;


    IPv4_Header_words[0] = 0x45881234;
    IPv4_Header_words[1] = 0x98765432;
    IPv4_Header_words[2] = 0xabcd5678;
    IPv4_Header_words[3] = 0x01020304;
    IPv4_Header_words[4] = 0x05060708;

    address = devObjPtr->PHA_FW_support.addr_PKT_REGs_lo;
    /* reset the place for expansion */
    for(ii = 0 ; ii < 5 ; ii++ , address+=4)
    {
        PPA_FW_SP_WRITE(address, IPv4_Header_words[ii]);
    }

    ipv4_hdr_ptr = FALCON_MEM_CAST(devObjPtr->PHA_FW_support.addr_PKT_REGs_lo,IPv4_Header);
    if(ipv4_hdr_ptr->version != 0x4)
    {
        skernelFatalError("ERROR casting is wrong version must be 0x4 but got [%d]!!! \n",
            ipv4_hdr_ptr->version);
    }
}


/**
* @internal updateNeededRegisters function
* @endinternal
*
* @brief   read and write back values to allow it to got to the device memory
*          note:same address as in Hawk device therefore we can use FALCON_FW_VERSION_ADDR define
*/
void updateNeededRegisters(void)
{
    PPA_FW_SP_WRITE(FALCON_FW_VERSION_ADDR  ,PPA_FW_SP_READ(FALCON_FW_VERSION_ADDR  ));
    PPA_FW_SP_WRITE(FALCON_FW_VERSION_ADDR+4,PPA_FW_SP_READ(FALCON_FW_VERSION_ADDR+4));
}


/**
* @internal GetUseCaseThreadId function
* @endinternal
*
* @brief   get the threadType of the 'emulated' firmware according to instruction_pointer
*         NOTE: 'GetUseCaseThreadType' is the function name in VERIFIER code as 'GetUseCaseThreadId'
*/
static GT_U32 GetUseCaseThreadType
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 instruction_pointer ,
    OUT FIRMWARE_THREAD_FUNC *firmwareFunctionPtr
)
{
    GT_U32  ii;
    GT_U32  threadType = 0xFF;
    PHA_THREAD_INFO *instructionPtrToThreadsName;

    instructionPtrToThreadsName = devObjPtr->PHA_FW_support.phaThreadsInfoPtr;
    *firmwareFunctionPtr = NULL;

    /* according to instruction_pointer look for the threadId */
    for(ii = 0 ;instructionPtrToThreadsName[ii].instruction_pointer !=  SMAIN_NOT_VALID_CNS;ii++)
    {
        if(instructionPtrToThreadsName[ii].instruction_pointer == instruction_pointer)
        {
            /* update fw thread ID */
            threadType = instructionPtrToThreadsName[ii].threadId;

            /* let the simulation environment make all the needed preparations for the firmware */
            *firmwareFunctionPtr = actual_firmware_processing;
            break;
        }
    }

    return threadType;
}


/**
* @internal snetPhaUnit_firmware function
* @endinternal
*
* @brief   PHA unit - emulate the firmware (fw) behavior for several cases
*/
static void snetPhaUnit_firmware
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32       egressPort
)
{
    GT_U32  threadType;
    FIRMWARE_THREAD_FUNC firmwareFunction;
    GT_U32 phaInstructionPointer = descrPtr->pha.pha_instructionPointer;

    phaInstructionPointer += SIP6_FW_INSTRUCTION_DOMAIN_ADDR_CNS;

    threadType = GetUseCaseThreadType(devObjPtr,phaInstructionPointer,&firmwareFunction);

    if(firmwareFunction)
    {
        /* Update fw thread ID */
        descrPtr->pha.pha_threadType = threadType;
        /* Activate the proper emulated firmware function */
        firmwareFunction(devObjPtr,descrPtr,egressPort);
    }
    else
    {
        skernelFatalError("PHA fw failure: The thread's PC address [0x%x] got no FW to handle it \n", phaInstructionPointer);
    }
}


/**
* @internal snetFalconPhaCalculateTargetPort function
* @endinternal
*
* @brief   Falcon : Get the target port .
*          For Egress mirrored packets, the original trg port is used for PHA target table lookup
*          For ingress mirror and non mirror packets, no change in egress port
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*                                       indication that packet was send to the DMA of the port
* COMMENTS :
*/
GT_U32 snetFalconPhaCalculateTargetPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconPhaCalculateTargetPort);

    GT_BOOL sameDevMirror = descrPtr->srcTrgDev == descrPtr->ownDev ;


    if (((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && (descrPtr->rxSniff == 0) && sameDevMirror)||
        ((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && ( descrPtr->srcTrg == 1) && sameDevMirror))))
    {
        /* use the egressPort that the 'orig descriptor' used */
        __LOG(("The PHA use configurations of the 'orig egress physical port' [%d] and not analyzer/cpu egress port[%d] \n",
            descrPtr->egressTrgPort,
            egressPort))

        return descrPtr->egressTrgPort;
    }
    else
    {
        /* use the egress port */
        return egressPort;
    }
}
/**
* @internal snetFalconPhaProcess function
* @endinternal
*
* @brief   Falcon : do PHA processing .
*         in the Control pipe : QAG --> HA --> EPCL --> PRE-Q --> EOAM --> EPLR --> ERMRK --> PHA --> EREP --> DP(RxDma).
*         in the Data Path  : --> RxDma --> TXQ-PDX --> (egress tile) TX-SDQ --> (TXFIFO,TXDMA) --> MAC.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*                                       indication that packet was send to the DMA of the port
* COMMENTS :
*/
void snetFalconPhaProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconPhaProcess);

    GT_U32  regAddr , *memPtr;
    GT_BIT  disable_ppa_clock,reserved_bit_8;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PHA_E);

    /*******************************************/
    /* check if the PPA clock is enabled       */
    /*******************************************/
    regAddr = SMEM_SIP6_PHA_CTRL_REG(devObjPtr);
    smemRegFldGet(devObjPtr ,regAddr ,2 ,1 ,&disable_ppa_clock);
    if(disable_ppa_clock)
    {
        __LOG(("NOTE: the PHA is (globally) disabled !!! (PPA clock disabled) \n"));

        return;
    }

    /*******************************************/
    /* check if the packet needs FW processing */
    /*******************************************/
    if(descrPtr->pha.pha_threadId == 0)
    {
        __LOG(("The PHA unit not triggered for the this packet (pha_threadId == 0) \n"));
        /* PHA CNC Trigger */
        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            snetCht3CncCount(devObjPtr, descrPtr, SNET_SIP6_10_CNC_CLIENT_PHA_E, egressPort);
        }
        return;
    }

    __LOG_PARAM(descrPtr->pha.pha_threadId);

    /* access the HA table */
    regAddr = SMEM_SIP6_PHA_PPA_THREADS_CONF2_MEM(devObjPtr,descrPtr->pha.pha_threadId);
    memPtr  = smemMemGet(devObjPtr, regAddr);

    descrPtr->pha.pha_template[0] = memPtr[0];
    descrPtr->pha.pha_template[1] = memPtr[1];
    descrPtr->pha.pha_template[2] = memPtr[2];
    descrPtr->pha.pha_template[3] = memPtr[3];

    __LOG_PARAM(descrPtr->pha.pha_template[0]);
    __LOG_PARAM(descrPtr->pha.pha_template[1]);
    __LOG_PARAM(descrPtr->pha.pha_template[2]);
    __LOG_PARAM(descrPtr->pha.pha_template[3]);

    descrPtr->pha.pha_instructionPointer = snetFieldValueGet(memPtr,128,16);
    descrPtr->pha.pha_SkipCounter        = snetFieldValueGet(memPtr,144, 5);

    __LOG_PARAM(descrPtr->pha.pha_instructionPointer);
    __LOG_PARAM(descrPtr->pha.pha_SkipCounter);

    regAddr = SMEM_SIP6_PHA_PPA_THREADS_CONF1_MEM(devObjPtr,descrPtr->pha.pha_threadId);
    memPtr  = smemMemGet(devObjPtr, regAddr);

    descrPtr->pha.pha_HeaderWindowSize  = snetFieldValueGet(memPtr,21,2);
    descrPtr->pha.pha_HeadeWindowAnchor = snetFieldValueGet(memPtr,19,2);
    descrPtr->pha.pha_StallDropCode     = snetFieldValueGet(memPtr,11,8);
    descrPtr->pha.pha_PPABusyStallMode  = snetFieldValueGet(memPtr, 9,2);
    /* bit 8 reserved (expected to be 0) */
    reserved_bit_8 = snetFieldValueGet(memPtr, 8,1);
    descrPtr->pha.pha_StatisticalProcessingFactor = snetFieldValueGet(memPtr,0,8);

    __LOG_PARAM(descrPtr->pha.pha_HeaderWindowSize );
    __LOG_PARAM(descrPtr->pha.pha_HeadeWindowAnchor);
    __LOG_PARAM(descrPtr->pha.pha_StallDropCode    );
    __LOG_PARAM(descrPtr->pha.pha_PPABusyStallMode );
    __LOG_PARAM(descrPtr->pha.pha_StatisticalProcessingFactor);

    if(reserved_bit_8)
    {
        __LOG(("Warning : bit 8 in PHA - PPA Threads Conf1 expected to be ZERO (potential configuration ERROR) \n"));
    }

    __LOG(("Access the target physical port data for egress port [%d]\n",
        egressPort));
    /* access the target port data */
    regAddr = SMEM_SIP6_PHA_TARGET_PORT_DATA_MEM(devObjPtr,egressPort);
    memPtr  = smemMemGet(devObjPtr, regAddr);

    descrPtr->pha.pha_trgPhyPortData[0] = memPtr[0];
    __LOG_PARAM(descrPtr->pha.pha_trgPhyPortData[0]);


    /* access the source port data */
    __LOG(("Access the source physical port data for source port [%d]\n",
        descrPtr->localDevSrcPort));
    regAddr = SMEM_SIP6_PHA_SRC_PORT_DATA_MEM(devObjPtr,descrPtr->localDevSrcPort);
    memPtr  = smemMemGet(devObjPtr, regAddr);

    descrPtr->pha.pha_srcPhyPortData[0] = memPtr[0];
    __LOG_PARAM(descrPtr->pha.pha_srcPhyPortData[0]);

    /* trigger the emulated firmware operation */
    SIM_LOG_PACKET_DESCR_SAVE
    snetPhaUnit_firmware(devObjPtr,descrPtr,egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("PPN Unit (firmware)");

    /* PHA CNC Trigger */
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        snetCht3CncCount(devObjPtr, descrPtr, SNET_SIP6_10_CNC_CLIENT_PHA_E, egressPort);
    }

    return;
}

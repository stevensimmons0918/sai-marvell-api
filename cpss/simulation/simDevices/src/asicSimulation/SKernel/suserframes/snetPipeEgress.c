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
* @file snetPipeEgress.c
*
* @brief This is the implementation for the Egress processing of 'PIPE' in SKernel.
*
* @version   1
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetPipe.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/cheetahCommon/sregPipe.h>


extern SKERNEL_FRAME_PIPE_DESCR_STC * snetPipeDuplicateDescr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
);


/*PHA Use-Cases Thread ID enumeration
NOTE: values taken from VERIFIER code
*/
enum {
    PHA_THREAD_DO_NOTHING               = enum_THR0_DoNothing,
    /*start ETAG */
    PHA_THREAD_ETAG_E2U                 = enum_THR1_E2U,
    PHA_THREAD_ETAG_U2E                 = enum_THR2_U2E,
    PHA_THREAD_ETAG_U2C                 = enum_THR3_U2C,
    /*end ETAG */

    /* start DSA (non-EDSA)*/
    PHA_THREAD_DSA_ET2U                 = enum_THR4_ET2U ,
    PHA_THREAD_DSA_EU2U                 = enum_THR5_EU2U ,
    PHA_THREAD_DSA_U2E                  = enum_THR6_U2E  ,
    PHA_THREAD_DSA_MRR2E                = enum_THR7_Mrr2E,
    /* end DSA (non-EDSA)*/

    /* start EDSA */
    PHA_THREAD_EDSA_EV2U                = enum_THR8_E_V2U,
    PHA_THREAD_EDSA_E2U                 = enum_THR9_E2U  ,
    PHA_THREAD_EDSA_CV2U                = enum_THR10_C_V2U ,
    PHA_THREAD_EDSA_C2U                 = enum_THR11_C2U   ,
    PHA_THREAD_EDSA_UUC2C               = enum_THR12_U_UC2C,
    PHA_THREAD_EDSA_UMC2C               = enum_THR13_U_MC2C,
    PHA_THREAD_EDSA_UMR2C               = enum_THR14_U_MR2C,
    /* end EDSA */

    /*others*/
    PHA_THREAD_QCN                      = enum_THR15_QCN,
    PHA_THREAD_U2E                      = enum_THR16_U2E,

    /* IPL threads */
    PHA_THREAD_U2IPL                    = enum_THR17_U2IPL,
    PHA_THREAD_IPL2IPL                  = enum_THR18_IPL2IPL,

    /* Extended Port to Upstream Port for Untagged VLAN Packets */
    PHA_THREAD_E2U_Untagged             = enum_THR19_E2U_Untagged,
    /* Upstream to extended Multicast threads */
    PHA_THREAD_U2E_M4                   = enum_THR20_U2E_M4,
    PHA_THREAD_U2E_M8                   = enum_THR21_U2E_M8,
    /* Discard packet thread */
    PHA_THREAD_Discard                  = enum_THR22_Discard,

    /* EVB threads */
    PHA_THREAD_EVB_E2U                  = enum_THR23_EVB_E2U,
    PHA_THREAD_EVB_U2E                  = enum_THR24_EVB_U2E,
    PHA_THREAD_EVB_QCN                  = enum_THR25_EVB_QCN,

    /* PRE DA threads */
    PHA_THREAD_PRE_DA_U2E               = enum_THR26_PRE_DA_U2E,
    PHA_THREAD_PRE_DA_E2U               = enum_THR27_PRE_DA_E2U,

    PHA_THREAD_VARIABLE_CYCLES_LENGTH_WITH_ACCL_CMD                      = enum_THR45_VariableCyclesLengthWithAcclCmd,
    PHA_THREAD_REMOVE_ADD_BYTES         = enum_THR46_RemoveAddBytes,
    PHA_THREAD_ADD_20B                  = enum_THR47_Add20Bytes,
    PHA_THREAD_REMOVE_20B               = enum_THR48_Remove20Bytes,
    PHA_THREAD_VARIABLE_CYCLE_LENGTH    = enum_THR49_VariableCyclesLength
};

#define PLACE_HOLDER_CNS (SMAIN_NOT_VALID_CNS - 1)
static struct {
    IN GT_U32 instruction_pointer;
    OUT GT_U32  threadId;
} instruction_pointer_to_threadId[] = {
        /*instruction_pointer*/                                     /*threadId*/
    {1          ,  PHA_THREAD_DO_NOTHING           },
    {1          ,  PHA_THREAD_ETAG_E2U             },
    {1          ,  PHA_THREAD_ETAG_U2E             },
    {1          ,  PHA_THREAD_ETAG_U2C             },
    {1          ,  PHA_THREAD_DSA_ET2U             },
    {1          ,  PHA_THREAD_DSA_EU2U             },
    {1          ,  PHA_THREAD_DSA_U2E              },
    {1          ,  PHA_THREAD_DSA_MRR2E            },
    {1          ,  PHA_THREAD_EDSA_EV2U            },
    {1          ,  PHA_THREAD_EDSA_E2U             },
    {1          ,  PHA_THREAD_EDSA_CV2U            },
    {1          ,  PHA_THREAD_EDSA_C2U             },
    {1          ,  PHA_THREAD_EDSA_UUC2C           },
    {1          ,  PHA_THREAD_EDSA_UMC2C           },
    {1          ,  PHA_THREAD_EDSA_UMR2C           },
    {1          ,  PHA_THREAD_QCN                  },
    {1          ,  PHA_THREAD_U2E                  },
    {1          ,  PHA_THREAD_U2IPL                },
    {1          ,  PHA_THREAD_IPL2IPL              },
    {1          ,  PHA_THREAD_VARIABLE_CYCLES_LENGTH_WITH_ACCL_CMD},
    {1          ,  PHA_THREAD_REMOVE_ADD_BYTES     },
    {1          ,  PHA_THREAD_ADD_20B              },
    {1          ,  PHA_THREAD_REMOVE_20B           },
    {1          ,  PHA_THREAD_VARIABLE_CYCLE_LENGTH},
    {1          ,  PHA_THREAD_E2U_Untagged         },
    {1          ,  PHA_THREAD_U2E_M4               },
    {1          ,  PHA_THREAD_U2E_M8               },
    {1          ,  PHA_THREAD_Discard              },
    {1          ,  PHA_THREAD_EVB_E2U              },
    {1          ,  PHA_THREAD_EVB_U2E              },
    {1          ,  PHA_THREAD_EVB_QCN              },
    {1          ,  PHA_THREAD_PRE_DA_U2E           },
    {1          ,  PHA_THREAD_PRE_DA_E2U           },
    /* place holders to set other values */
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    {PLACE_HOLDER_CNS , PHA_THREAD_DO_NOTHING},
    /* must be last */
    {SMAIN_NOT_VALID_CNS , PHA_THREAD_DO_NOTHING},
};

/* called from the CPSS to set the 'instruction_pointer' for the 'threadId' */
GT_STATUS simulationPipeFirmwareThreadIdToInstructionPointerSet
(
    IN GT_U32  threadId,/*0..31*/
    IN GT_U32 instruction_pointer
)
{
    GT_U32 ii;

    /* according to instruction_pointer look for the threadId */
    for(ii = 0 ;instruction_pointer_to_threadId[ii].instruction_pointer !=  SMAIN_NOT_VALID_CNS;ii++)
    {
        if(instruction_pointer_to_threadId[ii].threadId == threadId)
        {
            instruction_pointer_to_threadId[ii].instruction_pointer = instruction_pointer;
            return GT_OK;
        }
    }

    return GT_NOT_FOUND;
}

/*******************************************************************************
*   FIRMWARE_THREAD_FUNC
*
* DESCRIPTION:
*       prototype for emulated 'firmware' function
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       pipe_descrPtr - descriptor for the packet
*   OUT:
*
*
*
*******************************************************************************/
typedef void (*FIRMWARE_THREAD_FUNC)
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
);

/* build packet ready for egress the port */
/**
* @internal haTagsManipulations function
* @endinternal
*
* @brief   add/remove tag bytes to/from the egress buffer.
*/
static void haTagsManipulations
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN GT_U32    startByte,      /* when startByte == 0 and numberOfBytes == 0 --> copy full ingress packet for egress */
    IN GT_U32    numberOfBytes,
    IN GT_U8     *tagToAddPtr  /* when NULL --> remove bytes
                                  when ! NULL --> add bytes  */
)
{
    GT_U32  numberOfBytesToMove;

    if(startByte == 0 && numberOfBytes == 0)
    {
        /* copy full packet from ingress */
        memcpy(devObjPtr->egressBuffer,
               pipe_descrPtr->cheetah_descrPtr->startFramePtr,
               pipe_descrPtr->cheetah_descrPtr->byteCount);

        pipe_descrPtr->cheetah_descrPtr->egressByteCount =
            pipe_descrPtr->cheetah_descrPtr->byteCount;
        return;
    }

    numberOfBytesToMove = pipe_descrPtr->cheetah_descrPtr->egressByteCount - startByte;

    if(tagToAddPtr)/* we add tag */
    {
        /* move the rest of the packet */
        memmove(&devObjPtr->egressBuffer[startByte],
                &devObjPtr->egressBuffer[startByte+numberOfBytes],
                numberOfBytesToMove);

        memcpy(&devObjPtr->egressBuffer[startByte],tagToAddPtr,numberOfBytes);

        pipe_descrPtr->cheetah_descrPtr->egressByteCount += numberOfBytes;
    }
    else
    {
        /* move the rest of the packet */
        memmove(&devObjPtr->egressBuffer[startByte+numberOfBytes],
                &devObjPtr->egressBuffer[startByte],
                numberOfBytesToMove);

        pipe_descrPtr->cheetah_descrPtr->egressByteCount -= numberOfBytes;
    }
}

/**
* @internal pipe_actual_firmware_processing function
* @endinternal
*
* @brief   actual 'firmware' processing.
*/
extern void pipe_actual_firmware_processing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
);


/* ALL addresses of instruction pointers need to be with prefix 0x00400000 */
/* the PHA table need to hold only lower 16 bits (the prefix is added internally by the HW) */
#define FW_INSTRUCTION_DOMAIN_ADDR_CNS      0x00400000

/**
* @internal GetUseCaseThreadId function
* @endinternal
*
* @brief   get the threadId of the 'emulated' firmware according to instruction_pointer
*         NOTE: 'GetUseCaseThreadId' is the function name also in VERIFIER code.
*/
static GT_U32 GetUseCaseThreadId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 instruction_pointer ,
    OUT FIRMWARE_THREAD_FUNC *firmwareFunctionPtr
)
{
    GT_U32  ii;
    GT_U32  threadId = PHA_THREAD_DO_NOTHING;

    /* according to instruction_pointer look for the threadId */
    for(ii = 0 ;instruction_pointer_to_threadId[ii].instruction_pointer !=  SMAIN_NOT_VALID_CNS;ii++)
    {
        if(instruction_pointer_to_threadId[ii].instruction_pointer == instruction_pointer)
        {
            threadId = instruction_pointer_to_threadId[ii].threadId;
            break;
        }
    }
    /* let the simulation environment make all the needed preparations for the firmware */
    *firmwareFunctionPtr = pipe_actual_firmware_processing;

    return threadId;
}


/**
* @internal snetPipeEgressHaUnit_firmware function
* @endinternal
*
* @brief   HA unit - emulate the firmware (fw) behavior for several cases
*/
static void snetPipeEgressHaUnit_firmware
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    GT_U32  emulated_threadId;
    FIRMWARE_THREAD_FUNC firmwareFunction;
    GT_U32 pipe_haInstructionPointer = pipe_descrPtr->pipe_haInstructionPointer;

    pipe_haInstructionPointer += FW_INSTRUCTION_DOMAIN_ADDR_CNS;

    emulated_threadId = GetUseCaseThreadId(devObjPtr,pipe_haInstructionPointer,&firmwareFunction);

    if(firmwareFunction)
    {
        pipe_descrPtr->pipe_emulated_threadId = emulated_threadId;
        /* activate the proper emulated firmware function */
        firmwareFunction(devObjPtr,pipe_descrPtr);
    }

}

/**
* @internal pipePpaEgressCncCounting function
* @endinternal
*
* @brief   Do PPA egress CNC counting
*/
static GT_VOID pipePpaEgressCncCounting
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeEgressCncCounting);
    GT_U32  cncIndexValue = pipe_descrPtr->pipe_egressCncIndex;
    /*if the dest address is legal, send to cnc*/
    if (pipe_descrPtr->pipe_egressPacketCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("The CNC not applied due to 'HARD_DROP' by PPA unit \n"));
        return;
    }

    if(pipe_descrPtr->pipe_egressCncGenerate == 0)
    {
        __LOG(("The CNC not counting because trigger was not set (see <Generate_CNC_Default> or firmware set it to 0) \n"));
        return;
    }


    if(pipe_descrPtr->pipe_egressCncIndex == pipe_descrPtr->pipe_haTableIndex)
    {
        __LOG(("CNC - PPA client - {TrgPort, PktType} , index[%d] \n",
            cncIndexValue));
    }
    else
    {
        __LOG(("CNC - PPA client - as set by firmware , index[%d] \n",
            cncIndexValue));
    }

    snetCht3CncCount(devObjPtr,
        pipe_descrPtr->cheetah_descrPtr,
        SNET_CNC_PIPE_DEVICE_CLIENT_PPA_TRG_PORT_PKT_TYPE_E,
        cncIndexValue);

}


/**
* @internal snetPipeEgressHaUnit function
* @endinternal
*
* @brief   HA unit
*/
static GT_VOID snetPipeEgressHaUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetPipeEgressHaUnit);

    GT_U32  regAddress , *memPtr;
    GT_U32  pha_bypass,Generate_CNC_Default;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PHA_E);


    /* check if the PHA is not Bypassed */
    regAddress = SMEM_PIPE_PHA_CTRL_REG(devObjPtr);
    smemRegFldGet(devObjPtr ,regAddress ,1 ,1 ,&pha_bypass);
    if(pha_bypass)
    {
        __LOG(("WARNING: PHA is (globally) Bypassed !!! (no modifications on any packet !!!) \n"));

        __LOG(("Copy the ingress packet to the egress buffer , without any modifications \n"));
        /* Copy the ingress packet to the egress buffer , without any modifications */
        haTagsManipulations(devObjPtr,pipe_descrPtr,
            0,0,NULL);

        return;
    }

    /*The HA table access index = {Target_Port[4:0], Packet_Type[4:0]}.*/
    pipe_descrPtr->pipe_haTableIndex = (egressPort << 5) | pipe_descrPtr->pipe_PktTypeIdx;
    __LOG(("The HA table access index = [%d] from {Target_Port[4:0]=[%d], Packet_Type[4:0]=[%d]}.  \n",
        pipe_descrPtr->pipe_haTableIndex,
        egressPort,
        pipe_descrPtr->pipe_PktTypeIdx
        ));
    __LOG_PARAM(pipe_descrPtr->pipe_haTableIndex);

    /* CNC index and 'generate' can be overridden by firmware */
    regAddress = SMEM_PIPE_PHA_GENERAL_CONFIG_REG(devObjPtr);
    smemRegFldGet(devObjPtr ,regAddress ,0 ,1 ,&Generate_CNC_Default);
    __LOG_PARAM(Generate_CNC_Default);
    pipe_descrPtr->pipe_egressCncGenerate = Generate_CNC_Default;
    pipe_descrPtr->pipe_egressCncIndex = pipe_descrPtr->pipe_haTableIndex;

    /* access the HA table */
    regAddress = SMEM_PIPE_PHA_HA_TABLE_MEM(devObjPtr,pipe_descrPtr->pipe_haTableIndex);
    memPtr = smemMemGet(devObjPtr, regAddress);

    pipe_descrPtr->pipe_haTemplate[0] = memPtr[0];
    pipe_descrPtr->pipe_haTemplate[1] = memPtr[1];
    pipe_descrPtr->pipe_haTemplate[2] = memPtr[2];
    pipe_descrPtr->pipe_haTemplate[3] = memPtr[3];

    __LOG_PARAM(pipe_descrPtr->pipe_haTemplate[0]);
    __LOG_PARAM(pipe_descrPtr->pipe_haTemplate[1]);
    __LOG_PARAM(pipe_descrPtr->pipe_haTemplate[2]);
    __LOG_PARAM(pipe_descrPtr->pipe_haTemplate[3]);

    pipe_descrPtr->pipe_haInstructionPointer = snetFieldValueGet(memPtr,128,16);
    __LOG_PARAM(pipe_descrPtr->pipe_haInstructionPointer);

    __LOG(("Access the HA target port data for egress port [%d]\n",
        egressPort));
    /* access the target port data */
    regAddress = SMEM_PIPE_PHA_TARGET_PORT_DATA_MEM(devObjPtr,egressPort);
    memPtr = smemMemGet(devObjPtr, regAddress);

    pipe_descrPtr->pipe_haTrgPortData[0] = memPtr[0];
    pipe_descrPtr->pipe_haTrgPortData[1] = memPtr[1];

    __LOG_PARAM(pipe_descrPtr->pipe_haTrgPortData[0]);
    __LOG_PARAM(pipe_descrPtr->pipe_haTrgPortData[1]);

    /* access the source port data */
    __LOG(("Access the HA source port data for source port [%d]\n",
        pipe_descrPtr->pipe_SrcPort));
    regAddress = SMEM_PIPE_PHA_SRC_PORT_DATA_MEM(devObjPtr,pipe_descrPtr->pipe_SrcPort);
    memPtr = smemMemGet(devObjPtr, regAddress);

    pipe_descrPtr->pipe_haSrcPortData = memPtr[0];
    __LOG_PARAM(pipe_descrPtr->pipe_haSrcPortData);

    /* trigger the emulated firmware operation */
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    snetPipeEgressHaUnit_firmware(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("PPN Unit (firmware)");

    pipePpaEgressCncCounting(devObjPtr,pipe_descrPtr);

}

/**
* @internal snetPipeEgressTxqToPortTillTxFifo function
* @endinternal
*
* @brief   let the TXQ to handle the packet to the egress port
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] pipe_descrPtr            - frame descriptor
* @param[in] egressPort               - egress Port
*                                       final egress TxFifo port.
*                                       value SMAIN_NOT_VALID_CNS meaning that packet was dropped.
*                                       COMMENTS :
*/
static GT_U32 snetPipeEgressTxqToPortTillTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetPipeEgressTxqToPortTillTxFifo);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_BOOL queueEnabled;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);

    /* get the device object and the macPort for this egress physical port */
    if(GT_FALSE == snetChtTxMacPortGet_forPipeDevice(devObjPtr,
        cheetah_descrPtr,
        /*MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E,*/
        egressPort,GT_TRUE,
        &cheetah_descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr,
        &cheetah_descrPtr->egressPhysicalPortInfo.txDmaMacPort))
    {
        __LOG(("MAC port [%d] not exists \n",
            egressPort));
        /* MAC port not exists */
        return SMAIN_NOT_VALID_CNS;
    }


    if(cheetah_descrPtr->continueFromTxqDisabled)
    {
        __LOG(("continueFromTxqDisabled : TXQ counter sets already counted for this descriptor (before the 'enqueue') \n"));
    }
    else
    {
        /* TXQ counter sets counting on the egress device */
        snetChtTxQCounterSets_forPipeDevice(
            devObjPtr, cheetah_descrPtr, NULL/*destPorts*/,
            NULL, SKERNEL_EGR_PACKET_BRG_UCAST_E, NULL,
            TXQ_COUNTE_MODE_EGRESS_DEVICE_ONLY_E,
            egressPort);
    }


    /* 1. Filter disabled TC
     * 2. Enqueue frame(if need).
     */
    queueEnabled = txqToPortQueueDisableCheck_forPipeDevice(devObjPtr,cheetah_descrPtr,
        egressPort);

    if(queueEnabled == GT_FALSE)
    {
        __LOG(("NOTE: the queue of 'TXQ-PORT' [%d] is disabled ('physical port'[%d]) no more TXQ processing \n",
            cheetah_descrPtr->txqDestPorts[egressPort],
            egressPort));
        return (SMAIN_NOT_VALID_CNS - 1);
    }


    __LOG(("NOTE: sip 5 : Egress Queue Pass/Tail-Drop CNC Trigger , using 'TXQ-PORT' [%d] (not 'physical port'[%d]) \n",
        cheetah_descrPtr->txqDestPorts[egressPort],
        egressPort));

    /* Egress Queue Pass/Tail-Drop CNC Trigger */
    snetCht3CncCount(devObjPtr, cheetah_descrPtr,
                     SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,
                     cheetah_descrPtr->txqDestPorts[egressPort]);

    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    snetPipeEgressHaUnit(devObjPtr,pipe_descrPtr,egressPort);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("HA Unit");

    /* restore to TXQ */
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);

    if (pipe_descrPtr->pipe_egressPacketCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet that sent to egress port [%d] was DROPPED \n",
            egressPort));
        /* packet need to be dropped */
        return SMAIN_NOT_VALID_CNS;
    }


    /* was not dropped */
    return egressPort;
}

/**
* @internal snetPipeEgressTxqToPort function
* @endinternal
*
* @brief   let the TXQ to handle the packet to the egress port
*
* @param[in] devObjPtr                - pointer to device object.
*                                      descrPtr  - Cht frame descriptor
* @param[in] egressPort               - egress Port
*                                       indication that packet was send to the DMA of the port
*                                       COMMENTS :
*/
static GT_BOOL snetPipeEgressTxqToPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetPipeEgressTxqToPort);
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  txFifoPort;
    GT_U8  *frameDataPtr;/* pointer to egress buffer (that hold egress packet) */
    GT_U32  frameDataSize;/*number of bytes to send from egress buffer*/
    GT_U32  ii;

    /* save value for the FIRMWARE */
    pipe_descrPtr->pipe_currentEgressPort = egressPort;

    /* state explicitly packet command */
    pipe_descrPtr->pipe_egressPacketCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;


    /* call part 1 - TXQ + egress processing */
    txFifoPort = snetPipeEgressTxqToPortTillTxFifo(devObjPtr,pipe_descrPtr,egressPort);
    if( txFifoPort == SMAIN_NOT_VALID_CNS)
    {
        __LOG(("packet was dropped, will not egress port [%d] \n",
            egressPort));
        /* packet was dropped */
        return GT_FALSE;
    }
    else
    if( txFifoPort == (SMAIN_NOT_VALID_CNS - 1))
    {
        /* packet was queued ... no more processing for it */
        return GT_FALSE;
    }

    frameDataSize = cheetah_descrPtr->egressByteCount;
    frameDataPtr = devObjPtr->egressBuffer;

    /* the txDmaMacPort hold the the txFifo port but as 'global device port' */
    txFifoPort = cheetah_descrPtr->egressPhysicalPortInfo.txDmaMacPort;

    /* All packets need 4 Bytes CRC calculation */
    cheetah_descrPtr->calcCrc = 1;
    /* All packets need padding to minimal 64 bytes (including the CRC) */
    if (frameDataSize <= SGT_MIN_FRAME_LEN)
    {
        for(ii = frameDataSize ; ii < (SGT_MIN_FRAME_LEN-4) ; ii++)
        {
            /* PAD with ZERO */
            frameDataPtr[ii] = 0;
        }
        frameDataSize = SGT_MIN_FRAME_LEN;
    }

    /* call part 2  - TxFifo + MAC */
    egressTxFifoAndMac_forPipeDevice(devObjPtr,cheetah_descrPtr,txFifoPort,frameDataPtr,frameDataSize);

    return GT_TRUE;
}

/**
* @internal pipeTxqPreperations function
* @endinternal
*
* @brief   Pipe device preparations for the TXQ (TXQ of 'DX' device)
*/
static GT_VOID pipeTxqPreperations
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  egressPort;
    GT_U32  numOfPorts = 17;

    cheetah_descrPtr->dp =  pipe_descrPtr->pipe_DP;
    cheetah_descrPtr->tc =  pipe_descrPtr->pipe_TC;

    cheetah_descrPtr->queue_dp       = cheetah_descrPtr->dp;
    cheetah_descrPtr->queue_priority = cheetah_descrPtr->tc;

    for(egressPort = 0 ; egressPort < numOfPorts ; egressPort++)
    {
        cheetah_descrPtr->txqDestPorts[egressPort] = egressPort;
    }
}


/**
* @internal snetPipeEgressPacketProc function
* @endinternal
*
* @brief   Egress pipe Processing (for PIPE device)
*/
GT_VOID snetPipeEgressPacketProc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(snetPipeEgressPacketProc);

    SKERNEL_FRAME_PIPE_DESCR_STC *newDescPtr;/* (pointer to)new descriptor info */
    GT_BIT    isFirst = 1;/*is first packet sent */

    GT_U32  pipe_ForwardingPortmap = pipe_descrPtr->pipe_ForwardingPortmap;
    GT_BIT  dropped;/* indication that replication was dropped */
    GT_U32  numOfPorts = 17;
    GT_U32  egressPort;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);

    if(pipe_ForwardingPortmap == 0)
    {
        __LOG(("WARNING: No ports to send ... (all replications filtered by ingress pipe) \n"));
    }
    else
    {
        __LOG((SIM_LOG_IMPORTANT_INFO_STR("Start TXQ Replications:")
                "To ports BMP[0x%x] \n",
                pipe_ForwardingPortmap));

        SIM_PIPE_LOG_PACKET_DESCR_SAVE
        pipeTxqPreperations(devObjPtr,pipe_descrPtr);
        SIM_PIPE_LOG_PACKET_DESCR_COMPARE("TXQ preparations ");

    }

    /* Get pointer to duplicated descriptor (after TXQ modifications) */
    /* duplicate descriptor from the ingress core */
    newDescPtr = snetPipeDuplicateDescr(devObjPtr,pipe_descrPtr);

    for(egressPort = 0 ; egressPort < numOfPorts ; egressPort++)
    {
        if(0 == (pipe_ForwardingPortmap & (1<<egressPort)))
        {
            /* this port is not part of the forwarding bmp */
            continue;
        }

        if(isFirst == 0)
        {
            /* on each sending (except for first one) ... need to copy values
               saved from the original descriptor */
            *pipe_descrPtr = *newDescPtr;
        }

        isFirst = 0;

        __LOG(("Start egress processing for port [%d] \n",
            egressPort));

        if(GT_FALSE == snetPipeEgressTxqToPort(devObjPtr,pipe_descrPtr,egressPort))
        {
            dropped = 1;
        }
        else
        {
            dropped = 0;
        }

        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

        __LOG((SIM_LOG_OUT_REPLICATION_STR,
            "Ended processing replication that [%s]: "
            "   deviceName[%s],deviceId[%d], \n"
            "   portGroupId[%d],port[%d] \n",
                dropped ? "DROPPED" : "egress",
                devObjPtr->deviceName,
                devObjPtr->deviceId,
                devObjPtr->portGroupId ,
                egressPort));
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    if(pipe_ForwardingPortmap != 0)
    {
        __LOG(("end loop on ports to get packets \n"));
    }

    return;
}

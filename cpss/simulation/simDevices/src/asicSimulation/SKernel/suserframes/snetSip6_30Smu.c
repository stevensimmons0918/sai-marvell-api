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
* @file snetSip6_30Smu.c
*
* @brief (Sip6_30) SMU (Stream management unit) processing for frame -- simulation
*
* @version   1
********************************************************************************
*/
#include <os/simTypesBind.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahIngress.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <common/Utils/Math/sMath.h>


/**
* @internal incrementIrfCounter function
* @endinternal
*
* @brief   do the sip6.30 SMU-IRF increment of value to counter.
*/
static GT_VOID incrementIrfCounter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               index,
    IN GT_U32               *countersMemPtr,
    IN SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_ENT  counterType,
    IN GT_U32               incrementValue
)
{
    DECLARE_FUNC_NAME(incrementIrfCounter);

    GT_U32  origValue;
    GT_U64  u64Value;
    GT_U64  u64Add;

    if(counterType == SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E)
    {
        /* the field is NOT wraparound */
        origValue = SMEM_SIP6_30_SMU_IRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,index,counterType);

        if((origValue + incrementValue) >= (1<<11))
        {
            /* we are not allowed to increment as it will cause wraparound */
            __LOG(("IRF_DISCARDED_PACKETS_COUNTER : not increment as it is not 'wraparound counter' \n"));
        }
        else
        {
            /* we are allowed to increment */
            SMEM_SIP6_30_SMU_IRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,index,counterType,origValue + incrementValue);
        }
    }
    else
    if(counterType == SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E)
    {
        /* u64Add = addValue */
        CNV_U32_TO_U64(incrementValue,u64Add);

        SMEM_SIP6_30_SMU_IRF_COUNTERS_BYTES_FIELD_GET(devObjPtr,countersMemPtr,index,u64Value.l);

        /* u64Value = u64Value + u64Add */
        u64Value =
            prvSimMathAdd64(u64Value, u64Add);

        SMEM_SIP6_30_SMU_IRF_COUNTERS_BYTES_FIELD_SET(devObjPtr,countersMemPtr,index,u64Value.l);
    }
    else
    {
        origValue = SMEM_SIP6_30_SMU_IRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,index,counterType);
        SMEM_SIP6_30_SMU_IRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,index,counterType,origValue + incrementValue);
    }

}

/**
* @internal smuIrfByteCountCalc function
* @endinternal
*
* @brief   do the sip6.30 SMU-IRF calc the L2/L3 byte count of the packet.
*   logic based from cnc : function cncByteCountGet(...)
*/
static GT_U32 smuIrfByteCountCalc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   irf_sng_counting_mode
)
{
    DECLARE_FUNC_NAME(smuIrfByteCountCalc);

    GT_U32  byteCount,l2HeaderSize,l3ByteCount;

    l2HeaderSize = descrPtr->l2HeaderSize;

    if(descrPtr->tunnelTerminated == 0)
    {
        byteCount = descrPtr->byteCount;
    }
    else
    {
        byteCount = descrPtr->origByteCount;
        if(descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_ETHERNET_WITH_CRC)
        {
            __LOG(("CNC : for diff between L3 and L2 ignore 4 CRC bytes of TT with Ethernet passenger (beside CRC of packet) \n"));
            l2HeaderSize += 4;/* this is the CRC of the passenger */
        }
    }

    l3ByteCount = byteCount - l2HeaderSize;
    /* For L3 byte count mode the 4 L2 CRC bytes are also subtract */
    l3ByteCount -= 4;

    return irf_sng_counting_mode ? l3ByteCount : byteCount;
}

/**
* @internal snetSip6_30SmuIrfProcess function
* @endinternal
*
* @brief   do the sip6.30 SMU-IRF (Individual Recovery Function) processing.
*/
static GT_VOID snetSip6_30SmuIrfProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30SmuIrfProcess);

    GT_U32  regValue,irf_global_enable,recovery_sequence_num,global_config;
    GT_U32  irf_same_sequence_id_command,irf_same_sequence_id_drop_code;
    GT_U32  SeqNr,byteCount,irf_sng_counting_mode;
    GT_U32  *countersMemPtr,*memPtr;

    smemRegGet(devObjPtr,
        SMEM_SIP6_30_SMU_IRF_SNG_GLOBAL_CONFIG_REG(devObjPtr) , &global_config);

    irf_global_enable = SMEM_U32_GET_FIELD(global_config,1,1);
    __LOG_PARAM(irf_global_enable);


    if(!irf_global_enable)
    {
        __LOG(("SMU : The IRF is globally disabled \n"));
        return;
    }

    memPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_SMU_SNG_IRF_TBL_MEM(devObjPtr,descrPtr->streamId));

    countersMemPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_SMU_IRF_COUNTERS_TBL_MEM(devObjPtr,descrPtr->streamId));

    SeqNr = descrPtr->up1     << 13 |
            descrPtr->cfidei1 << 12 |
            descrPtr->vid1;
    __LOG_PARAM(SeqNr);

    if(descrPtr->sip6_isCtByteCount)
    {
        __LOG(("The byte count considered 'Cut Through BC' , so not inclement the IRF_BYTE_COUNT \n"));
    }
    else
    {
        irf_sng_counting_mode = SMEM_U32_GET_FIELD(global_config,17,1);
        __LOG_PARAM(irf_sng_counting_mode);
        byteCount = smuIrfByteCountCalc(devObjPtr,descrPtr,irf_sng_counting_mode);
        incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
            SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E                        ,
            byteCount);
    }

    smemRegFldGet(devObjPtr,
        SMEM_SIP6_30_SMU_IRF_SNG_IRF_AGE_BIT_REG(devObjPtr, descrPtr->streamId >> 5),
        descrPtr->streamId & 0x1f , 1 , &regValue);

    __LOG(("StreamId [%d] : Set the <age bit> = 1 (was [%d]) \n" ,
        descrPtr->streamId,regValue));
    smemRegFldSet(devObjPtr,
        SMEM_SIP6_30_SMU_IRF_SNG_IRF_AGE_BIT_REG(devObjPtr, descrPtr->streamId >> 5),
        descrPtr->streamId & 0x1f , 1 , 1);

    if(descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS] == 0)
    {
        __LOG(("The packet is untagged (tag less) \n"));
        incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
            SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_NUMBER_OF_TAGLESS_PACKETS_COUNTER_E ,
            1);

        incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
            SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E            ,
            1);

        return;
    }

    if(SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_TAKE_ANY_E))
    {
        /* reset the 'take any' (after used it) */
        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_TAKE_ANY_E,
            0);

        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E,
            SeqNr);


        incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
            SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E            ,
            1);

        return;
    }

    recovery_sequence_num = SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E);

    if(SeqNr == recovery_sequence_num)
    {
        __LOG(("SeqNr[%d] == recovery_sequence_num \n",SeqNr));

        incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
            SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E         ,
            1);

        if(SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_INDIVIDUAL_RECOVERY_ENABLED_E))
        {
            irf_same_sequence_id_command = SMEM_U32_GET_FIELD(global_config,2,3);
            __LOG_PARAM(irf_same_sequence_id_command);
            irf_same_sequence_id_drop_code = SMEM_U32_GET_FIELD(global_config,5,8);
            __LOG_PARAM(irf_same_sequence_id_drop_code);

            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                descrPtr->packetCmd,/* previous command     */
                irf_same_sequence_id_command,          /* command to resolve   */
                descrPtr->cpuCode,  /* CPU code command     */
                irf_same_sequence_id_drop_code,        /* new CPU code         */
                SNET_CHEETAH_ENGINE_UNIT_SMU_E,
                GT_FALSE);
        }
        else
        {
            incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
                SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E            ,
                1);
        }
    }
    else  /*SeqNr != recovery_sequence_num*/
    {
        __LOG(("SeqNr[%d] != recovery_sequence_num[%d] \n",SeqNr,recovery_sequence_num));

        __LOG_PARAM((SeqNr+1));

        if(recovery_sequence_num != ((SeqNr+1) & 0xFFFF))
        {
            incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
                SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_OUT_OF_ORDER_PACKETS_COUNTER_E      ,
                1);
        }

        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E,
            SeqNr);

        incrementIrfCounter(devObjPtr,descrPtr->streamId , countersMemPtr ,
            SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E            ,
            1);
    }

}
/**
* @internal snetSip6_30SmuSngProcess function
* @endinternal
*
* @brief   do the sip6.30 SMU-SNG (generate sequence numbers) processing.
*/
static GT_VOID snetSip6_30SmuSngProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30SmuSngProcess);

    GT_U32  sng_global_enable,sng_src_id_bit_add_tag1;
    GT_U32  currentSeqNum;
    GT_U32  *memPtr;
    GT_U32  global_config;

    smemRegGet(devObjPtr,
        SMEM_SIP6_30_SMU_IRF_SNG_GLOBAL_CONFIG_REG(devObjPtr) , &global_config);

    sng_global_enable = SMEM_U32_GET_FIELD(global_config,0,1);
    __LOG_PARAM(sng_global_enable);

    if(descrPtr->streamId == 0)
    {
        __LOG(("SMU : do not access to SNG IRF table ,because streamId = 0 (not assigned) \n"));
        return;
    }

    if(0 == sng_global_enable)
    {
        __LOG(("SMU : the SNG (generate sequence numbers) globally disabled \n"));
        return;
    }

    __LOG(("SMU : Access to SNG IRF table at index  = streamId[0x%8.8x] \n",
        descrPtr->streamId));

    memPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_SMU_SNG_IRF_TBL_MEM(devObjPtr,descrPtr->streamId));

    if(SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,
        memPtr,
        descrPtr->streamId,
        SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E))
    {
        currentSeqNum = SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E);

        __LOG(("SMU : set currentSeqNum[0x%4.4x] into the descriptor \n",currentSeqNum));

        descrPtr->up1     = SMEM_U32_GET_FIELD(currentSeqNum,13, 3);
        descrPtr->cfidei1 = SMEM_U32_GET_FIELD(currentSeqNum,12, 1);
        descrPtr->vid1    = SMEM_U32_GET_FIELD(currentSeqNum, 0,12);

        __LOG_PARAM(descrPtr->up1);
        __LOG_PARAM(descrPtr->cfidei1);
        __LOG_PARAM(descrPtr->vid1);

        /* update counter only after the assignment into Desc<VID1>, Desc<UP1> and Desc<CFI1>*/
        currentSeqNum ++;

        SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(devObjPtr,
            memPtr,
            descrPtr->streamId,
            SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E,
            currentSeqNum);


        sng_src_id_bit_add_tag1 = SMEM_U32_GET_FIELD(global_config,13,4);
        __LOG_PARAM(sng_src_id_bit_add_tag1);

        if(sng_src_id_bit_add_tag1 < 12)
        {
            if(SMEM_U32_GET_FIELD(descrPtr->sstId,sng_src_id_bit_add_tag1,1))
            {
                __LOG(("SMU : bit[%d] is already set in the SRC-ID field that match [<Src-ID bit - Add Tag1>] \n",
                    sng_src_id_bit_add_tag1));
            }
            else
            {
                /* Desc<SST ID>[<Src-ID bit - Add Tag1>]=1 */
                SMEM_U32_SET_FIELD(descrPtr->sstId,sng_src_id_bit_add_tag1,1,1);

                __LOG(("SMU : set bit[%d] in the SRC-ID field according to [<Src-ID bit - Add Tag1>] \n",
                    sng_src_id_bit_add_tag1));
            }

            __LOG_PARAM(descrPtr->sstId);
        }

    }
    else
    {
        __LOG(("SMU : not update the <SNG_PACKET_COUNTER> (current SeqNum) and not assign it to the 'SeqNum' (up1,cfi1,vid1) \n"));
    }

    return;
}

/**
* @internal snetSip6_30SmuSgcProcess function
* @endinternal
*
* @brief   do the sip6.30 SMU-SGC (sequence gate control) processing.
*/
static GT_VOID snetSip6_30SmuSgcProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30SmuSgcProcess);

    __LOG(("SMU : the SGC (sequence gate control) not implemented \n"));

    return;
}

/**
* @internal snetSip6_30SmuProcess function
* @endinternal
*
* @brief   do the sip6.30 SMU processing.
*/
GT_VOID snetSip6_30SmuProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30SmuProcess);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_SMU_E);

    /* protect from CPU access to the entry , while doing 'atomic' update of 'read only' counters */
    SCIB_SEM_TAKE;

    /* used by : HSR/PRP , 802.1cb */
    __LOG(("SMU : do SNG logic \n"));
    snetSip6_30SmuSngProcess(devObjPtr,descrPtr);

    __LOG(("SMU : do IRF logic \n"));
    /* used by : 802.1cb */
    snetSip6_30SmuIrfProcess(devObjPtr,descrPtr);

    /* release the protection */
    SCIB_SEM_SIGNAL;

    snetSip6_30SmuSgcProcess(devObjPtr,descrPtr);

    return;
}


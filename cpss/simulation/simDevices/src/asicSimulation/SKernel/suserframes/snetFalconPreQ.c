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
* @file snetFalconPreQ.c
*
* @brief Falcon PreQ processing
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#ifdef _WIN32
extern void SHOSTG_psos_reg_asic_task(void);
#endif /*_WIN32*/

#define SET_CNC_STAT_VALUE(index,value,trigger)             \
    (((trigger) << SNET_CNC_PREQ_TRIGGER_BIT_INDEX_CNC) |   \
     (index) |                                              \
     (value) << (SNET_CNC_PREQ_TRIGGER_BIT_INDEX_CNC + 1))

#define __LOG_SIP6_PREQ_MIB_COUNTER(counterName,setId,oldValue,addedValue) \
    __LOG(("PREQ mib counter [%s] of set[%d] , incremented by [0x%x] from value of [0x%8.8x] \n", \
        counterName , setId ,addedValue , oldValue))

/**
* @internal snetPreqCounterSets function
* @endinternal
*
* @brief   Update egress counters of PREQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port
*                                       None
*
* @note code taken from snetChtTxQCounterSets(...)
*
*/
static void snetPreqCounterSets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetPreqCounterSets);

    GT_U32 counterRegAddr0,counterRegAddr1;/*address of the counters registers*/
    GT_U32 outCount1 = 0, outCount0 = 0;/* egress counters */
    GT_U32 set1CntrlRegData , set0CntrlRegData; /* Txq MIB Counters config register */
    GT_U32 set1CntrlRegData_port , set0CntrlRegData_port; /* Txq MIB Counters port config register */
    /* The TC of the packets counted by this set*/
    GT_U32 Set0TcCntMode , Set0TC,Set1TcCntMode, Set1TC;
    /* The VID of the packets counted by this set*/
    GT_U32 Set0VlanCntMode ,Set1VID , Set0VID , Set1VlanCntMode;
    /* The egress port number of packets counted by this set*/
    GT_U32 Set0PortCntMode,Set0Port,Set1Port,Set1PortCntMode  ;
    /* The DP of the packets counted by this set*/
    GT_U32 Set0DpCntMode , Set0DP , Set1DpCntMode , Set1DP;
    GT_U32 Counter ; /* counter */
    GT_U32  txqMibPort;/*TXQ mib port for 'eBrdige Egress Counter Set' */
    GT_U32  txqMibTc = descrPtr->tc;/*TXQ mib TC (not using descrPtr->queue_priority) */
    GT_U32  txqMibDp; /* value of the DP in the egress pipe */
    char*   counterNameStr="";

    if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E ||
        descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("packet was dropped, PREQ counters not updated\n"));
        return;
    }

    txqMibDp = descrPtr->queue_dp;
    txqMibTc = descrPtr->queue_priority;
    __LOG((
           "SIP 6:  txqMibTc use queue_priority[%d] instead of tc[%d]\n",
           descrPtr->queue_priority, descrPtr->tc));
    if(descrPtr->queue_dp != descrPtr->dp)
    {
        __LOG(("The DP       used for PreQ mib counters is the DP after the remap\n"));
    }
    if(descrPtr->queue_priority != descrPtr->tc)
    {
        __LOG(("The Priority used for PreQ mib counters is the Priority after the remap\n"));
    }

    __LOG_PARAM(txqMibTc);
    __LOG_PARAM(txqMibDp);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBCntrsSetConfig[0],
        &set0CntrlRegData);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBCntrsSetConfig[1],
        &set1CntrlRegData);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBCntrsPortSetConfig[0],
        &set0CntrlRegData_port);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBCntrsPortSetConfig[1],
        &set1CntrlRegData_port);

    /*Set 0*/
    Set0VlanCntMode = SMEM_U32_GET_FIELD(set0CntrlRegData,1,1);
    Set0TcCntMode   = SMEM_U32_GET_FIELD(set0CntrlRegData,2,1);
    Set0DpCntMode   = SMEM_U32_GET_FIELD(set0CntrlRegData,3,1);
    Set0DP          = SMEM_U32_GET_FIELD(set0CntrlRegData,30,2);
    if(SMEM_CHT_IS_SIP6_20_GET(devObjPtr))
    {
        Set0TC          = SMEM_U32_GET_FIELD(set0CntrlRegData,22,8);
        Set0VID         = SMEM_U32_GET_FIELD(set0CntrlRegData,6,16);
    }
    else
    {
        Set0TC          = SMEM_U32_GET_FIELD(set0CntrlRegData,26,4);
        Set0VID         = SMEM_U32_GET_FIELD(set0CntrlRegData,10,16);
    }

    Set0PortCntMode = SMEM_U32_GET_FIELD(set0CntrlRegData_port,0,1);
    Set0Port        = SMEM_U32_GET_FIELD(set0CntrlRegData_port,4,17);

    /*Set 1*/
    Set1VlanCntMode = SMEM_U32_GET_FIELD(set1CntrlRegData,1,1);
    Set1TcCntMode   = SMEM_U32_GET_FIELD(set1CntrlRegData,2,1);
    Set1DpCntMode   = SMEM_U32_GET_FIELD(set1CntrlRegData,3,1);
    Set1DP          = SMEM_U32_GET_FIELD(set1CntrlRegData,30,2);

    if(SMEM_CHT_IS_SIP6_20_GET(devObjPtr))
    {
        Set1TC          = SMEM_U32_GET_FIELD(set1CntrlRegData,22,8);
        Set1VID         = SMEM_U32_GET_FIELD(set1CntrlRegData,6,16);
    }
    else
    {
        Set1TC          = SMEM_U32_GET_FIELD(set1CntrlRegData,26,4);
        Set1VID         = SMEM_U32_GET_FIELD(set1CntrlRegData,10,16);
    }

    Set1PortCntMode = SMEM_U32_GET_FIELD(set1CntrlRegData_port,0,1);
    Set1Port        = SMEM_U32_GET_FIELD(set1CntrlRegData_port,4,17);

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(Set0TcCntMode      );
        __LOG_PARAM(Set0TC             );
        __LOG_PARAM(Set0VlanCntMode    );
        __LOG_PARAM(Set0VID            );
        __LOG_PARAM(Set0PortCntMode    );
        __LOG_PARAM(Set0Port           );
        __LOG_PARAM(Set0DpCntMode      );
        __LOG_PARAM(Set0DP             );
        __LOG_PARAM(Set1TcCntMode      );
        __LOG_PARAM(Set1TC             );
        __LOG_PARAM(Set1VlanCntMode    );
        __LOG_PARAM(Set1VID            );
        __LOG_PARAM(Set1PortCntMode    );
        __LOG_PARAM(Set1Port           );
        __LOG_PARAM(Set1DpCntMode      );
        __LOG_PARAM(Set1DP             );
    }

    /* not depended from counting */
    snetLionResourceHistogramCount(devObjPtr, descrPtr);

    /* packet counted by egress ports for regular counters          */
    if(descrPtr->useVidx == 0 && descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        /* logic taken from GM */
        txqMibPort = descrPtr->trgEPort;
        __LOG(("TXQ Mib Port is the target ePort [0x%x] \n" ,
            txqMibPort));
    }
    else
    {
        txqMibPort = egressPort;
        __LOG(("TXQ Mib Port is the <Local Dev Trg Phy Port> [0x%x] \n" ,
            txqMibPort));
    }

    /* default to be overriden */
    outCount0 = 1;
    if (Set0PortCntMode && (Set0Port != txqMibPort))
    {
        outCount0 = 0;
    }
    if (Set0VlanCntMode && (descrPtr->eVid != Set0VID))
    {
        outCount0 = 0;
    }
    if (Set0TcCntMode && (txqMibTc != Set0TC))
    {
        outCount0 = 0;
    }
    if (Set0DpCntMode && (txqMibDp != Set0DP))
    {
        outCount0 = 0;
    }

    /* default to be overriden */
    outCount1 = 1;
    if (Set1PortCntMode && (Set1Port != txqMibPort))
    {
        outCount1 = 0;
    }
    if (Set1VlanCntMode && (descrPtr->eVid != Set1VID))
    {
        outCount1 = 0;
    }
    if (Set1TcCntMode && (txqMibTc != Set1TC))
    {
        outCount1 = 0;
    }
    if (Set1DpCntMode && (txqMibDp != Set1DP))
    {
        outCount1 = 0;
    }

    if (outCount0 || outCount1)
    {
        if ( (descrPtr->egressPacketType == SKERNEL_EGR_PACKET_CNTRL_UCAST_E) ||
             (descrPtr->egressPacketType == SKERNEL_EGR_PACKET_CNTRL_MCAST_E) )
        {
            /* update control packets counter */
            __LOG(("update control packets counter"));
            counterRegAddr0 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBCtrlPktCntr[0];
            counterRegAddr1 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBCtrlPktCntr[1];

            counterNameStr = "egrMIBCtrlPktCntr";
        }
        else if (descrPtr->macDaType == SKERNEL_UNICAST_MAC_E)
        {
            /* update unicast packet counter */
            __LOG(("update unicast packet counter"));
            counterRegAddr0 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBOutgoingUcPktCntr[0];
            counterRegAddr1 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBOutgoingUcPktCntr[1];

            counterNameStr = "egrMIBOutgoingUcPktCntr";
        }
        else if (descrPtr->macDaType == SKERNEL_MULTICAST_MAC_E)
        {
            /* update multicast packet counter */
            __LOG(("update multicast packet counter"));
            counterRegAddr0 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBOutgoingMcPktCntr[0];
            counterRegAddr1 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBOutgoingMcPktCntr[1];

            counterNameStr = "egrMIBOutgoingMcPktCntr";
        }
        else   /* update broadcast packet counter */
        {
            __LOG(("update broadcast packet counter"));
            counterRegAddr0 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBOutgoingBcPktCntr[0];
            counterRegAddr1 =
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->PREQ.egrMIBCntrs.egrMIBOutgoingBcPktCntr[1];

            counterNameStr = "egrMIBOutgoingBcPktCntr";
        }

        if(outCount0)
        {
            smemRegGet(devObjPtr,counterRegAddr0, &Counter);
            __LOG_SIP6_PREQ_MIB_COUNTER(counterNameStr,0,Counter,outCount0);
            Counter += outCount0;
            smemRegSet(devObjPtr,counterRegAddr0, Counter);
        }

        if(outCount1)
        {
            smemRegGet(devObjPtr,counterRegAddr1, &Counter);
            __LOG_SIP6_PREQ_MIB_COUNTER(counterNameStr,1,Counter,outCount1);
            Counter += outCount1;
            smemRegSet(devObjPtr,counterRegAddr1, Counter);
        }
    }

    return;
}

/**
* @internal snetPreqTxMirror function
* @endinternal
*
* @brief   egress mirroring in SIP6 PREQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
*/
static void snetPreqTxMirror
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetPreqTxMirror);

    GT_U32  *memPtr;                /* pointer to memory */
    GT_U32  regAddr;                /* address of registers */
    GT_U32  txSniffEn;              /* Enable/Disable to analyser port */

    /* preq target physical port table */
    regAddr = SMEM_SIP6_PREQ_TARGET_PHYSICAL_PORT_TBL_MEM(devObjPtr, egressPort);
    memPtr = smemMemGet(devObjPtr, regAddr);
    /* bits [5:3]*/
    txSniffEn = snetFieldValueGet(memPtr, 3, 3);

    /*Mirror To Analyzer Index*/
    snetXcatEgressMirrorAnalyzerIndexSelect(devObjPtr,descrPtr,txSniffEn);

    if(txSniffEn >= descrPtr->analyzerIndex)
    {
        __LOG(("PREQ unit: Per egress physical port [%d] - Mirror To Analyzer Index [%d] \n",
            egressPort,
            txSniffEn));
    }
}

/**
* @internal snetPreqCnc function
* @endinternal
*
* @brief   PREQ Cnc cl;ients
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
*/
static void snetPreqCnc
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          egressPort
)
{
    DECLARE_FUNC_NAME(snetPreqCnc);

    GT_U32  *memPtr;                /* pointer to memory                      */
    GT_U32  regAddr;                /* address of registers                   */
    GT_U32  eVlanCountEn;           /* Enable/Disable eVlan counting per port */
    GT_U32  pass_drop_port_base,status_port_base;
    GT_U32  Egress_Queue_Pass_Drop_CNC_index;
    GT_U32  statistics_Queue_Pass_Drop_CNC_index;
    GT_U32  triggerStatistics,value;

    /* Egress VLAN Pass/Drop CNC Trigger - counting is done in block attached to */
    /* ingress device.                                                           */
    /* Only Tail dropped packets should be counted as dropped.                   */
    /* Simulation does not support tail drop.                                    */
    /* preq target physical port table */
    regAddr = SMEM_SIP6_PREQ_TARGET_PHYSICAL_PORT_TBL_MEM(devObjPtr, egressPort);
    memPtr = smemMemGet(devObjPtr, regAddr);
    /* bits [5:3]*/
    eVlanCountEn = snetFieldValueGet(memPtr, 0, 1);
    __LOG_PARAM(eVlanCountEn);

    if (eVlanCountEn)
    {
        snetCht3CncCount(
            descrPtr->ingressDevObjPtr, descrPtr,
            SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E, 1/*outPorts*/);
    }


    regAddr = SMEM_SIP6_PREQ_QUEUE_PORT_MAPPING_TBL_MEM(devObjPtr,
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index);
    memPtr = smemMemGet(devObjPtr, regAddr);
    pass_drop_port_base = SMEM_U32_GET_FIELD((*memPtr),0, 13);/* 13 bits */
    __LOG_PARAM(pass_drop_port_base);
    status_port_base = SMEM_U32_GET_FIELD((*memPtr),13, 13);/* 13 bits */
    __LOG_PARAM(status_port_base);

     if(SMEM_CHT_IS_SIP6_20_GET(devObjPtr))
     {
        /*check if 8 bit queue offset is enabled*/
        regAddr  = SMEM_SIP6_PREQ_GLB_CONF_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 25, 1, &value);
        if(value==0&&descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset>0xF)
        {
            descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset&=0xF;
            __LOG(("sip6_queue_group_offset is bigger then 4bit but 8bit offset was not enabled!"));
            __LOG_PARAM(descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset);
        }
     }

    Egress_Queue_Pass_Drop_CNC_index = pass_drop_port_base + descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset;
    __LOG_PARAM(Egress_Queue_Pass_Drop_CNC_index);

    /* Egress Queue Pass/Tail-Drop CNC Trigger */
    snetCht3CncCount(
        devObjPtr, descrPtr,
        SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,
        Egress_Queue_Pass_Drop_CNC_index);

    statistics_Queue_Pass_Drop_CNC_index = status_port_base + descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset;
    __LOG_PARAM(statistics_Queue_Pass_Drop_CNC_index);

    /* common to both status clients */
    triggerStatistics = 0;/* simulation not hold packets in the queue , so will
        not trigger the threshold value */

    __LOG_PARAM(triggerStatistics);

    /* convert sip6_queue_group_index,sip6_queue_group_offset to base of the port */

    /* NOTE: the CNC is always called for this client */
    /* and the CNC will decide if to update the counter and which part(s) of it !!!! */
    value = 0;/* should be the 'queue fill level' */
    /* Egress Queue statistics */
    snetCht3CncCount(
        devObjPtr, descrPtr,
        SNET_SIP6_CNC_CLIENT_PREQ_QUEUE_STATISTICS_E,
        SET_CNC_STAT_VALUE(
        statistics_Queue_Pass_Drop_CNC_index ,
        value,triggerStatistics));

    /* NOTE: the CNC is always called for this client */
    /* and the CNC will decide if to update the counter and which part(s) of it !!!! */

    value = 0;/* should be the 'port fill level' */
    /* Egress port statistics */
    snetCht3CncCount(
        devObjPtr, descrPtr,
        SNET_SIP6_CNC_CLIENT_PREQ_PORT_STATISTICS_E,
        SET_CNC_STAT_VALUE(
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index ,
        value,triggerStatistics));
}

/* copied from prvCpssPpConfigBitmapNumBitsGet */
static  GT_U32  bitcount
(
    IN  GT_U32     bitmap
)
{
    GT_U32 i;         /* loop index         */
    GT_U32  numOfBits = 0;
    /* the "num of bits" for values 0-15       */
    /* the value for 0 is not relevant                      */
    static const GT_U8 numBits[16] = {
    /* binary           num of bits */
    /*  0000          */  0 ,
    /*  0001          */  1 ,
    /*  0010          */  1 ,
    /*  0011          */  2 ,
    /*  0100          */  1 ,
    /*  0101          */  2 ,
    /*  0110          */  2 ,
    /*  0111          */  3 ,
    /*  1000          */  1 ,
    /*  1001          */  2 ,
    /*  1010          */  2 ,
    /*  1011          */  3 ,
    /*  1100          */  2 ,
    /*  1101          */  3 ,
    /*  1110          */  3 ,
    /*  1111          */  4 };

    /* loop on (up to) 8 4-bit in 32-bit bitmap        */
    for (i = 0; (i < 8) && bitmap; i++, bitmap >>= 4)
    {
        numOfBits += numBits[bitmap & 0x0F];
    }

    return numOfBits;
}

static GT_U32   preqSrf_IAS_1_30 = 1;
GT_STATUS snet6_30Preq_IAS_1_30(IN GT_U32  enable)
{
    preqSrf_IAS_1_30 = enable;
    return GT_OK;
}

/**
* @internal snet6_30PreqSrfIsApplicable function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           check if the packet applicable for the SRF
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
* @param[out] srfNumberPtr            - (pointer to) the SRF table index
*                                       (when the function return 'applicable = GT_TRUE')
*
*   return : GT_TRUE  : the packet should     do SRF
*            GT_FALSE : the packet should not do SRF
*/
static GT_BOOL snet6_30PreqSrfIsApplicable
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          egressPort,
  OUT   GT_U32                       *srfNumberPtr
)
{
    DECLARE_FUNC_NAME(snet6_30PreqSrfIsApplicable);

    GT_U32  FRE_SRF_Global_Config_1;
    GT_U32  Enable_Streams_TO_CPU;
    GT_U32  *memPtr;
    GT_U32  tempValue;
    GT_U32  portIndex;
    GT_U32  ii,totalBits = 0;

    *srfNumberPtr = 0;/* dummy */

    if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ||
       descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E)
    {
        __LOG(("EREP : SRF : not applicable for soft/hard drops \n"));
        return GT_FALSE;
    }

    if(descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E &&
       descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        __LOG(("EREP : SRF : not applicable for 'FROM_CPU' or 'TO_TRG_SNIFFER' \n"));
        return GT_FALSE;
    }

    smemRegGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_1_REG(devObjPtr),
        &FRE_SRF_Global_Config_1);

    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
    {
        if(descrPtr->localDevPacketSource != SKERNEL_SRC_RX_NOT_REPLICATED_TYPE_E)
        {
            __LOG(("EREP : SRF : not applicable for 'TO_CPU' that is mirror copy (by EQ/EREP) \n"));
            return GT_FALSE;
        }

        Enable_Streams_TO_CPU = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,17,1);
        __LOG_PARAM(Enable_Streams_TO_CPU);

        if(!Enable_Streams_TO_CPU)
        {
            __LOG(("EREP : SRF : not applicable for 'TO_CPU' as globally disabled for such packets \n"));
            return GT_FALSE;
        }
    }

    if(descrPtr->streamId == 0)
    {
        __LOG(("EREP : SRF : not applicable streamId == 0 \n"));
        return GT_FALSE;
    }


    __LOG(("EREP : Access to SRF mapping table at index  = streamId[0x%8.8x] \n",
        descrPtr->streamId));

    memPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_PREQ_SRF_MAPPING_TBL_MEM(devObjPtr,descrPtr->streamId));

    if(! SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
        descrPtr->streamId,
        SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E))
    {
        __LOG(("EREP : SRF : SRF disabled on streamId [0x%8.8x]  \n",descrPtr->streamId));
        return GT_FALSE;
    }

    /* stream type */
    tempValue = SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
        descrPtr->streamId,
        SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E);


    if(preqSrf_IAS_1_30 && tempValue == 1/*multicast*/)
    {
        /* According to IAS 1.3 */
        /* Configuration Note: In case of a LAG the user would choose one of the ports of the LAGs.
           This is port L. All the ports in the LAG would be mapped to L (In the mapping above).
           Port L would be the only port that would be set in the port bitmap for the LAG ports
        */
        smemRegGet(devObjPtr,
            SMEM_SIP6_PREQ_TARGET_PHYSICAL_PORT_TBL_MEM(devObjPtr, egressPort),
            &tempValue);
        portIndex = SMEM_U32_GET_FIELD(tempValue,6,
            devObjPtr->flexFieldNumBitsSupport.phyPort);

        __LOG(("SRF Mapping for Multicast traffic for egress physical port [%d] to [%d] index \n",
            egressPort,portIndex));
    }
    else
    {
        portIndex = egressPort;
        if(tempValue == 1)
        {
            /* According to IAS 1.2 */
            __LOG(("SRF NOT support Mapping for Multicast traffic for egress physical port [%d] to other index \n",
                egressPort));
        }
        else
        {
            /* According to IAS 1.2 and 1.3 */
            __LOG(("SRF for Unicast traffic for egress physical port [%d] \n",
                egressPort));
        }
    }

    tempValue = SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
        descrPtr->streamId,
        (SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_0_E +
        (portIndex >> 5)));

    if(0 == SMEM_U32_GET_FIELD(tempValue , (portIndex & 0x1f) , 1))
    {
        __LOG(("EREP : SRF : egress port [%d] is not in 'egress ports bmp' for streamId [0x%8.8x]  \n",
            egressPort,descrPtr->streamId));
        return GT_FALSE;
    }

    __LOG(("EREP : the packet applicable for SRF \n"));

    if(SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
        descrPtr->streamId,
        SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E))
    {
        __LOG(("The entry type is 'Multicast stream' \n"));

        /* need to count the number of bits that came before the used port in the 'ports bmp' */
        for(ii = 0 ; ii < (portIndex >> 5); ii++)
        {
            /* count number of bits in full words */
            tempValue = SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
                descrPtr->streamId,
                (SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_0_E +
                ii));

            totalBits += bitcount(tempValue);
            __LOG_PARAM(totalBits);
        }

        /* get the last word */
        tempValue = SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
            descrPtr->streamId,
            (SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_0_E +
            ii));

        /* in the last word need to count till the 'port index' */
        totalBits += bitcount(SMEM_U32_GET_FIELD(tempValue,0,portIndex));
    }
    else
    {
        __LOG(("The entry type is 'Unicast stream' \n"));
    }
    __LOG_PARAM(totalBits);

    *srfNumberPtr = totalBits +
        SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(devObjPtr,memPtr,
        descrPtr->streamId,
        SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E);


    return GT_TRUE;
}

/**
* @internal incrementSrfCounter function
* @endinternal
*
* @brief   do the sip6.30 EREP-SRF increment of value to counter.
*/
static GT_VOID incrementSrfCounter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               index,
    IN GT_U32               *countersMemPtr,
    IN SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_ENT  counterType,
    IN GT_U32               incrementValue
)
{
    GT_U32  tempValue;

    tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,index,counterType);
    tempValue += incrementValue;
    SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,index,counterType,tempValue);

    return;
}

/**
* @internal incrementSrfConfigCounter function
* @endinternal
*
* @brief   do the sip6.30 EREP-SRF increment of value to counter.
*/
static GT_VOID incrementSrfCounter_ROGUE_PKTS_CNT
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               index,
    IN GT_U32               *memPtr
)
{
    GT_U32  tempValue;

    tempValue = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,index,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E);
    tempValue ++;
    SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,index,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E,tempValue);

    return;
}


typedef enum{
    SRF_STATE_DONE_TAG_LESS_E, /* tag less operation done */
    SRF_STATE_DONE_TAKE_ANY_E, /* Take any operation done */
    SRF_STATE_DONE_NONE_E      /* no operation done */
}SRF_STATE_DONE_ENT;


/**
* @internal snet6_30PreqSrfCommonAlgorithm function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           do common logic for 'Match Recovery Algorithm' / 'Vector Recovery Algorithm'
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] srfNumber                - the SRF table index
* @param[in] SeqNr                    - the sequence number on the packet
* @param[in] memPtr                   - pointer to the SRF config entry
* @param[in] countersMemPtr           - pointer to the SRF counters entry
*
*   return SRF_STATE_DONE_ENT that indicate the operation of this function
*/
static SRF_STATE_DONE_ENT snet6_30PreqSrfCommonAlgorithm
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          srfNumber,
  IN GT_U32                          SeqNr,
  IN GT_U32                          *memPtr,
  IN GT_U32                          *countersMemPtr
)
{
    DECLARE_FUNC_NAME(snet6_30PreqSrfCommonAlgorithm);

    GT_U32  FRE_SRF_Global_Config_1;
    GT_U32  Src_ID_Indicates_Tag1,Src_ID_Add_Tag1_location,Tagless_Packets_Counting_Disable;
    GT_U32  Tagless_Packet_Command,Tagless_Packet_Drop_Code;
    GT_U32  TakeAny;
    GT_U32  tempValue;

    smemRegGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_1_REG(devObjPtr),
        &FRE_SRF_Global_Config_1);

    Src_ID_Indicates_Tag1    = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,11,1);
    Src_ID_Add_Tag1_location = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,12,4);
    Tagless_Packets_Counting_Disable = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,16,1);
    Tagless_Packet_Command = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,0,3);
    Tagless_Packet_Drop_Code = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,3,8);

    __LOG_PARAM(Src_ID_Indicates_Tag1);
    __LOG_PARAM(Src_ID_Add_Tag1_location);
    __LOG_PARAM(Tagless_Packets_Counting_Disable);

    if((descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS] == 0) &&
       (Src_ID_Indicates_Tag1 == 0 || (0 == SMEM_U32_GET_FIELD(descrPtr->sstId,Src_ID_Add_Tag1_location,1))))
    {
        /* name of 'function' from the IAS */
        __LOG(("TaglessProcedure() \n"));


        if(Src_ID_Indicates_Tag1 == 0)
        {
            __LOG(("Tag less treatment for untagged packet (tag1) and not use bit in the SSTID for untagged \n"));
        }
        else
        {
            __LOG(("Tag less treatment for untagged packet (tag1) and bit [%d] for untagged in SSTID[0x%x] is zero \n",
                Src_ID_Add_Tag1_location,descrPtr->sstId));
        }

        if(SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E))
        {
            if(Tagless_Packets_Counting_Disable == 0)
            {
                incrementSrfCounter(devObjPtr,srfNumber , countersMemPtr ,
                    SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E,
                    1);
            }

            __LOG(("restart the counter according to the threshold (refresh the counter) \n"));

            tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E);

            SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E,
                tempValue);

            __LOG(("take no sequence : Accept the packet \n"));
        }
        else /* <TakeNoSequence> == 0 */
        {
            if(Tagless_Packets_Counting_Disable == 0)
            {
                incrementSrfCounter(devObjPtr,srfNumber , countersMemPtr ,
                    SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E,
                    1);
            }

            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                descrPtr->packetCmd,/* previous command     */
                Tagless_Packet_Command,          /* command to resolve   */
                descrPtr->cpuCode,  /* CPU code command     */
                Tagless_Packet_Drop_Code,        /* new CPU code         */
                SNET_CHEETAH_ENGINE_UNIT_PREQ_E,
                GT_FALSE);

            __LOG(("not 'take no sequence' : not Accept the packet \n"));
        }

        return GT_TRUE;
    }

    /* <TakeAny> */
    smemRegFldGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_SRF_TAKE_ANY_REG(devObjPtr,srfNumber>>5),
        (srfNumber & 0x1f),1,
        &TakeAny);
    __LOG_PARAM(TakeAny);

    if(TakeAny)
    {
        __LOG(("TakeAnyProcedure() \n"));

        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal snet6_30PreqSrfMatchRecoveryAlgorithm function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           do logic of 'Match Recovery Algorithm'
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] srfNumber                - the SRF table index
* @param[in] SeqNr                    - the sequence number on the packet
* @param[in] memPtr                   - pointer to the SRF config entry
* @param[in] countersMemPtr           - pointer to the SRF counters entry
*
*   return
*/
static void snet6_30PreqSrfMatchRecoveryAlgorithm
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          srfNumber,
  IN GT_U32                          SeqNr,
  IN GT_U32                          *memPtr,
  IN GT_U32                          *countersMemPtr
)
{
    DECLARE_FUNC_NAME(snet6_30PreqSrfMatchRecoveryAlgorithm);

    GT_U32  Duplicated_Packet_Command,Duplicated_Packet_Drop_Code;
    GT_U32  FRE_SRF_Global_Config_0;
    GT_U32  tempValue,recov_seq_num;

    recov_seq_num = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E);

    if (SeqNr == recov_seq_num)
    {
        incrementSrfCounter(devObjPtr,srfNumber , countersMemPtr ,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E,
            1);

        smemRegGet(devObjPtr,
            SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_0_REG(devObjPtr),
            &FRE_SRF_Global_Config_0);

        Duplicated_Packet_Command = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_0,0,3);
        Duplicated_Packet_Drop_Code = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_0,3,8);

        __LOG_PARAM(Duplicated_Packet_Command);
        __LOG_PARAM(Duplicated_Packet_Drop_Code);

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
            descrPtr->packetCmd,/* previous command     */
            Duplicated_Packet_Command,          /* command to resolve   */
            descrPtr->cpuCode,  /* CPU code command     */
            Duplicated_Packet_Drop_Code,        /* new CPU code         */
            SNET_CHEETAH_ENGINE_UNIT_PREQ_E,
            GT_FALSE);

        __LOG(("SeqNr == recov_seq_num : not Accept the packet \n"));
    }
    else /* PacketSequenceNumber != <RecovSeqNum> */
    {
        if (SeqNr != ((recov_seq_num + 1) & 0xFFFF))
        {
            incrementSrfCounter(devObjPtr,srfNumber , countersMemPtr ,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E,
                1);
        }

        SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E,
            SeqNr);

        incrementSrfCounter(devObjPtr,srfNumber , countersMemPtr ,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E,
            1);

        __LOG(("restart the counter according to the threshold (refresh the counter) \n"));

        tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E);

        SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E,
            tempValue);

        __LOG(("Accept the packet \n"));
    }
}

/**
* @internal srfZero function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           set value in zero bit vector
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] startBit                 - start bit
* @param[in] lastBit                  - last bit
* @param[in] value                    - value to set
*
*   return
*/
static void srfZero
(
  IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
  IN GT_U32                         startBit,
  IN GT_U32                         lastBit,
  IN GT_U32                         value
)
{
    GT_U32  zeroReg;
    GT_U32  startLine = startBit / 32;
    GT_U32  lastLine  = lastBit  / 32;
    GT_U32  ii,index,numLines,numOfBits,local_startBit;
    GT_U32  regAddr0,regAddr1;
    GT_U32  usedBits = 0;
    GT_U32  value0,value1;

    numOfBits = (startBit < lastBit) ?
                lastBit + 1 - startBit :
                1024 - startBit + lastBit + 1;/* 1024 is number of bits in the 2 tables */

    numLines = (numOfBits + 31)/32;

    for(ii = startLine; ii < (startLine + numLines); ii++)
    {
        index = (ii & 0x1f);

        /* treat the memory as 32 bits even though the memory is of 16 bits width
           for each of the vectors

           this is done by 'merging' the values of 2*16 bits to single 32 bits word
           */
        regAddr0 = SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_0_TBL_MEM(devObjPtr,index);
        regAddr1 = SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_1_TBL_MEM(devObjPtr,index);

        smemRegGet(devObjPtr,regAddr0,&value0);
        smemRegGet(devObjPtr,regAddr1,&value1);

        zeroReg = (value1 << 16) | value0;

        if(ii == startLine)
        {
            /* first line */

            local_startBit = startBit & 0x1f;/*32 bits in memory width of 2 tables */

            if(numLines == 1)
            {
                SMEM_U32_SET_FIELD(zeroReg,local_startBit,numOfBits,1);
            }
            else
            {
                SMEM_U32_SET_FIELD(zeroReg,local_startBit,32-local_startBit,1);
                usedBits += 32-local_startBit;
            }

            smemRegSet(devObjPtr,regAddr0,zeroReg & 0xFFFF);
            smemRegSet(devObjPtr,regAddr1,zeroReg >> 16);
        }
        else
        if(ii == lastLine)
        {
            /* last line (that is not 'first line')*/
            SMEM_U32_SET_FIELD(zeroReg,0,numOfBits-usedBits,0);
            smemRegSet(devObjPtr,regAddr0,zeroReg & 0xFFFF);
            smemRegSet(devObjPtr,regAddr1,zeroReg >> 16);
        }
        else
        {
            usedBits += 32;
            smemRegSet(devObjPtr,regAddr0,0);
            smemRegSet(devObjPtr,regAddr1,0);
        }
    }/* ii loop */

    return;
}

static GT_U32   srfGetZero(
  IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
  IN GT_U32                         startBit
)
{
    GT_U32  zeroReg;
    GT_U32  index = startBit / 32;
    GT_U32  localBit = startBit % 32;
    GT_U32  regAddr0,regAddr1;
    GT_U32  value0,value1;

    regAddr0 = SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_0_TBL_MEM(devObjPtr,index);
    regAddr1 = SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_1_TBL_MEM(devObjPtr,index);

    smemRegGet(devObjPtr,regAddr0,&value0);
    smemRegGet(devObjPtr,regAddr1,&value1);

    zeroReg = (value1 << 16) | value0;

    return SMEM_U32_GET_FIELD(zeroReg,localBit,1);
}
static void  srfSetZero(
  IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
  IN GT_U32                         startBit,
  IN GT_U32                         value
)
{
    GT_U32  zeroReg;
    GT_U32  index = startBit / 32;
    GT_U32  localBit = startBit % 32;
    GT_U32  regAddr0,regAddr1;
    GT_U32  value0,value1;

    regAddr0 = SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_0_TBL_MEM(devObjPtr,index);
    regAddr1 = SMEM_SIP6_30_PREQ_ZERO_BIT_VECTOR_1_TBL_MEM(devObjPtr,index);

    smemRegGet(devObjPtr,regAddr0,&value0);
    smemRegGet(devObjPtr,regAddr1,&value1);

    zeroReg = (value1 << 16) | value0;

    SMEM_U32_SET_FIELD(zeroReg,localBit,1,value);

    smemRegSet(devObjPtr,regAddr0,zeroReg & 0xFFFF);
    smemRegSet(devObjPtr,regAddr1,zeroReg >> 16);

    return;
}

/* Translate (delta) => <Buffer Num, Bit Num> */
static void translateDeltaToBuffAndBit
(
    IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
    IN  GT_32   delta ,

    IN  GT_U32  firstBuff,
    IN  GT_U32  currBuff,
    IN  GT_U32  currBit,
    IN  GT_U32  ActualNumberOfBuffers,

    OUT GT_U32  *buffPtr ,
    OUT GT_U32  *bitBuffPtr
)
{
    DECLARE_FUNC_NAME(translateDeltaToBuffAndBit);

    GT_32  BitNum,BufferNum,y;
    /*
    BitNum= (<CurBit> + delta) mod 128
    y= (<CurBit> + delta) div 128
    BufferNum= <FirstBuffer> + (<CurBuffer> + y - <FirstBuffer>) mod <ActualNumberOfBuffers>
    */
    BitNum = (currBuff + delta) / 128;
    y      = (currBuff + delta) % 128;
    BufferNum =  firstBuff + (currBuff + y - firstBuff) % ActualNumberOfBuffers;

    /*
    Note: a= (a div 128) * 128 + (a mod 128).
          If a < 0 then (a div 128) <= a/128
    Note: <SHL> <= <ActualNumberOfBuffers>*128
    */

    if(BufferNum < (GT_32)firstBuff)
    {
        __LOG(("WARNING : ERROR in calculation of 'BufferNum' \n"));
    }
    else
    if(BitNum < 0)
    {
        __LOG(("WARNING : ERROR in calculation of 'BitNum' \n"));
    }

    *buffPtr    = (GT_U32)BufferNum;
    *bitBuffPtr = (GT_U32)BitNum;

    return;
}


static GT_U32 HistoryDeltaNeg(
    IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
    IN GT_U32                          srfNumber,
    IN GT_U32                          *countersMemPtr,

    IN  GT_32   delta ,

    IN  GT_U32  firstBuff,
    IN  GT_U32  currBuff,
    IN  GT_U32  currBit,
    IN  GT_U32  ActualNumberOfBuffers
)
{
    DECLARE_FUNC_NAME(HistoryDeltaNeg);

    GT_U32  BufferNum,BitNum;
    GT_U32  Zero_BufferNum;
    GT_U32  pkt_received_bit;
    GT_U32  *histMemPtr,*currHistMemPtr;
    GT_U32  bitCount,ii;

    /*-32768<=delta<32768*/
    /*Translate (delta) => <BufferNum, BitNum>*/

    translateDeltaToBuffAndBit(devObjPtr,delta ,firstBuff,currBuff,currBit,ActualNumberOfBuffers,
        &BufferNum , &BitNum);
    __LOG_PARAM(BufferNum);
    __LOG_PARAM(BitNum);

    Zero_BufferNum = srfGetZero(devObjPtr,BufferNum);
    __LOG_PARAM(Zero_BufferNum);

    histMemPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_PREQ_HISTROY_BUFFER_TBL_MEM(devObjPtr,BufferNum));

    if (Zero_BufferNum == 0)  /* HistoryBuffers[BufferNum,*] is updated */
    {
        /*pkt_received_bit = HistoryBuffers[BufferNum, BitNum];*/
        pkt_received_bit = snetFieldValueGet(histMemPtr,BitNum,1);
        __LOG_PARAM(pkt_received_bit);

        /*HistoryBuffers[BufferNum, BitNum]=1;*/
        snetFieldValueSet(histMemPtr,BitNum,1,1);

        return pkt_received_bit;
    }

    if(BufferNum != currBuff)
    /* HistoryBuffers[BufferNum,*] should be read as 0 */
    {
        /*y = Number of zero bits in HistoryBuffers[BufferNum, 0:127];*/
        bitCount = bitcount(histMemPtr[0]) +
                   bitcount(histMemPtr[1]) +
                   bitcount(histMemPtr[2]) +
                   bitcount(histMemPtr[3]) ;

        /*<ClearedZeroHistoryBitsCounter>+= y;*/
        incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E,
            bitCount);

        /*HistoryBuffers[BufferNum, 0:127]=0;*/
        histMemPtr[0] = 0;
        histMemPtr[1] = 0;
        histMemPtr[2] = 0;
        histMemPtr[3] = 0;

        /*HistoryBuffers[BufferNum, BitNum]=1;*/
        snetFieldValueSet(histMemPtr,BitNum,1,1);

        /*Zero[BufferNum]= 0 */ /* HistoryBuffers[BuffersNum, *] is now updated*/
        srfSetZero(devObjPtr,BufferNum,0);

        return 0;
    }
    else /* BufferNum == <CurBuffer>*/
    /* HistoryBuffers[BufferNum,0:<CurBit>-1] should be read as 0 */
    {
        currHistMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_HISTROY_BUFFER_TBL_MEM(devObjPtr,currBuff));

        bitCount = 0;
        /*y= Number of zero bits in HistoryBuffers[<CurBuffer>, 0:<CurBit>-1];*/
        for(ii = 0 ; ii < 3 ; ii++)
        {
            if(currBit < ((ii*32)+32))
            {
                /* treated outside the loop */
                break;
            }
            bitCount += bitcount(currHistMemPtr[ii]);
        }

        bitCount += bitcount(SMEM_U32_GET_FIELD(currHistMemPtr[ii],0,currBit%32));

        /*<ClearedZeroHistoryBitsCounter>+= y;*/
        incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E,
            bitCount);

        /*HistoryBuffers[BufferNum, 0:<CurBit>-1]=0;*/
        for(ii = 0 ; ii < 3 ; ii++)
        {
            if(currBit < ((ii*32)+32))
            {
                /* treated outside the loop */
                break;
            }
            histMemPtr[ii] = 0;
        }
        snetFieldValueSet(histMemPtr,ii*32,currBit%32,0);

        /*pkt_received_bit= HistoryBuffers[BufferNum, BitNum];*/
        pkt_received_bit = snetFieldValueGet(histMemPtr,BitNum,1);
        __LOG_PARAM(pkt_received_bit);
        /*HistoryBuffers[BufferNum, BitNum]=1;*/
        snetFieldValueSet(histMemPtr,BitNum,1,1);

        /*Zero[BufferNum]= 0;*/ /* HistoryBuffers[BuffersNum, *] is now updated */
        srfSetZero(devObjPtr,BufferNum,0);

        return pkt_received_bit;
    }
}

static void HistoryDeltaPos(
    IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
    IN GT_U32                          srfNumber,
    IN GT_U32                          *memPtr,
    IN GT_U32                          *countersMemPtr,

    IN  GT_32   delta ,

    IN  GT_U32  firstBuff,
    IN  GT_U32  currBuff,
    IN  GT_U32  currBit,
    IN  GT_U32  ActualNumberOfBuffers
)
{
    DECLARE_FUNC_NAME(HistoryDeltaPos);

    GT_U32  BufferNum,BitNum;
    GT_U32  Zero_currBuff;
    GT_U32  *currHistMemPtr;
    GT_U32  bitCount,ii,x,numBits;

    /*
    shows 4 cases:
     Case 1: BufNum = CurBuffer and BufBit  > CurBit
     Case 2: BufNum > CurBuffer
     Case 3: BufNum < CurBuffer
     Case 4: BufNum = CurBuffer and BufBit <= CurBit
    */

    /*Translate (delta) => <BufferNum, BitNum>*/

    translateDeltaToBuffAndBit(devObjPtr,delta ,firstBuff,currBuff,currBit,ActualNumberOfBuffers,
        &BufferNum , &BitNum);
    __LOG_PARAM(BufferNum);
    __LOG_PARAM(BitNum);

    Zero_currBuff = srfGetZero(devObjPtr,currBuff);
    __LOG_PARAM(Zero_currBuff);

    currHistMemPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_PREQ_HISTROY_BUFFER_TBL_MEM(devObjPtr,currBuff));

    /* If Zero[<CurBuffer>] == 1 it should be set to 0 */
    /* But this requires to update HistoryBuffers[<CurBuffer>, 0:<CurBit>-1] */
    if (Zero_currBuff == 1)
    {
        if(currBit != 0)
        {
            bitCount = 0;
            /*y= Number of zero bits in HistoryBuffers[<CurBuffer>, 0:<CurBit>-1];*/
            for(ii = 0 ; ii < 3 ; ii++)
            {
                if(currBit < ((ii*32)+32))
                {
                    /* treated outside the loop */
                    break;
                }
                bitCount += bitcount(currHistMemPtr[ii]);
            }

            bitCount += bitcount(SMEM_U32_GET_FIELD(currHistMemPtr[ii],0,currBit%32));

            /*<ClearedZeroHistoryBitsCounter>+= y;*/
            incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E,
                bitCount);

            /*HistoryBuffers[CurBuffer, 0:<CurBit>-1]=0;*/
            for(ii = 0 ; ii < 3 ; ii++)
            {
                if(currBit < ((ii*32)+32))
                {
                    /* treated outside the loop */
                    break;
                }
                currHistMemPtr[ii] = 0;
            }
            snetFieldValueSet(currHistMemPtr,ii*32,currBit%32,0);

        }
        /*Zero[<CurBuffer>]=0;*/
        srfSetZero(devObjPtr,currBuff,0);
    }

    /* Now clear the bits in <CurBuffer> that should be cleared because of history */
    /* vector shifting and set <CurBit>                                            */
    /* Finds the msb in <CurBuffer> that is in the new history buffer              */
    /* For case 2,3,4 msb=127. For case 1 msb= BitNum                              */
    if (currBuff == BufferNum && currBit < BitNum)
    {
        x = BitNum;
    }
    else
    {
        x = 128;
    }

    /* Counting the number of bits that were zero before clearing in <CurBuffer> */
    bitCount = 0;
    /*y= Number of zero bits in HistoryBuffers[<CurBuffer>, <CurBit> :x-1];*/
    for(ii = 0 ; ii < 3 ; ii++)
    {
        if(currBit >= (32*(ii+1)))
        {
            /* nothing to count yet */
            continue;
        }

        if(x < ((ii*32)+32))
        {
            /* treated outside the loop */
            break;
        }

        if((currBit / 32) == ii && (0 != (currBit % 32)))
        {
            /* not count all bits in this 32 bits */
            bitCount += bitcount(SMEM_U32_GET_FIELD(currHistMemPtr[ii],currBit%32,31));
        }
        else
        {
            bitCount += bitcount(currHistMemPtr[ii]);
        }
    }

    if((currBit / 32) != (x / 32))
    {
        /* not in the same word */
        bitCount += bitcount(SMEM_U32_GET_FIELD(currHistMemPtr[ii],0,x%32));
    }
    else
    {
        /* in the same word */
        bitCount += bitcount(SMEM_U32_GET_FIELD(currHistMemPtr[ii],currBit%32,x-currBit));
    }
    /*<ClearedZeroHistoryBitsCounter>+= y;*/
    incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
        SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E,
        bitCount);

    /* Setting to 1 the bit associated with old <RecovSeqNum> */
    /*HistoryBuffers[<CurBuffer>, <CurBit>]= 1;*/
    snetFieldValueSet(currHistMemPtr,currBit,1,1);

    /* Clearing bits in <CurBuffer> */
    /*HistoryBuffers[<CurBuffer>, <CurBit>+1:x-1]= 0;*/
    numBits = x -(currBit+1);
    for(ii = 0 ; ii < 4 ; ii++)
    {
        if(numBits >= 32)
        {
            snetFieldValueSet(currHistMemPtr,currBit+1 + 32*ii,32,0);
            numBits -= 32;
        }
        else
        {
            snetFieldValueSet(currHistMemPtr,currBit+1 + 32*ii,numBits,0);
            break;/* no more*/
        }
    }

    /* Setting the Zero Vector Bits */
    /* For case 2 */
    if (BufferNum > currBuff)
    {
        /*Zero[<CurBuffer>+1: BufferNum]=1*/
        srfZero(devObjPtr,currBuff+1,BufferNum,1);
    }

    /* For case 3 */
    if (BufferNum < currBuff)
    {
        /*Zero[<FirstBuffer>: BufferNum]=1;*/
        srfZero(devObjPtr,firstBuff,BufferNum,1);
        /*Zero[<CurBuffer>+1:<FirstBuffer>+<ActualNumberOfBuffers>-1]=1;*/
        srfZero(devObjPtr,currBuff+1,firstBuff+ActualNumberOfBuffers-1,1);
    }

    /* For case 4 */
    if (currBuff == BufferNum && currBit >= BitNum)
    {
        /*Zero[<FirstBuffer>:<FirstBuffer>+<ActualNumberOfBuffers>-1]=1;*/
        srfZero(devObjPtr,firstBuff,firstBuff+ActualNumberOfBuffers-1,1);
    }

    /* Finally update <CurBuffer> and <CurBit> */
    /*<CurBuffer>= BufferNum;*/
    /*<CurBit>= BitNum;*/
    SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E,
        BufferNum);

    SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E,
        BitNum);

    return;
}

#define ABS(val) ((val) < 0) ? -(val) : (val)

/**
* @internal snet6_30PreqSrfVectorRecoveryAlgorithm function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           do logic of 'Vector Recovery Algorithm'
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] srfNumber                - the SRF table index
* @param[in] SeqNr                    - the sequence number on the packet
* @param[in] memPtr                   - pointer to the SRF config entry
* @param[in] countersMemPtr           - pointer to the SRF counters entry
*
*   return
*/
static void snet6_30PreqSrfVectorRecoveryAlgorithm
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          srfNumber,
  IN GT_U32                          SeqNr,
  IN GT_32                           SHL,

  IN GT_U32                          firstBuff,
  IN GT_U32                          currBuff,
  IN GT_U32                          currBit,
  IN GT_U32                          ActualNumberOfBuffers,

  IN GT_U32                          *memPtr,
  IN GT_U32                          *countersMemPtr
)
{
    DECLARE_FUNC_NAME(snet6_30PreqSrfVectorRecoveryAlgorithm);

    GT_U32  RecovSeqNum;
    GT_32   Diff,delta;
    GT_32   absDelta;
    GT_U32  FRE_SRF_Global_Config_0,Rogue_Packet_Command,Rogue_Packet_Drop_Code;
    GT_U32  FRE_SRF_Global_Config_1;
    GT_U32  roguePacket;
    GT_U32  CBdb_Enable;
    GT_U32  Duplicated_Packet_Command,Duplicated_Packet_Drop_Code;
    GT_U32  tempValue;

    RecovSeqNum = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E);

    Diff = (GT_32)SeqNr - (GT_32)RecovSeqNum;


    if (-32768 <= Diff && Diff < 32768)
    {
        delta = Diff;
    }
    else
    if (Diff >= 32768)
    {
        delta = Diff - 65536;
    }
    else
    /*if diff < -32768*/
    {
        delta = Diff + 65536;
    }

    __LOG(("Diff[%d] = SeqNr[%d] - RecovSeqNum[%d] ; delta = [%d] \n",
        Diff , SeqNr , RecovSeqNum , delta));

    absDelta = ABS(delta);
    roguePacket = 0;
    /* after this calculation -32768<=delta<32768;*/

    if(preqSrf_IAS_1_30)
    {
        /*x= Global<802.1CBdb>*/
        /*if (!x && (|delta| >= <SHL>)) || (x && ((delta <= -<SHL>) || (delta > <SHL>)))*/
        /* If set, enables the 802.1CB update to the Sequence Recovery Function */
        smemRegGet(devObjPtr,
            SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_1_REG(devObjPtr),
            &FRE_SRF_Global_Config_1);
        CBdb_Enable = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_1,18,1);

        __LOG_PARAM(CBdb_Enable);

        if ((!CBdb_Enable && absDelta >= SHL) ||
            ( CBdb_Enable && ((delta <= -SHL)  || (delta > SHL))))
        {
            roguePacket = 1;
        }
    }
    else
    {
        if (absDelta > SHL)
        {
            roguePacket = 1;
        }
    }

    smemRegGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_SRF_GLOBAL_CONFIG_0_REG(devObjPtr),
        &FRE_SRF_Global_Config_0);

    if(roguePacket)
    {
        incrementSrfCounter_ROGUE_PKTS_CNT(devObjPtr,srfNumber , memPtr/*in config ... not counters*/);

        Rogue_Packet_Command = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_0,11,3);
        Rogue_Packet_Drop_Code = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_0,14,8);

        __LOG_PARAM(Rogue_Packet_Command);
        __LOG_PARAM(Rogue_Packet_Drop_Code);

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
            descrPtr->packetCmd,/* previous command     */
            Rogue_Packet_Command,          /* command to resolve   */
            descrPtr->cpuCode,  /* CPU code command     */
            Rogue_Packet_Drop_Code,        /* new CPU code         */
            SNET_CHEETAH_ENGINE_UNIT_PREQ_E,
            GT_FALSE);

        __LOG(("rogue violation : not Accept the packet \n"));
    }
    else if(delta < 0)
    {
        /*if delta == 0 don’t call HistoryDeltaNeg(delta)*/
        if (delta == 0 ||
            HistoryDeltaNeg(devObjPtr,srfNumber,countersMemPtr,delta,firstBuff,currBuff,currBit,ActualNumberOfBuffers) == 1)
        {
            /*<DiscardedPacketsCounter>++;*/
            incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E,
                1);

            Duplicated_Packet_Command = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_0,0,3);
            Duplicated_Packet_Drop_Code = SMEM_U32_GET_FIELD(FRE_SRF_Global_Config_0,3,8);

            __LOG_PARAM(Duplicated_Packet_Command);
            __LOG_PARAM(Duplicated_Packet_Drop_Code);

            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                descrPtr->packetCmd,/* previous command     */
                Duplicated_Packet_Command,          /* command to resolve   */
                descrPtr->cpuCode,  /* CPU code command     */
                Duplicated_Packet_Drop_Code,        /* new CPU code         */
                SNET_CHEETAH_ENGINE_UNIT_PREQ_E,
                GT_FALSE);

        }
        else /* HistoryRead(delta) == 0*/
        {
            /*<PassedPacketsCounter>++;*/
            incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E,
                1);
            /*<OutOfOrderPacketsCounter>++;*/
            incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E,
                1);

            /*<RestartCounter>= <RestartThreshold>;*/
            tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E);

            SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E,
                tempValue);

            /*Accept the packet;*/
            __LOG(("Accept the packet \n"));
        }
    }
    else /*For 802.1CB: 0 < delta <  SHL. For 802.1CBdb: 0 <  delta <= SHL */
    {
        /*HistoryDeltaPos(delta);*/
        HistoryDeltaPos(devObjPtr,srfNumber,memPtr,countersMemPtr,delta,firstBuff,
            currBuff,currBit,ActualNumberOfBuffers);

        /*<RecovSeqNum>= PacketSequenceNumber;*/
        SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E,
            SeqNr);

        /*<PassedPacketsCounter>++;*/
        incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E,
            1);

        if (delta !=1)
        {
            /*<OutOfOrderPacketsCounter>++;*/
            incrementSrfCounter(devObjPtr,srfNumber,countersMemPtr,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E,
                1);
        }

        /*<RestartCounter>= <RestartThreshold>;*/
        tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E);

        SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,
            srfNumber,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E,
            tempValue);

        __LOG(("Accept the packet \n"));
    }
}



/**
* @internal snet6_30PreqSrfApply function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           access the SRF table and do logic
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] srfNumber                - the SRF table index
*
*   return
*/
static void snet6_30PreqSrfApply
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                           srfNumber
)
{
    DECLARE_FUNC_NAME(snet6_30PreqSrfApply);

    GT_U32  *memPtr;
    GT_U32  *countersMemPtr;
    GT_U32  tempValue;
    GT_U32  FirstBuffer,ActualNumberOfBuffers,CurrBuff,CurrBit;
    SRF_STATE_DONE_ENT  state;
    GT_U32  SeqNr;
    GT_32   SHL;

    __LOG(("EREP : Access to SRF config table at index  = srfNumber[0x%8.8x] \n",
        srfNumber));

    memPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TBL_MEM(devObjPtr,srfNumber));

    __LOG(("EREP : Access to SRF counters table at index  = srfNumber[0x%8.8x] \n",
        srfNumber));

    countersMemPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TBL_MEM(devObjPtr,srfNumber));


    SeqNr = descrPtr->up1     << 13 |
            descrPtr->cfidei1 << 12 |
            descrPtr->vid1;
    __LOG_PARAM(SeqNr);

    tempValue = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E);

    if(tempValue == 0)
    {
        __LOG(("Use the Match Recovery Algorithm \n"));

        state = snet6_30PreqSrfCommonAlgorithm(devObjPtr,descrPtr,srfNumber,SeqNr,memPtr,countersMemPtr);
        if (SRF_STATE_DONE_NONE_E != state)
        {
            /* the common check for both Match/Vector state that no more needed */
            return;
        }
        snet6_30PreqSrfMatchRecoveryAlgorithm(devObjPtr,descrPtr,srfNumber,SeqNr,memPtr,countersMemPtr);

        return;
    }

    __LOG(("Use the Vector Recovery Algorithm , with [%d] number of history buffers \n",
        tempValue + 1));
    SHL = tempValue + 1;

    ActualNumberOfBuffers = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E);

    state = snet6_30PreqSrfCommonAlgorithm(devObjPtr,descrPtr,srfNumber,SeqNr,memPtr,countersMemPtr);
    if (SRF_STATE_DONE_NONE_E != state)
    {
        /* the common check for both Match/Vector state that no more needed */
        if(state == SRF_STATE_DONE_TAKE_ANY_E)
        {
            /* need to restart the history too */
            __LOG(("HistoryRestart() \n"));

            __LOG(("Set current buffer according to first buffer \n"));

            FirstBuffer = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E);

            SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E,
                FirstBuffer);

            SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(devObjPtr,memPtr,
                srfNumber,
                SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E,
                0);


            /*Zero[<FirstBuffer>: <FirstBuffer>+<ActualNumberOfBuffers>-1]=1*/
            srfZero(devObjPtr,FirstBuffer,FirstBuffer+ActualNumberOfBuffers-1,1);
        }

        return;
    }

    FirstBuffer = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E);

    CurrBuff = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E);

    CurrBit = SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(devObjPtr,memPtr,
        srfNumber,
        SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E);


    snet6_30PreqSrfVectorRecoveryAlgorithm(devObjPtr,descrPtr,srfNumber,SeqNr,SHL,
        FirstBuffer,CurrBuff,CurrBit,ActualNumberOfBuffers,memPtr,countersMemPtr);

    return;
}

/**
* @internal snet6_30PreqSrfProcess function
* @endinternal
*
* @brief   PREQ : SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
*/
static void snet6_30PreqSrfProcess
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          egressPort
)
{
    GT_U32  srfNumber;

    if(!snet6_30PreqSrfIsApplicable(devObjPtr, descrPtr, egressPort,&srfNumber))
    {
        /* we already got all LOG info why we not doing SRF */
        return;
    }

    /* protect from CPU access to the entry , while doing 'atomic' update of 'read only' counters */
    SCIB_SEM_TAKE;
    snet6_30PreqSrfApply(devObjPtr, descrPtr, srfNumber);
    /* release the protection */
    SCIB_SEM_SIGNAL;

}

/**
* @internal snetPreqProcess function
* @endinternal
*
* @brief   PREQ - supported on SIP6 devices
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
*/
void snetPreqProcess
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          egressPort
)
{
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PREQ_E);

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        snet6_30PreqSrfProcess(devObjPtr, descrPtr, egressPort);
    }

    snetPreqCounterSets(devObjPtr, descrPtr, egressPort);

    snetPreqTxMirror(devObjPtr, descrPtr, egressPort);

    snetPreqCnc(devObjPtr, descrPtr, egressPort);
}

/**
* @internal snet6_30PreqSrfRestartDaemonLoop function
* @endinternal
*
* @brief   PREQ : restart daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           loop on the SRF Table
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
static void snet6_30PreqSrfRestartDaemonLoop
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
)
{
    GT_U32  *countersMemPtr;        /* pointer to counters memory */
    GT_U32  *daemonMemPtr;          /* pointer to daemon memory */
    GT_U32  ii,tempValue;

    for(ii = 0 ; ii < devObjPtr->limitedResources.preqSrfNum ; ii++)
    {
        countersMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_SRC_COUNTERS_TBL_MEM(devObjPtr,ii));

        SCIB_SEM_TAKE;

        tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E);
        if(tempValue == 0)
        {
            SCIB_SEM_SIGNAL;
            continue;
        }

        tempValue = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E);
        if(tempValue == 0)
        {
            SCIB_SEM_SIGNAL;
            continue;
        }

        /* <RestartCounter>-- */
        tempValue--;
        SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E,
            tempValue);

        if(tempValue == 0)
        {
            /* <TakeAny> = 1 */
            smemRegFldSet(devObjPtr,
                SMEM_SIP6_30_PREQ_FRE_SRF_TAKE_ANY_REG(devObjPtr,ii>>5),
                (ii & 0x1f),1,
                1);
        }

        daemonMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_DAEMONS_TBL_MEM(devObjPtr,ii));

        /*<NumberOfResets>++;*/
        tempValue = SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E);

        SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_SET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E,
            tempValue + 1);

        /*<ClearedZeroHistoryBitsCounter>=0;*/
        SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(devObjPtr,countersMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E,
            0);

        SCIB_SEM_SIGNAL;
    }

    return;
}

/**
* @internal snet6_30PreqSrfLatentErrorDetectionDaemonLoop function
* @endinternal
*
* @brief   PREQ : latent error Detection daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
static void snet6_30PreqSrfLatentErrorDetectionDaemonLoop
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
)
{
    GT_U32  *countersMemPtr;        /* pointer to counters memory */
    GT_U32  *daemonMemPtr;          /* pointer to daemon memory */
    GT_U32  *errorDetectedMemPtr;   /* pointer to error detected memory */
    GT_U32  ii,tempValue;
    GT_U32  PassedPacketsCounter,DiscardedPacketsCounter;
    GT_U32  diff_samples,diff;

    errorDetectedMemPtr = smemMemGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_SRF_LATENT_ERROR_DETECTED_REG(devObjPtr,0));

    for(ii = 0 ; ii < devObjPtr->limitedResources.preqSrfNum ; ii++)
    {
        daemonMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_DAEMONS_TBL_MEM(devObjPtr,ii));


        SCIB_SEM_TAKE;
        tempValue = SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E);
        if(tempValue == 0)
        {
            SCIB_SEM_SIGNAL;
            continue;
        }

        countersMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_SRC_COUNTERS_TBL_MEM(devObjPtr,ii));

        PassedPacketsCounter = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E);

        DiscardedPacketsCounter = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E);

        if(PassedPacketsCounter > DiscardedPacketsCounter)
        {
            diff = PassedPacketsCounter - DiscardedPacketsCounter;
        }
        else
        {
            diff = DiscardedPacketsCounter - PassedPacketsCounter;
        }

        tempValue = SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E);

        /*diff_samples= diff - <LE BaseDifference>;*/
        if(diff > tempValue)
        {
            diff_samples = diff - tempValue;
        }
        else
        {
            diff_samples = tempValue - diff;
        }

        tempValue = SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_DIFFERENCE_E);

        if (diff_samples > tempValue)
        {
            /* set BitVector<ErrorDetected> */
            snetFieldValueSet(errorDetectedMemPtr,ii,1,1);

            /* set <LatentErrorDetected Interrupt> */
            /* Generate interrupt  "PREQ Latent Error Detected" bit 3 */
            snetChetahDoInterrupt(devObjPtr,
                                  SMEM_SIP6_30_PREQ_INTERRUPT_CAUSE_REG(devObjPtr),
                                  SMEM_SIP6_30_PREQ_INTERRUPT_MASK_REG(devObjPtr),
                                  (1<<3),/* bit 3 is <PREQ Latent Error Detected> */
                                  0/*not used*/);
        }

        SCIB_SEM_SIGNAL;
    }

}

/**
* @internal snet6_30PreqSrfLatentErrorDetectionDaemon function
* @endinternal
*
* @brief   PREQ : latent error Detection daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
static void snet6_30PreqSrfLatentErrorDetectionDaemon
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
)
{
    GT_U32  timeToSleepInMilliSec,regValue;
    GT_U32  currentTime;
    GT_U32  timeElapsed;

    smemRegFldGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_SRF_LATENT_ERROR_PERIOD_REG(devObjPtr),
        0 , 26 , &regValue);

    /* regValue is in core clocks */
    timeToSleepInMilliSec = (regValue / 1000) / devObjPtr->coreClk;

    currentTime = SIM_OS_MAC(simOsTickGet)(); /* in milisec */

    if(0 == devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0])
    {
        devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0] = currentTime;
        /* save the first time */
        return;
    }

    if(currentTime > devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0])
    {
        timeElapsed = currentTime -  devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0];
    }
    else
    {
        timeElapsed = (0xFFFFFFFF - devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0]) + currentTime;
    }

    if(timeElapsed < timeToSleepInMilliSec)
    {
        /* not enough time waited */
        return;
    }

    /* Enable LE Daemon */
    smemRegFldGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_GLOBAL_CONFIG_REG(devObjPtr),
        0 , 3/*bits 0..2*/ , &regValue);
    if( SMEM_U32_GET_FIELD(regValue,2,1) == 0/*Enable LE Daemon*/ ||
        SMEM_U32_GET_FIELD(regValue,0,1) == 0/*Enable LE Detection Algorithm*/)
    {
        /* update the 'last time' */
        devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0] = currentTime;
        return;
    }

    /* do the loop on the SRF tables */
    snet6_30PreqSrfLatentErrorDetectionDaemonLoop(devObjPtr);

    /* update the 'last time' */
    devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[0] = currentTime;
}

/**
* @internal snet6_30PreqSrfLatentErrorResetDaemonLoop function
* @endinternal
*
* @brief   PREQ : latent error Reset daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           do the loop on the SRF tables , and for each valid decrement the seconds
*           time till reset , and apply reset on those applicable
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
static void snet6_30PreqSrfLatentErrorResetDaemonLoop
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
)
{
    GT_U32  *countersMemPtr;        /* pointer to counters memory */
    GT_U32  *daemonMemPtr;          /* pointer to daemon memory */
    GT_U32  ii;
    GT_U32  PassedPacketsCounter,DiscardedPacketsCounter;
    GT_U32  le_reset_time,TimeSinceLastReset;
    GT_U32  diff;

    for(ii = 0 ; ii < devObjPtr->limitedResources.preqSrfNum ; ii++)
    {
        daemonMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_DAEMONS_TBL_MEM(devObjPtr,ii));


        SCIB_SEM_TAKE;
        le_reset_time = SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E);
        if(le_reset_time == 0)
        {
            SCIB_SEM_SIGNAL;
            continue;
        }

        countersMemPtr = smemMemGet(devObjPtr,
            SMEM_SIP6_30_PREQ_SRC_COUNTERS_TBL_MEM(devObjPtr,ii));

        /*<LE TimeSinceLastReset>++*/
        TimeSinceLastReset = SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E);
        TimeSinceLastReset++;

        SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_SET(devObjPtr,daemonMemPtr,
            ii,
            SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E,
            TimeSinceLastReset);

        if (TimeSinceLastReset >= le_reset_time)
        {
            /*<LE TimeSinceLastReset>=0;*/
            SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_SET(devObjPtr,daemonMemPtr,
                ii,
                SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E,
                0);

            /*diff = <PassedPacketsCounter> - <DiscardedPacketsCounter>;*/
            PassedPacketsCounter = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
                ii,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E);

            DiscardedPacketsCounter = SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(devObjPtr,countersMemPtr,
                ii,
                SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E);

            if(PassedPacketsCounter > DiscardedPacketsCounter)
            {
                diff = PassedPacketsCounter - DiscardedPacketsCounter;
            }
            else
            {
                diff = DiscardedPacketsCounter - PassedPacketsCounter;
            }

            /* <LE BaseDifference>= diff */
            SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_SET(devObjPtr,daemonMemPtr,
                ii,
                SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E,
                diff);

        }

        SCIB_SEM_SIGNAL;
    }
}

/**
* @internal snet6_30PreqSrfLatentErrorResetDaemon function
* @endinternal
*
* @brief   PREQ : latent error Reset daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
static void snet6_30PreqSrfLatentErrorResetDaemon
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
)
{
    GT_U32  timeToSleepInMilliSec,regValue;
    GT_U32  currentTime;
    GT_U32  timeElapsed;

    timeToSleepInMilliSec = 1000;/* 1 second interval */

    currentTime = SIM_OS_MAC(simOsTickGet)(); /* in milisec */

    if(0 == devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1])
    {
        devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1] = currentTime;
        /* save the first time */
        return;
    }

    if(currentTime > devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1])
    {
        timeElapsed = currentTime -  devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1];
    }
    else
    {
        timeElapsed = (0xFFFFFFFF - devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1]) + currentTime;
    }

    if(timeElapsed < timeToSleepInMilliSec)
    {
        /* not enough time waited */
        return;
    }

    /*<Enable LE Daemon> , <Enable LE Detection Reset> */
    smemRegFldGet(devObjPtr,
        SMEM_SIP6_30_PREQ_FRE_GLOBAL_CONFIG_REG(devObjPtr),
        1 , 2/* bits 1,2*/ , &regValue);
    if(regValue != 3)
    {
        /* one of the bits is not enabled */
        /* update the 'last time' */
        devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1] = currentTime;
        return;
    }

    /* do the loop on the SRF tables , and for each valid decrement the seconds
       time till reset , and apply reset on those applicable */
    snet6_30PreqSrfLatentErrorResetDaemonLoop(devObjPtr);


    /* update the 'last time' */
    devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic.extParamArr[1] = currentTime;
}

/**
* @internal snet6_30PreqSrfRestartDaemon function
* @endinternal
*
* @brief   PREQ : restart daemon for SRF (Sequence Recovery Function) for 802.1cb :
*           supported on SIP6.30 devices
*           The Restart Daemon scans the SRF Table (1K entries for Ironman L)
*           The Restart Daemon usually set to 10ms between scans.
*
* @param[in] devObjPtr                - pointer to device object.
*
*/
void snet6_30PreqSrfRestartDaemonTask
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr
)
{
    GT_U32  timeToSleepInMilliSec,regValue,ii;
    SIM_OS_TASK_PURPOSE_TYPE_ENT taskType = SIM_OS_TASK_PURPOSE_TYPE_PREQ_SRF_DAEMON_E;
    /* notify that task starts to work */
    SIM_OS_MAC(simOsSemSignal)(devObjPtr->smemInitPhaseSemaphore);

#ifdef _WIN32
    /* call SHOST to register the application task in the asic task table*/
    SHOSTG_psos_reg_asic_task();
#endif /*_WIN32*/

    while(1)
    {
        /* set task type - only after SHOSTG_psos_reg_asic_task */
        SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(taskType,
            &devObjPtr->task_PreqSrfRestartDaemonCookieInfo.generic);

        /* do the Latent error detection check */
        snet6_30PreqSrfLatentErrorDetectionDaemon(devObjPtr);

        /* do the Latent error Reset check */
        snet6_30PreqSrfLatentErrorResetDaemon(devObjPtr);

        smemRegFldGet(devObjPtr,
            SMEM_SIP6_30_PREQ_FRE_SRF_RESTART_PERIOD_REG(devObjPtr),
            0 , 23 , &regValue);

        /* regValue is in core clocks */
        timeToSleepInMilliSec = (regValue / 1000) / devObjPtr->coreClk;

        for(ii = 0 ; ii < timeToSleepInMilliSec / 100 ; ii++)
        {
            devObjPtr = skernelSleep(devObjPtr,100);

            /* refresh the value in case it got larger */
            smemRegFldGet(devObjPtr,
                SMEM_SIP6_30_PREQ_FRE_SRF_RESTART_PERIOD_REG(devObjPtr),
                0 , 23 , &regValue);
            timeToSleepInMilliSec = (regValue / 1000) / devObjPtr->coreClk;
        }

        /* do the last part of the sleep */
        devObjPtr = skernelSleep(devObjPtr,timeToSleepInMilliSec % 100);

        smemRegFldGet(devObjPtr,
            SMEM_SIP6_30_PREQ_FRE_GLOBAL_CONFIG_REG(devObjPtr),
            3 , 1 , &regValue);

        if(regValue == 0)
        {
            /* <Enable Restart Daemon> is disabled */
            devObjPtr = skernelSleep(devObjPtr,100);
            continue;
        }

        /* do the loop on the SRF tables */
        snet6_30PreqSrfRestartDaemonLoop(devObjPtr);
    }
}



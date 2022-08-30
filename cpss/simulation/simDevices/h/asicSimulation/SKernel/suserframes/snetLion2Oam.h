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
* @file snetLion2Oam.h
*
* @brief Lion2 OAM Engine processing for frame -- simulation
*
* @version   12
********************************************************************************
*/
#ifndef __snetLion2Oamh
#define __snetLion2Oamh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @enum SNET_LION2_OAM_LM_COUNTING_MODE_ENT
 *
 * @brief Enumeration for OAM Unit Type.
*/
typedef enum{

    /** @brief Retain: Keep the previous decision of whether to enable
     *  Policer counting for this packet or not.
     */
    SNET_LION2_OAM_LM_COUNTING_MODE_RETAIN_E = 0,

    /** Disable Counting */
    SNET_LION2_OAM_LM_COUNTING_MODE_DISABLE_E,

    /** @brief Enable Counting: If <OAM Opcode Parsing Enable> is enabled,
     *  then OAM packets with an opcode that is equal to <OAM Counted Opcode<n>>
     *  are counted by the LM counter. - If <OAM Opcode Parsing Enable> is disabled,
     *  all packets that access the OAM table are counted by the LM counter.
     */
    SNET_LION2_OAM_LM_COUNTING_MODE_ENABLE_E

} SNET_LION2_OAM_LM_COUNTING_MODE_ENT;

/* OAM table structure */
typedef struct
{
    GT_U32 entryIndex; /* Setting by Ingress OAM/Egress OAM unit: <Flow ID>-<OAM Table Base Flow ID>. */
    GT_BIT opCodeParsingEnable; /* When enabled, the packet is treated as an OAM packet. */
    GT_BIT lmCounterCaptureEnable; /* Determines whether the OAM LM counter is captured, i.e., copied to <Green Counter Snapshot>.  */
    GT_BIT dualEndedLmEnable; /* When enabled, OAM packets whose opcode is <Dual-Ended LM Opcode> are treated as LM packets */
    SNET_LION2_OAM_LM_COUNTING_MODE_ENT lmCountingMode; /* Determines which packets are counted by the LM counters */
    GT_BIT megLevelCheckEnable; /* Determines if the packets MEG level is checked, using the <MEG level> field */
    GT_U32 megLevel; /* Determines the MEG level of the current MEP */
    GT_BIT keepaliveAgingEnable; /* Determines whether the current entry is subject to the aging mechanism */
    GT_U32 ageState; /* Determines the age state of the current entry */
    GT_U32 agingPeriodIndex; /* Determines one of eight possible aging periods */
    GT_U32 agingThreshold; /* The number of age periods that indicate loss of continuity */
    GT_BIT hashVerifyEnable; /* If enabled, the correctness of the packet header is verified by comparing the packet hash[5:0] with the <Flow Hash> */
    GT_BIT lockHashValueEnable; /* When enabled, the OAM engine does not change the value of the <Flow Hash> field */
    GT_U32 flowHash; /* Determines the hash value of the flow */
    GT_U32 offsetIndex; /* An index to the OAM Offset Table, specifying the offset in the packet where the DM timestamp or LM counter is inserted into. Relevant only for Egress OAM.*/
    GT_BIT timestampEnable; /* When enabled, the current timestamp is copied into the packet header, according to the configurable offset. Relevant only for Egress OAM. */
    GT_U32 packetCommandProfile; /* Determines the profile of the packet command according to the opcode to packet command table */
    GT_U32 cpuCodeOffset; /* If the packet is trapped/mirrored to the CPU by the OAM Opcode Packet Command table, this field is added to the CPU Code. */
    GT_BIT sourceInterfaceCheckEnable; /* Determines whether the packets source interface is checked. */
    GT_BIT sourceIsTrunk; /* Determines whether <LocalDevSRCePort / LocalDevSrcTrunk> is a trunk. */
    GT_BIT sourceInterfaceCheckMode; /* Mode for checking source interface */
    GT_U32 localDevSrcEPortTrunk; /* Includes the source ePort or the local source trunk of the packet, depending on <sourceIsTrunk> */
    GT_BIT protectionLocUpdateEnable; /* When enabled, the Aging Daemon updates the <OAM Protection LOC Status<n>> when an OAM flow n ages out. */
    GT_BIT excessKeepaliveDetectionEnable; /* If enabled, the current entry is subject to excess keepalive detection. */
    GT_U32 excessKeepalivePeriodCounter; /* This counter is incremented by 1 each time the daemon updates this entry. */
    GT_U32 excessKeepalivePeriodThreshold; /* Determines the number of Aging Periods used as the observation interval for Excess Keepalive detection */
    GT_U32 excessKeepaliveMessageCounter; /* This counter is incremented each time a keepalive message arrives and this OAM entry is accessed */
    GT_U32 excessKeepaliveMessageThreshold; /* Determines the minimal number of keepalive messages during the observation interval that trigger an Excess event */
    GT_U32 rdiCheckEnable;  /* When enabled, the packets RDI bit is compared to the <RDI Status<n>> */
    GT_U32 rdiStatus; /* This bit contains the RDI status of the current OAM flow */
    GT_U32 periodCheckEnable; /* When enabled, the packets Period field is compared to the <Keepalive TX Period Field> */
    GT_U32 keepaliveTxPeriod; /* The expected transmission period field */

    /* sip5_10 : new fields.*/
    GT_BIT locDetectionEnable;/* When enabled, Loss Of Continuity (LOC) detection is enabled for this entry.
                                 When disabled, LOC detection is disabled, but other keepalive mechanisms
                                    (excess detection, RDI detection, period verification, hash verification) may be enabled*/
}SNET_LION2_OAM_INFO_STC;

/* Enumeration of Ingress/Egress OAM Interrupts */
typedef enum {
    SNET_LION2_OAM_INTR_KEEP_ALIVE_E = 0x2,
    SNET_LION2_OAM_INTR_ADDR_ILLEGAL_OAM_ENTRY_INDEX_E = 0x4,
    SNET_LION2_OAM_INTR_MEG_LEVEL_EXCEPTION_E = 0x20,
    SNET_LION2_OAM_INTR_SRC_INTERFACE_EXCEPTION_E = 0x40,
    SNET_LION2_OAM_INTR_INVALID_KEEPALIVE_HASH_E = 0x80,
    SNET_LION2_OAM_INTR_EXCESS_KEEPALIVE_E = 0x100,
    SNET_LION2_OAM_INTR_RDI_STATUS_E = 0x200,
    SNET_LION2_OAM_INTR_TX_PERIOD_E = 0x400
}SNET_LION2_OAM_INTR_ENT;

/**
* @enum SNET_LION2_OAM_STAGE_TYPE_ENT
 *
 * @brief Enumeration for OAM Unit Type.
*/
typedef enum{

    /** Ingress OAM stage. */
    SNET_LION2_OAM_STAGE_INGRESS_E,

    /** Egress OAM stage. */
    SNET_LION2_OAM_STAGE_EGRESS_E

} SNET_LION2_OAM_STAGE_TYPE_ENT;

/**
* @enum SNET_LION2_OAM_OPCODE_TYPE_ENT
 *
 * @brief Structure of OAM opcode types
*/
typedef enum{

    /** OAM Loss Measurement Counted Opcodes. */
    SNET_LION2_OAM_OPCODE_TYPE_LM_COUNTED_E = 0,

    /** OAM SingleEnded Loss Measurement Opcodes. */
    SNET_LION2_OAM_OPCODE_TYPE_LM_SINGLE_ENDED_E,

    /** OAM DualEnded Loss Measurement Opcodes. */
    SNET_LION2_OAM_OPCODE_TYPE_LM_DUAL_ENDED_E,

    /** OAM Delay Measurement Opcodes. */
    SNET_LION2_OAM_OPCODE_TYPE_DM_E,

    /** OAM Keep Alive Opcodes. */
    SNET_LION2_OAM_OPCODE_TYPE_KEEPALIVE_E

} SNET_LION2_OAM_OPCODE_TYPE_ENT;

/**
* @internal snetLion2IOamProcess function
* @endinternal
*
* @brief   Ingress OAM processing
*/
GT_VOID snetLion2IOamProcess
(
   IN SKERNEL_DEVICE_OBJECT * devObjPtr,
   IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLion2EOamProcess function
* @endinternal
*
* @brief   Egress OAM processing
*/
GT_VOID snetLion2EOamProcess
(
   IN SKERNEL_DEVICE_OBJECT * devObjPtr,
   IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLion2OamKeepAliveAging function
* @endinternal
*
* @brief   Age out OAM table entries.
*
* @param[in] devObjPtr                - pointer to device object. 
*  
*/
GT_VOID snetLion2OamKeepAliveAging
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal snetLion2OamPsuPacketModify function
* @endinternal
*
* @brief   Packet Stamping Unit logic
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      OUTPUT:
*                                      None
*
* @note It is safe to assume that the application must prevent a situation
*       where one packet is triggered for both timestamping and loss measurement
*       counter capture.
*
*/
GT_VOID snetLion2OamPsuPacketModify
(
   IN SKERNEL_DEVICE_OBJECT * devObjPtr,
   IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetLion2PclOamDescriptorSet function
* @endinternal
*
* @brief   Set OAM relevant fields in descriptor
*/
GT_VOID snetLion2PclOamDescriptorSet
(
    IN SKERNEL_DEVICE_OBJECT                 * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC    * descrPtr,
    IN SMAIN_DIRECTION_ENT                   direction,
    IN SNET_LION3_PCL_ACTION_OAM_STC         * oamInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetLion2Oamh */




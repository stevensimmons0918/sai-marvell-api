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
* @file snetCheetahEq.h
*
* @brief
*
* @version   6
********************************************************************************
*/
#ifndef __snetCheetahEqh
#define __snetCheetahEqh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum{
    SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_TRUNK_E,
    SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_L2_ECMP_E,
    SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_L3_ECMP_E,
    SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_L2_DLB_E
}SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_ENT;

/**
* @struct SNET_CHT_EQ_INTERNAL_DESC_STC
 *
 * @brief EQ internal descriptor info.
*/
typedef struct{

    /** @brief is the sniffer targeted to trunk
     *  0 - to port (ePort)
     *  1 - to trunk
     */
    GT_BIT sniffTargetIsTrunk;

    /** @brief the trunk ID of sniffer trunk
     *  (valid when sniffTargetIsTrunk == 1)
     */
    GT_U32 sniffTargeTrunkId;

} SNET_CHT_EQ_INTERNAL_DESC_STC;


/**
* @internal snetChtEq function
* @endinternal
*
* @brief   EQ block processing
*/
GT_VOID snetChtEq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtEqTxMirror function
* @endinternal
*
* @brief   Send to Tx Sniffer or To CPU by Egress STC. Called from TxQ unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
GT_VOID  snetChtEqTxMirror
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtEqDoTargetSniff function
* @endinternal
*
* @brief   Send Rx/Tx sniffed frame to Target Sniffer
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - frame data buffer Id
*                                      rxSniffDev  - Rx sniffer device
*                                      rxSniffPort - Rx sniffer port
*                                      RETURN:
*/
GT_VOID snetChtEqDoTargetSniff
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 trgSniffDev,
    IN GT_U32 trgSniffPort
);


/*******************************************************************************
*  snetChtEqDuplicateDescr
*
* DESCRIPTION:
*        Duplicate Cheetah's descriptor
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       descrPtr    - frame data buffer Id
*
* RETURN:
*       SKERNEL_FRAME_CHEETAH_DESCR_STC *
*                   - pointer to the duplicated descriptor of the Cheetah
*
*******************************************************************************/
SKERNEL_FRAME_CHEETAH_DESCR_STC * snetChtEqDuplicateDescr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/*******************************************************************************
*  snetChtEqDuplicateDescrForTxqEnqueue
*
* DESCRIPTION:
*        Duplicate Cheetah's descriptor
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       descrPtr    - frame data buffer Id
*
* RETURN:
*       SKERNEL_FRAME_CHEETAH_DESCR_STC *
*                   - pointer to the duplicated descriptor of the Cheetah
*
*******************************************************************************/
SKERNEL_FRAME_CHEETAH_DESCR_STC * snetChtEqDuplicateDescrForTxqEnqueue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtEqDoToCpuNoCallToTxq function
* @endinternal
*
* @brief   To CPU frame handling from the egress pipe line - don't call the TXQ.
*/
extern GT_VOID snetChtEqDoToCpuNoCallToTxq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtEqHashIndexResolution function
* @endinternal
*
* @brief   EQ/IPvX - Hash Index Resolution (for eArch)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] numOfMembers             - number of members to select index from.
*                                      this number may be different for each packet.
*                                      In ePort ECMP this number specifies the number of ECMP members,
*                                      in the range 1-64.
*                                      In Trunk member selection the possible values of #members is 1-8.
* @param[in] randomEcmpPathEnable     -      Random ECMP Path Enable
* @param[in] instanceType             - instance type : trunk/ecmp .
* @param[in,out] selectedEPortPtr         - (pointer to) the primary EPort (for
*                                      SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_L2_ECMP_E)
*
* @param[out] newHashIndexPtr          - pointer to the new hash index to be used by the calling
*                                      engine.
*                                      the selected member. In ePort ECMP this is a number in the range 0-63,
*                                      while in trunk member selection it is a number in the range 0-7
*                                      NOTE: this value should not modify the descriptor value.
* @param[out] selectedDevNumPtr        - (pointer to) the selected devNum field
* @param[in,out] selectedEPortPtr         - (pointer to) the selected EPort field
*/
GT_VOID snetChtEqHashIndexResolution
(
    IN SKERNEL_DEVICE_OBJECT                          *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC                *descrPtr,
    IN GT_U32                                          numOfMembers,
    IN GT_U32                                          randomEcmpPathEnable,
    OUT GT_U32                                        *newHashIndexPtr,
    IN SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_ENT  instanceType,
    OUT GT_U32                                        *selectedDevNumPtr,
    INOUT GT_U32                                      *selectedEPortPtr
);

/**
* @internal snetEqTablesFormatInit function
* @endinternal
*
* @brief   init the format of EQ tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetEqTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);

/**
* @internal snetChtEqDuplicateToCpuFromEgress function
* @endinternal
*
* @brief Send to CPU from the Egress engine
* Supported from SIP6 devices
*/
extern GT_VOID snetChtEqToCpuFromEgress
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetahEqh */




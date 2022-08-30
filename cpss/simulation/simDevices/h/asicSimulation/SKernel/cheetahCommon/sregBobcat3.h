/******************************************************************************
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
* @file sregBobcat3.h
*
* @brief Defines for Bobcat3 memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregBobcat3h
#define __sregBobcat3h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SIP_5_20_REG(dev,reg)  \
    (SMEM_CHT_IS_SIP5_20_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->reg : \
        SMAIN_NOT_VALID_CNS)

/* TTI - Physical Port Attribute Table */
#define SMEM_SIP5_20_TTI_PHYSICAL_PORT_2_ATTRIBUTE_TBL_MEM(dev, phyPortNum) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ttiPhysicalPort2Attribute, phyPortNum)

/* EQ: TX OAM Protection LOC Status Table */
#define SMEM_SIP5_20_EQ_OAM_PROTECTION_LOC_STATUS_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, oamTxProtectionLocStatusTable, index)


/* EGF QAG - port target Attribute Table */
#define SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(dev, phyPortNum) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egfQagPortTargetAttribute, phyPortNum)

/* EGF QAG - port source Attribute Table */
#define SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTE_TBL_MEM(dev, phyPortNum) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egfQagPortSourceAttribute, phyPortNum)


/* EGF QAG - TC,DP mapper Table */
#define SMEM_SIP5_20_EGF_QAG_TC_DP_MAPPER_TBL_MEM(dev, phyPortNum) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egfQagTcDpMapper, phyPortNum)



/*TXDMA : localDevSrcPortToDmaNumberMap[] */
#define SMEM_SIP5_20_TXDMA_LOCAL_DEV_SRC_PORT_TO_DMA_NUMBER_MAP_REG(dev,dpUnit,index) \
    SIP_5_20_REG(devObjPtr,TXDMA[dpUnit].TxDMAGlobalConfigurations.localDevSrcPortToDmaNumberMap[index])

/* IA: Ingress Aggregator PIP enable register */
#define SMEM_SIP_5_20_IA_PIP_ENABLE_REG(dev, index) \
    SIP_5_20_REG(dev, ingressAggregator[index].PIPEnable)

/* IA: Ingress Aggregator PIP priority thresholds0 register */
#define SMEM_SIP_5_20_IA_PIP_PRIO_THRESH0_REG(dev, index) \
    SIP_5_20_REG(dev, ingressAggregator[index].PIPPrioThresholds0)

/* IA: Ingress Aggregator PIP priority thresholds1 register */
#define SMEM_SIP_5_20_IA_PIP_PRIO_THRESH1_REG(dev, index) \
    SIP_5_20_REG(dev, ingressAggregator[index].PIPPrioThresholds1)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregBobcat3h */



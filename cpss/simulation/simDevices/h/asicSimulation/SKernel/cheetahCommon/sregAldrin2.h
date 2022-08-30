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
* @file sregAldrin2.h
*
* @brief Defines for Armsrtrong2 memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregAldrin2h
#define __sregAldrin2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SIP_5_25_REG(dev,reg)  \
    (SMEM_CHT_IS_SIP5_25_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->reg : \
        SMAIN_NOT_VALID_CNS)

/* IPVX ECMP Pointer Entry */
#define SMEM_SIP5_25_IPVX_ECMP_POINTER_ENTRY_TBL_MEM(dev,entry_indx)          \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ipvxEcmpPointer, entry_indx)

/* IPVX - Router Additional Control 3 */
#define SMEM_SIP5_25_IPVX_ROUTER_ADDITIONAL_CONTROL_3_REG(dev)    \
    (SMEM_CHT_IS_SIP5_25_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->IPvX.routerGlobalCtrl.routerGlobalCtrl3 : \
    SMAIN_NOT_VALID_CNS)

/* HA - Global Config2 Register */
#define SMEM_SIP5_25_HA_GLOBAL_CONFIG_2_REG(dev)    \
    (SMEM_CHT_IS_SIP5_25_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.haGlobalConfig2 : \
    SMAIN_NOT_VALID_CNS)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregAldrin2h */



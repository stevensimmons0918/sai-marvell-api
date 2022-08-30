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
* @file cpssDxChTypes.h
*
* @brief CPSS DXCH Generic types.
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChTypesh
#define __cpssDxChTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>

/**
* @struct CPSS_DXCH_OUTPUT_INTERFACE_STC
 *
 * @brief Defines the interface info
*/

typedef struct
{
    GT_BOOL                 isTunnelStart;

    struct{
        CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType;
        GT_U32                              ptr;
    }tunnelStartInfo;

    CPSS_INTERFACE_INFO_STC physicalInterface;
} CPSS_DXCH_OUTPUT_INTERFACE_STC;

/**
* @enum CPSS_DXCH_MEMBER_SELECTION_MODE_ENT
 *
 * @brief enumerator that hold values for the type of how many bits
 * are used in a member selection function.
 * Used for trunk member selection and by L2 ECMP member selection.
*/
typedef enum{

    /** @brief Use the entire 12 bit hash in the member selection function.
     *  ((Hash[11:0] #members)/4096)
     */
    CPSS_DXCH_MEMBER_SELECTION_MODE_12_BITS_E = 0,

    /** @brief Use only the 6 least significant bits in the member selection.
     *  ((Hash[5:0] #members)/64)
     */
    CPSS_DXCH_MEMBER_SELECTION_MODE_6_LSB_E,

    /** @brief Use only the 6 most significant bits in the member selection.
     *  ((Hash[11:6] #members)/64)
     */
    CPSS_DXCH_MEMBER_SELECTION_MODE_6_MSB_E

} CPSS_DXCH_MEMBER_SELECTION_MODE_ENT;

/**
* @enum CPSS_DXCH_FDB_LEARN_PRIORITY_ENT
 *
 * @brief Enumeration of FDB Learn Priority
*/
typedef enum{

    /** low priority */
    CPSS_DXCH_FDB_LEARN_PRIORITY_LOW_E,

    /** high priority */
    CPSS_DXCH_FDB_LEARN_PRIORITY_HIGH_E

} CPSS_DXCH_FDB_LEARN_PRIORITY_ENT;

/**
* @enum CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT
 *
 * @brief the possible forms to write a pair of entries
*/
typedef enum{

    /** @brief reads/writes just the
     *  first entry of the pair.
     */
    CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E                = 0,

    /** @brief reads/writes
     *  just the second entry of the pair and the next pointer
     */
    CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E  = 1,

    /** reads/writes the whole pair. */
    CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E                     = 2

} CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT;


/**
* @enum CPSS_DXCH_MLL_ACCESS_CHECK_MIN_MAX_ENT
 *
 * @brief whether a limit is used as upper limit or lower limit
*/
typedef enum{

    /** MLL pointer limit is used as a lower limit */
    CPSS_DXCH_MLL_ACCESS_CHECK_MIN_ENT = 0,

    /** MLL pointer limit is used as a upper limit */
    CPSS_DXCH_MLL_ACCESS_CHECK_MAX_ENT = 1

} CPSS_DXCH_MLL_ACCESS_CHECK_MIN_MAX_ENT;

/**
* @enum CPSS_DXCH_ETHERTYPE_TABLE_ENT
 *
 * @brief This enumeration defines etherType table selection
*/
typedef enum{

    /** Ingress TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E = CPSS_DIRECTION_INGRESS_E,

    /** Egress TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E = CPSS_DIRECTION_EGRESS_E,

    /** Egress Logical Port TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_LOGICAL_PORT_E,

    /** Egress tunnel start TPID table */
    CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_TS_E

} CPSS_DXCH_ETHERTYPE_TABLE_ENT;

/**
* @enum CPSS_DXCH_PORT_PROFILE_ENT
 *
 * @brief Type of the source/target port profile.
*/
typedef enum{

    /** network port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_NETWORK_E = GT_FALSE,

    /** cascade port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_CSCD_E = GT_TRUE,

    /** fabric port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_FABRIC_E,

    /** lopback port profile used in index calculation. */
    CPSS_DXCH_PORT_PROFILE_LOOPBACK_E,

    /** preemptive port profile used in index calculation (APPLICABLE DEVICES: AC5P).*/
    CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E

} CPSS_DXCH_PORT_PROFILE_ENT;

/**
* @enum CPSS_DXCH_UNIT_ENT
*
* @brief Defines silicon units of DxCh family.
*
*/
typedef enum {

    /** TTI unit */
    CPSS_DXCH_UNIT_TTI_E,

    /** PCL unit */
    CPSS_DXCH_UNIT_PCL_E,

    /** L2I unit */
    CPSS_DXCH_UNIT_L2I_E,

    /** FDB unit */
    CPSS_DXCH_UNIT_FDB_E,

    /** EQ unit */
    CPSS_DXCH_UNIT_EQ_E,

    /** LPM unit */
    CPSS_DXCH_UNIT_LPM_E,

    /** EGF_EFT unit */
    CPSS_DXCH_UNIT_EGF_EFT_E,

    /** EGF_QAG unit */
    CPSS_DXCH_UNIT_EGF_QAG_E,

    /** EGF_SHT unit */
    CPSS_DXCH_UNIT_EGF_SHT_E,

    /** HA unit */
    CPSS_DXCH_UNIT_HA_E,

    /** ETS unit */
    CPSS_DXCH_UNIT_ETS_E,

    /** MLL unit */
    CPSS_DXCH_UNIT_MLL_E,

    /** IPLR unit */
    CPSS_DXCH_UNIT_IPLR_E,

    /** EPLR unit */
    CPSS_DXCH_UNIT_EPLR_E,

    /** IPvX unit */
    CPSS_DXCH_UNIT_IPVX_E,

    /** Ingress OAM unit */
    CPSS_DXCH_UNIT_IOAM_E,

    /** Egress OAM unit */
    CPSS_DXCH_UNIT_EOAM_E,

    /** TCAM unit */
    CPSS_DXCH_UNIT_TCAM_E,

    /** RXDMA unit */
    CPSS_DXCH_UNIT_RXDMA_E,

    /** EPCL unit */
    CPSS_DXCH_UNIT_EPCL_E,

    /** TM_FCU unit */
    CPSS_DXCH_UNIT_TM_FCU_E,

    /** TM_DROP unit */
    CPSS_DXCH_UNIT_TM_DROP_E,

    /** TM_QMAP unit */
    CPSS_DXCH_UNIT_TM_QMAP_E,

    /** TM unit */
    CPSS_DXCH_UNIT_TM_E,

    /** GOP unit */
    CPSS_DXCH_UNIT_GOP_E,

    /** MIB unit */
    CPSS_DXCH_UNIT_MIB_E,

    /** SERDES unit */
    CPSS_DXCH_UNIT_SERDES_E,

    /** ERMRK unit */
    CPSS_DXCH_UNIT_ERMRK_E,

    /** BM unit */
    CPSS_DXCH_UNIT_BM_E,

    /** TM_INGRESS_GLUE */
    CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E,

    /** TM_EGRESS_GLUE */
    CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,

    /** ETH_TXFIFO unit */
    CPSS_DXCH_UNIT_ETH_TXFIFO_E,

    /** ILKN_TXFIFO unit */
    CPSS_DXCH_UNIT_ILKN_TXFIFO_E,

    /** ILKN unit */
    CPSS_DXCH_UNIT_ILKN_E,

    /** LMS0_0 unit */
    CPSS_DXCH_UNIT_LMS0_0_E,

    /** LMS0_1 unit */
    CPSS_DXCH_UNIT_LMS0_1_E,

    /** LMS0_2 unit */
    CPSS_DXCH_UNIT_LMS0_2_E,

    /** LMS0_3 unit */
    CPSS_DXCH_UNIT_LMS0_3_E,

    /** TX_FIFO unit */
    CPSS_DXCH_UNIT_TX_FIFO_E,

    /** BMA unit */
    CPSS_DXCH_UNIT_BMA_E,

    /** CNC_0 unit */
    CPSS_DXCH_UNIT_CNC_0_E,

    /** CNC_1 unit */
    CPSS_DXCH_UNIT_CNC_1_E,

    /** TXQ_QUEUE unit */
    CPSS_DXCH_UNIT_TXQ_QUEUE_E,

    /** TXQ_LL unit */
    CPSS_DXCH_UNIT_TXQ_LL_E,

    /** TXQ_PFC unit */
    CPSS_DXCH_UNIT_TXQ_PFC_E,

    /** TXQ_QCN unit */
    CPSS_DXCH_UNIT_TXQ_QCN_E,

    /** TXQ_DQ unit */
    CPSS_DXCH_UNIT_TXQ_DQ_E,

    /** DFX_SERVER unit */
    CPSS_DXCH_UNIT_DFX_SERVER_E,

    /** MPPM unit */
    CPSS_DXCH_UNIT_MPPM_E,

    /** LMS1_0 unit */
    CPSS_DXCH_UNIT_LMS1_0_E,

    /** LMS1_1 unit */
    CPSS_DXCH_UNIT_LMS1_1_E,

    /** LMS1_2 unit */
    CPSS_DXCH_UNIT_LMS1_2_E,

    /** LMS1_3 unit */
    CPSS_DXCH_UNIT_LMS1_3_E,

    /** LMS2_0 unit */
    CPSS_DXCH_UNIT_LMS2_0_E,

    /** LMS2_1 unit */
    CPSS_DXCH_UNIT_LMS2_1_E,

    /** LMS2_2 unit */
    CPSS_DXCH_UNIT_LMS2_2_E,

    /** LMS2_3 unit */
    CPSS_DXCH_UNIT_LMS2_3_E,

    /** MPPM_1 unit */
    CPSS_DXCH_UNIT_MPPM_1_E,

    /** CTU_0 unit */
    CPSS_DXCH_UNIT_CTU_0_E,

    /** CTU_1 unit */
    CPSS_DXCH_UNIT_CTU_1_E,

    /** TXQ_SHT unit */
    CPSS_DXCH_UNIT_TXQ_SHT_E,

    /** TXQ_EGR0 */
    CPSS_DXCH_UNIT_TXQ_EGR0_E,

    /** TXQ_EGR1 unit */
    CPSS_DXCH_UNIT_TXQ_EGR1_E,

    /** TXQ_DIST unit */
    CPSS_DXCH_UNIT_TXQ_DIST_E,

    /** IPLR_1 unit */
    CPSS_DXCH_UNIT_IPLR_1_E,

    /** TXDMA unit */
    CPSS_DXCH_UNIT_TXDMA_E,

    /** @brief MG unit
     *  NOTE: in Falcon is also known as MG_0_0 (sub unit in MNG unit)
     */
    CPSS_DXCH_UNIT_MG_E,

    /** TCC IPCL unit */
    CPSS_DXCH_UNIT_TCC_IPCL_E,

    /** TCC IPVX unit */
    CPSS_DXCH_UNIT_TCC_IPVX_E,

    /** RFU in CNM unit */
    CPSS_DXCH_UNIT_CNM_RFU_E,

    /** MPP_RFU in CNM unit */
    CPSS_DXCH_UNIT_CNM_MPP_RFU_E,

    /** AAC in CNM unit */
    CPSS_DXCH_UNIT_CNM_AAC_E,

    /** SMI_0 unit */
    CPSS_DXCH_UNIT_SMI_0_E,

    /** SMI_1 unit */
    CPSS_DXCH_UNIT_SMI_1_E,

    /** SMI_2 unit */
    CPSS_DXCH_UNIT_SMI_2_E,

    /** SMI_3 unit */
    CPSS_DXCH_UNIT_SMI_3_E,

    /** LED_0 unit */
    CPSS_DXCH_UNIT_LED_0_E,

    /** LED_1 unit */
    CPSS_DXCH_UNIT_LED_1_E,

    /** LED_2 unit */
    CPSS_DXCH_UNIT_LED_2_E,

    /** LED_3 unit */
    CPSS_DXCH_UNIT_LED_3_E,

    /** LED_4 unit */
    CPSS_DXCH_UNIT_LED_4_E,

    /** RXDMA unit index 1 (bobk)*/
    CPSS_DXCH_UNIT_RXDMA1_E,

    /** TXDMA unit index 1 (bobk)*/
    CPSS_DXCH_UNIT_TXDMA1_E,

    /** TX_FIFO unit index 1 (bobk) */
    CPSS_DXCH_UNIT_TX_FIFO1_E,

    /** ETH_TXFIFO1 unit index 1 (bobk) */
    CPSS_DXCH_UNIT_ETH_TXFIFO1_E,

    /** RXDMA GLUE unit (bobk) */
    CPSS_DXCH_UNIT_RXDMA_GLUE_E,

    /** TXDMA GLUE unit (bobk) */
    CPSS_DXCH_UNIT_TXDMA_GLUE_E,

    /** RxDMA unit index 2 (bobcat3) */
    CPSS_DXCH_UNIT_RXDMA2_E,

    /** @brief RxDMA unit index 3 (bobcat3)
     * BC3: pipe 1 , Falcon : Pipe 0
     */
    CPSS_DXCH_UNIT_RXDMA3_E,

    /** @brief RxDMa unit index 4 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 1
     */
    CPSS_DXCH_UNIT_RXDMA4_E,

    /** @brief RxDMA unit index 5 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 1
     */
    CPSS_DXCH_UNIT_RXDMA5_E,

    /** @brief RxDMA unit index 6
     *  Falcon: pipe 1
     */
    CPSS_DXCH_UNIT_RXDMA6_E,

    /** @brief RxDMA unit index 7
     *  Falcon: pipe 1
     */
    CPSS_DXCH_UNIT_RXDMA7_E,

    /** TxDMA unit index 2 (bobcat3) */
    CPSS_DXCH_UNIT_TXDMA2_E,

    /** @brief TxDMA unit index 3 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 0
     */
    CPSS_DXCH_UNIT_TXDMA3_E,

    /** @brief TxDMA unit index 4 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 1
     */
    CPSS_DXCH_UNIT_TXDMA4_E,

    /** @brief TxDMA unit index 5 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 1
     */
    CPSS_DXCH_UNIT_TXDMA5_E,

    /** @brief TxDMA unit index 6
     *  Falcon: pipe 1
     */
    CPSS_DXCH_UNIT_TXDMA6_E,

    /** @brief TxDMA unit index 6
     *  Falcon: pipe 1
     */
    CPSS_DXCH_UNIT_TXDMA7_E,

    /** TxFIFO unit index 2 (bobcat3) */
    CPSS_DXCH_UNIT_TX_FIFO2_E,

    /** @brief TxFIFO unit index 3 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 0
     */
    CPSS_DXCH_UNIT_TX_FIFO3_E,

    /** @brief TxFIFO unit index 4 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 1
     */
    CPSS_DXCH_UNIT_TX_FIFO4_E,

    /** @brief TxFIFO unit index 5 (bobcat3)
     *  BC3: pipe 1 , Falcon : Pipe 1
     */
    CPSS_DXCH_UNIT_TX_FIFO5_E,

    /** @brief TxFIFO unit index 6
     *  Falcon: pipe 1
     */
    CPSS_DXCH_UNIT_TX_FIFO6_E,

    /** @brief TxFIFO unit index 7
     *  Falcon: pipe 1
     */
    CPSS_DXCH_UNIT_TX_FIFO7_E,

    /** txq-DQ unit index 1 (bobcat3) */
    CPSS_DXCH_UNIT_TXQ_DQ1_E,

    /** txq-DQ unit index 2 (bobcat3) */
    CPSS_DXCH_UNIT_TXQ_DQ2_E,

    /** @brief txq-DQ unit index 3 (bobcat3)
     *  BC3: pipe 1
     */
    CPSS_DXCH_UNIT_TXQ_DQ3_E,

    /** @brief txq-DQ unit index 4 (bobcat3)
     *  BC3: pipe 1
     */
    CPSS_DXCH_UNIT_TXQ_DQ4_E,

    /** @brief txq-DQ unit index 5 (bobcat3)
     *  BC3: pipe 1
     */
    CPSS_DXCH_UNIT_TXQ_DQ5_E,

    /** MIB pipe 1*/
    CPSS_DXCH_UNIT_MIB1_E,

    /** GOP pipe 1 unit */
    CPSS_DXCH_UNIT_GOP1_E,

    /** TAI for single CPSS_DXCH_UNIT_TAI_E device */
    CPSS_DXCH_UNIT_TAI_E,

    /** TAI for single CPSS_DXCH_UNIT_TAI_E device of pipe 1 (bobcat3) */
    CPSS_DXCH_UNIT_TAI1_E,

    /** EGF_SHT unit of pipe 1 (bobcat3) */
    CPSS_DXCH_UNIT_EGF_SHT_1_E,

    /** TTI pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_TTI_1_E,

    /** IPCL pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_IPCL_1_E,

    /** L2I pipe 1 (bobcat3) */
    CPSS_DXCH_UNIT_L2I_1_E,

    /** IPVX pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_IPVX_1_E,

    /** IPLR pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_IPLR_0_1_E,

    /** IPLR1 pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_IPLR_1_1_E,

    /** IOAM pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_IOAM_1_E,

    /** MLL pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_MLL_1_E,

    /** EQ pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_EQ_1_E,

    /** EGF_EFT pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_EGF_EFT_1_E,

    /** CNC0 pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_CNC_0_1_E,

    /** CNC1 pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_CNC_1_1_E,

    /** SERDES pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_SERDES_1_E,

    /** HA pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_HA_1_E,

    /** ERMRK pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_ERMRK_1_E,

    /** EPCL pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_EPCL_1_E,

    /** EPLR pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_EPLR_1_E,

    /** EOAM pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_EOAM_1_E,

    /** RXDMA GLUE pipe 1 unit (bobcat3) */
    CPSS_DXCH_UNIT_RX_DMA_GLUE_1_E,

    /** POE pipe 1 unit (Aldrin) */
    CPSS_DXCH_UNIT_POE_E,

    /** TXQ_BMX unit */
    CPSS_DXCH_UNIT_TXQ_BMX_E,

    /** LPM pipe 1 unit */
    CPSS_DXCH_UNIT_LPM_1_E,

    /** IA pipe 0 unit */
    CPSS_DXCH_UNIT_IA_E,

    /** IA pipe 0 unit */
    CPSS_DXCH_UNIT_IA_1_E,

    /** EREP pipe 0 */
    CPSS_DXCH_UNIT_EREP_E,

    /** EREP pipe 1 */
    CPSS_DXCH_UNIT_EREP_1_E,

    /** PREQ pipe 0 */
    CPSS_DXCH_UNIT_PREQ_E,

    /** PREQ pipe 1 */
    CPSS_DXCH_UNIT_PREQ_1_E,

    /** Falcon : PHA unit (pipe 0) */
    CPSS_DXCH_UNIT_PHA_E,

    /** Falcon : PHA unit (pipe 1) */
    CPSS_DXCH_UNIT_PHA_1_E,

    /** Falcon : configuration for shared memories : FDB,TS/ARP/LPM/EM */
    CPSS_DXCH_UNIT_SHM_E,

    /** Falcon : MG_0_1 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_0_1_E,

    /** Falcon : MG_0_2 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_0_2_E,

    /** Falcon : MG_0_3 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_0_3_E,

    /** Falcon : MG_1_0 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_1_0_E,

    /** Falcon : MG_1_1 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_1_1_E,

    /** Falcon : MG_1_2 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_1_2_E,

    /** Falcon : MG_1_3 (sub unit in MNG unit) */
    CPSS_DXCH_UNIT_MG_1_3_E,

    /** Falcon : BMA in pipe 1 . dummy for simulation purposes  */
    CPSS_DXCH_UNIT_BMA_1_E,

    /** Falcon : EGF_QAG in pipe 1 */
    CPSS_DXCH_UNIT_EGF_QAG_1_E,

    /** Falcon : Txq_PSI unit */
    CPSS_DXCH_UNIT_TXQ_PSI_E,

    /** Falcon : Txq_PDX unit */
    CPSS_DXCH_UNIT_TXQ_PDX_E,

    /** Falcon : Txq_PDX_PAC_0 unit */
    CPSS_DXCH_UNIT_TXQ_PDX_PAC_0_E,

    /** Falcon : Txq_PDX_PAC_1 unit */
    CPSS_DXCH_UNIT_TXQ_PDX_PAC_1_E,

    /** Falcon : Txq_PFCC unit */
    CPSS_DXCH_UNIT_TXQ_PFCC_E,

    /** Falcon : EM */
    CPSS_DXCH_UNIT_EM_E,

    /** Falcon : Txq_PDS - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,

    /** Falcon : Txq_PDS - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_PDS1_E,

    /** Falcon : Txq_PDS - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_PDS2_E,

    /** Falcon : Txq_PDS - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_PDS3_E,

    /** Falcon : Txq_PDS - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_PDS0_E,

    /** Falcon : Txq_PDS - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_PDS1_E,

    /** Falcon : Txq_PDS - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_PDS2_E,

    /** Falcon : Txq_PDS - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_PDS3_E,

    /** Falcon : Txq_SDQ - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,

    /** Falcon : Txq_SDQ - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E,

    /** Falcon : Txq_SDQ - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E,

    /** Falcon : Txq_SDQ - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E,

    /** Falcon : Txq_SDQ - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ0_E,

    /** Falcon : Txq_SDQ - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ1_E,

    /** Falcon : Txq_SDQ - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ2_E,

    /** Falcon : Txq_SDQ - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ3_E,

    /** Falcon : Txq_QFC - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E,

    /** Falcon : Txq_QFC - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_QFC1_E,

    /** Falcon : Txq_QFC - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_QFC2_E,

    /** Falcon : Txq_QFC - pipe 0*/
    CPSS_DXCH_UNIT_TXQ_PIPE0_QFC3_E,

    /** Falcon : Txq_QFC - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_QFC0_E,

    /** Falcon : Txq_QFC - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_QFC1_E,

    /** Falcon : Txq_QFC - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_QFC2_E,

    /** Falcon : Txq_QFC - pipe 1*/
    CPSS_DXCH_UNIT_TXQ_PIPE1_QFC3_E,

    /** Falcon : TAI Slave - pipe 0*/
    CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI0_E,

    /** Falcon : TAI Slave - pipe 0*/
    CPSS_DXCH_UNIT_TAI_SLAVE_PIPE0_TAI1_E,

    /** Falcon : TAI Slave - pipe 1*/
    CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI0_E,

    /** Falcon : TAI Slave - pipe 1*/
    CPSS_DXCH_UNIT_TAI_SLAVE_PIPE1_TAI1_E,

    /** Falcon : TXQ TAI Slave - pipe 0 */
    CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI0_E,

    /** Falcon : TXQ TAI Slave - pipe 0 */
    CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE0_TAI1_E,

    /** Falcon : TXQ TAI Slave - pipe 1 */
    CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI0_E,

    /** Falcon : TXQ TAI Slave - pipe 1 */
    CPSS_DXCH_UNIT_TXQ_TAI_SLAVE_PIPE1_TAI1_E,

    /** @brief Falcon Packet Buffer Subunits
     *  Center and counters and Cell write arbiter unit
     */
    CPSS_DXCH_UNIT_PB_CENTER_BLK_E,

    /** Cell SMB Write Arbiter unit */
    CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E,

    /** Packet write subunit0 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E,

    /** Cell Read subunit0 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E,

    /** Cell Read subunit1 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E,

    /** Packet read unit0 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E,

    /** Packet read unit1 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E,

    /** Packet write subunit1 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E,

    /** Cell Read subunit2 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E,

    /** Cell Read subunit3 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E,

    /** Packet Read unit2 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E,

    /** Packet Read unit3 */
    CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E,

    /** Next Pointer Memory Cluster0 */
    CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E,

    /** Next Pointer Memory Cluster1 */
    CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E,

    /** Next Pointer Memory Cluster2 */
    CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_2_E,

    /** Shared Cell Memory Cluster0 */
    CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E,

    /** Shared Cell Memory Cluster1 */
    CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_1_E,

    /** Shared Cell Memory Cluster2 */
    CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E,

    /** Shared Cell Memory Cluster3 */
    CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_1_E,

    /** Shared Cell Memory Cluster4 */
    CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E,

    /** Shared Cell Memory Cluster5 */
    CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_1_E,

    /** CHIPLET Base Address in Falcon (access to all units in CHIPLET via D2D_CP unit)*/
    CPSS_DXCH_UNIT_CHIPLET_0_E,

    /** CHIPLET Base Address in Falcon (access to all units in CHIPLET via D2D_CP unit)*/
    CPSS_DXCH_UNIT_CHIPLET_1_E,

    /** CHIPLET Base Address in Falcon (access to all units in CHIPLET via D2D_CP unit)*/
    CPSS_DXCH_UNIT_CHIPLET_2_E,

    /** CHIPLET Base Address in Falcon (access to all units in CHIPLET via D2D_CP unit)*/
    CPSS_DXCH_UNIT_CHIPLET_3_E,

    /** CHIPLET0 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_0_TAI0_E,

    /** CHIPLET0 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_0_TAI1_E,

    /** CHIPLET1 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_1_TAI0_E,

    /** CHIPLET1 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_1_TAI1_E,

    /** CHIPLET2 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_2_TAI0_E,

    /** CHIPLET2 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_2_TAI1_E,

    /** CHIPLET3 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_3_TAI0_E,

    /** CHIPLET3 TAI base */
    CPSS_DXCH_UNIT_CHIPLET_3_TAI1_E,

    /** Falcon : pipe 0*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_0_E,

    /** Falcon : pipe 0*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_1_E,

    /** Falcon : pipe 0*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_2_E,

    /** Falcon : pipe 0*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_3_E,

    /** Falcon : pipe 1*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_4_E,

    /** Falcon : pipe 1*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_5_E,

    /** Falcon : pipe 1*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_6_E,

    /** Falcon : pipe 1*/
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_7_E,

    /** Falcon : the MAIN_DIE hold D2D CP unit in the CHIPLET memory space */
    CPSS_DXCH_UNIT_MAIN_DIE_D2D_CP_IN_CHIPLET_E,

    /** Falcon : The HBU unit */
    CPSS_DXCH_UNIT_HBU_E,

    /** Falcon : pipe1 HBU unit */
    CPSS_DXCH_UNIT_HBU_1_E,

    /** must be last */
    CPSS_DXCH_UNIT_LAST_E

}CPSS_DXCH_UNIT_ENT;


/**
 * @struct CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC
 *
 * @brief  L2 ECMP hash bit selection profile fields
 *         (APPLICABLE DEVICES: AC5P, AC5X, Harrier, Ironman)
 */
typedef struct{

    /** @brief  Determines the index of the first bit (least significant bit)
     *  of the hash that will be used for the ECMP load balancing.
     *  (APPLICABLE RANGES: 0..31)
     */
     GT_U32  firstBit;

    /** @brief  Determines the index of the last bit (most significant bit)
     *  of the hash that will be used for the ECMP load balancing.
     *  (NOTE:  lastBit >= firstBit)
     * (APPLICABLE RANGES: 1..31)
     */
     GT_U32 lastBit;

    /** @brief  The salt (16 bits) is an offset that is added to the hash
     *  determined by the <First Bit> and <Last Bit>.
     *  (APPLICABLE RANGES: 0..0xFFFF)
     */
     GT_U32 salt;

    /** @brief Determines whether the source physical port hash
     *  or packet hash is used in ECMP member selection mechanism.
     *  If enabled source physical port hash is used.
     */
     GT_BOOL srcPortHashEnable;

} CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTypesh */


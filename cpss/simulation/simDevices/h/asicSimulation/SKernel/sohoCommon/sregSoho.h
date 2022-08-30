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
* @file sregSoho.h
*
* @brief Defines API for Soho memory registers access.
*
* @version   18
********************************************************************************
*/
#ifndef __sregsohoh
#define __sregsohoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PER_PORT_LEGACY_BASE(port,offset)        (0x10100000 + (0x10000 * (port)) + ((offset)<<4))
#define PER_PORT_PERIDOT_BASE(port,offset)       (0x10000000 + (0x10000 * (port)) + ((offset)<<4))

#define PER_PORT_BASE(dev) ((dev)->soho_ports_base_addr)

#define PER_PHY_BASE(port,offset)               (0x20000000 + (0x10000 * (port)) + ((offset)<<4))
#define GLOBAL1_BASE_ADDR_REG(offset)           (0x001b0000 + ((offset)<<4))
#define GLOBAL2_BASE_ADDR_REG(offset)           (0x001c0000 + ((offset)<<4))
#define SOHO_PER_PORT_REG_ADDR(dev,port,offset) (PER_PORT_BASE(dev) + (0x10000 * (port)) + ((offset)<<4))
#define CNT_BANK_BASE_ADDR_REG(bank,port,offset)     (0x50000000 + ((bank+1)<<20) + ((port)<<16) + ((offset)<<4))
#define CNT_BANK0_BASE_ADDR_REG(port,offset)         CNT_BANK_BASE_ADDR_REG(0/*bank 0*/,port,offset)
#define CNT_BANK1_BASE_ADDR_REG(port,offset)         CNT_BANK_BASE_ADDR_REG(1/*bank 1*/,port,offset)

#define GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(offset)   (0x001e0000 + ((offset)<<4))
#define GLOBAL_TCAM_BASE_ADDR_REG(offset)           (0x001f0000 + ((offset)<<4))



/*********** addresses on : 0x001b0000 'global 1' ********/
#define GLB_STATUS_REG              GLOBAL1_BASE_ADDR_REG(0x00)
#define GLB_CTRL_REG                GLOBAL1_BASE_ADDR_REG(0x04)
#define GLB_MON_CTRL_REG            GLOBAL1_BASE_ADDR_REG(0x1a)
#define GLB_CTRL_2_REG              GLOBAL1_BASE_ADDR_REG(0x1c)
/* IP QPRI Global Mapping Register */
#define GLB_IP_QPRI_MAP_REG         GLOBAL1_BASE_ADDR_REG(0x10)

/* Global Stats Counters registers */
#define GLB_STATS_CNT3_2_REG        GLOBAL1_BASE_ADDR_REG(0x1e)
#define GLB_STATS_CNT1_0_REG        GLOBAL1_BASE_ADDR_REG(0x1f)

/* Global VTU registers */
#define GLB_VTU_OPER_REG            GLOBAL1_BASE_ADDR_REG(0x05)
#define GLB_VTU_DATA0_3_REG         GLOBAL1_BASE_ADDR_REG(0x07)
#define GLB_VTU_DATA4_7_REG         GLOBAL1_BASE_ADDR_REG(0x08)
#define GLB_VTU_DATA8_9_REG         GLOBAL1_BASE_ADDR_REG(0x09)

/* Global ATU registers */
#define GLB_ATU_CTRL_REG            GLOBAL1_BASE_ADDR_REG(0x0a)
#define GLB_ATU_OPER_REG            GLOBAL1_BASE_ADDR_REG(0x0b)
#define GLB_ATU_DATA_REG            GLOBAL1_BASE_ADDR_REG(0x0c)
#define GLB_ATU_MAC0_1_REG          GLOBAL1_BASE_ADDR_REG(0x0d)
#define GLB_ATU_MAC2_3_REG          GLOBAL1_BASE_ADDR_REG(0x0e)
#define GLB_ATU_MAC4_5_REG          GLOBAL1_BASE_ADDR_REG(0x0f)
/* Global IEEE-PRI register */
#define GLB_IEEE_PRI_REG            GLOBAL1_BASE_ADDR_REG(0x18)
/* Global core tag type */
#define GLB_CORE_TAG_TYPE_REG       GLOBAL1_BASE_ADDR_REG(0x19)
/*Stats operation register */
#define GLB_STAT_OPER_REG           GLOBAL1_BASE_ADDR_REG(0x1d)


/* Global Opal Plus registers */
#define GLB_ATU_FID_REG             GLOBAL1_BASE_ADDR_REG(0x01)
#define GLB_VTU_SID_REG             GLOBAL1_BASE_ADDR_REG(0x03)
#define GLB_MON_CTRL_REG            GLOBAL1_BASE_ADDR_REG(0x1a)

/*********** addresses on : 0x001c0000 'global 2' ********/

/* Global 2 registers */
#define GLB2_INTERUPT_SOURCE        GLOBAL2_BASE_ADDR_REG(0x00)
#define GLB2_INTERUPT_MASK          GLOBAL2_BASE_ADDR_REG(0x01)
#define GLB2_MGMT_EN_REG_2X         GLOBAL2_BASE_ADDR_REG(0x02)
#define GLB2_MGMT_EN_REG            GLOBAL2_BASE_ADDR_REG(0x03)
#define GLB2_FLOW_CTRL_REG          GLOBAL2_BASE_ADDR_REG(0x04)
#define GLB2_MNG_REG                GLOBAL2_BASE_ADDR_REG(0x05)
#define GLB2_ROUT_REG               GLOBAL2_BASE_ADDR_REG(0x06)
#define GLB2_TRUNK_MASK_REG         GLOBAL2_BASE_ADDR_REG(0x07)
#define GLB2_TRUNK_ROUT_REG         GLOBAL2_BASE_ADDR_REG(0x08)
#define GLB2_CROSS_CHIP_ADDR_REG    GLOBAL2_BASE_ADDR_REG(0x0b)
#define GLB2_CROSS_CHIP_DATA_REG    GLOBAL2_BASE_ADDR_REG(0x0c)
#define GLB2_SWITCH_MAC_REG         GLOBAL2_BASE_ADDR_REG(0x0d)
#define GLB2_ATU_STST_REG           GLOBAL2_BASE_ADDR_REG(0x0e)
#define GLB2_ENERGY_MNGMT_REG       GLOBAL2_BASE_ADDR_REG(0x12)
#define GLB2_IMP_COMM_DEBUG_REG     GLOBAL2_BASE_ADDR_REG(0x13)
#define GLB2_SMI_PHY_COMMND_REG     GLOBAL2_BASE_ADDR_REG(0x18)
#define GLB2_SMI_PHY_DATA_REG       GLOBAL2_BASE_ADDR_REG(0x19)
#define GLB2_SCRATCH_MISC_REG       GLOBAL2_BASE_ADDR_REG(0x1a)
#define GLB2_QOS_WEIGHT_REG         GLOBAL2_BASE_ADDR_REG(0x1c)
#define GLB2_DEST_POLARITY_REG      GLOBAL2_BASE_ADDR_REG(0x1d)

/*********** addresses on : 0x001f0000 'TCAM' ********/

/* TCAM registers */
#define TCAM_COMMON_REG             GLOBAL_TCAM_BASE_ADDR_REG(0x00)

/*********** addresses on : 0x001e0000 'CPU port' ********/
/* Port Registers Addresses */
#define CPU_SWITCH_PORT0_STATUS_REG      GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0)
/* Port PCS Control */
#define CPU_PORT_PCS_REG                 GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(1)
/* Port Flow Control */
#define CPU_PORT_FLOW_CONTROL_REG        GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(2)
/* Port switch identifier  */
#define CPU_PORT_SWTC_ID_REG             GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(3)
/* Port control register */
#define CPU_PORT_CTRL_REG                GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(4)
/* Port control 1 register */
#define CPU_PORT_CTRL1_REG               GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(5)
/* Port based VLAN map */
#define CPU_PORT_BASED_VLAN_MAP_REG      GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(6)
/* Default port Vlan ID & Priority */
#define CPU_PORT_DFLT_VLAN_PRI_REG       GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(7)
/* Port control 2 register */
#define CPU_PORT_CTRL_2_REG              GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(8)
/* Port Rate Control 2 */
#define CPU_PORT_RATE_CTRL_2_REG         GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0xa)
/* Port Association Vector */
#define CPU_PORT_ASSOC_VECTOR_REG        GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0xb)
/* Port Atu Control */
#define CPU_PORT_ATU_CONTROL             GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0xc)
/* Port Rate Override */
#define CPU_PORT_PRIORITY_OVERRIDE       GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0xd)
/* Port Policy control */
#define CPU_PORT_POLICY_CONTROL          GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0xe)
/* Port Etype          */
#define CPU_PORT_ETYPE                   GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0xf)
/* Port In Discard low Counter */
#define CPU_PORT_INDISCARDLOW_CNTR_REG   GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x10)
/* Port Out Discard low  Counter */
#define CPU_PORT_INDISCARDHGH_CNTR_REG   GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x11)
/* Port InFiltered Counter */
#define CPU_PORT_INFILTERED_CNTR_REG     GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x12)
/* Port OutFiltered Counter */
#define CPU_PORT_OUTFILTERED_CNTR_REG    GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x13)
/* Port IEEE Priority remapping register */
#define CPU_PORT_IEEE_PRIO_REMAP_REG     GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x18)
/* Port control 3 register */
#define CPU_PORT_CTRL_3_REG              GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x19)
/* Port Queue control register */
#define CPU_PORT_Q_CTRL_REG              GLOBAL_INTERNAL_CPU_BASE_ADDR_REG(0x1c)

/*********** addresses on : 0x01000000 or 0x00000000(peridot)  per port ********/
/* Port Registers Addresses */
#define SWITCH_PORT0_STATUS_REG(dev,port)      SOHO_PER_PORT_REG_ADDR(dev,port  ,0/*offset*/)
/* Port PCS Control */
#define PORT_PCS_REG(dev,port)                 SOHO_PER_PORT_REG_ADDR(dev,port ,1/*offset*/)
/* Port Flow Control */
#define PORT_FLOW_CONTROL_REG(dev,port)        SOHO_PER_PORT_REG_ADDR(dev,port ,2/*offset*/)
/* Port switch identifier  */
#define PORT_SWTC_ID_REG(dev,port)             SOHO_PER_PORT_REG_ADDR(dev,port ,3/*offset*/)
/* Port control register */
#define PORT_CTRL_REG(dev,port)                SOHO_PER_PORT_REG_ADDR(dev,port ,4/*offset*/)
/* Port control 1 register */
#define PORT_CTRL1_REG(dev,port)               SOHO_PER_PORT_REG_ADDR(dev,port ,5/*offset*/)
/* Port based VLAN map */
#define PORT_BASED_VLAN_MAP_REG(dev,port)      SOHO_PER_PORT_REG_ADDR(dev,port ,6/*offset*/)
/* Default port Vlan ID & Priority */
#define PORT_DFLT_VLAN_PRI_REG(dev,port)       SOHO_PER_PORT_REG_ADDR(dev,port ,7/*offset*/)
/* Port control 2 register */
#define PORT_CTRL_2_REG(dev,port)              SOHO_PER_PORT_REG_ADDR(dev,port ,8/*offset*/)
/* Port Rate Control 2 */
#define PORT_RATE_CTRL_2_REG(dev,port)         SOHO_PER_PORT_REG_ADDR(dev,port ,0xa/*offset*/)
/* Port Association Vector */
#define PORT_ASSOC_VECTOR_REG(dev,port)        SOHO_PER_PORT_REG_ADDR(dev,port ,0xb/*offset*/)
/* Port Atu Control */
#define PORT_ATU_CONTROL(dev,port)             SOHO_PER_PORT_REG_ADDR(dev,port ,0xc/*offset*/)
/* Port Rate Override */
#define PORT_PRIORITY_OVERRIDE(dev,port)       SOHO_PER_PORT_REG_ADDR(dev,port ,0xd/*offset*/)
/* Port Policy control */
#define PORT_POLICY_CONTROL(dev,port)          SOHO_PER_PORT_REG_ADDR(dev,port ,0xe/*offset*/)
/* Port Etype          */
#define PORT_ETYPE(dev,port)                   SOHO_PER_PORT_REG_ADDR(dev,port ,0xf/*offset*/)
/* Port In Discard low Counter */
#define PORT_INDISCARDLOW_CNTR_REG(dev,port)   SOHO_PER_PORT_REG_ADDR(dev,port ,0x10/*offset*/)
/* Port Out Discard low  Counter */
#define PORT_INDISCARDHGH_CNTR_REG(dev,port)   SOHO_PER_PORT_REG_ADDR(dev,port ,0x11/*offset*/)
/* Port InFiltered Counter */
#define PORT_INFILTERED_CNTR_REG(dev,port)     SOHO_PER_PORT_REG_ADDR(dev,port ,0x12/*offset*/)
/* Port OutFiltered Counter */
#define PORT_OUTFILTERED_CNTR_REG(dev,port)    SOHO_PER_PORT_REG_ADDR(dev,port ,0x13/*offset*/)
/* Port IEEE Priority remapping register */
#define PORT_IEEE_PRIO_REMAP_REG(dev,port)     SOHO_PER_PORT_REG_ADDR(dev,port ,0x18/*offset*/)
/* Port control 3 register */
#define PORT_CTRL_3_REG(dev,port)              SOHO_PER_PORT_REG_ADDR(dev,port ,0x19/*offset*/)
/* Port Queue control register */
#define PORT_Q_CTRL_REG(dev,port)              SOHO_PER_PORT_REG_ADDR(dev,port ,0x1c/*offset*/)

/*********** addresses on : 0x20000000  per PHYs ******************************/
#define PHY_CONTROL_REG(port)                 PER_PHY_BASE(port,0x0)
#define PHY_STATUS_REG(port)                  PER_PHY_BASE(port,0x01)
#define PHY_PORT_STATUS_REG(port)             PER_PHY_BASE(port,0x11)
#define PHY_INTERRUPT_ENABLE_REG(port)        PER_PHY_BASE(port,0x12)
#define PHY_INTERRUPT_STATUS_REG(port)        PER_PHY_BASE(port,0x13)
#define PHY_INTERRUPT_PORT_SUM_REG(port)      PER_PHY_BASE(port,0x14)

/*********** addresses on : 0x50000000  counters ******************************/
/* Stats ingress counters */
#define CNT_IN_GOOD_OCTETS_LO_REG(port)       CNT_BANK0_BASE_ADDR_REG(port,0x0)
#define CNT_IN_BAD_OCTETS_LO_REG(port)        CNT_BANK0_BASE_ADDR_REG(port,0x2)
#define CNT_IN_UCAST_FRAMES_REG(port)         CNT_BANK0_BASE_ADDR_REG(port,0x4)
#define CNT_IN_BCST_REG(port)                 CNT_BANK0_BASE_ADDR_REG(port,0x6)
#define CNT_IN_MCST_REG(port)                 CNT_BANK0_BASE_ADDR_REG(port,0x7)
#define CNT_IN_OVERSIZE_REG(port)             CNT_BANK0_BASE_ADDR_REG(port,0x1a)

/* Egress counters */
#define CNT_OUT_OCTETS_LO_REG(port)           CNT_BANK0_BASE_ADDR_REG(port,0xe)
#define CNT_OUT_UNICAST_FRAMES_REG(port)      CNT_BANK0_BASE_ADDR_REG(port,0x10)
#define CNT_OUT_MCST_REG(port)                CNT_BANK0_BASE_ADDR_REG(port,0x12)
#define CNT_OUT_BCST_REG(port)                CNT_BANK0_BASE_ADDR_REG(port,0x13)

/* Stats egress counters */
#define CNT_64_OCTETS_REG(port)               CNT_BANK0_BASE_ADDR_REG(port,0x8)
#define CNT_65_TO_127_OCTETS_REG(port)        CNT_BANK0_BASE_ADDR_REG(port,0x9)
#define CNT_128_TO_255_OCTETS_REG(port)       CNT_BANK0_BASE_ADDR_REG(port,0xa)
#define CNT_256_TO_511_OCTETS_REG(port)       CNT_BANK0_BASE_ADDR_REG(port,0xb)
#define CNT_512_TO_1023_OCTETS_REG(port)      CNT_BANK0_BASE_ADDR_REG(port,0xc)
#define CNT_1024_OCTETS_REG(port)             CNT_BANK0_BASE_ADDR_REG(port,0xd)

/* Bank 1 - counters */
#define CNT_BANK1_IN_DISCARDS_REG(port)       CNT_BANK1_BASE_ADDR_REG(port,0x0)
#define CNT_BANK1_IN_FILTERED_REG(port)       CNT_BANK1_BASE_ADDR_REG(port,0x1)
#define CNT_BANK1_OUT_DISCARDS_REG(port)      CNT_BANK1_BASE_ADDR_REG(port,0x1e)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregsohoh */




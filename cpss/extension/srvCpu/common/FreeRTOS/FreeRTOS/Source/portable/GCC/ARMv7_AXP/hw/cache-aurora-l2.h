/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).
*******************************************************************************/


#ifndef __INCmvCpuIfL2Regsh
#define __INCmvCpuIfL2Regsh

#define L2_MAX_COUNTERS	2

/**********************************
* Discovery Level-2 Cache registers
***********************************/

/* Cache Controls and Configurations */
#define L2_CTRL_REG							0x100
#define L2_AUX_CTRL_REG						0x104
#define L2_CNTR_CTRL_REG					0x200
#define L2_CNTR_CONFIG_REG( cntrNum )		( 0x204 + cntrNum * 0xc )
#define L2_CNTR_VAL_LOW_REG( cntrNum )		( 0x208 + cntrNum * 0xc )
#define L2_CNTR_VAL_HIGH_REG( cntrNum )		( 0x20c + cntrNum * 0xc )
#define L2_INT_CAUSE_REG					0x220
#define L2_INT_MASK_REG						0x224
#define L2_ERR_INJECT_CTRL_REG				0x228
#define L2_ECC_ERR_INJECT_MASK_REG			0x22c
#define WAY_SRAM_CONFIG_REG( wayNum )		( 0x500 + wayNum * 0x4 )
#define L2_ECC_ERR_COUNT_REG				0x600
#define L2_ECC_ERR_THRESHOLD_REG			0x604
#define L2_ECC_ERR_ADDR_CAPTURE_REG			0x608
#define L2_PARITY_ERR_WAY_SET_LATCH_REG		0x60c
#define L2_ECC_ERR_WAY_SET_LATCH_REG		0x610
#define L2_RANGE_BASE_ADDR_REG( cpuNum )	( 0x710 + cpuNum * 0x4 )
#define L2_CACHE_SYNC_REG					0x730
#define L2_INVAL_PHY_ADDR_REG				0x770
#define L2_INVAL_RANGE_REG					0x774
#define L2_INVAL_INDEX_WAY_REG				0x778
#define L2_INVAL_WAY_REG					0x77c
#define L2_CLEAN_PHY_ADDR_REG				0x7b0
#define L2_CLEAN_RANGE_REG					0x7b4
#define L2_CLEAN_INDEX_WAY_REG				0x7b8
#define L2_CLEAN_WAY_REG					0x7bc
#define L2_FLUSH_PHY_ADDR_REG				0x7f0
#define L2_FLUSH_RANGE_REG					0x7f4
#define L2_FLUSH_INDEX_WAY_REG				0x7f8
#define L2_FLUSH_WAY_REG					0x7fc
#define L2_DATA_LOC_REG( cpuNum )			( 0x900 + cpuNum * 0x8 )
#define L2_INST_LOC_REG( cpuNum )			( 0x904 + cpuNum * 0x8 )
#define L2_PFU_LOCK_REG						0x980
#define IO_BRIDGE_LOCK_REG					0x984

/*  L2_CTRL_REG ( L2CR ) */
#define L2CR_ENABLE							BIT0

/*  L2_AUX_CTRL_REG ( L2ACR ) */
#define L2ACR_FORCE_WRITE_POLICY_OFFSET		0
#define L2ACR_FORCE_WRITE_POLICY_MASK		( 0x3 << L2ACR_FORCE_WRITE_POLICY_OFFSET )
#define L2ACR_FORCE_WRITE_POLICY_DIS		( 0 << L2ACR_FORCE_WRITE_POLICY_OFFSET )
#define L2ACR_FORCE_WRITE_BACK_POLICY		( 1 << L2ACR_FORCE_WRITE_POLICY_OFFSET )
#define L2ACR_FORCE_WRITE_THRO_POLICY		( 2 << L2ACR_FORCE_WRITE_POLICY_OFFSET )
#define L2ACR_SIZE_OFFSET					10
#define L2ACR_SIZE_MASK						( 0x3 << L2ACR_SIZE_OFFSET )
#define L2ACR_SIZE_512KB					( 0x1 << L2ACR_SIZE_OFFSET )
#define L2ACR_SIZE_1MB						( 0x2 << L2ACR_SIZE_OFFSET )
#define L2ACR_SIZE_2MB						( 0x3 << L2ACR_SIZE_OFFSET )
#define L2ACR_ASSOCIATIVITY_OFFSET			13
#define L2ACR_ASSOCIATIVITY_MASK			( 0xF << L2ACR_ASSOCIATIVITY_OFFSET )
#define L2ACR_ASSOCIATIVITY_4WAY			( 3 << L2ACR_ASSOCIATIVITY_OFFSET )
#define L2ACR_ASSOCIATIVITY_8WAY			( 7 << L2ACR_ASSOCIATIVITY_OFFSET )
#define L2ACR_WAY_SIZE_OFFSET				17
#define L2ACR_WAY_SIZE_MASK					( 0x7 << L2ACR_WAY_SIZE_OFFSE )
#ifdef CONFIG_ARMADA_XP_REV_Z1
#define L2ACR_WAY_SIZE_16KB					( 1 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_32KB					( 2 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_64KB					( 3 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_128KB				( 4 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_256KB				( 5 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_512KB				( 6 << L2ACR_WAY_SIZE_OFFSET )
#else  /* A0 and over */
#define L2ACR_WAY_SIZE_16KB					( 2 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_32KB					( 3 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_64KB					( 4 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_128KB				( 5 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_256KB				( 6 << L2ACR_WAY_SIZE_OFFSET )
#define L2ACR_WAY_SIZE_512KB				( 7 << L2ACR_WAY_SIZE_OFFSET )
#endif
#define L2ACR_ECC_ENABLE					BIT20
#define L2ACR_PARITY_ENABLE					BIT21
#define L2ACR_INV_EVIC_LINE_UC_ERR			BIT22
#define L2ACR_FORCE_WR_ALLOC_OFFSET			23
#define L2ACR_FORCE_WR_ALLOC_MASK			( 0x3 << L2ACR_FORCE_WR_ALLOC_OFFSET )
#define L2ACR_FORCE_WR_ALLOC_DISABLE		( 0 << L2ACR_FORCE_WR_ALLOC_OFFSET )
#define L2ACR_FORCE_NO_WR_ALLOC				( 1 << L2ACR_FORCE_WR_ALLOC_OFFSET )
#define L2ACR_REPLACEMENT_OFFSET			27
#define L2ACR_REPLACEMENT_MASK				( 0x3 << L2ACR_REPLACEMENT_OFFSET )
#define L2ACR_REPLACEMENT_TYPE_WAYRR		( 0 << L2ACR_REPLACEMENT_OFFSET )
#define L2ACR_REPLACEMENT_TYPE_LFSR			( 1 << L2ACR_REPLACEMENT_OFFSET )
#ifdef CONFIG_ARMADA_XP_REV_Z1
#define L2ACR_REPLACEMENT_TYPE_SEMIPLRU		( 2 << L2ACR_REPLACEMENT_OFFSET )
#else
#define L2ACR_REPLACEMENT_TYPE_SEMIPLRU		( 3 << L2ACR_REPLACEMENT_OFFSET )
#endif

/* L2_CNTR_CTRL_REG ( L2CCR ) */
#define L2CCR_CPU0_EVENT_ENABLE				( 1 << 16 )
#define L2CCR_CPU1_EVENT_ENABLE				( 1 << 17 )
#define L2CCR_FPU_EVENT_ENABLE				( 1 << 30 )
#define L2CCR_IO_BRIDGE_EVENT_ENABLE		( 1 << 31 )

#endif /* __INCmvCpuIfL2Regsh */

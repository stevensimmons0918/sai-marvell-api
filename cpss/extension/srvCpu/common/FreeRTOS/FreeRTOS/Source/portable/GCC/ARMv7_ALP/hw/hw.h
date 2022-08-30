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

#ifndef HW_H
#define HW_H

#include "common.h"
#include "ic.h"
#include "uart.h"
#include "cntmr.h"
#include "cache-aurora-l2.h"

/* GPIO control */
#define MPPREG_BASE				( 0x18000 )
#define MPPREG_CONTROL( i )		( MPPREG_BASE + ((( i ) & ~0x7 ) >> 1 ))
#define MPPBIT_PIN( i )			(( i ) % 8 )
#define MPPMASK_PIN( i )		( 0xf << MPPBIT_PIN( i ))
#define GPIOREG_BASE			( 0x18100 )
#define GPIOREG_DATA_OUT( i )	( GPIOREG_BASE + ((( i ) & ~0x1f ) << 1 ))
#define GPIOBIT_OUT( p )		( 1 << (( p ) % 32 ) )

/* Soft Reset control */
#define SYSREG_RSTOUT			( 0x18254 )
#define SYSREG_SOFT_RESET		( 0x18258 )
#define SYSBIT_RSTOUT_GLOBAL_EN	( 0 )
#define SYSBIT_SOFT_RESET_GLOB	( 0 )

/* IOCC */
#define SCU_CTRL						( 0x00 )
#define SCU_CONFIG						( 0x04 )
#define SCU_CPU_STATUS					( 0x08 )
#define SCU_INVALIDATE					( 0x0c )
#define MV_COHERENCY_FABRIC_CTRL_REG	( MV_COHERENCY_FABRIC_OFFSET + 0x0 )
#define MV_COHERENCY_FABRIC_CFG_REG		( MV_COHERENCY_FABRIC_OFFSET + 0x4 )
#define MV_CIB_CTRL_CFG_REG				( MV_COHERENCY_FABRIC_OFFSET + 0x80 )

/* CPU */
#define CPU_CONFIG_REG( cpu )			( MV_CPUIF_REGS_BASE( cpu ))
#define CPU_IO_SYNC_BARRIER_REG( cpu )	( MV_CPUIF_REGS_BASE( cpu ) + 0x10 )

/* IO Synchronization Barrier */
#if ENABLE_IOCC
#define mvOsCacheIoSync()	\
					{ \
						MV_REG_WRITE( CPU_IO_SYNC_BARRIER( CPUID ), 0x1 ); \
						while ( MV_REG_READ( CPU_IO_SYNC_BARRIER( CPUID )) & 0x1 ); \
					}
#endif

#endif

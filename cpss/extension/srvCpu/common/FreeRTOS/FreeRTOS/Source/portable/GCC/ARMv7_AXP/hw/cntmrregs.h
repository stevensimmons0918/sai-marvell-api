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

#ifndef __INCmvTmrwtdgRegsh
#define __INCmvTmrwtdgRegsh

#include "common.h"
#include "cntmr.h"

/*******************************************/
/* ARM Timers Registers Map                */
/*******************************************/
/*
	TIMER0  = Global counter 0
	TIMER1  = Global counter 1
	TIMER2  = Global counter 2
	TIMER3  = Global counter 3
	TIMER4  = Global Watchdog 0
	TIMER5  = CPU0 Timer 0
	TIMER6  = CPU0 Timer 1
	TIMER7  = CPU0 Watchdog
	TIMER8  = CPU1 Timer 0
	TIMER9  = CPU1 Timer 1
	TIMER10 = CPU1 Watchdog
	TIMER11 = CPU2 Timer 0
	TIMER12 = CPU2 Timer 1
	TIMER13 = CPU2 Watchdog
	TIMER14 = CPU73 Timer 0
	TIMER15 = CPU73 Timer 1
	TIMER16 = CPU73 Watchdog
*/
#define INVALID_CNTMR( cntmrNum )		(( cntmrNum ) >= MV_CNTMR_MAX_COUNTER )

#ifdef MV88F78X60_Z1
#define TIMER_TO_CPU( t )			(( t < TIMER8 ) ? 0 :					\
											(( t < TIMER11 ) ? 1 : 	\
											(( t < TIMER14 ) ? 2 : 3 )))
#define CPU_TIMER( t )			(( t < TIMER8 ) ? ( t-TIMER5 ) :					\
											(( t < TIMER11 ) ? ( t-TIMER8 ) :	\
											(( t < TIMER14 ) ? ( t-TIMER11 ) : ( t-TIMER14 ))))
#else
#define CPU_TIMER( t )			( t-TIMER5 )
#endif


#ifdef MV88F78X60_Z1
#define CNTMR_BASE( tmrNum )		(( tmrNum <= MAX_GLOBAL_TIMER ) ? ( MV_CNTMR_REGS_OFFSET ) : \
								( MV_CPUIF_REGS_OFFSET( TIMER_TO_CPU( tmrNum )) + 0x40 ))

#define CNTMR_RELOAD_REG( tmrNum )	 (( tmrNum <= MAX_GLOBAL_TIMER ) ? 				 \
										( CNTMR_BASE( tmrNum )  + 0x10 + ( tmrNum * 8 )) : \
										(( MV_CPUIF_REGS_OFFSET( TIMER_TO_CPU( tmrNum )) + 0x50+CPU_TIMER( tmrNum )*8 )))

#define CNTMR_VAL_REG( tmrNum )		(( tmrNum <= MAX_GLOBAL_TIMER ) ? 					 \
										( CNTMR_BASE( tmrNum )  + 0x14 + ( tmrNum * 8 )) : \
										(( MV_CPUIF_REGS_OFFSET( TIMER_TO_CPU( tmrNum )) + 0x54+CPU_TIMER( tmrNum )*8 )))


#else
#define CNTMR_BASE( tmrNum )		(( tmrNum <= MAX_GLOBAL_TIMER ) ? ( MV_CNTMR_REGS_OFFSET ) : \
																( MV_CPUIF_LOCAL_REGS_OFFSET + 0x40 ))

#define CNTMR_RELOAD_REG( tmrNum )	 (( tmrNum <= MAX_GLOBAL_TIMER ) ? 				 \
										( CNTMR_BASE( tmrNum )  + 0x10 + ( tmrNum * 8 )) : \
										( MV_CPUIF_LOCAL_REGS_OFFSET + 0x50 + (( tmrNum-5 ) * 8 )))

#define CNTMR_VAL_REG( tmrNum )		(( tmrNum <= MAX_GLOBAL_TIMER ) ? 					 \
										( CNTMR_BASE( tmrNum )  + 0x14 + ( tmrNum * 8 )) : \
										( MV_CPUIF_LOCAL_REGS_OFFSET + 0x54 + (( tmrNum-5 ) * 8 )))

#endif

/* #define CNTMR_CTRL_REG( tmrNum )	( tmrNum <=MAX_GLOBAL_TIMER ) ? ( MV_CNTMR_REGS_OFFSET ) :
						 ( MV_CPUIF_REGS_OFFSET( 0 ) + 0x84 ) */
#define CNTMR_CTRL_REG( tmrNum )		CNTMR_BASE( tmrNum )

/* ARM Timers Registers Map                */
/*******************************************/

/* ARM Timers Control Register */
/* CPU_TIMERS_CTRL_REG ( CTCR ) */

#define CTCR_ARM_TIMER_EN_OFFS( timer )	(( timer <= MAX_GLOBAL_TIMER ) ? ( timer * 2 ) : (( CPU_TIMER( timer ))*2 ))

#define CTCR_ARM_TIMER_EN_MASK( cntr )	( 1 << CTCR_ARM_TIMER_EN_OFFS( cntr ))
#define CTCR_ARM_TIMER_EN( cntr )		( 1 << CTCR_ARM_TIMER_EN_OFFS( cntr ))
#define CTCR_ARM_TIMER_DIS( cntr )	( 0 << CTCR_ARM_TIMER_EN_OFFS( cntr ))

#define CTCR_ARM_TIMER_AUTO_OFFS( timer )	(( timer <= MAX_GLOBAL_TIMER ) ? ( 1 + ( timer * 2 )) : \
										( 1 + (( CPU_TIMER( timer ))) * 2 ))
#define CTCR_ARM_TIMER_AUTO_MASK( cntr )	( 1 << CTCR_ARM_TIMER_EN_OFFS( cntr ))
#define CTCR_ARM_TIMER_AUTO_EN( cntr )	( 1 << CTCR_ARM_TIMER_AUTO_OFFS( cntr ))
#define CTCR_ARM_TIMER_AUTO_DIS( cntr )	( 0 << CTCR_ARM_TIMER_AUTO_OFFS( cntr ))


#define CTCR_ARM_TIMER_RATIO_OFFS( timer ) (( timer < TIMER4 ) ? ( 19 + timer*3 ) :	\
										 (( timer == TIMER4 ) ? 16 :				\
										 (( CPU_TIMER( timer ) == 0 ) ? 19 :      \
										 (( CPU_TIMER( timer ) == 1 ) ? 22 : 16 ))))

#define CTCR_ARM_TIMER_RATIO_MASK( cntr )	( 0x7 << CTCR_ARM_TIMER_RATIO_OFFS( cntr ))

#define CTCR_ARM_TIMER_25MhzFRQ_ENABLE_OFFS( timer ) (( timer < TIMER4 ) ? ( 11 + timer ) :		\
													 (( timer == TIMER4 ) ? 10 :				\
													 (( CPU_TIMER( timer ) == 0 ) ? 11 :	\
													 (( CPU_TIMER( timer ) == 1 ) ? 12 : 10 ))))

#define CTCR_ARM_TIMER_25MhzFRQ_MASK( cntr )	( 1 << CTCR_ARM_TIMER_25MhzFRQ_ENABLE_OFFS( cntr ))
#define CTCR_ARM_TIMER_25MhzFRQ_EN( cntr )	( 1 << CTCR_ARM_TIMER_25MhzFRQ_ENABLE_OFFS( cntr ))
#define CTCR_ARM_TIMER_25MhzFRQ_DIS( cntr )	( 0 << CTCR_ARM_TIMER_25MhzFRQ_ENABLE_OFFS( cntr ))


/* ARM Timer\Watchdog Reload Register */
/* CNTMR_RELOAD_REG (TRR) */

#define TRG_ARM_TIMER_REL_OFFS			0
#define TRG_ARM_TIMER_REL_MASK			0xffffffff

/* ARM Timer\Watchdog Register */
/* CNTMR_VAL_REG (TVRG) */

#define TVR_ARM_TIMER_OFFS			0
#define TVR_ARM_TIMER_MASK			0xffffffff
#define TVR_ARM_TIMER_MAX			0xffffffff

#endif				/* __INCmvTmrwtdgRegsh */

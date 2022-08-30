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

#ifndef __INCmvTmrWtdgh
#define __INCmvTmrWtdgh

/* includes */
#include "common.h"

/* 3 timers + 1 WD */
#define MV_CNTMR_MAX_COUNTER		4

/* This enumerator describe counters\watchdog numbers       */
	typedef enum _mvCntmrID {
		TIMER0 = 0,		/* Global counter 0 */
		TIMER1,			/* Global counter 1 */
		TIMER2,			/* Global counter 2 */
		TIMER3,			/* Global Watchdog 0 */
	} MV_CNTMR_ID;

/* Registers */
#define TMREG_CONTROL			( 0x18 )
#define TMREG_RELOAD( n )		( 0x20 + ( n ) * 4 )
#define TMREG_VALUE( n )		( 0x30 + ( n ) * 4 )

#define TMBIT_CTRL_EN( n )		( 1 << ( n ))
#define TMBIT_CTRL_AUTO( n )	( 1 << (( n ) + 4 ))
#define TMBIT_CTRL_WDRST		( 1 << 8 )

#define TIMER_ENABLE			( 1 )
#define TIMER_AUTO_EN			( 1 )
#define TIMER_WDTRST_EN			( 1 )
#define TIMER_DISABLE			( 0 )
#define TIMER_AUTO_DIS			( 0 )
#define TIMER_WDTRST_DIS		( 0 )

/* Functions */
/* Load an init Value to a given counter/timer */
	MV_STATUS mvCntmrLoad( MV_U32 countNum, MV_U32 value );

/* Returns the value of the given Counter/Timer */
	MV_U32 mvCntmrRead( MV_U32 countNum );

/* Write a value of the given Counter/Timer */
	void mvCntmrWrite( MV_U32 countNum, MV_U32 countVal );

/* Set the Enable-Bit to logic '1' ==> starting the counter. */
	MV_STATUS mvCntmrEnable( MV_U32 countNum );

/* Stop the counter/timer running, and returns its Value. */
	MV_STATUS mvCntmrDisable( MV_U32 countNum );

/* Combined all the sub-operations above to one function: Load,setMode,Enable */
	MV_STATUS mvCntmrSetup( MV_U32 countNum, MV_U32 value,
			MV_U32 enable, MV_U32 autoReload, MV_U32 wdtReset);

#endif				/* __INCmvTmrWtdgh */

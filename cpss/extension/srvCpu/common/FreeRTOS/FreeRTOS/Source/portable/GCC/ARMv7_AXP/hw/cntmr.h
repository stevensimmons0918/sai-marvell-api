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
#include "cntmrregs.h"

/* 4 global + 1 global WD + 2 private CPU + 1 private CPU WD*/
#define MV_CNTMR_MAX_COUNTER		8

/* This enumerator describe counters\watchdog numbers       */
	typedef enum _mvCntmrID {
		TIMER0 = 0,		/* Global counter 0 */
		TIMER1,			/* Global counter 1 */
		TIMER2,			/* Global counter 2 */
		TIMER3,			/* Global counter 3 */
		TIMER4,			/* Global Watchdog 0*/
		TIMER5,			/* CPU0 Timer 0   for A0 this is private CPU timer 0  */
		TIMER6, 		/* CPU0 Timer 1   for A0 this is private CPU timer 1   */
		TIMER7, 		/* CPU0 Watchdog  for A0 this is private CPU WD  */
#ifdef MV88F78X60_Z1
		TIMER8, 		/* CPU1 Timer 0     */
		TIMER9, 		/* CPU1 Timer 1     */
		TIMER10,		/* CPU1 Watchdog    */
		TIMER11,		/* CPU2 Timer 0     */
		TIMER12,		/* CPU2 Timer 1     */
		TIMER13,		/* CPU2 Watchdog    */
		TIMER14,		/* CPU73 Timer 0    */
		TIMER15,		/* CPU73 Timer 1    */
		TIMER16 		/* CPU73 Watchdog   */
#endif
	} MV_CNTMR_ID;

#define MAX_GLOBAL_TIMER	TIMER4
#define FIRST_PRIVATE_TIMER TIMER5

	typedef enum _mvCntmrRatio {
		MV_RATIO_1 = 0,		/*  0 = 1: Timer tic occurs every source clock        */
		MV_RATIO_2,			/*  1 = 2: Timer tic occurs every 2 source clocks     */
		MV_RATIO_4,			/*  2 = 4: Timer tic occurs every 4 source clocks     */
		MV_RATIO_8,			/*  3 = 8: Timer tic occurs every 8 source clocks     */
		MV_RATIO_16,		/*  4 = 16: Timer tic occurs every 16 source clocks   */
		MV_RATIO_32,		/*  5 = 32: Timer tic occurs every 32 source clocks   */
		MV_RATIO_64,		/*  6 = 64: Timer tic occurs every 64 source clocks   */
		MV_RATIO_128		/*  7 = 128: Timer tic occurs every 128 source clocks */
	} MV_CNTMR_RATIO_ID;

/* Counter / Timer control structure */
	typedef struct _mvCntmrCtrl {
		MV_BOOL enable;	/* enable */
		MV_BOOL autoEnable;	/* counter/Timer  */
#ifndef MV88F78X60_Z1
		MV_CNTMR_RATIO_ID	Ratio;
		MV_BOOL enable_25Mhz;	/* enable timer count frequency is to 25Mhz*/
#endif
	} MV_CNTMR_CTRL;

/* Functions */

/* Load an init Value to a given counter/timer */
	MV_STATUS mvCntmrLoad( MV_U32 countNum, MV_U32 value );

/* Returns the value of the given Counter/Timer */
	MV_U32 mvCntmrRead( MV_U32 countNum );

/* Write a value of the given Counter/Timer */
	void mvCntmrWrite( MV_U32 countNum, MV_U32 countVal );

/* Set the Control to a given counter/timer */
	MV_STATUS mvCntmrCtrlSet( MV_U32 countNum, MV_CNTMR_CTRL *pCtrl );

/* Get the value of a given counter/timer */
	MV_STATUS mvCntmrCtrlGet( MV_U32 countNum, MV_CNTMR_CTRL *pCtrl );

/* Set the Enable-Bit to logic '1' ==> starting the counter. */
	MV_STATUS mvCntmrEnable( MV_U32 countNum );

/* Stop the counter/timer running, and returns its Value. */
	MV_STATUS mvCntmrDisable( MV_U32 countNum );

/* Combined all the sub-operations above to one function: Load,setMode,Enable */
	MV_STATUS mvCntmrStart( MV_U32 countNum, MV_U32 value, MV_CNTMR_CTRL *pCtrl );

/*	Clear an Counter/Timer interrupt ( Ack ) */
	MV_STATUS mvCntmrIntClear( MV_U32 cntmrNum );


#endif				/* __INCmvTmrWtdgh */

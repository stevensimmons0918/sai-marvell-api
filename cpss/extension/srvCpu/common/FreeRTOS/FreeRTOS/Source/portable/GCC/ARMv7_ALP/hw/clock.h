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

#ifndef __MV_CLOCKS__
#define __MV_CLOCKS__

#include "common.h"

/* Clock related definitions */
#define MV_SAR_FREQ_MODES { \
	{ 0,  266,  266, 133 }, \
	{ 1,  333,  167, 167 }, \
	{ 2,  333,  222, 167 }, \
	{ 3,  333,  333, 167 }, \
	{ 4,  400,  200, 200 }, \
	{ 5,  400,  267, 200 }, \
	{ 6,  400,  400, 200 }, \
	{ 7,  500,  250, 250 }, \
	{ 8,  500,  334, 250 }, \
	{ 9,  500,  400, 250 }, \
	{ 10, 533,  267, 267 }, \
	{ 11, 533,  356, 267 }, \
	{ 12, 533,  533, 267 }, \
	{ 13, 600,  300, 300 }, \
	{ 14, 600,  400, 300 }, \
	{ 15, 600,  600, 300 }, \
	{ 16, 666,  333, 333 }, \
	{ 17, 666,  444, 333 }, \
	{ 18, 666,  666, 333 }, \
	{ 19, 800,  267, 400 }, \
	{ 20, 800,  400, 400 }, \
	{ 21, 800,  534, 400 }, \
	{ 22, 900,  300, 450 }, \
	{ 23, 900,  450, 450 }, \
	{ 24, 900,  600, 450 }, \
	{ 25, 1000, 500, 500 }, \
	{ 26, 1000, 667, 500 }, \
	{ 27, 1000, 500, 333 }, \
	{ 28, 400,  400, 400 }, \
	{ 29, 1100, 550, 550 } \
};

#define MV_SAR_TCLK_166MHZ	( 166666667 )
#define MV_SAR_TCLK_200MHZ	( 200000000 )

/* Default required for short delays in low-level code */
#define MV_CPU_CLK_DEFAULT	266000000

typedef struct {
	MV_U8 id;
	MV_U32 cpuFreq;
	MV_U32 ddrFreq;
	MV_U32 l2Freq;
} MV_FREQ_MODE;

void vGetBoardClocks( void );

#endif		/* __MV_CLOCKS__ */

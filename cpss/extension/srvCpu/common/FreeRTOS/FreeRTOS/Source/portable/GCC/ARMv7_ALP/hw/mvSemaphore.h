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
#ifndef mvsemaphore_h
#define mvsemaphore_h

#include <common.h>

#define MV_AMP_GLOBAL_REG( x )	( 0x20980 + ( 4 * ( x )))
#define ADR_WIN_EN_REG			0

#define MV_SEMA_REG_BASE		( 0x20500 )
#define MV_MAX_SEMA				128
#define MV_SEMA_SMI				50
#define MV_SEMA_RTC				51
#define MV_SEMA_NOR_FLASH		0
#define MV_SEMA_BOOT			1
#define MV_SEMA_PEX0			2
#define MV_SEMA_BRIDGE			3
#define MV_SEMA_IRQ				4
#define MV_SEMA_CLOCK			5
#define MV_SEMA_L2				6
#define MV_SEMA_TWSI			7
#define MV_SEMA_ADR_WIN			8
#define MV_SEMA_UART			9

#define MV_SEMA_BARRIER( cpu )	( 50 + cpu )

/* Turn on HW semapores only if AMP is enabled */
/* Disable semaphores on Avanta LP due to hardware bug:
 * Accessing part of registers always returns 0.
 */
#if 0
MV_BOOL mvSemaLock( MV_32 num );
MV_BOOL mvSemaTryLock( MV_32 num );
MV_BOOL mvSemaUnlock( MV_32 num );
MV_32   mvReadAmpReg( long regId );
MV_32 	mvWriteAmpReg( long regId, MV_32 value );
#else
#define mvSemaLock( num )
#define mvSemaTryLock( num )
#define mvSemaUnlock( num )
#define mvReadAmpReg( reg )
#define mvWriteAmpReg( reg, value )
#endif /* CONFIG_MV_AMP_ENABLE */

#endif /* mvsemaphore_h */

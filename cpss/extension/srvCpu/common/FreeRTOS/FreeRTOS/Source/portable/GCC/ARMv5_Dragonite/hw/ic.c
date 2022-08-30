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

#include "FreeRTOS.h"
#include "hw.h"
#include "ic.h"
#include <portmacro.h>
#include <printf.h>

irqvector ICVectors[ ICMAX_INT ];

static inline unsigned long uICRegRead( long reg )
{
	return MV_REG_READ( reg );
}

static inline void vICRegWrite( long reg, unsigned long value )
{
	MV_REG_WRITE( reg, value );
}

long iICRegisterHandler( long irq, irqhandler func, void *UserData,
						long enable, long config )
{
	long ret;

	if (( irq < 0 ) || ( irq >= ICMAX_INT ))
		return -1;
	if ( func == NULL )
		return -2;

	ret = iICDisableInt( irq );

	ICVectors[ irq ].Handler = func;
	ICVectors[ irq ].UserData = UserData;

	if ( enable )
		ret = iICEnableInt( irq );

	return ret;
}

inline void iICSetupInt( long irq, long secure, long priority,
						 long edge, long enable )
{
	if (( irq < 0 ) || ( irq >= ICMAX_INT ))
		return;

	if ( enable )
		iICEnableInt( irq );
	else
		iICDisableInt( irq );
}

long iICUnregisterHandler( long irq )
{
	long ret;

	ret = iICDisableInt( irq );
	ICVectors[ irq ].Handler = NULL;

	return ret;
}

long iICEnableInt( long irq )
{
	long reg;

	if (( irq < 0 ) || ( irq >= ICMAX_INT ))
		return -1;
	if ( ICVectors[ irq ].Handler == NULL )
		return -2;

	if ( irq < ICMAX_IRQ ) {
		reg = uICRegRead( ICREG_IRQ_MASK );
		vICRegWrite( ICREG_IRQ_MASK, reg | ( 1 << ( irq + 1 )));
	} else {
		reg = uICRegRead( ICREG_FIQ_MASK );
		vICRegWrite( ICREG_FIQ_MASK, reg | ( 1 << ( irq - ICMAX_IRQ + 1 )));
	}

	return 0;
}

long iICDisableInt( long irq )
{
	long reg;

	if (( irq < 0 ) || ( irq >= ICMAX_INT ))
		return -1;

	if ( irq < ICMAX_IRQ ) {
		reg = uICRegRead( ICREG_IRQ_MASK );
		vICRegWrite( ICREG_IRQ_MASK, reg & ~( 1 << ( irq + 1 )));
	} else {
		reg = uICRegRead( ICREG_FIQ_MASK );
		vICRegWrite( ICREG_FIQ_MASK, reg & ~( 1 << ( irq - ICMAX_IRQ + 1 )));
	}

	return 0;
}

long iICAckInt( long irq )
{
	long reg;

	if (( irq < 0 ) || ( irq >= ICMAX_INT ))
		return -1;

	if ( irq < ICMAX_IRQ ) {
		reg = uICRegRead( ICREG_IRQ_CAUSE );
		vICRegWrite( ICREG_IRQ_CAUSE, reg & ~( 1 << ( irq + 1 )));
	} else {
		reg = uICRegRead( ICREG_FIQ_CAUSE );
		vICRegWrite( ICREG_FIQ_CAUSE, reg & ~( 1 << ( irq - ICMAX_IRQ + 1 )));
	}

	return 0;
}

void vICDispatcher( void )
{
	long reg, irq, irq_max;
	long ret, fiqm;

	fiqm = cpu_read_cpsr();
	if (( fiqm & CPSR_MODE_MASK ) == CPSR_MODE_FIQ )
		fiqm = 1;
	else
		fiqm = 0;

	if ( fiqm ) {
		reg = uICRegRead( ICREG_FIQ_CAUSE ) & uICRegRead( ICREG_FIQ_MASK );
		irq = ICMAX_IRQ;
		irq_max = ICMAX_INT;
	} else {
		reg = uICRegRead( ICREG_IRQ_CAUSE ) & uICRegRead( ICREG_IRQ_MASK );
		irq = 0;
		irq_max = ICMAX_IRQ;
	}
	if ( reg == 0 )
		return;

	for ( reg >>= 1; irq < irq_max; reg >>= 1, irq++ ) {
		if ( ( reg & 1 ) == 0 )
			continue;

		ret = -1;
		if ( ICVectors[ irq ].Handler != NULL )
			ret = ICVectors[ irq ].Handler( ICVectors[ irq ].UserData );

		if ( ret < 0 )
			iICDisableInt( irq );

		iICAckInt( irq );
	}
}

void vICInit( void )
{
	/* Disable interrupts */
	vICRegWrite( ICREG_IRQ_MASK, 0x0 );
	vICRegWrite( ICREG_FIQ_MASK, 0x0 );
}


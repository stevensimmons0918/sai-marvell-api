/*-
 * Copyright (c) 2011 The FreeBSD Foundation
 * All rights reserved.
 *
 * Developed by Damjan Marion <damjan.marion@gmail.com>
 *
 * Based on OMAP4 GIC code by Ben Gray
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the company nor the name of the author may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "hw.h"
#include "gic.h"

long arm_gic_init( void )
{
	unsigned long	i;
	unsigned long	nirqs, temp;

	/* Disable interrupt forwarding to the CPU interface */
	gic_d_write_4( GICD_CTLR, 0x00 );

	/* Get the number of interrupts */
	nirqs = gic_d_read_4( GICD_TYPER );
	nirqs = 32 * (( nirqs & 0x1f ) + 1 );

	/* Configure SGI/PPI */
	gic_d_write_4( GICD_IGROUPR( 0 ), 0xffff0000 );
	for ( i = 0; i < ICMAX_SGI + ICMAX_PPI; i += 4 )
		gic_d_write_4( GICD_IPRIORITYR( i ),  0xffffffff );
	gic_d_write_4( GICD_ICFGR( 1 ), 0x00000000 );
	gic_d_write_4( GICD_ICENABLER( 0 ), 0xffffffff );

	/* Mask SPI on current CPU */
	for ( i = ICMAX_SGI + ICMAX_PPI; i < nirqs; i += 4 ) {
		temp = gic_d_read_4( GICD_ITARGETSR( i ));
		temp &= ~( 0x01010101 << CPUID );
		gic_d_write_4( GICD_ITARGETSR( i ), temp );
	}

	/* Enable CPU interface for Secure and Non-secure interrupts.
	 * Use FIQ line to signal secure interrupts.
	 * Do not handle Non-secure interrupts on FIQ.
	 */
	gic_c_write_4( GICC_CTLR, GICC_CTLR_FIQEN | GICC_CTLR_ACKCTL |
			GICC_CTLR_ENNSEC | GICC_CTLR_ENSEC );

	/* Set priority mask register. */
	gic_c_write_4( GICC_PMR, 0xff );

	/* Enable Secure [ 0 ] and Non-secure [ 1 ] interrupts distribution */
	gic_d_write_4( GICD_CTLR, 0x03 );

	return ( 0 );
}

void arm_post_filter( unsigned long irq )
{

	gic_c_write_4( GICC_EOIR, irq );
}

long arm_get_next_irq( void )
{

	return gic_c_read_4( GICC_IAR );
}

void arm_mask_irq( unsigned long irq )
{
	unsigned char temp;

	if ( irq < ICMAX_SGI + ICMAX_PPI ) {
		gic_d_write_4( GICD_ICENABLER( irq >> 5 ), ( 1UL << ( irq & 0x1F )));
	} else {
		temp = gic_d_read_1( GICD_ITARGETSR( irq ));
		temp &= ~( 0x01 << CPUID );
		gic_d_write_1( GICD_ITARGETSR( irq ), temp );
	}
}

void arm_unmask_irq( unsigned long irq )
{
	unsigned char temp;

	if ( irq >= ICMAX_SGI + ICMAX_PPI ) {
		temp = gic_d_read_1( GICD_ITARGETSR( irq ));
		temp |= ( 0x01 << CPUID );
		gic_d_write_1( GICD_ITARGETSR( irq ), temp );
	}

	gic_d_write_4( GICD_ISENABLER( irq >> 5 ), ( 1UL << ( irq & 0x1F )));
}

void arm_setup_irq( unsigned long irq, long secure, long priority,
		long edge, long enable )
{
	unsigned long reg, mask;

	if ( irq >= ICMAX_GIC_IRQ )
		return;

	/* Set secure */
	if ( secure >= 0 ) {
		reg = gic_d_read_4( GICD_IGROUPR( irq >> 5 ));
		mask = 1 << ( irq & 0x1F );
		/* Force all SGI interrupts to be secure.
		 * It is a workaround for issue observed on Avanta LP.
		 * Non-secure SGI was working only if sent from CPU0 to CPU1.
		 */
		if ( secure || ( irq < ICMAX_SGI ))
			reg &= ~mask;
		else
			reg |= mask;
		gic_d_write_4( GICD_IGROUPR( irq >> 5 ), reg );
	}

	/* Set priority */
	if ( priority >= 0 ) {
		reg = gic_d_read_4( GICD_IGROUPR( irq >> 5 ));
		mask = 1 << ( irq & 0x1F );
		if ( reg & mask )
			/* Interrupt is Non-secure. Force lower priority. */
			priority += IRQ_PRIO_HIGHEST_NONSECURE;
		/* Values higher than 254 disable the interrupt. */
		if ( priority >= 0xff )
			priority = 0xff;

		gic_d_write_1( GICD_IPRIORITYR( irq ), priority );
	}

	/* Set edge/level */
	if (( edge >= 0 ) && ( irq >= ICMAX_SGI )) {
		reg = gic_d_read_4( GICD_ICFGR( irq >> 4 ));
		mask = 1 << ( 2 * ( irq & 0x0F ) + 1 );
		if ( edge )
			reg |= mask;
		else
			reg &= ~mask;
		gic_d_write_4( GICD_ICFGR( irq >> 4 ), reg );
	}

	/* Set enable */
	if ( enable >= 0 ) {
		if ( enable )
			arm_unmask_irq( irq );
		else
			arm_mask_irq( irq );
	}
}

void arm_send_sgi( unsigned long filter, unsigned long target, unsigned long sgiid )
{

	/* Ensure that all stores to memory are finished before issuing the SGI */
	dsb();
	gic_d_write_4( GICD_SGIR, (( filter & 0x3 ) << 24 ) |
			(( target & 0xFF ) << 16 ) | ( sgiid & 0xF ));
}


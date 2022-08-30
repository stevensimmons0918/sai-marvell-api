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

/* Register base */
#define MV_CA9X2_MPIC		( 0xC000 )
#define A9_MPCORE_SCU		( MV_CA9X2_MPIC + 0x0000 )
#define A9_MPCORE_GIC_CPU	( MV_CA9X2_MPIC + 0x0100 )
#define A9_MPCORE_GIT		( MV_CA9X2_MPIC + 0x0200 )
#define A9_MPCORE_TWD		( MV_CA9X2_MPIC + 0x0600 )
#define A9_MPCORE_GIC_DIST	( MV_CA9X2_MPIC + 0x1000 )

/* We are using GICv2 register naming */
/* Distributor Registers */
#define GICD_CTLR			0x000						/* v1 ICDDCR */
#define GICD_TYPER			0x004						/* v1 ICDICTR */
#define GICD_IIDR			0x008						/* v1 ICDIIDR */
#define GICD_IGROUPR( n )	( 0x0080 + (( n ) * 4 ))	/* v1 ICDISER */
#define GICD_ISENABLER( n )	( 0x0100 + (( n ) * 4 ))	/* v1 ICDISER */
#define GICD_ICENABLER( n )	( 0x0180 + (( n ) * 4 ))	/* v1 ICDICER */
#define GICD_ISPENDR( n )	( 0x0200 + (( n ) * 4 ))	/* v1 ICDISPR */
#define GICD_ICPENDR( n )	( 0x0280 + (( n ) * 4 ))	/* v1 ICDICPR */
#define GICD_ICACTIVER( n )	( 0x0380 + (( n ) * 4 ))	/* v1 ICDABR */
#define GICD_IPRIORITYR( n ) ( 0x0400 + ( n ))			/* v1 ICDIPR */
#define GICD_ITARGETSR( n )	( 0x0800 + ( n ))			/* v1 ICDIPTR */
#define GICD_ICFGR( n )		( 0x0C00 + (( n ) * 4 ))	/* v1 ICDICFR */
#define GICD_SGIR			( 0x0F00 )					/* v1 ICDSGIR */

/* CPU Registers */
#define GICC_CTLR		0x0000			/* v1 ICCICR */
#define GICC_PMR		0x0004			/* v1 ICCPMR */
#define GICC_BPR		0x0008			/* v1 ICCBPR */
#define GICC_IAR		0x000C			/* v1 ICCIAR */
#define GICC_EOIR		0x0010			/* v1 ICCEOIR */
#define GICC_RPR		0x0014			/* v1 ICCRPR */
#define GICC_HPPIR		0x0018			/* v1 ICCHPIR */
#define GICC_ABPR		0x001C			/* v1 ICCABPR */
#define GICC_IIDR		0x00FC			/* v1 ICCIIDR*/

#define GICC_CTLR_SBPR		( 1 << 4 )
#define GICC_CTLR_FIQEN		( 1 << 3 )
#define GICC_CTLR_ACKCTL	( 1 << 2 )
#define GICC_CTLR_ENNSEC	( 1 << 1 )
#define GICC_CTLR_ENSEC		( 1 << 0 )

/* Register access functions */
#define	gic_c_read_1( reg )			MV_REG_BYTE_READ( A9_MPCORE_GIC_CPU + reg )
#define	gic_c_write_1( reg, val )	MV_REG_BYTE_WRITE( A9_MPCORE_GIC_CPU + reg, val )
#define	gic_d_read_1( reg )			MV_REG_BYTE_READ( A9_MPCORE_GIC_DIST + reg )
#define	gic_d_write_1( reg, val )	MV_REG_BYTE_WRITE( A9_MPCORE_GIC_DIST + reg, val )
#define	gic_c_read_4( reg )			MV_REG_READ( A9_MPCORE_GIC_CPU + reg )
#define	gic_c_write_4( reg, val )	MV_REG_WRITE( A9_MPCORE_GIC_CPU + reg, val )
#define	gic_d_read_4( reg )			MV_REG_READ( A9_MPCORE_GIC_DIST + reg )
#define	gic_d_write_4( reg, val )	MV_REG_WRITE( A9_MPCORE_GIC_DIST + reg, val )

/* API */
#define arm_gic_read_reg( reg )			MV_REG_READ( reg )
#define arm_gic_write_reg( reg, val )	MV_REG_WRITE( reg, val )
long arm_gic_init( void );
void arm_post_filter( unsigned long irq );
long arm_get_next_irq( void );
void arm_mask_irq( unsigned long irq );
void arm_unmask_irq( unsigned long irq );
void arm_setup_irq( unsigned long irq, long secure, long priority, long edge, long enable );
void arm_send_sgi( unsigned long filter, unsigned long target, unsigned long sgiid );


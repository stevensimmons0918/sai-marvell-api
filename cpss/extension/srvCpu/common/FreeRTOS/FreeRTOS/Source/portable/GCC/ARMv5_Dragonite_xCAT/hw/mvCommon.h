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

#ifndef __INCmvCommonh
#define __INCmvCommonh

/* Swap tool */

/* 16bit nibble swap. For example 0x1234 -> 0x2143						*/
#define MV_NIBBLE_SWAP_16BIT( X )	(((( X ) & 0xf ) << 4 ) |	\
									((( X ) & 0xf0 ) >> 4 ) |	\
									((( X ) & 0xf00 ) << 4 ) |	\
									((( X ) & 0xf000 ) >> 4 ))

/* 32bit nibble swap. For example 0x12345678 -> 0x21436587				*/
#define MV_NIBBLE_SWAP_32BIT( X )	(((( X ) & 0xf ) << 4 ) |		\
									((( X ) & 0xf0 ) >> 4 ) |		\
									((( X ) & 0xf00 ) << 4 ) |		\
									((( X ) & 0xf000 ) >> 4 ) |		\
									((( X ) & 0xf0000 ) << 4 ) |	\
									((( X ) & 0xf00000 ) >> 4 ) |	\
									((( X ) & 0xf000000 ) << 4 ) |	\
									((( X ) & 0xf0000000 ) >> 4 ))

/* 16bit byte swap. For example 0x1122 -> 0x2211							*/
#define MV_BYTE_SWAP_16BIT( X ) (((( X ) & 0xff ) << 8 ) | ((( X ) & 0xff00 ) >> 8 ))

/* 32bit byte swap. For example 0x11223344 -> 0x44332211					*/
#define MV_BYTE_SWAP_32BIT( X ) (((( X ) & 0xff ) << 24 ) |						\
								((( X ) & 0xff00 ) << 8 ) |						\
								((( X ) & 0xff0000 ) >> 8 ) |					\
								((( X ) & 0xff000000 ) >> 24 ))

/* 64bit byte swap. For example 0x11223344.55667788 -> 0x88776655.44332211	*/
#define MV_BYTE_SWAP_64BIT( X ) (( l64 ) (((( X ) & 0xffULL ) << 56 ) |			\
										((( X ) & 0xff00ULL ) << 40 ) |			\
										((( X ) & 0xff0000ULL ) <<24 ) |		\
										((( X ) & 0xff000000ULL ) <<8 ) |		\
										((( X ) & 0xff00000000ULL ) >> 8 ) |	\
										((( X ) & 0xff0000000000ULL ) >> 24 ) | \
										((( X ) & 0xff000000000000ULL ) >> 40 ) | \
										((( X ) & 0xff00000000000000ULL ) >> 56 )))

/* Endianess macros.														*/
#if defined( MV_CPU_LE )
	#define MV_16BIT_LE( X )	( X )
	#define MV_32BIT_LE( X )	( X )
	#define MV_64BIT_LE( X )	( X )
	#define MV_16BIT_BE( X )	MV_BYTE_SWAP_16BIT( X )
	#define MV_32BIT_BE( X )	MV_BYTE_SWAP_32BIT( X )
	#define MV_64BIT_BE( X )	MV_BYTE_SWAP_64BIT( X )
#elif defined( MV_CPU_BE )
	#define MV_16BIT_LE( X )	MV_BYTE_SWAP_16BIT( X )
	#define MV_32BIT_LE( X )	MV_BYTE_SWAP_32BIT( X )
	#define MV_64BIT_LE( X )	MV_BYTE_SWAP_64BIT( X )
	#define MV_16BIT_BE( X )	( X )
	#define MV_32BIT_BE( X )	( X )
	#define MV_64BIT_BE( X )	( X )
#else
	#error "CPU endianess isn't defined!\n"
#endif


/* Bit field definitions */
#define NO_BIT		0x00000000
#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10		0x00000400
#define BIT11		0x00000800
#define BIT12		0x00001000
#define BIT13		0x00002000
#define BIT14		0x00004000
#define BIT15		0x00008000
#define BIT16		0x00010000
#define BIT17		0x00020000
#define BIT18		0x00040000
#define BIT19		0x00080000
#define BIT20		0x00100000
#define BIT21		0x00200000
#define BIT22		0x00400000
#define BIT23		0x00800000
#define BIT24		0x01000000
#define BIT25		0x02000000
#define BIT26		0x04000000
#define BIT27		0x08000000
#define BIT28		0x10000000
#define BIT29		0x20000000
#define BIT30		0x40000000
#define BIT31		0x80000000

/* Handy sizes */
#define _1K			0x00000400
#define _2K			0x00000800
#define _4K			0x00001000
#define _8K			0x00002000
#define _16K		0x00004000
#define _32K		0x00008000
#define _64K		0x00010000
#define _128K		0x00020000
#define _256K		0x00040000
#define _512K		0x00080000

#define _1M			0x00100000
#define _2M			0x00200000
#define _4M			0x00400000
#define _8M			0x00800000
#define _16M		0x01000000
#define _32M		0x02000000
#define _64M		0x04000000
#define _128M		0x08000000
#define _256M		0x10000000
#define _512M		0x20000000

#define _1G			0x40000000
#define _2G			0x80000000

/* Tclock and Sys clock define */
#define _100MHz		100000000
#define _125MHz		125000000
#define _133MHz		133333334
#define _150MHz		150000000
#define _160MHz		160000000
#define _166MHz		166666667
#define _175MHz		175000000
#define _178MHz		178000000
#define _183MHz		183333334
#define _187MHz		187000000
#define _192MHz		192000000
#define _194MHz		194000000
#define _200MHz		200000000
#define _233MHz		233333334
#define _250MHz		250000000
#define _266MHz		266666667
#define _300MHz		300000000

/* For better address window table readability */
#define EN			MV_TRUE
#define DIS			MV_FALSE
#define N_A			-1			/* Not applicable */

/* Cache configuration options for memory ( DRAM, SRAM, ... ) */

/* Memory uncached, HW or SW cache coherency is not needed */
#define MV_UNCACHED				0
/* Memory cached, HW cache coherency supported in WriteThrough mode */
#define MV_CACHE_COHER_HW_WT	1
/* Memory cached, HW cache coherency supported in WriteBack mode */
#define MV_CACHE_COHER_HW_WB	2
/* Memory cached, No HW cache coherency, Cache coherency must be in SW */
#define MV_CACHE_COHER_SW		3


/* Macros for testing aligment. */
#define MV_IS_NOT_ALIGN( number, align )	((( MV_U32 )number ) & (( align ) - 1 ))
#define MV_IS_ALIGN( number, align )		( MV_IS_NOT_ALIGN( number, align ) == 0 )

/* Macro for alignment up. For example, MV_ALIGN_UP( 0x0330, 0x20 ) = 0x0340	*/
#define MV_ALIGN_UP( number, align )											\
((( number ) & (( align ) - 1 )) ? ((( number ) + ( align )) & ~(( align )-1 )) : ( number ))

/* Macro for alignment down. For example, MV_ALIGN_UP( 0x0330, 0x20 ) = 0x0320 */
#define MV_ALIGN_DOWN( number, align )		(( number ) & ~(( align ) - 1 ))

/* This macro returns absolute value										*/
#define MV_ABS( number )	((( MV_32 )( number ) < 0 ) ? -( MV_32 )( number ) : ( MV_32 )( number ))


/* Bit fields manipulation macros											*/

/* An integer word which its 'x' bit is set									*/
#define MV_BIT_MASK( bitNum )		 ( 1 << ( bitNum ) )

/* Checks wheter bit 'x' in integer word is set								*/
#define MV_BIT_CHECK( word, bitNum )	( ( word ) & MV_BIT_MASK( bitNum ) )

/* Clear ( reset ) bit 'x' in integer word ( RMW - Read-Modify-Write )		*/
#define MV_BIT_CLEAR( word, bitNum )	( ( word ) &= ~( MV_BIT_MASK( bitNum )) )

/* Set bit 'x' in integer word ( RMW )										*/
#define MV_BIT_SET( word, bitNum )	( ( word ) |= MV_BIT_MASK( bitNum ) )

/* Invert bit 'x' in integer word ( RMW )									*/
#define MV_BIT_INV( word, bitNum )	( ( word ) ^= MV_BIT_MASK( bitNum ) )

/* Get the min between 'a' or 'b'											*/
#define MV_MIN( a, b )	((( a ) < ( b )) ? ( a ) : ( b ))

/* Get the max between 'a' or 'b'											*/
#define MV_MAX( a, b )	((( a ) < ( b )) ? ( b ) : ( a ))

#endif	/* __INCmvCommonh */

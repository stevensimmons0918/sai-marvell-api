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

#ifndef __INCmvSpiSpecH
#define __INCmvSpiSpecH

/* Constants */
#define		MV_SPI_WAIT_RDY_MAX_LOOP			100000
#define		MV_SPI_16_BIT_CHUNK_SIZE			2
#define		MV_SPI_DUMMY_WRITE_16BITS			0xFFFF
#define		MV_SPI_DUMMY_WRITE_8BITS			0xFF

/* Marvell Flash Device Controller Registers */
#define		MV_SPI_CTRLR_OFST				0x600	/*0x10600 KW*/
#define		MV_SPI_IF_CTRL_REG				( MV_SPI_CTRLR_OFST + 0x00 )
#define		MV_SPI_IF_CONFIG_REG				( MV_SPI_CTRLR_OFST + 0x04 )
#define		MV_SPI_DATA_OUT_REG				( MV_SPI_CTRLR_OFST + 0x08 )
#define		MV_SPI_DATA_IN_REG				( MV_SPI_CTRLR_OFST + 0x0c )
#define		MV_SPI_INT_CAUSE_REG				( MV_SPI_CTRLR_OFST + 0x10 )
#define		MV_SPI_INT_CAUSE_MASK_REG			( MV_SPI_CTRLR_OFST + 0x14 )

/* Serial Memory Interface Control Register Masks */
#define		MV_SPI_CS_ENABLE_OFFSET				0		/* bit 0 */
#define		MV_SPI_MEMORY_READY_OFFSET			1		/* bit 1 */
#define		MV_SPI_CS_ENABLE_MASK				( 0x1  << MV_SPI_CS_ENABLE_OFFSET )
#define		MV_SPI_MEMORY_READY_MASK			( 0x1  << MV_SPI_MEMORY_READY_OFFSET )

/* Serial Memory Interface Configuration Register Masks */
#define		MV_SPI_CLK_PRESCALE_OFFSET			0		/* bit 0-4 */
#define		MV_SPI_BYTE_LENGTH_OFFSET			5		/* bit 5 */
#define		MV_SPI_ADDRESS_BURST_LENGTH_OFFSET	8		/* bit 8-9 */
#define		MV_SPI_CLK_PRESCALE_MASK			( 0x1F << MV_SPI_CLK_PRESCALE_OFFSET )
#define		MV_SPI_BYTE_LENGTH_MASK				( 0x1  << MV_SPI_BYTE_LENGTH_OFFSET )
#define		MV_SPI_ADDRESS_BURST_LENGTH_MASK	( 0x3  << MV_SPI_ADDRESS_BURST_LENGTH_OFFSET )

#endif /* __INCmvSpiSpecH */


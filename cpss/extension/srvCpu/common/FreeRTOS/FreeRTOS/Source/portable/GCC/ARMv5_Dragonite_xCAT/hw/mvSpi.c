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

#include "mvSpi.h"
#include "mvSpiSpec.h"
#include "common.h"

#include <printf.h>

/*#define MV_DEBUG*/
#ifdef MV_DEBUG
#define DB( x ) x
#else
#define DB( x )
#endif


/*******************************************************************************
* mvSpi16bitDataTxRx - Transmt and receive data
*
* DESCRIPTION:
*		Tx data and block waiting for data to be transmitted
*
********************************************************************************/
MV_STATUS mvSpi16bitDataTxRx ( MV_U16 txData, MV_U16 * pRxData )
{
	MV_U32 i;
	MV_BOOL ready = MV_FALSE;

	/* First clear the bit in the interrupt cause register */
	MV_REG_WRITE( MV_SPI_INT_CAUSE_REG, 0x0 );

	/* Transmit data */
	MV_REG_WRITE( MV_SPI_DATA_OUT_REG, MV_16BIT_LE( txData ));

	/* wait with timeout for memory ready */
	for ( i = 0; i < MV_SPI_WAIT_RDY_MAX_LOOP; i++ )
	{
		if ( MV_REG_READ( MV_SPI_INT_CAUSE_REG ))
		{
			ready = MV_TRUE;
			break;
		}
#ifdef MV_SPI_SLEEP_ON_WAIT
		mvOsSleep( 1 );
#endif /* MV_SPI_SLEEP_ON_WAIT */
	}

	if ( !ready )
		return MV_TIMEOUT;

	/* check that the RX data is needed */
	if ( pRxData )
	{
		if (( MV_U32 )pRxData & 0x1 ) /* check if address is not alligned to 16bit */
		{
#if defined( MV_CPU_LE )
			/* perform the data write to the buffer in two stages with 8bit each */
			MV_U8 * bptr = ( MV_U8 * )pRxData;
			MV_U16 data = MV_16BIT_LE( MV_REG_READ( MV_SPI_DATA_IN_REG ));
			*bptr = ( data & 0xFF );
			++bptr;
			*bptr = (( data >> 8 ) & 0xFF );

#elif defined( MV_CPU_BE )

			/* perform the data write to the buffer in two stages with 8bit each */
			MV_U8 * bptr = ( MV_U8 * )pRxData;
			MV_U16 data = MV_16BIT_LE( MV_REG_READ( MV_SPI_DATA_IN_REG ));
			*bptr = (( data >> 8 ) & 0xFF );
			++bptr;
			*bptr = ( data & 0xFF );

#else
	#error "CPU endianess isn't defined!\n"
#endif

		}
		else
			*pRxData = MV_16BIT_LE( MV_REG_READ( MV_SPI_DATA_IN_REG ));
	}

	return MV_OK;
}


/*******************************************************************************
* mvSpi8bitDataTxRx - Transmt and receive data ( 8bits )
*
* DESCRIPTION:
*		Tx data and block waiting for data to be transmitted
*
********************************************************************************/
static MV_STATUS mvSpi8bitDataTxRx ( MV_U8 txData, MV_U8 * pRxData )
{
	MV_U32 i;
	MV_BOOL ready = MV_FALSE;

	/* set the mode for 8bits temporarly */
	MV_REG_BIT_RESET( MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK );

	/* First clear the bit in the interrupt cause register */
	MV_REG_WRITE( MV_SPI_INT_CAUSE_REG, 0x0 );

	/* Transmit data */
	MV_REG_WRITE( MV_SPI_DATA_OUT_REG, txData );

	/* wait with timeout for memory ready */
	for ( i = 0; i < MV_SPI_WAIT_RDY_MAX_LOOP; i++ )
	{
		if ( MV_REG_READ( MV_SPI_INT_CAUSE_REG ))
		{
			ready = MV_TRUE;
			break;
		}
#ifdef MV_SPI_SLEEP_ON_WAIT
		mvOsSleep( 1 );
#endif /* MV_SPI_SLEEP_ON_WAIT */
	}

	/* set the mode back to the default 16bit */
	MV_REG_BIT_SET( MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK );

	if ( !ready )
		return MV_TIMEOUT;

	/* check that the RX data is needed */
	if ( pRxData )
		*pRxData = MV_REG_READ( MV_SPI_DATA_IN_REG );

	return MV_OK;
}

/*
#####################################################################################
#####################################################################################
*/

/*******************************************************************************
* mvSpiInit - Initialize the SPI controller
*
* DESCRIPTION:
*		Perform the neccessary initialization in order to be able to send an
*		receive over the SPI interface.
*
* INPUT:
*		serialBaudRate: Baud rate ( SPI clock frequency )
*		use16BitMode: Whether to use 2bytes ( MV_TRUE ) or 1bytes ( MV_FALSE )
*
* OUTPUT:
*		None.
*
* RETURN:
*		Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSpiInit	( MV_U32 serialBaudRate )
{
	MV_STATUS ret;

	/* Set the serial clock */
	if (( ret = mvSpiBaudRateSet( serialBaudRate )) != MV_OK )
		return ret;

	/* Configure the default SPI mode to be 16bit */
	MV_REG_BIT_SET( MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK );

	/* Verify that the CS is deasserted */
	mvSpiCsDeassert();

	return MV_OK;
}

/*******************************************************************************
* mvSpiBaudRateSet - Set the Frequency of the SPI clock
*
* DESCRIPTION:
*		Set the Prescale bits to adapt to the requested baud rate ( the clock
*		used for thr SPI ).
*
* INPUT:
*		serialBaudRate: Baud rate ( SPI clock frequency )
*
* OUTPUT:
*		None.
*
* RETURN:
*		Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSpiBaudRateSet ( MV_U32 serialBaudRate )
{
	MV_U8 i;
	/* MV_U8 preScale[ 32 ] = { 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
						 2, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30 };
	*/
	MV_U8 preScale[ 14 ] = { 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30 };
	MV_U8 bestPrescaleIndx = 100;
	MV_U32 minBaudOffset = 0xFFFFFFFF;
	MV_U32 cpuClk = CONFIG_SYS_TCLK;
	MV_U32 tempReg;

	/* Find the best prescale configuration - less or equal */
	for ( i = 0; i < 14; i++ )
	{
		/* check for higher - irrelevent */
		if (( cpuClk / preScale[ i ]) > serialBaudRate )
			continue;

		/* check for exact fit */
		if (( cpuClk / preScale[ i ]) == serialBaudRate )
		{
			bestPrescaleIndx = i;
			break;
		}

		/* check if this is better than the previous one */
		if (( serialBaudRate - ( cpuClk / preScale[ i ])) < minBaudOffset )
		{
			minBaudOffset = ( serialBaudRate - ( cpuClk / preScale[ i ]));
			bestPrescaleIndx = i;
		}
	}

	if ( bestPrescaleIndx > 14 )
	{
		mvOsPrintf( "%s ERROR: SPI baud rate prescale error!\n", __FUNCTION__ );
		return MV_OUT_OF_RANGE;
	}

	/* configure the Prescale */
	tempReg = MV_REG_READ( MV_SPI_IF_CONFIG_REG );
	tempReg = (( tempReg & ~MV_SPI_CLK_PRESCALE_MASK ) | ( bestPrescaleIndx + 0x12 ));
	MV_REG_WRITE( MV_SPI_IF_CONFIG_REG, tempReg );

	return MV_OK;
}

/*******************************************************************************
* mvSpiCsAssert - Assert the Chip Select pin indicating a new transfer
*
* DESCRIPTION:
*		Assert The chip select - used to select an external SPI device
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		Success or Error code.
*
********************************************************************************/
MV_VOID mvSpiCsAssert( MV_VOID )
{
	MV_REG_BIT_SET( MV_SPI_IF_CTRL_REG, MV_SPI_CS_ENABLE_MASK );
}

/*******************************************************************************
* mvSpiCsDeassert - DeAssert the Chip Select pin indicating the end of a
*					SPI transfer sequence
*
* DESCRIPTION:
*		DeAssert the chip select pin
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		Success or Error code.
*
********************************************************************************/
MV_VOID mvSpiCsDeassert( MV_VOID )
{
	MV_REG_BIT_RESET( MV_SPI_IF_CTRL_REG, MV_SPI_CS_ENABLE_MASK );
}

/*******************************************************************************
* mvSpiRead - Read a buffer over the SPI interface
*
* DESCRIPTION:
*		Receive ( read ) a buffer over the SPI interface in 16bit chunks. If the
*		buffer size is odd, then the last chunk will be 8bits. Chip select is not
*		handled at this level.
*
* INPUT:
*		pRxBuff: Pointer to the buffer to hold the received data
*		buffSize: length of the pRxBuff
*
* OUTPUT:
*		pRxBuff: Pointer to the buffer with the received data
*
* RETURN:
*		Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSpiRead( MV_U8 * pRxBuff, MV_U32 buffSize )
{
	MV_STATUS ret;
	MV_U32 bytesLeft = buffSize;
	MV_U8 * rxPtr = pRxBuff;

	/* check for null parameters */
	if ( pRxBuff == NULL )
	{
		mvOsPrintf( "%s ERROR: Null pointer parameter!\n", __FUNCTION__ );
		return MV_BAD_PARAM;
	}

	/* Verify that the SPI mode is in 16bit mode */
	MV_REG_BIT_SET( MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK );

	/* TX/RX as long we have bytes left */
	while ( bytesLeft )
		if (( bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE ) &&
				( MV_IS_ALIGN( rxPtr, 2 ))) {

			/* Transmitted and wait for the transfer to be completed */
			if (( ret = mvSpi16bitDataTxRx( MV_SPI_DUMMY_WRITE_16BITS,
					( MV_U16 * )( void * )rxPtr )) != MV_OK )
				return ret;

			/* increment the pointers */
			rxPtr += MV_SPI_16_BIT_CHUNK_SIZE;
			bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
		} else {
			/* Transmitted and wait for the transfer to be completed */
			if (( ret = mvSpi8bitDataTxRx( MV_SPI_DUMMY_WRITE_8BITS, rxPtr )) != MV_OK )
				return ret;

			/* increment the pointers */
			rxPtr++;
			bytesLeft--;
		}

	return MV_OK;
}

/*******************************************************************************
* mvSpiWrite - Transmit a buffer over the SPI interface
*
* DESCRIPTION:
*		Transmit a buffer over the SPI interface in 16bit chunks. If the
*		buffer size is odd, then the last chunk will be 8bits. No chip select
*		action is taken.
*
* INPUT:
*		pTxBuff: Pointer to the buffer holding the TX data
*		buffSize: length of the pTxBuff
*
* OUTPUT:
*		None.
*
* RETURN:
*		Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSpiWrite( MV_U8 * pTxBuff, MV_U32 buffSize )
{
	MV_STATUS ret;
	MV_U32 bytesLeft = buffSize;
	MV_U8 * txPtr = pTxBuff;

	/* check for null parameters */
	if ( pTxBuff == NULL )
	{
		mvOsPrintf( "%s ERROR: Null pointer parameter!\n", __FUNCTION__ );
		return MV_BAD_PARAM;
	}

	/* Verify that the SPI mode is in 16bit mode */
	MV_REG_BIT_SET( MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK );

	/* TX/RX as long we have bytes left */
	while ( bytesLeft )
		if (( bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE ) &&
				( MV_IS_ALIGN( txPtr, 2 ))) {

			/* Transmitted and wait for the transfer to be completed */
			if (( ret = mvSpi16bitDataTxRx( *( MV_U16 * )( void * )txPtr,
					NULL )) != MV_OK )
				return ret;

			/* increment the pointers */
			txPtr += MV_SPI_16_BIT_CHUNK_SIZE;
			bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
		} else {
			/* Transmitted and wait for the transfer to be completed */
			if (( ret = mvSpi8bitDataTxRx( *txPtr, NULL )) != MV_OK )
				return ret;

			/* increment the pointers */
			txPtr++;
			bytesLeft--;
		}

	return MV_OK;
}


/*******************************************************************************
* mvSpiReadWrite - Read and Write a buffer simultanuosely
*
* DESCRIPTION:
*		Transmit and receive a buffer over the SPI in 16bit chunks. If the
*		buffer size is odd, then the last chunk will be 8bits. The SPI chip
*		select is not handled implicitely.
*
* INPUT:
*		pRxBuff: Pointer to the buffer to write the RX info in
*		pTxBuff: Pointer to the buffer holding the TX info
*		buffSize: length of both the pTxBuff and pRxBuff
*
* OUTPUT:
*		pRxBuff: Pointer of the buffer holding the RX data
*
* RETURN:
*		Success or Error code.
*
*
*******************************************************************************/
MV_STATUS mvSpiReadWrite( MV_U8 * pRxBuff, MV_U8 * pTxBuff, MV_U32 buffSize )
{
	MV_STATUS ret;
	MV_U32 bytesLeft = buffSize;
	MV_U8 * txPtr = pTxBuff;
	MV_U8 * rxPtr = pRxBuff;

	/* check for null parameters */
	if (( pRxBuff == NULL ) || ( pTxBuff == NULL ))
	{
		mvOsPrintf( "%s ERROR: Null pointer parameter!\n", __FUNCTION__ );
		return MV_BAD_PARAM;
	}

	/* Verify that the SPI mode is in 16bit mode */
	MV_REG_BIT_SET( MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK );

	/* TX/RX as long we have bytes left */
	while ( bytesLeft )
		if (( bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE ) &&
				( MV_IS_ALIGN( rxPtr, 2 )) &&
				( MV_IS_ALIGN( txPtr, 2 ))) {

			/* Transmitted and wait for the transfer to be completed */
			if (( ret = mvSpi16bitDataTxRx( *( MV_U16 * )( void * )txPtr,
					( MV_U16 * )( void * )rxPtr )) != MV_OK )
				return ret;

			/* increment the pointers */
			rxPtr += MV_SPI_16_BIT_CHUNK_SIZE;
			txPtr += MV_SPI_16_BIT_CHUNK_SIZE;
			bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
		} else {
			/* Transmitted and wait for the transfer to be completed */
			if (( ret = mvSpi8bitDataTxRx( *txPtr, rxPtr )) != MV_OK )
				return ret;

			/* increment the pointers */
			rxPtr++;
			bytesLeft--;
		}

	return MV_OK;
}


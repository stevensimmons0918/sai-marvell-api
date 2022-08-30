/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/

/**
********************************************************************************
* @file miiInf.h
*
* @brief Interface for ethernet communication to MII-interface based ASICS.
*
* @version   2
********************************************************************************
*/

#ifndef __INCmiiInfh
#define __INCmiiInfh

#define MAC_SRC_ADDR_SIZE       6
#define MARVELL_HEADER_SIZE     2

#define RESERVED_BIT_31        (1<<31)
#define RESERVED_BIT_15        (1<<15)
#define HDM_PASS               (1<<14)
#define UNIMAC_EN              (1<<7)
#define PROMISCUOUS_MODE       (1<<3)
#define PROMISCUOUS_UNICAST     1
#define UINIMAC_PCR_VALUE (RESERVED_BIT_31|RESERVED_BIT_15|HDM_PASS|UNIMAC_EN|PROMISCUOUS_MODE|PROMISCUOUS_UNICAST)/*0x8000C089*/

#define IN
#define GT_FAIL -1
#define GT_OK 0
#ifndef NULL
#define NULL 0
#endif

typedef unsigned long GT_BOOL;
typedef unsigned long GT_STATUS;
typedef unsigned char GT_U8;
typedef unsigned short GT_U16;
typedef unsigned long GT_U32;

/*******************************************************************************
* GT_Rx_FUNCP
*
* DESCRIPTION:
*       The prototype of the routine to be called after a packet was received
*
* INPUTS:
*       packet_PTR    - The recieved packet.
*       packetLen     - The recived packet len
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE if it has handled the input packet and no further action should
*               be taken with it, or
*       GT_FALSE if it has not handled the input packet and normal processing.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_BOOL (*GT_Rx_FUNCP)
(
    IN GT_U8     *packet_PTR,
    IN GT_U32     packetLen
);

/**
* @internal miiInfInit function
* @endinternal
*
* @brief   This function inits the internal data structure of the ethernet port
*/
GT_STATUS miiInfInit
(
    void
);

/**
* @internal miiInfOutput function
* @endinternal
*
* @brief   This function transmits an Ethenet packet to the Packet processor
*
* @param[in] header_PTR               - pointer the the 14 bytes packet header.
* @param[in] paylaod_PTR              - pointer to the reset of the packet.
* @param[in] payloadLen               - length ot the payload.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS miiInfOutput
(
    IN GT_U8    *header_PTR,
    IN GT_U8    *paylaod_PTR,
    IN GT_U32    payloadLen
);

/**
* @internal miiInfInputHookAdd function
* @endinternal
*
* @brief   This bind the user Rx callback
*
* @param[in] userRxFunc               - the user Rx callbak function
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
GT_STATUS miiInfInputHookAdd
(
    IN GT_Rx_FUNCP userRxFunc
);

/**
* @internal miiInfInputHookDelete function
* @endinternal
*
* @brief   This unbinds the user Rx callback
*/
GT_STATUS miiInfInputHookDelete
(
    void
);

#endif /* __INCmiiInfh */




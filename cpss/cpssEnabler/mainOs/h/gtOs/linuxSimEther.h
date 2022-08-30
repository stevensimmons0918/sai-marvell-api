/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file linuxSimEther.h
*
* @brief Used for linuxSim/osLinuxSimEther.c
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsLinuxSimEtherh
#define __gtOsLinuxSimEtherh

#ifdef __cplusplus
extern "C" {
#endif

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
    IN GT_U8_PTR  packet_PTR,
    IN GT_U32     packetLen
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtOsLinuxSimEtherh */
/* Do Not Add Anything Below This Line */




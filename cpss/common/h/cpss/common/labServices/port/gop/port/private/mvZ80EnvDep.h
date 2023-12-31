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

#ifndef __mvZ80EnvDeph
#define __mvZ80EnvDeph



#define AP_Ports_Num (12)

#define  STATIC
/* Return the mask including "numOfBits" bits.          */
#define U16BIT_MASK(numOfBits) (~(0xFFFFFFFF << (numOfBits)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0xFFFFFCFF                     */
#define U16FIELD_MASK_NOT(offset,len)                      \
        (~(U16BIT_MASK((len)) << (offset)))

/* Calculate the field mask for a given offset & length */
/* e.g.: BIT_MASK(8,2) = 0x00000300                     */
#define U16FIELD_MASK(offset,len)                      \
        ( (U16BIT_MASK((len)) << (offset)) )
/* Returns the info located at the specified offset & length in data.   */
#define U16_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & ((1 << (length)) - 1))


/* Sets the field located at the specified offset & length in data.     */
#define U16_SET_FIELD(data,offset,length,val)           \
   (data) = (((data) & U16FIELD_MASK_NOT((offset),(length))) | ((val) <<(offset)))


#endif   /* __mvZ80EnvDeph */




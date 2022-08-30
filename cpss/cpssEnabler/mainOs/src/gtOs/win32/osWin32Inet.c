/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file osWin32Inet.c
*
* @brief Operating System wrapper. Internet functions.
*
* @version   2
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <winsock2.h>

/**
* @internal osNtohl function
* @endinternal
*
* @brief   convert long integer from network order to host order
*
* @param[in] data                     - long integer in network order
*                                       long integer in host order
*/
GT_U32 osNtohl
(
    IN GT_U32   data
)
{
    return ntohl(data);
}

/**
* @internal osHtonl function
* @endinternal
*
* @brief   convert long integer from host order to network order
*/
GT_U32 osHtonl
(
    IN GT_U32   data
)
{
    return htonl(data);
}

/**
* @internal osNtohs function
* @endinternal
*
* @brief   convert short integer from network order to host order
*
* @param[in] data                     - short integer in network order
*                                       short integer in host order
*/
GT_U16 osNtohs
(
    IN GT_U16   data
)
{
    return ntohs(data);
}

/**
* @internal osHtons function
* @endinternal
*
* @brief   convert short integer from host order to network order
*/
GT_U16 osHtons
(
    IN GT_U16   data
)
{
    return htons(data);
}

/**
* @internal osInetNtoa function
* @endinternal
*
* @brief   convert an network address to dot notation, store it in a buffer.
*
* @param[in] ipAddr                   -  inet address
*
* @param[out] buf                      -  where to return ASCII string
*/
void    osInetNtoa
(
    IN  GT_U32      ipAddr,
    OUT GT_U8       *buf
)
{
    char * ptr;
    struct in_addr in;

    if(buf == NULL)
        return;
    
    in.S_un.S_addr = htonl(ipAddr);
    ptr = inet_ntoa(in);
    if(ptr == NULL)
        return;

    /* copy result to buffer */
    strcpy(buf, ptr);
}




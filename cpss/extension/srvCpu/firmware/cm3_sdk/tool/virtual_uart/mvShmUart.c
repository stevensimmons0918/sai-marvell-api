
#include <stdio.h>
#include <unistd.h>

#include "mvShmUart.h"

#undef VUART_DEBUG

extern unsigned long long cm3_shm_buffs_phys;

MV_U32 CLIUartBaud;
MV_U32 CLIUartPort;

/* static variables */
static MV_U32 *uartRxBase = NULL, *uartTxBase = NULL;
static MV_U32 uartRxOffset = 0, uartTxOffset = 0, uartRingSize = 0;
static MV_U32 *reconnectFlag = NULL, *lastRxIndex = NULL;
static MV_U8  uartTxWhichByte = 0;
static MV_U32 uartTxWordValue = 0;
#ifdef VUART_DEBUG
static unsigned long long uartTxPhy;
#endif

/**
* @internal mvShmUartInit function
* @endinternal
*
*/
MV_VOID mvShmUartInit(MV_U32 *base, MV_U32 size)
{
    uartRingSize = size;
    uartRxBase = base;
    uartTxBase = uartRxBase + uartRingSize/sizeof(MV_U32);
    /* Flag and index for support of reopen of VUART is located after the Watchdog word */
    reconnectFlag =  base + (size/sizeof(MV_U32)) * 2  + 1;
    lastRxIndex = reconnectFlag + 1;

#ifdef VUART_DEBUG
    uartTxPhy  = cm3_shm_buffs_phys + uartRingSize;
#endif
    uartTxOffset = 0;
    *reconnectFlag = 1;
    uartRxOffset = *lastRxIndex;
    uartTxWordValue = 0;
    uartTxWhichByte = 0;
    CLIUartPort = 0;

    return;
}

/**
* @internal mvShmUartPutc function
* @endinternal
*
*/
MV_VOID mvShmUartPutc( MV_U32 port, MV_U8 c )
{
    MV_U8 ii = 0;
    if  (c != 0){
        while(ii < 100)
        {
            if((!uartTxWhichByte) && (*(uartTxBase + uartTxOffset/sizeof(MV_U32)) != 0))
            {
                /* first byte in word, if the aligned word is not empty yet than sleep for 1 ms  */
                usleep(1000);
                ii++;
            }
            else
            {
                uartTxWordValue |= (MV_U32)(c) << (uartTxWhichByte * 8); /* shift left according to which byte*/
                *(uartTxBase + uartTxOffset/sizeof(MV_U32)) = uartTxWordValue;
                uartTxWhichByte++;
                if (uartTxWhichByte > 3)
                {
                    uartTxWhichByte = 0;
                    uartTxWordValue = 0;
                    uartTxOffset+=sizeof(MV_U32);
                    if (uartTxOffset >= uartRingSize)
                        uartTxOffset = 0;
                }
                break;
            }
        }
    }
    return;
}

/**
* @internal mvShmUartGetc function
* @endinternal
*
*/
MV_U8 mvShmUartGetc( MV_U32 port )
{
    char c = 0;
    MV_U32 uartRxWordValue;
    static MV_U8 uartRxWhichByte;

    uartRxWordValue = *(uartRxBase + uartRxOffset/sizeof(MV_U32));
    c = (MV_U8)(uartRxWordValue >> (uartRxWhichByte * 8)) & 0xff;
    if (c != 0)
    {
        uartRxWhichByte++;
        if(uartRxWhichByte > 3) /* if read all 4 bytes - reset counter and erase word */
        {
            uartRxWhichByte = 0;
            *(uartRxBase + uartRxOffset/sizeof(MV_U32)) = 0;
            uartRxOffset+=sizeof(MV_U32);
            if (uartRxOffset >= uartRingSize)
                uartRxOffset = 0;
        }
        *lastRxIndex = uartRxOffset;
    }
    return c;
}

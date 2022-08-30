
#ifndef MV_SHM_UART_H
#define MV_SHM_UART_H

#include "common.h"

MV_VOID mvShmUartInit(MV_U32 *base, MV_U32 size);
MV_VOID mvShmUartPutc( MV_U32 port, MV_U8 c );
MV_U8 mvShmUartGetc( MV_U32 port );


#endif


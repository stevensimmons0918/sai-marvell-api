// slanSendPkt.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "slanLib.h"

void* rx_thread(void* unused)
{
    slanLibMainLoop(0);
    return NULL;
}

void ev_handler_func
(
    IN  SLAN_ID             slanId,
    IN  void*               userData,
    IN  SLAN_LIB_EVENT_TYPE eventType,
    IN  char*               pktData,
    IN  int                 pktLen
)
{
    printf("Got event for slan %d, userData=%p, evType=%d pktLen=%d\n",
            slanId, userData, eventType, pktLen);
}

void Usage(void)
{
    printf("Usage; slanSendPkt <slanName> <packet_file>\n");
    exit(0);
}

int main(int argc, char **argv)
{
    pthread_t rxthreadId;
    SLAN_ID slan;
    char buf[2048];
    int buflen;
    FILE    *f;

    if (argc < 2)
        Usage();

    if ((f = fopen(argv[2], "r")) == NULL)
    {
        perror("fopen()");
        Usage();
    }
    buflen = fread(buf, 1, sizeof(buf), f);
    fclose(f);
    if (buflen < 0)
        perror("fread()");
    if (buflen <= 0)
        return 1;

    slanLibInit(3,"slanSendPkt");

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);

    slanLibBind(argv[1], ev_handler_func, NULL, &slan);

    sleep(1);
    printf("transmitting packet...\n");
    slanLibTransmit(slan, buf, buflen);

    sleep(1);

    return 0;
}

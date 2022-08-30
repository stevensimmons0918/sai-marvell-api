// testSlan.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
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

int main()
{
    pthread_t rxthreadId;
    SLAN_ID slanA, slanB,slanC;

    slanLibInit(3,"test");

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);

    slanLibBind("test", ev_handler_func, (void*)0xa, &slanA);
    slanLibBind("test", ev_handler_func, (void*)0xb, &slanB);
    slanLibBind("test", ev_handler_func, (void*)0xc, &slanC);

    sleep(3);
    printf("transmitting packet...\n");
    slanLibTransmit(slanA, "12345", 5);

    sleep(4);

    while (1)
        sleep(10);

    return 0;
}

// testSlanPerf.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "slanLib.h"

#define PKT_COUNT 1000000
#define PKT_SIZE  1500

void* rx_thread(void* unused)
{
    slanLibMainLoop(0);
    return NULL;
}


struct timeval t_start;
int rcount = 0;
long rbytes = 0;

void ev_handler_func
(
    IN  SLAN_ID             slanId,
    IN  void*               userData,
    IN  SLAN_LIB_EVENT_TYPE eventType,
    IN  char*               pktData,
    IN  int                 pktLen
)
{
    struct timeval t_end;
#if 0
    printf("Got event for slan %d, userData=%p, evType=%d pktLen=%d\n",
            slanId, userData, eventType, pktLen);
#endif
    if (eventType != SLAN_LIB_EV_PACKET)
    {
        printf("Got event for slan %d, userData=%p, evType=%d pktLen=%d\n",
                slanId, userData, eventType, pktLen);
        return;
    }
    rcount++;
    rbytes += pktLen;
    if (rcount < PKT_COUNT)
        return;
    
    gettimeofday(&t_end, NULL);
    t_end.tv_sec -= t_start.tv_sec;
    if (t_end.tv_usec < t_start.tv_usec)
    {
        t_end.tv_sec--;
        t_end.tv_usec += 1000000;
    }
    t_end.tv_usec -= t_start.tv_usec; \
    printf("Received %d packets, %ld bytes in %d.%06d seconds\n",
            rcount, rbytes, (int)t_end.tv_sec, (int)t_end.tv_usec);
    exit(0);
}

int main()
{
    pthread_t rxthreadId;
    SLAN_ID slanA, slanB;
    char    buf[1600];
    int     k;

    slanLibInit(2, "test");

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);

    slanLibBind("perf", ev_handler_func, (void*)0xa, &slanA);
    slanLibBind("perf", ev_handler_func, (void*)0xb, &slanB);

    sleep(3);

    printf("start transmitting packets...\n");
    strcpy(buf, "12345");
    gettimeofday(&t_start, NULL);
    for (k = 0; k < PKT_COUNT; k++)
        slanLibTransmit(slanA, buf, PKT_SIZE);

    while (1)
        sleep(10);

    return 0;
}

// slanRcv.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/* receive data from slan
 * Build:
 *  cc -pthreads -Wall -o slanRcv slanRcv.c slanLib.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "slanLib.h"

/* get/set methods */
int get_u16(unsigned char*  pktData) {
    return (pktData[0]<<8) + pktData[1];
}
int get_u8(unsigned char*  pktData) {
    return pktData[0];
}
void set_u16(unsigned char*  pktData, int data) {
    pktData[0] = (data >> 8) & 0x00ff;
    pktData[1] = (data) & 0x00ff;
}
void set_u8(unsigned char*  pktData, int data) {
    pktData[0] = (data) & 0x00ff;
}


void dumpPkt(
    unsigned char*      pktData,
    int                 pktLen
)
{
    int k;
    for (k = 0; k < pktLen; k++)
    {
        if (k % 16 == 0)
            printf("  data:");
        printf(" %02x", pktData[k]);
        if (k % 16 == 15 && k + 1 < pktLen)
            printf("\n");
    }
    printf("\n");
}

unsigned char myMac[6]={0,1,2,3,4,5};
void handle_arp(
    SLAN_ID         slanId,
    unsigned char*  pktData,
    int             pktLen,
    int             etOffset
)
{
    unsigned char reply[512];
    int arpOffset = etOffset+2;
#define Q pktData+arpOffset
    int hType = get_u16(Q+0);
    int pType = get_u16(Q+2);
    int hLen = get_u8(Q+4);
    int pLen = get_u8(Q+5);
    int oper = get_u16(Q+6);
    int l;
    if (hType != 0x0001 || pType != 0x0800)
    {
        printf("ARP: hType(0x%x) != 0x0001(ethernet) || pType(0x%x) != 0x0800\n",hType,pType);
        return;
    }
    if (hLen != 6 || pLen != 4)
    {
        printf("hLen(%d) != 6 || pLen(%d) != 4\n",hLen, pLen);
        return;
    }
    if (oper == 1)
    {
        /* ARP request */
        /*L2*/
        memcpy(reply+0, pktData + arpOffset + 8, hLen); /* srcMac */
        memcpy(reply+6, myMac, 6); /* dstMac */

        memcpy(reply+12, pktData+12, etOffset+2+8-12); /* vlan tag+ethertype+arp header */
#define R reply+arpOffset
        set_u16(R+6,2); /* oper = 2 == reply */
        memcpy(R+8,myMac,hLen);/* sender HA */
        memcpy(R+8+hLen,Q+8+hLen+pLen+hLen,pLen);/* sender PA == q.targetPA*/
        memcpy(R+8+hLen+pLen,Q+8,hLen); /* target HA = q.senderHA*/
        memcpy(R+8+hLen+pLen+hLen,Q+8+hLen,pLen); /* target PA = q.senderPA */

        l = arpOffset+8+hLen+pLen+hLen+pLen;
        printf("send ARP reply:\n");

        dumpPkt(reply, l);
        if (l < 64)
            l = 64;

        slanLibTransmit(slanId, (char*)reply, l);
    }
#undef R
#undef Q
}

int crc16(
    unsigned char*  pktData,
    int             pktLen
)
{
    int sum = 0;
    int k;
    for (k = 0; k+1<pktLen; k+=2)
    {
        sum += get_u16(pktData+k);
        if (sum > 0xffff)
            sum = (sum & 0xffff) + 1;
    }
    if (k < pktLen)
    {
        sum += get_u8(pktData+k);
        if (sum > 0xffff)
            sum = (sum & 0xffff) + 1;
    }
    return (~sum) & 0xffff;
}
void handle_ip(
    SLAN_ID         slanId,
    unsigned char*  pktData,
    int             pktLen,
    int             etOffset
)
{
    int ipOffset = etOffset+2;
    int icmpOffset;
    int icmpLen;
    int ipHlen;
    unsigned char reply[512];
#define Q pktData+ipOffset

    /* check target MAC */
    if (memcmp(pktData, myMac, 6) != 0)
    {
        /* doesn't match */
        return;
    }
    if ((get_u8(Q) & 0xf0) != 0x40)
    {
        printf("bad ip type length (0x%02x)\n",get_u8(Q));
        return;
    }
    ipHlen = (get_u8(Q) & 0x0f) * 4;
    /* checksum verify */
    if (crc16(Q,ipHlen) != 0)
    {
        printf("bad ip header checksum\n");
        return;
    }
    if (get_u8(Q+9) != 1) /* protocol != ICMP */
    {
        return;
    }
    /* handle ICMP */
    icmpOffset = ipOffset + ipHlen;
    icmpLen = get_u16(Q+2) - ipHlen;
#define I pktData+icmpOffset
    /* ICMP checksum verify */
    if (crc16(I, icmpLen) != 0)
    {
        printf("bad ICMP checksum\n");
    }
    /* handle icmptype == 8 && code == 0  - echo request */
    if (get_u8(I+0) != 8 && get_u8(I+1) != 0)
        return;
    /* ICMP echo request */
    /* data:
     *      I+4 id(16)
     *      I+8 seqN(16)
     *      I+12 data(variable)
     */

    memcpy(reply, pktData, pktLen);
#define RQ reply + ipOffset
#define RI reply + icmpOffset

    /* L2 */
    memcpy(reply, pktData + 6, 6); /* dst mac == inpkt.srcMac */
    memcpy(reply+6, myMac, 6);
    /* L3 */
    memcpy(RQ+12,Q+16,4); /* src ip = inpkt.dstIp */
    memcpy(RQ+16,Q+12,4); /* dst ip = inpkt.srcIp */
    set_u16(RQ+10,0); /* crc */
    set_u16(RQ+10,crc16(RQ,ipHlen));

    /* ICMP */
    set_u8(RI,0);
    set_u8(RI+1,0); /* echo reply */
    /* ICMP checksum */
    set_u16(RI+2,0);
    set_u16(RI+2,crc16(RI,icmpLen));

    printf("send ICMP reply:\n");

    dumpPkt(reply, pktLen);
    slanLibTransmit(slanId, (char*)reply, pktLen);
#undef RQ
#undef RI
#undef I
#undef Q
}

int handle_proto = 0;

void ev_handler_func
(
    IN  SLAN_ID             slanId,
    IN  void*               userData,
    IN  SLAN_LIB_EVENT_TYPE eventType,
    IN  unsigned char*      pktData,
    IN  int                 pktLen
)
{
    printf("Got event for slan %d, userData=%p, evType=%d pktLen=%d\n",
            slanId, userData, eventType, pktLen);
    if (eventType == SLAN_LIB_EV_PACKET)
    {
        dumpPkt(pktData, pktLen);
        if (handle_proto)
        {
            int etOffset = 12;
            int etherType;
            etherType = get_u16(pktData+etOffset);
            /* skip vlan tags */
            while (etherType == 0x8100)
            {
                etOffset += 4;
                etherType = get_u16(pktData+etOffset);
            }
            if (etherType == 0x0806)
            {
                handle_arp(slanId, pktData, pktLen, etOffset);
            }
            if (etherType == 0x0800)
            {
                handle_ip(slanId, pktData, pktLen, etOffset);
            }

        }
    }
}

void* rx_thread(void* unused)
{
    slanLibMainLoop(0);
    return NULL;
}

void Usage(void)
{
    printf("Usage; slanRcv <slanName> [handleIcmp]\n");
    exit(0);
}

int main(int argc, char **argv)
{
    pthread_t rxthreadId;
    SLAN_ID slan;

    if (argc < 2)
        Usage();

    if (argc > 2 && strcmp(argv[2], "handleIcmp") == 0)
        handle_proto = 1;

    slanLibInit(3,"slanRcv");

    pthread_create(&rxthreadId, NULL, rx_thread, NULL);

    slanLibBind(argv[1], (SLAN_LIB_EVENT_HANDLER_FUNC)ev_handler_func, NULL, &slan);

    sleep(4);

    while (1)
        sleep(10);

    return 0;
}

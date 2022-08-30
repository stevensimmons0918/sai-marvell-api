// xpAppIpc.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpApp.h"
#include "xpsApp.h"
#include "xpAppUtil.h"
#include "xpPyInc.h"

#include "xpsPacketDrv.h"

int xpAppAllocatedDeviceIds[XP_MAX_DEVICES] = {0};

#define XP_RX_DESC_OFF            (16)
#define XP_RX_DESC_PTR_OFF        (40)
#define XP_RX_DESC_HDR_OFF        (32)

XP_STATUS xpAppGetNextDeviceId(xpDevice_t* devId)
{
    int index = 0;

    if (!devId)
    {
        return XP_ERR_NULL_POINTER;
    }

    for (index = 0; index < XP_MAX_DEVICES; ++index)
    {
        if (xpAppAllocatedDeviceIds[index] == 0)
        {
            xpAppAllocatedDeviceIds[index] = 1;
            *devId = (xpDevice_t) index;
            return XP_NO_ERR;
        }
    }

    return XP_ERR_OUT_OF_IDS;
}

XP_STATUS xpAppReleaseDeviceId(xpDevice_t devId)
{
    printf("In xpAppReleaseDeviceId for devId: %u\n", devId);

    if (!xpAppAllocatedDeviceIds[devId])
    {
        printf("Device ID %u already free\n", devId);
        return XP_ERR_BUFFERS_NOT_AVAILABLE;
    }
    else
    {
        printf("Marking deviceId for removal %u\n", devId);
        xpAppAllocatedDeviceIds[devId] = 0;
    }

    return XP_NO_ERR;
}

XP_STATUS xpAppGetValidDeviceIds(xpDevice_t* devIds, unsigned int* n)
{
    XP_STATUS status = XP_NO_ERR;
    xpDevice_t devId;

    *n = 0; /* in case there is an error/null pointer, we should return number of valid deviceIds as 0 */

    if (!devIds)
    {
        printf("devIds array was not allocated\n");
        return XP_ERR_NULL_POINTER;
    }

    for (devId = 0; devId < XP_MAX_DEVICES; ++devId)
    {
        if (xpAppAllocatedDeviceIds[devId] != 0)
        {
            devIds[*n] = devId;
            *n = (*n) + 1;
        }
    }

    return status;
}

int32_t recvTrapPacket(xpsDevice_t devId, uint16_t NumOfPkt)
{
    XP_STATUS ret;
    uint16_t pkts_received = 0;
    uint16_t pkts_recv = 0;
    struct xpPacketInfo **rxPacket = (struct xpPacketInfo **)malloc(sizeof(
                                                                        struct xpPacketInfo *) * NumOfPkt);
    xpPacketInterface interface;
    uint32_t rxPktNum = 0;

    for (rxPktNum = 0; rxPktNum < NumOfPkt; rxPktNum++)
    {
        rxPacket[rxPktNum]  = (struct xpPacketInfo *) malloc(sizeof(
                                                                 struct xpPacketInfo));
        memset(rxPacket[rxPktNum], 0, sizeof(struct xpPacketInfo));
        rxPacket[rxPktNum]->buf = malloc(XP_MAX_PACKET_SIZE);
        if (!rxPacket[rxPktNum]->buf)
        {
            printf("ERROR: memory not available to allocate buffer for Rx\n");
            return XP_ERR_MEM_ALLOC_ERROR;
        }
        rxPacket[rxPktNum]->bufSize = XP_MAX_PACKET_SIZE;
    }

    xpsPacketDriverInterfaceGet(devId, &interface);

    if (interface == ETHER)
    {
        rxPacket[pkts_recv]->bufSize = 1000;
    }

    pkts_received = NumOfPkt;
    if ((ret = xpsPacketDriverReceive(devId, &rxPacket[pkts_recv],
                                      &pkts_received)) == XP_NO_ERR)
    {
        // Process the incoming packet
        for (rxPktNum = 0 ; rxPktNum < pkts_received; rxPktNum++)
        {
            xpSalProcessRxTrapPacket(devId, rxPacket[rxPktNum]->buf,
                                     rxPacket[rxPktNum]->bufSize);
        }
    }

    for (rxPktNum = 0; rxPktNum < NumOfPkt; rxPktNum++)
    {
        if (rxPacket[rxPktNum]->buf)
        {
            free(rxPacket[rxPktNum]->buf);
        }
        free(rxPacket[rxPktNum]);
    }

    free(rxPacket);
    if (pkts_received != NumOfPkt)
    {
        return -1;
    }
    else
    {
        return 0;
    }

    return 0;
}

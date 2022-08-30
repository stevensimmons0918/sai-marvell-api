/* xpSaiHostIfFd.c TODO This file is Linux Specific. Should use SAL APIs.*/

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef MAC
#include <sys/socket.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include "xpSaiHostInterface.h"
#include "xpsPacketDrv.h"

XP_SAI_LOG_REGISTER_API(SAI_API_HOSTIF);

static uint8_t xpSaiFdRecvPktBuf_g[XP_MAX_PACKET_SIZE];
static uint8_t xpSaiFdSendPktBuf_g[XP_MAX_PACKET_SIZE];

extern sai_status_t xpSaiHostIfTxHeaderGet(xpsDevice_t devId, xpVif_t egressVif,
                                           bool sendToEgress, xphTxHdr *txHeader);

sai_status_t xpSaiFdObjCreate(uint32_t *pFd)
{
    int fd  = 0;
    int err = 0;
    struct sockaddr_in recvSock= {0};

    if (pFd == NULL)
    {
        XP_SAI_LOG_ERR("A null pointer\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        XP_SAI_LOG_ERR("Error: socket create failed\n");
        return SAI_STATUS_FAILURE;
    }

    recvSock.sin_family = AF_INET;
    recvSock.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Linux will pick a port number. */
    recvSock.sin_port = 0;

    if ((err = bind(fd, (struct sockaddr *)&recvSock, sizeof(recvSock))) < 0)
    {
        XP_SAI_LOG_ERR("Error: bind failed \n");
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    /* Set the fds as non-blocking. Select can be used if reqd. */
    if (0 != fcntl(fd, F_SETFL, O_NONBLOCK))
    {
        XP_SAI_LOG_ERR("Error: file descriptor manipulation failed \n");
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    *pFd = fd;

    XP_SAI_LOG_DBG("DBG:: Created FD : %d\n", fd);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiFdObjDelete(uint32_t fd)
{
    close(fd);

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   hostif receive function
*
* Arguments:
*    [in]  hif_id  - host interface id
*    [out] buffer - packet buffer
*    [in,out] buffer_size - [in] allocated buffer size. [out] actual packet size in bytes
*    [in,out] attr_count - [in] allocated list size. [out] number of attributes
*    [out] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    SAI_STATUS_BUFFER_OVERFLOW if buffer_size is insufficient,
*    and buffer_size will be filled with required size. Or
*    if attr_count is insufficient, and attr_count
*    will be filled with required count.
*    Failure status code on error
*/
sai_status_t xpSaiFdPacketRecv(sai_object_id_t  hif_id,
                               sai_size_t *buffer_size,
                               void *buffer,
                               uint32_t *attr_count,
                               sai_attribute_t *attr_list
                              )
{
    sai_status_t     saiStatus      = SAI_STATUS_SUCCESS;
    xphRxHdr         rxH;
    uint16_t         reasonCode     = 0;
    xpsInterfaceId_t ingressVif     = XPS_INTF_INVALID_ID;
    xpsDevice_t      devId          = 0;
    XP_STATUS        status         = XP_NO_ERR;
    uint32_t         channelId      = 0;
    uint32_t          cpuPortNum     = 0;

    xpsInterfaceType_e                  intfType        = XPS_PORT;
    xpSaiHostInterfaceChannelDbEntryT   *pChannelEntry  = NULL;

    struct xpPacketInfo         rxPacket;
    struct xpPacketInfo         *packets[1];

    memset(&rxPacket, 0, sizeof(rxPacket));
    memset(packets, 0, sizeof(packets));
    memset(&rxH, 0, sizeof(rxH));

    packets[0] = &rxPacket;

    if (xpGetSalType() != XP_SAL_HW_TYPE)
    {
        return XP_NO_ERR;
    }

    saiStatus = xpSaiConvertHostIntfOid(hif_id, &channelId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertHostIntfOid() failed with error code: %d\n",
                       saiStatus);
        return saiStatus;
    }

    saiStatus = xpSaiHostIntfChannelDbInfoGet(channelId, &pChannelEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find a channel %d in DB\n", channelId);
        return saiStatus;
    }

    devId = xpSaiObjIdSwitchGet(hif_id);

    memset(xpSaiFdRecvPktBuf_g, 0, XP_MAX_PACKET_SIZE);

    do
    {
        rxPacket.buf = xpSaiFdRecvPktBuf_g;
        rxPacket.bufSize = XP_MAX_PACKET_SIZE;
        /* Get CPU send interface ID. */
        status = xpsGlobalSwitchControlGetCpuPhysicalPortNum(devId, &cpuPortNum);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Couldn't get CPU physical port number, status:%d\n",
                           status);
            return xpsStatus2SaiStatus(status);
        }

    }
    while (rxH.ingressPortNum ==
           cpuPortNum);   /* Ignore Broadcast ARP looped back to CPU*/

    rxPacket.bufSize -= sizeof(xphRxHdr);

    if (*buffer_size < rxPacket.bufSize)
    {
        XP_SAI_LOG_ERR("Buffer of size %zu is not enough for a packet\n", *buffer_size);
        *buffer_size = rxPacket.bufSize;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    *buffer_size = rxPacket.bufSize;
    memcpy(buffer, rxPacket.buf, *buffer_size);

    reasonCode = rxH.reasonCodeMSB;
    reasonCode = (reasonCode << 2) | rxH.reasonCodeLsbs;

    ingressVif = rxH.ingressVifLsbByte2;
    ingressVif = ingressVif << 8 | rxH.ingressVifLsbByte1;
    ingressVif = ingressVif << 8 | rxH.ingressVifLsbByte0;

    status = xpsInterfaceGetType(ingressVif, &intfType);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Interface Get type failed, vif %d, error %d\n", ingressVif,
                       status);
        return xpsStatus2SaiStatus(status);
    }

    if (*attr_count < ((intfType == XPS_LAG) ? 3 : 2))
    {
        XP_SAI_LOG_ERR("attr_count (%u) is insufficient\n", *attr_count);
        *attr_count = ((intfType == XPS_LAG) ? 3 : 2);
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    saiStatus = xpSaiHostInterfaceTrapIdParse(devId, reasonCode, &attr_list[0]);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not identify a packet trap type using reasonCode %d, error %d\n",
                       reasonCode, saiStatus);
        return saiStatus;
    }

    /* Encode ingress port */
    saiStatus = xpSaiHostInterfacePortTypeParse(devId, XPS_PORT, rxH.ingressPortNum,
                                                &attr_list[1]);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not encode an ingress port, error %d\n", saiStatus);
        return saiStatus;
    }

    if (intfType == XPS_LAG)
    {
        /* Encode ingress LAG */
        saiStatus = xpSaiHostInterfacePortTypeParse(devId, XPS_LAG, ingressVif,
                                                    &attr_list[2]);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Could not encode an ingress LAG, error %d\n", saiStatus);
            return saiStatus;
        }
    }

    *attr_count = ((intfType == XPS_LAG) ? 3 : 2);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetPktTxAttr(int32_t attr_count,
                               const sai_attribute_t* attr_list,
                               sai_hostif_tx_type_t* pTxType, uint32_t* pEgressPort)
{
    int32_t             count           = 0;
    bool                txTypeFound     = false;
    bool                egressPortFound = false;
    sai_object_type_t   objType         = SAI_OBJECT_TYPE_NULL;

    for (count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_HOSTIF_PACKET_ATTR_HOSTIF_TX_TYPE:
                {
                    *pTxType = (sai_hostif_tx_type_t)attr_list[count].value.s32;
                    if (*pTxType != SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS &&
                        *pTxType != SAI_HOSTIF_TX_TYPE_PIPELINE_LOOKUP)
                    {
                        XP_SAI_LOG_ERR("Invalid Tx type %d\n", *pTxType);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                    txTypeFound = true;
                    break;
                }
            case SAI_HOSTIF_PACKET_ATTR_EGRESS_PORT_OR_LAG:
                {
                    objType = xpSaiObjIdTypeGet(attr_list[count].value.oid);
                    if (objType != SAI_OBJECT_TYPE_PORT && objType != SAI_OBJECT_TYPE_LAG)
                    {
                        XP_SAI_LOG_ERR("Invalid Tx type %d\n", *pTxType);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                    *pEgressPort = xpSaiObjIdValueGet(attr_list[count].value.oid);
                    egressPortFound = true;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Attribute %d is invalid\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
                }
        }
    }

    if (txTypeFound == false)
    {
        XP_SAI_LOG_ERR("A mandatory SAI_HOSTIF_PACKET_ATTR_TX_TYPE field is not received\n");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    if ((*pTxType == SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS) &&
        (egressPortFound == false))
    {
        XP_SAI_LOG_ERR("A mandatory SAI_HOSTIF_PACKET_EGRESS_PORT_OR_LAG field is not received\n");
        return SAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   hostif send function
*
* Arguments:
*    [in] hif_id  - host interface id. only valid for send through FD channel. Use SAI_NULL_OBJECT_ID for send through CB channel.
*    [In] buffer - packet buffer
*    [in] buffer size - packet size in bytes
*    [in] attr_count - number of attributes
*    [in] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t xpSaiFdPacketSend(sai_object_id_t  hif_id,
                               sai_size_t buffer_size,
                               const void *buffer,
                               uint32_t attr_count,
                               const sai_attribute_t *attr_list
                              )
{
    sai_status_t            retVal      = SAI_STATUS_SUCCESS;
    XP_STATUS               xpStatus    = XP_NO_ERR;
    sai_hostif_tx_type_t    txType      = SAI_HOSTIF_TX_TYPE_PIPELINE_BYPASS;
    uint32_t                egressPort  = 0;
    uint8_t                 *pPktData   = NULL;
    sai_object_type_t       objType     = SAI_OBJECT_TYPE_NULL;
    xpsDevice_t             devId       = 0;
    xpsInterfaceId_t        cpuIfId     = XPS_INTF_INVALID_ID;
    xpsInterfaceType_e      ifType      = XPS_PORT;
    struct xpPacketInfo     pktInfo;
    struct xpPacketInfo     *pPktInfo;

    pPktInfo = &pktInfo;
    memset(pPktInfo, 0, sizeof(struct xpPacketInfo));

    objType = xpSaiObjIdTypeGet(hif_id);
    if (objType == SAI_OBJECT_TYPE_SWITCH || objType == SAI_OBJECT_TYPE_HOSTIF)
    {
        devId = xpSaiObjIdSwitchGet(hif_id);
    }
    else if (objType == SAI_OBJECT_TYPE_NULL)
    {
        devId = 0;
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid SAI Object type (%d) passed\n", objType);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* To avoid fragmentation, allocated @ init time. */
    if ((buffer_size > (XP_MAX_PACKET_SIZE - sizeof(xphTxHdr))) || (buffer == NULL))
    {
        XP_SAI_LOG_ERR("Buffer is invalid\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetPktTxAttr(attr_count, attr_list, &txType, &egressPort);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attributes are wrong\n");
        return retVal;
    }

    memset(xpSaiFdSendPktBuf_g, 0, sizeof(xpSaiFdSendPktBuf_g));
    pPktInfo->buf = xpSaiFdSendPktBuf_g;

    xpStatus = xpsPortGetCPUPortIntfId(devId, &cpuIfId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get CPU interface ID\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpStatus = xpsInterfaceGetType(egressPort, &ifType);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get egress port interface type\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    pPktData = (uint8_t *)pPktInfo->buf + sizeof(xphTxHdr);
    memcpy(pPktData, buffer, buffer_size);

    /* Min packet length to be send is XP_MIN_PACKET_LENGTH bytes
     * Pad the packet with zeroes to XP_MIN_PACKET_LENGTH byte length */
    if (buffer_size < XP_MIN_PACKET_LENGTH)
    {
        memset(pPktData + buffer_size, 0, (XP_MIN_PACKET_LENGTH - buffer_size));
        buffer_size = XP_MIN_PACKET_LENGTH;
    }

    /* Update packet size */
    pPktInfo->bufSize = buffer_size + sizeof(xphTxHdr);


    /* Updating the TC to 8 for all control plane originated traffic */
    xphTxHdr *pTxXpHeader = (xphTxHdr *)pPktInfo->buf;
    pTxXpHeader->TC = 8;


    return SAI_STATUS_SUCCESS;
}
#endif

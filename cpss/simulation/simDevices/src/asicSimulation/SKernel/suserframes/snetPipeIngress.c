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
/**
********************************************************************************
* @file snetPipeIngress.c
*
* @brief This is the implementation for the Ingress processing of 'PIPE' in SKernel.
*
* @version   1
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetPipe.h>
#include <asicSimulation/SKernel/cheetahCommon/sregPipe.h>

#define BIT_12          (0x1000)
#define BIT_11          (0x0800)

/* Number of Transmit Queues in PIPE device */
#define SDMA_TX_QUEUE_MAX_NUMBER       8

/*******************************************************************************
*  snetPipeDuplicateDescr
*
* DESCRIPTION:
*        Duplicate Pipe's descriptor
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*       descrPtr    - frame data buffer Id
*
* RETURN:
*       SKERNEL_FRAME_PIPE_DESCR_STC *
*                   - pointer to the duplicated descriptor of the Pipe
*
*******************************************************************************/
SKERNEL_FRAME_PIPE_DESCR_STC * snetPipeDuplicateDescr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(snetPipeDuplicateDescr);

    SKERNEL_FRAME_PIPE_DESCR_STC *  pipe_newDescrPtr = NULL;


    if(pipe_descrPtr == NULL)
    {
        devObjPtr->pipeDevice.pipe_descrFreeIndx = 0;
    }

    while(1)
    {
        if (devObjPtr->pipeDevice.pipe_descrFreeIndx >= devObjPtr->pipeDevice.pipe_descrNumber)
        {
            /* not enough descriptors ?! */
            skernelFatalError("snetPipeDuplicateDescr: not enough descriptors ?! \n");
        }

        /* Get pointer to the free descriptor */
        pipe_newDescrPtr = devObjPtr->pipeDevice.pipe_descriptorPtr +
                           devObjPtr->pipeDevice.pipe_descrFreeIndx;

        /* Increment free pointer and copy old descriptor to the new one */
        devObjPtr->pipeDevice.pipe_descrFreeIndx++;

        if(pipe_newDescrPtr->numberOfSubscribers == 0)
        {
            /*found descriptor that is not subscribed to (or used by) other operations */
            break;
        }
    }

    if(pipe_descrPtr == NULL)
    {
        __LOG(("Create new descriptor \n"));
        memset(pipe_newDescrPtr, 0, sizeof(SKERNEL_FRAME_PIPE_DESCR_STC));

        /* support creation of cheetah descriptor */
        pipe_newDescrPtr->cheetah_descrPtr =
            snetChtEqDuplicateDescr(devObjPtr,
                NULL);
    }
    else
    { /* descrPtr != NULL */
        __LOG(("Duplicate given descriptor \n"));
        memcpy(pipe_newDescrPtr, pipe_descrPtr, sizeof(SKERNEL_FRAME_PIPE_DESCR_STC));

        /* support duplication of cheetah descriptor */
        if(pipe_descrPtr->cheetah_descrPtr)
        {
            __LOG(("Duplicate descriptor for the cheetah descriptor \n"));

            pipe_newDescrPtr->cheetah_descrPtr = /*the duplication*/
                snetChtEqDuplicateDescr(devObjPtr,
                    pipe_descrPtr->cheetah_descrPtr);/*the original inner descriptor*/
        }
    }

    return pipe_newDescrPtr;
}

/**
* @internal etherTypeIsTpidMatch function
* @endinternal
*
* @brief   check if the etherType match one of the TPIDs
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] pipe_descrPtr            - descriptor for the packet
* @param[in] etherType                - the  to check for TPID match
*
* @param[out] tagLengthPtr             - the length of the recognized tag
*/
static GT_BOOL etherTypeIsTpidMatch
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN GT_U32   etherType,
    OUT GT_U32   *tagLengthPtr
)
{
    DECLARE_FUNC_NAME(etherTypeIsTpidMatch);

    GT_U32  TPID_Valid,TPID_Value,TPID_Tag_Size;
    GT_U32  regAddress , regValue,tpidIndex;
    GT_U32  maxSize = 24;

    __LOG(("check if etherType[0x%4.4x] is recognized as 'tag' \n",
        etherType));

    for(tpidIndex = 0 ; tpidIndex < 4; tpidIndex++)
    {
        regAddress = SMEM_PIPE_PCP_PORT_TPID_REG(devObjPtr,tpidIndex);
        smemRegGet(devObjPtr, regAddress , &regValue);

        TPID_Valid      = SMEM_U32_GET_FIELD(regValue,0,1);
        TPID_Value      = SMEM_U32_GET_FIELD(regValue,1,16);
        TPID_Tag_Size   = SMEM_U32_GET_FIELD(regValue,17,4);

        __LOG_PARAM(TPID_Valid);
        __LOG_PARAM(TPID_Value);

        TPID_Tag_Size *= 2;/*Tag size in 2B resolution for this TPID*/

        LOG_FIELD_VALUE("TPID_Tag_Size(in Bytes)" ,TPID_Tag_Size);

        if(TPID_Valid && (TPID_Value == etherType))
        {
            if(TPID_Tag_Size > maxSize)
            {
                __LOG(("TPID_Tag_Size[%d] is over the max supported[%d] , so considered 'max' \n",
                    TPID_Tag_Size,maxSize));
                /*Up to 24B Tag Size is supported*/
                TPID_Tag_Size = maxSize;
            }

            __LOG(("etherType[0x%4.4x] is recognized as 'tag' with length of [%d] bytes \n",
                etherType,TPID_Tag_Size));
            /* match */
            *tagLengthPtr = TPID_Tag_Size;
            return GT_TRUE;
        }
    }

    __LOG(("etherType[0x%4.4x] is NOT recognized as 'tag' \n",
        etherType));
    *tagLengthPtr = 0;
    return GT_FALSE;
}

/**
* @internal pipeIngressTpidOffset function
* @endinternal
*
* @brief   Skip up to 4 TPIDs ... to get to L3 offset
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] pipe_descrPtr            - descriptor for the packet
* @param[in,out] ethTypeOffsetPtr         - the current offset after the MAC SA.
* @param[in,out] ethTypeOffsetPtr         - the new offset after the TPID
*/
static void pipeIngressTpidOffset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    INOUT GT_U32    *ethTypeOffsetPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressTpidOffset);
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  ii;
    GT_U32  length = 0;
    GT_U32  maxVlanTagsToCheck = 4;
    GT_U32 etherType;               /* real frame ethernet type */
    GT_U32  ethTypeOffset = *ethTypeOffsetPtr;/* ethernet type byte offset */

    for (ii = 0; ii < (maxVlanTagsToCheck+1); ii++, ethTypeOffset += length)
    {
        etherType =
            SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(cheetah_descrPtr, ethTypeOffset);
        __LOG_PARAM(etherType);

        if(ii == maxVlanTagsToCheck)
        {
            break;
        }

        /* check if current ethertype is on of the recognized TPIDs */
        if(GT_FALSE == etherTypeIsTpidMatch(devObjPtr, pipe_descrPtr, etherType, &length))
        {
            break;
        }
    }

    *ethTypeOffsetPtr = ethTypeOffset;
}

/**
* @internal pipeIngressDsaOffset function
* @endinternal
*
* @brief   Skip the DSA tag
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] pipe_descrPtr            - descriptor for the packet
* @param[in,out] ethTypeOffsetPtr         - the current offset after the MAC SA.
* @param[in,out] ethTypeOffsetPtr         - the new offset after the DSA
*/
static void pipeIngressDsaOffset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    INOUT GT_U32    *ethTypeOffsetPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressDsaOffset);
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  dsaOffset = *ethTypeOffsetPtr;
    GT_U8  *dsaOffsetPtr = cheetah_descrPtr->startFramePtr + dsaOffset;
    SKERNEL_EXT_DSA_TAG_TYPE_ENT marvellTaggedExtended;

    marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_1_WORDS_E;
    /* check DSA tag format : TO_CPU ? */
    if((dsaOffsetPtr[0] >> 6) == 0)
    {
        if((dsaOffsetPtr[1] & 7) == 7 &&
           (dsaOffsetPtr[2] & 0x10))
        {
            marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_2_WORDS_E;
        }
    }
    else
    {
        if(dsaOffsetPtr[2] & 0x10)
        {
            marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_2_WORDS_E;
        }
    }

    if(marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_2_WORDS_E)
    {
        if(dsaOffsetPtr[4] & 0x80)
        {
            marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_3_WORDS_E;
        }
    }

    if(marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_3_WORDS_E)
    {
        if(dsaOffsetPtr[8] & 0x80)
        {
            marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_3_WORDS_E;
        }
    }

    if(simLogIsOpenFlag)
    {
        scibAccessLock();

        __LOG(("[%s]",
                    marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E ? "SKERNEL_EXT_DSA_TAG_1_WORDS_E" :
                    marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_2_WORDS_E ? "SKERNEL_EXT_DSA_TAG_2_WORDS_E" :
                    marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_3_WORDS_E ? "SKERNEL_EXT_DSA_TAG_3_WORDS_E" :
                    marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E ? "SKERNEL_EXT_DSA_TAG_4_WORDS_E" :
                    "unknown DSA length format ?!"
                      ));

        __LOG(("dump the bytes of the DSA : (network order)" ));
        /* dump the bytes of the DSA */
        simLogPacketDump(devObjPtr,GT_TRUE/*ingress*/,
                    pipe_descrPtr->pipe_SrcPort,/* local device source port */
                    dsaOffsetPtr,/*start of DSA */
            (1+marvellTaggedExtended) * 4);

        scibAccessUnlock();
    }

    *ethTypeOffsetPtr += 4*(1+marvellTaggedExtended);

    return;
}



/**
* @internal pipeIngressEtherTypeAndL3Offset function
* @endinternal
*
* @brief   EtherType & L3 Offset Assignment
*/
static GT_VOID pipeIngressEtherTypeAndL3Offset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressEtherTypeAndL3Offset);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;

    GT_U32 ethTypeOffset;           /* ethernet type byte offset */
    GT_U32 etherType;               /* real frame ethernet type */
    GT_U32  regAddress;
    GT_U32  DA_Offset;
    GT_U32  ii;
    SKERNEL_L2_ENCAPSULATION_TYPE_ENT l2encapsul;  /* encapsulation */
    /* A packet may contain one or more tags that must be parsed over to locate
       the Ethernet EtherType/Len field.
       A tag is identified by its TPID.
       For this device, the parser can parse over up 4 tags.
    */

    /*
        Ethernet packet are classified as having one of the following formats:
        If the packet is EthernetV2
            EtherType is set to the 16-bit value at the EtherType/Len offset
            L3 offset is set to the EtherType/Len offset.
        If the packet is LLC/SNAP
            EtherType is set to the 16-bit value at the EtherType/Len offset + 8B
            L3 offset is set to the EtherType/Len offset + 8B.
        If the packet is LLC/Non-SNAP
            EtherType is the 16-bit DSAP-SSAP at EtherType/Len offset + 2B
            L3 offset is set to the EtherType/Len offset + 2B


        NOTE: The IPv4/6 and MPLS EtherTypes may appear in either
            EthernetV2 or LLC-SNAP encapsulations.
    */

    /*<PCP_IP> PCP/PCP/PRS/PCP Port L3 Offset Config <<%n>>*/
    /*<DA Offset>*/
    regAddress = SMEM_PIPE_PCP_PORT_DA_OFFSET_CONFIG_REG(devObjPtr,pipe_descrPtr->pipe_SrcPort);
    smemRegFldGet(devObjPtr, regAddress , 0,5 , &DA_Offset);
    __LOG_PARAM(DA_Offset);

    pipe_descrPtr->pipe_DA_Offset = DA_Offset;

    ethTypeOffset = DA_Offset;

    /* save the 6 bytes of mac DA to the 'packetTypeKey' first 6 bytes */
    for(ii = 0 ; ii < 6 ; ii++)
    {
        pipe_descrPtr->pipe_packetTypeKey[ii] =  cheetah_descrPtr->startFramePtr[DA_Offset+ii];
    }

    ethTypeOffset += SGT_MAC_ADDR_BYTES*2; /* skip 12 bytes of  mac addresses */

    if(cheetah_descrPtr->marvellTagged)
    {
        __LOG(("jumping over the DSA tag \n"));
        pipeIngressDsaOffset(devObjPtr,pipe_descrPtr ,&ethTypeOffset);
    }

    /* save the outer most etherType/Length 2 bytes */
    pipe_descrPtr->pipe_etherTypeOrSsapDsap_outerMost  =
        SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(cheetah_descrPtr, ethTypeOffset);
    __LOG_PARAM(pipe_descrPtr->pipe_etherTypeOrSsapDsap_outerMost);

    __LOG(("jumping over the ether tag(s) \n"));
    pipeIngressTpidOffset(devObjPtr,pipe_descrPtr,&ethTypeOffset);

    etherType  =
        SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(cheetah_descrPtr, ethTypeOffset);
    __LOG_PARAM(etherType);

    /* L2 encapsulation parsing */
    if (etherType < 0x0600)
    {
        ethTypeOffset += 2 ;
        etherType = SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(cheetah_descrPtr, ethTypeOffset);

        if (etherType == 0xAAAA &&
            cheetah_descrPtr->startFramePtr[ethTypeOffset + 2] == 0x3)
        {
            __LOG(("l2Encaps : IEEE 802.3 LLC/SNAP (LLC-SNAP) \n"));
            l2encapsul = SKERNEL_LLC_SNAP_E;
            ethTypeOffset += 6;
            etherType = SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(cheetah_descrPtr, ethTypeOffset);
            __LOG_PARAM(etherType);
        }
        else
        {
            __LOG(("l2Encaps : IEEE 802.3 (LLC Encapsulation) (non-SNAP) \n"));
            l2encapsul = SKERNEL_LLC_E;
        }
    }
    else
    {
        l2encapsul = SKERNEL_ETHERNET_II_E;
        __LOG(("l2Encaps : Ethernet v2 \n"));
    }

    pipe_descrPtr->pipe_l2Encaps = l2encapsul;
    __LOG_PARAM(pipe_descrPtr->pipe_l2Encaps);

    pipe_descrPtr->pipe_L3Offset = ethTypeOffset;
    __LOG_PARAM(pipe_descrPtr->pipe_L3Offset);

}

/**
* @internal pipeIngressIpv4Parse function
* @endinternal
*
* @brief   L4 Offset Calculation for Ipv4
*         NOTE: no validity check as done on the IPV4 header (like checksum and others)
*/
static GT_VOID pipeIngressIpv4Parse
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressIpv4Parse);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8   *l3StartOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset + 2;
    GT_U32  ipxHeaderLength,ipv4FragmentOffset;
    GT_BOOL fragment;
    GT_U32  regAddress , IPV4_Enable_ECN_Marking;
    GT_U32  ecnValue;

    /* ipv4 ip header length (in words) = IHL */
    ipxHeaderLength = (l3StartOffsetPtr[0] & 0xf); /* IHL */
    __LOG_PARAM(ipxHeaderLength);
    ipv4FragmentOffset = ((GT_U32)(l3StartOffsetPtr[6] & 0x1f) << 8) |
                                  (l3StartOffsetPtr[7]);
    __LOG_PARAM(ipv4FragmentOffset);

    fragment = (l3StartOffsetPtr[6] & (1<<5)) != 0 ?
            GT_TRUE :/* fragment */
        /* 13 bits of FragmentOffset in the ip header */
        ((ipv4FragmentOffset) == 0) ?
            GT_FALSE :/* not fragment */
            GT_TRUE;/* fragment */

    __LOG_PARAM(fragment);

    if(fragment == GT_TRUE)
    {
        pipe_descrPtr->pipe_L4Offset = 0;
        __LOG((" IPv4 fragment --> hold no L4 offset \n"));
    }
    else
    {
        pipe_descrPtr->pipe_L4Offset = ipxHeaderLength*4;
    }

    __LOG_PARAM(pipe_descrPtr->pipe_L4Offset);

    ecnValue = l3StartOffsetPtr[1] & 0x3;
    __LOG_PARAM(ecnValue);

    regAddress = SMEM_PIPE_PCP_ECN_ENABLE_CONFIG_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddress , 0,1 , &IPV4_Enable_ECN_Marking);
    __LOG_PARAM(IPV4_Enable_ECN_Marking);

    pipe_descrPtr->pipe_ECNCapable = (IPV4_Enable_ECN_Marking && ecnValue) ? 1 : 0;

    pipe_descrPtr->pipe_isIpv4 = 1;
    pipe_descrPtr->pipe_IP_header_Protocol = l3StartOffsetPtr[9];
    pipe_descrPtr->pipe_IP_isFragment = fragment ? 1 : 0;
}

/**
* @internal pipe_calcIpv6HeaderLength function
* @endinternal
*
* @brief   calculates ipv6 header length (in words)
*
* @retval ipv6 header length (in words) - 0 means not relevant
*                                       based on calcIpv6HeaderLength(...)
*/
static GT_U32 pipe_calcIpv6HeaderLength
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    GT_U32                        ipProt,
    IN    GT_U8                        *l3StartOffsetPtr
)
{
    GT_U32 val = 0;
    GT_U32 hdrExtLen = 0;

    if((ipProt == SNET_IPV6_HBH_PROT_E) &&
            (l3StartOffsetPtr[40 + 16*2] == SNET_IPV6_NO_NEXT_HEADER_PROT_E))
    { /* If a single HBH header exists */

        /* get Hdr-Ext-Len -
           Length of HBH header in 8-octet units, not including the first 8 octets. */
        hdrExtLen = l3StartOffsetPtr[40 + 16*2 + 1];

        /* Desc<IPx Header Length> = 10 + (Hdr-Ext-Len+1)*2.
           Where Hdr-Ext-Len is taken from the HBH header */
        val = 10 + (hdrExtLen+1)*2;
    }
    else
    { /* If there are no header extensions Desc<IPx Header Length> = 10 */
        val = 10;
    }

    return val;
}

/**
* @internal pipeIngressIpv6Parse function
* @endinternal
*
* @brief   L4 Offset Calculation for Ipv6
*         NOTE: no validity check as done on the IPV6 header
*/
static GT_VOID pipeIngressIpv6Parse
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressIpv6Parse);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8  *l3StartOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset + 2;
    GT_U32  ipxHeaderLength,ipProt;
    GT_BOOL fragment;
    GT_U32  regAddress , IPV6_Enable_ECN_Marking;
    GT_U32  ecnValue;

    ipProt = l3StartOffsetPtr[6];
    __LOG_PARAM(ipProt);

    /* calculate ipv6 header length */
    ipxHeaderLength = pipe_calcIpv6HeaderLength(devObjPtr, ipProt , l3StartOffsetPtr);

    __LOG_PARAM(ipxHeaderLength);

    fragment = (ipProt == 44) ? GT_TRUE : GT_FALSE;
    __LOG_PARAM(fragment);

    if(fragment == GT_TRUE)
    {
        pipe_descrPtr->pipe_L4Offset = 0;
        __LOG((" IPv6 fragment --> hold no L4 offset \n"));
    }
    else
    {
        pipe_descrPtr->pipe_L4Offset = ipxHeaderLength;
    }

    __LOG_PARAM(pipe_descrPtr->pipe_L4Offset);

    ecnValue = (l3StartOffsetPtr[1] >> 4) & 0x3;
    __LOG_PARAM(ecnValue);

    regAddress = SMEM_PIPE_PCP_ECN_ENABLE_CONFIG_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddress , 1,1 , &IPV6_Enable_ECN_Marking);
    __LOG_PARAM(IPV6_Enable_ECN_Marking);

    pipe_descrPtr->pipe_ECNCapable = (IPV6_Enable_ECN_Marking && ecnValue) ? 1 : 0;

    pipe_descrPtr->pipe_isIpv6 = 1;
    pipe_descrPtr->pipe_IP_header_Protocol = ipProt;
    pipe_descrPtr->pipe_IP_isFragment = fragment ? 1 : 0;

}

/**
* @internal pipeIngressL4Offset function
* @endinternal
*
* @brief   L4 Offset Calculation
*/
static GT_VOID pipeIngressL4Offset
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressL4Offset);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  regAddress , regValue;
    GT_U32  IPV4_EtherType_Valid,IPV4_Ethertype;
    GT_U32  IPV6_EtherType_Valid,IPV6_Ethertype;
    GT_U32  etherType;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;

    __LOG(("Dump first 32 bytes of the L3 header \n"));
    /* dump the start of L3 */
    simLogPacketDump(devObjPtr,
                    GT_TRUE/*ingress*/,
                    pipe_descrPtr->pipe_SrcPort,/* local device source port */
                    &cheetah_descrPtr->startFramePtr[ethTypeOffset],/*start of L3 header */
                    32);/* dump 32 bytes of the header */

    etherType = SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(cheetah_descrPtr, ethTypeOffset);

    regAddress = SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_0_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    IPV4_EtherType_Valid = SMEM_U32_GET_FIELD(regValue,0,1);
    IPV4_Ethertype       = SMEM_U32_GET_FIELD(regValue,1,16);
    __LOG_PARAM(IPV4_EtherType_Valid);
    __LOG_PARAM(IPV4_Ethertype);

    if(IPV4_EtherType_Valid && (IPV4_Ethertype != SKERNEL_L3_PROT_TYPE_IPV4_E))
    {
        __LOG(("Warning : the IPv4 etherType in the register is NOT 'standard' IPv4 !!! \n"));
    }

    if(IPV4_EtherType_Valid && (IPV4_Ethertype == etherType))
    {
        __LOG(("L3 is Ipv4 \n"));
        pipeIngressIpv4Parse(devObjPtr,pipe_descrPtr);

        return;
    }

    regAddress = SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_1_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    IPV6_EtherType_Valid = SMEM_U32_GET_FIELD(regValue,0,1);
    IPV6_Ethertype       = SMEM_U32_GET_FIELD(regValue,1,16);
    __LOG_PARAM(IPV6_EtherType_Valid);
    __LOG_PARAM(IPV6_Ethertype);

    if(IPV6_EtherType_Valid && (IPV6_Ethertype != SKERNEL_L3_PROT_TYPE_IPV6_E))
    {
        __LOG(("Warning : the IPv4 etherType in the register is NOT 'standard' IPv6 !!! \n"));
    }

    if(IPV6_EtherType_Valid && (IPV6_Ethertype == etherType))
    {
        __LOG(("L3 is Ipv6 \n"));
        pipeIngressIpv6Parse(devObjPtr,pipe_descrPtr);
        return;
    }

    pipe_descrPtr->pipe_L4Offset = 0;
    __LOG((" non IPv4 and non IPv6 --> hold no L4 offset \n"));

    return;

}

/* macro to return result of which bits caused the 'NO MATCH'
   The logic is according to verifier code and NOT like in FS description */
#define P_M_K_FIND_NO_MATCH(p,m,k,mask)  \
    ((((k)|(m))&(mask))!=(((p)|(m))&(mask)))

/* BYTE check : macro to return result of which bits caused the 'NO MATCH' */
#define BYTE_P_M_K_FIND_NO_MATCH(p,m,k)  \
    P_M_K_FIND_NO_MATCH(p,m,k,0xFF)

/* BIT in BYTE check : macro to return result of which bits caused the 'NO MATCH' */
#define BIT_IN_BYTE_P_M_K_FIND_NO_MATCH(p,m,k,startBit)  \
    P_M_K_FIND_NO_MATCH(p,m,k,(1<<startBit))

/**
* @internal pipeIngressIp2meIndexCalculate function
* @endinternal
*
* @brief   Calculate IP2ME index. New in PIPE A1.
*/
static GT_VOID pipeIngressIp2meIndexCalculate
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressIp2meIndexCalculate);

    GT_U8 ipAddress[16];
    GT_U8 ipMask[16];
    GT_U8 *dipOffsetPtr = NULL;
    GT_U32 regAddr;
    GT_U32 regValue;
    GT_U32 ii;
    GT_U32 jj;
    GT_U32 prefixLength;
    GT_U32 compareOffset;
    GT_BOOL matched;
    GT_U32 mask;

    if(pipe_descrPtr->pipe_isIpv4 == 1)
    {
        dipOffsetPtr = pipe_descrPtr->cheetah_descrPtr->startFramePtr +
            pipe_descrPtr->pipe_L3Offset + 18;
        compareOffset = 12;
    }
    else
    if(pipe_descrPtr->pipe_isIpv6 == 1)
    {
        dipOffsetPtr = pipe_descrPtr->cheetah_descrPtr->startFramePtr +
            pipe_descrPtr->pipe_L3Offset + 26;
        compareOffset = 0;
    }
    else
    {
        __LOG(("The packet is not IP. Skipping IP address classification.\n"));
        return;
    }

    pipe_descrPtr->pipe_ip2meIdx = 0;
    for(ii = 0; ii < 7; ii++)
    {
        regAddr = SMEM_PIPE_PCP_IP2ME_CTLBITS_REG(devObjPtr, ii);
        smemRegGet(devObjPtr, regAddr , &regValue);
        if(SMEM_U32_GET_FIELD(regValue,0,1) == 0)
        {
            __LOG(("IP2ME entry %d is not valid. Continue.\n", ii));
            continue;
        }
        if(SMEM_U32_GET_FIELD(regValue,8,1) != pipe_descrPtr->pipe_isIpv6)
        {
            __LOG(("IP2ME entry[%d] skipped because : entry type [%s] but "
                   "current packet is [%s] \n", ii ,
                   SMEM_U32_GET_FIELD(regValue,8,1) ? "IPv4":"IPv6",
                   pipe_descrPtr->pipe_isIpv6 ? "IPv6":"IPv4"));
            continue;
        }

        memset(ipAddress, 0, sizeof(ipAddress));
        memset(ipMask, 0, sizeof(ipMask));
        /* The actual prefix length value = register field value + 1, */
        prefixLength = 1 + SMEM_U32_GET_FIELD(regValue,1,7);

        if(pipe_descrPtr->pipe_isIpv6 == 1)
        {
            for(jj = 0; jj < 4; jj++)
            {
                regAddr = SMEM_PIPE_PCP_IP2ME_IPADDR_REG(devObjPtr, ii, jj);
                smemRegGet(devObjPtr, regAddr , &regValue);

                ipAddress[((3 - jj) * 4)    ] = SMEM_U32_GET_FIELD(regValue,24,8);
                ipAddress[((3 - jj) * 4) + 1] = SMEM_U32_GET_FIELD(regValue,16,8);
                ipAddress[((3 - jj) * 4) + 2] = SMEM_U32_GET_FIELD(regValue,8 ,8);
                ipAddress[((3 - jj) * 4) + 3] = SMEM_U32_GET_FIELD(regValue,0 ,8);

                if(prefixLength >= 32)
                {
                    /* Prefix length more than 32 bits - fill in the hole 32 bit word[jj] by 0xFFFFFFFF */
                    ipMask[(jj * 4)    ] = 0xFF;
                    ipMask[(jj * 4) + 1] = 0xFF;
                    ipMask[(jj * 4) + 2] = 0xFF;
                    ipMask[(jj * 4) + 3] = 0xFF;
                    prefixLength = prefixLength - 32;
                }
                else
                {
                    if (prefixLength != 0)
                    {
                        /* Align mask to be LSB left most bits in word[jj] */
                        mask = SMEM_BIT_MASK(prefixLength) << (32 - prefixLength);
                        ipMask[(jj * 4)    ] = SMEM_U32_GET_FIELD(mask,24,8);
                        ipMask[(jj * 4) + 1] = SMEM_U32_GET_FIELD(mask,16,8);
                        ipMask[(jj * 4) + 2] = SMEM_U32_GET_FIELD(mask,8 ,8);
                        ipMask[(jj * 4) + 3] = SMEM_U32_GET_FIELD(mask,0 ,8);
                        prefixLength = 0;
                    }
                }
            }
        }
        else
        {
            regAddr = SMEM_PIPE_PCP_IP2ME_IPADDR_REG(devObjPtr, ii, 3);
            smemRegGet(devObjPtr, regAddr , &regValue);
            ipAddress[12 + 0] = SMEM_U32_GET_FIELD(regValue,24,8);
            ipAddress[12 + 1] = SMEM_U32_GET_FIELD(regValue,16,8);
            ipAddress[12 + 2] = SMEM_U32_GET_FIELD(regValue,8 ,8);
            ipAddress[12 + 3] = SMEM_U32_GET_FIELD(regValue,0 ,8);

            /* Align mask to be LSB left most bits in word */
            mask = SMEM_BIT_MASK(prefixLength) << (32 - prefixLength);
            ipMask[12 + 0] = SMEM_U32_GET_FIELD(mask,24,8);
            ipMask[12 + 1] = SMEM_U32_GET_FIELD(mask,16,8);
            ipMask[12 + 2] = SMEM_U32_GET_FIELD(mask,8 ,8);
            ipMask[12 + 3] = SMEM_U32_GET_FIELD(mask,0 ,8);
        }

        matched = GT_TRUE;
        for(jj = compareOffset; jj < 16; jj++)
        {
            if((ipAddress[jj] & ipMask[jj]) !=
                (dipOffsetPtr[jj - compareOffset] & ipMask[jj]))
            {
                __LOG(("IP2ME entry[%d] skipped because : at byte index [%d] "
                       "mask[0x%2.2x] , pattern[0x%2.2x] and packet[0x%2.2x] not match.\n",
                        ii,jj,
                        ipMask[jj],
                        ipAddress[jj],
                        dipOffsetPtr[jj - compareOffset]));
                matched = GT_FALSE;
                break;
            }
        }
        if(GT_TRUE == matched)
        {
            pipe_descrPtr->pipe_ip2meIdx = ii + 1;
            __LOG(("Found IP2ME match at entry [%d].\n", ii));
            return;
        }
    }
    __LOG(("No IP2ME matches found.\n"));
}

/**
* @internal pipeIngressPacketTypeKey function
* @endinternal
*
* @brief   Packet Type Key Calculation (set pipe_descrPtr->pipe_packetTypeKey[6..16])
*/
static GT_VOID pipeIngressPacketTypeKey
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressPacketTypeKey);
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32 regAddress ,regValue;
    GT_U32 UDBP_Anchor_Type[4];
    GT_U32 UDBP_Byte_Offset[4];
    GT_U32 Src_Port_Profile;
    GT_U32 byteIndex;
    GT_U32 llcNonSnap;
    GT_U8  *startFramePtr = cheetah_descrPtr->startFramePtr;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8   *l3StartOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset;
    GT_U8   *udbOffsetPtr;
    GT_U32  ii;

    regAddress = SMEM_PIPE_PCP_PORT_PACKET_TYPE_KEY_TABLE_REG(devObjPtr,pipe_descrPtr->pipe_SrcPort);
    smemRegGet(devObjPtr, regAddress , &regValue);

    UDBP_Byte_Offset[2] = SMEM_U32_GET_FIELD(regValue,22,6);
    UDBP_Anchor_Type[2] = SMEM_U32_GET_FIELD(regValue,21,1);
    UDBP_Byte_Offset[1] = SMEM_U32_GET_FIELD(regValue,15,6);
    UDBP_Anchor_Type[1] = SMEM_U32_GET_FIELD(regValue,14,1);
    UDBP_Byte_Offset[0] = SMEM_U32_GET_FIELD(regValue, 8,6);
    UDBP_Anchor_Type[0] = SMEM_U32_GET_FIELD(regValue, 7,1);
    Src_Port_Profile    = SMEM_U32_GET_FIELD(regValue, 0,7);

    regAddress = SMEM_PIPE_PCP_PORT_PACKET_TYPE_KEY_TABLE_EXT_REG(devObjPtr,pipe_descrPtr->pipe_SrcPort);
    smemRegGet(devObjPtr, regAddress , &regValue);

    UDBP_Byte_Offset[3] = SMEM_U32_GET_FIELD(regValue, 1,6);
    UDBP_Anchor_Type[3] = SMEM_U32_GET_FIELD(regValue, 0,1);

    /* Packet Type Key is the concatenation of the following 17 bytes:
        6 bytes extracted from packet MAC DA.
        2 bytes extracted from packet outermost EtherType/LEN1.
        1 byte of :
            1 bit indicating if packet is LLC-non-SNAP
            7 bits from configuration Port<src port profile>.
        8 bytes (4 byte-pairs) extracted from the packet header.
        per port configuration defines 4 User Defined Byte-Pairs (UDBP), where each UDBP is defined by {Anchor Type, Byte Offset}.
    */

    /* NOTE: the MACDA already set into pipe_descrPtr->pipe_packetTypeKey[0..5] */
    byteIndex = 6;
    pipe_descrPtr->pipe_packetTypeKey[byteIndex++] = (GT_U8)(pipe_descrPtr->pipe_etherTypeOrSsapDsap_outerMost >> 8);
    pipe_descrPtr->pipe_packetTypeKey[byteIndex++] = (GT_U8)(pipe_descrPtr->pipe_etherTypeOrSsapDsap_outerMost >> 0);

    llcNonSnap = (pipe_descrPtr->pipe_l2Encaps == SKERNEL_LLC_E) ? 1 : 0/* non LLC or non-snap */;

    pipe_descrPtr->pipe_packetTypeKey[byteIndex++] = (GT_U8)((llcNonSnap << 7) | Src_Port_Profile);

    /* handle the 4 UDBPs */
    for(ii = 0 ; ii < 4 ; ii++)
    {
        udbOffsetPtr = (UDBP_Anchor_Type[ii] == 1) ? /*L3 Anchor*/l3StartOffsetPtr : startFramePtr;

        pipe_descrPtr->pipe_packetTypeKey[byteIndex++] = udbOffsetPtr[UDBP_Byte_Offset[ii]+0];
        pipe_descrPtr->pipe_packetTypeKey[byteIndex++] = udbOffsetPtr[UDBP_Byte_Offset[ii]+1];
    }

    /* IP2ME index */
    if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
    {
        pipe_descrPtr->pipe_packetTypeKey[byteIndex++] =
            pipe_descrPtr->pipe_ip2meIdx;
    }

    __LOG(("Packet Type Fields:\n"));
    __LOG(("<MAC DA> [0x%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] \n",
           pipe_descrPtr->pipe_packetTypeKey[0],
           pipe_descrPtr->pipe_packetTypeKey[1],
           pipe_descrPtr->pipe_packetTypeKey[2],
           pipe_descrPtr->pipe_packetTypeKey[3],
           pipe_descrPtr->pipe_packetTypeKey[4],
           pipe_descrPtr->pipe_packetTypeKey[5]));
    __LOG(("<EtherType> [0x%4.4x] \n", pipe_descrPtr->pipe_etherTypeOrSsapDsap_outerMost));
    __LOG(("<Is LLC-non-SNAP> [0x%x] \n", llcNonSnap));
    __LOG(("From Port configuration: \n"));
    __LOG(("<Src port profile> [0x%x] \n", Src_Port_Profile));
      for(ii = 0 ; ii < 4 ; ii++)
      {
        __LOG(("<UDBP%d> Type[0x%x], Offset[0x%x] \n", ii, UDBP_Anchor_Type[ii],  UDBP_Byte_Offset[ii]));
      }

    if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
    {
        __LOG(("<IP2ME index> [0x%x] \n",  pipe_descrPtr->pipe_ip2meIdx));
    }
    __LOG(("dump the bytes packet Type Key : (network order)"));
    simLogPacketDump(devObjPtr,GT_TRUE/*ingress*/,
                pipe_descrPtr->pipe_SrcPort,/* local device source port */
                pipe_descrPtr->pipe_packetTypeKey,/*the 'packet Type Key' */
                byteIndex);/*17*/
}

/**
* @internal pipePtpPacketTypeKey function
* @endinternal
*
* @brief   PTP Type Key Calculation (set pipe_descrPtr->pipe_PTPTypeKey[0..10])
*/
static GT_VOID pipePtpPacketTypeKey
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipePtpPacketTypeKey);
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32 regAddress, regValue;
    GT_U32 UDBP_Anchor_Type[2];
    GT_U32 UDBP_Byte_Offset[2];
    GT_U32 UDB_Anchor_Type[6];
    GT_U32 UDB_Byte_Offset[6];
    GT_U32 Src_Port_Profile;
    GT_U32 byteIndex;
    GT_U32 isUdp;
    GT_U8  *startFramePtr = cheetah_descrPtr->startFramePtr;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8   *l3StartOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset;
    GT_U8   *l4StartOffsetPtr = l3StartOffsetPtr + pipe_descrPtr->pipe_L4Offset;
    GT_U8   *ptpOffset = l4StartOffsetPtr + 8;
    GT_U8   *udbOffsetPtr;
    GT_U32  ii;

    for (ii = 0; ii < 2; ii++)
    {
        regAddress = SMEM_PIPE_PCP_PORT_PTP_TYPE_UDBP_CONFIG_REG(devObjPtr, ii, pipe_descrPtr->pipe_SrcPort);
        smemRegGet(devObjPtr, regAddress, &regValue);

        UDBP_Byte_Offset[ii] = SMEM_U32_GET_FIELD(regValue, 2, 7);
        UDBP_Anchor_Type[ii] = SMEM_U32_GET_FIELD(regValue, 0, 2);
    }

    for (ii = 0; ii < 6; ii++)
    {
        regAddress = SMEM_PIPE_PCP_PORT_PTP_TYPE_UDB_CONFIG_REG(devObjPtr, ii, pipe_descrPtr->pipe_SrcPort); 
        smemRegGet(devObjPtr, regAddress, &regValue);

        UDB_Byte_Offset[ii] = SMEM_U32_GET_FIELD(regValue, 2, 7);
        UDB_Anchor_Type[ii] = SMEM_U32_GET_FIELD(regValue, 0, 2);
    }

    /*Packet Type Key is the concatenation of the following 11 bytes:
        10 bytes (2 byte-pairs, 6 bytes) extracted from the packet header
         7 bits from configuration Port<src port profile>.
         1 bit indicating if packet is UDP.
         Per port configuration defines the following 8 user-defined fields:
            2 User Defined Byte-Pairs (UDBP) where each UDBP is defined by {Anchor Type, Byte Offset}
            6 User Defined Bytes (UDB) */
    byteIndex = 0;

    /* handle the 2 UDBPs */
    for(ii = 0; ii < 2; ii++)
    {
        switch (UDBP_Anchor_Type[ii])
        {
            case 0:
                udbOffsetPtr = /*L2 Anchor*/ startFramePtr;
                break;
            case 1:
                udbOffsetPtr = /*L3 Anchor*/l3StartOffsetPtr;
                break;
            case 2:
                udbOffsetPtr = /*L4 Anchor*/l4StartOffsetPtr;
                break;
            default:
                udbOffsetPtr = ptpOffset;
        }

        pipe_descrPtr->pipe_PTPTypeKey[byteIndex++] = udbOffsetPtr[UDBP_Byte_Offset[ii]+0];
        pipe_descrPtr->pipe_PTPTypeKey[byteIndex++] = udbOffsetPtr[UDBP_Byte_Offset[ii]+1];
    }

    /* handle the 6 UDBs */
    for(ii = 0; ii < 6; ii++)
    {
        switch (UDB_Anchor_Type[ii])
        {
            case 0:
                udbOffsetPtr = /*L2 Anchor*/ startFramePtr;
                break;
            case 1:
                udbOffsetPtr = /*L3 Anchor*/l3StartOffsetPtr;
                break;
            case 2:
                udbOffsetPtr = /*L4 Anchor*/l4StartOffsetPtr;
                break;
            default:
                udbOffsetPtr = ptpOffset;
        }

        pipe_descrPtr->pipe_PTPTypeKey[byteIndex++] = udbOffsetPtr[UDB_Byte_Offset[ii]];

    }

    regAddress = SMEM_PIPE_PCP_PORT_PACKET_TYPE_KEY_TABLE_REG(devObjPtr, pipe_descrPtr->pipe_SrcPort);
    smemRegGet(devObjPtr, regAddress , &regValue);

    Src_Port_Profile    = SMEM_U32_GET_FIELD(regValue, 0, 7);
    isUdp = (pipe_descrPtr->pipe_IP_header_Protocol == SNET_UDP_PROT_E) ? 1 : 0;

    pipe_descrPtr->pipe_PTPTypeKey[byteIndex++] = (GT_U8)((isUdp << 7) | Src_Port_Profile);

    __LOG(("PTP Type Fields:\n"));
    __LOG(("<Is UDP> [0x%x] \n", isUdp));
    __LOG(("<Src port profile> [0x%x] \n", Src_Port_Profile));
    __LOG(("From Port configuration: \n"));
      for(ii = 0; ii < 2 ; ii++)
      {
        __LOG(("<UDBP%d> Type[0x%x], Offset[0x%x] \n", ii, UDBP_Anchor_Type[ii],  UDBP_Byte_Offset[ii]));
      }
      for(ii = 0; ii < 6 ; ii++)
      {
        __LOG(("<UDB%d> Type[0x%x], Offset[0x%x] \n", ii, UDB_Anchor_Type[ii],  UDB_Byte_Offset[ii]));
      }

    __LOG(("dump the bytes PTP Type Key : (network order)"));
    simLogPacketDump(devObjPtr,GT_TRUE/*ingress*/,
                pipe_descrPtr->pipe_SrcPort,/* local device source port */
                pipe_descrPtr->pipe_PTPTypeKey,/*the 'PTP Type Key' */
                byteIndex);/*17*/
}

/**
* @internal compareKeyToPatternMask function
* @endinternal
*
* @brief   compare the key to the pattern mask
*/
static GT_BOOL compareKeyToPatternMask
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   tableIndex,
    IN GT_U8    *keyPtr,
    IN TCAM_KEY_METADATA_STC keyMetadataArr[],
    IN GT_U8    *patternPtr,
    IN GT_U8    *maskPtr
)
{
    DECLARE_FUNC_NAME(compareKeyToPatternMask);

    GT_U32  ii,jj,kk;
    GT_U32  byteIndex;
    GT_U32  fieldBitIndexForLog;

    byteIndex = 0;

    for(ii = 0 ; keyMetadataArr[ii].fieldName != NULL ;ii++)
    {
        for(jj = 0 ; jj < keyMetadataArr[ii].numBytes ; jj++ , byteIndex++)
        {
            if(BYTE_P_M_K_FIND_NO_MATCH(patternPtr[byteIndex],
                                        maskPtr[byteIndex],
                                        keyPtr[byteIndex]))
            {
                if(simLogIsOpenFlag)
                {
                    __LOG(("NOTE: the 'no match' at index [%d] caused at field[%s] in byte[%d] out of [%d] bytes \n",
                        tableIndex,
                        keyMetadataArr[ii].fieldName,
                        jj,
                        keyMetadataArr[ii].numBytes));

                    __LOG(("-- : key[0x%2.2x] , data[0x%2.2x] , mask[0x%2.2x] \n",
                        keyPtr[byteIndex],
                        patternPtr[byteIndex] ,
                        maskPtr[byteIndex]));

                    __LOG(("Bits:"));
                    fieldBitIndexForLog = 8 * jj;
                    for(kk = 0 ; kk < 8 ; kk++)
                    {
                        if(BIT_IN_BYTE_P_M_K_FIND_NO_MATCH(patternPtr[byteIndex],
                                                            maskPtr[byteIndex],
                                                            keyPtr[byteIndex],
                                                            kk))
                        {
                            /* this bit caused no match */
                            __LOG_NO_LOCATION_META_DATA(("%d,",
                                (kk + fieldBitIndexForLog)));
                        }
                    }
                    __LOG(("\n"));
                }

                return GT_FALSE;
            }
        }
    }

    return GT_TRUE;
}

/**
* @internal pipeIngressSearchPacketTypeTable function
* @endinternal
*
* @brief   Search Packet Type Table (set pipe_descrPtr->pipe_PktTypeIdx)
*/
static GT_VOID pipeIngressSearchPacketTypeTable
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressSearchPacketTypeTable);

    GT_U32  regAddress,regValue;
    GT_U32  entryIndexValidBmp;
    GT_U32  ii;
    GT_U8   patternArr[18];/* pattern array */
    GT_U8   maskArr[18]; /* mask array */
    GT_U32  pId;/* pattern Index */
    GT_U32  mId;/* mask Index */
    GT_U32  llcNonSnap,srcPortProfile;
    GT_U32  udbpId;/* udbp index (0..3)*/
    GT_BOOL matchFound;
    TCAM_KEY_METADATA_STC keyMetadataArr[]=
    {
         {"macDA",6}
        ,{"outerMost etherType",2}
        ,{"nonLlcAndSrcPortProfile",1}
        ,{"UDBP0",2}
        ,{"UDBP1",2}
        ,{"UDBP2",2}
        ,{"UDBP3",2}
        ,{NULL,0}
        ,{NULL,0}
    };


    /*
        The Packet Type Key is used to perform a TCAM-like search for a matching key in the Packet Type
        Table.
        'TCAM-like search' table contains 32 entries x 34B comprised of 17B Data + 17B Mask.
        The first matching entry index is the assigned Packet Type Index.
        A matching entry should always be present in the table.
        If a match is not found due to a configuration error, the following actions are performed:
         - An exception interrupt is generated.
         - The key that was not matched is latched in a registered form for later examination by the CPU.
         - An Unknown Packet Type counter is incremented.
         - The packet is dropped.
    */

    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_ENTRIES_ENABLE_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    entryIndexValidBmp = regValue;
    __LOG_PARAM(entryIndexValidBmp);

    for(ii = 0 ; ii < 32; ii++)
    {
        if(0 == (entryIndexValidBmp & (1<<ii)))
        {
            __LOG(("Entry index[%d] is not valid \n",
                ii));

            continue;
        }

        pId = 0;
        mId = 0;

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_4MSB_ENTRY_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        patternArr[pId++] = (GT_U8)(regValue >> 24);
        patternArr[pId++] = (GT_U8)(regValue >> 16);
        patternArr[pId++] = (GT_U8)(regValue >>  8);
        patternArr[pId++] = (GT_U8)(regValue >>  0);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_4MSB_ENTRY_MASK_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        maskArr[mId++] = (GT_U8)(regValue >> 24);
        maskArr[mId++] = (GT_U8)(regValue >> 16);
        maskArr[mId++] = (GT_U8)(regValue >>  8);
        maskArr[mId++] = (GT_U8)(regValue >>  0);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_2LSB_ENTRY_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        patternArr[pId++] = (GT_U8)(regValue >>  8);
        patternArr[pId++] = (GT_U8)(regValue >>  0);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_MACDA_2LSB_ENTRY_MASK_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        maskArr[mId++] = (GT_U8)(regValue >>  8);
        maskArr[mId++] = (GT_U8)(regValue >>  0);


        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_TPID_ETHER_TYPE_ENTRY_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        patternArr[pId++] = (GT_U8)(regValue >>  8);
        patternArr[pId++] = (GT_U8)(regValue >>  0);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_TPID_ETHER_TYPE_ENTRY_MASK_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        maskArr[mId++] = (GT_U8)(regValue >>  8);
        maskArr[mId++] = (GT_U8)(regValue >>  0);


        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IS_LLC_NON_SNAP_ENTRY_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        llcNonSnap = SMEM_U32_GET_FIELD(regValue,0,1);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_SRC_PORT_PROFILE_ENTRY_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        srcPortProfile = SMEM_U32_GET_FIELD(regValue,0,7);

        patternArr[pId++] = (GT_U8)((llcNonSnap) << 7 | srcPortProfile);


        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IS_LLC_NON_SNAP_ENTRY_MASK_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        llcNonSnap = SMEM_U32_GET_FIELD(regValue,0,1);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_SRC_PORT_PROFILE_ENTRY_MASK_REG(devObjPtr,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        srcPortProfile = SMEM_U32_GET_FIELD(regValue,0,7);

        maskArr[mId++] = (GT_U8)((llcNonSnap) << 7 | srcPortProfile);


        for(udbpId = 0 ;udbpId < 4; udbpId++)
        {
            regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_UDBP_ENTRY_REG(devObjPtr,ii,udbpId);
            smemRegGet(devObjPtr, regAddress , &regValue);

            patternArr[pId++] = (GT_U8)(regValue >>  8);
            patternArr[pId++] = (GT_U8)(regValue >>  0);

            regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_UDBP_ENTRY_MASK_REG(devObjPtr,ii,udbpId);
            smemRegGet(devObjPtr, regAddress , &regValue);

            maskArr[mId++] = (GT_U8)(regValue >>  8);
            maskArr[mId++] = (GT_U8)(regValue >>  0);
        }

        if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
        {
            regAddress =  SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IP2ME_IDX_ENTRY_REG(devObjPtr,ii);
            smemRegGet(devObjPtr, regAddress , &regValue);
            patternArr[pId++] = (GT_U8)(SMEM_U32_GET_FIELD(regValue,0,3));

            regAddress =  SMEM_PIPE_PCP_PACKET_TYPE_KEY_TABLE_IP2ME_IDX_ENTRY_MASK_REG(devObjPtr,ii);
            smemRegGet(devObjPtr, regAddress , &regValue);
            maskArr[mId++] = (GT_U8)(SMEM_U32_GET_FIELD(regValue,0,3));

            keyMetadataArr[7].fieldName ="ip2meIdx";
            keyMetadataArr[7].numBytes = 1;
        }

        /* compare the key to the pattern mask */
        matchFound = compareKeyToPatternMask(devObjPtr,ii,pipe_descrPtr->pipe_packetTypeKey,keyMetadataArr,patternArr,maskArr);
        if(matchFound == GT_TRUE)
        {
            __LOG(("Entry index[%d] was matched \n",
                ii));
            pipe_descrPtr->pipe_PktTypeIdx = ii;
            __LOG_PARAM(pipe_descrPtr->pipe_PktTypeIdx);
            return;
        }
        __LOG(("Entry index[%d] in valid but NOT matched \n",
            ii));
    }

    __LOG(("WARNING : the key was not matched --> this considered as 'Configuration ERROR' (packet will be dropped)\n"));
    pipe_descrPtr->pipe_errorOccurred_BMP |= PIPE_ERROR_PACKET_TYPE_KEY_SEARCH_NO_MATCH_BIT;

    /*
        If a match is not found due to a configuration error, the following actions are performed:
         - An exception interrupt is generated.
         - The key that was not matched is latched in a registered form for later examination by the CPU.
         - An Unknown Packet Type counter is incremented.
         - The packet is dropped.
    */


    scibAccessLock();

    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_REG(devObjPtr);
    smemRegFldSet(devObjPtr, regAddress , 0,1, 1);

    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_LOOK_UP_MISS_COUNTER_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);
    regValue++;
    smemRegSet(devObjPtr, regAddress , regValue);

    regValue = (pipe_descrPtr->pipe_packetTypeKey[0] << 24) |
               (pipe_descrPtr->pipe_packetTypeKey[1] << 16) |
               (pipe_descrPtr->pipe_packetTypeKey[2] <<  8) |
               (pipe_descrPtr->pipe_packetTypeKey[3] <<  0) ;


    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_MACDA_4MSB_MISS_VALUE_REG(devObjPtr);
    smemRegSet(devObjPtr, regAddress , regValue);

    regValue = (pipe_descrPtr->pipe_packetTypeKey[4] <<  8) |
               (pipe_descrPtr->pipe_packetTypeKey[5] <<  0) ;


    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_MACDA_2LSB_MISS_VALUE_REG(devObjPtr);
    smemRegSet(devObjPtr, regAddress , regValue);


    regValue = (pipe_descrPtr->pipe_packetTypeKey[6] <<  8) |
               (pipe_descrPtr->pipe_packetTypeKey[7] <<  0) ;
    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_TPID_ETHERTYPE_MISS_VALUE_REG(devObjPtr);
    smemRegSet(devObjPtr, regAddress , regValue);


    regValue = (pipe_descrPtr->pipe_packetTypeKey[8] >> 7) ;
    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_IS_LLC_NON_SNAP_MISS_VALUE_REG(devObjPtr);
    smemRegSet(devObjPtr, regAddress , regValue);

    regValue = (pipe_descrPtr->pipe_packetTypeKey[8] & 0x7f) ;
    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_SRC_PORT_PROFILE_MISS_VALUE_REG(devObjPtr);
    smemRegSet(devObjPtr, regAddress , regValue);


    for(udbpId = 0 ;udbpId < 4; udbpId++)
    {
        regValue = (pipe_descrPtr->pipe_packetTypeKey[9+(udbpId+2)+0] << 8) |
                   (pipe_descrPtr->pipe_packetTypeKey[9+(udbpId+2)+1] << 0) ;

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_KEY_UDBP_MISS_VALUE_REG(devObjPtr,udbpId);
        smemRegSet(devObjPtr, regAddress , regValue);
    }

    /* An exception interrupt is generated (after data is ready in the registers) */
    /* PCP_Packet_Type_Key_Look_Up_Miss */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_PIPE_PCP_INTERRUPTS_CAUSE_REG(devObjPtr),
                          SMEM_PIPE_PCP_INTERRUPTS_MASK_REG(devObjPtr),
                          1 << PCP_Packet_Type_Key_Look_Up_Miss,
                          0/*not used*/);



    scibAccessUnlock();

}

/**
* @internal pipeIngressSearchPtpTypeTable function
* @endinternal
*
* @brief   Search PTP Type Table (set pipe_descrPtr->pipe_PTPTypeIdx)
*/
static GT_VOID pipeIngressSearchPtpTypeTable
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressSearchPtpTypeTable);

    GT_U32  regAddress,regValue;
    GT_U32  entryIndexValidBmp;
    GT_U32  ii;
    GT_U8   patternArr[11];/* pattern array */
    GT_U8   maskArr[11]; /* mask array */
    GT_U32  pId;/* pattern Index */
    GT_U32  mId;/* mask Index */
    GT_U32  isUdp, srcPortProfile;
    GT_U32  udbpId;/* udbp index (0..2)*/
    GT_U32  udbId; /* udb  index (2..7)*/
    GT_BOOL matchFound;
    TCAM_KEY_METADATA_STC keyMetadataArr[]=
    {
        {"UDBP0",2}
       ,{"UDBP1",2}
       ,{"UDB2", 1}
       ,{"UDB3", 1}
       ,{"UDB4", 1}
       ,{"UDB5", 1}
       ,{"UDB6", 1}
       ,{"UDB7", 1}
       ,{"isUdpSrcPortProfile",1}
       ,{NULL,0}
    };

    /*
        The PTP Type Key is used to perform a TCAM-like search for a matching key in the PTP Type
        Table.
        'TCAM-like search' table contains 32 entries x 22B comprised of 11B Data + 11B Mask.
        If there is a match in the PTP Type Table and the packet is not LLC-non-SNAP, then the following
        actions occur:
        - the <Is PTP> indication is set.
        - the first matching table entry is selected, and assigned in <PTP Type Index> indication.
        If there is no match in the PTP Type Table or the packet is LLC-non-SNAP, then the <Is PTP>
        indication is cleared.
    */

    if (pipe_descrPtr->pipe_l2Encaps == SKERNEL_LLC_SNAP_E)
    {
        __LOG(("WARNING : The packet is LLC Snap - No PTP key lookup\n"));
        pipe_descrPtr->pipe_IsPTP = GT_FALSE;
    }

    regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_TABLE_ENTRIES_ENABLE_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    entryIndexValidBmp = regValue;
    __LOG_PARAM(entryIndexValidBmp);

    for(ii = 0; ii < 32; ii++)
    {
        if(0 == (entryIndexValidBmp & (1<<ii)))
        {
            __LOG(("Entry index[%d] is not valid \n",
                ii));

            continue;
        }

        pId = 0;
        mId = 0;

        for(udbpId = 0; udbpId < 2; udbpId++)
        {
            regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_UDBP_TABLE_REG(devObjPtr, udbpId, ii);
            smemRegGet(devObjPtr, regAddress , &regValue);

            patternArr[pId++] = (GT_U8)(regValue >>  8);
            patternArr[pId++] = (GT_U8)(regValue >>  0);

            regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_UDBP_MASK_TABLE_REG(devObjPtr, udbpId, ii);
            smemRegGet(devObjPtr, regAddress , &regValue);

            maskArr[mId++] = (GT_U8)(regValue >>  8);
            maskArr[mId++] = (GT_U8)(regValue >>  0);
        }

        for(udbId = 0; udbId < 6; udbId++)
        {
            regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_TABLE_REG(devObjPtr, udbId, ii);
            smemRegGet(devObjPtr, regAddress , &regValue);

            patternArr[pId++] = (GT_U8)(regValue);

            regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_MASK_TABLE_REG(devObjPtr, udbId, ii);
            smemRegGet(devObjPtr, regAddress , &regValue);

            maskArr[mId++] = (GT_U8)(regValue);
        }

        regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_PORT_PROFILE_TABLE_REG(devObjPtr, ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        srcPortProfile = SMEM_U32_GET_FIELD(regValue, 0, 7);
        isUdp          = SMEM_U32_GET_FIELD(regValue, 7, 1);

        patternArr[pId++] = (GT_U8)((isUdp) << 7 | srcPortProfile);

        regAddress = SMEM_PIPE_PCP_PTP_TYPE_KEY_UDB_PORT_PROFILE_MASK_TABLE_REG(devObjPtr, ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        srcPortProfile = SMEM_U32_GET_FIELD(regValue, 0, 7);
        isUdp          = SMEM_U32_GET_FIELD(regValue, 7, 1);

        maskArr[mId++] = (GT_U8)((isUdp) << 7 | srcPortProfile);

        /* compare the key to the pattern mask */
        matchFound = compareKeyToPatternMask(devObjPtr, ii, pipe_descrPtr->pipe_PTPTypeKey, keyMetadataArr, patternArr, maskArr);
        if(matchFound == GT_TRUE)
        {
            __LOG(("Entry index[%d] was matched \n",
                ii));
            pipe_descrPtr->pipe_PTPTypeIdx = ii;
            pipe_descrPtr->pipe_IsPTP = GT_TRUE;
            pipe_descrPtr->pipe_PTPOffset = 2;
            if (pipe_descrPtr->pipe_L4Offset)
            {
                pipe_descrPtr->pipe_PTPOffset += pipe_descrPtr->pipe_L4Offset + 8;  /* abridged logic from VERIFIER */
            }
            __LOG_PARAM(pipe_descrPtr->pipe_PTPTypeIdx);
            __LOG_PARAM(pipe_descrPtr->pipe_IsPTP);
            __LOG_PARAM(pipe_descrPtr->pipe_PTPOffset);
            return;
        }
        __LOG(("Entry index[%d] in valid but NOT matched \n",
            ii));
    }

    __LOG(("WARNING : the key was not matched\n"));
    pipe_descrPtr->pipe_IsPTP = GT_FALSE;
    pipe_descrPtr->pipe_errorOccurred_BMP |= PIPE_ERROR_PACKET_TYPE_KEY_SEARCH_NO_MATCH_BIT;
}

/**
* @internal pipeIngressCalculateDestinationIndex function
* @endinternal
*
* @brief   Calculate Destination Index and get <pipe_dstPortsBmp>
*/
static GT_VOID pipeIngressCalculateDestinationIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculateDestinationIndex);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  ii;
    GT_U32  regAddress,regValue;
    GT_U32  PTD_Dst_Idx_Constant ,PTD_Max_Dst_Idx , Dst_Exeption_PortMap;
    GT_U32  PTD_Bit_Field_Byte_Offset[4];
    GT_U32  PTD_Bit_Field_Start_Bit[4];
    GT_U32  PTD_Bit_Field_Number_Bits[4];
    GT_U32  fieldValue,byteValue;
    GT_U32  startBit , numBits, fieldOffset;
    GT_U32  dst_index;
    GT_U32  bypassLagDesignatedPortmap;

    regAddress = SMEM_PIPE_PCP_PKT_TYPE_DESTINATION_FORMAT0_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);

    PTD_Dst_Idx_Constant = SMEM_U32_GET_FIELD(regValue,0,13);
    PTD_Max_Dst_Idx      = SMEM_U32_GET_FIELD(regValue,13,13);
    __LOG_PARAM(PTD_Dst_Idx_Constant);
    __LOG_PARAM(PTD_Max_Dst_Idx);


    regAddress = SMEM_PIPE_PCP_PKT_TYPE_DESTINATION_FORMAT1_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);
    for(ii = 0 ; ii < 2 ; ii++)
    {
        PTD_Bit_Field_Byte_Offset[ii] = SMEM_U32_GET_FIELD(regValue,0 + 13 * (ii%2),6);
        PTD_Bit_Field_Start_Bit[ii]   = SMEM_U32_GET_FIELD(regValue,6 + 13 * (ii%2),3);
        PTD_Bit_Field_Number_Bits[ii] = SMEM_U32_GET_FIELD(regValue,9 + 13 * (ii%2),4);
    }

    regAddress = SMEM_PIPE_PCP_PKT_TYPE_DESTINATION_FORMAT2_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);
    for(/*continue*/ ; ii < 4 ; ii++)
    {
        PTD_Bit_Field_Byte_Offset[ii] = SMEM_U32_GET_FIELD(regValue,0 + 13 * (ii%2),6);
        PTD_Bit_Field_Start_Bit[ii]   = SMEM_U32_GET_FIELD(regValue,6 + 13 * (ii%2),3);
        PTD_Bit_Field_Number_Bits[ii] = SMEM_U32_GET_FIELD(regValue,9 + 13 * (ii%2),4);
    }

    /*
        The Destination Index Dst_Idx is created by the concatenation of up to 4 bit-fields.
        Then add or subtract the signed value Dst_Idx_Constant.
        If any of the bit-fields are configured with <Number-bits>==0,
        the field is skipped in the calculation of Dst_Idx.
    */

    dst_index = 0;
    fieldOffset = 0;
    for(ii = 0 ; ii < 4 ; ii++)
    {
        /*The start bit in the byte specified by the Byte Offset*/
        startBit = PTD_Bit_Field_Start_Bit[ii];
        /*The number of bits to take starting from Start-Bit.
        - '0' means do not use this Bit-Field.
        - Up to 8 bits may be extracted. The Start-Bit + Number-Bits
        must not exceed 8. For example, the bit field must not cross
        an 8-bit byte boundary*/
        numBits = PTD_Bit_Field_Number_Bits[ii];

        if(numBits == 0)
        {
            continue;
        }

        /* NOTE: startBit in HW refers to REVERSE order of all bits in the BYTE !!! */
        /* so convert it to 'SIMULATION logic' */
        startBit = 8 - (numBits + startBit);

        if((startBit + numBits) > 8)
        {
            /* limit the number of bits to get */
            __LOG(("Warning : startBit[%d] + numBits[%d] are more than 8 (so using only[%d] bits) \n",
                startBit, numBits , (8 - startBit)));

            numBits = 8 - startBit;
        }

        /*Byte Offset relative to start of packet*/
        byteValue = cheetah_descrPtr->startFramePtr[PTD_Bit_Field_Byte_Offset[ii]];
        __LOG_PARAM(byteValue);

        fieldValue = SMEM_U32_GET_FIELD(byteValue,startBit,numBits);
        __LOG_PARAM(fieldValue);

        /* concatenate the bits from the new field */
        dst_index = (fieldValue << fieldOffset) | dst_index;
        __LOG_PARAM(dst_index);

        /* increment offset for next field */
        fieldOffset += numBits;
        __LOG_PARAM(fieldOffset);

    }

    __LOG(("dst_index[0x%x] from concatenation of the bits from the fields \n",
        dst_index));

    if(PTD_Dst_Idx_Constant & (BIT_12))
    {
        GT_U32  negative_PTD_Dst_Idx_Constant =  ((~PTD_Dst_Idx_Constant) & 0xfff) + 1;/* based on VERIFIER code */

        __LOG(("The <PTD_Dst_Idx_Constant> considered 'negative value' (- 0x%x)\n",
            negative_PTD_Dst_Idx_Constant));
        dst_index -= negative_PTD_Dst_Idx_Constant;
    }
    else
    {
        __LOG(("The <PTD_Dst_Idx_Constant> considered 'positive value' (+ 0x%x) \n",
            PTD_Dst_Idx_Constant));

        dst_index += PTD_Dst_Idx_Constant;
    }

    pipe_descrPtr->pipe_DstIdx = dst_index;
    __LOG_PARAM(pipe_descrPtr->pipe_DstIdx);

    __LOG(("final dst_index[0x%x] after 'add/subtract' 'const' to/from it \n",
        dst_index));

    /*
        If the computed Dst_Idx is greater than the entry <Max Dst_Idx>,
        the packet is assigned the Dst Exception Forwarding PortMap
        (by default set to the all zero PortMap, effectively dropping the packet).
        The counter Dst Idx Exception Counter is incremented.
    */

    if(pipe_descrPtr->pipe_DstIdx > PTD_Max_Dst_Idx)
    {
        __LOG(("Warning : dst_index[%d] > PTD_Max_Dst_Idx[%d] , so the packet is assigned the Dst Exception Forwarding PortMap \n",
            pipe_descrPtr->pipe_DstIdx , PTD_Max_Dst_Idx));

        pipe_descrPtr->pipe_errorOccurred_BMP |= PIPE_ERROR_DST_EXCEPTION_FORWARDING_PORTMAP_BIT;

        regAddress = SMEM_PIPE_PCP_DST_EXCEPTION_FORWARDING_PORT_MAP_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress , &regValue);
        Dst_Exeption_PortMap = SMEM_U32_GET_FIELD(regValue,0,17);
        __LOG_PARAM(Dst_Exeption_PortMap);

        /* increment counter */
        regAddress = SMEM_PIPE_PCP_DST_IDX_EXCEPTION_CNTR_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress , &regValue);
        regValue++;
        smemRegSet(devObjPtr, regAddress , regValue);

        pipe_descrPtr->pipe_dstPortsBmp = Dst_Exeption_PortMap;
        LOG_BMP_ARR("pipe_dstPortsBmp",&pipe_descrPtr->pipe_dstPortsBmp,17);

        return;
    }

    regAddress = SMEM_PIPE_PCP_DST_PORT_MAPTABLE_MEM(devObjPtr,pipe_descrPtr->pipe_DstIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);

    pipe_descrPtr->pipe_dstPortsBmp = SMEM_U32_GET_FIELD(regValue,0,17);

    if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
    {
        bypassLagDesignatedPortmap = SMEM_U32_GET_FIELD(regValue,17,1);
        if (bypassLagDesignatedPortmap)
        {
            __LOG(("Bypass LAG Designated Portmap \n"));
            pipe_descrPtr->pipe_lagDisableVector = 0x1FFFF;
            LOG_BMP_ARR("pipe_lagDisableVector",&pipe_descrPtr->pipe_lagDisableVector,17);
        }
    }

    LOG_BMP_ARR("pipe_dstPortsBmp",&pipe_descrPtr->pipe_dstPortsBmp,17);

}

/**
* @internal pipeIngressCalculatePortFilterIndex function
* @endinternal
*
* @brief   Calculate Port filter Index and get <pipe_portFilterBmp>
*/
static GT_VOID pipeIngressCalculatePortFilterIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculatePortFilterIndex);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  ii;
    GT_U32  regAddress,regValue;
    GT_U32  PTS_PortFilter_Idx_Constant ,PTS_Max_PortFilter_Idx , PortFIlter_Exeption_PortMap;
    GT_U32  PTS_Bit_Field_Byte_Offset[4];
    GT_U32  PTS_Bit_Field_Start_Bit[4];
    GT_U32  PTS_Bit_Field_Number_Bits[4];
    GT_U32  fieldValue,byteValue;
    GT_U32  startBit , numBits, fieldOffset;
    GT_U32  port_filter_index;

    regAddress = SMEM_PIPE_PCP_PKT_TYPE_PORT_FILTER_FORMAT0_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);

    PTS_PortFilter_Idx_Constant = SMEM_U32_GET_FIELD(regValue,0,12);
    PTS_Max_PortFilter_Idx      = SMEM_U32_GET_FIELD(regValue,12,12);
    __LOG_PARAM(PTS_PortFilter_Idx_Constant);
    __LOG_PARAM(PTS_Max_PortFilter_Idx);


    regAddress = SMEM_PIPE_PCP_PKT_TYPE_PORT_FILTER_FORMAT1_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);
    for(ii = 0 ; ii < 2 ; ii++)
    {
        PTS_Bit_Field_Byte_Offset[ii] = SMEM_U32_GET_FIELD(regValue,0 + 13 * (ii%2),6);
        PTS_Bit_Field_Start_Bit[ii]   = SMEM_U32_GET_FIELD(regValue,6 + 13 * (ii%2),3);
        PTS_Bit_Field_Number_Bits[ii] = SMEM_U32_GET_FIELD(regValue,9 + 13 * (ii%2),4);
    }

    regAddress = SMEM_PIPE_PCP_PKT_TYPE_PORT_FILTER_FORMAT2_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);
    for(/*continue*/ ; ii < 4 ; ii++)
    {
        PTS_Bit_Field_Byte_Offset[ii] = SMEM_U32_GET_FIELD(regValue,0 + 13 * (ii%2),6);
        PTS_Bit_Field_Start_Bit[ii]   = SMEM_U32_GET_FIELD(regValue,6 + 13 * (ii%2),3);
        PTS_Bit_Field_Number_Bits[ii] = SMEM_U32_GET_FIELD(regValue,9 + 13 * (ii%2),4);
    }

    /*
        The resulting PortFilter_Idx is used for accessing the Src_Portmap Table to provide
        the source filtering portmap.
        This is required when the PIPE device performs the multicast replication, e.g. BR PE.
        If ANY of the Bit Fields is configured with <Number-bits>==0, this means the
        field is skipped in the calculation of PortFilter_Idx.
    */

    port_filter_index = 0;
    fieldOffset = 0;
    for(ii = 0 ; ii < 4 ; ii++)
    {
        /*The start bit in the byte specified by the Byte Offset*/
        startBit = PTS_Bit_Field_Start_Bit[ii];
        /*The number of bits to take starting from Start-Bit.
        - '0' means do not use this Bit-Field.
        - Up to 8 bits may be extracted. The Start-Bit + Number-Bits
        must not exceed 8. For example, the bit field must not cross
        an 8-bit byte boundary*/
        numBits = PTS_Bit_Field_Number_Bits[ii];

        if(numBits == 0)
        {
            continue;
        }

        /* NOTE: startBit in HW refers to REVERSE order of all bits in the BYTE !!! */
        /* so convert it to 'SIMULATION logic' */
        startBit = 8 - (numBits + startBit);

        if((startBit + numBits) > 8)
        {
            /* limit the number of bits to get */
            __LOG(("Warning : startBit[%d] + numBits[%d] are more than 8 (so using only[%d] bits) \n",
                startBit, numBits , (8 - startBit)));

            numBits = 8 - startBit;
        }

        /*Byte Offset relative to start of packet*/
        byteValue = cheetah_descrPtr->startFramePtr[PTS_Bit_Field_Byte_Offset[ii]];
        __LOG_PARAM(byteValue);

        fieldValue = SMEM_U32_GET_FIELD(byteValue,startBit,numBits);
        __LOG_PARAM(fieldValue);

        /* concatenate the bits from the new field */
        port_filter_index = (fieldValue << fieldOffset) | port_filter_index;
        __LOG_PARAM(port_filter_index);

        /* increment offset for next field */
        fieldOffset += numBits;
        __LOG_PARAM(fieldOffset);
    }

    __LOG(("port_filter_index[0x%x] from concatenation of the bits from the fields \n",
        port_filter_index));


    if(PTS_PortFilter_Idx_Constant & (BIT_11))
    {
        GT_U32  negative_PTS_PortFilter_Idx_Constant =  ((~PTS_PortFilter_Idx_Constant) & 0x7ff) + 1;

        __LOG(("The <PTS_PortFilter_Idx_Constant> considered 'negative value' (- 0x%x)\n",
            negative_PTS_PortFilter_Idx_Constant));
        port_filter_index -= negative_PTS_PortFilter_Idx_Constant;
    }
    else
    {
        __LOG(("The <PTS_PortFilter_Idx_Constant> considered 'positive value' (+ 0x%x) \n",
            PTS_PortFilter_Idx_Constant));

        port_filter_index += PTS_PortFilter_Idx_Constant;
    }

    __LOG(("final port_filter_index[0x%x] after 'add/subtract' 'const' to/from it \n",
        port_filter_index));

    pipe_descrPtr->pipe_PortFilterIdx = port_filter_index;

    __LOG_PARAM(pipe_descrPtr->pipe_PortFilterIdx);


    /*
        If the computed PortFilter_Idx is greater than the entry <Max PortFilter_Idx>,
        the packet is assigned the Src Exception Forwarding PortMap
        (by default set to the all zero PortMap, effectively dropping the packet)
        and the counter Port Filter Idx Exception Counter is incremented.
    */

    if(pipe_descrPtr->pipe_PortFilterIdx > PTS_Max_PortFilter_Idx)
    {
        __LOG(("Warning : portFilter_index[%d] > PTS_Max_PortFilter_Idx[%d] , so the packet is assigned the Src Exception Forwarding PortMap \n",
            pipe_descrPtr->pipe_PortFilterIdx , PTS_Max_PortFilter_Idx));

        pipe_descrPtr->pipe_errorOccurred_BMP |= PIPE_ERROR_PORT_FILTER_EXCEPTION_FORWARDING_PORTMAP_BIT;

        regAddress = SMEM_PIPE_PCP_PORT_FILTER_EXCEPTION_FORWARDING_PORT_MAP_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress , &regValue);
        PortFIlter_Exeption_PortMap = SMEM_U32_GET_FIELD(regValue,0,17);
        __LOG_PARAM(PortFIlter_Exeption_PortMap);

        /* increment counter */
        regAddress = SMEM_PIPE_PCP_PORT_FILTER_IDX_EXCEPTION_CNTR_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress , &regValue);
        regValue++;
        smemRegSet(devObjPtr, regAddress , regValue);

        pipe_descrPtr->pipe_portFilterBmp = PortFIlter_Exeption_PortMap;
        LOG_BMP_ARR("pipe_portFilterBmp",&pipe_descrPtr->pipe_portFilterBmp,17);

        return;
    }


    regAddress = SMEM_PIPE_PCP_PORT_FILTER_TABLE_MEM(devObjPtr,pipe_descrPtr->pipe_PortFilterIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);

    pipe_descrPtr->pipe_portFilterBmp = regValue;
    LOG_BMP_ARR("pipe_portFilterBmp",&pipe_descrPtr->pipe_portFilterBmp,17);

}

typedef enum{
    HASH_TYPE_IPV4_TCP_UDP_E = 0,
    HASH_TYPE_IPV6_TCP_UDP_E = 1,
    HASH_TYPE_IPV4_NONE_TCP_UDP_E = 2,
    HASH_TYPE_IPV6_NONE_TCP_UDP_E = 3,
    HASH_TYPE_MPLS_SINGLE_LABLE_E = 4,
    HASH_TYPE_MPLS_MORE_THAN_SINGLE_LABLE_E = 5,
    HASH_TYPE_UDE_1_E = 6,
    HASH_TYPE_UDE_2_E = 7,
    HASH_TYPE_ETHERNET_E = 8
}HASH_TYPE_ENT;

/**
* @internal pipeIngressCalculateHashKeyBuild function
* @endinternal
*
* @brief   build Hash key
*/
static GT_VOID pipeIngressCalculateHashKeyBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN HASH_TYPE_ENT    hashType
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculateHashKeyBuild);

    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32 regAddress ,regValue;
    GT_U8  *startFramePtr = cheetah_descrPtr->startFramePtr;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8   *l3StartOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset;
    GT_U8   *l4StartOffsetPtr = l3StartOffsetPtr + pipe_descrPtr->pipe_L4Offset;
    GT_U8   *udbOffsetPtr;
    GT_U32  byteMask,byteValue;

    GT_U32  ii;
    GT_U32  Hash_Conf_UDBP_Anchor_Type;
    GT_U32  Hash_Conf_UDBP_Byte_Offset;
    GT_U32  Hash_Conf_UDBP_Nibble_Mask;
    GT_U32 byteIndex = 0;
    GT_U32  Enable_Ingress_Port_in_Hash_Key;

    for(ii = 0 ; ii < 21 ; ii++)
    {
        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_HASH_CONFIG_UDBP_REG(devObjPtr,hashType,ii);
        smemRegGet(devObjPtr, regAddress , &regValue);
        if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
        {
            Hash_Conf_UDBP_Anchor_Type = SMEM_U32_GET_FIELD(regValue,0,2);
            Hash_Conf_UDBP_Byte_Offset = SMEM_U32_GET_FIELD(regValue,2,6);
            Hash_Conf_UDBP_Nibble_Mask = SMEM_U32_GET_FIELD(regValue,8,4);
        }
        else
        {
            Hash_Conf_UDBP_Anchor_Type = SMEM_U32_GET_FIELD(regValue,0,2);
            Hash_Conf_UDBP_Byte_Offset = SMEM_U32_GET_FIELD(regValue,2,4);
            Hash_Conf_UDBP_Nibble_Mask = SMEM_U32_GET_FIELD(regValue,6,4);
        }

        if(Hash_Conf_UDBP_Nibble_Mask == 0)
        {
            __LOG(("index[%d] with nibble mask of '0' that cause to ignore the 'anchor' and 'offset' \n",
                ii));

            pipe_descrPtr->pipe_hashKey[byteIndex++] = 0;
            pipe_descrPtr->pipe_hashKey[byteIndex++] = 0;

            continue;
        }

        __LOG(("index[%d] with [%s] Anchor offset[%d] (2 consecutive bytes) and Nibble mask [0x%4.4x] \n",
            byteIndex,
            ((Hash_Conf_UDBP_Anchor_Type == 1) ? "L3 " :
             (Hash_Conf_UDBP_Anchor_Type == 2) ? "L4 " :
             "L2 "),
             Hash_Conf_UDBP_Byte_Offset,
             Hash_Conf_UDBP_Nibble_Mask
            ));

        udbOffsetPtr = (Hash_Conf_UDBP_Anchor_Type == 1) ? /*L3 Anchor*/l3StartOffsetPtr :
                       (Hash_Conf_UDBP_Anchor_Type == 2) ? /*L4 Anchor*/l4StartOffsetPtr :
                        startFramePtr;

        if(Hash_Conf_UDBP_Anchor_Type == 2 && /*L4 Anchor*/
           pipe_descrPtr->pipe_L4Offset == 0)
        {
            __LOG(("A UDBP (index[%d]) with L4 Anchor is set to '0' because the L4 offset is not valid. \n",
                ii));

            pipe_descrPtr->pipe_hashKey[byteIndex++] = 0;
            pipe_descrPtr->pipe_hashKey[byteIndex++] = 0;

            continue;
        }

        byteMask = 0;
        if(Hash_Conf_UDBP_Nibble_Mask & (1<<3))
        {
            byteMask |= 0xF << 4;
        }
        if(Hash_Conf_UDBP_Nibble_Mask & (1<<2))
        {
            byteMask |= 0xF << 0;
        }

        byteValue = udbOffsetPtr[Hash_Conf_UDBP_Byte_Offset+1];

        __LOG(("byteIndex[%d] with byteMask  [0x%2.2x] byteValue [0x%2.2x]\n",
            byteIndex,byteMask,byteValue));

        pipe_descrPtr->pipe_hashKey[byteIndex++] = byteValue & byteMask;

        byteMask = 0;
        if(Hash_Conf_UDBP_Nibble_Mask & (1<<1))
        {
            byteMask |= 0xF << 4;
        }
        if(Hash_Conf_UDBP_Nibble_Mask & (1<<0))
        {
            byteMask |= 0xF << 0;
        }

        byteValue = udbOffsetPtr[Hash_Conf_UDBP_Byte_Offset+0];

        __LOG(("byteIndex[%d] with byteMask  [0x%2.2x] byteValue [0x%2.2x]\n",
            byteIndex,byteMask,byteValue));
        pipe_descrPtr->pipe_hashKey[byteIndex++] = byteValue & byteMask;
    }

    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_HASH_CONFIG_INGRESS_PORT_REG(devObjPtr,hashType);
    smemRegGet(devObjPtr, regAddress , &regValue);
    Enable_Ingress_Port_in_Hash_Key = SMEM_U32_GET_FIELD(regValue,0,1);
    __LOG_PARAM(Enable_Ingress_Port_in_Hash_Key);

    if(Enable_Ingress_Port_in_Hash_Key)
    {
        /* byteIndex is 42 */
        pipe_descrPtr->pipe_hashKey[byteIndex++] = pipe_descrPtr->pipe_SrcPort;
    }
    else
    {
        /* byteIndex is 42 */
        pipe_descrPtr->pipe_hashKey[byteIndex++] = 0;
    }

    /* byteIndex is 43 */

    __LOG(("dump the bytes of Hash Key : (network order)" ));
    simLogPacketDump(devObjPtr,GT_TRUE/*ingress*/,
                pipe_descrPtr->pipe_SrcPort,/* local device source port */
                pipe_descrPtr->pipe_hashKey,/*the 'packet Type Key' */
                byteIndex);/*17*/

}
#if 0 /* VERIFIER code */

void SipPcpCrc::crcInit() {

    crc  remainder;

    /*
     * Compute the remainder of each possible dividend.
     */
    for (int dividend = 0; dividend < 256; ++dividend)
    {
        /*
         * Start with the dividend followed by zeros.
         */
        remainder = dividend << (CRC_WIDTH - 8);

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & CRC_TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }

        /*
         * Store the result into the table.
         */
        crcTable[dividend] = remainder;
    }

}   /* crcInit() */


typedef uint32_t crc;
#define CRC_WIDTH  (8 * sizeof(crc))
#define CRC_TOPBIT (1 << (CRC_WIDTH - 1))
//#define POLYNOMIAL 0xD8  /* 11011 followed by 0's */
#define POLYNOMIAL 0x04C11DB7
crc SipPcpCrc::crcFast(clsData message , int nBytes, crc seed) {
    uint8_t data,byteMsg;
    crc remainder = seed;

    /*
     * Divide the message by the polynomial, a byte at a time.
     */
    for (int byte = 0; byte < nBytes; ++byte)
    {
//        byteMsg = (unsigned)objParser.Get(message, (byte)*8, (byte+1)*8-1);
        byteMsg = (unsigned)objParser.Get(message, (nBytes-1-byte)*8, (nBytes-byte)*8-1);
        data = byteMsg ^ (remainder >> (CRC_WIDTH - 8));
        remainder = crcTable[data] ^ (remainder << 8);
    }

    /*
     * The final remainder is the CRC.
     */
    return (remainder);

}   /* crcFast() */
#endif /*0*/

#define CRC_WIDTH  (8 * sizeof(GT_U32))
#define CRC_TOPBIT (1 << (CRC_WIDTH - 1))
#define POLYNOMIAL 0x04C11DB7

static GT_U32   pipe_crcTable[256];
#if 0 /* values printed by the INIT of GM PIPE device */
0x0             ,0x4c11db7       ,0x9823b6e       ,0xd4326d9       , 0x130476dc      ,0x17c56b6b      ,0x1a864db2      ,0x1e475005      ,
0x2608edb8      ,0x22c9f00f      ,0x2f8ad6d6      ,0x2b4bcb61      , 0x350c9b64      ,0x31cd86d3      ,0x3c8ea00a      ,0x384fbdbd      ,
0x4c11db70      ,0x48d0c6c7      ,0x4593e01e      ,0x4152fda9      , 0x5f15adac      ,0x5bd4b01b      ,0x569796c2      ,0x52568b75      ,
0x6a1936c8      ,0x6ed82b7f      ,0x639b0da6      ,0x675a1011      , 0x791d4014      ,0x7ddc5da3      ,0x709f7b7a      ,0x745e66cd      ,
0x9823b6e0      ,0x9ce2ab57      ,0x91a18d8e      ,0x95609039      , 0x8b27c03c      ,0x8fe6dd8b      ,0x82a5fb52      ,0x8664e6e5      ,
0xbe2b5b58      ,0xbaea46ef      ,0xb7a96036      ,0xb3687d81      , 0xad2f2d84      ,0xa9ee3033      ,0xa4ad16ea      ,0xa06c0b5d      ,
0xd4326d90      ,0xd0f37027      ,0xddb056fe      ,0xd9714b49      , 0xc7361b4c      ,0xc3f706fb      ,0xceb42022      ,0xca753d95      ,
0xf23a8028      ,0xf6fb9d9f      ,0xfbb8bb46      ,0xff79a6f1      , 0xe13ef6f4      ,0xe5ffeb43      ,0xe8bccd9a      ,0xec7dd02d      ,
0x34867077      ,0x30476dc0      ,0x3d044b19      ,0x39c556ae      , 0x278206ab      ,0x23431b1c      ,0x2e003dc5      ,0x2ac12072      ,
0x128e9dcf      ,0x164f8078      ,0x1b0ca6a1      ,0x1fcdbb16      , 0x18aeb13       ,0x54bf6a4       ,0x808d07d       ,0xcc9cdca       ,
0x7897ab07      ,0x7c56b6b0      ,0x71159069      ,0x75d48dde      , 0x6b93dddb      ,0x6f52c06c      ,0x6211e6b5      ,0x66d0fb02      ,
0x5e9f46bf      ,0x5a5e5b08      ,0x571d7dd1      ,0x53dc6066      , 0x4d9b3063      ,0x495a2dd4      ,0x44190b0d      ,0x40d816ba      ,
0xaca5c697      ,0xa864db20      ,0xa527fdf9      ,0xa1e6e04e      , 0xbfa1b04b      ,0xbb60adfc      ,0xb6238b25      ,0xb2e29692      ,
0x8aad2b2f      ,0x8e6c3698      ,0x832f1041      ,0x87ee0df6      , 0x99a95df3      ,0x9d684044      ,0x902b669d      ,0x94ea7b2a      ,
0xe0b41de7      ,0xe4750050      ,0xe9362689      ,0xedf73b3e      , 0xf3b06b3b      ,0xf771768c      ,0xfa325055      ,0xfef34de2      ,
0xc6bcf05f      ,0xc27dede8      ,0xcf3ecb31      ,0xcbffd686      , 0xd5b88683      ,0xd1799b34      ,0xdc3abded      ,0xd8fba05a      ,
0x690ce0ee      ,0x6dcdfd59      ,0x608edb80      ,0x644fc637      , 0x7a089632      ,0x7ec98b85      ,0x738aad5c      ,0x774bb0eb      ,
0x4f040d56      ,0x4bc510e1      ,0x46863638      ,0x42472b8f      , 0x5c007b8a      ,0x58c1663d      ,0x558240e4      ,0x51435d53      ,
0x251d3b9e      ,0x21dc2629      ,0x2c9f00f0      ,0x285e1d47      , 0x36194d42      ,0x32d850f5      ,0x3f9b762c      ,0x3b5a6b9b      ,
0x315d626       ,0x7d4cb91       ,0xa97ed48       ,0xe56f0ff       , 0x1011a0fa      ,0x14d0bd4d      ,0x19939b94      ,0x1d528623      ,
0xf12f560e      ,0xf5ee4bb9      ,0xf8ad6d60      ,0xfc6c70d7      , 0xe22b20d2      ,0xe6ea3d65      ,0xeba91bbc      ,0xef68060b      ,
0xd727bbb6      ,0xd3e6a601      ,0xdea580d8      ,0xda649d6f      , 0xc423cd6a      ,0xc0e2d0dd      ,0xcda1f604      ,0xc960ebb3      ,
0xbd3e8d7e      ,0xb9ff90c9      ,0xb4bcb610      ,0xb07daba7      , 0xae3afba2      ,0xaafbe615      ,0xa7b8c0cc      ,0xa379dd7b      ,
0x9b3660c6      ,0x9ff77d71      ,0x92b45ba8      ,0x9675461f      , 0x8832161a      ,0x8cf30bad      ,0x81b02d74      ,0x857130c3      ,
0x5d8a9099      ,0x594b8d2e      ,0x5408abf7      ,0x50c9b640      , 0x4e8ee645      ,0x4a4ffbf2      ,0x470cdd2b      ,0x43cdc09c      ,
0x7b827d21      ,0x7f436096      ,0x7200464f      ,0x76c15bf8      , 0x68860bfd      ,0x6c47164a      ,0x61043093      ,0x65c52d24      ,
0x119b4be9      ,0x155a565e      ,0x18197087      ,0x1cd86d30      , 0x29f3d35       ,0x65e2082       ,0xb1d065b       ,0xfdc1bec       ,
0x3793a651      ,0x3352bbe6      ,0x3e119d3f      ,0x3ad08088      , 0x2497d08d      ,0x2056cd3a      ,0x2d15ebe3      ,0x29d4f654      ,
0xc5a92679      ,0xc1683bce      ,0xcc2b1d17      ,0xc8ea00a0      , 0xd6ad50a5      ,0xd26c4d12      ,0xdf2f6bcb      ,0xdbee767c      ,
0xe3a1cbc1      ,0xe760d676      ,0xea23f0af      ,0xeee2ed18      , 0xf0a5bd1d      ,0xf464a0aa      ,0xf9278673      ,0xfde69bc4      ,
0x89b8fd09      ,0x8d79e0be      ,0x803ac667      ,0x84fbdbd0      , 0x9abc8bd5      ,0x9e7d9662      ,0x933eb0bb      ,0x97ffad0c      ,
0xafb010b1      ,0xab710d06      ,0xa6322bdf      ,0xa2f33668      , 0xbcb4666d      ,0xb8757bda      ,0xb5365d03      ,0xb1f740b4
#endif /*0*/

static GT_BIT   pipe_crcTable_ready = 0;
/**
* @internal pipe_crcInit function
* @endinternal
*
* @brief   initialize value into pipe_crcTable[256]
*/
static void pipe_crcInit(void)
{
    GT_U32 dividend;
    GT_U32 remainder;
    GT_32  bitIndex;

    /*
     * Compute the remainder of each possible dividend.
     */
    for (dividend = 0; dividend < 256; ++dividend)
    {
        /*
         * Start with the dividend followed by zeros.
         */
        remainder = dividend << (CRC_WIDTH - 8);

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (bitIndex = 8; bitIndex > 0; --bitIndex)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & CRC_TOPBIT)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }

        /*
         * Store the result into the table.
         */
        pipe_crcTable[dividend] = remainder;
    }

}   /* crcInit() */


/**
* @internal pipeIngressCalculateHashIndex function
* @endinternal
*
* @brief   calculate the Hash index from the key
*/
static GT_VOID pipeIngressCalculateHashIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN HASH_TYPE_ENT    hashType
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculateHashIndex);

    GT_U32 regAddress ,regValue;
    GT_U32  Hash_Mode;
    GT_U32  Hash_Bit_offset[8] = {0};
    GT_U32  bitIndex , bitValue;
    GT_U32  byteOffset , bitOffset;
    GT_U32  hashValue;
    GT_U32  maxNumOfBytes = 43;
    GT_U8   byteValue , u8Data;
    GT_U32  remainder;
    GT_U32  ii;
    GT_U32  Hash_CRC32_Seed;
    GT_U32  bitOffsetsNumber = 7;


    regAddress = SMEM_PIPE_PCP_PACKET_TYPE_HASH_MODE_0_ENTRY_REG(devObjPtr,hashType);
    smemRegGet(devObjPtr, regAddress , &regValue);

    Hash_Mode = SMEM_U32_GET_FIELD(regValue,0,1);
    __LOG_PARAM(Hash_Mode);

    if(Hash_Mode == 0)
    {
        __LOG(("CRC32 Hash Mode \n"));

        if(pipe_crcTable_ready == 0)
        {
            scibAccessLock();

            if(pipe_crcTable_ready == 0)/* second check in case that 2 devices wait for the SCIB_LOCK */
            {
                pipe_crcInit();
                pipe_crcTable_ready = 1;
            }

            scibAccessUnlock();
        }

        regAddress = SMEM_PIPE_PCP_HASH_CRC32_SEED_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress , &regValue);
        Hash_CRC32_Seed = regValue;

        __LOG_PARAM(Hash_CRC32_Seed);

        remainder = Hash_CRC32_Seed;
        /*
         * Divide the message by the polynomial, a byte at a time.
         */
        for (ii = 0; ii < maxNumOfBytes; ii++)
        {
            byteOffset = (maxNumOfBytes - 1) - ii;

            byteValue = pipe_descrPtr->pipe_hashKey[byteOffset];

            u8Data = byteValue ^  (remainder >> (CRC_WIDTH - 8));
            remainder = pipe_crcTable[u8Data] ^ (remainder << 8);
        }

        hashValue = remainder;
    }
    else
    {
        __LOG(("Extract 7 bits from Hash Key Mode \n"));

        bitIndex = 0;
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,1,9);
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,10,9);
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,19,9);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_HASH_MODE_1_ENTRY_REG(devObjPtr,hashType);
        smemRegGet(devObjPtr, regAddress , &regValue);
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,0,9);
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,9,9);
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,18,9);

        regAddress = SMEM_PIPE_PCP_PACKET_TYPE_HASH_MODE_2_ENTRY_REG(devObjPtr,hashType);
        smemRegGet(devObjPtr, regAddress , &regValue);
        Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,0,9);

        __LOG_PARAM(Hash_Bit_offset[0]);
        __LOG_PARAM(Hash_Bit_offset[1]);
        __LOG_PARAM(Hash_Bit_offset[2]);
        __LOG_PARAM(Hash_Bit_offset[3]);
        __LOG_PARAM(Hash_Bit_offset[4]);
        __LOG_PARAM(Hash_Bit_offset[5]);
        __LOG_PARAM(Hash_Bit_offset[6]);

        if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
        {
            bitOffsetsNumber = 8;
            __LOG(("Pipe A1 Extract 1 bit (Hash Bit7 Offset) from Hash Key Mode 2 \n"));

            Hash_Bit_offset[bitIndex++] = SMEM_U32_GET_FIELD(regValue,9,9);
            __LOG_PARAM(Hash_Bit_offset[7]);
        }

        hashValue = 0;
        for(bitIndex = 0 ; bitIndex < bitOffsetsNumber; bitIndex++)
        {
            bitOffset = Hash_Bit_offset[bitIndex];

            byteOffset = bitOffset / 8;

            if(byteOffset >= maxNumOfBytes)/* size of pipe_hashKey */
            {
                __LOG(("Warning : Hash_Bit_offset[%d]=[%d] is out of range ((43*8) -1)=[%d] \n",
                    bitIndex,
                    bitOffset,
                    ((maxNumOfBytes*8) -1)));

                bitValue = 0;
            }
            else
            {
                bitValue = (pipe_descrPtr->pipe_hashKey[byteOffset] >> (bitOffset - (byteOffset*8))) & 1;
            }

            if(bitValue)
            {
                hashValue |= 1 << bitIndex;
            }
        }

        __LOG_PARAM(hashValue);
    }

    /* currently the pipe_PacketHash hold ALL bits of the calculation */
    pipe_descrPtr->pipe_PacketHash = hashValue & 0xfff;/* only 12 bits in the descriptor */

    __LOG_PARAM(pipe_descrPtr->pipe_PacketHash);

}



/**
* @internal pipeIngressCalculateHash function
* @endinternal
*
* @brief   Calculate Hash
*/
static GT_VOID pipeIngressCalculateHash
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculateHash);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U32  regAddress,regValue;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8  *ethTypeOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset;
    GT_U8  *l3OffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset + 2;
    GT_U16  etherType = (ethTypeOffsetPtr[0] << 8) |
                         ethTypeOffsetPtr[1];
    GT_U32  sbit;
    GT_U32 llcNonSnap;
    GT_U32  UDE1_Hash_EtherType_Valid , UDE1_Hash_EtherType;
    GT_U32  UDE2_Hash_EtherType_Valid , UDE2_Hash_EtherType;
    GT_U32  MPLS_EtherType1_valid ,MPLS_EtherType1;
    GT_U32  MPLS_EtherType2_valid ,MPLS_EtherType2;
    HASH_TYPE_ENT hashType;

    regAddress = SMEM_PIPE_PCP_HASH_PACKET_TYPE_UDE1_ETHER_TYPE_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);
    UDE1_Hash_EtherType_Valid = SMEM_U32_GET_FIELD(regValue,0,1);
    UDE1_Hash_EtherType = SMEM_U32_GET_FIELD(regValue,1,16);
    __LOG_PARAM(UDE1_Hash_EtherType_Valid);
    __LOG_PARAM(UDE1_Hash_EtherType);

    regAddress = SMEM_PIPE_PCP_HASH_PACKET_TYPE_UDE2_ETHER_TYPE_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);
    UDE2_Hash_EtherType_Valid = SMEM_U32_GET_FIELD(regValue,0,1);
    UDE2_Hash_EtherType = SMEM_U32_GET_FIELD(regValue,1,16);
    __LOG_PARAM(UDE2_Hash_EtherType_Valid);
    __LOG_PARAM(UDE2_Hash_EtherType);


    regAddress = SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_2_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    MPLS_EtherType1_valid = SMEM_U32_GET_FIELD(regValue,0,1);
    MPLS_EtherType1       = SMEM_U32_GET_FIELD(regValue,1,16);
    __LOG_PARAM(MPLS_EtherType1_valid);
    __LOG_PARAM(MPLS_EtherType1);

    regAddress = SMEM_PIPE_PCP_GLOBAL_ETHER_TYPE_CONFIG_3_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    MPLS_EtherType2_valid = SMEM_U32_GET_FIELD(regValue,0,1);
    MPLS_EtherType2       = SMEM_U32_GET_FIELD(regValue,1,16);
    __LOG_PARAM(MPLS_EtherType2_valid);
    __LOG_PARAM(MPLS_EtherType2);

    llcNonSnap = (pipe_descrPtr->pipe_l2Encaps == SKERNEL_LLC_E) ? 1 : 0/* non LLC or non-snap */;

    if(llcNonSnap)
    {
        __LOG(("NOTE: UDE is not relevant if packet is LLC-Non-SNAP. \n"));
    }


    if((pipe_descrPtr->pipe_isIpv4) &&
       ((pipe_descrPtr->pipe_IP_header_Protocol == SNET_TCP_PROT_E) ||
        (pipe_descrPtr->pipe_IP_header_Protocol == SNET_UDP_PROT_E)) &&
       (pipe_descrPtr->pipe_IP_isFragment == 0))
    {
        /*  EtherType = 0x0800, and
            IPv4 header <Protocol> = 6/17, and
            packet is NOT IPv4 fragment (IPv4 header <more fragments>=0, IPv4
            header<frag offset>=0)
            */

        __LOG(("hash type is : IPv4 TCP/UDP \n"));
        hashType = HASH_TYPE_IPV4_TCP_UDP_E;
    }
    else
    if((pipe_descrPtr->pipe_isIpv6) &&
       ((pipe_descrPtr->pipe_IP_header_Protocol == SNET_TCP_PROT_E) ||
        (pipe_descrPtr->pipe_IP_header_Protocol == SNET_UDP_PROT_E)) &&
       (pipe_descrPtr->pipe_IP_isFragment == 0))
    {
        /* EtherType = 0x086DD, and
            IPv6 Fragment Extension Header ‘44’ does NOT exist1, and
            IPv6 <Next Header> = 6/17 (after skipping over extension headers)
        */
        __LOG(("hash type is : IPv6 TCP/UDP \n"));
        hashType = HASH_TYPE_IPV6_TCP_UDP_E;
    }
    else
    if(pipe_descrPtr->pipe_isIpv4)
    {
        /*
            EtherType = 0x0800, and
            IPv4 header <Protocol> != 6/17 OR packet is IPv4 fragment (IPv4 header
            <more fragments>=1 or IPv4 header<frag offset> > 0)
        */
        __LOG(("hash type is : IPv4 non-TCP/UDP \n"));
        hashType = HASH_TYPE_IPV4_NONE_TCP_UDP_E;
    }
    else
    if(pipe_descrPtr->pipe_isIpv6)
    {
        /*
            (EtherType = 0x086DD, and
            IPv6 <Next Header> != 6/17)
            OR
            IPv6 Fragment Extension Header '44' exists
        */
        __LOG(("hash type is : IPv6 non-TCP/UDP \n"));
        hashType = HASH_TYPE_IPV6_NONE_TCP_UDP_E;
    }
    else
    if((MPLS_EtherType1_valid && etherType == MPLS_EtherType1) ||
       (MPLS_EtherType2_valid && etherType == MPLS_EtherType2))
    {
        sbit = l3OffsetPtr[2] & 1; /*S: Bottom-of-Stack*/

        if(sbit)
        {
            __LOG(("hash type is : MPLS with single label stack \n"));
            hashType = HASH_TYPE_MPLS_SINGLE_LABLE_E;
        }
        else
        {
            __LOG(("hash type is : MPLS with at least 2 labels in the label stack \n"));
            hashType = HASH_TYPE_MPLS_MORE_THAN_SINGLE_LABLE_E;
        }
    }
    else
    if(UDE1_Hash_EtherType_Valid && etherType == UDE1_Hash_EtherType && llcNonSnap == 0)
    {
        /*  EtherType == global<UDE1>
            NOTE: UDE is not relevant if packet is LLC-Non-SNAP. UDE EtherType must
            not match EtherType for IPv4, IPv6, or MPLS.
        */
        __LOG(("hash type is : UDE1 \n"));
        hashType = HASH_TYPE_UDE_1_E;

    }
    else
    if(UDE2_Hash_EtherType_Valid && etherType == UDE2_Hash_EtherType && llcNonSnap == 0)
    {
        __LOG(("hash type is : UDE2 \n"));
        hashType = HASH_TYPE_UDE_2_E;
    }
    else
    {
        /*Default Hash Packet Type if none of the above types match*/
        __LOG(("hash type is : Ethernet \n"));
        hashType = HASH_TYPE_ETHERNET_E;
    }

    /* build the 43 bytes key */
    pipeIngressCalculateHashKeyBuild(devObjPtr,pipe_descrPtr,hashType);

    /* generate the hash index */
    pipeIngressCalculateHashIndex(devObjPtr,pipe_descrPtr,hashType);
}

/**
* @internal pipeIngressCalculateLagDesignatedPortmap function
* @endinternal
*
* @brief   Calculate Lag designated ports bmp
*/
static GT_VOID pipeIngressCalculateLagDesignatedPortmap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculateLagDesignatedPortmap);

    GT_U32  regAddress,regValue;
    GT_U32  lagIndex;
    GT_U32  Lag_Designated_entry;
    GT_U32  lagTableMode;
    GT_U32  lagTableNumber;
    GT_U32  indexMode;

    regAddress = SMEM_PIPE_PCP_LAG_INDEX_MODE_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);
    indexMode = SMEM_U32_GET_FIELD(regValue, pipe_descrPtr->pipe_SrcPort, 1);

    if(indexMode == 1)
    {
        __LOG(("Designated port table indexing with pseudo-random number generator\n"));
        lagIndex = rand();
        lagIndex &= 0x7F;
    }
    else
    {
        __LOG(("Designated port table indexing with hash\n"));
        lagIndex = pipe_descrPtr->pipe_PacketHash & 0x7f;

        if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
        {
            regAddress = SMEM_PIPE_PCP_GLOBAL_CONFIG_REG(devObjPtr);
            smemRegGet(devObjPtr, regAddress , &regValue);
            lagTableMode = SMEM_U32_GET_FIELD(regValue, 0, 1);
            if (lagTableMode == 0)
            {
                __LOG(("The LAG Designated Port Table mode - Single Table\n"));
                lagIndex = pipe_descrPtr->pipe_PacketHash & 0xFF;
            }
            else
            {
                regAddress = SMEM_PIPE_PCP_PACKET_TYPE_LAG_TABLE_NUMBER_REG(devObjPtr);
                smemRegGet(devObjPtr, regAddress , &regValue);
                lagTableNumber = SMEM_U32_GET_FIELD(regValue, pipe_descrPtr->pipe_PktTypeIdx, 1);
                __LOG(("The LAG Designated Port Table mode - Two Tables\n"));
                lagIndex |= lagTableNumber << 7;
            }
        }
    }

    __LOG_PARAM(lagIndex);

    regAddress = SMEM_PIPE_PCP_LAG_DESIGNATED_PORT_ENTRY_REG(devObjPtr , lagIndex);
    smemRegGet(devObjPtr, regAddress , &regValue);

    Lag_Designated_entry = SMEM_U32_GET_FIELD(regValue,0,17);

    pipe_descrPtr->pipe_lagDesignatedPortsBmp = Lag_Designated_entry | pipe_descrPtr->pipe_lagDisableVector;
    LOG_BMP_ARR("pipe_lagDesignatedPortsBmp",&pipe_descrPtr->pipe_lagDesignatedPortsBmp,17);
}

/**
* @internal pipeIngressCalculatePortEnablePortmap function
* @endinternal
*
* @brief   Calculate port enable ports bmp
*/
static GT_VOID pipeIngressCalculatePortEnablePortmap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculatePortEnablePortmap);

    GT_U32  regAddress,regValue;

    regAddress = SMEM_PIPE_PCP_PORTS_ENABLE_CONFIG_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress , &regValue);

    pipe_descrPtr->pipe_portEnableBmp = SMEM_U32_GET_FIELD(regValue,0,17);
    LOG_BMP_ARR("pipe_descrPtr->pipe_portEnableBmp :",&pipe_descrPtr->pipe_portEnableBmp,17);
}


/**
* @internal pipeIngressCalculateForwardingPortmap function
* @endinternal
*
* @brief   Calculate Forwarding ports bmp
*/
static GT_VOID pipeIngressCalculateForwardingPortmap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCalculateForwardingPortmap);

    LOG_BMP_ARR("INPUT : pipe_descrPtr->pipe_dstPortsBmp :",&pipe_descrPtr->pipe_dstPortsBmp,17);
    LOG_BMP_ARR("INPUT : pipe_descrPtr->pipe_portFilterBmp :",&pipe_descrPtr->pipe_portFilterBmp,17);
    LOG_BMP_ARR("INPUT : pipe_descrPtr->pipe_lagDesignatedPortsBmp :",&pipe_descrPtr->pipe_lagDesignatedPortsBmp,17);
    LOG_BMP_ARR("INPUT : pipe_descrPtr->pipe_portEnableBmp :",&pipe_descrPtr->pipe_portEnableBmp,17);

    if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
    {
        GT_U32  regAddress;
        GT_U32  ingressFilterEnable;
        GT_U32  portFilteringDisableVector;

        regAddress = SMEM_PIPE_PCP_ENABLE_PACKET_TYPE_EGRESS_FILTERING_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddress , pipe_descrPtr->pipe_PktTypeIdx, 1,
            &portFilteringDisableVector);
        if(portFilteringDisableVector == 0)
        {
            portFilteringDisableVector = 0x1FFFF;
        }
        else
        {
            portFilteringDisableVector = 0x00000;
        }
        LOG_BMP_ARR("INPUT : port filtering disable vector :",&portFilteringDisableVector,17);

        regAddress = SMEM_PIPE_PCP_ENABLE_PACKET_TYPE_INGRESS_FILTERING_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddress , pipe_descrPtr->pipe_PktTypeIdx, 1,
            &ingressFilterEnable);
        if((ingressFilterEnable == 1) && (((1 << pipe_descrPtr->pipe_SrcPort) &
            pipe_descrPtr->pipe_portFilterBmp) == 0))
        {
            ingressFilterEnable = 0x00000;
            __LOG(("WARNING : Packet drop decision made based on source port "
                   "disabled in port filtering table\n"));
        }
        else
        {
            ingressFilterEnable = 0x1FFFF;
        }

        pipe_descrPtr->pipe_ForwardingPortmap =
            pipe_descrPtr->pipe_dstPortsBmp & ingressFilterEnable &
            (pipe_descrPtr->pipe_portFilterBmp | portFilteringDisableVector) &
            pipe_descrPtr->pipe_lagDesignatedPortsBmp &
            pipe_descrPtr->pipe_portEnableBmp;/* like sip5.20 there is no link indication from the MAC */
    }
    else
    {
        pipe_descrPtr->pipe_ForwardingPortmap =
            pipe_descrPtr->pipe_dstPortsBmp &
            pipe_descrPtr->pipe_portFilterBmp &
            pipe_descrPtr->pipe_lagDesignatedPortsBmp &
            pipe_descrPtr->pipe_portEnableBmp;/* like sip5.20 there is no link indication from the MAC */
    }


    LOG_BMP_ARR("OUTPUT : pipe_descrPtr->pipe_ForwardingPortmap :",&pipe_descrPtr->pipe_ForwardingPortmap,17);


    if(pipe_descrPtr->pipe_ForwardingPortmap == 0)
    {
        __LOG(("WARNING : all port were filtered ! (no egress replications) \n"));
    }
}


/**
* @internal pipeIngressCosAttributeAssignment function
* @endinternal
*
* @brief   CoS Attribute Assignment
*/
static GT_VOID pipeIngressCosAttributeAssignment
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCosAttributeAssignment);

    GT_U32  regAddress,regValue;
    GT_U32  Cos_Mode,Cos_Type,Cos_Byte_offset,Cos_Bit_offset,Num_Cos_Bits;
    GT_U32  Entry_Cos_Attributes,Entry_Cos_Attributes_tc,Entry_Cos_Attributes_dp,Entry_Cos_Attributes_up,Entry_Cos_Attributes_dei;
    GT_U32  Port_Cos_TC,Port_Cos_DP,Port_Cos_UP,Port_Cos_DEI;
    enum {COS_TYPE_L2_UPLINK,COS_TYPE_L2_DOWNLINK,COS_TYPE_L3,COS_TYPE_MPLS};
    static GT_U32  numBitsLimits[] = {7,4,6,3};/* check came from VERIFIER code */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;
    GT_U8  *startFramePtr = cheetah_descrPtr->startFramePtr;
    GT_U32  ethTypeOffset = pipe_descrPtr->pipe_L3Offset;
    GT_U8   *l3StartOffsetPtr = cheetah_descrPtr->startFramePtr + ethTypeOffset + 2;
    GT_U8   *udbOffsetPtr;
    GT_U32  u32Field;
    GT_U32  cosIndex;
    GT_U32  pipe_TC,pipe_DP,pipe_UP,pipe_DEI;
    GT_U32  L2_dl_Cos_TC,L2_dl_Cos_DP,L2_dl_Cos_UP,L2_dl_Cos_DEI;
    GT_U32  L2_ul_Cos_TC,L2_ul_Cos_DP,L2_ul_Cos_UP,L2_ul_Cos_DEI;
    GT_U32  L3_Cos_TC,L3_Cos_DP,L3_Cos_UP,L3_Cos_DEI;
    GT_U32  MPLS_Cos_TC,MPLS_Cos_DP,MPLS_Cos_UP,MPLS_Cos_DEI;


    regAddress = SMEM_PIPE_PCP_COS_FORMAT_TABLE_ENTRY_REG(devObjPtr,pipe_descrPtr->pipe_PktTypeIdx);
    smemRegGet(devObjPtr, regAddress , &regValue);

    Cos_Mode = SMEM_U32_GET_FIELD(regValue,0,2);

    __LOG_PARAM(Cos_Mode);

    if(Cos_Mode == 2)
    {
        __LOG(("Cos Mode : Use Entry CoS Attributes \n"));

        Entry_Cos_Attributes = SMEM_U32_GET_FIELD(regValue,15,9);

        Entry_Cos_Attributes_tc  = SMEM_U32_GET_FIELD(Entry_Cos_Attributes,6,3);
        Entry_Cos_Attributes_dp  = SMEM_U32_GET_FIELD(Entry_Cos_Attributes,4,2);
        Entry_Cos_Attributes_up  = SMEM_U32_GET_FIELD(Entry_Cos_Attributes,1,3);
        Entry_Cos_Attributes_dei = SMEM_U32_GET_FIELD(Entry_Cos_Attributes,0,1);

        __LOG_PARAM(Entry_Cos_Attributes_tc) ;
        __LOG_PARAM(Entry_Cos_Attributes_dp) ;
        __LOG_PARAM(Entry_Cos_Attributes_up) ;
        __LOG_PARAM(Entry_Cos_Attributes_dei);

        pipe_TC   = Entry_Cos_Attributes_tc ;
        pipe_DP   = Entry_Cos_Attributes_dp ;
        pipe_UP   = Entry_Cos_Attributes_up ;
        pipe_DEI  = Entry_Cos_Attributes_dei;

    }
    else
    if(Cos_Mode == 1)
    {
        __LOG(("Cos Mode : Map CoS Attributes from Packet CoS \n"));

        Cos_Type = SMEM_U32_GET_FIELD(regValue,2,2);
        Cos_Byte_offset = SMEM_U32_GET_FIELD(regValue,4,5);
        Cos_Bit_offset = SMEM_U32_GET_FIELD(regValue,9,3);
        Num_Cos_Bits = SMEM_U32_GET_FIELD(regValue,12,3);

        __LOG_PARAM(Cos_Type);
        __LOG_PARAM(Cos_Byte_offset);
        __LOG_PARAM(Cos_Bit_offset);
        __LOG_PARAM(Num_Cos_Bits);

        __LOG(("CoS Type: [%s] \n" ,
            (Cos_Type == COS_TYPE_L2_UPLINK ? "L2 Uplink CoS (from controlling bridge, e.g. xDSA, E-Tag)" :
             Cos_Type == COS_TYPE_L2_DOWNLINK ? "L2 Downlink CoS (from network, e.g. VLAN tag)" :
             Cos_Type == COS_TYPE_L3 ? "L3 CoS (e.g. DSCP)" :
             /*COS_TYPE_MPLS*/         "MPLS CoS (e.g. EXP)")
            ));

        if (Num_Cos_Bits > numBitsLimits[Cos_Type])
        {
            __LOG(("WARNING : Num_Cos_Bits[%d] exceeds it's size [%d] (use 'max' allowed bits)\n",
                Num_Cos_Bits,
                numBitsLimits[Cos_Type]));

            Num_Cos_Bits = numBitsLimits[Cos_Type];
        }


        udbOffsetPtr = (Cos_Type == COS_TYPE_L3 || Cos_Type == COS_TYPE_MPLS) ? /*L3 Anchor*/l3StartOffsetPtr : startFramePtr;

        /* NOTE: startBit in HW refers to REVERSE order of all bits in the BYTE !!! */
        /* so convert it to 'SIMULATION logic' */
        Cos_Bit_offset = 8 - (Cos_Bit_offset + Num_Cos_Bits);

        if((Num_Cos_Bits + Cos_Bit_offset) > 8)
        {
            /* field cross two bytes*/
            u32Field = udbOffsetPtr[Cos_Byte_offset-1] << 8 | udbOffsetPtr[Cos_Byte_offset];
        }
        else
        {
            u32Field = udbOffsetPtr[Cos_Byte_offset];
        }

        cosIndex = SMEM_U32_GET_FIELD(u32Field,Cos_Bit_offset,Num_Cos_Bits);


        __LOG_PARAM_WITH_NAME("Access To proper Cos table at entry index:",cosIndex);


        switch(Cos_Type)
        {
            case COS_TYPE_L2_UPLINK:
                regAddress = SMEM_PIPE_PCP_DSA_COS_MAPPING_ENTRY_REG(devObjPtr,cosIndex);
                smemRegGet(devObjPtr, regAddress , &regValue);

                L2_ul_Cos_TC  = SMEM_U32_GET_FIELD(regValue,0,3);
                L2_ul_Cos_DP  = SMEM_U32_GET_FIELD(regValue,3,2);
                L2_ul_Cos_UP  = SMEM_U32_GET_FIELD(regValue,5,3);
                L2_ul_Cos_DEI = SMEM_U32_GET_FIELD(regValue,8,1);

                __LOG_PARAM(L2_ul_Cos_TC );
                __LOG_PARAM(L2_ul_Cos_DP );
                __LOG_PARAM(L2_ul_Cos_UP );
                __LOG_PARAM(L2_ul_Cos_DEI);

                pipe_TC   = L2_ul_Cos_TC ;
                pipe_DP   = L2_ul_Cos_DP ;
                pipe_UP   = L2_ul_Cos_UP ;
                pipe_DEI  = L2_ul_Cos_DEI;
                break;

            case COS_TYPE_L2_DOWNLINK:
                regAddress = SMEM_PIPE_PCP_PORT_L2_COS_MAPPING_ENTRY_REG(devObjPtr,
                                pipe_descrPtr->pipe_SrcPort,
                                cosIndex);
                smemRegGet(devObjPtr, regAddress , &regValue);

                L2_dl_Cos_TC  = SMEM_U32_GET_FIELD(regValue,0,3);
                L2_dl_Cos_DP  = SMEM_U32_GET_FIELD(regValue,3,2);
                L2_dl_Cos_UP  = SMEM_U32_GET_FIELD(regValue,5,3);
                L2_dl_Cos_DEI = SMEM_U32_GET_FIELD(regValue,8,1);

                __LOG_PARAM(L2_dl_Cos_TC );
                __LOG_PARAM(L2_dl_Cos_DP );
                __LOG_PARAM(L2_dl_Cos_UP );
                __LOG_PARAM(L2_dl_Cos_DEI);

                pipe_TC   = L2_dl_Cos_TC ;
                pipe_DP   = L2_dl_Cos_DP ;
                pipe_UP   = L2_dl_Cos_UP ;
                pipe_DEI  = L2_dl_Cos_DEI;
                break;
            case COS_TYPE_L3:
                regAddress = SMEM_PIPE_PCP_L3_COS_MAPPING_ENTRY_REG(devObjPtr,cosIndex);
                smemRegGet(devObjPtr, regAddress , &regValue);

                L3_Cos_TC  = SMEM_U32_GET_FIELD(regValue,0,3);
                L3_Cos_DP  = SMEM_U32_GET_FIELD(regValue,3,2);
                L3_Cos_UP  = SMEM_U32_GET_FIELD(regValue,5,3);
                L3_Cos_DEI = SMEM_U32_GET_FIELD(regValue,8,1);

                __LOG_PARAM(L3_Cos_TC );
                __LOG_PARAM(L3_Cos_DP );
                __LOG_PARAM(L3_Cos_UP );
                __LOG_PARAM(L3_Cos_DEI);

                pipe_TC   = L3_Cos_TC ;
                pipe_DP   = L3_Cos_DP ;
                pipe_UP   = L3_Cos_UP ;
                pipe_DEI  = L3_Cos_DEI;
                break;
            case COS_TYPE_MPLS:
            default:
                regAddress = SMEM_PIPE_PCP_MPLS_COS_MAPPING_ENTRY_REG(devObjPtr,cosIndex);
                smemRegGet(devObjPtr, regAddress , &regValue);

                MPLS_Cos_TC  = SMEM_U32_GET_FIELD(regValue,0,3);
                MPLS_Cos_DP  = SMEM_U32_GET_FIELD(regValue,3,2);
                MPLS_Cos_UP  = SMEM_U32_GET_FIELD(regValue,5,3);
                MPLS_Cos_DEI = SMEM_U32_GET_FIELD(regValue,8,1);

                __LOG_PARAM(MPLS_Cos_TC );
                __LOG_PARAM(MPLS_Cos_DP );
                __LOG_PARAM(MPLS_Cos_UP );
                __LOG_PARAM(MPLS_Cos_DEI);

                pipe_TC   = MPLS_Cos_TC ;
                pipe_DP   = MPLS_Cos_DP ;
                pipe_UP   = MPLS_Cos_UP ;
                pipe_DEI  = MPLS_Cos_DEI;
                break;
        }
    }
    else
    {
        __LOG(("Use Port CoS Attributes \n"));

        regAddress = SMEM_PIPE_PCP_PORT_COS_ATTRIBUTES_REG(devObjPtr,pipe_descrPtr->pipe_SrcPort);
        smemRegGet(devObjPtr, regAddress , &regValue);

        Port_Cos_TC  = SMEM_U32_GET_FIELD(regValue,0,3);
        Port_Cos_DP  = SMEM_U32_GET_FIELD(regValue,3,2);
        Port_Cos_UP  = SMEM_U32_GET_FIELD(regValue,5,3);
        Port_Cos_DEI = SMEM_U32_GET_FIELD(regValue,8,1);

        __LOG_PARAM(Port_Cos_TC) ;
        __LOG_PARAM(Port_Cos_DP) ;
        __LOG_PARAM(Port_Cos_UP) ;
        __LOG_PARAM(Port_Cos_DEI);

        pipe_TC   = Port_Cos_TC ;
        pipe_DP   = Port_Cos_DP ;
        pipe_UP   = Port_Cos_UP ;
        pipe_DEI  = Port_Cos_DEI;
    }


    pipe_descrPtr->pipe_TC       = pipe_TC ;
    pipe_descrPtr->pipe_DP       = pipe_DP ;
    pipe_descrPtr->pipe_UP       = pipe_UP ;
    pipe_descrPtr->pipe_DEI      = pipe_DEI;

    return;

}

/**
* @internal pipeIngressCncCounting function
* @endinternal
*
* @brief   Do CNC counting
*/
static GT_VOID pipeIngressCncCounting
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(pipeIngressCncCounting);
    GT_U32  cncIndexValue = pipe_descrPtr->pipe_DstIdx;
    /*if the dest address is legal, send to cnc*/
    if (pipe_descrPtr->pipe_errorOccurred_BMP & PIPE_ERROR_DST_EXCEPTION_FORWARDING_PORTMAP_BIT)
    {
        __LOG(("The CNC not applied due to 'Dst Exception Forwarding PortMap' \n"));
        return;
    }

    __LOG(("CNC - PCP client - destination ports bmp table index , index[%d] \n",
        cncIndexValue));
    snetCht3CncCount(devObjPtr,
        pipe_descrPtr->cheetah_descrPtr,
        SNET_CNC_PIPE_DEVICE_CLIENT_PCP_DEST_INDEX_E,
        cncIndexValue);

}



/**
* @internal snetPipeIngressPacketProc function
* @endinternal
*
* @brief   Calculate Destination Index
*/
static GT_VOID snetPipeIngressPacketProc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    DECLARE_FUNC_NAME(snetPipeIngressPacketProc);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PIPE_PCP_E);

    /*EtherType & L3 Offset Assignment*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressEtherTypeAndL3Offset(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("EtherType & L3 Offset Assignment");

    /*L4 Offset Calculation*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressL4Offset(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("L4 Offset Calculation");

    /*Calculate IP2ME index*/
    if(SKERNEL_IS_PIPE_REVISON_A1_DEV(devObjPtr))
    {
        SIM_PIPE_LOG_PACKET_DESCR_SAVE
        pipeIngressIp2meIndexCalculate(devObjPtr,pipe_descrPtr);
        SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate IP2ME index");
    }

    /*Packet Type Key Calculation (set pipe_descrPtr->pipe_packetTypeKey[6..16])*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressPacketTypeKey(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Packet Type Key");
    /*Search Packet Type Table (set pipe_descrPtr->pipe_PktTypeIdx)*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressSearchPacketTypeTable(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Search Packet Type Table");

    if(pipe_descrPtr->pipe_errorOccurred_BMP)
    {
        __LOG(("The packet is dropped !!! \n"));
        return;
    }

    /*Calculate Destination Index and get <pipe_dstPortsBmp>*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCalculateDestinationIndex(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate Destination Index");
    /*Calculate Source Index*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCalculatePortFilterIndex(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate Source Index");
    /*Calculate Hash*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCalculateHash(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate Hash");

    /*Calculate Lag designated Portmap*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCalculateLagDesignatedPortmap(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate Lag designated Portmap");

    /*Calculate port enable Portmap*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCalculatePortEnablePortmap(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate port enable Portmap");

    /*Calculate Forwarding Portmap*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCalculateForwardingPortmap(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Calculate Forwarding Portmap");

    /*CoS Attribute Assignment*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCosAttributeAssignment(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("CoS Attribute Assignment");

    /*CNC counting*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressCncCounting(devObjPtr,pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("CNC counting");

    /*PTP Type Key Calculation (set pipe_descrPtr->pipe_PTPTypeKey[0..10])*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipePtpPacketTypeKey(devObjPtr, pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("PTP type key");

    /*Search PTP Type Table (set pipe_descrPtr->pipe_PTPTypeIdx)*/
    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    pipeIngressSearchPtpTypeTable(devObjPtr, pipe_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("Search PTP Type Table");
}

/**
* @internal pipeFrameProcess function
* @endinternal
*
* @brief   device packet processing (from network/CPU)
*         ingress+egress processing
*/
static GT_VOID pipeFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
)
{
    GT_BOOL st;

    DECLARE_FUNC_NAME(pipeFrameProcess);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;

    cheetah_descrPtr->startFramePtr = cheetah_descrPtr->frameBuf->actualDataPtr;
    cheetah_descrPtr->byteCount = cheetah_descrPtr->frameBuf->actualDataSize;
    cheetah_descrPtr->origByteCount = cheetah_descrPtr->byteCount;

    cheetah_descrPtr->ingressDevObjPtr = devObjPtr;

    {/* used by egress to CPU port to SGMII/SDMA */

        /* state that the device supports SDMA to the CPU */
        cheetah_descrPtr->isCpuUseSdma = 1;
        /* the egress CPU device is OWN (not multi-core device) */
        cheetah_descrPtr->cpuPortGroupDevObjPtr = devObjPtr;
    }

    __LOG(("frame dump:  \n"));
    simLogPacketDescrFrameDump(devObjPtr, cheetah_descrPtr);

    SIM_PIPE_LOG_PACKET_DESCR_SAVE
    st = snetChtRxPort(devObjPtr, pipe_descrPtr->cheetah_descrPtr);
    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("snetChtRxPort");
    /* Rx Port layer processing of frames */
    if (st == GT_FALSE)
    {
        return;
    }

    pipe_descrPtr->pipe_SrcPort = cheetah_descrPtr->localDevSrcPort;

    __LOG(("Continue Ingress packet processing "));
    snetPipeIngressPacketProc(devObjPtr, pipe_descrPtr);

    __LOG(("Start egress packet processing "));
    snetPipeEgressPacketProc(devObjPtr, pipe_descrPtr);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    __LOG((SIM_LOG_ENDED_IN_PACKET_STR
            "ended processing frame from: deviceName[%s],deviceId[%d], srcPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            cheetah_descrPtr->localDevSrcPort));
}
/**
* @internal snetPipeFrameProcess function
* @endinternal
*
* @brief   Process frame from 'MAC' port (not SDMA)
*/
GT_VOID snetPipeFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
)
{
    DECLARE_FUNC_NAME(snetPipeFrameProcess);

    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr;
    /* Pipe : pointer to the frame's descriptor */
    SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E);

    pipe_descrPtr    = snetPipeDuplicateDescr(devObjPtr, NULL);
    /* get the 'cheetah' descriptor */
    cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;

    /* save info to log */
    __LOG((SIM_LOG_IN_PACKET_STR
            "start new frame on: deviceName[%s],deviceId[%d], srcPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            srcPort));

    SIM_PIPE_LOG_PACKET_DESCR_SAVE

    cheetah_descrPtr->frameBuf = bufferId;
    cheetah_descrPtr->localDevSrcPort = srcPort;
    cheetah_descrPtr->byteCount = (GT_U16)bufferId->actualDataSize;

    {
        /* the packet came with CRC that need to be reset.
            otherwise if packet may get shorten to 60 bytes before adding egress CRC
            we will get the 'ingress CRC' just before the 'egress CRC'.

            example: 64 bytes come from 'uplink port' (4 last bytes are CRC),
                    this packet on egress to 'network port' remove 4 bytes of
                    'from_cpu' DSA tag.
                    and considered as 60 bytes so added with the 4 bytes egress CRC.
                    so packet egress with 2 CRCs (4 bytes each) on this 64 bytes packet !
        */
        GT_U32  ii,index;
        GT_U32  removedCrc = 0;

        for(ii = cheetah_descrPtr->byteCount-4 , index = 0;
            ii < cheetah_descrPtr->byteCount ; ii++ , index++)
        {
            removedCrc |= cheetah_descrPtr->frameBuf->actualDataPtr[ii] << ((3-index) * 8);
            cheetah_descrPtr->frameBuf->actualDataPtr[ii] = 0;
        }

        __LOG(("cleared 4 CRC [0x%8.8x] from ingress packet \n",removedCrc));
    }

    /* get the RXDMA port number */
    cheetah_descrPtr->ingressRxDmaPortNumber = srcPort;

    /* get the MAC port number */
    cheetah_descrPtr->ingressGopPortNumber = cheetah_descrPtr->localDevSrcPort;

    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("snetPipeFrameProcess : prepare packet from network port \n");

    if(simLogIsOpenFlag)
    {
        /* protect the full processing */
        SIM_OS_MAC(simOsMutexLock)(LOG_fullPacketWalkThroughProtectMutex);
    }

    pipeFrameProcess(devObjPtr, pipe_descrPtr);

    if(simLogIsOpenFlag)
    {
        SIM_OS_MAC(simOsMutexUnlock)(LOG_fullPacketWalkThroughProtectMutex);
    }

}

/**
* @internal pipeFromCpuDmaTxQueue function
* @endinternal
*
* @brief   Process transmitted frames per single SDMA queue
*/
static GT_VOID pipeFromCpuDmaTxQueue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr,
    IN GT_U32 txQue,
    OUT GT_BOOL *isLastPacketPtr
)
{
    DECLARE_FUNC_NAME(pipeFromCpuDmaTxQueue);

    GT_U32  mutexUsed;
    GT_BOOL queueEmpty;                         /* queue is empty */
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;

    /* packet from cpu (DMA)*/
    __LOG((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_STR " \n"));

    /* increment the number of packets in the system */
    skernelNumOfPacketsInTheSystemSet(GT_TRUE);

    /* consider as RXDMA of the CPU port */
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_RXDMA_E);

    SIM_PIPE_LOG_PACKET_DESCR_SAVE

    /* Fill frame descriptor from Tx SDMA descriptor */
    snetChtPerformFromCpuDma_forPipeDevice(devObjPtr, txQue, cheetah_descrPtr, &queueEmpty, isLastPacketPtr);

    SIM_PIPE_LOG_PACKET_DESCR_COMPARE("snetChtPerformFromCpuDma : prepare packet from CPU SDMA");

    if(queueEmpty == GT_FALSE)
    {
        if(simLogIsOpenFlag)
        {
            mutexUsed = 1;
            /* protect the full processing */
            SIM_OS_MAC(simOsMutexLock)(LOG_fullPacketWalkThroughProtectMutex);
        }
        else
        {
            mutexUsed = 0;
        }

        /* packet from cpu (DMA) : Pass packet for further processing  */
        pipeFrameProcess(devObjPtr, pipe_descrPtr);

        if(mutexUsed)
        {
            /* protect the egress processing */
            SIM_OS_MAC(simOsMutexUnlock)(LOG_fullPacketWalkThroughProtectMutex);
        }
    }

    /* decrement the number of packets in the system */
    skernelNumOfPacketsInTheSystemSet(GT_FALSE);

    /* packet from cpu (DMA) - Ended */
    __LOG((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_ENDED_STR " \n"));
}

/**
* @internal snetPipeFromCpuDmaProcess function
* @endinternal
*
* @brief   Process frame from 'MG TXQ-SDMA' port (not MAC)
*/
GT_VOID snetPipeFromCpuDmaProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId
)
{
    GT_U32 startSdmaBmp;                        /* tx queue SDMA bitmap */
    GT_U32 txQue;                               /* index of queue */
    GT_U32 txSdmaCmdReg;                        /* Register entry value */
    GT_U32 i;                                   /* tx queue index */
    GT_BOOL isLastPacket;                       /* is last packet in chain */
    /* cheetah : pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * cheetah_descrPtr;
    /* Pipe : pointer to the frame's descriptor */
    SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr;


    /*state from cpu command ()*/
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_FROM_CPU_E);

    pipe_descrPtr    = snetPipeDuplicateDescr(devObjPtr, NULL);
    /* get the 'cheetah' descriptor */
    cheetah_descrPtr = pipe_descrPtr->cheetah_descrPtr;

    /* save the ingress device*/
    cheetah_descrPtr->ingressDevObjPtr = devObjPtr;

    cheetah_descrPtr->frameBuf = bufferId;

    /* Get tx SMDA queue bitmap */
    startSdmaBmp = bufferId->userInfo.data.txSdmaQueueBmp;

    /* Transmit SDMA Queue Command Register */
    smemRegGet(devObjPtr, SMEM_PIPE_TX_SDMA_QUE_CMD_REG(devObjPtr), &txSdmaCmdReg);

    /* Scan all 8 bits in SDMA queue bitmap starting from highest */
    for (i = 0; i < SDMA_TX_QUEUE_MAX_NUMBER; i++)
    {
        txQue = SDMA_TX_QUEUE_MAX_NUMBER - (i+1);

        if (((txSdmaCmdReg >> (txQue + 8)) & 1) == 1)
        {
            /* Queue disabled */
            continue;
        }

        if (((startSdmaBmp >> txQue) & 1) == 0)
        {
            /* Queue was not triggered */
            continue;
        }

        /* send all packets that are in this queue */
        do
        {
            pipeFromCpuDmaTxQueue(devObjPtr, pipe_descrPtr, txQue, &isLastPacket);
            if (((txSdmaCmdReg >> (txQue + 8)) & 1) == 1)
            {
                /* Generate interrupt in case if Tx SDMA stopped processing
                after setting DISQ bit */
                snetChtInterruptTxSdmaEnd_forPipeDevice(devObjPtr,txQue);
                break;
            }
        } while (isLastPacket == GT_FALSE);
    }

}



/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetFalcon3Lpm.c
*
* DESCRIPTION:
*      This is SIP6 LPM engine
*
* DEPENDENCIES:
*      None.
*
* FILE REVISION NUMBER:
*      $Revision: 1 $
*
*******************************************************************************/
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/suserframes/snetFalconLpm.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>


/* number of dip or sip stages in sip5 lpm lookup */
#define SIP6_LPM_NUM_OF_DIP_SIP_STAGES   16

/*******************************************************************************
*   snetSip6CalculateLpmUnitId
*
* DESCRIPTION:
*       Calculate LPM unit id
*
* INPUTS:
*        devObjPtr      - pointer to device object
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*         none
*
* RETURNS:
*       none
*
*******************************************************************************/
static GT_VOID snetSip6CalculateLpmUnitId
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr
)
{
    GT_U32                          pipeId;/* the PipeId */
    GT_U8                           oldlpmUnitIndex = descrPtr->lpmUnitIndex;

    DECLARE_FUNC_NAME(snetSip6CalculateLpmUnitId);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        pipeId = smemGetCurrentPipeId(devObjPtr);

        descrPtr->lpmUnitIndex = pipeId / 2;/* every 2 pipes share the LPM memory */
    }
    else
    {
        descrPtr->lpmUnitIndex = SMAIN_NOT_VALID_CNS;
    }

    if(descrPtr->lpmUnitIndex!= oldlpmUnitIndex)
    {
        __LOG(("lpmUnitIndex from %d to %d \n",oldlpmUnitIndex,descrPtr->lpmUnitIndex));
    }
}

/*******************************************************************************
*   checkSip6IfSipIsZero
*
* DESCRIPTION:
*        check if sip address is all zero
*
* INPUTS:
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*       GT_TRUE      - sip is all zero
*       GT_FALSE     - sip is not all zero
*
*******************************************************************************/
static GT_BOOL checkSip6IfSipIsZero
(
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    GT_BOOL sipIsAllZero = GT_FALSE;
    GT_BIT  isIpv6 = (descrPtr->isIPv4 || descrPtr->isFcoe) ? 0 : 1;
    if (isIpv6 == 0)
    {
        if (descrPtr->sip[0] == 0)
        {
            sipIsAllZero = GT_TRUE;
        }
    }
    else
    {
        if  ((descrPtr->sip[0] == 0 ) &&
             (descrPtr->sip[1] == 0 ) &&
             (descrPtr->sip[2] == 0 ) &&
             (descrPtr->sip[3] == 0 ))
        {
            sipIsAllZero = GT_TRUE;
        }
    }
    return sipIsAllZero;
}
/*******************************************************************************
*   getHeadOfTheTrieAndBucketType
*
* DESCRIPTION:
*        gets head of the trie and bucket type based on packet type and vrId
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*       nodeTypePtr      - pointer to bucket type
*       headOfTheTriePtr - pointer to head of the trie
*
*******************************************************************************/
static GT_VOID getHeadOfTheTrieAndBucketType
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT LPM_NODE_TYPE_ENT               *nodeTypePtr,
    OUT GT_U32                          *headOfTheTriePtr
)
{
    DECLARE_FUNC_NAME(getHeadOfTheTrieAndBucketType);

    GT_U32 regAddress = 0;
    GT_U32 fldValue   = 0;
    LPM_NODE_TYPE_ENT   nodeType;
    GT_U32              headOfTheTrie;


    __LOG(("Get lpm head of the trie and node type"));

    /* get relevant register address */
    if(descrPtr->isIp && descrPtr->isFcoe)
    {
        skernelFatalError("getHeadOfTheTrieAndBucketType: got wrong packet");
    }
    else if(descrPtr->isIp || descrPtr->arp)
    {
        if(descrPtr->isIPv4 || descrPtr->arp)
        { /* ipv4 */
            if(descrPtr->arp)/*ARP Broadcast Mirroring/Trap based on ARP DIP*/
            {
                __LOG(("ARP DIP lookup \n"));
            }
            else
            {
                __LOG(("ipv4 lookup \n"));
            }
            regAddress = SMEM_LION3_LPM_IPV4_VRF_ID_TBL_MEM(devObjPtr, descrPtr->vrfId, descrPtr->lpmUnitIndex);
        }
        else
        { /* ipv6 */
            __LOG(("ipv6 lookup \n"));
            regAddress = SMEM_LION3_LPM_IPV6_VRF_ID_TBL_MEM(devObjPtr, descrPtr->vrfId,descrPtr->lpmUnitIndex);
        }
    }
    else if(descrPtr->isFcoe)
    { /* fcoe */
        __LOG(("FCoE lookup \n"));
        regAddress = SMEM_LION3_LPM_FCOE_VRF_ID_TBL_MEM(devObjPtr, descrPtr->vrfId,descrPtr->lpmUnitIndex);
    }
    else
    { /* unknown protocol */
        skernelFatalError("getHeadOfTheTrieAndBucketType: got unknown lpm lookup");
    }

    /* get head of the trie */
    smemRegFldGet(devObjPtr, regAddress, 0, 20, headOfTheTriePtr);
    headOfTheTrie = *headOfTheTriePtr;
    __LOG_PARAM(headOfTheTrie);

    /* get bucket type, length one bit:
       only regular and compressed nodes can be used in head of the trie */
    smemRegFldGet(devObjPtr, regAddress, 20, 1, &fldValue);
    nodeType = fldValue ? LPM_NODE_TYPE_COMPRESSED_E : LPM_NODE_TYPE_REGULAR_E;

    __LOG_PARAM(nodeType);


    *nodeTypePtr = nodeType;
}

/*******************************************************************************
*   convertStageNumToDataByte
*
* DESCRIPTION:
*        prepares address data byte
*
* INPUTS:
*        devObjPtr   - pointer to device object.
*        descrPtr    - pointer to the frame's descriptor.
*        isSipLookup - indicates is sip lookup
*        stage       - current stage value
*
* OUTPUTS:
*        dataBytePtr - address data byte
*
*******************************************************************************/
static GT_VOID convertStageNumToDataByte
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_BOOL                          isSipLookup,
    IN  GT_U32                           stage,
    OUT GT_U8                           *dataBytePtr
)
{
    DECLARE_FUNC_NAME(convertStageNumToDataByte);

    GT_U32 *dataPtr;
    GT_U8  tmpStage = stage;

    __LOG(("get address data byte"));


    /* prepare relevant data pointer and temp stage */
    if(isSipLookup)
    {
        if(stage < 16)
        {
            skernelFatalError("convertStageNumToDataByte: got wrong stage");
        }

        tmpStage = stage - 16;
        dataPtr  = descrPtr->sip;
    }
    else
    {
        dataPtr  = descrPtr->dip;
    }


    if(4 == tmpStage)
    { /* stage 4: relevant for ip protocols only (and not fcoe) */

        if((descrPtr->isIp == 0 && descrPtr->arp == 0) || descrPtr->isFcoe)
        { /* not ip */
            skernelFatalError("convertStageNumToDataByte: got wrong stage");
        }
    }
    else if (tmpStage > 4)
    {
        if(!descrPtr->isIp || descrPtr->isIPv4 || descrPtr->isFcoe)
        { /* not ipv6 */
            skernelFatalError("convertStageNumToDataByte: got wrong stage");
        }
    }

    if(!descrPtr->isFcoe)
    {
        *dataBytePtr = 0xFF & (dataPtr[tmpStage/4] >> (24 - (tmpStage % 4) * 8));
    }
    else
    {
        *dataBytePtr = 0xFF & (dataPtr[tmpStage/4] >> (16 - (tmpStage % 4) * 8));
    }

    __LOG(("address data byte [%x]", *dataBytePtr));
}



/*******************************************************************************
*   setNoHitLpmException
*
* DESCRIPTION:
*        set lpm exception
*
* INPUTS:
*        devObjPtr   - pointer to device object
*        descrPtr    - pointer to the frame's descriptor.
*        exception   - exception number
*           In the following cases mark that there was no match on the address (<Hit> = 0):
*           0 = Hit: No exceptions
*           1 = LPM ECC: An non recoverable ECC error was found in the LPM database
*           2 = ECMP ECC: An non recoverable ECC error was found in the ECMP
*                       database and the leaf is from ECMP/Qos type
*           3 = PBR Bucket: Policy Base routing lookup and the entry
*               fetch from the memory is from type Bucket (LPM<PointerType> = Bucket)
*           4 = Continue To Lookup 1: Unicast lookup IPv4/IPv6/FCoE that reach a bucket with <PointToLookup1> set
*           5 = Unicast Lookup 0 IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket
*                   (pass the next DIP LPM stages transparently)
*           6 = Unicast Lookup 1 IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket
*                   (pass the next SIP LPM stages transparently)
*           7 = DST G IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket and <PointToSIP> is unset
*                   (pass the next DIP LPM stages transparently)
*           8 = SRC G IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket
*                   (pass the next SIP LPM stages transparently)
*           9 = Unicast Lookup 0 IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType>
*           10 = Unicast Lookup 1 IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType> is Bucket
*           11 = DST G IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType> is Bucket and <PointToSIP> is unset
*           12 = SRC G IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType> is Bucket
*           13 = FCoE D_ID lookup: After lookup in LPM stage5 the received LPM<PointerType> is Bucket
*                       (pass the next DIP LPM stages transparently)
*           14 = FCoE S_ID lookup: After lookup in LPM stage5 the received LPM<PointerType> is Bucket
*                       (pass the next SIP LPM stages transparently)
*          updateExceptionStatusRegister - GT_TRUE if need to save exception number in register,GT_FALSE otherwise
* OUTPUTS:
*       None
*
* RETURNS:
*
* COMMENTS:
*       INTERNAL: In the event the LPM doesn't find a match, there is an internal configurable command:
*       Router Global Control/Router Global Control1 <TCAM Lookup Not Found Packet Command > with
*       default value HARD DROP. The associated DROP_CODE is
*           IPV4_MC_DIP_NOT_FOUND
*           IPV4_UC_DIP_NOT_FOUND
*           IPV6_MC_DIP_NOT_FOUND
*           IPV6_UC_DIP_NOT_FOUND
*           IPV4_UC_SIP_NOT_FOUND
*           IPV6_UC_SIP_NOT_FOUND
*           FCOE_SIP_NOT_FOUND
*
*******************************************************************************/
static GT_VOID setNoHitLpmException
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32                           exceptionNumber
)
{
    DECLARE_FUNC_NAME(setNoHitLpmException);

    GT_U32 notFoundCommand;

    if(exceptionNumber > 14)
    {
        skernelFatalError("setNoHitLpmException: wrong exception number given [%d]", exceptionNumber);
    }

    if(exceptionNumber)
    {
        __LOG(("No hit, apply TCAM Lookup Not Found Packet Command\n"));

        /* Router Global Control1, TCAM Lookup Not Found Packet Command */
        smemRegFldGet(devObjPtr, SMEM_CHT2_ROUTER_ADDITIONAL_CONTROL_REG(devObjPtr),24,3,&notFoundCommand);

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  notFoundCommand,
                                                  descrPtr->cpuCode,
                                                  0, /* TBD */
                                                  SNET_CHEETAH_ENGINE_UNIT_ROUTER_E,
                                                  GT_FALSE);
    }

    __LOG_PARAM(exceptionNumber);

    smemRegFldSet(devObjPtr, SMEM_LION3_LPM_EXCEPTION_STATUS_REG(devObjPtr,descrPtr->lpmUnitIndex), 0, 4, exceptionNumber);

}

/*******************************************************************************
*   lpmEntryAddrGet
*
* DESCRIPTION:
*        Calc LPM buckets next pointer address from previous next pointer data
*
* INPUTS:
*        devObjPtr   - pointer to device object.
*        descrPtr    - pointer to the frame's descriptor.
*        lpmEntryPtr - lpm entry data pointer
*        ipOctet     - ip octet to search
*
* OUTPUTS:
*       None
*
* RETURNS:
*       next pointer address
*
*******************************************************************************/
static GT_U32 lpmEntryAddrGet
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN SNET_SIP6_LPM_MEM_ENTRY_STC     *lpmEntryPtr,
    IN GT_U8                            ipOctet
)
{
    DECLARE_FUNC_NAME(lpmEntryAddrGet);

    GT_U32 bitVectorLine = 0;
    GT_U32 bitVectorAddr;
    GT_U32 lpmAddr;

    LPM_NODE_TYPE_ENT nodeType = lpmEntryPtr->nodeType;

    __LOG_PARAM(lpmEntryPtr->nextPtr);

    if(nodeType == LPM_NODE_TYPE_REGULAR_E)
    {
        bitVectorLine = ipOctet / 44;
        __LOG(("LPM_NODE_TYPE_REGULAR_E : ipOctet[0x%2.2x] --> bitVectorLine[%d] \n",
            ipOctet,bitVectorLine));
    }
    else if (nodeType == LPM_NODE_TYPE_COMPRESSED_E)
    {
        __LOG(("LPM_NODE_TYPE_COMPRESSED_E : bitVectorLine = 0 \n"));
        bitVectorLine = 0;
    }
    else if(nodeType == LPM_NODE_TYPE_LEAF_E)
    {
        __LOG(("LPM_NODE_TYPE_LEAF_E : bitVectorLine = 0 \n"));
        bitVectorLine = 0;
    }
    else
    {
        skernelFatalError("lpmEntryAddrGet: wrong node type given");
    }

    __LOG_PARAM(bitVectorLine);

    bitVectorAddr = (lpmEntryPtr->nextPtr >> 15) * 1024*32 + (lpmEntryPtr->nextPtr & 0x7FFF) + bitVectorLine;
    __LOG_PARAM(bitVectorAddr);

    lpmAddr = SMEM_FALCON_LPM_MEMORY_TBL_MEM(devObjPtr, bitVectorAddr);

    __LOG_PARAM(lpmAddr);

    return lpmAddr;
}

/*******************************************************************************
*   lpmLeafEntryParse
*
* DESCRIPTION:
*        Parse Leaf Entry
*
* INPUTS:
*        devObjPtr     - pointer to device object
*        descrPtr      - pointer to the frame's descriptor.
*        leafEntryData - leaf Entry data value
*
* OUTPUTS:
*        lpmLeafEntryPtr - pointer to leaf entry structure
*
*******************************************************************************/
static void lpmLeafEntryParse
(
    IN  SKERNEL_DEVICE_OBJECT      *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                      leafEntryData,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC *lpmLeafEntryPtr
)
{
    DECLARE_FUNC_NAME(lpmLeafEntryParse);

    GT_U32                nextHopPointer = 0;
    GT_U32                regAddr;       /* Register address */
    GT_U32                *ecmpEntryPtr;
    LPM_LEAF_TYPE_ENT     leafType;
    GT_U32              startBit;

    /* clear lpmNextEntry first */
    memset(lpmLeafEntryPtr, 0, sizeof(SNET_SIP6_LPM_MEM_LEAF_STC));

    /* get entry type */
    lpmLeafEntryPtr->isTrigger = (GT_BOOL)SMEM_U32_GET_FIELD(leafEntryData, 1, 1);
    __LOG_PARAM(lpmLeafEntryPtr->isTrigger);

    if (lpmLeafEntryPtr->isTrigger)
    {
        nextHopPointer = (GT_U32)SMEM_U32_GET_FIELD(leafEntryData, 2, 20);

        lpmLeafEntryPtr->nextHopPointer = nextHopPointer;
        lpmLeafEntryPtr->isNextNodeCopmressed = (GT_BOOL)SMEM_U32_GET_FIELD(leafEntryData, 22, 1);

        __LOG_PARAM(lpmLeafEntryPtr->nextHopPointer);
        __LOG_PARAM(lpmLeafEntryPtr->isNextNodeCopmressed);

    }
    else /* regular leaf */
    {
        /* get pointer type */
        leafType = (LPM_LEAF_TYPE_ENT)SMEM_U32_GET_FIELD(leafEntryData, 2, 1);
        lpmLeafEntryPtr->leafType = leafType;

        __LOG_PARAM(leafType);


        /* get next hop pointer  */
        nextHopPointer = (GT_U32)SMEM_U32_GET_FIELD(leafEntryData, 7, 15);

        __LOG_PARAM(nextHopPointer);

        if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            lpmLeafEntryPtr->applyPbr = (GT_BOOL)SMEM_U32_GET_FIELD(leafEntryData, 22, 1);
            __LOG_PARAM(lpmLeafEntryPtr->applyPbr);
        }

        switch(leafType)
        {
            case LPM_LEAF_TYPE_MULTIPATH_E:
                /* get ecmp or qos data and then get common leaf data */
                /*regAddr = SMEM_LION3_LPM_ECMP_TBL_MEM(devObjPtr, nextHopPointer, descrPtr->lpmUnitIndex);*/
                /* 2 entries in line */
                regAddr = SMEM_SIP6_IPVX_ECMP_ENTRY_TBL_MEM(devObjPtr, nextHopPointer / 2);
                ecmpEntryPtr = smemMemGet(devObjPtr, regAddr);

                /* 2 entries in line */
                startBit = (nextHopPointer & 1) * 29 ;
                lpmLeafEntryPtr->ecmpOrQosData.nextHopBaseAddr =
                                          (GT_U16)snetFieldValueGet(ecmpEntryPtr, startBit + 0, 15);
                lpmLeafEntryPtr->ecmpOrQosData.numOfPaths =
                                          (GT_U16)snetFieldValueGet(ecmpEntryPtr, startBit + 16, 12);
                lpmLeafEntryPtr->ecmpOrQosData.randomEn =
                                          (GT_BOOL)snetFieldValueGet(ecmpEntryPtr,startBit +  15, 1);
                lpmLeafEntryPtr->ecmpOrQosData.mode =
                                          (LPM_MULTIPATH_MODE_ENT)snetFieldValueGet(ecmpEntryPtr, startBit + 28, 1);

                __LOG_PARAM(lpmLeafEntryPtr->ecmpOrQosData.nextHopBaseAddr);
                __LOG_PARAM(lpmLeafEntryPtr->ecmpOrQosData.numOfPaths);
                __LOG_PARAM(lpmLeafEntryPtr->ecmpOrQosData.randomEn);
                __LOG_PARAM(lpmLeafEntryPtr->ecmpOrQosData.mode);

                /* no need for break here */
                GT_ATTR_FALLTHROUGH;

            case LPM_LEAF_TYPE_REGULAR_E:

                /* get leaf ltt data, relevant for all leafs */
                lpmLeafEntryPtr->lttData.lttUnicastRpfCheckEnable =
                                          (GT_BOOL)SMEM_U32_GET_FIELD(leafEntryData, 3, 1);
                lpmLeafEntryPtr->lttData.lttUnicastSrcAddrCheckMismatchEnable =
                                          (GT_BOOL)SMEM_U32_GET_FIELD(leafEntryData, 4, 1);
                lpmLeafEntryPtr->lttData.lttIpv6MulticastGroupScopeLevel =
                                          (GT_U8)SMEM_U32_GET_FIELD(leafEntryData, 5, 2);

                /* get leaf nextPtr, relevant for all leafs, the same for all leafs */
                lpmLeafEntryPtr->nextHopPointer = nextHopPointer;
                /* get leaf routing priority value */
                lpmLeafEntryPtr->priority = (LPM_LEAF_PRIORITY_ENT)SMEM_U32_GET_FIELD(leafEntryData, 0, 1);

                __LOG_PARAM(lpmLeafEntryPtr->lttData.lttIpv6MulticastGroupScopeLevel);
                __LOG_PARAM(lpmLeafEntryPtr->lttData.lttUnicastRpfCheckEnable);
                __LOG_PARAM(lpmLeafEntryPtr->lttData.lttUnicastSrcAddrCheckMismatchEnable);
                __LOG_PARAM(lpmLeafEntryPtr->nextHopPointer);
                __LOG_PARAM(lpmLeafEntryPtr->priority);

                break;

            default:
                skernelFatalError("lpmLeafEntryParse: wrong leaf type");
        }

    }
    return;
}

/*******************************************************************************
*   lpmLeafEntryGet
*
* DESCRIPTION:
*        Get LPM entry next pointer
*
* INPUTS:
*        devObjPtr    - pointer to device object
*        descrPtr     - pointer to the frame's descriptor.
*        lpmEntryPtr  - pointer to LPM entry
*
* OUTPUTS:
*        lpmLeafEntryPtr - pointer to leaf entry structure
*
* RETURNS:
*        lpm pointer type
*
*******************************************************************************/
static void lpmLeafEntryGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  SNET_SIP6_LPM_MEM_ENTRY_STC     *lpmEntryPtr,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC      *lpmLeafEntryPtr
)
{
    GT_U32                leafEntryData;
    GT_U32                leafEntryAddr;
    GT_U32               *leafEntryPtr;

    /* Fetch LPM next pointer entry */
    leafEntryAddr = lpmEntryAddrGet(devObjPtr, descrPtr, lpmEntryPtr, 0);
    leafEntryPtr = smemMemGet(devObjPtr, leafEntryAddr);
    leafEntryData = snetFieldValueGet(leafEntryPtr, (92 - (lpmEntryPtr->leafNumber*23)), 23);

    lpmLeafEntryParse(devObjPtr,  descrPtr, leafEntryData, lpmLeafEntryPtr);

    return;
}

/*******************************************************************************
*   getLpmLastStageExceptionStatus
*
* DESCRIPTION:
*        return lpm last stage exception status
*
* INPUTS:
*        devObjPtr       - pointer to device object.
*        descrPtr        - pointer to the frame's descriptor.
*        isSipLookup     - indicates is sip lookup
*        lpmEntryPtr     - last stage lpm entry ptr
*
* OUTPUTS:
*
* RETURNS:
*        exceptionStatus - exception status
*
*******************************************************************************/
static GT_U32 getLpmLastStageExceptionStatus
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           isSipLookup,
    INOUT SNET_SIP6_LPM_MEM_ENTRY_STC     *lpmEntryPtr
)
{
    DECLARE_FUNC_NAME(getLpmLastStageExceptionStatus);

    GT_U32  exceptionStatus = 0xFF; /* wrong value by default */

    if(NULL == lpmEntryPtr)
    {
        __LOG(("LPM: got null pointer lpmEntryPtr error, return"));
        return exceptionStatus;
    }

    if(descrPtr->ipm)
    {
        __LOG(("LPM: multicast\n"));

        if(descrPtr->isIp || descrPtr->arp)
        {
            if(descrPtr->isIPv4 ||
                descrPtr->arp)/*ARP Broadcast Mirroring/Trap based on ARP DIP*/
            {
                if(!isSipLookup /* dip lookup */)
                {
                    __LOG(("LPM no hit exception: ipv4, MC, stage 3, not point to leaf  \n"));
                    exceptionStatus = 7;
                }
                else
                {
                    __LOG(("LPM no hit exception: ipv4, MC, stage 19, not point to leaf  \n"));
                    exceptionStatus = 8;
                }
            }
            else
            {
                if(!isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv6, MC, stage 15, got bucket, not point to leaf  \n"));
                    exceptionStatus = 11;
                }
                else
                {
                    __LOG(("LPM no hit exception: ipv6, MC, stage 31, not point to leaf  \n"));
                    exceptionStatus = 12;
                }
            }
        }
    }
    else
    {
        __LOG(("LPM: unicast\n"));

        if(descrPtr->isIp || descrPtr->arp)
        {
            if(descrPtr->isIPv4 ||
                descrPtr->arp)/*ARP Broadcast Mirroring/Trap based on ARP DIP*/
            {
                if(!isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv4, UC, stage 3, not point to leaf  \n"));
                    exceptionStatus = 5;
                }
                else
                {
                    __LOG(("LPM no hit exception: ipv4, UC, stage 19, not point to leaf  \n"));
                    exceptionStatus = 6;
                }
            }
            else
            {
                if(!isSipLookup /* dip lookup */)
                {
                    __LOG(("LPM no hit exception: ipv6, UC, stage 15, not point to leaf  \n"));
                    exceptionStatus = 9;
                }
                else if (isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv6, UC, stage 31, not point to leaf  \n"));
                    exceptionStatus = 10;
                }
            }
        }
        else if(descrPtr->isFcoe)
        {
            if(!isSipLookup /* dip lookup */)
            {
                __LOG(("LPM no hit exception: fcoe, UC, stage 2, got bucket, not point to leaf  \n"));
                exceptionStatus = 13;
            }
            else
            {
                __LOG(("LPM no hit exception: ipv6, UC, stage 18, not point to leaf  \n"));
                exceptionStatus = 14;
            }
        }
    }

    return exceptionStatus;
}

/*******************************************************************************
*   processRegularNode
*
* DESCRIPTION:
*        process regular node
*
* INPUTS:
*        devObjPtr      - pointer to device object.
*        descrPtr       - pointer to the frame's descriptor.
*        lpmEntryAddr   - lpm entry address
*        dataByte       - address data byte
*
* OUTPUTS:
*       lpmEntryPtr     - lpm entry pointer
*
*******************************************************************************/
static void processRegularNode
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr,
    IN  GT_U32                          lpmEntryAddr,
    IN  GT_U32                          dataByte,
    OUT SNET_SIP6_LPM_MEM_ENTRY_STC     *lpmEntryPtr,
    OUT GT_U32                          *isErrorPtr
)
{
    DECLARE_FUNC_NAME(processRegularNode);

    GT_U32  *lpmRegularBitVectorPtr;
    GT_U32 leafsToLeft     = 0;
    GT_U32 regulNodeToLeft = 0;
    GT_U32 comprNodeToLeft = 0;
    GT_U32 leafsNum        = 0;
    GT_U32 regulNodeNum    = 0;
    GT_U32 comprNodeNum    = 0;
    GT_U32 leafNumber      = 0;
    GT_U32 childAddress    = 0;
    GT_U32  i;
    GT_U32 childPointerBase;

    LPM_NODE_TYPE_ENT nodeType         = LPM_NODE_TYPE_EMPTY_E;
    LPM_NODE_TYPE_ENT childType        = LPM_NODE_TYPE_EMPTY_E;
    LPM_NODE_TYPE_ENT currentRangeType = LPM_NODE_TYPE_EMPTY_E;
    GT_U32            didMatch = 0;

    *isErrorPtr = 0;

    lpmRegularBitVectorPtr = smemMemGet(devObjPtr, lpmEntryAddr);

    for(i = 0; i < 44; i++)
    {

        nodeType = snetFieldValueGet(lpmRegularBitVectorPtr, 106 - (i*2), 2);

        if(nodeType != LPM_NODE_TYPE_EMPTY_E)
            currentRangeType = nodeType;

      /*  if(i == dataByte % 44)
        {
            leafsToLeft     = leafsNum;
            regulNodeToLeft = regulNodeNum;
            comprNodeToLeft = comprNodeNum;
            childType = currentRangeType;
        }*/
        if(i == dataByte % 44)
        {
            leafsToLeft     = leafsNum;
            regulNodeToLeft = regulNodeNum;
            comprNodeToLeft = comprNodeNum;
            childType = currentRangeType;
            didMatch = 1;

            if (nodeType == LPM_NODE_TYPE_EMPTY_E)
            {
                switch(currentRangeType)
                {
                    case LPM_NODE_TYPE_LEAF_E:
                        leafsToLeft--;
                        break;
                    case LPM_NODE_TYPE_REGULAR_E:
                        regulNodeToLeft--;
                        break;
                    case LPM_NODE_TYPE_COMPRESSED_E:
                        comprNodeToLeft--;
                        break;
                    case LPM_NODE_TYPE_EMPTY_E:
                        break;

                }
            }
        }



        switch(nodeType)
        {
            case LPM_NODE_TYPE_LEAF_E:
                leafsNum++;
                break;
            case LPM_NODE_TYPE_REGULAR_E:
                regulNodeNum++;
                break;
            case LPM_NODE_TYPE_COMPRESSED_E:
                comprNodeNum++;
                break;
            case LPM_NODE_TYPE_EMPTY_E:
                break;

        }
    }

    if(!didMatch)
    {
        *isErrorPtr = 1;
        __LOG(("configuration error: not matched any byte \n"));
        return;
    }

    childPointerBase = snetFieldValueGet(lpmRegularBitVectorPtr, 0, 20);

    if(childType == LPM_NODE_TYPE_REGULAR_E)
    {
        childAddress = childPointerBase + (6 * regulNodeToLeft);
    }
    else if(childType == LPM_NODE_TYPE_COMPRESSED_E)
    {
        childAddress = childPointerBase + (6 * regulNodeNum) + comprNodeToLeft;
    }
    else if(childType == LPM_NODE_TYPE_LEAF_E)
    {
        childAddress = childPointerBase + (6 * regulNodeNum) + comprNodeNum + leafsToLeft/5;
        leafNumber = leafsToLeft%5;
    }
    else
    {
        *isErrorPtr = 1;
        __LOG(("processCompressedNode: childType was not determined \n"));
        /*skernelFatalError("processCompressedNode: childType was not determined \n");*/
        return;
    }

    lpmEntryPtr->nextPtr = childAddress;
    lpmEntryPtr->nodeType = childType;
    lpmEntryPtr->leafNumber = leafNumber;

    __LOG_PARAM(lpmEntryPtr->nextPtr);
    __LOG_PARAM(lpmEntryPtr->nodeType);
    __LOG_PARAM(lpmEntryPtr->leafNumber);

    return;
}

/*******************************************************************************
*   processCompressedNode
*
* DESCRIPTION:
*        process compressed node
*
* INPUTS:
*        devObjPtr      - pointer to device object.
*        descrPtr       - pointer to the frame's descriptor.
*        lpmEntryAddr   - lpm entry address
*        dataByte       - address data byte
*
* OUTPUTS:
*       lpmEntryPtr     - lpm entry pointer
*       lpmLeafEntryPtr - pointer to leaf entry structure (only relevant for
*                           embedded leaf)
*
* RETURNS:
*        isEmbeddedLeaf - indicates that embedded leaf was read
*
*******************************************************************************/
static GT_BOOL processCompressedNode
(
    IN  SKERNEL_DEVICE_OBJECT  *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                  lpmEntryAddr,
    IN  GT_U32                  dataByte,
    OUT SNET_SIP6_LPM_MEM_ENTRY_STC *lpmEntryPtr,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC *lpmLeafEntryPtr,
    OUT GT_U32                          *isErrorPtr
)
{
    DECLARE_FUNC_NAME(processCompressedNode);

    GT_U32  *lpmCompressedNodePtr;
    GT_U32 leafsToLeft     = 0;
    GT_U32 regulNodeToLeft = 0;
    GT_U32 comprNodeToLeft = 0;
    GT_U32 leafsNum        = 0;
    GT_U32 regulNodeNum    = 0;
    GT_U32 comprNodeNum    = 0;
    GT_U32 embeddedLeafNum = 0;
    GT_U32 leafNumber      = 0;
    GT_U32 childAddress    = 0;
    GT_U32 rangeNum, rangeStart, rangeEnd, childPointerBase;
    GT_U32 leafEntryData;
    GT_U32  i;
    LPM_NODE_TYPE_ENT nodeType         = LPM_NODE_TYPE_EMPTY_E;
    LPM_NODE_TYPE_ENT childType        = LPM_NODE_TYPE_EMPTY_E;
    LPM_NODE_TYPE_ENT currentRangeType = LPM_NODE_TYPE_EMPTY_E;
    GT_BOOL isEmbeddedLeaf = GT_FALSE;
    GT_U32 startBit,tmpValue;
    GT_BOOL didMatch = GT_FALSE;

    *isErrorPtr = 0;

    lpmCompressedNodePtr = smemMemGet(devObjPtr, lpmEntryAddr);

    embeddedLeafNum = snetFieldValueGet(lpmCompressedNodePtr, 112, 2);

    __LOG(("Processing compressed node with %d embedded leafs\n",embeddedLeafNum));

    rangeNum = embeddedLeafNum != 0 ?
        ((9-1) - (2 * embeddedLeafNum)) :/* embedded_1 --> 6 */
                                         /* embedded_2 --> 4 */
                                         /* embedded_3 --> 2 */
        9;
    rangeStart = 0;

    startBit = (rangeNum-1) * 10 + 20 + 2;/* skip 20 bits of <Child Pointer> + 2 bits of 'last' <Child Type>*/

    rangeNum++;/* allow last range for rangeEnd = 256 */

    for(i = 0; i < rangeNum; i++, startBit-=10)
    {
        if(i == (rangeNum-1))
        {
            /*'last' <Child Type>*/
            nodeType = snetFieldValueGet(lpmCompressedNodePtr, 20 , 2);
            rangeEnd = 256;
        }
        else
        {
            tmpValue = snetFieldValueGet(lpmCompressedNodePtr, startBit , 10);
            nodeType = tmpValue >> 8;/* 2 MSbits */
            rangeEnd = tmpValue & 0xFF;/*8 LSBits*/
        }

        if(didMatch == GT_FALSE && /* check that we not 'saved' the info already */
           (rangeStart <= dataByte && dataByte < rangeEnd))
        {
            if(nodeType != LPM_NODE_TYPE_EMPTY_E)
            {
                currentRangeType = nodeType;
            }
            else
            {
                __LOG(("Configuration Error : nodeType == LPM_NODE_TYPE_EMPTY_E \n"));
            }

            leafsToLeft     = leafsNum;
            regulNodeToLeft = regulNodeNum;
            comprNodeToLeft = comprNodeNum;
            childType = currentRangeType;
            __LOG(("Range found:(iteration[%d]) rangeStart = %d rangeEnd = %d rangeType = %d\n",
                i,rangeStart,rangeEnd,currentRangeType));

            /* allow to continue and increment the counters of the GONs */
            didMatch = GT_TRUE;
        }
        /* NOTE : the SW must set 'LPM_NODE_TYPE_EMPTY_E' for cases of rangeStart == rangeEnd */
        /* so the counters will not be incremented */
        switch(nodeType)
        {
            case LPM_NODE_TYPE_LEAF_E:
                leafsNum++;
                break;
            case LPM_NODE_TYPE_REGULAR_E:
                regulNodeNum++;
                break;
            case LPM_NODE_TYPE_COMPRESSED_E:
                comprNodeNum++;
                break;
            case LPM_NODE_TYPE_EMPTY_E:
                break;
        }

        rangeStart = rangeEnd;
    }

    childPointerBase = snetFieldValueGet(lpmCompressedNodePtr, 0, 20);
    __LOG_PARAM(childPointerBase);

    if(childType == LPM_NODE_TYPE_REGULAR_E)
    {
        __LOG(("childType = LPM_NODE_TYPE_REGULAR_E , skip [%d] previous regulars \n",
            regulNodeToLeft));
        childAddress = childPointerBase + (6 * regulNodeToLeft);
    }
    else if(childType == LPM_NODE_TYPE_COMPRESSED_E)
    {
        __LOG(("childType = LPM_NODE_TYPE_COMPRESSED_E , skip [%d] previous regulars , skip [%d] previous compressed \n",
            regulNodeNum,
            comprNodeToLeft));
        childAddress = childPointerBase + (6 * regulNodeNum) + comprNodeToLeft;
    }
    else if(childType == LPM_NODE_TYPE_LEAF_E)
    {
        if (embeddedLeafNum == 0) {

            __LOG(("childType = LPM_NODE_TYPE_LEAF_E , skip [%d] previous regulars , skip [%d] previous compressed , skip [%d] previous leafs \n",
                regulNodeNum,
                comprNodeNum,
                leafsToLeft));

            childAddress = childPointerBase + (6 * regulNodeNum) + comprNodeNum + (leafsToLeft/5);
            leafNumber = leafsToLeft%5;
        }
        else if((leafsToLeft + 1) <= embeddedLeafNum)
        {
            __LOG(("childType = LPM_NODE_TYPE_LEAF_E , from embedded entry , skip [%d] previous leafs\n",
                leafsToLeft));

            leafEntryData = snetFieldValueGet(lpmCompressedNodePtr, (89 - (leafsToLeft*23)) , 23);
            lpmLeafEntryParse(devObjPtr, descrPtr, leafEntryData, lpmLeafEntryPtr);
            leafNumber = leafsToLeft%5;
            isEmbeddedLeaf = GT_TRUE;
        }
        else
        {
            __LOG(("Configuration Error : the current leaf is leaf #[%d] that exceed the number of leafs [%d] in the 'embedded entry'  \n",
            leafsToLeft+1 , embeddedLeafNum));
            *isErrorPtr = 1;
            return 0;
        }
    }
    else
    {
        __LOG(("Configuration Error : wrong child type  \n"));
        *isErrorPtr = 1;
        return 0;
    }
    __LOG_PARAM(childAddress);

    lpmEntryPtr->nextPtr = childAddress;
    lpmEntryPtr->nodeType = childType;
    lpmEntryPtr->leafNumber = leafNumber;

    __LOG_PARAM(lpmEntryPtr->nextPtr);
    __LOG_PARAM(lpmEntryPtr->nodeType);
    __LOG_PARAM(lpmEntryPtr->leafNumber);
    __LOG_PARAM(isEmbeddedLeaf);

    return isEmbeddedLeaf;
}

/*******************************************************************************
*   processSip6LpmStages
*
* DESCRIPTION:
*        search lpm, stages 0-15 or 16-31, depends on isSipLookup param
*
* INPUTS:
*        devObjPtr       - pointer to device object.
*        descrPtr        - pointer to the frame's descriptor.
*        addressDataLen  - address data length
*        isSipLookup     - indicates that stages 0-15 (value 0) or 16-31 (value 1) will be processed
*        lpmEntryPtr     - lpm entry ptr with head of the trie start address and node type
*
* OUTPUTS:
*        lpmLeafEntryPtr - pointer to leaf entry structure
*
*
*******************************************************************************/
static GT_VOID processSip6LpmStages
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U8                            addressDataLen,
    IN    GT_U32                           isSipLookup,
    IN    SNET_SIP6_LPM_MEM_ENTRY_STC     *lpmEntryPtr,
    OUT   SNET_SIP6_LPM_MEM_LEAF_STC      *lpmLeafEntryPtr
)
{
    DECLARE_FUNC_NAME(processSip6LpmStages);

    GT_U8   stage;
    GT_U8   startStage = isSipLookup * SIP6_LPM_NUM_OF_DIP_SIP_STAGES;
    GT_U8   endStage   = startStage  + SIP6_LPM_NUM_OF_DIP_SIP_STAGES;
    GT_U8   dataByte   = 0;
    GT_U32  lpmEntryAddr = 0;
    GT_U32  entryIndex,offsetInBank,bankNumber = 0;
    GT_U32 agingBitNumber,agingEntryIndex = 0;

    GT_U32  isAgingEnabled;
    GT_U32  exceptionStatus = 0xFF; /* wrong value by default */
    GT_U32  isError = 0;
    GT_BOOL embeddedLeaf = GT_FALSE;
    GT_BOOL sipAllZero = GT_FALSE;
    GT_U32  bankSize = 0;
    sipAllZero = checkSip6IfSipIsZero(descrPtr);
    lpmLeafEntryPtr->nextHopPointer = SNET_CHT2_IP_ROUT_NO_MATCH_INDEX_CNS;

    for(stage = startStage; stage < endStage; stage++)
    {
        if(stage >= (startStage + addressDataLen) || lpmEntryPtr->nodeType == LPM_NODE_TYPE_LEAF_E)
        {
            break;
        }

        __LOG(("process address ip octet: stage [%d]", stage));

        /* prepare data byte based on current stage */
        convertStageNumToDataByte(devObjPtr, descrPtr, isSipLookup, stage, &dataByte);

        /* Get LPM entry address */
        lpmEntryAddr = lpmEntryAddrGet(devObjPtr, descrPtr, lpmEntryPtr, dataByte);

        /* Process node */
        if (lpmEntryPtr->nodeType == LPM_NODE_TYPE_REGULAR_E) {
            processRegularNode(devObjPtr, descrPtr, lpmEntryAddr, dataByte, lpmEntryPtr,&isError);
            if(isError)
            {
                __LOG(("Regular Node error \n"));
                exceptionStatus = 1;/* dummy number */
                goto exceptionStatus_lbl;
            }
        }
        else if(lpmEntryPtr->nodeType == LPM_NODE_TYPE_COMPRESSED_E)
        {
            embeddedLeaf = processCompressedNode(devObjPtr, descrPtr, lpmEntryAddr, dataByte, lpmEntryPtr, lpmLeafEntryPtr,&isError);
            if(isError)
            {
                __LOG(("Compressed Node error \n"));
                exceptionStatus = 1;/* dummy number */
                goto exceptionStatus_lbl;
            }
        }
        else
        {
            __LOG(("Configuration Error : wrong lpmEntryPtr->nodeType[%d]  \n",
                lpmEntryPtr->nodeType));
            exceptionStatus = 1;/* dummy number */
            goto exceptionStatus_lbl;
        }
    }


    if(lpmEntryPtr->nodeType != LPM_NODE_TYPE_LEAF_E)
    {
        exceptionStatus = getLpmLastStageExceptionStatus(devObjPtr, descrPtr, isSipLookup, lpmEntryPtr);
    }
    else
    {
        if (embeddedLeaf == GT_FALSE)
        {
            /* read the leaf */
            lpmLeafEntryGet(devObjPtr,  descrPtr, lpmEntryPtr, lpmLeafEntryPtr);
        }

        __LOG(("Get aging enabled value"));
        smemRegFldGet(devObjPtr, SMEM_LION3_LPM_GLOBAL_CONFIG_REG(devObjPtr,descrPtr->lpmUnitIndex), 2, 1, &isAgingEnabled);
        if (sipAllZero == GT_FALSE)
        {
            if (isAgingEnabled)
            {
                if ( ((!descrPtr->ipm) && (!isSipLookup)) ||
                     ((descrPtr->ipm) && (!isSipLookup) && (!lpmLeafEntryPtr->isTrigger)) ||
                     ( (descrPtr->ipm) && (isSipLookup) ) )
                {
                    if (embeddedLeaf == GT_TRUE)
                    {
                        entryIndex = (lpmEntryAddr & 0x00ffffff) >> 4;
                    }
                    else
                    {
                        entryIndex = lpmEntryPtr->nextPtr;
                    }

                    bankNumber = entryIndex/0x8000;
                    offsetInBank = entryIndex%0x8000;

                    if (SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
                    {
                        bankSize = 1536; /* Ironman - L*/
                    }
                    else if (SMEM_CHT_IS_SIP6_15_GET(devObjPtr))
                    {
                        bankSize = _1K; /* AC5X/Harrier */
                    }
                    else if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
                    {
                        bankSize = 10 * _1K; /* AC5P */
                    }
                    else
                    {
                        bankSize = 14 * _1K; /* Falcon */
                    }

                    if (bankNumber <= 28)
                    {
                        agingBitNumber = 5 * (bankNumber * bankSize + offsetInBank) + lpmEntryPtr->leafNumber;
                    }
                    else
                    {
                        /* bank 29 */
                        agingBitNumber = 5 *(28 * bankSize + 640 + offsetInBank) + lpmEntryPtr->leafNumber;
                    }
                    agingEntryIndex = agingBitNumber/32;
                    __LOG(("aging enabled: set the leaf activity bit"));
                    smemRegFldSet(devObjPtr, SMEM_FALCON_LPM_AGING_MEMORY_ENTRY_TBL_MEM(devObjPtr, agingEntryIndex),
                                  agingBitNumber%32, 1, 1);
                }
            }
        }
        exceptionStatus = 0;
    }


    if (lpmLeafEntryPtr->isTrigger == GT_TRUE)
    {
        __LOG(("LPM: got leaf with entry type trigger set, stage [%d] \n", stage));

        if( !descrPtr->ipm)
        {
            __LOG(("LPM no hit exception: UC, any stage, got bucket, and points to sip trie  \n"));
            exceptionStatus = 4;
        }
        else
        {
            __LOG(("this is a MC search and we reached a new root that represents the"
               " source address. Need to continue with a SIP lookup. "));

            exceptionStatus = 0;
        }

    }

exceptionStatus_lbl:
    if(exceptionStatus!=0)
    {
        __LOG(("LPM: set exception status code: %d\n", exceptionStatus));
        setNoHitLpmException(devObjPtr, descrPtr, exceptionStatus);
    }
}

/*******************************************************************************
*   getAddressDataLen
*
* DESCRIPTION:
*        gets address data length
*
* INPUTS:
*        devObjPtr   - pointer to device object.
*        descrPtr    - pointer to the frame's descriptor.
*
* RETURNS:
*        address data length (0 mean error)
*
*******************************************************************************/
static GT_U8 getAddressDataLen
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(getAddressDataLen);

    GT_U8   len = 0;

    /* get relevant register address */
    if(descrPtr->isIp || descrPtr->arp)
    {
        if(descrPtr->isIPv4 ||
            descrPtr->arp)/*ARP Broadcast Mirroring/Trap based on ARP DIP*/
        { /* ipv4 */
            len = 4;
        }
        else
        { /* ipv6 */
            len = 16;
        }
    }
    else if(descrPtr->isFcoe)
    { /* fcoe */
        len = 3;
    }
    else
    { /* unknown protocol */
        skernelFatalError("getAddressDataLen: got unknown lpm lookup");
    }

    __LOG(("address data len: %d", len));

    return len;
}

/*******************************************************************************
*   processSip6PbrAndFetchLeaf
*
* DESCRIPTION:
*        sip6 Fetch PBR Leaf function
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*       lpmLeafPtr - pointer to leaf entry structure
*
* RETURNS:
*       None
*
*******************************************************************************/
static GT_VOID processSip6PbrAndFetchLeaf
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC      *lpmLeafPtr
)
{
    DECLARE_FUNC_NAME(processSip6PbrAndFetchLeaf);

    GT_U32               leafEntryData;
    GT_U32               leafNumber;
    GT_U32               *leafEntryPtr;
    GT_U32               pbrLpmEntryAddr;

    __LOG(("PBR lpm lookup of DIP\n"));
    smemRegFldGet(devObjPtr, SMEM_LION3_LPM_DIRECT_ACCESS_MODE_REG(devObjPtr,descrPtr->lpmUnitIndex), 0, 20, &pbrLpmEntryAddr);

    __LOG(("LPM PBR base address: 0x%8.8x \n", pbrLpmEntryAddr));

    if(descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E ||
       (SMEM_CHT_IS_SIP6_10_GET(devObjPtr) &&
        descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_AND_ASSIGN_VRF_ID_E))
    {
        __LOG_PARAM(descrPtr->pceRoutLttIdx);
        pbrLpmEntryAddr += descrPtr->pceRoutLttIdx >> 3;
        leafNumber = descrPtr->pceRoutLttIdx & 0x7;
    }
    else /*descrPtr->ttiRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E*/
    {
        __LOG_PARAM(descrPtr->ttRouterLTT);
        pbrLpmEntryAddr += descrPtr->ttRouterLTT >> 3;
        leafNumber = descrPtr->ttRouterLTT & 0x7;
    }

    __LOG_PARAM(pbrLpmEntryAddr);
    __LOG_PARAM(leafNumber);

    leafEntryPtr = smemMemGet(devObjPtr, SMEM_FALCON_LPM_MEMORY_TBL_MEM(devObjPtr, pbrLpmEntryAddr));
    leafEntryData = snetFieldValueGet(leafEntryPtr, (92 - (leafNumber*23)), 23);
    lpmLeafEntryParse(devObjPtr,  descrPtr, leafEntryData, lpmLeafPtr);

    if (lpmLeafPtr->isTrigger)
    {
        __LOG(("LPM PBR no hit exception: got bucket\n"));

        setNoHitLpmException(devObjPtr, descrPtr, 3);
    }
}

/*******************************************************************************
*   lpmLookup
*
* DESCRIPTION:
*        sip6 LPM lookup function
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*       isPbrPtr      - pointer to isPbr with values
*                         GT_TRUE  - Policy based routing
*                         GT_FALSE - LPM lookup
*       dipLpmLeafPtr - pointer to dip leaf entry structure
*       sipLpmLeafPtr - pointer to sip leaf entry structure
*
* RETURNS:
*       whether sip lookup was performed
*
*******************************************************************************/
static GT_U32 lpmLookup
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT GT_BOOL                         *isPbrPtr,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC      *dipLpmLeafPtr,
    OUT SNET_SIP6_LPM_MEM_LEAF_STC      *sipLpmLeafPtr
)
{
    DECLARE_FUNC_NAME(lpmLookup);

    GT_U32               headOfTheTrie;
    LPM_NODE_TYPE_ENT    headBucketType;
    GT_U8                addressDataLen    = 0;
    GT_U32               continueSipStages = 0;

    SNET_SIP6_LPM_MEM_ENTRY_STC dipLpmData;
    SNET_SIP6_LPM_MEM_ENTRY_STC sipLpmData;

    memset(dipLpmLeafPtr, 0, sizeof(SNET_SIP6_LPM_MEM_LEAF_STC));
    memset(sipLpmLeafPtr, 0, sizeof(SNET_SIP6_LPM_MEM_LEAF_STC));

    snetSip6CalculateLpmUnitId(devObjPtr,descrPtr);

    /* Get lpm head of the trie and bucket type */
    getHeadOfTheTrieAndBucketType(devObjPtr, descrPtr, &headBucketType, &headOfTheTrie);

    /* get address data length */
    addressDataLen = getAddressDataLen(devObjPtr, descrPtr);

    /* init dip lpm entry with head of the trie */
    dipLpmData.nextPtr = headOfTheTrie;
    dipLpmData.nodeType = headBucketType;

    __LOG_PARAM(dipLpmData.nextPtr);
    __LOG_PARAM(dipLpmData.nodeType);

    /* init sip lpm entry with head of the trie */
    memcpy(&sipLpmData, &dipLpmData, sizeof(SNET_SIP6_LPM_MEM_ENTRY_STC));

    /* initial value */
    *isPbrPtr = GT_FALSE;

    if(descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E ||
       descrPtr->ttiRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E ||
       (SMEM_CHT_IS_SIP6_10_GET(devObjPtr) &&
        descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_AND_ASSIGN_VRF_ID_E))
    {
        if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && descrPtr->pbrMode == SKERNEL_PBR_MODE_PBR_LPM_E)
        {
            __LOG(("PBR + LPM: Regular lpm lookup of DIP\n"));

            /* process DIP lookup (stages  0 - 15) */
            processSip6LpmStages(devObjPtr, descrPtr, addressDataLen, 0/*dip*/, &dipLpmData, dipLpmLeafPtr);
        }

        if(!SMEM_CHT_IS_SIP6_10_GET(devObjPtr) ||
           (SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (descrPtr->pbrMode == SKERNEL_PBR_MODE_PBR_E)))
        {
            *isPbrPtr = GT_TRUE;
            processSip6PbrAndFetchLeaf(devObjPtr, descrPtr, dipLpmLeafPtr);
        }

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (descrPtr->pbrMode == SKERNEL_PBR_MODE_PBR_LPM_E) &&
             (dipLpmLeafPtr->applyPbr == GT_TRUE))
        {
            *isPbrPtr = GT_TRUE;
        }
    }
    else
    {
        __LOG(("Regular (not PBR) lpm lookup of DIP\n"));

        /* process DIP lookup (stages  0 - 15) */
        processSip6LpmStages(devObjPtr, descrPtr, addressDataLen, 0/*dip*/, &dipLpmData, dipLpmLeafPtr);
    }

    /* check if need to continue sip stages */
    if(descrPtr->ipm)
    {
        /* following check is not relevant for pbr (will never happen in pbr) */
        if( dipLpmLeafPtr->isTrigger == GT_TRUE )
        {
            continueSipStages = 1;

            /* pass dip lpm next ptr and bucket type to sip lookup */
            sipLpmData.nextPtr  = dipLpmLeafPtr->nextHopPointer;
            sipLpmData.nodeType = (dipLpmLeafPtr->isNextNodeCopmressed == GT_TRUE) ?
                                        LPM_NODE_TYPE_COMPRESSED_E : LPM_NODE_TYPE_REGULAR_E;
            __LOG(("MC lookup: continue sip stages after dip\n"));
        }
    }
    else
    {
        /* check is SIP lookup not disabled for source ePort */
        continueSipStages =  ! lion3IpvxLocalDevSrcEportBitsGet(devObjPtr, descrPtr, IPVX_PER_SRC_PORT_FIELD_DISABLE_SIP_LOOKUP_E);
    }

    /* process SIP lookup (stages 16 - 31) */
    if(continueSipStages)
    {
        __LOG(("LPM: continue sip stages after dip\n"));

        processSip6LpmStages(devObjPtr, descrPtr, addressDataLen, 1/*sip*/, &sipLpmData, sipLpmLeafPtr);

        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (descrPtr->pbrMode == SKERNEL_PBR_MODE_PBR_LPM_E) &&
             (sipLpmLeafPtr->applyPbr == GT_TRUE))
        {
            *isPbrPtr = GT_TRUE;
        }

        return 1; /* sip lookup performed */
    }

    return 0; /* sip lookup was not performed */
}


/*******************************************************************************
*   snetSip6FetchRouteEntry
*
* DESCRIPTION:
*       process sip/dip ecmp or qos leafs (if any)
*       and update matchIndex array (if nessecary)
*
* INPUTS:
*        devObjPtr     - pointer to device object.
*        descrPtr      - pointer to the frame's descriptor.
*        dipLpmDataPtr - pointer to dip leaf entry structure
*        sipLpmDataPtr - pointer to sip leaf entry structure
*        matchIndexPtr - array to indicate dip sip match
*
* OUTPUTS:
*        matchIndexPtr         - array to indicate dip sip match
*
*******************************************************************************/
GT_VOID snetSip6FetchRouteEntry
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    SNET_SIP6_LPM_MEM_LEAF_STC          *dipLpmDataPtr,
    IN    SNET_SIP6_LPM_MEM_LEAF_STC          *sipLpmDataPtr,
    INOUT GT_U32                              *matchIndexPtr
)
{
    DECLARE_FUNC_NAME(snetSip6FetchRouteEntry);

    GT_U32    idx_match;      /* Index of match  */
    GT_U32    fldValue;       /* field value in QoS register */
    GT_U32    regAddr;
    GT_U32    startBit;
    GT_U32    *ecmpEntryPtr;
    SNET_SIP6_LPM_MEM_LEAF_STC *lookupDataPtr = dipLpmDataPtr;

    for (idx_match = 0; idx_match < SNET_CHT2_TCAM_SEARCH; idx_match++)
    {
        if(matchIndexPtr[idx_match] == SNET_CHT2_IP_ROUT_NO_MATCH_INDEX_CNS)
        {
            continue;
        }

        if((1 == idx_match) || (dipLpmDataPtr->isTrigger == GT_TRUE))
        {
            /* Silicon bug for AC5P and AC5X */
            if(!((SMEM_CHT_IS_SIP6_10_GET(devObjPtr) || SMEM_CHT_IS_SIP6_15_GET(devObjPtr)) &&
                 ((descrPtr->ipm == 1) && (descrPtr->pbrMode == SKERNEL_PBR_MODE_PBR_LPM_E))))
            {
                lookupDataPtr = sipLpmDataPtr;
            }
        }

        /* Overwrite leafType for PBR+LPM scenario */
        if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (descrPtr->pbrMode == SKERNEL_PBR_MODE_PBR_LPM_E) &&
            (lookupDataPtr->applyPbr == GT_TRUE))
        {
            lookupDataPtr->leafType = LPM_LEAF_TYPE_MULTIPATH_E;

            /* get ecmp or qos data and then get common leaf data */
            /*regAddr = SMEM_LION3_LPM_ECMP_TBL_MEM(devObjPtr, nextHopPointer, descrPtr->lpmUnitIndex);*/
            /* 2 entries in line */
            regAddr = SMEM_SIP6_IPVX_ECMP_ENTRY_TBL_MEM(devObjPtr, descrPtr->pceRoutLttIdx / 2);
            ecmpEntryPtr = smemMemGet(devObjPtr, regAddr);

            /* 2 entries in line */
            startBit = (descrPtr->pceRoutLttIdx & 1) * 29 ;
            lookupDataPtr->ecmpOrQosData.nextHopBaseAddr =
                                      (GT_U16)snetFieldValueGet(ecmpEntryPtr, startBit + 0, 15);
            lookupDataPtr->ecmpOrQosData.numOfPaths =
                                      (GT_U16)snetFieldValueGet(ecmpEntryPtr, startBit + 16, 12);
            lookupDataPtr->ecmpOrQosData.randomEn =
                                      (GT_BOOL)snetFieldValueGet(ecmpEntryPtr,startBit +  15, 1);
            lookupDataPtr->ecmpOrQosData.mode =
                                      (LPM_MULTIPATH_MODE_ENT)snetFieldValueGet(ecmpEntryPtr, startBit + 28, 1);

            __LOG_PARAM(lookupDataPtr->ecmpOrQosData.nextHopBaseAddr);
            __LOG_PARAM(lookupDataPtr->ecmpOrQosData.numOfPaths);
            __LOG_PARAM(lookupDataPtr->ecmpOrQosData.randomEn);
            __LOG_PARAM(lookupDataPtr->ecmpOrQosData.mode);
        }

        if (lookupDataPtr->leafType == LPM_LEAF_TYPE_MULTIPATH_E) {
            descrPtr->indirectNhAccess = 1;
            if (LPM_MULTIPATH_MODE_ECMP_E == lookupDataPtr->ecmpOrQosData.mode)
            {
                __LOG(("Calculating Route Entry index in ECMP mode"));
                snetChtEqHashIndexResolution(devObjPtr,descrPtr,
                    lookupDataPtr->ecmpOrQosData.numOfPaths + 1,
                    lookupDataPtr->ecmpOrQosData.randomEn,
                    &matchIndexPtr[idx_match],
                    SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_L3_ECMP_E,
                    NULL,NULL);

                matchIndexPtr[idx_match] += lookupDataPtr->ecmpOrQosData.nextHopBaseAddr;
            }
            else if(LPM_MULTIPATH_MODE_QOS_E == lookupDataPtr->ecmpOrQosData.mode)
            {
                __LOG(("Calculating Route Entry index in QoS mode"));
                regAddr = SMEM_CHT2_QOS_ROUTING_REG(devObjPtr, descrPtr->qos.qosProfile/4);
                smemRegFldGet(devObjPtr, regAddr, (descrPtr->qos.qosProfile%4)*3, 3, &fldValue);

                matchIndexPtr[idx_match] = fldValue * (lookupDataPtr->ecmpOrQosData.numOfPaths + 1) / 8;
                matchIndexPtr[idx_match] += lookupDataPtr->ecmpOrQosData.nextHopBaseAddr;
            }
        }
    }
}

/*******************************************************************************
*   snetFalconLpm
*
* DESCRIPTION:
*        Falcon LPM lookup function
*
* INPUTS:
*        devObjPtr - pointer to device object.
*        descrPtr  - pointer to the frame's descriptor.
*
* OUTPUTS:
*        matchIndexPtr         - array to indicate dip sip match
*        priorityPtr           - routing priority between FDB and LPM lookup
*        isPbrPtr              - pointer to isPbr with values
*                                  GT_TRUE  - Policy Based Routing
*                                  GT_FALSE - LPM lookup
*        ipSecurChecksInfoPtr  - routing security checks information
*
* RETURNS:
*       whether sip lookup was performed
*
*******************************************************************************/
GT_U32 snetFalconLpm
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    OUT GT_U32                              *matchIndexPtr,
    OUT GT_BOOL                             *isPbrPtr,
    OUT LPM_LEAF_PRIORITY_ENT               *priorityPtr,
    OUT SNET_ROUTE_SECURITY_CHECKS_INFO_STC *ipSecurChecksInfoPtr
)
{
    DECLARE_FUNC_NAME(snetFalconLpm);

    SNET_SIP6_LPM_MEM_LEAF_STC    dipLpmData;
    SNET_SIP6_LPM_MEM_LEAF_STC    sipLpmData;
    GT_U32                        isSipLookupPerformed;

    __LOG(("lpm main func"));

    isSipLookupPerformed = lpmLookup(devObjPtr, descrPtr, isPbrPtr, &dipLpmData, &sipLpmData);

    /* get match index */
    if (descrPtr->ipm)
    {
        /* Multicast */
        if (dipLpmData.isTrigger == GT_TRUE)
        {
            /* (S,G) search */
            matchIndexPtr[0] = sipLpmData.nextHopPointer;
        }
        else
        {
            /* (*,G) search */
            matchIndexPtr[0] = dipLpmData.nextHopPointer;
        }
    }
    else
    {
        /* Unicast */
        matchIndexPtr[0] = dipLpmData.nextHopPointer;
        matchIndexPtr[1] = sipLpmData.nextHopPointer;

    }

    /* get security checks info */
    if(SNET_CHT2_IP_ROUT_NO_MATCH_INDEX_CNS != sipLpmData.nextHopPointer)
    {
        ipSecurChecksInfoPtr->rpfCheckMode = (sipLpmData.lttData.lttUnicastRpfCheckEnable) ?
                                               SNET_RPF_VLAN_BASED_E : SNET_RPF_DISABLED_E;
        ipSecurChecksInfoPtr->unicastSipSaCheck =
                        sipLpmData.lttData.lttUnicastSrcAddrCheckMismatchEnable;

        ipSecurChecksInfoPtr->ipv6MulticastGroupScopeLevel =
                        sipLpmData.lttData.lttIpv6MulticastGroupScopeLevel;
    }
    if( (descrPtr->ipm) && (isSipLookupPerformed == 0) )
    {
        /* *,G lookup */
        ipSecurChecksInfoPtr->ipv6MulticastGroupScopeLevel =
            dipLpmData.lttData.lttIpv6MulticastGroupScopeLevel;
    }

    if(LPM_LEAF_TYPE_MULTIPATH_E == sipLpmData.leafType)
    {
        ipSecurChecksInfoPtr->sipFromEcmpOrQosBlock  = 1;

        ipSecurChecksInfoPtr->sipNumberOfPaths       = sipLpmData.ecmpOrQosData.numOfPaths;
        ipSecurChecksInfoPtr->sipBaseRouteEntryIndex = sipLpmData.ecmpOrQosData.nextHopBaseAddr;

        __LOG(("unicast sip sa check is not supported when source IP address is associated with ECMP/QOS block"));
        ipSecurChecksInfoPtr->unicastSipSaCheck = 0;
    }

    if(LPM_LEAF_TYPE_REGULAR_E == dipLpmData.leafType)
    {
       *priorityPtr = dipLpmData.priority;
    }
    else
    {
        *priorityPtr = LPM_LEAF_PRIORITY_FDB_E;
    }

    /* process sip/dip ecmp or qos leafs (if any) and updates matchIndex array (if nessecary) */
    snetSip6FetchRouteEntry(devObjPtr, descrPtr, &dipLpmData, &sipLpmData, /*IN*/
                                                           matchIndexPtr); /*OUT*/

    return isSipLookupPerformed;
}


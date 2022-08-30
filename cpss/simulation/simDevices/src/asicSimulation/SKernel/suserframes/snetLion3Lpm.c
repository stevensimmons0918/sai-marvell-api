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
* @file snetLion3Lpm.c
*
* @brief This is SIP5 LPM engine
*
* @version   17
********************************************************************************
*/
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Lpm.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>


/* number of dip or sip stages in sip5 lpm lookup */
#define SIP5_LPM_NUM_OF_DIP_SIP_STAGES   16

/* indicates whether lpm test enabled */
extern GT_BOOL isLpmEnabled_test;



/**
* @internal checkIfSipIsZero function
* @endinternal
*
* @brief   check if sip address is all zero
*
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @retval GT_TRUE                  - if sip address is all zero
* @retval GT_FALSE                 - otherwise
*/
static GT_BOOL checkIfSipIsZero (
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

/**
* @internal snetSip5CalculateLpmUnitId function
* @endinternal
*
* @brief   Calculate LPM unit id
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] isSipLookup              - GT_TRUE in case of SIP lookup ,GT_FALSE otherwise
*                                       none
*/
static GT_VOID snetSip5CalculateLpmUnitId
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN  GT_BOOL                             isSipLookup
)
{
    GT_U32                          pipeId,localPipeId;/* the PipeId */
    GT_U8                           oldlpmUnitIndex = descrPtr->lpmUnitIndex;

    DECLARE_FUNC_NAME(snetSip5CalculateLpmUnitId);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        pipeId = smemGetCurrentPipeId(devObjPtr);

        localPipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,NULL);

        /* the final address that adds the 'tile offset' comes from
           smemFalconConvertDevAndAddrToNewDevAndAddr(...) */
        descrPtr->lpmUnitIndex = localPipeId;/* 0 or 1 */
    }
    else
    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        pipeId = smemGetCurrentPipeId(devObjPtr);

        switch(pipeId)
        {
            case 0:
                if(isSipLookup == GT_TRUE)
                {
                    descrPtr->lpmUnitIndex = 1;
                }
                else
                {
                    descrPtr->lpmUnitIndex = 0;
                }
             break;
            case 1:
                if(isSipLookup == GT_TRUE)
                {
                    descrPtr->lpmUnitIndex = 3;
                }
                else
                {
                    descrPtr->lpmUnitIndex = 2;
                }
             break;
            default:
             skernelFatalError("snetCalculateLpmUnitId: illegal pipe id\n");
            break;
        }
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

/**
* @internal snetSip5IsLpmHalfMemoryModeSet function
* @endinternal
*
* @brief   Check if lpm memory is at the half memory mode
*
* @param[in] devObjPtr                - pointer to device object
*                                       GT_TRUE in case pm memory is at the half memory mode,FALSE otherwise
*/
static GT_BOOL snetSip5IsLpmHalfMemoryModeSet
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr
)
{   GT_STATUS result = GT_FALSE;
    GT_U32    value;

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) &&
        (SMEM_CHT_IS_SIP6_GET(devObjPtr) == GT_FALSE))
    {
        /* lpm memory mode */
        smemDfxRegFldGet(devObjPtr, SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl15,
            25, 1, &value);

        if(value == 0x0)
        {
            result = GT_TRUE;
        }
    }

    return result;
}

/**
* @internal snetSip5ChangeLpmAddressIfRequired function
* @endinternal
*
* @brief   Change the LPM unit in the HW address in order to access LPM engine 2/3
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] lpmLinePtr               - pointer to lpm line number
* @param[in,out] lpmLinePtr               - pointer to lpm line number
*                                       LPM engine 2/3 is used by pipe 1, and should be accessed if LPM memory
*                                       mode is half LPM memory and the packet is received on pipe 1 (BC3 only).
*                                       In other cases LPM engine 0/1 shuold be accessed.
*/
static GT_VOID snetSip5ChangeLpmAddressIfRequired
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT GT_U32                            *lpmLinePtr
)
{
    GT_U32                          pipeId;/* the PipeId */

    DECLARE_FUNC_NAME(snetSip5ChangeLpmAddressIfRequired);

    if(snetSip5IsLpmHalfMemoryModeSet(devObjPtr) == GT_TRUE)
    {
        pipeId = smemGetCurrentPipeId(devObjPtr);

        __LOG(("Half memory mode is set at LPM. Pipe is %d \n"));

        if(devObjPtr->numOfTiles)
        {
            /* convert the global pipeId to local pipeId */
            pipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,
                pipeId,NULL);
        }

        if(pipeId == 1)
        {
            *lpmLinePtr = ((devObjPtr->memUnitBaseAddrInfo.lpm[2]) | (*lpmLinePtr & 0x00FFFFFF));
        }
    }
}

/**
* @internal smemLion3LpmMemRead function
* @endinternal
*
* @brief   reads lpm memory from sram
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] lpmAddr                  - LPM address in NSRAM
*                                       lpm pointer type
*/
static GT_U32 smemLion3LpmMemRead
(
    IN  SKERNEL_DEVICE_OBJECT  *devObjPtr,
    IN  GT_U32                  lpmAddr
)
{
    DECLARE_FUNC_NAME(smemLion3LpmMemRead);

    GT_U32   *memPtr;

    __LOG(("get lpm data for line [0x%8.8x]",lpmAddr));

    lpmAddr = SMEM_LION3_LPM_MEMORY_TBL_MEM(devObjPtr, lpmAddr);
    snetSip5ChangeLpmAddressIfRequired(devObjPtr, &lpmAddr);
    memPtr = smemMemGet(devObjPtr, lpmAddr);

    return *memPtr;
}

/**
* @internal getHeadOfTheTrieAndBucketType function
* @endinternal
*
* @brief   gets head of the trie and bucket type based on packet type and vrId
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] bucketTypePtr            - pointer to bucket type
* @param[out] headOfTheTriePtr         - pointer to head of the trie
*/
static GT_VOID getHeadOfTheTrieAndBucketType
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT LPM_BUCKET_TYPE_ENT             *bucketTypePtr,
    OUT GT_U32                          *headOfTheTriePtr
)
{
    DECLARE_FUNC_NAME(getHeadOfTheTrieAndBucketType);

    GT_U32 regAddress = 0;
    GT_U32 fldValue   = 0;
    LPM_BUCKET_TYPE_ENT             bucketType;
    GT_U32                          headOfTheTrie;


    __LOG(("Get lpm head of the trie and bucket type"));

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
            regAddress = SMEM_LION3_LPM_IPV4_VRF_ID_TBL_MEM(devObjPtr, descrPtr->vrfId,descrPtr->lpmUnitIndex);
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
    smemRegFldGet(devObjPtr, regAddress, 0, 24, headOfTheTriePtr);
    headOfTheTrie = *headOfTheTriePtr;
    __LOG_PARAM(headOfTheTrie);

    /* get bucket type, length one bit:
       only regular and one compressed bucket can be used in head of the trie */
    smemRegFldGet(devObjPtr, regAddress, 24, 1, &fldValue);
    bucketType = (LPM_BUCKET_TYPE_ENT)fldValue;
    __LOG_PARAM(bucketType);


    *bucketTypePtr = bucketType;
}

/**
* @internal getBypassBitMask function
* @endinternal
*
* @brief   gets stages bypass bit mask
*         Bypass configuration exists only due to "HW limitations":
*         The HW needs for ipv6 lookup more processing time then for Ipv4.
*         So to 'normalize' the processing time,
*         the HW designer choose that IPv4 and IPv6 will take the same time.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] addressDataLen           - address data length
*
* @param[out] bypassBitmaskPtr         - pointer to stages bypass bitmask - 16 bits relevant
*/
static GT_VOID getBypassBitMask
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U8                            addressDataLen,
    OUT GT_U32                          *bypassBitmaskPtr
)
{
    DECLARE_FUNC_NAME(getBypassBitMask);

    GT_U32 isBypassModeEnabled;
    GT_U32 mask16bits = 0;

    __LOG(("Get bypass bit mask"));
    smemRegFldGet(devObjPtr, SMEM_LION3_LPM_GLOBAL_CONFIG_REG(devObjPtr,descrPtr->lpmUnitIndex), 1, 1, &isBypassModeEnabled);

    if(!isBypassModeEnabled)
    {
        __LOG(("bypass mode disabled\n"));
        *bypassBitmaskPtr = 0;
        return;
    }

    /* bypass mode enabled - get bypass bitmask */
    if(descrPtr->isIp || descrPtr->arp)
    {
        if(descrPtr->isIPv4 ||
            descrPtr->arp)/*ARP Broadcast Mirroring/Trap based on ARP DIP*/
        {
            __LOG(("ipv4 lookup"));
            /* stages 4-15 and 20-31 will be bypassed */
        }
        else
        {
            __LOG(("ipv6 lookup"));
            /* stages 4-15 and 20-31 will be bypassed */

            /* this case implementation contradicts to current FS,
                but implemented according designers words, GM behaviour, and BM behaviour.
                the feature is covered by enhanced UT */
            addressDataLen = 4;
        }
    }
    else if(descrPtr->isFcoe)
    {
        __LOG(("FCoE lookup"));
        /* stages 3-15 and 19-31 will be bypassed */
    }
    else
    {
        skernelFatalError("getBypassBitMask: got unknown lpm lookup");
    }

    __LOG_PARAM(addressDataLen);

    mask16bits = (0xFFFF << addressDataLen) & 0xFFFF;

    __LOG(("16 bits mask: [0x%x]\n", mask16bits));
    *bypassBitmaskPtr = (mask16bits << 16) + mask16bits;

    __LOG(("32 bits mask: [0x%x]\n", *bypassBitmaskPtr));

}

/**
* @internal convertStageNumToDataByte function
* @endinternal
*
* @brief   prepares address data byte
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] isSipLookup              - indicates is sip lookup
* @param[in] stage                    - current  value
*
* @param[out] dataBytePtr              - address data byte
*/
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



/**
* @internal setNoHitLpmException function
* @endinternal
*
* @brief   set lpm exception
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      exception   - exception number
*                                      In the following cases mark that there was no match on the address (<Hit> = 0):
*                                      0 = Hit: No exceptions
*                                      1 = LPM ECC: An non recoverable ECC error was found in the LPM database
*                                      2 = ECMP ECC: An non recoverable ECC error was found in the ECMP
*                                      database and the leaf is from ECMP/Qos type
*                                      3 = PBR Bucket: Policy Base routing lookup and the entry
*                                      fetch from the memory is from type Bucket (LPM<PointerType> = Bucket)
*                                      4 = Continue To Lookup 1: Unicast lookup IPv4/IPv6/FCoE that reach a bucket with <PointToLookup1> set
*                                      5 = Unicast Lookup 0 IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket
*                                      (pass the next DIP LPM stages transparently)
*                                      6 = Unicast Lookup 1 IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket
*                                      (pass the next SIP LPM stages transparently)
*                                      7 = DST G IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket and <PointToSIP> is unset
*                                      (pass the next DIP LPM stages transparently)
*                                      8 = SRC G IPv4 packets: After lookup in LPM stage7 the received LPM<PointerType> is Bucket
*                                      (pass the next SIP LPM stages transparently)
*                                      9 = Unicast Lookup 0 IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType>
*                                      10 = Unicast Lookup 1 IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType> is Bucket
*                                      11 = DST G IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType> is Bucket and <PointToSIP> is unset
*                                      12 = SRC G IPv6 packets: After lookup in LPM stage31 the received LPM<PointerType> is Bucket
*                                      13 = FCoE D_ID lookup: After lookup in LPM stage5 the received LPM<PointerType> is Bucket
*                                      (pass the next DIP LPM stages transparently)
*                                      14 = FCoE S_ID lookup: After lookup in LPM stage5 the received LPM<PointerType> is Bucket
*                                      (pass the next SIP LPM stages transparently)
* @param[in] updateExceptionStatusRegister - GT_TRUE if need to save exception number in register,GT_FALSE otherwise
*
* @note INTERNAL: In the event the LPM doesn't find a match, there is an internal configurable command:
*       Router Global Control/Router Global Control1 <TCAM Lookup Not Found Packet Command > with
*       default value HARD DROP. The associated DROP_CODE is
*       IPV4_MC_DIP_NOT_FOUND
*       IPV4_UC_DIP_NOT_FOUND
*       IPV6_MC_DIP_NOT_FOUND
*       IPV6_UC_DIP_NOT_FOUND
*       IPV4_UC_SIP_NOT_FOUND
*       IPV6_UC_SIP_NOT_FOUND
*       FCOE_SIP_NOT_FOUND
*
*/
static GT_VOID setNoHitLpmException
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32                           exceptionNumber,
    IN GT_BOOL                          updateExceptionStatusRegister
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

    if(updateExceptionStatusRegister == GT_TRUE)
    {

        __LOG(("Set exception code"));
        __LOG_PARAM(exceptionNumber);

        smemRegFldSet(devObjPtr, SMEM_LION3_LPM_EXCEPTION_STATUS_REG(devObjPtr,descrPtr->lpmUnitIndex), 0, 4, exceptionNumber);
    }
    else
    {    /*FE-1469243 */
        __LOG(("Warning : Exception code is not saved at register due to errata"));
    }
}

/**
* @internal nextEntryAddrGet function
* @endinternal
*
* @brief   Calculate LPM buckets next pointer structure address for bit vector
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] bitVector                - vector of bits used
* @param[in] bitVectorAddr            - address of bits vector
* @param[in] bucketType               - bucket type
* @param[in] ipOctet                  - ip octet
* @param[in] bvIdx                    - bit vector index
*                                       lpm entry next entry address
*/
static GT_U32 nextEntryAddrGet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 bitVector,
    IN GT_U32                 bitVectorAddr,
    IN LPM_BUCKET_TYPE_ENT    bucketType,
    IN GT_U8                  ipOctet,
    IN GT_U32                 bvIdx
)
{
    DECLARE_FUNC_NAME(nextEntryAddrGet);

    GT_U32 prevCount      = 0; /* num of set bits in the previous lines of the bit vector */
    GT_U32 counter        = 0; /* num of bits set in bit vector */
    GT_U8  currentByteVal = 0; /* current byte value */
    GT_U32 lpmEntryAddr   = 0;

    if (bucketType == LPM_BUCKET_TYPE_REGULAR_E)
    {
        GT_U32 i;
        prevCount = bitVector >> 24;
        bvIdx = ipOctet % 24;

        /* loop on bits include the bvIdx index too */
        for (i = 0; i <= bvIdx; i++)
        {
            counter += (bitVector & 0x01);
            bitVector >>= 1;
        }
    }
    else
    {
        if(bucketType == LPM_BUCKET_TYPE_COMPRESS_1_E)
        {
            prevCount = 1;
        }
        else if(bucketType == LPM_BUCKET_TYPE_COMPRESS_2_E)
        {
            prevCount = (bvIdx == 0) ? 2 : 6;
            /* 6 - is offset from bitVectorAddr and not from the start of the bucket */
        }

        for (counter = 0; counter < 4; counter++)
        {
            currentByteVal = (GT_U8)(bitVector & 0xFF);

            if (currentByteVal > ipOctet ||
                currentByteVal == 0)
            {
                break;
            }

            bitVector >>= 8;
        }
    }

    lpmEntryAddr = bitVectorAddr + prevCount + counter;

    __LOG(("lpmEntryAddr: %x", lpmEntryAddr));

    return lpmEntryAddr;
}


/**
* @internal lpmEntryAddrGet function
* @endinternal
*
* @brief   Calc LPM buckets next pointer address from previous next pointer data
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] lpmEntryPtr              - lpm entry data pointer
* @param[in] ipOctet                  - ip octet to search
*                                       next pointer address
*/
static GT_U32 lpmEntryAddrGet
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN SNET_LPM_MEM_ENTRY_STC          *lpmEntryPtr,
    IN GT_U8                            ipOctet
)
{
    DECLARE_FUNC_NAME(lpmEntryAddrGet);

    GT_U32 bitVectorLine = 0;
    GT_U32 bitVectorAddr;
    GT_U32 bitVector;
    GT_U32 nextLpmEntryAddr;

    LPM_BUCKET_TYPE_ENT bucketType = lpmEntryPtr->data.bucket.bucketType;

    __LOG_PARAM(lpmEntryPtr->nextPtr);

    if(bucketType == LPM_BUCKET_TYPE_REGULAR_E)
    {
        bitVectorLine = ipOctet / 24;
        __LOG(("LPM_BUCKET_TYPE_REGULAR_E : ipOctet[0x%2.2x] --> bitVectorLine[%d] \n",
            ipOctet,bitVectorLine));
    }
    else if (bucketType == LPM_BUCKET_TYPE_COMPRESS_1_E)
    {
        __LOG(("LPM_BUCKET_TYPE_COMPRESS_1_E : bitVectorLine = 0 \n"));
        bitVectorLine = 0;
    }
    else if(bucketType == LPM_BUCKET_TYPE_COMPRESS_2_E)
    {
        if (ipOctet < lpmEntryPtr->data.bucket.twoCompressedFirstLineLastOffset)
        {
            __LOG(("LPM_BUCKET_TYPE_COMPRESS_2_E : bitVectorLine = 0 \n"));
            bitVectorLine = 0;
        }
        else
        {
            __LOG(("LPM_BUCKET_TYPE_COMPRESS_2_E : bitVectorLine = 1 \n"));
            bitVectorLine = 1;
        }
    }
    else
    {
        skernelFatalError("lpmEntryAddrGet: wrong bucket type given");
    }

    bitVectorAddr = lpmEntryPtr->nextPtr + bitVectorLine;

    bitVector = smemLion3LpmMemRead(devObjPtr, bitVectorAddr);

    nextLpmEntryAddr = nextEntryAddrGet(devObjPtr, bitVector, bitVectorAddr,
                                        bucketType, ipOctet, bitVectorLine);

    __LOG(("nextLpmEntryAddr: %x", nextLpmEntryAddr));

    return nextLpmEntryAddr;
}

/**
* @internal lpmEntryGet function
* @endinternal
*
* @brief   Get LPM entry next pointer
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] lpmEntryAddr             - LPM entry address in NSRAM
*
* @param[out] lpmEntryPtr              - pointer to next lpm entry structure
*                                       lpm pointer type
*/
static LPM_POINTER_TYPE_ENT lpmEntryGet
(
    IN  SKERNEL_DEVICE_OBJECT  *devObjPtr,
    IN  GT_U32                  lpmEntryAddr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT SNET_LPM_MEM_ENTRY_STC *lpmEntryPtr
)
{
    DECLARE_FUNC_NAME(lpmEntryGet);

    GT_U32                lpmNextEntry;
    GT_U32                tmpNextPtr = 0;
    GT_U32                regAddr;       /* Register address */
    GT_U32               *ecmpEntryPtr;
    LPM_BUCKET_TYPE_ENT   bucketType;
    LPM_POINTER_TYPE_ENT  pointerType;
    GT_BOOL               pointsToSipTree;

    /* clear lpmNextEntry first */
    memset(lpmEntryPtr, 0, sizeof(SNET_LPM_MEM_ENTRY_STC));

    /* Fetch LPM next pointer entry */
    lpmNextEntry = smemLion3LpmMemRead(devObjPtr, lpmEntryAddr);

    /* get pointer type */
    pointerType = (LPM_POINTER_TYPE_ENT)SMEM_U32_GET_FIELD(lpmNextEntry, 0, 2);
    lpmEntryPtr->pointerType = pointerType;

    __LOG_PARAM(pointerType);

    if(LPM_POINTER_TYPE_BUCKET_E != pointerType)
    {
        /* get next ptr value, relevant for leafs only */
        tmpNextPtr = (GT_U32)SMEM_U32_GET_FIELD(lpmNextEntry, 7, 15);

        __LOG_PARAM(tmpNextPtr);
    }

    switch(pointerType)
    {
        case LPM_POINTER_TYPE_BUCKET_E:
            /* get bucket type */
            bucketType = (LPM_BUCKET_TYPE_ENT)SMEM_U32_GET_FIELD(lpmNextEntry, 3, 2);
            lpmEntryPtr->data.bucket.bucketType = bucketType;
            __LOG_PARAM(bucketType);

            /* check if the next bucket is a new root */
            pointsToSipTree = (LPM_POINTER_TYPE_ENT)SMEM_U32_GET_FIELD(lpmNextEntry, 2, 1);
            lpmEntryPtr->data.bucket.pointToSipTrie = pointsToSipTree;
            __LOG_PARAM(pointsToSipTree);

            /* get bucket data */
            if (bucketType == LPM_BUCKET_TYPE_REGULAR_E ||
                bucketType == LPM_BUCKET_TYPE_COMPRESS_1_E)
            {
                lpmEntryPtr->nextPtr = (GT_U32)SMEM_U32_GET_FIELD(lpmNextEntry,  5, 24);
            }
            else if (bucketType == LPM_BUCKET_TYPE_COMPRESS_2_E)
            {
                lpmEntryPtr->nextPtr = (GT_U32)SMEM_U32_GET_FIELD(lpmNextEntry, 13, 19);
                lpmEntryPtr->data.bucket.twoCompressedFirstLineLastOffset =
                                        (GT_U8)SMEM_U32_GET_FIELD(lpmNextEntry,  5,  8);
            __LOG_PARAM(lpmEntryPtr->data.bucket.twoCompressedFirstLineLastOffset);
            }
            else
            {
                skernelFatalError("lpmEntryGet: wrong bucket type");
            }
            __LOG_PARAM(lpmEntryPtr->nextPtr);

            break;

        case LPM_POINTER_TYPE_ECMP_LEAF_E:
        case LPM_POINTER_TYPE_QOS_LEAF_E:
            /* get ecmp or qos data and then get common leaf data */
            regAddr = SMEM_LION3_LPM_ECMP_TBL_MEM(devObjPtr, tmpNextPtr,descrPtr->lpmUnitIndex);
            ecmpEntryPtr = smemMemGet(devObjPtr, regAddr);

            lpmEntryPtr->data.leaf.ecmpOrQosData.nextHopBaseAddr =
                                      (GT_U16)snetFieldValueGet(ecmpEntryPtr,  0, 15);
            /* ERROR in Cider : numOfPaths in bits 15..26 , randomEn - bit 27 */
            /* should be: numOfPaths in bits 16..27 , randomEn - bit 15 */
            lpmEntryPtr->data.leaf.ecmpOrQosData.numOfPaths =
                                      (GT_U16)snetFieldValueGet(ecmpEntryPtr, 16, 12);
            lpmEntryPtr->data.leaf.ecmpOrQosData.randomEn =
                                      (GT_BOOL)snetFieldValueGet(ecmpEntryPtr, 15, 1);

            __LOG_PARAM(lpmEntryPtr->data.leaf.ecmpOrQosData.nextHopBaseAddr);
            __LOG_PARAM(lpmEntryPtr->data.leaf.ecmpOrQosData.numOfPaths);
            __LOG_PARAM(lpmEntryPtr->data.leaf.ecmpOrQosData.randomEn);

            /* no need for break here */
            GT_ATTR_FALLTHROUGH;

        case LPM_POINTER_TYPE_REGULAR_LEAF_E:

            /* get leaf ltt data, relevant for all leafs */
            lpmEntryPtr->data.leaf.lttData.lttIpv6MulticastGroupScopeLevel =
                                      (GT_U8)SMEM_U32_GET_FIELD(lpmNextEntry, 2, 2);
            lpmEntryPtr->data.leaf.lttData.lttUnicastRpfCheckEnable =
                                      (GT_BOOL)SMEM_U32_GET_FIELD(lpmNextEntry, 4, 1);
            lpmEntryPtr->data.leaf.lttData.lttUnicastSrcAddrCheckMismatchEnable =
                                      (GT_BOOL)SMEM_U32_GET_FIELD(lpmNextEntry, 5, 1);

            /* get leaf nextPtr, relevant for all leafs, the same for all leafs */
            lpmEntryPtr->nextPtr = tmpNextPtr;

            __LOG_PARAM(lpmEntryPtr->data.leaf.lttData.lttIpv6MulticastGroupScopeLevel);
            __LOG_PARAM(lpmEntryPtr->data.leaf.lttData.lttUnicastRpfCheckEnable);
            __LOG_PARAM(lpmEntryPtr->data.leaf.lttData.lttUnicastSrcAddrCheckMismatchEnable);
            __LOG_PARAM(lpmEntryPtr->nextPtr);

            break;

        default:
            skernelFatalError("lpmEntryGet: wrong pointer type");
    }

    return pointerType;
}

/**
* @internal getLpmLastStageExceptionStatus function
* @endinternal
*
* @brief   return lpm last stage exception status
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] isSipLookup              - indicates is sip lookup
* @param[in,out] lpmEntryPtr              - last stage lpm entry ptr
*
* @retval exceptionStatus          - exception status
*/
static GT_U32 getLpmLastStageExceptionStatus
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           isSipLookup,
    INOUT SNET_LPM_MEM_ENTRY_STC          *lpmEntryPtr
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
                if(!isSipLookup /* dip lookup */ &&
                    lpmEntryPtr->data.bucket.pointToSipTrie == GT_FALSE)
                {
                    __LOG(("LPM no hit exception: ipv4, MC, stage 3, got bucket, and not points to sip trie  \n"));
                    exceptionStatus = 7;
                }
                else if (isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv4, MC, stage 19, got bucket  \n"));
                    exceptionStatus = 8;
                }
            }
            else
            {
                if(!isSipLookup /* dip lookup */ &&
                    lpmEntryPtr->data.bucket.pointToSipTrie == GT_FALSE)
                {
                    __LOG(("LPM no hit exception: ipv6, MC, stage 15, got bucket, and not points to sip trie  \n"));
                    exceptionStatus = 11;
                }
                else if (isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv6, MC, stage 31, got bucket  \n"));
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
                if(!isSipLookup /* dip lookup */ &&
                    lpmEntryPtr->data.bucket.pointToSipTrie == GT_FALSE)
                {
                    __LOG(("LPM no hit exception: ipv4, UC, stage 3, got bucket, and not points to sip trie  \n"));
                    exceptionStatus = 5;
                }
                else if (isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv4, UC, stage 19, got bucket  \n"));
                    exceptionStatus = 6;
                }
            }
            else
            {
                if(!isSipLookup /* dip lookup */ &&
                    lpmEntryPtr->data.bucket.pointToSipTrie == GT_FALSE)
                {
                    __LOG(("LPM no hit exception: ipv6, UC, stage 15, got bucket, and not points to sip trie  \n"));
                    exceptionStatus = 9;
                }
                else if (isSipLookup)
                {
                    __LOG(("LPM no hit exception: ipv6, UC, stage 31, got bucket  \n"));
                    exceptionStatus = 10;
                }
            }
        }
        else if(descrPtr->isFcoe)
        {
            if(!isSipLookup /* dip lookup */)
            {
                __LOG(("LPM no hit exception: fcoe, UC, stage 2, got bucket, and not points to sip trie  \n"));
                exceptionStatus = 13;
            }
            else
            {
                __LOG(("LPM no hit exception: ipv6, UC, stage 18, got bucket  \n"));
                exceptionStatus = 14;
            }
        }
    }

    return exceptionStatus;
}

/**
* @internal processLpmStages function
* @endinternal
*
* @brief   search lpm, stages 0-15 or 16-31, depends on isSipLookup param
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] addressDataLen           - address data length
* @param[in] bypassBitmask            - pointer to stages bypass bitmask - 32 bits relevant
* @param[in] isSipLookup              - indicates that stages 0-15 (value 0) or 16-31 (value 1) will be processed
* @param[in,out] lpmEntryPtr              - lpm entry ptr with head of the trie start address and bucket type
* @param[in,out] lpmEntryPtr              - pointer to next lpm entry
*
* @retval nextHopFound             - indicates that lookup success or not
*/
static GT_VOID processLpmStages
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U8                            addressDataLen,
    IN    GT_U32                           bypassBitmask,
    IN    GT_U32                           isSipLookup,
    INOUT SNET_LPM_MEM_ENTRY_STC          *lpmEntryPtr
)
{
    DECLARE_FUNC_NAME(processLpmStages);

    GT_U8   stage;
    GT_U8   startStage = isSipLookup * SIP5_LPM_NUM_OF_DIP_SIP_STAGES;
    GT_U8   endStage   = startStage  + SIP5_LPM_NUM_OF_DIP_SIP_STAGES;
    GT_U8   dataByte   = 0;
    GT_U32  lpmEntryAddr = 0;
    GT_U32  isAgingEnabled;
    GT_U32  exceptionStatus = 0xFF; /* wrong value by default */
    GT_BOOL updateExceptionStatusRegister = GT_TRUE;
    GT_BOOL sipAllZero = GT_FALSE;
    LPM_POINTER_TYPE_ENT lastRelevantStagePointerType = LPM_POINTER_TYPE_BUCKET_E;
    SNET_LPM_MEM_ENTRY_STC *lastRelevantLpmEntryPtr = NULL;

    if(isLpmEnabled_test)
    { /* need for testing purposes only */
        if(isSipLookup)
        {
            return;
        }
    }
    sipAllZero = checkIfSipIsZero(descrPtr);
    for(stage = startStage; stage < endStage; stage++)
    {
        if(!bypassBitmask)
        { /* bypass mode disabled */

            if(stage >= (startStage + addressDataLen))
            {
                /* No need to process stage - do access to memory,
                   just waste of time, results are not relevant

                   This can happen only if result (leaf) not found
                */
                lpmEntryAddr = lpmEntryAddrGet(devObjPtr,descrPtr, lpmEntryPtr, dataByte);

                lpmEntryGet(devObjPtr, lpmEntryAddr,descrPtr, lpmEntryPtr);
                continue;
            }
        }
        else if( (bypassBitmask >> stage) & 0x01 )
        { /* bypass mode enabled */

            __LOG(("bypass this stage according to configuration: stage [%d]", stage));
            continue;
        }

        __LOG(("process address ip octet: stage [%d]", stage));

        /* prepare data byte based on current stage */
        convertStageNumToDataByte(devObjPtr, descrPtr, isSipLookup, stage, &dataByte);

        /* Get next LPM entry address */
        lpmEntryAddr = lpmEntryAddrGet(devObjPtr, descrPtr, lpmEntryPtr, dataByte);

        /* Clear lpmEntryPtr and get next LPM entry data */
        lastRelevantStagePointerType = lpmEntryGet(devObjPtr, lpmEntryAddr,descrPtr, lpmEntryPtr);

        lastRelevantLpmEntryPtr = lpmEntryPtr; /* save the pointer */
        if(LPM_POINTER_TYPE_BUCKET_E != lastRelevantStagePointerType)
        {
            /* got leaf (and not bucket), done */
            __LOG(("got leaf: stage [%d], isSip [%d]", stage, isSipLookup));

            if(devObjPtr->errata.ipLpmActivityStateDoesNotWorks)
            {
                __LOG(("WARNING : ERRATA : no activity state marking performed\n"));
            }
            else
            {
                __LOG(("Get aging enabled value"));
                smemRegFldGet(devObjPtr, SMEM_LION3_LPM_GLOBAL_CONFIG_REG(devObjPtr,descrPtr->lpmUnitIndex), 2, 1, &isAgingEnabled);
                if (sipAllZero == GT_FALSE)
                {
                    if(isAgingEnabled)
                    {
                        if (!((!descrPtr->ipm) && (isSipLookup)))
                        {
                            __LOG(("aging enabled: set the leaf activity bit"));
                            smemRegFldSet(devObjPtr, SMEM_LION3_LPM_MEMORY_TBL_MEM(devObjPtr, lpmEntryAddr), 6, 1, 1);
                        }
                    }
                }
            }


            __LOG(("LPM hit: clear exception status\n"));
            exceptionStatus = 0;

            break;
        }
        else
        {
            __LOG(("LPM: got bucket\n"));

            if (lpmEntryPtr->data.bucket.pointToSipTrie == GT_TRUE)
            {
                __LOG(("LPM: got bucket with pointToSipTrie set, stage [%d] \n", stage));

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

                break;
            }
        }
    }

    if(stage == endStage &&
       lastRelevantStagePointerType == LPM_POINTER_TYPE_BUCKET_E)
    {
        exceptionStatus = getLpmLastStageExceptionStatus(devObjPtr, descrPtr, isSipLookup, lastRelevantLpmEntryPtr);
    }
    /*
        The LPM performs IPv4/IPv6 and FCOE address lookups. A bypass mode is implemented for low latency IPv4/FCOE
        lookups. In this mode, the LPM does not perform IPv6 lookups for addresses longer than 4 bytes. Addresses longer
        than 4 bytes are returned by the LPM as a no-hit result and the IPvX operates according to the No Hit command.
        However, due to this erratum, the LPM/LPM Exception Status register indicates that a hit was achieved. In the case of
        an IPv6 lookup, there is no exception indication.
        */
    if(exceptionStatus == 9)
    {
        if((devObjPtr->errata.lpmExceptionStatusForAdressGreaterThen4B == 1)&&(bypassBitmask == 0xfff0fff0))
        {
            updateExceptionStatusRegister = GT_FALSE;
        }
    }

    if(exceptionStatus!=0)
    {

        __LOG(("LPM: set exception status code: %d\n", exceptionStatus));
        setNoHitLpmException(devObjPtr, descrPtr, exceptionStatus,updateExceptionStatusRegister);
    }
}

/**
* @internal getAddressDataLen function
* @endinternal
*
* @brief   gets address data length
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                       address data length (0 mean error)
*/
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

/**
* @internal lpmLookup function
* @endinternal
*
* @brief   sip5 LPM lookup function
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] dipLpmDataPtr            - dip lpm entry data
* @param[out] sipLpmDataPtr            - dip lpm entry data
*                                       whether sip lookup was performed
*/
static GT_U32 lpmLookup
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT SNET_LPM_MEM_ENTRY_STC          *dipLpmDataPtr,
    OUT SNET_LPM_MEM_ENTRY_STC          *sipLpmDataPtr
)
{
    DECLARE_FUNC_NAME(lpmLookup);

    GT_U32               headOfTheTrie;
    LPM_BUCKET_TYPE_ENT  headBucketType;
    GT_U32               bypassBitmask     = 0;
    GT_U8                addressDataLen    = 0;
    GT_U32               continueSipStages = 0;
    GT_U32               pbrLpmEntryAddr;

    snetSip5CalculateLpmUnitId(devObjPtr,descrPtr,GT_FALSE);

    /* Get lpm head of the trie and bucket type */
    getHeadOfTheTrieAndBucketType(devObjPtr, descrPtr, &headBucketType, &headOfTheTrie);

    /* get address data length */
    addressDataLen = getAddressDataLen(devObjPtr, descrPtr);

    /* gets stages bypass bit mask */
    getBypassBitMask(devObjPtr, descrPtr, addressDataLen, &bypassBitmask);

    /* clear dip lpm entry data first */
    memset(dipLpmDataPtr, 0, sizeof(SNET_LPM_MEM_ENTRY_STC));

    /* init dip lpm entry with head of the trie */
    dipLpmDataPtr->nextPtr = headOfTheTrie;
    dipLpmDataPtr->data.bucket.bucketType = headBucketType;

    __LOG_PARAM(dipLpmDataPtr->nextPtr);
    __LOG_PARAM(dipLpmDataPtr->data.bucket.bucketType);

    /* init sip lpm entry with head of the trie */
    memcpy(sipLpmDataPtr, dipLpmDataPtr, sizeof(SNET_LPM_MEM_ENTRY_STC));


    if(descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E ||
       descrPtr->ttiRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E)
    {
        __LOG(("PBR lpm lookup of DIP\n"));
        smemRegFldGet(devObjPtr, SMEM_LION3_LPM_DIRECT_ACCESS_MODE_REG(devObjPtr,descrPtr->lpmUnitIndex), 0, 19, &pbrLpmEntryAddr);

        __LOG(("LPM PBR base address: 0x%8.8x \n", pbrLpmEntryAddr));

        if(descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E)
        {
            __LOG_PARAM(descrPtr->pceRoutLttIdx);
            pbrLpmEntryAddr += descrPtr->pceRoutLttIdx;
        }
        else /*descrPtr->ttiRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E*/
        {
            __LOG_PARAM(descrPtr->ttRouterLTT);
            pbrLpmEntryAddr += descrPtr->ttRouterLTT;
        }

        __LOG_PARAM(pbrLpmEntryAddr);

        if(LPM_POINTER_TYPE_BUCKET_E == lpmEntryGet(devObjPtr, pbrLpmEntryAddr,descrPtr, /*OUT only*/dipLpmDataPtr))
        {
            __LOG(("LPM PBR no hit exception: got bucket\n"));

            setNoHitLpmException(devObjPtr, descrPtr, 3,GT_TRUE);
        }
    }
    else
    {
        __LOG(("Regular (not PBR) lpm lookup of DIP\n"));

        /* process DIP lookup (stages  0 - 15) */
        processLpmStages(devObjPtr, descrPtr, addressDataLen, bypassBitmask, 0/*dip*/, dipLpmDataPtr);
    }

    /* check if need to continue sip stages */
    if(descrPtr->ipm)
    {
        /* following check is not relevant for pbr (will never happen in pbr) */
        if( (LPM_POINTER_TYPE_BUCKET_E == dipLpmDataPtr->pointerType) &&
             dipLpmDataPtr->data.bucket.pointToSipTrie )
        {
            continueSipStages = 1;

            /* pass dip lpm next ptr and bucket type to sip lookup */
            sipLpmDataPtr->nextPtr                = dipLpmDataPtr->nextPtr;
            sipLpmDataPtr->data.bucket.bucketType = dipLpmDataPtr->data.bucket.bucketType;
            sipLpmDataPtr->data.bucket.twoCompressedFirstLineLastOffset = dipLpmDataPtr->data.bucket.twoCompressedFirstLineLastOffset;

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

        snetSip5CalculateLpmUnitId(devObjPtr,descrPtr,GT_TRUE);

        processLpmStages(devObjPtr, descrPtr, addressDataLen, bypassBitmask,
                                                   1/*sip*/, sipLpmDataPtr);
        return 1; /* sip lookup performed */
    }

    return 0; /* sip lookup was not performed */
}

/**
* @internal snetLion3LpmPrefixAdd_test function
* @endinternal
*
* @brief   lion3 LPM lookup test function
*/
GT_VOID snetLion3LpmPrefixAdd_test
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    GT_U32  ipOctet     = 1;

    GT_U32  bvVal       = 0;
    GT_U32  entryVal    = 0;

    GT_U32  bucketAddr   = 0;
    GT_U32  lpmEntryAddr = 0;

    GT_U32  regAddr  = 0;

    GT_U32  i = 0;

    for(i = 0; i < 4; i++)
    {
        if(3 == i)
        {
            ipOctet = 3;
        }

        /* write bv */
        regAddr = SMEM_LION3_LPM_MEMORY_TBL_MEM(devObjPtr, bucketAddr);
        bvVal   = 1 << ipOctet;
        smemMemSet(devObjPtr, regAddr, &bvVal, 1);

        /* write lpm entry */
        lpmEntryAddr = bucketAddr + 12 + 1 /* always first range used */;

        bucketAddr += 12 + 256;       /* get next bucket addr */
        entryVal   = bucketAddr << 5; /* set bucket ptr to lpm entry */

        if(3 == i)
        {
            /* write byte 4  - next hop */
            entryVal = 3 << 7; /* set nex hop ptr to lpm entry */
            entryVal |= 1;     /* set pointer type to regular leaf */
        }

        regAddr = SMEM_LION3_LPM_MEMORY_TBL_MEM(devObjPtr, lpmEntryAddr);
        smemMemSet(devObjPtr, regAddr, &entryVal, 1);
    }
}

/**
* @internal snetSip5FetchRouteEntry function
* @endinternal
*
* @brief   process sip/dip ecmp or qos leafs (if any)
*         and update matchIndex array (if nessecary)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] dipLpmDataPtr            - pointer to the dip info
* @param[in] sipLpmDataPtr            - pointer to the sip info
* @param[in,out] matchIndexPtr            - array to indicate dip sip match
* @param[in,out] matchIndexPtr            - array to indicate dip sip match
*/
static GT_VOID snetSip5FetchRouteEntry
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    SNET_LPM_MEM_ENTRY_STC              *dipLpmDataPtr,
    IN    SNET_LPM_MEM_ENTRY_STC              *sipLpmDataPtr,
    INOUT GT_U32                              *matchIndexPtr
)
{
    DECLARE_FUNC_NAME(snetSip5FetchRouteEntry);

    GT_U32    idx_match;      /* Index of match  */
    GT_U32    fldValue;       /* field value in QoS register */
    GT_U32    regAddr;

    SNET_LPM_MEM_ENTRY_STC *lookupDataPtr = dipLpmDataPtr;


    for (idx_match = 0; idx_match < SNET_CHT2_TCAM_SEARCH; idx_match++)
    {
        if(matchIndexPtr[idx_match] == SNET_CHT2_IP_ROUT_NO_MATCH_INDEX_CNS)
        {
            continue;
        }

        if(1 == idx_match)
        {
            lookupDataPtr = sipLpmDataPtr;
        }

        if(LPM_POINTER_TYPE_ECMP_LEAF_E == lookupDataPtr->pointerType)
        {
            __LOG(("Calculating Route Entry index in ECMP mode"));
            if (SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
            {
                descrPtr->indirectNhAccess = 1;
            }
            snetChtEqHashIndexResolution(devObjPtr,descrPtr,
                lookupDataPtr->data.leaf.ecmpOrQosData.numOfPaths + 1,
                lookupDataPtr->data.leaf.ecmpOrQosData.randomEn,
                &matchIndexPtr[idx_match],
                SNET_CHT_EQ_HASH_INDEX_RESOLUTION_INSTANCE_L3_ECMP_E,
                NULL,NULL);

            matchIndexPtr[idx_match] += lookupDataPtr->data.leaf.ecmpOrQosData.nextHopBaseAddr;
        }
        else if(LPM_POINTER_TYPE_QOS_LEAF_E == lookupDataPtr->pointerType)
        {
            __LOG(("Calculating Route Entry index in QoS mode"));
            if (SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
            {
                descrPtr->indirectNhAccess = 1;
            }
            regAddr = SMEM_CHT2_QOS_ROUTING_REG(devObjPtr, descrPtr->qos.qosProfile/4);
            smemRegFldGet(devObjPtr, regAddr, (descrPtr->qos.qosProfile%4)*3, 3, &fldValue);

            matchIndexPtr[idx_match] = fldValue * (lookupDataPtr->data.leaf.ecmpOrQosData.numOfPaths + 1) / 8;
            matchIndexPtr[idx_match] += lookupDataPtr->data.leaf.ecmpOrQosData.nextHopBaseAddr;
        }
    }
}

/**
* @internal snetLion3Lpm function
* @endinternal
*
* @brief   lion3 LPM lookup function
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] matchIndexPtr            - array to indicate dip sip match
* @param[out] ipSecurChecksInfoPtr     - routing security checks information
*                                       whether sip lookup was performed
*/
GT_U32 snetLion3Lpm
(
    IN  SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    OUT GT_U32                              *matchIndexPtr,
    OUT SNET_ROUTE_SECURITY_CHECKS_INFO_STC *ipSecurChecksInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3Lpm);

    SNET_LPM_MEM_ENTRY_STC    dipLpmData;
    SNET_LPM_MEM_ENTRY_STC    sipLpmData;

    GT_U32 isSipLookupPerformed;

    __LOG(("lpm main func"));

    isSipLookupPerformed = lpmLookup(devObjPtr, descrPtr, &dipLpmData, &sipLpmData);

    /* get match index */
    if (descrPtr->ipm)
    {
        /* Multicast */
        if( (LPM_POINTER_TYPE_BUCKET_E == dipLpmData.pointerType) &&
             (GT_TRUE == dipLpmData.data.bucket.pointToSipTrie) )
        {
            /* (S,G) search */
            matchIndexPtr[0] = sipLpmData.nextPtr;
        }
        else
        {
            /* (*,G) search */
            matchIndexPtr[0] = dipLpmData.nextPtr;
        }
    }
    else
    {
        /* Unicast */
        if (LPM_POINTER_TYPE_BUCKET_E != dipLpmData.pointerType)
        {
            matchIndexPtr[0] = dipLpmData.nextPtr;
        }
        if (LPM_POINTER_TYPE_BUCKET_E != sipLpmData.pointerType)
        {
            matchIndexPtr[1] = sipLpmData.nextPtr;
        }
    }

    /* get security checks info */
    if(LPM_POINTER_TYPE_BUCKET_E != sipLpmData.pointerType)
    {
        ipSecurChecksInfoPtr->rpfCheckMode = (sipLpmData.data.leaf.lttData.lttUnicastRpfCheckEnable) ?
                                               SNET_RPF_VLAN_BASED_E : SNET_RPF_DISABLED_E;
        ipSecurChecksInfoPtr->unicastSipSaCheck =
                        sipLpmData.data.leaf.lttData.lttUnicastSrcAddrCheckMismatchEnable;

        ipSecurChecksInfoPtr->ipv6MulticastGroupScopeLevel =
                        sipLpmData.data.leaf.lttData.lttIpv6MulticastGroupScopeLevel;
    }

    if( (descrPtr->ipm) && (isSipLookupPerformed == 0) )
    {
        /* *,G lookup */
        ipSecurChecksInfoPtr->ipv6MulticastGroupScopeLevel = 
            dipLpmData.data.leaf.lttData.lttIpv6MulticastGroupScopeLevel;
    }

    if((LPM_POINTER_TYPE_ECMP_LEAF_E == sipLpmData.pointerType) ||
       (LPM_POINTER_TYPE_QOS_LEAF_E  == sipLpmData.pointerType))
    {
        ipSecurChecksInfoPtr->sipFromEcmpOrQosBlock  = 1;

        ipSecurChecksInfoPtr->sipNumberOfPaths       = sipLpmData.data.leaf.ecmpOrQosData.numOfPaths;
        ipSecurChecksInfoPtr->sipBaseRouteEntryIndex = sipLpmData.data.leaf.ecmpOrQosData.nextHopBaseAddr;

        __LOG(("unicast sip sa check is not supported when source IP address is associated with ECMP/QOS block"));
        ipSecurChecksInfoPtr->unicastSipSaCheck = 0;
    }


    /* process sip/dip ecmp or qos leafs (if any) and updates matchIndex array (if nessecary) */
    snetSip5FetchRouteEntry(devObjPtr, descrPtr, &dipLpmData, &sipLpmData, /*IN*/
                                                           matchIndexPtr); /*OUT*/

    return isSipLookupPerformed;
}



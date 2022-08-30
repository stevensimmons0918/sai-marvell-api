/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetCheetah.c
*
* DESCRIPTION:
*       This is the file for cheetah3 network file.
*
* FILE REVISION NUMBER:
*       $Revision: 31 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3MacLookup.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Reassembly.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2TStart.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#define UDP_LITE_CHECKSUM_LENGTH    8

typedef enum{
    HEADER_BUILD_STAGE_1_E,/* set most fields , set the checksum field to 0 */
    HEADER_BUILD_STAGE_2_E,/* set the length that include the payload*/
    HEADER_BUILD_STAGE_3_E /* set the checksum field */
}HEADER_BUILD_STAGE_ENT;

/* get field from MLL entry according to index in snetCh3MllEntryFormat[] */
#define CHT3_MLL_FIELD_VALUE_GET_MAC(_mllEntryPtr,_index)  \
    snetFieldValueGet(_mllEntryPtr,                       \
        snetCh3MllEntryFormat[_index].startBit,           \
        (snetCh3MllEntryFormat[_index].lastBit - snetCh3MllEntryFormat[_index].startBit + 1))

/* the fields of the MLL in CH3+ */
const  HW_FIELD snetCh3MllEntryFormat[]=
{
    {  0, 0},  /*Last0*/                              /*SNET_CHT3_MLL_FIELDS_LAST_0_E                                 */
    {  1, 1},  /*StartOfTunnel0*/                     /*SNET_CHT3_MLL_FIELDS_START_OF_TUNNEL_0_E                      */
    {  3,14},  /*MLLVID0*/                            /*SNET_CHT3_MLL_FIELDS_MLL_VID_0_E                              */
    { 15,15},  /*Use_Vidx0*/                          /*SNET_CHT3_MLL_FIELDS_USE_VIDX_0_E                             */
    { 16,16},  /*TargetIsTrunk0*/                     /*SNET_CHT3_MLL_FIELDS_TARGET_IS_TRUNK_0_E                      */
    { 17,22},  /*TrgPort0*/                           /*SNET_CHT3_MLL_FIELDS_TRG_PORT_0_E                             */
    { 21,27},  /*TrgTrunkId0*/                        /*SNET_CHT3_MLL_FIELDS_TRG_TRUNK_ID_0_E                         */
    { 23,27},  /*TrgDev0*/                            /*SNET_CHT3_MLL_FIELDS_TRG_DEV_0_E                              */
    { 16,28},  /*VIDX0*/                              /*SNET_CHT3_MLL_FIELDS_VIDX_0_E                                 */
    { 29,31},  /*MLLRPFFailCMD0*/                     /*SNET_CHT3_MLL_FIELDS_MLL_RPF_FAIL_CMD_0_E                     */
    { 32,41},  /*TunnelPtr0[9:0]*/                    /*SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_LSB_0_E                       */
    { 42,42},  /*TunnelType0*/                        /*SNET_CHT3_MLL_FIELDS_TUNNEL_TYPE_0_E                          */
    { 43,50},  /*TTLThreshold0/HopLimitThreshold0 */  /*SNET_CHT3_MLL_FIELDS_TTL_THRESHOLD_0_HOP_LIMIT_THRESHOLD_0_E  */
    { 51,51},  /*EcludeSrcVlan0*/                     /*SNET_CHT3_MLL_FIELDS_EXCLUDE_SRC_VLAN_0_E                     */
    { 52,53},  /*TunnelPtr0[11:10]*/                  /*SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_MSB_0_E                       */
    { 54,55},  /*TunnelPtr1[11:10]*/                  /*SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_MSB_1_E                       */
    { 64,64},  /*Last1*/                              /*SNET_CHT3_MLL_FIELDS_LAST_1_E                                 */
    { 65,65},  /*StartOfTunnel1*/                     /*SNET_CHT3_MLL_FIELDS_START_OF_TUNNEL_1_E                      */
    { 67,78},  /*MLLVID1*/                            /*SNET_CHT3_MLL_FIELDS_MLL_VID_1_E                              */
    { 79,79},  /*Use_Vidx1*/                          /*SNET_CHT3_MLL_FIELDS_USE_VIDX_1_E                             */
    { 80,80},  /*TargetIsTrunk1*/                     /*SNET_CHT3_MLL_FIELDS_TARGET_IS_TRUNK_1_E                      */
    { 81,86},  /*TrgPort1*/                           /*SNET_CHT3_MLL_FIELDS_TRG_PORT_1_E                             */
    { 85,91},  /*TrgTrunkId1*/                        /*SNET_CHT3_MLL_FIELDS_TRG_TRUNK_ID_1_E                         */
    { 87,91},  /*TrgDev1*/                            /*SNET_CHT3_MLL_FIELDS_TRG_DEV_1_E                              */
    { 80,92},  /*VIDX1*/                              /*SNET_CHT3_MLL_FIELDS_VIDX_1_E                                 */
    { 93,95},  /*MLLRPFFailCMD1*/                     /*SNET_CHT3_MLL_FIELDS_MLL_RPF_FAIL_CMD_1_E                     */
    { 96,105}, /*TunnelPtr1[9:0]*/                    /*SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_LSB_1_E                       */
    {106,106}, /*TunnelType1*/                        /*SNET_CHT3_MLL_FIELDS_TUNNEL_TYPE_1_E                          */
    {107,114}, /*TTLThreshold0/HopLimitThreshold1*/   /*SNET_CHT3_MLL_FIELDS_TTL_THRESHOLD_1_HOP_LIMIT_THRESHOLD_1_E  */
    {115,115}, /*EcludeSrcVlan1*/                     /*SNET_CHT3_MLL_FIELDS_EXCLUDE_SRC_VLAN_1_E                     */
    {116,127}, /*NextMLLPtr*/                         /*SNET_CHT3_MLL_FIELDS_NEXT_MLL_PTR_E                           */

/* Added for WLAN bridging -- start */

    { 56,56},  /*unregBcFilterring0  */               /*SNET_CHT3_MLL_FIELDS_UNREG_BC_FILTERING_0_E                   */
    { 57,57},  /*unregMcFilterring0  */               /*SNET_CHT3_MLL_FIELDS_UNREG_MC_FILTERING_0_E                   */
    { 58,58},  /*unknownUcFilterring0*/               /*SNET_CHT3_MLL_FIELDS_UNKNOWN_UC_FILTERING_0_E                 */
    { 59,59},  /*vlanEgressTagMode0  */               /*SNET_CHT3_MLL_FIELDS_VLAN_EGRESS_TAG_MODE_0_E                 */

    { 60,60},  /*unregBcFilterring1  */               /*SNET_CHT3_MLL_FIELDS_UNREG_BC_FILTERING_1_E                   */
    { 61,61},  /*unregMcFilterring1  */               /*SNET_CHT3_MLL_FIELDS_UNREG_MC_FILTERING_1_E                   */
    { 62,62},  /*unknownUcFilterring1*/               /*SNET_CHT3_MLL_FIELDS_UNKNOWN_UC_FILTERING_1_E                 */
    { 63,63},  /*vlanEgressTagMode1  */               /*SNET_CHT3_MLL_FIELDS_VLAN_EGRESS_TAG_MODE_1_E                 */

/* Added for WLAN bridging -- end */

    { 0,0} /* dummy */
};




/**
* @internal checkSumCalc function
* @endinternal
*
* @brief   Perform ones-complement sum , and ones-complement on the final sum-word.
*         The function can be used to make checksum for various protocols.
* @param[in] pMsg                     - pointer to IP header.
* @param[in] lMsg                     - IP header length.
*
* @note 1. If there's a field CHECKSUM within the input-buffer
*       it supposed to be zero before calling this function.
*       2. The input buffer is supposed to be in network byte-order.
*
*/
GT_U16 checkSumCalc
(
    IN GT_U8 *pMsg,
    IN GT_U16 lMsg
)
{
    /* the UDP use the same checksum as the IPv4 */
    return (GT_U16)ipV4CheckSumCalc(pMsg,lMsg);
}


/**
* @internal lion3ReadMllEntrySection function
* @endinternal
*
* @brief   Lion3 : Read 1/2 Multicast Link List entry .
*         when read second half , also get the 'next pointer'
*
* @param[out] mllPtr                   - pointer to the MLL entry.
*/
static void lion3ReadMllEntrySection
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  mllIndex,
    IN GT_U32                  selectorIndex,
    OUT SNET_CHT3_DOUBLE_MLL_STC *mllPtr
)
{
    SNET_CHT3_SINGLE_MLL_STC *singleMllPtr;
    GT_U32  tmpVal;
    GT_U32  *mllEntryPtr;

    mllEntryPtr = smemMemGet(devObjPtr,
        SMEM_CHT3_ROUTER_MULTICAST_LIST_TBL_MEM(devObjPtr,mllIndex));

    singleMllPtr = selectorIndex ?
                        &mllPtr->second_mll :
                        &mllPtr->first_mll;

    singleMllPtr->rpf_fail_cmd =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0);
    singleMllPtr->ttlThres =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0);
    singleMllPtr->excludeSrcVlan =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0);
    singleMllPtr->last =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_0);
    singleMllPtr->vid =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_0);
    singleMllPtr->isTunnelStart =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0);
    if(singleMllPtr->isTunnelStart)
    {
        singleMllPtr->tsInfo.tunnelStartType =
            SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                selectorIndex ?
                SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1:
                SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0);
        singleMllPtr->tsInfo.tunnelStartPtr =
            SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                selectorIndex ?
                SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1:
                SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0);
    }

    tmpVal =
        SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
            selectorIndex ?
            SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_1:
            SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_0);
    if(tmpVal)/* useVidx */
    {
        singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_VIDX_E;
        singleMllPtr->lll.interfaceInfo.vidx =
            SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                selectorIndex ?
                SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1:
                SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0);
    }
    else
    {
        tmpVal =
            SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                selectorIndex ?
                SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1:
                SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0);
        if(tmpVal)/* isTrunk */
        {
            singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_TRUNK_E;
            singleMllPtr->lll.interfaceInfo.trunkId =
                SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                    selectorIndex ?
                    SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1:
                    SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0);
        }
        else
        {
            singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_PORT_E;
            singleMllPtr->lll.interfaceInfo.devPort.port =
                SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                    selectorIndex ?
                    SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_1:
                    SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_0);
            singleMllPtr->lll.interfaceInfo.devPort.devNum =
                SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                    selectorIndex ?
                    SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_1:
                    SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_0);
        }
    }

    if(selectorIndex)
    {
        mllPtr->nextPtr =
            SMEM_LION3_IP_MLL_ENTRY_FIELD_GET(devObjPtr,mllEntryPtr,mllIndex,
                SMEM_LION3_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR);
    }

    return;
}

/**
* @internal snetCht3ReadMllEntry function
* @endinternal
*
* @brief   Read Multicast Link List entry
*
* @param[out] mllPtr                   - pointer to the MLL entry.
*/
void snetCht3ReadMllEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  mllIndex,
    OUT SNET_CHT3_DOUBLE_MLL_STC *mllPtr
)
{
    DECLARE_FUNC_NAME(snetCht3ReadMllEntry);

    SNET_CHT3_MLL_FIELDS_ENT  index;
    SNET_CHT3_SINGLE_MLL_STC *singleMllPtr;
    GT_U32  tmpVal;
    GT_U32  *mllEntryPtr;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* read first half of the entry */
        lion3ReadMllEntrySection(devObjPtr,mllIndex,0,mllPtr);
        /* read second half of the entry .
           this also read the 'next pointer' */
        lion3ReadMllEntrySection(devObjPtr,mllIndex,1,mllPtr);

        return;
    }

    __LOG(("access MLL entry [%d] \n" , mllIndex));

    mllEntryPtr = smemMemGet(devObjPtr,
        SMEM_CHT3_ROUTER_MULTICAST_LIST_TBL_MEM(devObjPtr,mllIndex));
    /**************/
    /* first  MLL */
    /**************/
    singleMllPtr = &mllPtr->first_mll;

#define __LOG_FIRST_MLL_PARAM(mllFeild) \
    __LOG(("mllPtr->first_mll.%s = [0x%x]\n" , #mllFeild , mllPtr->first_mll.mllFeild))

    index = SNET_CHT3_MLL_FIELDS_MLL_RPF_FAIL_CMD_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->rpf_fail_cmd = tmpVal;
    __LOG_FIRST_MLL_PARAM(rpf_fail_cmd);

    index = SNET_CHT3_MLL_FIELDS_TTL_THRESHOLD_0_HOP_LIMIT_THRESHOLD_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->ttlThres = tmpVal;
    __LOG_FIRST_MLL_PARAM(ttlThres);

    index = SNET_CHT3_MLL_FIELDS_EXCLUDE_SRC_VLAN_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->excludeSrcVlan = tmpVal;
    __LOG_FIRST_MLL_PARAM(excludeSrcVlan);

    index = SNET_CHT3_MLL_FIELDS_LAST_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->last =tmpVal;
    __LOG_FIRST_MLL_PARAM(last);

    index = SNET_CHT3_MLL_FIELDS_MLL_VID_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->vid =tmpVal;
    __LOG_FIRST_MLL_PARAM(vid);


    index = SNET_CHT3_MLL_FIELDS_START_OF_TUNNEL_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->isTunnelStart =tmpVal;
    __LOG_FIRST_MLL_PARAM(isTunnelStart);

    if(singleMllPtr->isTunnelStart)
    {
        index = SNET_CHT3_MLL_FIELDS_TUNNEL_TYPE_0_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->tsInfo.tunnelStartType =tmpVal;
        __LOG_FIRST_MLL_PARAM(tsInfo.tunnelStartType);

        index = SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_LSB_0_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->tsInfo.tunnelStartPtr =tmpVal;

        index = SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_MSB_0_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->tsInfo.tunnelStartPtr |= tmpVal<<10;

        __LOG_FIRST_MLL_PARAM(tsInfo.tunnelStartPtr);
    }


    index = SNET_CHT3_MLL_FIELDS_USE_VIDX_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);

    if(tmpVal)/* use vidx */
    {
        singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_VIDX_E;
        index = SNET_CHT3_MLL_FIELDS_VIDX_0_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->lll.interfaceInfo.vidx =tmpVal;
        __LOG_FIRST_MLL_PARAM(lll.interfaceInfo.vidx);
    }
    else
    {
        index = SNET_CHT3_MLL_FIELDS_TARGET_IS_TRUNK_0_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);

        if(tmpVal)/* isTrunk */
        {
            singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_TRUNK_E;
            index = SNET_CHT3_MLL_FIELDS_TRG_TRUNK_ID_0_E;
            tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
            singleMllPtr->lll.interfaceInfo.trunkId =tmpVal;
            __LOG_FIRST_MLL_PARAM(lll.interfaceInfo.trunkId);
        }
        else
        {
            singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_PORT_E;
            index = SNET_CHT3_MLL_FIELDS_TRG_PORT_0_E;
            tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
            singleMllPtr->lll.interfaceInfo.devPort.port =tmpVal;
            __LOG_FIRST_MLL_PARAM(lll.interfaceInfo.devPort.port);

            index = SNET_CHT3_MLL_FIELDS_TRG_DEV_0_E;
            tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
            singleMllPtr->lll.interfaceInfo.devPort.devNum =tmpVal;
            __LOG_FIRST_MLL_PARAM(lll.interfaceInfo.devPort.devNum);
        }
    }

    index = SNET_CHT3_MLL_FIELDS_UNREG_BC_FILTERING_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->unregBcFiltering =tmpVal;
    __LOG_FIRST_MLL_PARAM(unregBcFiltering);

    index = SNET_CHT3_MLL_FIELDS_UNREG_MC_FILTERING_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->unregMcFiltering =tmpVal;
    __LOG_FIRST_MLL_PARAM(unregMcFiltering);

    index = SNET_CHT3_MLL_FIELDS_UNKNOWN_UC_FILTERING_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->unknownUcFiltering =tmpVal;
    __LOG_FIRST_MLL_PARAM(unknownUcFiltering);

    index = SNET_CHT3_MLL_FIELDS_VLAN_EGRESS_TAG_MODE_0_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->vlanEgressTagMode =tmpVal;
    __LOG_FIRST_MLL_PARAM(vlanEgressTagMode);


    /**************/
    /* second MLL */
    /**************/
#undef __LOG_FIRST_MLL_PARAM
#define __LOG_SECOND_MLL_PARAM(mllFeild) \
    __LOG(("mllPtr->second_mll.%s = [0x%x]\n" , #mllFeild , mllPtr->second_mll.mllFeild))

    singleMllPtr = &mllPtr->second_mll;

    index = SNET_CHT3_MLL_FIELDS_MLL_RPF_FAIL_CMD_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->rpf_fail_cmd =tmpVal;
    __LOG_SECOND_MLL_PARAM(rpf_fail_cmd);

    index = SNET_CHT3_MLL_FIELDS_TTL_THRESHOLD_1_HOP_LIMIT_THRESHOLD_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->ttlThres =tmpVal;
    __LOG_SECOND_MLL_PARAM(ttlThres);

    index = SNET_CHT3_MLL_FIELDS_EXCLUDE_SRC_VLAN_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->excludeSrcVlan =tmpVal;
    __LOG_SECOND_MLL_PARAM(excludeSrcVlan);

    index = SNET_CHT3_MLL_FIELDS_LAST_1_E;/* on the second MLL there is no specific
                                         bit for last , it is implicitly from
                                         the next pointer value */
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->last =tmpVal;
    __LOG_SECOND_MLL_PARAM(last);

    index = SNET_CHT3_MLL_FIELDS_MLL_VID_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->vid =tmpVal;
    __LOG_SECOND_MLL_PARAM(vid);

    index = SNET_CHT3_MLL_FIELDS_START_OF_TUNNEL_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->isTunnelStart =tmpVal;
    __LOG_SECOND_MLL_PARAM(isTunnelStart);

    if(singleMllPtr->isTunnelStart)
    {
        index = SNET_CHT3_MLL_FIELDS_TUNNEL_TYPE_1_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->tsInfo.tunnelStartType =tmpVal;
        __LOG_SECOND_MLL_PARAM(tsInfo.tunnelStartType);

        index = SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_LSB_1_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->tsInfo.tunnelStartPtr =tmpVal;

        index = SNET_CHT3_MLL_FIELDS_TUNNEL_PTR_MSB_1_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->tsInfo.tunnelStartPtr |= tmpVal<<10;
        __LOG_SECOND_MLL_PARAM(tsInfo.tunnelStartPtr);
    }

    index = SNET_CHT3_MLL_FIELDS_USE_VIDX_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);

    if(tmpVal)/* use vidx */
    {
        singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_VIDX_E;
        index = SNET_CHT3_MLL_FIELDS_VIDX_1_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
        singleMllPtr->lll.interfaceInfo.vidx =tmpVal;
        __LOG_SECOND_MLL_PARAM(lll.interfaceInfo.vidx);
    }
    else
    {
        index = SNET_CHT3_MLL_FIELDS_TARGET_IS_TRUNK_1_E;
        tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);

        if(tmpVal)/* isTrunk */
        {
            singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_TRUNK_E;
            index = SNET_CHT3_MLL_FIELDS_TRG_TRUNK_ID_1_E;
            tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
            singleMllPtr->lll.interfaceInfo.trunkId =tmpVal;
            __LOG_SECOND_MLL_PARAM(lll.interfaceInfo.trunkId);
        }
        else
        {
            singleMllPtr->lll.dstInterface = SNET_DST_INTERFACE_PORT_E;
            index = SNET_CHT3_MLL_FIELDS_TRG_PORT_1_E;
            tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
            singleMllPtr->lll.interfaceInfo.devPort.port =tmpVal;
            __LOG_SECOND_MLL_PARAM(lll.interfaceInfo.devPort.port);

            index = SNET_CHT3_MLL_FIELDS_TRG_DEV_1_E;
            tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
            singleMllPtr->lll.interfaceInfo.devPort.devNum =tmpVal;
            __LOG_SECOND_MLL_PARAM(lll.interfaceInfo.devPort.devNum);
        }
    }

    index = SNET_CHT3_MLL_FIELDS_UNREG_BC_FILTERING_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->unregBcFiltering =tmpVal;
    __LOG_SECOND_MLL_PARAM(unregBcFiltering);

    index = SNET_CHT3_MLL_FIELDS_UNREG_MC_FILTERING_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->unregMcFiltering =tmpVal;
    __LOG_SECOND_MLL_PARAM(unregMcFiltering);

    index = SNET_CHT3_MLL_FIELDS_UNKNOWN_UC_FILTERING_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->unknownUcFiltering =tmpVal;
    __LOG_SECOND_MLL_PARAM(unknownUcFiltering);

    index = SNET_CHT3_MLL_FIELDS_VLAN_EGRESS_TAG_MODE_1_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    singleMllPtr->vlanEgressTagMode =tmpVal;
    __LOG_SECOND_MLL_PARAM(vlanEgressTagMode);


    /********************/
    /* next MLL pointer */
    /********************/
    index = SNET_CHT3_MLL_FIELDS_NEXT_MLL_PTR_E;
    tmpVal = CHT3_MLL_FIELD_VALUE_GET_MAC(mllEntryPtr,index);
    mllPtr->nextPtr =tmpVal;

#undef __LOG_SECOND_MLL_PARAM

    __LOG(("mllPtr->nextPtr = [0x%x]\n" , tmpVal));
}


/**
* @internal snetCht3mllCounters function
* @endinternal
*
* @brief   Update MLL counters
*/
GT_VOID snetCht3mllCounters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL      isL2Mll
)
{
    DECLARE_FUNC_NAME(snetCht3mllCounters);

    GT_U32 regAddr;                 /* Register address                            */
    GT_U32 * regPtr;                /* Register entry pointer                      */
    GT_U32 cntset_port_trunk_mode;  /* port trunk mode for interface bind          */
    GT_U32 cntset_port_trunk;       /* port/trunk value for interface bind              */
    GT_U32 cntset_ip_mode;          /* counting mode interface route entry         */
    GT_U32 cntset_vlan_mode;        /* counting mode interface route entry         */
    GT_U32 cntset_vid;              /* vlan value  for interface bind              */
    GT_U32 cntset_dev;              /* device value  for interface bind            */
    GT_BOOL isCountVid;             /* whether packet's eVid is matching this counter-set             */
    GT_BOOL isCountProtocol;        /* whether packet's protocol is matching this counter-set        */
    GT_BOOL isCountIf;              /* whether packet's out interface is matching this counter-set   */
    GT_U32  ii;                     /* index for MLL */
    GT_U32  value;                  /* register's value*/
    GT_U32  value1;                 /* register's value 1*/


    for(ii = 0 ; ii < 2 ; ii++)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Counter Set  ROUTER_MNG_COUNTER_CONFIGURATION */

            if(isL2Mll == GT_TRUE)
            {
                regAddr = SMEM_LION3_L2_MLL_OUT_INTERFACE_CNTR_CONFIG_REG(devObjPtr,ii);
                smemRegGet(devObjPtr , regAddr, &value);
                regAddr = SMEM_LION3_L2_MLL_OUT_INTERFACE_CNTR_CONFIG_1_REG(devObjPtr,ii);
                smemRegGet(devObjPtr , regAddr, &value1);
            }
            else
            {
                regAddr = SMEM_LION3_IP_MLL_OUT_INTERFACE_CNTR_CONFIG_REG(devObjPtr,ii);
                smemRegGet(devObjPtr , regAddr, &value);
                regAddr = SMEM_LION3_IP_MLL_OUT_INTERFACE_CNTR_CONFIG_1_REG(devObjPtr,ii);
                smemRegGet(devObjPtr , regAddr, &value1);
            }

            cntset_port_trunk_mode = SMEM_U32_GET_FIELD(value, 30 , 2 );
            cntset_ip_mode         = SMEM_U32_GET_FIELD(value, 27 , 2 );
            cntset_vlan_mode       = SMEM_U32_GET_FIELD(value, 26 , 1 );
            cntset_vid             = SMEM_U32_GET_FIELD(value, 10 , 16);

            cntset_dev             = SMEM_U32_GET_FIELD(value1, 20  , 12 );
            cntset_port_trunk      = SMEM_U32_GET_FIELD(value1, 0  , 20 );

            if(cntset_port_trunk_mode == 2)
            {
                /*use only 12 bits*/
                cntset_port_trunk &= 0xFFF;
            }
        }
        else
        {
            /* Counter Set  ROUTER_MNG_COUNTER_CONFIGURATION */
            regAddr = SMEM_CHT3_MLL_OUT_INTERFACE_CNFG_TBL_MEM(devObjPtr,ii);
            smemRegGet(devObjPtr , regAddr, &value);

            cntset_port_trunk_mode = SMEM_U32_GET_FIELD(value, 30 , 2 );
            cntset_ip_mode         = SMEM_U32_GET_FIELD(value, 27 , 2 );
            cntset_vlan_mode       = SMEM_U32_GET_FIELD(value, 26 , 1 );
            cntset_vid             = SMEM_U32_GET_FIELD(value, 14 , 12);
            cntset_dev             = SMEM_U32_GET_FIELD(value, 8  , 5 );
            cntset_port_trunk      = SMEM_U32_GET_FIELD(value, 0  , 8 );
        }


        if(descrPtr->useVidx == 1)
        {
            isCountIf = GT_TRUE;
        }
        else
        if (
            (cntset_port_trunk_mode == 0) ||    /* disregard port/trunk */
                                                /* counts port+dev      */
            ((cntset_port_trunk_mode == 1) && (cntset_dev == descrPtr->trgDev) && (cntset_port_trunk == descrPtr->trgEPort)) ||
                                                /* counts trunk         */
            ((cntset_port_trunk_mode == 2) && (descrPtr->targetIsTrunk) && (cntset_port_trunk == descrPtr->trgTrunkId))
           )
        {
            isCountIf = GT_TRUE;
        }
        else
        {
            isCountIf = GT_FALSE;
        }

        if  ((cntset_vlan_mode == 0 )  ||             /* disregard eVid        */
             (descrPtr->eVid == cntset_vid))              /* counts specific eVid  */

        {
            isCountVid = GT_TRUE;
        }
        else
        {
            isCountVid = GT_FALSE;
        }

        /* [JIRA] (MLL-300) L2 MLL Out Multicast Packets Counter - Do not count non IP packets */
        if((SMEM_CHT_IS_SIP5_GET(devObjPtr)) && (devObjPtr->errata.l2MllOutMcPckCnt) && (isL2Mll==GT_TRUE))
        {
           if ((((descrPtr->isIPv4 == 1) && (descrPtr->isIp == 1)) && (cntset_ip_mode < 2)) ||
               (((descrPtr->isIp == 1) && (descrPtr->isIPv4 == 0)) && ((cntset_ip_mode == 0) || (cntset_ip_mode == 2))))
           {
               isCountProtocol = GT_TRUE;
           }
           else
           {
               isCountProtocol = GT_FALSE;
               __LOG(("Warning (Errata): L2 MLL Out Multicast Packets Counter - Do not count non IP packets although it should \n"));
           }
        }
        else
        {
            if (
                (cntset_ip_mode  == 0) ||               /* disregard protocol   */
                                                        /* counts IPv4 packets  */
                ((cntset_ip_mode   == 1)  && (descrPtr->isIPv4 == 1)) ||
                                                        /* counts IPv6 packets  */
                ((cntset_ip_mode   == 2)  && (descrPtr->isIPv4 == 0))
               )
            {
                isCountProtocol = GT_TRUE;
            }
            else
            {
                isCountProtocol = GT_FALSE;
            }
        }

        /* Address associated with DIP search */
        if ((isCountIf == GT_TRUE) & (isCountProtocol == GT_TRUE)
           && (isCountVid == GT_TRUE))
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                if(isL2Mll == GT_TRUE)
                {
                    __LOG(("L2 MLL increment count on counter set[%d] \n", ii));
                    regAddr = SMEM_LION3_L2_MLL_OUT_MULTICAST_PACKETS_COUNTER_REG(devObjPtr,ii);
                }
                else
                {
                    __LOG(("IP MLL increment count on counter set[%d] \n", ii));
                    regAddr = SMEM_LION3_IP_MLL_OUT_MULTICAST_PACKETS_COUNTER_REG(devObjPtr,ii);
                }
            }
            else
            {
                __LOG(("IP MLL increment count on counter set[%d] \n", ii));
                regAddr = SMEM_CHT3_MLL_OUT_INTERFACE_COUNTER_TBL_MEM(devObjPtr,ii);
            }

            regPtr = smemMemGet(devObjPtr,regAddr);
            (*regPtr)++;
        }
    }

    /*************************** DROP COUNTER REGISTERS *******************/

    if ((descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E) ||
        (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E))
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_MLL_MC_FIFO_DROP_CNTR_REG(devObjPtr);
        }
        else
        {
            regAddr = SMEM_CHT3_MLL_DROP_CNTR_TBL_MEM(devObjPtr);
        }

        regPtr = smemMemGet(devObjPtr,regAddr);
        (*regPtr)++;
    }



    /************************** DROP COUNTER REGISTERS *******************/

}

/**
* @internal snetCht3IngressMllSingleMllOutlifSet function
* @endinternal
*
* @brief   set the target outLif into descriptor - for single MLL
*         and Update mll counters
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
* @param[in] singleMllPtr             - single mll pointer
*/
void snetCht3IngressMllSingleMllOutlifSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN   SNET_CHT3_SINGLE_MLL_STC * singleMllPtr
)
{
    DECLARE_FUNC_NAME(snetCht3IngressMllSingleMllOutlifSet);

    descrPtr->eVid = singleMllPtr->vid;
    descrPtr->useVidx = 0;
    descrPtr->targetIsTrunk = 0;
    descrPtr->eVidx = 0xfff;

    if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
    {
        /* the replication should not send copy to the CPU */
        __LOG(("the replication should not send copy to the CPU"));
        descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;
    }

    switch(singleMllPtr->lll.dstInterface)
    {
        case SNET_DST_INTERFACE_PORT_E:
            descrPtr->trgEPort = singleMllPtr->lll.interfaceInfo.devPort.port;
            /* call after setting trgEPort */
            __LOG(("call after setting trgEPort"));
            SNET_E_ARCH_CLEAR_IS_TRG_PHY_PORT_VALID_MAC(devObjPtr,descrPtr,ipvx);
            descrPtr->trgDev  = singleMllPtr->lll.interfaceInfo.devPort.devNum;
            break;
        case SNET_DST_INTERFACE_TRUNK_E:
            descrPtr->targetIsTrunk = 1;
            descrPtr->trgTrunkId = singleMllPtr->lll.interfaceInfo.trunkId;
            break;
        case SNET_DST_INTERFACE_VIDX_E:
            descrPtr->useVidx = 1;
            descrPtr->eVidx = singleMllPtr->lll.interfaceInfo.vidx;
            break;
        default:
            skernelFatalError(" snetCht3Descriptor: bad interface[%d]", singleMllPtr->lll.dstInterface);
    }

    /* Update IP mll counters */
    __LOG(("Update IP_MLL counters \n"));
    snetCht3mllCounters(devObjPtr, descrPtr , GT_FALSE/*count IP MLL (not L2 MLL)*/);

    /* ts mll support */
    if (singleMllPtr->isTunnelStart == GT_TRUE)
    {
        descrPtr->tunnelStart = GT_TRUE;
        descrPtr->tunnelPtr = singleMllPtr->tsInfo.tunnelStartPtr;
        descrPtr->tunnelStartPassengerType = singleMllPtr->tsInfo.tunnelStartType;

         __LOG(("Tunnel Start [0x%4.4x] passengerType[%d] from MLL \n",
                 descrPtr->tunnelPtr, descrPtr->tunnelStartPassengerType));
    }

}



/**
* @internal snetCht3IngressMllReplication function
* @endinternal
*
* @brief   Ingress MLL replication
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*                                      mllAddr         - index to first MLL
*                                      OUTPUT:
*                                      RETURN:
*/
static GT_VOID snetCht3IngressMllReplication
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN GT_U32 mllIndex
)
{
    DECLARE_FUNC_NAME(snetCht3IngressMllReplication);

    SKERNEL_FRAME_CHEETAH_DESCR_STC *origInfoDescrPtr;/* original descriptor info */
    SKERNEL_FRAME_CHEETAH_DESCR_STC *nextDescPtr;/* (pointer to)'next' descriptor info */
    SNET_CHT3_DOUBLE_MLL_STC mll;/* current MLL pair entry (hold 2 single MLLs)*/
    SNET_CHT3_SINGLE_MLL_STC *singleMllPtr; /* pointer to the info about single
                                            MLL (first/second half of the pair)*/
    GT_U32  ii;/* iterator */
    GT_BOOL sendReplication; /* check if to send replication to current single
                                MLL , or skip it */
    GT_U32  routerMCSourceID;/*RouterMC Source-ID */
    GT_U32  value;/*register value*/
    GT_U32  srcIdMask,srcIdValue;
    GT_U32  *memPtr; /*pointer to memory*/
    GT_U32  ipMcBridgeViaMLLEn; /* Enable IP Multicast Bridging via MLL */
    GT_BOOL overrideBrgForwardDecision; /* MLL assignment overrides the Bridge forwarding decision */

    memPtr = smemMemGet(devObjPtr, SMEM_CHT2_MLL_GLB_CONTROL_REG(devObjPtr));
    ipMcBridgeViaMLLEn = SMEM_U32_GET_FIELD(*memPtr,13,1);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr, SMEM_LION3_IP_MLL_MC_SOURCE_ID_REG(devObjPtr), 0, 24, &value);
        srcIdMask  = SMEM_U32_GET_FIELD(value,0,12);
        srcIdValue = SMEM_U32_GET_FIELD(value,12,12);

        routerMCSourceID = (descrPtr->sstId & (~srcIdMask)) | (srcIdValue & srcIdMask);
    }
    else
    {
        routerMCSourceID = SMEM_U32_GET_FIELD(*memPtr,0,5);
    }
    __LOG(("Source ID assigned to IP Multicast packets duplicated to the down streams [%x] \n",
        routerMCSourceID));


    /*********************************/
    /* save original descriptor info */
    /*********************************/
    /* duplicate the descriptor and save it's info */
    origInfoDescrPtr = snetChtEqDuplicateDescr(devObjPtr,descrPtr);
    /* 'next replication' : duplicate descriptor from the ingress core */
    nextDescPtr = snetChtEqDuplicateDescr(devObjPtr,descrPtr);

    overrideBrgForwardDecision = GT_FALSE;

    while(1)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(GT_TRUE ==
                snetLion3IngressMllAccessCheck(devObjPtr,descrPtr,GT_TRUE,mllIndex))
            {
                if (overrideBrgForwardDecision)
                {
                    /*assigned a SOFT_DROP packet*/
                    __LOG(("assigned a SOFT_DROP packet , because IP MLL-Based Bridging overrides the Bridge forwarding decision(use original descriptor) \n"));
                    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                              descrPtr->packetCmd,
                                                              SKERNEL_EXT_PKT_CMD_SOFT_DROP_E,
                                                              descrPtr->cpuCode,
                                                              0,/*don't care*/
                                                              SNET_CHEETAH_ENGINE_UNIT_IPMLL_E,
                                                              GT_TRUE);
                }
                /* the mllIndex is 'out of range' */
                return;
            }
        }

        snetCht3ReadMllEntry(devObjPtr,mllIndex,&mll);

        /* check the 2 MLL sections */
        for(ii = 0 ; ii < 2 ; ii++)
        {
            __LOG(("check the MLL [%d] section \n",
                    ii));
            sendReplication = GT_TRUE;
            if(ii == 0)
            {
                singleMllPtr = &mll.first_mll;
            }
            else
            {
                singleMllPtr = &mll.second_mll;
            }

            if (singleMllPtr->excludeSrcVlan != 1 ||
             descrPtr->eVid != singleMllPtr->vid)
            {
                if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E &&
                        descrPtr->eVid == singleMllPtr->vid)
                {
                    if (ipMcBridgeViaMLLEn == 0)
                    {
                        __LOG(("IP Multicast bridging via MLL is disabled. Replication of original descriptor will be done \n"));
                    }
                    else
                    {
                        __LOG(("IP MLL-Based Bridging will be done. So doRouterHa and routed should be cleared."));

                        descrPtr->doRouterHa = 0;
                        descrPtr->routed = 0;

                        __LOG(("MLL assignment overrides the Bridge forwarding decision \n"));
                        overrideBrgForwardDecision = GT_TRUE;
                    }
                }

                if (descrPtr->ttl < singleMllPtr->ttlThres)
                {
                    sendReplication = GT_FALSE;
                    __LOG(("Replication filtered due to TTL[%d] below threshold[%d] \n",
                        descrPtr->ttl ,
                        singleMllPtr->ttlThres));
                }

                if(sendReplication == GT_TRUE)
                {
                    if(descrPtr->eVid != singleMllPtr->vid)
                    {
                        /* The Source ID assigned to IP Multicast packets duplicated
                           to the down streams*/
                        descrPtr->sstId = routerMCSourceID;
                    }
                    else
                    {
                        /* The Bridge copy of the IP Multicast packet is assigned
                          with a Source-ID assigned by the Bridge*/
                    }

                    /* set the target outLif into descriptor - for single MLL
                       and Update mll counters */
                    snetCht3IngressMllSingleMllOutlifSet(devObjPtr, descrPtr,singleMllPtr);

                    if(simLogIsOpenFlag)
                    {
                        *nextDescPtr = *descrPtr;

                        /*restore original values for the 'compare' of descriptors 'old and new'*/
                        *descrPtr = *origInfoDescrPtr;
                        SIM_LOG_PACKET_DESCR_SAVE

                        *descrPtr = *nextDescPtr;
                        SIM_LOG_PACKET_DESCR_COMPARE("(IPMC replication) IPMllEngine replication \n");
                    }


                    /* call the rest of the 'Ingress pipe' */
                    __LOG(("call the rest of the 'Ingress pipe'"));
                    snetChtIngressAfterL3IpReplication(devObjPtr, descrPtr);
                    /* restore original descriptor */
                    __LOG(("restore original descriptor"));
                    *descrPtr = *origInfoDescrPtr;

                }
                else
                {
                    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                    {
                        memPtr = smemMemGet(devObjPtr, SMEM_LION3_IP_MLL_SKIPPED_ENTRIES_COUNTER_REG (devObjPtr));
                        __LOG(("increment IP MLL Skipped Entries Counter from [%d]\n", *memPtr));
                        (*memPtr)++;
                    }
                }

            }

            if (singleMllPtr->last == 1)
            {
                if (overrideBrgForwardDecision)
                {
                    /*assigned a SOFT_DROP packet*/
                    __LOG(("assigned a SOFT_DROP packet , because IP MLL-Based Bridging overrides the Bridge forwarding decision(use original descriptor) \n"));
                    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                              descrPtr->packetCmd,
                                                              SKERNEL_EXT_PKT_CMD_SOFT_DROP_E,
                                                              descrPtr->cpuCode,
                                                              0,/*don't care*/
                                                              SNET_CHEETAH_ENGINE_UNIT_IPMLL_E,
                                                              GT_TRUE);
                }
                return;
            }
        }

        if(mll.nextPtr == 0)
        {
            if (overrideBrgForwardDecision)
            {
                /*assigned a SOFT_DROP packet*/
                __LOG(("assigned a SOFT_DROP packet , because IP MLL-Based Bridging overrides the Bridge forwarding decision(use original descriptor) \n"));
                snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                          descrPtr->packetCmd,
                                                          SKERNEL_EXT_PKT_CMD_SOFT_DROP_E,
                                                          descrPtr->cpuCode,
                                                          0,/*don't care*/
                                                          SNET_CHEETAH_ENGINE_UNIT_IPMLL_E,
                                                          GT_TRUE);
            }
            /* NOTE : we should not get here because the second section of MLL
                should have been set to singleMllPtr->last = 0 */
            return;
        }

        /* update the address for the next MLL */
        __LOG(("update the address for the next MLL"));
        mllIndex = mll.nextPtr;
    }
}

/**
* @internal snetCht3IngressL3IpReplication function
* @endinternal
*
* @brief   Ingress L3 Ipv4/6 replication
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*/
GT_VOID snetCht3IngressL3IpReplication
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetCht3IngressL3IpReplication);

    GT_U32  mllIndex;/* MLL index */
    GT_BIT  isIpv6 = (descrPtr->isIPv4 || descrPtr->isFcoe) ? 0 : 1;

    if (isIpv6 && (descrPtr->mllSelector == SKERNEL_MLL_SELECT_EXTERNAL_E))
    {
        mllIndex = descrPtr->mllexternal;
        __LOG(("do replications according to 'external' MLL pointer [0x%x] (descrPtr->mllexternal) \n",mllIndex));
    }
    else
    {
        mllIndex = descrPtr->mll;
        if(isIpv6)
        {
            __LOG(("do replications according to 'internal' MLL pointer [0x%x] (descrPtr->mll) \n",mllIndex));
        }
        else
        {
            __LOG(("do replications according to MLL pointer [0x%x] (descrPtr->mll) \n",mllIndex));
        }

    }

    /* do replications according to MLL */
    snetCht3IngressMllReplication(devObjPtr,descrPtr,mllIndex);

    /* all mll replications already were done.  So doRouterHa and routed are cleared. */
    __LOG(("all MLL replications already were done.  So doRouterHa and routed are cleared."));

    descrPtr->doRouterHa = 0;
    descrPtr->routed = 0;
    __LOG_PARAM(descrPtr->doRouterHa);
    __LOG_PARAM(descrPtr->routed);
    return;
}



/*******************************************************************************
*   snetCht3EgressConvert802dot11ToEthernetV2Frame
*
* DESCRIPTION:
*       Egress conversion from 802.11/11e to Ethernet v2 frame.
*
* INPUTS:
*       devObjPtr       - pointer to device object
*       descrPtr        - pointer to frame descriptor
*       egrBufPtr       - pointer to egress buffer
* OUTPUT:
*       egrBufPtr       - pointer to modified egress buffer
*
* RETURN:
*       pointer to end of modified egress buffer
*
* COMMENTS:
*       called from the HA (header alteration)
*******************************************************************************/
GT_U8 *  snetCht3EgressConvert802dot11ToEthernetV2Frame
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    INOUT GT_U8                             * egrBufPtr
)
{
    DECLARE_FUNC_NAME(snetCht3EgressConvert802dot11ToEthernetV2Frame);

    GT_U32  copySize;
    SBUF_BUF_ID   frameBuf;  /* pointer to frame buffer */

    /***********************************************/
    /* the mac addresses already set by the caller */
    /***********************************************/

    /* we need to set the ether type */
    __LOG(("we need to set the ether type"));
    *egrBufPtr = (GT_U8)(descrPtr->capwap.frame802dot11Info.etherType >> 8);
    egrBufPtr++;
    *egrBufPtr = (GT_U8)descrPtr->capwap.frame802dot11Info.etherType;
    egrBufPtr++;

    frameBuf = descrPtr->frameBuf;

    /* calculate the pay load length (of the payload in the 802.11 passenger)*/
    __LOG(("calculate the pay load length (of the payload in the 802.11 passenger)"));
    copySize = frameBuf->actualDataSize -
                (descrPtr->capwap.frame802dot11Info.payloadPtr -
                 frameBuf->actualDataPtr);

    /* we need to set the pay load */
    __LOG(("we need to set the pay load"));
    MEM_APPEND(egrBufPtr, descrPtr->capwap.frame802dot11Info.payloadPtr, copySize);

    return egrBufPtr;
}



/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalUtil.c
//

#include "xpsCommon.h"
#include "cpssHalUtil.h"
#include "cpss/generic/version/gtVersion.h"
#include "cpss/dxCh/dxChxGen/version/cpssDxChVersion.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* CONVERSION OF xpsPktCmd to cpssPktCmd */

CPSS_PACKET_CMD_ENT xpsConvertPktCmdToCpssPktCmd(xpsPktCmd_e cmd)
{
    switch (cmd)
    {
        case XP_PKTCMD_DROP:
            return CPSS_PACKET_CMD_DROP_HARD_E;
        //return CPSS_PACKET_CMD_DROP_SOFT_E;
        case XP_PKTCMD_FWD:
            return CPSS_PACKET_CMD_FORWARD_E;
        //return CPSS_PACKET_CMD_ROUTE_E;
        case XP_PKTCMD_TRAP:
            return CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        case XP_PKTCMD_FWD_MIRROR:
            return CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        //return CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E;
        //return CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E;
        default:
            return CPSS_PACKET_CMD_DROP_HARD_E;
    }
}


/* CONVERSION OF cpssPktCmd to xpsPktCmd */

xpsPktCmd_e xpsConvertPktCmdToXpsPktCmd(CPSS_PACKET_CMD_ENT cmd)
{
    switch (cmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
            return XP_PKTCMD_FWD;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            return XP_PKTCMD_FWD_MIRROR;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            return XP_PKTCMD_TRAP;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            return XP_PKTCMD_DROP;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            return XP_PKTCMD_DROP;
        case CPSS_PACKET_CMD_ROUTE_E:
            return XP_PKTCMD_FWD;
        case CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E:
            return XP_PKTCMD_FWD_MIRROR;
        case CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E:
            return XP_PKTCMD_FWD_MIRROR;
        case CPSS_PACKET_CMD_BRIDGE_E:
            return XP_PKTCMD_FWD;
        case CPSS_PACKET_CMD_NONE_E:
            return XP_PKTCMD_DROP;
        case CPSS_PACKET_CMD_LOOPBACK_E:
            return XP_PKTCMD_DROP;
        case CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E:
            return XP_PKTCMD_FWD;
        default:
            return XP_PKTCMD_DROP;
    }
}

void cpssHalPrintVersion(void)
{
    CPSS_VERSION_INFO_STC   versionInfo;
    GT_STATUS rc;

    rc = cpssDxChVersionGet(&versionInfo);
    if (rc == GT_OK)
    {
        cpssOsPrintf("CPSS: %s\r\n", versionInfo.version);
    }

#if defined (GIT_XDK_BRANCH) && defined (GIT_XDK_COMMIT)
#define STRINGIZE(x) #x
    cpssOsPrintf("XDK: %s %s\r\n", STRINGIZE(GIT_XDK_BRANCH),
                 STRINGIZE(GIT_XDK_COMMIT));
#endif
}


/* THIS FUNCTION CONVERTS THE GT_STATUS RETURN VALUES TO XP_STATUS RETURN VALUES   */
/* XP_ERR_FAILED IS THE NEW ENUM ENTRY IN XP_STATUS WHICH REPRESENT GENERAL ERROR PARAMETER  */


XP_STATUS xpsConvertCpssStatusToXPStatus(GT_STATUS status)
{
    XP_STATUS ret = XP_NO_ERR;
    switch (status)
    {
        case GT_ERROR:
            ret = XP_ERR_FAILED;
            break;
        case GT_OK:
            ret = XP_NO_ERR;
            break;
        case GT_FAIL:
            ret = XP_ERR_FAILED;
            break;
        case GT_BAD_VALUE:
            ret = XP_ERR_INVALID_VALUE;
            break;
        //ret =  XP_ERR_INVALID_PREAMBLE_LEN_VALUE;
        //ret =  XP_ERR_OF_BAD_MATCH_VALUE;
        case GT_OUT_OF_RANGE:
            ret = XP_ERR_OUT_OF_RANGE;
            break;
        //ret =  XP_ERR_VALUE_OUT_OF_RANGE;
        //ret =  XP_ERR_INVALID_ID_RANGE;
        //ret =  XP_ERR_ACM_STATIC_RANGE;
        //ret =  XP_ERR_DATA_OUT_OF_RANGE;
        //ret =  XP_ERR_ARRAY_OUT_OF_BOUNDS;
        case GT_BAD_PARAM:
            ret = XP_ERR_INVALID_DATA;
            break;
        //ret =  XP_ERR_INVALID_KPU;
        //ret =  XP_ERR_INVALID_ID;
        //ret =  XP_ERR_INVALID_SHARE_GROUP;
        //ret =  XP_ERR_INVALID_ROW_BLK_NUM;
        //ret =  XP_ERR_INVALID_TABLE_TYPE;
        //ret =  XP_ERR_INVALID_TABLE_PROPERTIES;
        //ret =  XP_ERR_INVALID_INPUT;
        //ret =  XP_ERR_INVALID_TBL_CTX;
        //ret =  XP_ERR_INVALID_TABLE_KEY;
        //ret =  XP_ERR_INVALID_TABLE_HANDLE;
        //ret =  XP_ERR_INVALID_TABLE_ENTRY;
        //ret =  XP_ERR_INVALID_ENTRY_HANDLE;
        //ret =  XP_ERR_INVALID_FIELD_REQ;
        //ret =  XP_ERR_INVALID_INSERTION_ID;
        //ret =  XP_ERR_INVALID_VIF_ID;
        //ret =  XP_ERR_INVALID_DEV_ID;
        //ret =  XP_ERR_INVALID_PORT_MODE;
        //ret =  XP_ERR_INVALID_BUS;
        //ret =  XP_ERR_INVALID_INTERFACE;
        //ret =  XP_ERR_INVALID_CONFIG_MODE;
        //ret =  XP_ERR_INVALID_POLICY;
        //ret =  XP_ERR_INVALID_ARG;
        //ret =  XP_ERR_INVALID_SEND_MODE;
        //ret =  XP_ERR_INVALID_QUEUE_NUM;
        //ret =  XP_ERR_INVALID_QUEUE;
        //ret =  XP_ERR_INVALID_MAGIC_NUM;
        //ret =  XP_ERR_AGE_INVALID_PROPERTIES;
        //ret =  XP_INVALID_MAC_MODE;
        //ret =  XP_INVALID_PCS_MODE;
        //ret =  XP_ERR_INVALID_PCS_CONFIG_MODE;
        //ret =  XP_ERR_INVALID_PORTNUM;
        //ret =  XP_ERR_INVALID_CHAN_NUM;
        //ret =  XP_ERR_INVALID_MODE_SPEED;
        //ret =  XP_ERR_INVALID_VLAN_FRM_LEN_CONFIG;
        //ret =  XP_ERR_INVALID_SPEED;
        //ret =  XP_ERR_INVALID_SERDES_CHANNEL_NUM;
        //ret =  XP_ERR_INVALID_PCSMODE;
        //ret =  XP_ERR_INVALID_LANE_NUM;
        //ret =  XP_ERR_INVALID_VALE;
        //ret =  XP_ERR_OF_INVALID_FLOW_ID;
        //ret =  XP_ERR_OF_INVALID_GROUP;
        //ret =  XP_ERR_INVALID_VLAN_ID;
        //ret =  XP_INVALID_FEC_MODE;
        //ret =  XP_ERR_INVALID_DEVICE_TYPE;
        //ret =  XP_PORT_NOT_VALID;
        //ret =  XP_ERR_OF_BAD_TABLE_ID;
        //ret =  XP_ERR_OF_BAD_GOTO_TABLE_ID;
        case GT_BAD_PTR:
            ret = XP_ERR_NULL_POINTER;
            break;
        //ret =  XP_ERR_INVALID_TABLE_PTR;
        //ret =  XP_ERR_RESOURCE_NOT_AVAILABLE;
        //ret =  XP_ERR_AAPL_NULL_POINTER;
        case GT_BAD_SIZE:
            ret = XP_ERR_INVALID_DATA_SIZE;
            break;
        //ret =  XP_ERR_INVALID_TABLE_DEPTH;
        //ret =  XP_ERR_INVALID_VALUE_SIZE;
        //ret =  XP_ERR_INVALID_MAX_ENTRIES;
        //ret =  XP_ERR_INVALID_KEY_SIZE;
        /*TODO - GT_BAD_STATE */
        case GT_BAD_STATE:
            ret = XP_ERR_INVALID_DATA;
            break;
        //ret =  XP_ERR_RESOURCE_BUSY;
        case GT_SET_ERROR:
            ret = XP_ERR_FAILED;
            break;
        case GT_GET_ERROR:
            ret = XP_ERR_FAILED;
            break;
        /*TODO - GT_CREATE_ERROR */
        case GT_CREATE_ERROR:
        //ret =  XP_ERR_THREAD_CREATION;
        //ret =  XP_PORT_INSTANCE_NOT_CREATED;
        case GT_NOT_FOUND:
            ret = XP_ERR_KEY_NOT_FOUND;
            break;
        //ret =  XP_ERR_NOT_FOUND;
        case GT_NO_MORE:
            ret = XP_ERR_OUT_OF_IDS;
            break;
        //ret =  XP_ERR_MAILBOX_FULL;
        //ret =  XP_ERR_MAX_LIMIT;
        case GT_NO_SUCH:
            ret = XP_ERR_RESOURCE_DNE;
            break;
        //ret =  XP_ERR_OP_NOT_SUPPORTED;
        //ret =  XP_ERR_OF_UNSUP_INST;
        /*TODO - GT_TIMEOUT */
        case GT_TIMEOUT:
        //ret =  XP_ERR_DMA0_TIMEOUT;
        //ret =  XP_ERR_TIMEOUT;
        //ret =  XP_ERR_AN_TIMEOUT;
        /*TODO - GT_NO_CHANGE clash with GT_ALREADY_EXISTS */
        case GT_NO_CHANGE:
            ret = XP_ERR_KEY_EXISTS;
            break;
        //ret =  XP_ERR_PM_DEV_ADD_DNE;
        case GT_NOT_SUPPORTED:
            ret = XP_ERR_NOT_SUPPORTED;
            break;
        //ret =  XP_ERR_INVALID_DEVICE_TYPE;
        //ret =  XP_ERR_MODE_NOT_SUPPORTED;
        case GT_NOT_IMPLEMENTED:
            ret = XP_ERR_FAILED;
            break;
        case GT_NOT_INITIALIZED:
            ret = XP_ERR_NOT_INITED;
            break;
        //ret =  XP_ERR_BUS_NOT_INITED;
        //ret =  XP_ERR_MAC_NOT_INITED;
        //ret =  XP_PORT_NOT_INITED;
        //ret =  XP_ERR_SERDES_NOT_INITED;
        //ret =  XP_ERR_SERDES_FW_NOT_INITED;
        //ret =  XP_ERR_PM_NOT_INIT;
        case GT_NO_RESOURCE:
        case GT_OUT_OF_PP_MEM:
            ret = XP_ERR_OUT_OF_MEM;
            break;
        //ret =  XP_ERR_MEM_ALLOC_ERROR;
        //ret =  XP_ERR_INVALID_OFFSET;
        /*TODO - GT_FULL */
        case GT_FULL:
        //ret =  XP_ERR_ARRAY_OUT_OF_BOUNDS;
        //ret =  XP_ERR_OUT_OF_IDS;
        /*TODO  - GT_EMPTY */
        case GT_EMPTY:
            ret = XP_ERR_DESCRIPTORS_NOT_AVAILABLE;
            break;
        //ret =  XP_ERR_RESOURCE_NOT_AVAILABLE;
        //ret =  XP_ERR_PM_SWPOOL_EMPTY;
        case GT_INIT_ERROR:
            ret = XP_ERR_INIT;
            break;
        //ret =  XP_ERR_INIT_FAILED;
        //ret =  XP_ERR_SERDES_INIT;
        //ret =  XP_ERR_LOCK_INIT;
        //ret =  XP_ERR_SOCKET_INIT;
        //ret =  XP_ERR_BUS_INIT;
        case GT_NOT_READY:
            ret = XP_ERR_RESOURCE_BUSY;
            break;
        //ret =  XP_ERR_SRAM_BUSY;
        //ret =  XP_ERR_IPC_BUSY;
        //ret =  XP_MDIO_MASTER_BUSY;
        case GT_ALREADY_EXIST:
            ret = XP_ERR_KEY_EXISTS;
            break;
        //ret =  XP_ERR_RESOURCE_IS_PRE_ALLOCATED;
        //ret =  XP_ERR_ID_IS_PRE_ALLOCATED;
        //ret =  XP_ERR_OF_GROUP_EXISTS;
        case GT_OUT_OF_CPU_MEM:
            ret = XP_ERR_MEM_ALLOC_ERROR;
            break;
        case GT_ABORTED:
            ret = XP_ERR_FAILED;
            break;
        case GT_NOT_APPLICABLE_DEVICE:
            ret = XP_ERR_INVALID_DEVICE_TYPE;
            break;
        /*TODO  - temporary for GT_UNFIXABLE_ECC_ERROR */
        case GT_UNFIXABLE_ECC_ERROR:
        //ret =  XP_ERR_MEM_FAIL;
        case GT_UNFIXABLE_BIST_ERROR:
            ret = XP_ERR_MEM_FAIL;
            break;
        /*TODO - GT_CHECKSUM_ERROR */
        case GT_CHECKSUM_ERROR:
            ret = XP_ERR_INVALID_BITFIELD;
            break;
        /*TODO - GT_DSA_PARSING_ERROR */
        case GT_DSA_PARSING_ERROR:
        //ret =  XP_ERR_OUT_OF_KPUS;
        case GT_TX_RING_ERROR:
            ret = XP_ERR_DMA_TX;
            break;
        default:
            ret = XP_ERR_UNKNOWN_EVENT;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "default xps %u cpss %u", ret,
                  status);
            break;
    }
    //LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_DEBUG,"xps %u cpss %u",ret, status);
    return ret;
}


uint32_t xpsUtilXpstoCpssInterfaceConvert(xpsInterfaceId_t intfId,
                                          xpsInterfaceType_e intfType)
{
    switch (intfType)
    {
        case XPS_LAG :
            return intfId -4095;
            break;
        default :
            return intfId;
            break;

    }
}

xpsInterfaceId_t xpsUtilCpssToXpsInterfaceConvert(uint32_t intfId,
                                                  xpsInterfaceType_e intfType)
{
    switch (intfType)
    {
        case XPS_LAG :
            return intfId + 4095;
            break;
        default :
            return intfId;
            break;

    }
}


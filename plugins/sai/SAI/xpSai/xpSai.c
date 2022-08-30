// xpSai.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSai.h"
#include "xpSaiUtil.h"
#include "xpSaiDev.h"
#include "xpSaiStub.h"
#include "cpssHalUtil.h"
XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

/* this table is provided by the host table as part of call to sai_api_initialize */
const sai_service_method_table_t* adapHostServiceMethodTable;

void* xpSaiApiTableArr[MAX_SAI_API_EXTENSION];
//Following is set from the saiSwitchApiInitialize
//xpsDevice_t xpSaiDevId = 0;

//Func: xpSaiDynamicArrayGrow

typedef struct
{
    GT_U32    enumVal;
    const GT_CHAR * strVal;
} xpSaiSaiObjIdEnumToStr;


const GT_CHAR * xpSaiObjIdToStr
(
    sai_object_type_t sai_object_type
)
{
    sai_uint32_t i;
    static xpSaiSaiObjIdEnumToStr objToStr [] =
    {
        _STR_VAL(SAI_OBJECT_TYPE_NULL),
        _STR_VAL(SAI_OBJECT_TYPE_PORT),
        _STR_VAL(SAI_OBJECT_TYPE_LAG),
        _STR_VAL(SAI_OBJECT_TYPE_VIRTUAL_ROUTER),
        _STR_VAL(SAI_OBJECT_TYPE_NEXT_HOP),
        _STR_VAL(SAI_OBJECT_TYPE_NEXT_HOP_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_ROUTER_INTERFACE),
        _STR_VAL(SAI_OBJECT_TYPE_ACL_TABLE),
        _STR_VAL(SAI_OBJECT_TYPE_ACL_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_ACL_COUNTER),
        _STR_VAL(SAI_OBJECT_TYPE_ACL_RANGE),
        _STR_VAL(SAI_OBJECT_TYPE_ACL_TABLE_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_ACL_TABLE_GROUP_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_HOSTIF),
        _STR_VAL(SAI_OBJECT_TYPE_MIRROR_SESSION),
        _STR_VAL(SAI_OBJECT_TYPE_SAMPLEPACKET),
        _STR_VAL(SAI_OBJECT_TYPE_STP),
        _STR_VAL(SAI_OBJECT_TYPE_HOSTIF_TRAP_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_POLICER),
        _STR_VAL(SAI_OBJECT_TYPE_WRED),
        _STR_VAL(SAI_OBJECT_TYPE_QOS_MAP),
        _STR_VAL(SAI_OBJECT_TYPE_QUEUE),
        _STR_VAL(SAI_OBJECT_TYPE_SCHEDULER),
        _STR_VAL(SAI_OBJECT_TYPE_SCHEDULER_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_BUFFER_POOL),
        _STR_VAL(SAI_OBJECT_TYPE_BUFFER_PROFILE),
        _STR_VAL(SAI_OBJECT_TYPE_INGRESS_PRIORITY_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_LAG_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_HASH),
        _STR_VAL(SAI_OBJECT_TYPE_UDF),
        _STR_VAL(SAI_OBJECT_TYPE_UDF_MATCH),
        _STR_VAL(SAI_OBJECT_TYPE_UDF_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_FDB_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_SWITCH),
        _STR_VAL(SAI_OBJECT_TYPE_HOSTIF_TRAP),
        _STR_VAL(SAI_OBJECT_TYPE_HOSTIF_TABLE_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_NEIGHBOR_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_ROUTE_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_VLAN),
        _STR_VAL(SAI_OBJECT_TYPE_VLAN_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_HOSTIF_PACKET),
        _STR_VAL(SAI_OBJECT_TYPE_TUNNEL_MAP),
        _STR_VAL(SAI_OBJECT_TYPE_TUNNEL),
        _STR_VAL(SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_FDB_FLUSH),
        _STR_VAL(SAI_OBJECT_TYPE_NEXT_HOP_GROUP_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_STP_PORT),
        _STR_VAL(SAI_OBJECT_TYPE_RPF_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_RPF_GROUP_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_L2MC_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_IPMC_GROUP),
        _STR_VAL(SAI_OBJECT_TYPE_IPMC_GROUP_MEMBER),
        _STR_VAL(SAI_OBJECT_TYPE_L2MC_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_IPMC_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_MCAST_FDB_ENTRY),
        _STR_VAL(SAI_OBJECT_TYPE_HOSTIF_USER_DEFINED_TRAP),
        _STR_VAL(SAI_OBJECT_TYPE_BRIDGE),
        _STR_VAL(SAI_OBJECT_TYPE_BRIDGE_PORT),
        _STR_VAL(SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY)
    };

    for (i=0; i<sizeof(objToStr)/sizeof(objToStr[0]); i++)
    {
        if (objToStr[i].enumVal == sai_object_type)
        {
            return objToStr[i].strVal;
        }
    }

    return "Unknown enum";

}


XP_STATUS xpSaiDynamicArrayGrow(void **ppNewData, void *pOldData,
                                uint32_t sizeOfStruct,
                                uint16_t sizeOfElement, uint32_t numOfElements, uint32_t defaultSize)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t numOfBytes = xpsDAGetCtxSizeWhileGrow(sizeOfStruct, sizeOfElement,
                                                   numOfElements, defaultSize);

    if ((xpsRetVal = xpsStateHeapMalloc(numOfBytes,
                                        (void**)ppNewData)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Heap Allocation failed |Error code: %d\n", xpsRetVal);
        return xpsRetVal;
    }
    if ((*ppNewData) == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    memset((*ppNewData), 0, numOfBytes);

    //Copy old context into new context (should copy old context size - smaller one)
    numOfBytes = sizeOfStruct + (sizeOfElement * (numOfElements - defaultSize));
    memcpy((*ppNewData), pOldData, numOfBytes);

    return XP_NO_ERR;
}

//Func: xpSaiDynamicArrayShrink

XP_STATUS xpSaiDynamicArrayShrink(void **ppNewData, void *pOldData,
                                  uint32_t sizeOfStruct,
                                  uint16_t sizeOfElement, uint32_t numOfElements, uint32_t defaultSize)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t numOfBytes = xpsDAGetCtxSizeWhileShrink(sizeOfStruct, sizeOfElement,
                                                     numOfElements, defaultSize);

    if ((xpsRetVal = xpsStateHeapMalloc(numOfBytes,
                                        (void**)ppNewData)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Heap Allocation failed |Error code: %d\n", xpsRetVal);
        return xpsRetVal;
    }
    if ((*ppNewData) == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    memset((*ppNewData), 0, numOfBytes);
    memcpy((*ppNewData), pOldData, numOfBytes);

    return XP_NO_ERR;
}

xpsDevice_t xpSaiGetDevId()
{
    return 0;
}

sai_status_t cpssStatus2SaiStatus(GT_STATUS status)
{
    switch (status)
    {
        case GT_OK:
            return SAI_STATUS_SUCCESS;

        case GT_NOT_SUPPORTED:
            return SAI_STATUS_NOT_SUPPORTED;

        case GT_OUT_OF_CPU_MEM:
            return SAI_STATUS_NO_MEMORY;

        case GT_NOT_INITIALIZED:
            return SAI_STATUS_UNINITIALIZED;

        case GT_ERROR:
        case GT_FAIL:
        case GT_BAD_PTR:
        case GT_INIT_ERROR:
        case GT_OUT_OF_RANGE:
        case GT_DSA_PARSING_ERROR:
        case GT_TX_RING_ERROR:
        case GT_UNFIXABLE_ECC_ERROR:
        case GT_UNFIXABLE_BIST_ERROR:
        case GT_SET_ERROR:
        case GT_GET_ERROR:
        case GT_ABORTED:
        case GT_NOT_APPLICABLE_DEVICE:
        case GT_CREATE_ERROR:
        case GT_CHECKSUM_ERROR:
        case GT_TIMEOUT:
        case GT_NOT_ALLOWED:
            return SAI_STATUS_FAILURE;

        case GT_NOT_IMPLEMENTED:
            return SAI_STATUS_NOT_IMPLEMENTED;

        case GT_FULL:
            return SAI_STATUS_TABLE_FULL;

        case GT_NO_RESOURCE:
            return SAI_STATUS_INSUFFICIENT_RESOURCES;

        case GT_NO_CHANGE:
        case GT_ALREADY_EXIST:
            return SAI_STATUS_ITEM_ALREADY_EXISTS;

        case GT_BAD_SIZE:
        case GT_BAD_PARAM:
        case GT_BAD_STATE:
        case GT_BAD_VALUE:
            return SAI_STATUS_INVALID_PARAMETER;

        case GT_NOT_FOUND:
        case GT_EMPTY:
        case GT_NO_SUCH:
        case GT_NO_MORE:
            return SAI_STATUS_ITEM_NOT_FOUND;

        case GT_NOT_READY:
            return SAI_STATUS_OBJECT_IN_USE;

        default:
            return SAI_STATUS_SUCCESS;
    }

    return  SAI_STATUS_SUCCESS;
}

sai_status_t xpsStatus2SaiStatus(XP_STATUS status)
{
    switch (status)
    {
        case XP_NO_ERR:
            return SAI_STATUS_SUCCESS;

        case XP_ERR_NOT_SUPPORTED:
            return SAI_STATUS_NOT_SUPPORTED;

        case XP_ERR_OUT_OF_MEM:
        case XP_ERR_MEM_ALLOC_ERROR:
            return SAI_STATUS_NO_MEMORY;

        case XP_ERR_FAILED:
        case XP_ERR_NULL_POINTER:
        case XP_ERR_UNPACK_FAILURE:
        case XP_ERR_PACK_FAILURE:
        case XP_ERR_INVALID_KPU:
        case XP_ERR_RESOURCE_DNE:
        case XP_ERR_DATA_IS_NOT_ALLIGNED:
        case XP_ERR_REHASH_FAIL:
        case XP_ERR_INVALID_SHARE_GROUP:
        case XP_ERR_UNRESOLVABLE_COL:
        case XP_ERR_CALL_BASE_CLASS_OBJ:
        case XP_ERR_PM_NOT_INIT:
        case XP_ERR_MDIO_READ_OP_NOT_SUPPORTED:
        case XP_ERR_MDIO_WRITE_OP_NOT_SUPPORTED:
        case XP_ERR_STATISTICS_STATUS_NOT_CLEAR:
        case XP_ERR_MODE_NOT_SUPPORTED:
        case XP_ERR_MAC_NOT_CONFIGURED:
        case XP_ERR_MEM_DEALLOC_ERROR:
        case XP_ERR_MUNMAP:
        case XP_ERR_SHMEM:
        case XP_ERR_NOT_INITED:
        case XP_ERR_INIT:
        case XP_ERR_BUS_NOT_INITED:
        case XP_ERR_INVALID_BUS:
        case XP_ERR_BUS_INIT:
        case XP_ERR_DMA0_TIMEOUT:
        case XP_ERR_QUEUE_NOT_ENABLED:
        case XP_ERR_DMA_TRANSMIT:
        case XP_ERR_REGACCESS:
        case XP_ERR_DEVICE_OPEN:
        case XP_ERR_FILE_OPEN:
        case XP_ERR_IOCTL:
        case XP_ERR_DEVICE_CLOSE:
        case XP_ERR_SOCKET_SELECT:
        case XP_ERR_SOCKET_RECV:
        case XP_ERR_SOCKET_SEND:
        case XP_ERR_SOCKET_INIT:
        case XP_ERR_SOCKET_CLOSE:
        case XP_ERR_INVALID_MAGIC_NUM:
        case XP_ERR_EVENT_REGISTRATION:
        case XP_ERR_REG_READ:
        case XP_ERR_REG_WRITE:
        case XP_ERR_SRAM_READ:
        case XP_ERR_SRAM_WRITE:
        case XP_ERR_SRAM_BUSY:
        case XP_ERR_IPC_BUSY:
        case XP_ERR_AGE_INVALID_PROPERTIES:
        case XP_ERR_AGE_CYCLE_BUSY:
        case XP_MDIO_MASTER_LIMIT_EXCEED:
        case XP_MDIO_MASTER_BUSY:
        case XP_PORT_NOT_ENABLED:
        case XP_ERR_PCS_SYNC:
        case XP_ERR_MAC_NOT_INITED:
        case XP_PORT_NOT_INITED:
        case XP_ERR_SERDES_NOT_INITED:
        case XP_ERR_AAPL_NULL_POINTER:
        case XP_ERR_MDIO_MASTER_NOT_CONFIGURED:
        case XP_ERR_INTERRUPT_HANDLER_NOT_REGISTERED:
        case XP_ERR_AN_SET_NOT_ALLOWED:
        case XP_ERR_AN_GET_NOT_ALLOWED:
        case XP_ERR_AN_RESTART_NOT_ALLOWED:
        case XP_ERR_MDIO_STATUS_NOT_CLEAR:
        case XP_ERR_INVALID_PCSMODE:
        case XP_ERR_OP_NOT_SUPPORTED:
        case XP_ERR_UNKNOWN_EVENT:
        case XP_ERR_INIT_FAILED:
        case XP_ERR_SERDES_INIT:
        case XP_ERR_SERDES_FW_NOT_INITED:
        case XP_ERR_THREAD_CREATION:
        case XP_ERR_LOCK_INIT:
        case XP_ERR_ACQUIRING_LOCK:
        case XP_ERR_RELEASING_LOCK:
        case XP_ERR_LOCK_DESTROY:
        case XP_ERR_SOCKET_TX:
        case XP_ERR_SOCKET_POLLING:
        case XP_ERR_TIMEOUT:
        case XP_ERR_CONFIG_OPEN:
        case XP_ERR_OFFSET_OPEN:
        case XP_SERDES_LOOPBACK_ERROR:
        case XP_ERR_TESTBENCH_FAIL:
        case XP_ERR_UNREGISTERED_OFFSET:
        case XP_ERR_SIGACTION_FAIL:
        case XP_ERR_DMA_DESC_NULL_ADDR:
        case XP_ERR_DMA_DESC_OWN:
        case XP_ERR_DMA_TX:
        case XP_ERR_DMA_RX:
        case XP_ERR_DMA_RX_EXIT:
        case XP_ERR_PORT_NOT_AVAILABLE:
        case XP_UMAC_RX_DISABLE:
        case XP_UMAC_TX_DISABLE:
        case XP_UMAC_NO_ACTION_REQD:
        case XP_ERR_UMAC_NOT_IN_SGMII_MODE:
        case XP_ERR_UMAC_NOT_IN_MIX_MODE:
        case XP_ERR_UMAC_NOT_IN_QSGMII_MODE:
        case XP_ERR_FILE_CLOSE:
        case XP_ERR_SHADOW_MEM_MGR_FAIL:
        case XP_ERR_SHADOW_MEM_IOCTL_FAIL:
        case XP_ERR_MEM_FAIL:
        case XP_ERR_MEM_RDWR_FAIL:
        case XP_ERR_DMAC_RAW_SOCKET_FAIL:
        case XP_ERR_DMAC_IOCTL_FAIL:
        case XP_ERR_DMAC_RAW_SOCKET_RD_FAIL:
        case XP_ERR_DMAC_RAW_SOCKET_WR_FAIL:
        case XP_ERR_FW_FILE_NOT_FOUND:
        case XP_ERR_FILE_READ:
        case XP_ERR_SCPU_EXCEPTION:
        case XP_ERR_UMAC_READ_ONLY:
        case XP_ERR_UMAC_WRITE_ONLY:
        case XP_ERR_SERDES_ACCESS:
        case XP_ERR_SBUS_THERMAL_SENSOR_FAILED:
        case XP_STATUS_NUM_OF_ENTRIES:
            return SAI_STATUS_FAILURE;

        case XP_ERR_ARRAY_OUT_OF_BOUNDS:
        case XP_ERR_OUT_OF_KPUS:
        case XP_ERR_RESOURCE_NOT_AVAILABLE:
        case XP_ERR_MAILBOX_FULL:
        case XP_ERR_OUT_OF_IDS:
        case XP_ERR_BUFFERS_NOT_AVAILABLE:
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        case XP_ERR_INVALID_ID:
        case XP_ERR_FREEING_UNALLOCATED_ID:
            return SAI_STATUS_INVALID_OBJECT_ID;

        case XP_ERR_RESOURCE_IS_PRE_ALLOCATED:
        case XP_ERR_KEY_EXISTS:
        case XP_ERR_PM_INIT_DNE:
        case XP_ERR_PM_DEV_ADD_DNE:
        case XP_ERR_ID_IS_PRE_ALLOCATED:
        case XP_ERR_PACKET_DRV_INTERFACE_INITED:
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        case XP_ERR_OUT_OF_RANGE:
            return SAI_STATUS_TABLE_FULL;

        case XP_ERR_INVALID_OFFSET:
        case XP_ERR_INVALID_KEY_SIZE:
        case XP_ERR_INVALID_DATA_SIZE:
        case XP_ERR_INVALID_DATA:
        case XP_ERR_DATA_OUT_OF_RANGE:
        case XP_ERR_INVALID_ROW_BLK_NUM:
        case XP_ERR_INVALID_TABLE_TYPE:
        case XP_ERR_INVALID_TABLE_PROPERTIES:
        case XP_ERR_INVALID_INPUT:
        case XP_ERR_INVALID_TABLE_DEPTH:
        case XP_ERR_INVALID_TABLE_PTR:
        case XP_ERR_INVALID_TBL_CTX:
        case XP_ERR_INVALID_TABLE_KEY:
        case XP_ERR_INVALID_TABLE_HANDLE:
        case XP_ERR_INVALID_TABLE_ENTRY:
        case XP_ERR_INVALID_VALUE_SIZE:
        case XP_ERR_INVALID_ENTRY_HANDLE:
        case XP_ERR_INVALID_FIELD_REQ:
        case XP_ERR_INVALID_INSERTION_ID:
        case XP_ERR_INVALID_VIF_ID:
        case XP_ERR_INVALID_DEV_ID:
        case XP_ERR_INVALID_ID_RANGE:
        case XP_ERR_INVALID_PORT_MODE:
        case XP_ERR_INVALID_PREAMBLE_LEN_VALUE:
        case XP_ERR_INVALID_INTERFACE:
        case XP_ERR_INVALID_CONFIG_MODE:
        case XP_ERR_INVALID_POLICY:
        case XP_ERR_INVALID_VALUE:
        case XP_ERR_INVALID_ARG:
        case XP_ERR_INVALID_SEND_MODE:
        case XP_ERR_INVALID_QUEUE_NUM:
        case XP_ERR_INVALID_QUEUE:
        case XP_INVALID_MAC_MODE:
        case XP_INVALID_PCS_MODE:
        case XP_ERR_INVALID_CHAN_NUM:
        case XP_ERR_INVALID_MODE_SPEED:
        case XP_ERR_INVALID_VLAN_FRM_LEN_CONFIG:
        case XP_ERR_VALUE_OUT_OF_RANGE:
        case XP_ERR_INVALID_SPEED:
        case XP_ERR_INVALID_SERDES_CHANNEL_NUM:
        case XP_ERR_INVALID_PCS_CONFIG_MODE:
        case XP_ERR_PORT_CONFIGURATION:
        case XP_ERR_INVALID_MAX_ENTRIES:
        case XP_ERR_INVALID_LANE_NUM:
        case XP_ERR_INVALID_VALE:
        case XP_ERR_INVALID_PARAMS:
        case XP_ERR_INVALID_UMAC_MODE:
        case XP_ERR_INVALID_SBUS_CMD:
        case XP_ERR_INVALID_BITFIELD:
            return SAI_STATUS_INVALID_PARAMETER;

        case XP_ERR_SEARCH_MISS:
        case XP_ERR_KEY_NOT_FOUND:
        case XP_ERR_PM_SWPOOL_EMPTY:
        case XP_ERR_GET_TABLE_OBJ:
        case XP_ERR_PM_DEV_NOT_ADD:
        case XP_ERR_PM_HWLOOKUP_FAIL:
        case XP_ERR_DESCRIPTORS_NOT_AVAILABLE:
        case XP_ERR_NOT_FOUND:
        case XP_ERR_PKT_NOT_AVAILABLE:
        case XP_PORT_INSTANCE_NOT_CREATED:
            return SAI_STATUS_ITEM_NOT_FOUND; ///< Failed to remove/update key because key does not exist

        case XP_PORT_NOT_VALID:
        case XP_ERR_INVALID_PORTNUM:
            return SAI_STATUS_INVALID_PORT_MEMBER;
        case XP_ERR_RESOURCE_BUSY:
            return SAI_STATUS_OBJECT_IN_USE;
        case XP_ERR_INVALID_VLAN_ID:
            return SAI_STATUS_INVALID_VLAN_ID;
        default:
            return SAI_STATUS_SUCCESS;
    }
    return  SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiConvertSaiPacketAction2xps(sai_packet_action_t
                                             sai_packet_action, xpsPktCmd_e* pXpsPktCmd)
{
    switch (sai_packet_action)
    {
        case SAI_PACKET_ACTION_DROP:
            {
                *pXpsPktCmd = XP_PKTCMD_DROP;
                break;
            }

        case SAI_PACKET_ACTION_FORWARD:
            {
                *pXpsPktCmd = XP_PKTCMD_FWD;
                break;
            }

        case SAI_PACKET_ACTION_COPY:
            {
                *pXpsPktCmd = XP_PKTCMD_FWD_MIRROR;
                break;
            }

        case SAI_PACKET_ACTION_TRAP:
            {
                *pXpsPktCmd = XP_PKTCMD_TRAP;
                break;
            }

        case SAI_PACKET_ACTION_LOG:
            {
                *pXpsPktCmd = XP_PKTCMD_FWD_MIRROR;
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Unknown packet action %d\n", sai_packet_action);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiConvertXpsPacketAction2Sai(xpPktCmd_e xpPktCmd,
                                             sai_packet_action_t* pPacketAction)
{
    switch (xpPktCmd)
    {
        case XP_PKTCMD_DROP:
            {
                *pPacketAction = SAI_PACKET_ACTION_DROP;
                break;
            }

        case XP_PKTCMD_FWD:
            {
                *pPacketAction = SAI_PACKET_ACTION_FORWARD;
                break;
            }

        case XP_PKTCMD_TRAP:
            {
                *pPacketAction = SAI_PACKET_ACTION_TRAP;
                break;
            }

        case XP_PKTCMD_FWD_MIRROR:
            {
                *pPacketAction = SAI_PACKET_ACTION_COPY;
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Unknown packet action %d\n", xpPktCmd);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_api_initialize(uint64_t flags,
                                const sai_service_method_table_t* services)
{
    // take the service table pointer to be used as required
    adapHostServiceMethodTable = services;

    XP_STATUS ret = XP_NO_ERR;
    sai_status_t saiRet = SAI_STATUS_SUCCESS;

    if (XP_SAI_LOG_FILE == XPSAI_DEFAULT_LOG_DEST)
    {
        xpSaiLogInit(XP_SAI_LOG_FILENAME);
    }
    else
    {
        xpSaiLogInit(NULL);
    }
    xpSaiSignalRegister();
    XP_SAI_LOG_NOTICE("xpSaiSignalRegister Success ...\n");

    // now call the init the method table for each sai_api_t

    // init for sai_api_t SAI_API_SWITCH
    if ((ret = xpSaiSwitchApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSwitchApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    //init for sai_api_t SAI_API_PORT
    if ((ret = xpSaiPortApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiPortApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_FDB
    if ((ret = xpSaiFdbApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiFdbApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_VLAN
    if ((ret = xpSaiVlanApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiVlanApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_STP
    if ((ret = xpSaiStpApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiStpApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_LAG
    if ((ret = xpSaiLagApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiLagApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_VIRTUAL_ROUTER
    if ((ret = xpSaiVirtualRouterApiInit(flags,
                                         adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiVirtualRouterApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_ROUTE
    if ((ret = xpSaiRouteApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiRouteApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_NEXT_HOP
    if ((ret = xpSaiNextHopApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiNextHopApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_NEXT_HOP_GROUP
    if ((ret = xpSaiNextHopGroupApiInit(flags,
                                        adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiNextHopGroupApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_ROUTER_INTERFACE
    if ((ret = xpSaiRouterInterfaceApiInit(flags,
                                           adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiRouterInterfaceApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_NEIGHBOR
    if ((ret = xpSaiNeighborApiInit(flags,
                                    adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiNeighborApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_MIRROR
    if ((saiRet = xpSaiMirrorApiInit(flags,
                                     adapHostServiceMethodTable)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiMirrorApiInit \n");
        return saiRet;
    }

    // init for sai_api_t SAI_API_ACL
    if ((ret = xpSaiAclApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiAclApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_HOST_INTERFACE
    if ((ret = xpSaiHostInterfaceApiInit(flags,
                                         adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiHostInterfaceApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_SAMPLE_PACKET
    if ((ret = xpSaiSamplePacketApiInit(flags,
                                        adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSamplePacketApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_POLICER
    if ((ret = xpSaiPolicerApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiPolicerApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_WRED
    if ((ret = xpSaiWredApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiWredApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_QOS_MAPS
    if ((ret = xpSaiQosMapApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiQosMapApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_QUEUE
    if ((ret = xpSaiQueueApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiQueueApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_SCHEDULER
    if ((ret = xpSaiSchedulerApiInit(flags,
                                     adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSchedulerApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_SCHEDULER_GROUP
    if ((ret = xpSaiSchedulerGroupApiInit(flags,
                                          adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSchedulerGroupApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_BUFFERS
    if ((ret = xpSaiBufferApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiBufferApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_HASH
    if ((ret = xpSaiHashApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiHashApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_UDF
    if ((ret = xpSaiUdfApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiUdfApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_BRIDGE
    if ((ret = xpSaiBridgeApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiBridgeApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_MCAST_FDB
    if ((saiRet = xpSaiMcastFdbApiInit(flags,
                                       adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiMcastFdbApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_L2MC
    if ((saiRet = xpSaiL2McApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiL2McApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_L2MC_GROUP
    if ((saiRet = xpSaiL2McGroupApiInit(flags,
                                        adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiL2McGroupApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_TUNNEL
    if ((ret = xpSaiTunnelApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiTunnelApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_IPMC
    if ((saiRet = xpSaiIpmcApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiIpmcApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_IPMC_GROUP
    if ((saiRet = xpSaiIpmcGroupApiInit(flags,
                                        adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiIpmcGroupApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_RPF_GROUP
    if ((saiRet = xpSaiRpfGroupApiInit(flags,
                                       adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiRpfGroupApiInit \n");
        return SAI_STATUS_FAILURE;
    }

    // init for sai_api_t SAI_API_UDF
    if ((ret = xpSaiCounterApiInit(flags, adapHostServiceMethodTable)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiCounterApiInit \n");
        return SAI_STATUS_FAILURE;
    }
    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiApiRegister(sai_api_t sai_api_id, void* xpSaiApi)
{
    if ((_sai_api_extensions_t)sai_api_id >= MAX_SAI_API_EXTENSION)
    {
        XP_SAI_LOG_ERR("Error: Invalid SAI API ID - %d\n", sai_api_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpSaiApiTableArr[sai_api_id] = xpSaiApi;

    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_api_query(_In_ sai_api_t sai_api_id,
                           _Out_ void** api_method_table)
{
    if ((_sai_api_extensions_t)sai_api_id >= MAX_SAI_API_EXTENSION)
    {
        XP_SAI_LOG_ERR("Error: Invalid SAI API ID - %d\n", sai_api_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *api_method_table = xpSaiApiTableArr[sai_api_id];

    return SAI_STATUS_SUCCESS;
}


sai_status_t sai_api_uninitialize()
{
    XP_STATUS ret = XP_NO_ERR;
    sai_status_t saiRet = SAI_STATUS_SUCCESS;

    // now call the deinit the method table for each sai_api_t

    // deinit for sai_api_t SAI_API_SWITCH
    if ((ret = xpSaiSwitchApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSwitchApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    //`deinit for sai_api_t SAI_API_PORT
    if ((ret = xpSaiPortApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiPortApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_FDB
    if ((ret = xpSaiFdbApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiFdbApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_VLAN
    if ((ret = xpSaiVlanApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiVlanApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_STP
    if ((ret = xpSaiStpApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiStpApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_LAG
    if ((ret = xpSaiLagApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiLagApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_VIRTUAL_ROUTER
    if ((ret = xpSaiVirtualRouterApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiVirtualRouterApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_ROUTE
    if ((ret = xpSaiRouteApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiRouteApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_NEXT_HOP
    if ((ret = xpSaiNextHopApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiNextHopApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_NEXT_HOP_GROUP
    if ((ret = xpSaiNextHopGroupApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiNextHopGroupApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_ROUTER_INTERFACE
    if ((ret = xpSaiRouterInterfaceApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiRouterInterfaceApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_NEIGHBOR
    if ((ret = xpSaiNeighborApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiNeighborApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_MIRROR
    if ((saiRet = xpSaiMirrorApiDeInit()) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiMirrorApiDeInit \n");
        return saiRet;
    }

    // deinit for sai_api_t SAI_API_ACL
    if ((ret = xpSaiAclApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiAclApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_HOST_INTERFACE
    if ((ret = xpSaiHostInterfaceApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiHostInterfaceApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // deinit for sai_api_t SAI_API_SAMPLE_PACKET
    if ((ret = xpSaiSamplePacketApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSamplePacketApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_POLICER
    if ((ret = xpSaiPolicerApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiPolicerApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_WRED
    if ((ret = xpSaiWredApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiWredApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_QOS_MAPS
    if ((ret = xpSaiQosMapApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiqosMapApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_QUEUE
    if ((ret = xpSaiQueueApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiQueueApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_SCHEDULER
    if ((ret = xpSaiSchedulerApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSchedulerApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_SCHEDULER_GROUP
    if ((ret = xpSaiSchedulerGroupApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiSchedulerGroupApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_BUFFERS
    if ((ret = xpSaiBufferApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiBufferApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_HASH
    if ((ret = xpSaiHashApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiHashApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_UDF
    if ((ret = xpSaiUdfApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiUdfApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_BRIDGE
    if ((ret = xpSaiBridgeApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiBridgeApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_MCAST_FDB
    xpSaiMcastFdbApiDeinit();

    // Deinit for sai_api_t SAI_API_L2MC
    xpSaiL2McApiDeinit();

    // Deinit for sai_api_t SAI_API_L2MC_GROUP
    xpSaiL2McGroupApiDeinit();

    // Deinit for sai_api_t SAI_API_TUNNEL
    if ((ret = xpSaiTunnelApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiTunnelApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // Deinit for sai_api_t SAI_API_IPMC
    xpSaiIpmcApiDeinit();

    // Deinit for sai_api_t SAI_API_IPMC_GROUP
    xpSaiIpmcGroupApiDeinit();

    // Deinit for sai_api_t SAI_API_RPF_GROUP
    xpSaiRpfGroupApiDeinit();

    // Deinit for sai_api_t SAI_API_COUNTER
    if ((ret = xpSaiCounterApiDeinit()) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: failed xpSaiCounterApiDeinit \n");
        return SAI_STATUS_FAILURE;
    }

    // take the service table pointer to be used as required
    adapHostServiceMethodTable = NULL;

    xpSaiLogDeInit();

    return SAI_STATUS_SUCCESS;
}

/*
 * SAI syncd implementation dependent API's
 * STUBS
 */
sai_object_id_t sai_switch_id_query(sai_object_id_t sai_object_id)
{
    return SAI_OBJECT_TYPE_NULL;
}

sai_status_t sai_dbg_generate_dump(const char*)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_object_type_get_availability(
    _In_ sai_object_id_t switch_id,
    _In_ sai_object_type_t object_type,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _Out_ uint64_t *count)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_query_attribute_enum_values_capability(
    _In_ sai_object_id_t switch_id,
    _In_ sai_object_type_t object_type,
    _In_ sai_attr_id_t attr_id,
    _Inout_ sai_s32_list_t *enum_values_capability)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_query_attribute_capability(
    _In_ sai_object_id_t switch_id,
    _In_ sai_object_type_t object_type,
    _In_ sai_attr_id_t attr_id,
    _Out_ sai_attr_capability_t *attr_capability)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}

sai_status_t sai_query_stats_capability(
    _In_ sai_object_id_t switch_id,
    _In_ sai_object_type_t object_type,
    _Inout_ sai_stat_capability_list_t *stats_capability)
{
    return SAI_STATUS_NOT_IMPLEMENTED;
}


/*
* Routine Description:
*     Convert device ID to scope ID
*
* Arguments:
*     [in] xpsDevice_t
*
* Return Values:
*    Return xpsScope_t
*/
xpsScope_t xpSaiScopeFromDevGet(xpsDevice_t xpsDevId)
{
    return xpsDevId;
}

/*
* Routine Description:
*     Query sai object type.
*
* Arguments:
*     [in] sai_object_id_t
*
* Return Values:
*    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
*    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
*/
sai_object_type_t sai_object_type_query(sai_object_id_t sai_object_id)
{
    sai_uint64_t sai_object_type = sai_object_id >> XDK_SAI_OBJID_VALUE_BITS;

    if ((SAI_OBJECT_TYPE_NULL < sai_object_type) &&
        (SAI_OBJECT_TYPE_MAX > sai_object_type))
    {
        return (sai_object_type_t)sai_object_type;
    }
    return SAI_OBJECT_TYPE_NULL;
}

/*
* Routine Description:
*     Creates SAI Object ID which contains switch ID
*
* Arguments:
*     [in] sai_object_type_t : object type
*     [in] sai_uint32_t      : switch ID
*     [in] sai_uint64_t      : only 40 bits of income value are being used
*     [out] sai_object_id_t
*
* Return Values:
*    sai_status_t
*/
sai_status_t xpSaiObjIdCreate(sai_object_type_t sai_object_type,
                              sai_uint32_t dev_id,
                              sai_uint64_t local_id, sai_object_id_t *sai_object_id)
{
    sai_uint64_t type_tmp = SAI_NULL_OBJECT_ID;
    sai_uint64_t switch_tmp = SAI_NULL_OBJECT_ID;

    if (sai_object_id == 0)
    {
        XP_SAI_LOG_ERR("Invalid SAI object id value.");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((SAI_OBJECT_TYPE_NULL >= sai_object_type) &&
        (SAI_OBJECT_TYPE_MAX <= sai_object_type))
    {
        XP_SAI_LOG_ERR("Invalid SAI object type received(%u).", sai_object_type);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (local_id > XDK_SAI_OBJID_VALUE_MASK)
    {
        XP_SAI_LOG_ERR("Local identifier is too big (%lu). Only 40 bits are allowed.",
                       (long unsigned int) local_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (dev_id > XDK_SAI_OBJID_SWITCH_MAX_VAL)
    {
        XP_SAI_LOG_ERR("Switch identifier is too big (%u). Only 8 bits are allowed.",
                       dev_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    type_tmp = (sai_uint64_t) sai_object_type << XDK_SAI_OBJID_VALUE_BITS;
    switch_tmp = (sai_uint64_t) dev_id << XDK_SAI_OBJID_SWITCH_BITS;
    *sai_object_id = type_tmp | switch_tmp | local_id;

    XP_SAI_LOG_DBG("xpSaiObjIdCreate:object type = %s ,ID is 0x%lx (%lu)\n",
                   xpSaiObjIdToStr(sai_object_type), *sai_object_id, *sai_object_id);

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Retrieves switch ID from SAI Object ID
*
* Arguments:
*     [in] sai_object_id_t : object type
*
* Return Values:
*    sai_uint32_t
*/
sai_uint32_t xpSaiObjIdSwitchGet(sai_object_id_t sai_object_id)
{
    return (sai_uint32_t)((sai_object_id & XDK_SAI_OBJID_SWITCH_MASK) >>
                          XDK_SAI_OBJID_SWITCH_BITS);
}

/*
* Routine Description:
*     Retrieves scope ID from SAI Object ID
*
* Arguments:
*     [in] sai_object_id_t : object type
*
* Return Values:
*    xpsScope_t
*/
xpsScope_t xpSaiObjIdScopeGet(sai_object_id_t sai_object_id)
{
    sai_uint32_t switch_id = 0;

    switch_id = xpSaiObjIdSwitchGet(sai_object_id);

    return xpSaiScopeFromDevGet(switch_id);
}

/*
* Routine Description:
*     Query sai object value.
*
* Arguments:
*     [out] sai_object_id_t
*
* Return Values:
*    sai_uint64_t : only 40 bits of income value are usable
*/
sai_uint64_t xpSaiObjIdValueGet(sai_object_id_t sai_object_id)
{
    /*Income validation haven't sense as there will be present upcoming validation for the local identifier.*/
    return sai_object_id & XDK_SAI_OBJID_VALUE_MASK;
}

/*
* Routine Description:
*     Query sai object type.
*
* Arguments:
*     [in] sai_object_id_t
*
* Return Values:
*    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
*    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
*/
sai_object_type_t xpSaiObjIdTypeGet(sai_object_id_t sai_object_id)
{
    return sai_object_type_query(sai_object_id);
}

/*
* Routine Description: Reverse MAC address.
*
* Arguments:
*     [inout] sai_mac_t mac - MAC address to be reversed
*
* Return Values:
*    Return: Pointer to reversed MAC address.
*/
uint8_t* xpSaiMacReverse(sai_mac_t mac)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t macAddr[XP_MAC_ADDR_LEN];

    memcpy(macAddr, mac, XP_MAC_ADDR_LEN);
    for (i = 0, j = XP_MAC_ADDR_LEN - 1; i < XP_MAC_ADDR_LEN; i++, j--)
    {
        mac[i] = macAddr[j];
    }

    return mac;
}

/*
* Routine Description: Reverse IPv4 address.
*
* Arguments:
*     [in] ipv4Addr - address
*     [out] buffer - reversed byte array
*
*/
void xpSaiIPv4ReverseCopy(sai_ip4_t ipv4Addr, uint8_t* buffer)
{
    uint32_t i   = 0;
    uint32_t len = sizeof(sai_ip4_t);

    for (i = 0; i < len; ++i)
    {
        buffer[len-1-i] = ((uint8_t*)&ipv4Addr)[i];
    }
}

/*
* Routine Description: Reverse IPv6 address.
*
* Arguments:
*     [inout] ipv6Addr - address to be reversed
*
* Return Values:
*    Return: Pointer to reversed address.
*/
uint8_t* xpSaiIPv6Reverse(sai_ip6_t ipv6Addr)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t ipv6Addr_buf[16];

    memcpy(ipv6Addr_buf, ipv6Addr, 16);

    for (i = 0, j = 16 - 1; i < 16; i++, j--)
    {
        ipv6Addr[i] = ipv6Addr_buf[j];
    }

    return ipv6Addr;
}

/*
* Routine Description: Check for valid MAC
*
* Arguments:
*     [in] mac - MAC to be validate
*
* Return Values:
*    Return: SUCCESS for valid MAC or appropriate error if not
*/
sai_status_t xpSaiIsValidUcastMac(sai_mac_t mac)
{
    uint8_t index = 0;
    sai_mac_t invalidMac[2] =
    {
        {0, 0, 0, 0, 0, 0},
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
    };

    /* Check whether MAC is empty or broadcast MAC address */
    for (index = 0; index < ARRAY_SIZE(invalidMac); index++)
    {
        if (0 == memcmp(mac, invalidMac[index], sizeof(sai_mac_t)))
        {
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
    }

    /* Check whether MAC is multicast address */
    if (mac[0] & 1)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description: Negate mask
*
* Arguments:
*     [in] mask - mask that need to negate
*     [in] size - size of mask in byte
*
* Return Values:
*    Return: Negate mask
*/
uint8_t* xpSaiNegMask(uint8_t* mask, uint8_t size)
{
    uint32_t i = 0;

    for (i = 0; i < size; i++)
    {
        mask[i] = (~(mask[i]));
    }

    return mask;
}

sai_status_t xpSaiGetCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                           void *key, void **ctxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;

    if (key == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((retVal = xpsStateSearchData(scopeId, xpSaiDbHndl, (xpsDbKey_t) key,
                                     ctxPtr)) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (!(*ctxPtr))
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiRemoveCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                              void *key)
{
    XP_STATUS retVal = XP_NO_ERR;
    void *ctxPtr = NULL;

    if (key == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((retVal = xpsStateDeleteData(scopeId, xpSaiDbHndl, (xpsDbKey_t) key,
                                     (void**) &ctxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Delete data failed | retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((retVal = xpsStateHeapFree((void*) ctxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Free failed\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiInsertCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                              void *entryCtxData, size_t size, void **entryCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;

    if ((retVal = xpsStateHeapMalloc(size, (void**) entryCtxPtr)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XPS state Heap Allocation failed \n");
        return xpsStatus2SaiStatus(retVal);
    }

    if (*entryCtxPtr == NULL || entryCtxData == NULL)
    {
        return SAI_STATUS_FAILURE;
    }

    memcpy(*entryCtxPtr, entryCtxData, size);

    // Insert the context into the database
    if ((retVal = xpsStateInsertData(scopeId, xpSaiDbHndl,
                                     (void*) *entryCtxPtr)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*) *entryCtxPtr);
        *entryCtxPtr = NULL;
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetNextCtxDb(xpsScope_t scopeId, xpsDbHandle_t xpSaiDbHndl,
                               void *key, void **ctxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsStateGetNextData(scopeId, xpSaiDbHndl, (xpsDbKey_t) key, ctxPtr);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve context object (retVal: %d)\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (*ctxPtr == NULL)
    {
        XP_SAI_LOG_ERR("Failed to get context object. No more elements in the list (retVal: %d)\n",
                       retVal);
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountCtxDbObjects(xpsScope_t scopeId,
                                    xpsDbHandle_t xpSaiDbHndl, uint32_t *count)
{
    XP_STATUS retVal = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter has been passed (count: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(scopeId, xpSaiDbHndl, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get number of context DB objects. (retVal: %d)\n",
                       retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


// Generic stub api the list of function pointers which are not registered with any SAI API.
sai_status_t xpSaiStubGenericApi(sai_object_id_t object_id, ...)
{
    return SAI_STATUS_NOT_SUPPORTED;
}


bool sai_shell_cmd_add_flexible(void * param, sai_shell_check_run_function fun)
{
    return true;
}

bool sai_shell_cmd_add(const char *cmd_name, sai_shell_function fun,
                       const char *description)
{
    return true;
}






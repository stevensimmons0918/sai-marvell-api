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
* @file sfdbCheetah.c
*
* @brief Cheetah FDB update, Triggered actions messages simulation
*
* @version   67
********************************************************************************
*/

#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smem/smem.h>
#include <asicSimulation/SKernel/sfdb/sfdb.h>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <common/SBUF/sbuf.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SLog/simLog.h>

/* Falcon : get the FDB tileId */
#define FDB_TILE_ID_GET(dev) \
    dev->numOfPipesPerTile ? \
        smemGetCurrentPipeId(dev) / dev->numOfPipesPerTile : \
        0


/* indication that the skernel task should wait until the application will reset
   the AUQ for new messages.

   the correct value is 0.

   for debug purposes only I allow to set it to 1 , to reconstruct starvation cases.
*/
GT_BIT  oldWaitDuringSkernelTaskForAuqOrFua = 0;
/* DEBUG function :
    to allow changing oldWaitDuringSkernelTaskForAuqOrFua from
   outside the simulation */
void simulationDebug_oldWaitDuringSkernelTaskForAuqOrFua(IN GT_BIT mode)
{
    oldWaitDuringSkernelTaskForAuqOrFua = mode;
}

static GT_STATUS sfdbChtNaMsgProcess(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbMsgPtr,
    OUT GT_U32               * entryIndexPtr
);

static GT_STATUS sfdbChtQxMsgProcess(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbMsgPtr
);

static GT_STATUS sfdbChtHrMsgProcess(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbMsgPtr
);

static GT_VOID sfdbChtWriteNaMsg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbMsgPtr,
    IN GT_U32                fdbIndex,
    OUT GT_U32               entryOffset
);

static GT_VOID sfdbChtTriggerTa(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * tblActPtr,
    IN GT_U32                tileId
);

static GT_VOID sfdbChtDoAging(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN GT_U32                * tblActPtr,
  IN GT_U32                  action,
  IN GT_U32                  firstEntryIdx,
  IN GT_U32                  numOfEntries,
  IN GT_U32                  tileId
);

static GT_VOID sfdbChtFdb2AuMsg(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  fdbIndex,
    IN GT_U32                * fdbEntryPtr,
    OUT GT_U32               * macUpdMsgPtr
);
static GT_VOID sip6FdbSpecialMuxedFieldsGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
);

static GT_STATUS sip6AuMsgConvertToFdbEntry
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 fdbIndex,
    IN GT_U32                *msgWordPtr,
    OUT GT_U32               *macEntry
);

typedef enum {
    SFDB_CHEETAH_TRIGGER_AGING_REMOVE_E = 0,
    SFDB_CHEETAH_TRIGGER_AGING_NO_REMOVE_E,
    SFDB_CHEETAH_TRIGGER_DELETE_E,
    SFDB_CHEETAH_TRIGGER_TRANS_E ,
    SFDB_CHEETAH_AUTOMATIC_AGING_REMOVE_E,
    SFDB_CHEETAH_AUTOMATIC_AGING_NO_REMOVE_E,
    SFDB_CHEETAH_TRIGGER_UPLOAD_E
}SFDB_CHEETAH_ACTION_ENT;

#define POWER_2(a) ((a)*(a))
#define _256K   (256*1024)
#define _512K   (512*1024)
#define _1_MILLION  (1000*1000)

#define SFDB_AUTO_AGING_BUFF_SIZE     100

#define SNET_CHEETAH_NO_FDB_ACTION_BUFFERS_SLEEP_TIME 50

typedef struct{
    /* transplant info : 'old interface' */
    GT_U32  oldPort;
    GT_U32  oldDev;
    GT_U32  oldIsTrunk;
    /* transplant info : 'new interface' */
    GT_U32  newPort;
    GT_U32  newDev;
    GT_U32  newIsTrunk;

    /* all actions (not transplant) info: value */
    GT_U32  actVlan;                /*also relevant for transplant action */
    GT_U32  actIsTrunk;
    GT_U32  actPort;
    GT_U32  actDevice;
    GT_U32  actUserDefined;
    GT_U32  actVID1;

    /* all actions (not transplant) info: mask */
    GT_U32  actVlanMask;            /*also relevant for transplant action */
    GT_U32  actIsTrunkMask;
    GT_U32  actPortMask;
    GT_U32  actDeviceMask;
    GT_U32  actUserDefinedMask;
    GT_U32  actVID1Mask;           /*also relevant for transplant action */


    GT_U32  staticDelEn;                /* Enable static entries deletion */
    GT_U32  staticTransEn;              /* Enable static entries translating */
    GT_U32  removeStaticOnNonExistDev;  /* The device automatically removes
                                        entries associated with devices that are
                                        not defined it the system. In addition
                                        to regular entries associated with
                                        devices that are not defined in the
                                        system, this field enables the removal
                                        of Static entries associated with those
                                        devices.
                                        0 - Static entries associated with non
                                            existing devices are not removed
                                        1- Static entries associated with non
                                        existing devices are removed */
    GT_U32  ageOutAllDevOnTrunk;/* do age on trunk entries that registered on
                                   other devices */
    GT_U32  ageOutAllDevOnNonTrunk;/* do age NOT on trunk entries that
                                      registered on other devices */
    GT_U32  aaAndTaMessageToCpuEn;/* enable/disable AA/TA messages to the CPU */

    GT_U32  routeUcAaAndTaMessageToCpuEn;/* route UC enable/disable AA/TA messages to the CPU */
    GT_U32  routeUcAgingEn;              /* route UC aging enable/disable */
    GT_U32  routeUcTransplantingEn;      /* route UC transplanting enable/disable */
    GT_U32  routeUcDeletingEn;           /* route UC deleting enable/disable */

    GT_U32* deviceTableBmpPtr;/* pointer to the BMP of 'device table'.
                              This device table is used by the aging mechanism
                              to delete all MAC entries associated with devices
                              that are not present in the system (hot removal)*/
    GT_U32  ownDevNum;        /* the 'own devNum' of this device */
    GT_U32  maskAuFuMessageToCpuOnNonLocal;/* Lion B -  When Enabled - AU/FU messages are
                                not sent to the CPU if the MAC entry does NOT reside
                                on the local core, i.e. the entry port[5:4] != LocalCore
                                0x0 = Disable;
                                0x1 = Enable; */
    GT_U32  maskAuFuMessageToCpuOnNonLocal_with3BitsCoreId; /*support mask of 3 bits of coreId with 3 bits from the entry : 2 of port + 1 of devNum */
    GT_U32  srcIdLengthInFdbMode;/* SIP5 (and not SIP6) is Max Length SrcID in FDB Enabled ? */

    GT_U32  multiHashEnable;/* SIP5: is the multi hash enabled --->
                            0 - <multi hash enabled> not enabled --> means to use the skip bit
                            1 - <multi hash enabled>     enabled --> means to ignore the skip bit
                            */
/* cq# 150852 */
#define AgeAccordingToConfiguration 0
#define ForceAgeWithoutRemoval      1
    GT_U32  trunksAgingMode;/*  Trunks aging control
                 0x0 = AgeAccordingToConfiguration; Entries associated with Trunks
                    are aged out according to the ActionMode configuration.
                 0x1 = ForceAgeWithoutRemoval; Entries associated with Trunks are
                        aged-without-removal
                        regardless of the dev# they are associated with and
                        regardless of the configuration in the ActionMode configuration
                         */


    GT_U32  McAddrDelMode;/*    Disable removal of Multicast addresses during the aging process.
                                0 = Remove: Remove MC addresses in Aging process
                                1 = DontRemove: Do not remove MC addresses in Aging process*/
    GT_U32  IPMCAddrDelMode;/*This field sets the address deleting mode for IP MC FDB entries
                                NOTE: This is in addition to current configuration <MCAddrDelMode>
                                that controls aging on non IP Multicast entries in FDB.
                                0 = AgeNonStatic: IP MC entries are subjected to aging if not static.
                                1 = NoAge: IP MC entries are not subjected to aging regardless if static.*/

    GT_U32 SPEntryFastAgeEn;    /* If set, enables fast aging of SP (Storm Prevention) FDB entries.
                                    0 = Disable: SP entries are treated as regular entries for aging process.
                                    1 = Enable: SP entries are aged in the first pass, and filtering aging by FID is disabled.
                                */
}AGE_DAEMON_ACTION_INFO_STC;


/**
* @internal parseAuMsgGetFdbEntryHashInfo function
* @endinternal
*
* @brief   parse the AU message and get the FDB entry hash info .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] msgWordPtr               - pointer to fdb message.
*
* @param[out] entryInfoPtr             - (pointer to) entry hash info
*                                      RETURN:
*                                      GT_OK           - FDB entry was found
*                                      GT_NOT_FOUND    - FDB entry was not found and assigned with new address
*                                      GT_FAIL         - FDB entry was not found and no free address space
*                                      COMMENTS:
*/
static GT_STATUS parseAuMsgGetFdbEntryHashInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr,
    OUT SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC    *entryInfoPtr
)
{
    DECLARE_FUNC_NAME(parseAuMsgGetFdbEntryHashInfo);

    GT_U32  fieldValue;
    GT_U32  macAddrWords[2];
    GT_U32  regValue = 0;

    memset(entryInfoPtr, 0, sizeof(SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC));

    entryInfoPtr->crcInitValueMode =
        SMEM_CHT_IS_SIP5_15_GET(devObjPtr) ? CRC_INIT_VALUE_MODE_FORCE_ALL_ONE_E :
        SMEM_CHT_IS_SIP5_GET(devObjPtr)    ? CRC_INIT_VALUE_MODE_FORCE_ALL_ZERO_E :
        CRC_INIT_VALUE_MODE_FORCE_ALL_ONE___64K_ALL_ZER0__E;


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* entry type */
        entryInfoPtr->entryType =
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE);
        /* FID */
        entryInfoPtr->fid =
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FID);
        entryInfoPtr->fdbLookupKeyMode =
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE);
        /* NOTE: the actual place that do hash need to check vid1 mux mode */
        entryInfoPtr->vid1 =
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_0);
        entryInfoPtr->vid1 |=
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_6_1) << 1;
        entryInfoPtr->vid1 |=
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_11_7) << 7;
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* entry type */
        entryInfoPtr->entryType =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE);
        /* FID */
        entryInfoPtr->fid =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FID);

        if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {
            /* FDB lookup mode */
            entryInfoPtr->fdbLookupKeyMode =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE);

            /* VID1_assignment_mode */
            fieldValue = SMEM_U32_GET_FIELD(regValue, 27, 1);
            __LOG(("VID1 assignemnet mode: %d\n", fieldValue));
            if (fieldValue)
            {
                /* VID1 used in FDB entry hash entry for double tag FDB Lookup */
                entryInfoPtr->vid1 =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1);
            }
        }
    }
    else
    {
        /* entry type bits [6:4] word0 */
        entryInfoPtr->entryType = SMEM_U32_GET_FIELD(msgWordPtr[3], 19, 2);
        entryInfoPtr->fid = snetFieldValueGet(msgWordPtr,64,12);
    }

    /* save the original fid before any change */
    entryInfoPtr->origFid = entryInfoPtr->fid;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        entryInfoPtr->fid16BitHashEn = SMEM_U32_GET_FIELD(regValue, 4, 1);

        if(entryInfoPtr->fid16BitHashEn == 0)
        {
            entryInfoPtr->fid &= 0xfff;
        }
    }
    else
    {
        entryInfoPtr->fid16BitHashEn = 0;
    }

    switch(entryInfoPtr->entryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    /* get MAC address from message */
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,msgWordPtr,
                        macAddrWords);
                }
                else
                {
                    /* get MAC address from message */
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,msgWordPtr,
                        macAddrWords);
                }

                entryInfoPtr->info.macInfo.macAddr[5] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 0) ,8);
                entryInfoPtr->info.macInfo.macAddr[4] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 1) ,8);
                entryInfoPtr->info.macInfo.macAddr[3] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 2) ,8);
                entryInfoPtr->info.macInfo.macAddr[2] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 3) ,8);
                entryInfoPtr->info.macInfo.macAddr[1] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 4) ,8);
                entryInfoPtr->info.macInfo.macAddr[0] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 5) ,8);
            }
            else
            {
                /* get MAC address from message */
                entryInfoPtr->info.macInfo.macAddr[5] = (GT_U8)((msgWordPtr[0] >> 16) & 0xff);
                entryInfoPtr->info.macInfo.macAddr[4] = (GT_U8)((msgWordPtr[0] >> 24) & 0xff);
                entryInfoPtr->info.macInfo.macAddr[3] = (GT_U8)(msgWordPtr[1]         & 0xff);
                entryInfoPtr->info.macInfo.macAddr[2] = (GT_U8)((msgWordPtr[1] >> 8)  & 0xff);
                entryInfoPtr->info.macInfo.macAddr[1] = (GT_U8)((msgWordPtr[1] >> 16) & 0xff);
                entryInfoPtr->info.macInfo.macAddr[0] = (GT_U8)((msgWordPtr[1] >> 24) & 0xff);
            }

            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                fieldValue = SMEM_U32_GET_FIELD(regValue, 0, 2);

                switch(fieldValue)
                {
                    case 0:
                        entryInfoPtr->info.macInfo.crcHashUpperBitsMode = SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
                        break;
                    case 1:
                        entryInfoPtr->info.macInfo.crcHashUpperBitsMode = SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_FID_E;
                        break;
                    case 2:
                        entryInfoPtr->info.macInfo.crcHashUpperBitsMode = SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_MAC_E;
                        break;
                    case 3:
                    default:
                        __LOG(("ERROR : unknown <crcHashUpperBitsMode> value [%d] \n",
                                      fieldValue));
                        entryInfoPtr->info.macInfo.crcHashUpperBitsMode = SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
                        break;
                }

            }
            else
            {
                entryInfoPtr->info.macInfo.crcHashUpperBitsMode = SNET_CHEETAH_FDB_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E;
            }

            break;
        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:/*IPv4 Multicast address entry (IGMP snooping);*/
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:/*IPv6 Multicast address entry (MLD snooping);*/
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* get SIP,DIP address from message */
                entryInfoPtr->info.ipmcBridge.dip =
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DIP);

                entryInfoPtr->info.ipmcBridge.sip =
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_26_0);
                entryInfoPtr->info.ipmcBridge.sip |=
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_30_27) << 27;
                entryInfoPtr->info.ipmcBridge.sip |=
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_31) << 31;
            }
            else
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /* get SIP,DIP address from message */
                entryInfoPtr->info.ipmcBridge.dip =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DIP);

                entryInfoPtr->info.ipmcBridge.sip =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SIP);
            }
            else
            {
                entryInfoPtr->info.ipmcBridge.dip =
                    SMEM_U32_GET_FIELD(msgWordPtr[0], 16, 16) |
                    SMEM_U32_GET_FIELD(msgWordPtr[1], 0, 16) << 16;

                entryInfoPtr->info.ipmcBridge.sip =
                    SMEM_U32_GET_FIELD(msgWordPtr[1], 16, 16) |
                    SMEM_U32_GET_FIELD(msgWordPtr[3], 0, 12) << 16 |
                    SMEM_U32_GET_FIELD(msgWordPtr[3], 27, 4) << 28;
            }
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* ipv4 dip */
                entryInfoPtr->info.ucRouting.dip[0] =
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP);

                /* vrf id */
                entryInfoPtr->info.ucRouting.vrfId =
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID);
            }
            else
            {
                /* ipv4 dip */
                entryInfoPtr->info.ucRouting.dip[0] =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP);

                /* vrf id */
                entryInfoPtr->info.ucRouting.vrfId =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID);
            }
            break;

        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* fcoe d_id */
                entryInfoPtr->info.ucRouting.dip[0] =
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID);

                /* vrf id */
                entryInfoPtr->info.ucRouting.vrfId =
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID);

            }
            else
            {
                /* fcoe d_id */
                entryInfoPtr->info.ucRouting.dip[0] =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID);

                /* vrf id */
                entryInfoPtr->info.ucRouting.vrfId =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID);
            }
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:
            __LOG(("ipv6 'not supported' (the device not support 'by message' the ipv6 entries)"));
            /* no break here */
            GT_ATTR_FALLTHROUGH;
        default:
            return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal parseAuMsgGetFdbEntryIndex function
* @endinternal
*
* @brief   parse the AU message and get the FDB entry index + entry's register address .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] msgWordPtr               - pointer to fdb message.
*
* @param[out] entryIndexPtr            - index for the FDB found or free entry
* @param[out] entryOffsetPtr           - FDB offset within the 'bucket'
*                                      RETURN:
*                                      GT_OK           - FDB entry was found
*                                      GT_NOT_FOUND    - FDB entry was not found and assigned with new address
*                                      GT_FAIL         - FDB entry was not found and no free address space
*                                      COMMENTS:
*/
static GT_STATUS parseAuMsgGetFdbEntryIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr,
    OUT GT_U32               * entryIndexPtr,
    OUT GT_U32               * entryOffsetPtr
)
{
    SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC    entryHashInfo;
    GT_STATUS       status;

    status = parseAuMsgGetFdbEntryHashInfo(devObjPtr,msgWordPtr,&entryHashInfo);
    if(status != GT_OK)
    {
        return status;
    }

    return snetChtL2iMacEntryAddress(devObjPtr, &entryHashInfo, entryIndexPtr, entryOffsetPtr);
}


/**
* @internal sfdbChtMsgProcess function
* @endinternal
*
* @brief   Process FDB update message.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbMsgPtr                - pointer to device object.
*/
GT_VOID sfdbChtMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * fdbMsgPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtMsgProcess);

    GT_U32 * msgWordPtr;                /* pointer to the fdb first word */
    SFDB_UPDATE_MSG_TYPE msgType;       /* AU Message Type */
    GT_U32 regEntry = 0;                    /* register entry  */
    GT_U32 fldValue;                    /* register field value */
    GT_STATUS status = GT_TRUE;         /* function return status */
    GT_U32  entryIndex = 0;             /* entry index */

    if(devObjPtr->fdbNumEntries == 0)
    {
        /* can be in sip6.30 when FDB used only by the DDE for HSR-PRP */
        __LOG(("FDB size is ZERO !!! no action supported \n"));
        return;
    }

    msgWordPtr = (GT_U32 *)fdbMsgPtr;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* MSG type */
        msgType =
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE);
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* MSG type */
        msgType =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE);
    }
    else
    {
        /* MSG type bits [6:4] word0 */
        msgType = SMEM_U32_GET_FIELD(msgWordPtr[0], 4, 3);
    }

    switch (msgType)
    {
        case SFDB_UPDATE_MSG_NA_E:
            status = sfdbChtNaMsgProcess(devObjPtr, msgWordPtr,&entryIndex);
            break;
        case SFDB_UPDATE_MSG_QA_E:
            status = sfdbChtQxMsgProcess(devObjPtr, msgWordPtr);

            if(devObjPtr->needResendMessage)
            {
                /* indication that the message was not processed */
                /* we need to re-try later */
                return;
            }

            break;
        case SFDB_UPDATE_MSG_HR_E:
            if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                skernelFatalError("sfdbChtMsgProcess: wrong message type %d",
                                   msgType);
            }
            /* Hash Request - called by the CPU to calculate all the HASH functions that the devices can generate :
                CRC -
                XOR -
                16 CRC multi-hash -
            */
            status = sfdbChtHrMsgProcess(devObjPtr, msgWordPtr);
            break;

        default:
            skernelFatalError("sfdbChtMsgProcess: wrong message type %d",
                               msgType);
    }

    /* Clear message trigger bit when the action is completed  */
    SMEM_U32_SET_FIELD(regEntry, 0, 1, 0);

    fldValue = (status == GT_FAIL) ? 0 : 1;
    /* AU Message Status */
    SMEM_U32_SET_FIELD(regEntry, 1, 1, fldValue);

    if(status != GT_FAIL &&
       SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       msgType == SFDB_UPDATE_MSG_NA_E)
    {
        GT_U32  mask;

        if(devObjPtr->multiHashEnable)
        {
            mask = 0xF; /*only 4 bits*/
        }
        else
        {
            mask = 0x3; /*only 2 bits*/
        }

        entryIndex &= mask;

        /* update the NAEntryOffset (with lower 5 bits of the index in the FDB) */
        smemRegFldSet(devObjPtr, SMEM_CHT_MSG_FROM_CPU_REG(devObjPtr), 2,5,entryIndex);
    }

    smemRegFldSet(devObjPtr, SMEM_CHT_MSG_FROM_CPU_REG(devObjPtr), 0,2,regEntry);

}

/**
* @internal sfdbSip6ParseFdbEntry function
* @endinternal
*
* @brief   Parses FDB entry to the struct
*/
GT_VOID sfdbSip6ParseFdbEntry
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    GT_U32                          *fdbEntryPtr,
    IN    GT_U32                           entryIndex,
    OUT   SNET_SIP6_FDB_ENTRY_INFO        *fdbEntryInfoPtr
)
{
    DECLARE_FUNC_NAME(sfdbSip6ParseFdbEntry);

#define SIP6_GET_VALUE(structFieldName, tableFieldName)\
    fdbEntryInfoPtr->structFieldName = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr, entryIndex, tableFieldName)

    SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC specialFields;
    GT_U32  fldValue,regValue;
    SNET_CHEETAH_FDB_ENTRY_ENT fdb_entry_type;
    GT_U32  fdb_da_cmd_drop_type; /* 0= Soft Drop , 1= Hard Drop */
    GT_U32  fdb_sa_cmd_drop_type; /* 0= Soft Drop , 1= Hard Drop */

    SIP6_GET_VALUE(valid                  , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID                     );
    if(fdbEntryInfoPtr->valid == 0)
    {
        /* do not parse the rest of the entry ... not relevant ... save some time in FDB scanning */
        return;
    }

    SIP6_GET_VALUE(skip                   , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                      );
    SIP6_GET_VALUE(age                    , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                       );
    SIP6_GET_VALUE(fdb_entry_type         , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE            );

    fdb_entry_type = fdbEntryInfoPtr->fdb_entry_type;

    if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_MAC_E ||
       fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E ||
       fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E)
    {
        SIP6_GET_VALUE(fid                    , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FID                       );
        SIP6_GET_VALUE(dev_id                 , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID                    );
        SIP6_GET_VALUE(sa_access_level        , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL           );
        SIP6_GET_VALUE(dip                    , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DIP                       );
        SIP6_GET_VALUE(da_access_level        , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL           );
        SIP6_GET_VALUE(vidx                   , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VIDX                      );
        SIP6_GET_VALUE(is_trunk               , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK                  );
        SIP6_GET_VALUE(trunk_num              , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM                 );
        SIP6_GET_VALUE(eport_num              , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM                 );
        SIP6_GET_VALUE(multiple               , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MULTIPLE                  );
        SIP6_GET_VALUE(is_static              , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC                 );
        SIP6_GET_VALUE(da_cmd                 , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD                    );
        SIP6_GET_VALUE(sa_cmd                 , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD                    );
        SIP6_GET_VALUE(da_route               , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE                  );
        SIP6_GET_VALUE(sp_unknown             , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN                );
        SIP6_GET_VALUE(app_specific_cpu_code  , SMEM_SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE     );
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            SIP6_GET_VALUE(epg_streamId       , SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_EPG_STREAM_ID           );
        }

        if(fdbEntryInfoPtr->sa_cmd == 1 || fdbEntryInfoPtr->da_cmd == 3)
        {
            /* NOTE: the  da_cmd , sa_cmd hold only 2 bits !!! and value '3' means 'drop'
               for the support of 'hard drop '/'soft drop' , we need to get values from the global config.
            */
            smemRegGet(devObjPtr, SMEM_SIP6_FDB_GLOBAL_CONFIG_2_REG(devObjPtr) ,&regValue);

            fdb_da_cmd_drop_type = SMEM_U32_GET_FIELD(regValue,1,1);
            fdb_sa_cmd_drop_type = SMEM_U32_GET_FIELD(regValue,0,1);

            if(fdbEntryInfoPtr->sa_cmd == 1)
            {
                /* 0= Soft Drop , 1= Hard Drop */
                fdbEntryInfoPtr->sa_cmd = fdb_sa_cmd_drop_type == 0 ?
                    SKERNEL_EXT_PKT_CMD_SOFT_DROP_E:
                    SKERNEL_EXT_PKT_CMD_HARD_DROP_E;

                __LOG(("The <sa_cmd> drop converted to [%s] from global config \n",
                    (fdb_sa_cmd_drop_type == 0) ? "SOFT DROP" : "HARD DROP"));
            }

            if(fdbEntryInfoPtr->da_cmd == 3)
            {
                /* 0= Soft Drop , 1= Hard Drop */
                fdbEntryInfoPtr->da_cmd = fdb_da_cmd_drop_type == 0 ?
                    SKERNEL_EXT_PKT_CMD_SOFT_DROP_E:
                    SKERNEL_EXT_PKT_CMD_HARD_DROP_E;

                __LOG(("The <da_cmd> drop converted to [%s] from global config \n",
                    (fdb_da_cmd_drop_type == 0) ? "SOFT DROP" : "HARD DROP"));
            }
        }


        SMEM_SIP6_FDB_FDB_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,
            fdbEntryPtr,
            entryIndex,
            &fdbEntryInfoPtr->mac_addr[0]);

        memset(&specialFields,0,sizeof(SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC));
        sip6FdbSpecialMuxedFieldsGet(devObjPtr,fdbEntryPtr,entryIndex,&specialFields);

        fdbEntryInfoPtr->orig_vid1          = specialFields.origVid1;
        fdbEntryInfoPtr->source_id          = specialFields.srcId;
        fdbEntryInfoPtr->user_defined       = specialFields.udb;

        /* SIP */
        fldValue =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                entryIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_26_0);
        fdbEntryInfoPtr->sip = fldValue;

        fldValue =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                entryIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_30_27);
        fdbEntryInfoPtr->sip |= fldValue << 27;

        fldValue =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                entryIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_31);
        fdbEntryInfoPtr->sip |= fldValue << 31;
    }
    else
    if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E ||
       fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E ||
       fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E)
    {
        SIP6_GET_VALUE(ucRoute.vrf_id                                 ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID                             );
        SIP6_GET_VALUE(ucRoute.dec_ttl_or_hop_count                   ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT               );
        SIP6_GET_VALUE(ucRoute.bypass_ttl_options_or_hop_extension    ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION);
        SIP6_GET_VALUE(ucRoute.counter_set_index                      ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                  );
        SIP6_GET_VALUE(ucRoute.icmp_redirect_excep_mirror_en          ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN      );
        SIP6_GET_VALUE(ucRoute.mtu_index                              ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                          );
        SIP6_GET_VALUE(ucRoute.use_vidx                               ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX                           );
        SIP6_GET_VALUE(ucRoute.evidx                                  ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX                              );
        SIP6_GET_VALUE(ucRoute.trg_is_trunk                           ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK                       );
        SIP6_GET_VALUE(ucRoute.trg_eport                              ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT                          );
        SIP6_GET_VALUE(ucRoute.trg_trunk_id                           ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID                       );
        SIP6_GET_VALUE(ucRoute.trg_dev                                ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV                            );
        SIP6_GET_VALUE(ucRoute.ecmp_pointer                           ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_POINTER                       );
        SIP6_GET_VALUE(ucRoute.next_hop_evlan                         ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                     );
        SIP6_GET_VALUE(ucRoute.start_of_tunnel                        ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                    );
        SIP6_GET_VALUE(ucRoute.arp_ptr                                ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR                            );
        SIP6_GET_VALUE(ucRoute.tunnel_type                            ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                        );
        SIP6_GET_VALUE(ucRoute.tunnel_ptr                             ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                         );
        SIP6_GET_VALUE(ucRoute.ts_is_nat                              ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                          );
        SIP6_GET_VALUE(ucRoute.route_type                             ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                         );
        SIP6_GET_VALUE(ucRoute.pointer_type                           ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_POINTER_TYPE                       );

        if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E)
        {
            SIP6_GET_VALUE(ucRoute.ipv4_dip                           ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                           );
        }
        else
        if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E)
        {
            SIP6_GET_VALUE(ucRoute.fcoe_d_id                          ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                          );
        }
        else /*SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E*/
        {
            SIP6_GET_VALUE(ucRoute.ipv6_dip_127_106                   ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                   );
            SIP6_GET_VALUE(ucRoute.ipv6_scope_check                   ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                   );
            SIP6_GET_VALUE(ucRoute.ipv6_dst_site_id                   ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                   );
        }
    }
    else
    if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E)
    {
        SIP6_GET_VALUE(ucRoute.ipv6_dip_31_0   ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0  );
        SIP6_GET_VALUE(ucRoute.ipv6_dip_63_32  ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32 );
        SIP6_GET_VALUE(ucRoute.ipv6_dip_95_64  ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64 );
        SIP6_GET_VALUE(ucRoute.ipv6_dip_105_96 ,SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96);
    }
}


/**
* @internal sfdbChtBankCounterActionAutoCalc function
* @endinternal
*
* @brief   SIP5 : auto calculate the increment/decrement/none action on the FDB bank
*         counters. as difference between 'old FDB entry' and the new FDB entry.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] oldIndex                 - index of old entry
* @param[in] oldEntryPtr              - pointer to the 'old' FDB entry (pointer to the FDB before
*                                      apply the FDB update)
* @param[in] oldIndex                 - index of new entry
* @param[in] newEntryPtr              - pointer to the 'new' FDB entry (pointer to buffer that will
*                                      be copied later into the FDB to update the existing entry)
* @param[in] client                   - triggering client
*
* @param[out] counterActionPtr         - (pointer to) the bank counter action.
*                                       None.
*/
static void sfdbChtBankCounterActionAutoCalc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   oldIndex,
    IN GT_U32                   *oldEntryPtr,
    IN GT_U32                   newIndex,
    IN GT_U32                   *newEntryPtr,
    OUT SFDB_CHT_BANK_COUNTER_ACTION_ENT     *counterActionPtr,
    IN SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_ENT  client
)
{
    GT_BIT  spUnknown;
    GT_BIT  skip;
    GT_BIT  valid;
    GT_BIT  spUnknown_old;
    GT_BIT  skip_old;
    GT_BIT  valid_old;
    GT_BIT  entryConsideredValid_new;/* new entry considered valid */
    GT_BIT  entryConsideredValid_old;/* old entry considered valid */

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        valid_old =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,oldEntryPtr,
                oldIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID);
        skip_old =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,oldEntryPtr,
                oldIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP);
        spUnknown_old =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,oldEntryPtr,
                oldIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);
        valid =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,newEntryPtr,
                newIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID);
        skip =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,newEntryPtr,
                newIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP);
        spUnknown =
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,newEntryPtr,
                newIndex,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);
    }
    else
    {
        valid_old =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,oldEntryPtr,
                oldIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_VALID);
        skip_old =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,oldEntryPtr,
                oldIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SKIP);
        spUnknown_old =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,oldEntryPtr,
                oldIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);

        valid =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,newEntryPtr,
                newIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_VALID);
        skip =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,newEntryPtr,
                newIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SKIP);
        spUnknown =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,newEntryPtr,
                newIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);
    }

    if(devObjPtr->multiHashEnable == 0)/* skip bit is NOT ignored */
    {
        entryConsideredValid_new = valid     && (skip == 0)       &&   (spUnknown == 0);
        entryConsideredValid_old = valid_old && (skip_old == 0)   &&   (spUnknown_old == 0);
    }
    else/* skip bit is ignored */
    {
        entryConsideredValid_new = valid     && /*(skip == 0)     &&*/ (spUnknown == 0);
        entryConsideredValid_old = valid_old && /*(skip_old == 0) &&*/ (spUnknown_old == 0);
    }


    if(entryConsideredValid_new)
    {
        /* although the validNew is 1 this entry should have been considered as deleted due to the errata */
        if((devObjPtr->fdbRouteUcDeletedEntryFlag==GT_TRUE) &&
           (client  == SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_AU_MSG_E))

        {
            /* new entry is NOT valid */
            *counterActionPtr = (entryConsideredValid_old) ?
                        SFDB_CHT_BANK_COUNTER_ACTION_DECREMENT_E :  /*old entry is valid*/
                        SFDB_CHT_BANK_COUNTER_ACTION_NONE_E;        /*old entry is NOT valid*/
        }
        else
        {
            /* new entry is valid */
            *counterActionPtr = (entryConsideredValid_old) ?
                        SFDB_CHT_BANK_COUNTER_ACTION_NONE_E :       /*old entry is valid*/
                        SFDB_CHT_BANK_COUNTER_ACTION_INCREMENT_E;   /*old entry is NOT valid*/
        }
    }
    else
    {
        /* new entry is NOT valid */
        *counterActionPtr = (entryConsideredValid_old) ?
                        SFDB_CHT_BANK_COUNTER_ACTION_DECREMENT_E :  /*old entry is valid*/
                        SFDB_CHT_BANK_COUNTER_ACTION_NONE_E;        /*old entry is NOT valid*/
    }


    return;
}

/**
* @internal sfdbChtNaMsgProcess function
* @endinternal
*
* @brief   Process New Address FDB update message.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] msgWordPtr               - pointer MAC update message.
* @param[in] entryIndexPtr            - pointer to entry index.
*/
static GT_STATUS sfdbChtNaMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr,
    OUT GT_U32               * entryIndexPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtNaMsgProcess);

    GT_U32 fdbIndex;                         /* FDB entry index */
    GT_STATUS status = GT_FAIL;                       /* function return status */
    GT_U32 causBitMap = 0;                  /* Interrupt cause bitmap  */
    GT_U32 entryOffset;                     /* MAC entry offset */

    * entryIndexPtr = 0;

    status = parseAuMsgGetFdbEntryIndex(devObjPtr,msgWordPtr,&fdbIndex, &entryOffset);
    if (status != GT_FAIL)
    {
        /* Write NA to FDB table */
        sfdbChtWriteNaMsg(devObjPtr, msgWordPtr, fdbIndex,entryOffset);
        * entryIndexPtr = fdbIndex;
    }

    if (status == GT_FAIL)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Na Not Learned */
            causBitMap = SMEM_CHT_NA_NOT_LEARN_INT;
            __LOG(("Interrupt: AU_PROC_COMPLETED and NA_NOT_LEARN \n"));
        }
        else
        {
        /* Na Not Learned | Dropped */
        causBitMap = SMEM_CHT_NA_NOT_LEARN_INT | SMEM_CHT_NA_DROPPED_INT;
        __LOG(("Interrupt: AU_PROC_COMPLETED and NA_NOT_LEARN and NA_DROPPED \n"));
    }

    }
    else
    {
        /* A new source MAC Address received was retained */
        causBitMap = SMEM_CHT_NA_LEARN_INT;

        __LOG(("Interrupt: AU_PROC_COMPLETED and NA_LEARN \n"));
    }


    /* AU completed */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        causBitMap |= SMEM_LION3_AU_PROC_COMPLETED_INT ;
    }
    else
    {
        causBitMap |= SMEM_CHT_AU_PROC_COMPLETED_INT;
    }

    /* A new source MAC Address received was retained */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr),
                          SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr),
                          causBitMap,
                          SMEM_CHT_FDB_SUM_INT(devObjPtr));
    return status;
}

/**
* @internal sip6FdbSpecialMuxedFieldsGet function
* @endinternal
*
* @brief   Sip6 : Get Muxed fields from the FDB that depend on :
*         fdb_mac_entry_muxing_mode or fdb_ipv_4_6_mc_entry_muxing_mode
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbEntryPtr              - pointer to FDB entry.
* @param[in] fdbIndex                 - fdb index
*
* @param[out] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
static GT_VOID sip6FdbSpecialMuxedFieldsGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    DECLARE_FUNC_NAME(sip6FdbSpecialMuxedFieldsGet);

    GT_U32              srcId = 0;/*do not use SMAIN_NOT_VALID_CNS */
    GT_U32              udb = 0;/*do not use SMAIN_NOT_VALID_CNS */
    GT_U32              origVid1 = 0;/*do not use SMAIN_NOT_VALID_CNS */
    GT_U32              daAccessLevel = 0;/*do not use SMAIN_NOT_VALID_CNS */
    GT_U32              saAccessLevel = 0;/*do not use SMAIN_NOT_VALID_CNS */
    GT_U32              value;
    GT_U32              regValue;
    GT_U32              fdb_entry_type;
    /*
        0x0 = src_id_7; src_id_7; <Src ID> (7 bits) is written and read from the FDB
        0x1 = src_id_3_udb_4; src_id_3_udb_4; <Src ID> (3 bits) and UDB (4 bits) are written and read from the FDB
        0x2 = udb_7; udb_7; UDB(7 bits) are written and read from the FDB
    */
    GT_U32  fdb_ipv_4_6_mc_entry_muxing_mode;
    /*
         Defines the muxing mode in case of MAC entry
         0= <Tag 1 VID> (12 bits) is written and read from the FD
         1= <Src ID> (12 bits) is written and read from the FDB
         2= UDB[10:7],UDB[4:1], <Src ID>[0]> and <DA Access Level> are written and read from the FDB
         3= UDB[8:1], <Src ID>[2:0]> and DA Access Level are written and read from the FDB
    */
    GT_U32  fdb_mac_entry_muxing_mode;


    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    fdb_ipv_4_6_mc_entry_muxing_mode = SMEM_U32_GET_FIELD(regValue,30,2);
    fdb_mac_entry_muxing_mode        = SMEM_U32_GET_FIELD(regValue,28,2);


    fdb_entry_type =
        SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE);

    /* not muxed */
    value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_0);
    udb = value;

    if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_MAC_E)
    {
        /* the field is NOT muxed */
        value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL);
        saAccessLevel = value;

        switch(fdb_mac_entry_muxing_mode)
        {
            case 0:
                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0);
                origVid1 = value;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1);
                origVid1 |= value << 1;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7);
                origVid1 |= value << 7;
                break;
            case 1:
                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0);
                srcId = value;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1);
                srcId |= value << 1;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3);
                srcId |= value << 3;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7);
                srcId |= value << 7;
                break;
            case 2:
            case 3:
                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7);
                udb |= value << 7;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1);
                udb |= value << 1;

                srcId = 0;
                if(fdb_mac_entry_muxing_mode == 2)
                {
                    value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5);
                    udb |= value << 5;
                }
                else
                {
                    value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1);
                    srcId = value << 1;
                }

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0);
                srcId |= value;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL);

                daAccessLevel = value;
                break;
            default:
                return;/*can't get here on 2 bits field*/
        }
    }
    else
    if(fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E ||
       fdb_entry_type == SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E)
    {
        /* the field is NOT muxed */
        value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL);
        daAccessLevel = value;

        switch(fdb_ipv_4_6_mc_entry_muxing_mode)
        {
            default:
            case 0:
                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0);
                srcId = value;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1);
                srcId |= value << 1;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3);
                srcId |= value << 3;
                break;
            case 1:
                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0);
                srcId = value;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1);
                srcId |= value << 1;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1);
                udb |= value << 1;

                break;

            case 2: /*udb_7; udb_7; UDB(7 bits) are written and read from the FDB */
                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1);
                udb |= value << 1;

                value = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5);
                udb |= value << 5;

        }
    }
    else
    {
        __LOG(("Warning : case not implemented \n"));
        /* not supported yet */
        return ;
    }

    specialFieldsPtr->srcId = srcId;
    specialFieldsPtr->udb = udb;
    specialFieldsPtr->origVid1 = origVid1;
    specialFieldsPtr->daAccessLevel = daAccessLevel;
    specialFieldsPtr->saAccessLevel = saAccessLevel;

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(specialFieldsPtr->srcId);
        __LOG_PARAM(specialFieldsPtr->udb);
        __LOG_PARAM(specialFieldsPtr->origVid1);
        __LOG_PARAM(specialFieldsPtr->daAccessLevel);
        __LOG_PARAM(specialFieldsPtr->saAccessLevel);
    }
}
/**
* @internal sip6FdbSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Sip6 : Set Muxed fields to the FDB that depend on :
*         fdb_mac_entry_muxing_mode or fdb_ipv_4_6_mc_entry_muxing_mode
* @param[in] devObjPtr                - pointer to device object.
* @param[in] specialFieldsPtr         - (pointer to) the special fields value from the entry
* @param[in] fdbIndex                 - fdb index
* @param[in] fdb_entry_type           - FDB entry type (supported only SNET_CHEETAH_FDB_ENTRY_MAC_E)
* @param[in] fdbEntryPtr              - pointer to FDB entry.
*
* @param[out] fdbEntryPtr              - pointer to FDB entry.
*
* @note function implemented only for 'Auto learn' (SNET_CHEETAH_FDB_ENTRY_MAC_E)
*       because not needed more than that.
*
*/
GT_VOID sip6FdbSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                fdb_entry_type,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    GT_U32              srcId;
    GT_U32              udb;
    GT_U32              origVid1;
    GT_U32              daAccessLevel;
    GT_U32              saAccessLevel;
    GT_U32              regValue;
    /*
        0x0 = src_id_7; src_id_7; <Src ID> (7 bits) is written and read from the FDB
        0x1 = src_id_3_udb_4; src_id_3_udb_4; <Src ID> (3 bits) and UDB (4 bits) are written and read from the FDB
        0x2 = udb_7; udb_7; UDB(7 bits) are written and read from the FDB
    */
    /*GT_U32  fdb_ipv_4_6_mc_entry_muxing_mode;*/
    /*
         Defines the muxing mode in case of MAC entry
         0= <Tag 1 VID> (12 bits) is written and read from the FD
         1= <Src ID> (12 bits) is written and read from the FDB
         2= UDB[10:7],UDB[4:1], <Src ID>[0]> and <DA Access Level> are written and read from the FDB
         3= UDB[8:1], <Src ID>[2:0]> and DA Access Level are written and read from the FDB
    */
    GT_U32  fdb_mac_entry_muxing_mode;

    if(fdb_entry_type != SNET_CHEETAH_FDB_ENTRY_MAC_E)
    {
        skernelFatalError("sip6FdbSpecialMuxedFieldsSet : function implemented only for 'Auto learn' (SNET_CHEETAH_FDB_ENTRY_MAC_E) \n");
        return;
    }

    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    /*fdb_ipv_4_6_mc_entry_muxing_mode = SMEM_U32_GET_FIELD(regValue,30,2);*/
    fdb_mac_entry_muxing_mode        = SMEM_U32_GET_FIELD(regValue,28,2);

    SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
        fdbIndex,
        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE,
        fdb_entry_type);

    srcId         = specialFieldsPtr->srcId;
    udb           = specialFieldsPtr->udb;
    origVid1      = specialFieldsPtr->origVid1;
    daAccessLevel = specialFieldsPtr->daAccessLevel;
    saAccessLevel = specialFieldsPtr->saAccessLevel;

    /* not muxed */
    SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
            fdbIndex,
        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_0,
        udb);

    {
        /* the field is NOT muxed */
        SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL,
            saAccessLevel);

        switch(fdb_mac_entry_muxing_mode)
        {
            case 0:
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0,
                    origVid1);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1,
                    origVid1 >> 1);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7,
                    origVid1 >> 7);
                break;
            case 1:
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0,
                    srcId);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1,
                    srcId >> 1);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3,
                    srcId >> 3);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7,
                    srcId >> 7);
                break;
            case 2:
            case 3:
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7,
                    udb >> 7);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1,
                    udb >> 1);

                srcId = 0;
                if(fdb_mac_entry_muxing_mode == 2)
                {
                    SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5,
                        udb >> 5);
                }
                else
                {
                    SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1,
                        srcId >> 1);
                }

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0,
                    srcId);

                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL,
                    daAccessLevel);
                break;
            default:
                return;/*can't get here on 2 bits field*/
        }
    }
}

/**
* @internal sip6AuMsgSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Sip6 : Set Muxed fields to the AU MSG that depend on :
*         fdb_mac_entry_muxing_mode or fdb_ipv_4_6_mc_entry_muxing_mode
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
*                                      fdbIndex     - fdb index
*
* @param[out] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sip6AuMsgSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    OUT GT_U32               * macUpdMsgPtr,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType,
    IN SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    GT_U32              srcId;
    GT_U32              udb;
    GT_U32              origVid1;
    GT_U32              daAccessLevel;
    GT_U32              saAccessLevel;
    GT_U32              regValue;
    /*
        0x0 = src_id_7; src_id_7; <Src ID> (7 bits) is written and read from the FDB
        0x1 = src_id_3_udb_4; src_id_3_udb_4; <Src ID> (3 bits) and UDB (4 bits) are written and read from the FDB
        0x2 = udb_7; udb_7; UDB(7 bits) are written and read from the FDB
    */
    GT_U32  fdb_ipv_4_6_mc_entry_muxing_mode;
    /*
         Defines the muxing mode in case of MAC entry
         0= <Tag 1 VID> (12 bits) is written and read from the FD
         1= <Src ID> (12 bits) is written and read from the FDB
         2= UDB[10:7],UDB[4:1], <Src ID>[0]> and <DA Access Level> are written and read from the FDB
         3= UDB[8:1], <Src ID>[2:0]> and DA Access Level are written and read from the FDB
    */
    GT_U32  fdb_mac_entry_muxing_mode;


    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    fdb_ipv_4_6_mc_entry_muxing_mode = SMEM_U32_GET_FIELD(regValue,30,2);
    fdb_mac_entry_muxing_mode        = SMEM_U32_GET_FIELD(regValue,28,2);

    srcId = specialFieldsPtr->srcId;
    udb = specialFieldsPtr->udb;
    origVid1 = specialFieldsPtr->origVid1;
    daAccessLevel = specialFieldsPtr->daAccessLevel;
    saAccessLevel = specialFieldsPtr->saAccessLevel;

    /* not muxed */
    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_0,
        specialFieldsPtr->udb);

    if(macEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E)/* MAC */
    {
        /* the field is NOT muxed */
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL,
            saAccessLevel);

        switch(fdb_mac_entry_muxing_mode)
        {
            case 0:
                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_0,
                    origVid1);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_6_1,
                    origVid1 >> 1);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_ORIG_VID1_11_7,
                    origVid1 >> 7);
                break;
            case 1:
                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_0,
                    srcId);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_2_1,
                    srcId >> 1);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_6_3,
                    srcId >> 3);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_11_7 ,
                    srcId >> 7);
                break;
            case 2:
            case 3:
                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_10_7,
                    udb >> 7);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_4_1,
                    udb >> 1);

                if(fdb_mac_entry_muxing_mode == 2)
                {
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_USER_DEFINED_6_5,
                        udb >> 5);
                }
                else
                {
                    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_2_1,
                        srcId >> 1);
                }

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_SOURCE_ID_0,
                    srcId);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL,
                    daAccessLevel);
                break;
            default:
                return;/*can't get here on 2 bits field*/
        }
    }
    else
    if(macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E ||
       macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E)/*IPMC*/
    {
        /* the field is NOT muxed */
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL,
            daAccessLevel);

        switch(fdb_ipv_4_6_mc_entry_muxing_mode)
        {
            default:
            case 0:
                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_0,
                    srcId);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_2_1,
                    srcId >> 1);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_6_3,
                    srcId >> 3);
                break;
            case 1:
                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_0,
                    srcId);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_2_1,
                    srcId >> 1);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_4_1,
                    udb >> 1);

                break;

            case 2: /*udb_7; udb_7; UDB(7 bits) are written and read from the FDB */
                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_4_1,
                    udb >> 1);

                SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_6_5,
                    udb >> 5);
        }
    }
    else
    {
        /* not supported yet */
        return ;
    }

}


/**
* @internal sfdbLion3FdbSpecialMuxedFieldsGet function
* @endinternal
*
* @brief   Get Muxed fields from the FDB that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
* @param[in] fdbIndex                 - fdb index
*
* @param[out] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sfdbLion3FdbSpecialMuxedFieldsGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    GT_U32              srcId;
    GT_U32              udb;
    GT_U32              origVid1;
    GT_U32              daAccessLevel;
    GT_U32              saAccessLevel;
    GT_U32              value;
    GT_U32              srcIdLengthInFDB;/* 0- The SrcID filed in FDB is 12b
                                            1- The SrcID field in FDB table is 9b.
                                            SrcID[11:9] are used for extending the user defined bits */
    GT_U32              vid1AssignmentMode;/* 0 - <OrigVID1> is not written in the FDB and is not read from the FDB.
                                                    <SrcID>[8:6] can be used for src-id filtering and
                                                    <SA Security Level> and <DA Security Level> are written/read from the FDB.
                                              1- <OrigVID1> is written in the FDB and read from the FDB as described in
                                                    Section N:1 Mac Based VLAN .
                                                    <SrcID>[8:6], <SA Security Level> and <DA Security Level>
                                                    are read as 0 from the FDB entry.*/
    GT_U32              regValue;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        sip6FdbSpecialMuxedFieldsGet(devObjPtr,fdbEntryPtr,fdbIndex,specialFieldsPtr);
        return;
    }

    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    srcIdLengthInFDB   = SMEM_U32_GET_FIELD(regValue,10,1);
    vid1AssignmentMode = SMEM_U32_GET_FIELD(regValue,27,1);

    value =
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0);
    srcId = value;

    if(vid1AssignmentMode == 0)
    {
        value =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6);
        SMEM_U32_SET_FIELD(srcId,6,3,value);
    }

    value =
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_USER_DEFINED);
    udb = value;

    if(srcIdLengthInFDB == 0)
    {
        value =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9);
        SMEM_U32_SET_FIELD(srcId,9,3,value);

        udb &= 0x1F;/* keep only bits 0..4 */
    }

    if(vid1AssignmentMode)
    {
        daAccessLevel = SMAIN_NOT_VALID_CNS;
        saAccessLevel = SMAIN_NOT_VALID_CNS;

        value =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_ORIG_VID1);
        origVid1 = value;
    }
    else
    {
        origVid1 = SMAIN_NOT_VALID_CNS;

        value =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL);
        daAccessLevel = value;

        value =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL);
        saAccessLevel = value;
    }

    specialFieldsPtr->srcId = srcId;
    specialFieldsPtr->udb = udb;
    specialFieldsPtr->origVid1 = origVid1;
    specialFieldsPtr->daAccessLevel = daAccessLevel;
    specialFieldsPtr->saAccessLevel = saAccessLevel;

    return;
}

/**
* @internal sfdbLion3FdbSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Set Muxed fields from the FDB that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
* @param[in] fdbIndex                 - fdb index
* @param[in] macEntryType             - entry type
* @param[in] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sfdbLion3FdbSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * fdbEntryPtr,
    IN GT_U32                fdbIndex,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType,
    IN SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    GT_U32              value;
    GT_U32              srcIdLengthInFDB;/* 0- The SrcID filed in FDB is 12b
                                            1- The SrcID field in FDB table is 9b.
                                            SrcID[11:9] are used for extending the user defined bits */
    GT_U32              vid1AssignmentMode;/* 0 - <OrigVID1> is not written in the FDB and is not read from the FDB.
                                                    <SrcID>[8:6] can be used for src-id filtering and
                                                    <SA Security Level> and <DA Security Level> are written/read from the FDB.
                                              1- <OrigVID1> is written in the FDB and read from the FDB as described in
                                                    Section N:1 Mac Based VLAN .
                                                    <SrcID>[8:6], <SA Security Level> and <DA Security Level>
                                                    are read as 0 from the FDB entry.*/
    GT_U32              regValue;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("sfdbLion3FdbSpecialMuxedFieldsSet : function not for sip6 devices \n");
        return;
    }

    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    srcIdLengthInFDB   = SMEM_U32_GET_FIELD(regValue,10,1);
    vid1AssignmentMode = SMEM_U32_GET_FIELD(regValue,27,1);

    if(macEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E) /* valid when MACEntryType = "MAC" */
    {
        value = specialFieldsPtr->srcId;
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0,
            value);

        if(vid1AssignmentMode == 0)
        {
            value = specialFieldsPtr->srcId >> 6;
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6,
                value);
        }
    }

    /* NOTE: setting the 8 bits of UDB must come before setting of SOURCE_ID_11_9
        to allow SOURCE_ID_11_9 to override the 3 bits ! */
    value = specialFieldsPtr->udb;
    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_USER_DEFINED,
        value);

    if(macEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E) /* valid when MACEntryType = "MAC" */
    {
        if(srcIdLengthInFDB == 0)
        {
            value = specialFieldsPtr->srcId >> 9;
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9,
                value);
        }
    }

    if(vid1AssignmentMode)
    {
        value = specialFieldsPtr->origVid1;
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_ORIG_VID1,
            value);
    }
    else
    {
        value = specialFieldsPtr->daAccessLevel;
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL,
            value);

        value = specialFieldsPtr->saAccessLevel;
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL,
            value);
    }

    return;
}

/**
* @internal sfdbLion3FdbAuMsgSpecialMuxedFieldsGet function
* @endinternal
*
* @brief   Get Muxed fields from the FDB Au Msg that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
* @param[in] auMsgEntryPtr            - pointer to AU MSG message.
*
* @param[out] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
static GT_VOID sfdbLion3FdbAuMsgSpecialMuxedFieldsGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * auMsgEntryPtr,
    OUT SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    GT_U32              srcId;
    GT_U32              udb;
    GT_U32              origVid1;
    GT_U32              daAccessLevel;
    GT_U32              saAccessLevel;
    GT_U32              value;
    GT_U32              srcIdLengthInFDB;/* 0- The SrcID filed in FDB is 12b
                                            1- The SrcID field in FDB table is 9b.
                                            SrcID[11:9] are used for extending the user defined bits */
    GT_U32              vid1AssignmentMode;/* 0 - <OrigVID1> is not written in the FDB and is not read from the FDB.
                                                    <SrcID>[8:6] can be used for src-id filtering and
                                                    <SA Security Level> and <DA Security Level> are written/read from the FDB.
                                              1- <OrigVID1> is written in the FDB and read from the FDB as described in
                                                    Section N:1 Mac Based VLAN .
                                                    <SrcID>[8:6], <SA Security Level> and <DA Security Level>
                                                    are read as 0 from the FDB entry.*/
    GT_U32              regValue;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("sfdbLion3FdbAuMsgSpecialMuxedFieldsGet : function not for sip6 devices \n");
        return;
    }

    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    srcIdLengthInFDB   = SMEM_U32_GET_FIELD(regValue,10,1);
    vid1AssignmentMode = SMEM_U32_GET_FIELD(regValue,27,1);

    value =
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_5_0);
    srcId = value;

    if(vid1AssignmentMode == 0)
    {
        value =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_8_6);
        SMEM_U32_SET_FIELD(srcId,6,3,value);
    }

    value =
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED);

    udb = value;

    if(srcIdLengthInFDB == 0)
    {
        value =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_9);
        SMEM_U32_SET_FIELD(srcId,9,3,value);

        udb &= 0x1F;/* keep only bits 0..4 */
    }

    if(vid1AssignmentMode)
    {
        daAccessLevel = SMAIN_NOT_VALID_CNS;
        saAccessLevel = SMAIN_NOT_VALID_CNS;

        value =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1);
        origVid1 = value;
    }
    else
    {
        origVid1 = SMAIN_NOT_VALID_CNS;

        value =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL);
        daAccessLevel = value;

        value =
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL);
        saAccessLevel = value;
    }

    specialFieldsPtr->srcId = srcId;
    specialFieldsPtr->udb = udb;
    specialFieldsPtr->origVid1 = origVid1;
    specialFieldsPtr->daAccessLevel = daAccessLevel;
    specialFieldsPtr->saAccessLevel = saAccessLevel;

    return;
}

/**
* @internal sfdbLion3FdbAuMsgSpecialMuxedFieldsSet function
* @endinternal
*
* @brief   Set Muxed fields from the FDB Au Msg that depend on :
*         1. vid1_assignment_mode
*         2. src_id_length_in_fdb
* @param[in] devObjPtr                - pointer to device object.
* @param[in] auMsgEntryPtr            - pointer to FDB AU MSG message.
* @param[in] macEntryType             - entry type
* @param[in] specialFieldsPtr         - (pointer to) the special fields value from the entry
*/
GT_VOID sfdbLion3FdbAuMsgSpecialMuxedFieldsSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * auMsgEntryPtr,
    IN SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType,
    IN SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC *specialFieldsPtr
)
{
    GT_U32              value;
    GT_U32              srcIdLengthInFDB;/* 0- The SrcID filed in FDB is 12b
                                            1- The SrcID field in FDB table is 9b.
                                            SrcID[11:9] are used for extending the user defined bits */
    GT_U32              vid1AssignmentMode;/* 0 - <OrigVID1> is not written in the FDB and is not read from the FDB.
                                                    <SrcID>[8:6] can be used for src-id filtering and
                                                    <SA Security Level> and <DA Security Level> are written/read from the FDB.
                                              1- <OrigVID1> is written in the FDB and read from the FDB as described in
                                                    Section N:1 Mac Based VLAN .
                                                    <SrcID>[8:6], <SA Security Level> and <DA Security Level>
                                                    are read as 0 from the FDB entry.*/
    GT_U32              regValue;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("sfdbLion3FdbAuMsgSpecialMuxedFieldsSet : function not for sip6 devices \n");
        return;
    }

    smemRegGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr) ,&regValue);

    srcIdLengthInFDB   = SMEM_U32_GET_FIELD(regValue,10,1);
    vid1AssignmentMode = SMEM_U32_GET_FIELD(regValue,27,1);

    if(macEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E) /* valid when MACEntryType = "MAC" */
    {
        value = specialFieldsPtr->srcId;
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_5_0,
            value);

        if(vid1AssignmentMode == 0)
        {
            value = specialFieldsPtr->srcId >> 6;
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_8_6,
                value);
        }
    }

    /* NOTE: setting the 8 bits of UDB must come before setting of SOURCE_ID_11_9
        to allow SOURCE_ID_11_9 to override the 3 bits ! */
    value = specialFieldsPtr->udb;
    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED,
        value);

    if(macEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E) /* valid when MACEntryType = "MAC" */
    {
        if(srcIdLengthInFDB == 0)
        {
            value = specialFieldsPtr->srcId >> 9;
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_9,
                value);
        }
    }

    if(vid1AssignmentMode)
    {
        value = specialFieldsPtr->origVid1;
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1,
            value);
    }
    else
    {
        if(specialFieldsPtr->daAccessLevel != SMAIN_NOT_VALID_CNS)
        {
            value = specialFieldsPtr->daAccessLevel;
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL,
                value);
        }

        if(specialFieldsPtr->saAccessLevel != SMAIN_NOT_VALID_CNS)
        {
            value = specialFieldsPtr->saAccessLevel;
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,auMsgEntryPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL,
                value);
        }
    }

    return;
}

/**
* @internal lion3AuMsgConvertToFdbEntry function
* @endinternal
*
* @brief   Lion3 : Create FDB entry from AU message
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbIndex                 - index in the FDB
* @param[in] msgWordPtr               - MAC update message pointer
*
* @param[out] macEntry                 - FDB entry pointer
*/
static GT_STATUS lion3AuMsgConvertToFdbEntry
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 fdbIndex,
    IN GT_U32                *msgWordPtr,
    OUT GT_U32               *macEntry
)
{
    DECLARE_FUNC_NAME(lion3AuMsgConvertToFdbEntry);

    GT_U32 isMulticastMac;/* bit 40 of the mac indicated multicast bit */
    GT_U8  macAddr[6];
    GT_U32 macAddrWords[2];
    GT_U32 fid = 0;
    GT_U32 fldValue;                    /* entry field value */
    GT_U32 useVidx;
    GT_U32 isTrunk;
    GT_U32 isTunnelStart;

    SNET_CHEETAH_FDB_ENTRY_ENT                macEntryType;
    SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC specialFields;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("lion3AuMsgConvertToFdbEntry : function not for sip6 devices \n");
        return GT_FAIL;
    }

    /*************** common fields for all mac entries ********************/

    /* Entry Type */
    macEntryType =
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE);

    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE,
        macEntryType);

    /* Valid */
    fldValue =SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                                                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_VALID);

    if((fldValue==0) && /* delete entry */
       (devObjPtr->errata.fdbRouteUcDeleteByMsg) &&
       ((macEntryType==SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E)||
        (macEntryType==SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E)))
    {
        /* [JIRA]:[MT-231] [FE-2293984]
           CPU NA message for deleting an entry does not work for UC route entries */
        fldValue = 1;
        devObjPtr->fdbRouteUcDeletedEntryFlag = GT_TRUE;
    }
    else
    {
        devObjPtr->fdbRouteUcDeletedEntryFlag = GT_FALSE;
    }


    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_VALID,
        fldValue);

    /* Skip */
    fldValue =
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SKIP);
    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_SKIP,
        fldValue);

    /* Age */
    fldValue =
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_AGE);
    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_AGE,
        fldValue);

    /********************* mac and mc fields: common **********************************/
    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:

            memset(&specialFields,0,sizeof(SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC));

            /* srcId          */
            /* udb            */
            /* origVid1       */
            /* daAccessLevel  */
            /* saAccessLevel  */
            sfdbLion3FdbAuMsgSpecialMuxedFieldsGet(devObjPtr,msgWordPtr,
                &specialFields);
            sfdbLion3FdbSpecialMuxedFieldsSet(devObjPtr,macEntry,fdbIndex,macEntryType,
                &specialFields);


            /* Vlan ID */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FID);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_FID,
                fldValue);
            fid = fldValue;

            /* Static */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_STATIC,
                fldValue);

            /* DA_CMD */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DA_CMD);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DA_CMD,
                fldValue);

            /* Mirror To Analyzer Port */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER,
                fldValue);


            /*DA Lookup Ingress Mirror to Analyzer Enable*/
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER,
                fldValue);

            /* DA QoS Profile Index */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX,
                fldValue);

            /* SA QoS Profile Index */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX,
                fldValue);

            /* SPUnknown */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN,
                fldValue);

            /* Enable application specific CPU code for this entry */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE,
                fldValue);

            /* DA rote */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DA_ROUTE,
                fldValue);

            break;

    /********************* routing fields: common **********************************/
        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:

            /* vrf_id */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID,
                fldValue);


            /* dec ttl */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT,
                fldValue);

            /* bypass ttl */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION,
                fldValue);

            /* Ingress Mirror to Analyzer Index */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX,
                fldValue);

            /* Qos profile marking en */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN,
                fldValue);

            /* Qos profile index */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX,
                fldValue);

            /* Qos profile precedence */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE,
                fldValue);

            /* Modify UP */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_UP);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MODIFY_UP,
                fldValue);

            /* Modify DSCP */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP,
                fldValue);

            /* Counter Set index */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX,
                fldValue);

            /* Arp bc trap mirror en */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN,
                fldValue);

            /* Dip access level */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL,
                fldValue);

            /* Ecmp */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEPTION_MIRROR,
                fldValue);

            /* Mtu Index */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX,
                fldValue);

            /* Use Vidx*/
            useVidx =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX,
                useVidx);

            if(0 == useVidx)
            {
                /* Is trunk*/
                isTrunk =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK,
                    fldValue);

                if(isTrunk)
                {
                    /* Trunk num */
                    fldValue =
                        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM);
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID,
                        fldValue);
                }
                else /* not trunk */
                {
                    /* Eport num */
                    fldValue =
                        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM);
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT,
                        fldValue);

                    /* Target Device */
                    fldValue =
                        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE);
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV,
                        fldValue);
                }
            }
            else /* useVidx true */
            {
                /* Evidx */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX,
                    fldValue);
            }

            /* Next Hop Evlan */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN,
                fldValue);

            /* Tunnel Start */
            isTunnelStart =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_START);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL,
                isTunnelStart);

            if(isTunnelStart)
            {
                /* Tunnel Type */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE,
                    fldValue);

                /* Tunnel Ptr */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR,
                    fldValue);
            }
            else /* not tunnel */
            {
                /* Arp Ptr */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR,
                    fldValue);
            }

            break;
        default:
            break;
    }


    /************** specific fields (depends on mac entry type) *********************/

    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:

            /* 48-bit MAC address */
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,msgWordPtr,
                macAddrWords);

            /* set the MAC ADDR words */
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_MAC_ADDR_SET(devObjPtr,
                macEntry,
                fdbIndex,
                macAddrWords);

            macAddr[5] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 0) ,8);
            macAddr[4] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 1) ,8);
            macAddr[3] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 2) ,8);
            macAddr[2] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 3) ,8);
            macAddr[1] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 4) ,8);
            macAddr[0] = (GT_U8)snetFieldValueGet(macAddrWords,(8 * 5) ,8);

            isMulticastMac = (macAddr[0] & 1);/*bit 40 of the MAC address */

            /* SA_CMD */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SA_CMD);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SA_CMD,
                fldValue);

            /* Multiple */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_MULTIPLE,
                fldValue);

            if(fldValue || /* multiple*/
               isMulticastMac)/* multicast mac address
                                   last bit in the most significant byte of mac
                                   address (network order)*/
            {
                /* use vidx */

                /* vidx */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_VIDX);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_VIDX,
                    fldValue);
            }
            else
            {
                /* Trunk */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_TRUNK,
                    fldValue);
                if(fldValue)
                {
                    /* TrunkNum */
                    fldValue =
                        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM);
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_TRUNK_NUM,
                        fldValue);
                }
                else
                {
                    /* PortNum */
                    fldValue =
                        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM);
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_EPORT_NUM,
                        fldValue);
                }

                /* DevID */
                fldValue =
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DEV_ID);
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_DEV_ID,
                    fldValue);
            }

                __LOG(("NA processed into FDB macAddr[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] vid[0x%3.3x] index [%d]\n"
                          ,macAddr[0]
                          ,macAddr[1]
                          ,macAddr[2]
                          ,macAddr[3]
                          ,macAddr[4]
                          ,macAddr[5]
                          ,fid
                          ,fdbIndex
                          ));
            break;


        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:
            /* DIP */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DIP);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DIP,
                fldValue);

            /* SIP */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SIP);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SIP,
                fldValue);

            /* VIDX */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_VIDX);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_VIDX,
                fldValue);
        break;



        /****************** routing specific fields ********************************/
        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:

            /* ipv4 dip */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP,
                fldValue);
            break;

        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:

            /* fcoe d_id */
            fldValue =
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID);
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID,
                fldValue);
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:
            __LOG(("ipv6 'not supported' (the device not support 'by message' the ipv6 entries)"));
            return GT_FAIL;
        default:
            skernelFatalError("wrong entry type given\n");
            break;
    }

    return GT_OK;

}


/**
* @internal lion3FdbChtWriteNaMsg function
* @endinternal
*
* @brief   Process New Address FDB update message and write to fdb table.
*
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
* @param[in] fdbIndex                 - fdb index
* @param[in] entryOffset              - FDB offset within the 'bucket'
*/
static GT_VOID lion3FdbChtWriteNaMsg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr,
    IN GT_U32                fdbIndex,
    IN GT_U32                entryOffset
)
{
    DECLARE_FUNC_NAME(lion3FdbChtWriteNaMsg);

    GT_U32 macEntry[SMEM_CHT_MAC_TABLE_WORDS];                 /* MAC entry */
    GT_U32  *preChangedEntryPtr;/* pointer to FDB entry before apply the new info */
    GT_U32  address;/* address of the FDB entry */
    SFDB_CHT_BANK_COUNTER_ACTION_ENT counterAction;
    GT_U32 macEntrySize;

    macEntrySize = SMEM_CHT_MAC_TABLE_WORDS_MAC(devObjPtr);

    memset(&macEntry, 0, sizeof(macEntry));

    __LOG_PARAM(fdbIndex);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        sip6AuMsgConvertToFdbEntry(devObjPtr, fdbIndex, msgWordPtr, macEntry);
        preChangedEntryPtr = SMEM_SIP6_HIDDEN_FDB_PTR(devObjPtr, fdbIndex);
        address = SMAIN_NOT_VALID_CNS;
    }
    else
    {
        address = SMEM_CHT_MAC_TBL_MEM(devObjPtr, fdbIndex);
        __LOG_PARAM(address);
        preChangedEntryPtr = smemMemGet(devObjPtr,address);

        lion3AuMsgConvertToFdbEntry(devObjPtr, fdbIndex, msgWordPtr, macEntry);
    }

    /* auto calc the counterAction by comparing the 'old' entry with the 'new' entry*/
    sfdbChtBankCounterActionAutoCalc(devObjPtr,
        fdbIndex, /*the old index - same as new index because we update the entry */
        preChangedEntryPtr,/* the old entry */
        fdbIndex, /* the new index - same as old index because we update the entry */
        macEntry,          /* the new entry */
        &counterAction,
        SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_AU_MSG_E);

    /*counter action mode*/
    sfdbChtBankCounterAction(devObjPtr, fdbIndex , counterAction,
        SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_AU_MSG_E);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* Write MAC entry into fdb table */
        smemGenericHiddenMemSet(devObjPtr,SMEM_GENERIC_HIDDEN_MEM_FDB_E ,fdbIndex, macEntry, macEntrySize);
    }
    else
    {
        /* Write MAC entry into fdb table */
        smemMemSet(devObjPtr, address, macEntry, macEntrySize);
    }

    /* update <NAEntryOffset> */
    smemRegFldSet(devObjPtr, SMEM_CHT_MSG_FROM_CPU_REG(devObjPtr), 2,5, entryOffset);

}

/**
* @internal sfdbChtWriteNaMsg function
* @endinternal
*
* @brief   Process New Address FDB update message and write to fdb table.
*
* @param[in] devObjPtr                - pointer to device object.
*                                      fdbMsgPtr   - pointer to FDB message.
* @param[in] fdbIndex                 - fdb index
* @param[in] entryOffset              - FDB offset within the 'bucket'
*/
static GT_VOID sfdbChtWriteNaMsg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr,
    IN GT_U32                fdbIndex,
    IN GT_U32                entryOffset
)
{
    GT_U32 fldValue;                    /* entry field value */
    GT_U32 fldEntryType;
    GT_U32 macEntry[SMEM_CHT_MAC_TABLE_WORDS];                 /* MAC entry */
    GT_U32  address;/* address of the FDB entry */

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        lion3FdbChtWriteNaMsg(devObjPtr,msgWordPtr,fdbIndex,entryOffset);
        return;
    }

    address = SMEM_CHT_MAC_TBL_MEM(devObjPtr, fdbIndex);

    memset(&macEntry, 0, sizeof(macEntry));

    /* Create MAC table X words entry */
    /* Valid */
    SMEM_U32_SET_FIELD(macEntry[0], 0, 1, 1);

    /* Skip */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 12, 1);
    SMEM_U32_SET_FIELD(macEntry[0], 1, 1, fldValue);

    /* Age */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 13, 1);
    SMEM_U32_SET_FIELD(macEntry[0], 2, 1, fldValue);

    /* Vlan ID */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 0, 12);
    SMEM_U32_SET_FIELD(macEntry[0], 5, 12, fldValue);

    /* Static */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 18, 1);
    SMEM_U32_SET_FIELD(macEntry[2], 25, 1, fldValue);

    /* DA_CMD */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 21, 3);
    SMEM_U32_SET_FIELD(macEntry[2], 27, 3, fldValue);

    /* Mirror To Analyzer Port */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 31, 1);
    SMEM_U32_SET_FIELD(macEntry[3], 9, 1, fldValue);

    /* DA QoS Profile Index */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 15, 3);
    SMEM_U32_SET_FIELD(macEntry[3], 6, 3, fldValue);

    /* SA QoS Profile Index */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 12, 3);
    SMEM_U32_SET_FIELD(macEntry[3], 3, 3, fldValue);

    /* SPUnknown */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 14, 1);
    SMEM_U32_SET_FIELD(macEntry[3], 2, 1, fldValue);

    /* Enable application specific CPU code for this entry */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 29, 1);
    SMEM_U32_SET_FIELD(macEntry[3], 10, 1, fldValue);

    /* DA rote */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 30, 1);
    SMEM_U32_SET_FIELD(macEntry[3], 1, 1, fldValue);

    /* Entry Type */
    fldEntryType = SMEM_U32_GET_FIELD(msgWordPtr[3], 19, 2);
    SMEM_U32_SET_FIELD(macEntry[0], 3, 2, fldEntryType);

    /* The MAC DA Access level for this entry */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[0], 1, 3);
    SMEM_U32_SET_FIELD(macEntry[3], 11, 3, fldValue);

    /* The MAC SA Access level for this entry */
    fldValue = SMEM_U32_GET_FIELD(msgWordPtr[0], 12, 3);
    SMEM_U32_SET_FIELD(macEntry[3], 14, 3, fldValue);

    if (fldEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E)
    {
        /* Bits 14:0 of the 48-bit MAC address */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[0], 16, 16);
        SMEM_U32_SET_FIELD(macEntry[0], 17, 15, fldValue);
        SMEM_U32_SET_FIELD(macEntry[1], 0, 1, fldValue >> 15);

        /* Bits 46:15 of the 48-bit MAC address */
        fldValue = msgWordPtr[1];
        SMEM_U32_SET_FIELD(macEntry[1], 1, 31, fldValue);
        SMEM_U32_SET_FIELD(macEntry[2], 0, 1, fldValue >> 31);

        /* SA_CMD */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 24, 3);
        SMEM_U32_SET_FIELD(macEntry[2], 30, 2, (fldValue & 0x3));
        SMEM_U32_SET_FIELD(macEntry[3], 0, 1, fldValue >> 2);

        /* Multiple */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 15, 1);
        SMEM_U32_SET_FIELD(macEntry[2], 26, 1, fldValue);

        if(fldValue || /* multiple*/
           SMEM_U32_GET_FIELD(msgWordPtr[1], 24, 1))/* multicast mac address
                                   last bit in the most significant byte of mac
                                   address (network order)*/
        {
            /* use vidx */

            /* vidx */
            fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 17, 12);
            SMEM_U32_SET_FIELD(macEntry[2], 13, 12, fldValue);
        }
        else
        {
            /* Trunk */
            fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 17, 1);
            SMEM_U32_SET_FIELD(macEntry[2], 13, 1, fldValue);
            /* PortNum/TrunkNum */
            fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 18, 7);
            SMEM_U32_SET_FIELD(macEntry[2], 14, 7, fldValue);
            /* User Defined */
            fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 25, 4);
            SMEM_U32_SET_FIELD(macEntry[2], 21, 4, fldValue);
        }

        /* DevID */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 7, 5);
        SMEM_U32_SET_FIELD(macEntry[2], 1, 5, fldValue);

        /* SrcID */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 2, 5);
        SMEM_U32_SET_FIELD(macEntry[2], 6, 5, fldValue);
    }
    else
    {
        /* DIP[15:0] */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[0], 16, 16);
        /* DIP[14:0] */
        SMEM_U32_SET_FIELD(macEntry[0], 17, 15, fldValue);
        /* DIP[15] */
        SMEM_U32_SET_FIELD(macEntry[1], 0, 1, fldValue >> 15);

        /* DIP[31:16] */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[1], 0, 16);
        /* DIP[31:16] */
        SMEM_U32_SET_FIELD(macEntry[1], 1, 16, fldValue);

        /* SIP[15:0] */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[1], 16, 16);
        /* SIP[14:0] */
        SMEM_U32_SET_FIELD(macEntry[1], 17, 15, fldValue);
        /* SIP[15] */
        SMEM_U32_SET_FIELD(macEntry[2], 0, 1, fldValue >> 15);

        /* SIP[27:16] */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 0, 12);
        /* SIP[27:16] */
        SMEM_U32_SET_FIELD(macEntry[2], 1, 12, fldValue);

        /* SIP[31:28] */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[3], 27, 4);
        /* SIP[28] */
        SMEM_U32_SET_FIELD(macEntry[2], 26, 1, fldValue);
        /* SIP[30:29] */
        SMEM_U32_SET_FIELD(macEntry[2], 30, 2, fldValue >> 1);
        /* SIP[31] */
        SMEM_U32_SET_FIELD(macEntry[3], 0, 1, fldValue >> 3);

        /* VIDX */
        fldValue = SMEM_U32_GET_FIELD(msgWordPtr[2], 17, 12);
        SMEM_U32_SET_FIELD(macEntry[2], 13, 12, fldValue);
    }

    /* Write MAC entry into fdb table */
    smemMemSet(devObjPtr, address, macEntry, 4);
}

/**
* @internal sfdbChtMacTableTriggerAction function
* @endinternal
*
* @brief   MAC Table Trigger Action
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - pointer table action data
*/
GT_VOID sfdbChtMacTableTriggerAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * tblActPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtMacTableTriggerAction);

    GT_U32 * tblActWordPtr;         /* 32 bit data pointer */
    GT_U32  *memPtr;/*pointer to memory*/
    CHT_AUQ_MEM  *  simAuqMem;              /* pointer to AU queue */
    GT_U32          auqSize; /* size of AUQ */
    GT_U32      regAddr;/*register address*/
    GT_U32      ActEn;/*Enables FDB Actions*/
    GT_U32      TriggerMode;
    GT_U32      ActionMode;
    GT_U32      fdbCfgRegAddr;/* FDB Global Configuration register address*/
    GT_U32      auqInMemory;  /* 1 - used PCI accessed memory, 0 - used on Chip queue */
    GT_U32 tileId = FDB_TILE_ID_GET(devObjPtr);

    tblActWordPtr = (GT_U32 *)tblActPtr;

    ActEn = SMEM_U32_GET_FIELD(tblActWordPtr[0], 0, 1);
    if(ActEn == 0)
    {
        /* action disabled by the application */
        return;
    }

    if(devObjPtr->fdbNumEntries == 0)
    {
        /* can be in sip6.30 when FDB used only by the DDE for HSR-PRP */
        __LOG(("FDB size is ZERO !!! no action supported \n"));
        return;
    }

    /* Trigger Mode */
    TriggerMode = SMEM_U32_GET_FIELD(tblActWordPtr[0], 2, 1);
    /* Action Mode */
    ActionMode  = SMEM_U32_GET_FIELD(tblActWordPtr[0], 3, 2);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        regAddr = SMEM_LION3_FDB_FDB_ACTION_GENERAL_REG(devObjPtr);
    }
    else
    {
        regAddr = SMEM_CHT_MAC_TBL_ACTION0_REG(devObjPtr);
    }

    fdbCfgRegAddr = SMEM_CHT_MAC_TBL_GLB_CONF_REG(devObjPtr);
    smemRegFldGet(devObjPtr, fdbCfgRegAddr, 20, 1, &auqInMemory);

    if ((devObjPtr->isPciCompatible) && (auqInMemory != 0))/* use DMA memory */
    {
        simAuqMem = SMEM_CHT_MAC_AUQ_MEM_GET(devObjPtr);
        auqSize = simAuqMem->auqBaseSize;
        /* don't start the action while the AUQ is full --> 'erraum' */
        if((simAuqMem->auqBaseValid == GT_FALSE || simAuqMem->auqOffset == auqSize)&&
           simAuqMem->auqShadowValid == GT_FALSE)
        {
            memPtr = smemMemGet(devObjPtr, regAddr);

            /* do some waiting so no storming of messages */
            SIM_OS_MAC(simOsSleep)(50);

            /* call function to send message to the 'smain' task again */
            smemChtActiveWriteFdbActionTrigger(devObjPtr,
                    regAddr,  /*address */
                    1,        /*memSize */
                    memPtr,   /*memPtr  */
                    0,        /*param   */
                    memPtr);  /*inMemPtr*/

            return;
        }
    }

    if (TriggerMode)
    {
        /* ActionMode */
        if (ActionMode == SFDB_CHEETAH_TRIGGER_TRANS_E)
        {
            sfdbChtTriggerTa(devObjPtr, tblActWordPtr,tileId);
        }
        else
        {
            sfdbChtDoAging(devObjPtr, tblActWordPtr,
                           ActionMode, devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb,
                           devObjPtr->fdbNumEntries - devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb,
                           tileId);
        }
    }

    /* Clear Aging Trigger */
    smemRegFldSet(devObjPtr, regAddr, 1, 1, 0);

    /* do interrupt after clearing the bit .
        JIRA: CPSS-5159 : BC2 simulation - sometimes after getting an interrupt of trigger end the next flush fails
    */
    __LOG(("Interrupt: AGE_VIA_TRIGGER_END \n"));

    /* Processing of an AU Message received by the device is completed */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr),
                          SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr),
                          SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                            SMEM_LION3_AGE_VIA_TRIGGER_END_INT :
                          SMEM_CHT_AGE_VIA_TRIGGER_END_INT,
                          SMEM_CHT_FDB_SUM_INT(devObjPtr));

}

/* open this flag manually to check the behavior of the aging daemon */
/*#define SIMULATION_CHECK_AGING_TIME*/


#ifdef SIMULATION_CHECK_AGING_TIME

typedef GT_STATUS (*CPSS_OS_TIME_RT_FUNC)
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
);
extern CPSS_OS_TIME_RT_FUNC        cpssOsTimeRT;

#endif /*SIMULATION_CHECK_AGING_TIME*/

/**
* @internal sfdbChtMacTableAutomaticAging function
* @endinternal
*
* @brief   MAC Table Trigger Action
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] data_PTR                 - pointer number of entries to age.
*/
GT_VOID sfdbChtMacTableAutomaticAging
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN GT_U8                 * data_PTR
)
{
    GT_U32  fldValue;                /* field value of register */
    GT_U32  * tblActPtr;
    GT_U32  firstEntryIdx;
    GT_U32  numOfEntries;
    GT_U32  * msg_data_PTR = (GT_U32 *)data_PTR;
    GT_U32  regAddr;
    GT_U32  ActionMode;
    GT_U32 tileId = FDB_TILE_ID_GET(devObjPtr);

    ASSERT_PTR(devObjPtr);
    ASSERT_PTR(data_PTR);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        regAddr = SMEM_LION3_FDB_FDB_ACTION_GENERAL_REG(devObjPtr);
    }
    else
    {
        regAddr = SMEM_CHT_MAC_TBL_ACTION0_REG(devObjPtr);
    }

    /* get mac action register */
    tblActPtr = smemMemGet(devObjPtr, regAddr);

    /* Action Mode */
    ActionMode  = SMEM_U32_GET_FIELD(tblActPtr[0], 3, 2);

    /* action to perform (with or without removal )*/
    if (ActionMode == 0)
    {
      fldValue = SFDB_CHEETAH_AUTOMATIC_AGING_REMOVE_E;
    }
    else
    {
      fldValue = SFDB_CHEETAH_AUTOMATIC_AGING_NO_REMOVE_E;
    }
    /* do aging with or without remove. data_PTR contains the indexes to search*/
    memcpy(&firstEntryIdx , msg_data_PTR , sizeof(GT_U32) );
    msg_data_PTR++;
    memcpy(&numOfEntries , msg_data_PTR , sizeof(GT_U32) );

    sfdbChtDoAging(devObjPtr, tblActPtr, fldValue, firstEntryIdx, numOfEntries , tileId);
}

/**
* @internal sfdbChtActionInfoGet function
* @endinternal
*
* @brief   MAC Table Triggering Action - transplant address
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - pointer table action data
*/
static GT_VOID sfdbChtActionInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * tblActPtr,
    OUT AGE_DAEMON_ACTION_INFO_STC      *actionInfoPtr
)
{
    GT_U32    portTrunkSize; /* size of portTrunk field in SIP5 and above registers */

    memset(actionInfoPtr,0,sizeof(AGE_DAEMON_ACTION_INFO_STC));

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {
            portTrunkSize = 15;
        }
        else
        {
            portTrunkSize = 13;
        }
        actionInfoPtr->removeStaticOnNonExistDev = 0;/* NOT supported in SIP5 ,
                                                       since no 'device table' */

        actionInfoPtr->staticTransEn = /* Static Address Transplant Enable */
            SMEM_U32_GET_FIELD(tblActPtr[0], 11, 1);

        actionInfoPtr->staticDelEn = /* Enables deleting of static addresses */
            SMEM_U32_GET_FIELD(tblActPtr[0], 12, 2);

        actionInfoPtr->ageOutAllDevOnTrunk =
            SMEM_U32_GET_FIELD(tblActPtr[0], 14, 1);

        actionInfoPtr->ageOutAllDevOnNonTrunk =
            SMEM_U32_GET_FIELD(tblActPtr[0], 15, 1);

        actionInfoPtr->actVlan =   /* Action Active VLAN */
            SMEM_U32_GET_FIELD(tblActPtr[1], 0, 13);

        actionInfoPtr->actVlanMask = /* Action Active VLAN Mask */
            SMEM_U32_GET_FIELD(tblActPtr[1], 16, 13);

        actionInfoPtr->actPort = /* Action Active Port/Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[2], 0, portTrunkSize);

        actionInfoPtr->actIsTrunk = /* Action Active Trunk bit */
            SMEM_U32_GET_FIELD(tblActPtr[2], portTrunkSize, 1);

        actionInfoPtr->actDevice = /* Action Active Device */
            SMEM_U32_GET_FIELD(tblActPtr[2], (portTrunkSize + 1), 10);

        actionInfoPtr->actPortMask = /* Action Active Port/Trunk Mask */
            SMEM_U32_GET_FIELD(tblActPtr[3], 0, portTrunkSize);

        actionInfoPtr->actIsTrunkMask = /* ActTrunkMask */
            SMEM_U32_GET_FIELD(tblActPtr[3], portTrunkSize, 1);

        actionInfoPtr->actDeviceMask = /* Action Active Device Mask */
            SMEM_U32_GET_FIELD(tblActPtr[3], (portTrunkSize + 1), 10);

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* 11 bits of UDB */
            actionInfoPtr->actUserDefined =
                SMEM_U32_GET_FIELD(tblActPtr[4],  0, 11);

            actionInfoPtr->actUserDefinedMask =
                SMEM_U32_GET_FIELD(tblActPtr[4], 11, 11);
        }
        else
        {
            /* 8 bits of UDB */
            actionInfoPtr->actUserDefined =
                SMEM_U32_GET_FIELD(tblActPtr[4], 0, 8);

            actionInfoPtr->actUserDefinedMask =
                SMEM_U32_GET_FIELD(tblActPtr[4], 8, 8);
        }

        SIM_TBD_BOOKMARK /* need to use those fields */
        actionInfoPtr->actVID1 =   /* Action Active VID1 */
            SMEM_U32_GET_FIELD(tblActPtr[5], 0, 12);

        actionInfoPtr->actVID1Mask = /* Action Active VID1 Mask */
            SMEM_U32_GET_FIELD(tblActPtr[5], 12, 12);

        actionInfoPtr->oldPort = /* Old Port/Old Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[6], 0, portTrunkSize);

        actionInfoPtr->oldIsTrunk = /* Old Is Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[6], portTrunkSize, 1);

        actionInfoPtr->oldDev = /* Old Device */
            SMEM_U32_GET_FIELD(tblActPtr[6], (portTrunkSize + 1), 10);

        actionInfoPtr->newPort = /* new Port/Old Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[7], 0, portTrunkSize);

        actionInfoPtr->newIsTrunk = /* new Is Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[7], portTrunkSize, 1);

        actionInfoPtr->newDev = /* new Device */
            SMEM_U32_GET_FIELD(tblActPtr[7], (portTrunkSize + 1), 10);

    }
    else
    {
        /* NOTE : the tblActPtr is pointer to memory in address 0x06000004 */
        /* we need data from registers :
           0x06000004 , 0x06000008 , 0x06000020

           so  tblActPtr[0] is 0x06000004
               tblActPtr[1] is 0x06000008
               tblActPtr[7] is 0x06000020
        */

        actionInfoPtr->staticDelEn = /* Enables deleting of static addresses */
            SMEM_U32_GET_FIELD(tblActPtr[0], 12, 1);

        actionInfoPtr->oldPort = /* Old Port/Old Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[0], 13, 7);

        actionInfoPtr->newPort = /* New Port/New Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[0], 25, 7);

        actionInfoPtr->oldDev = /* Old Device */
            SMEM_U32_GET_FIELD(tblActPtr[0], 20, 5);

        actionInfoPtr->newDev = /* New Device */
            SMEM_U32_GET_FIELD(tblActPtr[1], 0, 5);

        actionInfoPtr->oldIsTrunk = /* Old Is Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[1], 6, 1);

        actionInfoPtr->newIsTrunk = /* New Is Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[1], 5, 1);

        actionInfoPtr->actVlan =   /* Action Active VLAN */
            SMEM_U32_GET_FIELD(tblActPtr[1], 8, 12);

        actionInfoPtr->actVlanMask = /* Action Active VLAN Mask */
            SMEM_U32_GET_FIELD(tblActPtr[1], 20, 12);

        actionInfoPtr->staticTransEn = /* Static Address Transplant Enable */
            SMEM_U32_GET_FIELD(tblActPtr[0], 11, 1);

        actionInfoPtr->actIsTrunk = /* Action Active Trunk bit */
            SMEM_U32_GET_FIELD(tblActPtr[7], 14, 1);

        actionInfoPtr->actIsTrunkMask = /* ActTrunkMask */
            SMEM_U32_GET_FIELD(tblActPtr[7], 15, 1);

        actionInfoPtr->actPort = /* Action Active Port/Trunk */
            SMEM_U32_GET_FIELD(tblActPtr[7], 0, 7);

        actionInfoPtr->actPortMask = /* Action Active Port/Trunk Mask */
            SMEM_U32_GET_FIELD(tblActPtr[7], 7, 7);

        actionInfoPtr->ageOutAllDevOnTrunk =
            SMEM_U32_GET_FIELD(tblActPtr[7], 16, 1);

        actionInfoPtr->ageOutAllDevOnNonTrunk =
            SMEM_U32_GET_FIELD(tblActPtr[7], 17, 1);

        actionInfoPtr->actDevice = /* Action Active Device */
            SMEM_U32_GET_FIELD(tblActPtr[7], 18, 5);

        actionInfoPtr->actDeviceMask = /* Action Active Device Mask */
            SMEM_U32_GET_FIELD(tblActPtr[7], 23, 5);

        actionInfoPtr->removeStaticOnNonExistDev =
            SMEM_U32_GET_FIELD(tblActPtr[7], 28, 1);

        actionInfoPtr->deviceTableBmpPtr = smemMemGet(devObjPtr, SFDB_CHT_DEVICE_TBL_MEM(devObjPtr));
    }

    smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 4,
        devObjPtr->flexFieldNumBitsSupport.hwDevNum,
        &actionInfoPtr->ownDevNum);

    if(devObjPtr->supportMaskAuFuMessageToCpuOnNonLocal)
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_MAC_TBL_GLB_CONF_REG(devObjPtr) , 26 , 1 , &actionInfoPtr->maskAuFuMessageToCpuOnNonLocal);

        if(actionInfoPtr->maskAuFuMessageToCpuOnNonLocal &&
           devObjPtr->supportMaskAuFuMessageToCpuOnNonLocal_with3BitsCoreId)
        {
            smemRegFldGet(devObjPtr, SMEM_LION2_FDB_METAL_FIX_REG(devObjPtr) , 16 , 1 , &actionInfoPtr->maskAuFuMessageToCpuOnNonLocal_with3BitsCoreId);
        }
    }


    /* AA and TA message to CPU Enable */
    smemRegFldGet(devObjPtr, SMEM_CHT_MAC_TBL_GLB_CONF_REG(devObjPtr), 19, 1, &actionInfoPtr->aaAndTaMessageToCpuEn);
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_MAC_TBL_GLB_CONF_REG(devObjPtr),      20, 1, &actionInfoPtr->routeUcAaAndTaMessageToCpuEn);
        smemRegFldGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr), 21, 1, &actionInfoPtr->routeUcAgingEn);
        smemRegFldGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr), 22, 1, &actionInfoPtr->routeUcTransplantingEn);
        smemRegFldGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr), 23, 1, &actionInfoPtr->routeUcDeletingEn);
    }

    /* <TrunksAgingMode> */
    smemRegFldGet(devObjPtr, SMEM_CHT_MAC_TBL_GLB_CONF_REG(devObjPtr), 8, 1, &actionInfoPtr->trunksAgingMode);

    if(actionInfoPtr->trunksAgingMode == ForceAgeWithoutRemoval)
    {
        /* 1. force aging on trunk regardless to device number */
        actionInfoPtr->ageOutAllDevOnTrunk = 1;
    }

    /**/

    if(devObjPtr->supportEArch)
    {
        /*max Length Src-Id In Fdb Enable*/
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* not used by SIP6 code */
            actionInfoPtr->srcIdLengthInFdbMode = 0;
        }
        else
        {
            smemRegFldGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr),
                          10, 1, &actionInfoPtr->srcIdLengthInFdbMode);
        }

        /* get the <multi hash enable> bit*/
        actionInfoPtr->multiHashEnable = devObjPtr->multiHashEnable;

        /*McAddrDelMode*/
        smemRegFldGet(devObjPtr, SMEM_LION3_FDB_FDB_ACTION_GENERAL_REG(devObjPtr),
                      19, 1, &actionInfoPtr->McAddrDelMode);

        /*IPMCAddrDelMode*/
        smemRegFldGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr),
                      5, 1, &actionInfoPtr->IPMCAddrDelMode);

        /*SPEntryFastAgeEn*/
        smemRegFldGet(devObjPtr, SMEM_LION3_FDB_GLOBAL_CONFIG_1_REG(devObjPtr),
                      9, 1, &actionInfoPtr->SPEntryFastAgeEn);

    }
    else
    {
        /*McAddrDelMode*/
        smemRegFldGet(devObjPtr, SMEM_CHT_MAC_TBL_ACTION2_REG(devObjPtr),
            31, 1, &actionInfoPtr->McAddrDelMode);


        /* this device NOT support to aging/delete of IPMC entry ! */
        actionInfoPtr->IPMCAddrDelMode = 1;
    }

}

/**
* @internal sfdbChtFdbActionEntry_invalidate function
* @endinternal
*
* @brief   MAC Table Auto/triggered Action - invalidate the entry
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] entryPtr                 - pointer to the FDB entry
* @param[in] actionInfoPtr            - (pointer to)common info need for the 'Aging daemon'
* @param[in] index                    -  in the FDB
*                                       none
*/
static void sfdbChtFdbActionEntry_invalidate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 *             entryPtr,
    IN AGE_DAEMON_ACTION_INFO_STC *actionInfoPtr,
    IN GT_U32 index
)
{
    /* 'delete' the entry :
    the SIP5 device with <multiHashEnable> = 1 uses the valid bit --> set to 0
    other devices uses that skip bit --> set to 1.
    */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(actionInfoPtr->multiHashEnable)/*relevant to SIP5*/
        {
            /* the skip bit ignored ... use the valid bit */
            /*valid bit*/
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                index,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID,
                0);

            /* decrement the bank counter of the entry due to 'removed' entry */
            sfdbChtBankCounterAction(devObjPtr,index,
                SFDB_CHT_BANK_COUNTER_ACTION_DECREMENT_E,
                SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_PP_AGING_DAEMON_E);
        }
        else
        {
            /*skip bit*/
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                index,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP,
                1);
        }
    }
    else
    {
        /* skip bit */
        SMEM_U32_SET_FIELD(entryPtr[0], 1, 1, 1);
    }
}



/**
* @internal sfdbChtFdbAaAndTaMsgToCpu function
* @endinternal
*
* @brief   FDB: send AA and TA message to cpu
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] action                   - remove or not the aged out entries
* @param[in] actionInfoPtr            - (pointer to)common info need for the 'Aging daemon'
* @param[in] index                    -  in the FDB
* @param[in] macEntryType             - type of fdb entry
* @param[in] entryPtr                 - MAC table entry pointer
*                                       none
*/
static GT_BOOL sfdbChtFdbAaAndTaMsgToCpu
(
    IN SKERNEL_DEVICE_OBJECT      *devObjPtr,
    IN SFDB_CHEETAH_ACTION_ENT     action,
    IN AGE_DAEMON_ACTION_INFO_STC *actionInfoPtr,
    IN GT_U32                      index,
    IN SNET_CHEETAH_FDB_ENTRY_ENT  macEntryType,
    IN GT_U32                      tileId,
    IN GT_U32                     *entryPtr
)
{
    GT_U32  macUpdMsg[SMEM_CHT_AU_MSG_WORDS] = {0};  /* MAC update message */
    GT_U32  fldValue  = 0;                           /* register's field value */
    GT_BOOL status    = GT_FALSE;                    /* return status */
    GT_U32  isEnabled = 0;                           /* is feature enabled */
    GT_U32  isRouting = 0;                           /* routing indication */


    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:

            isEnabled = actionInfoPtr->aaAndTaMessageToCpuEn;
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:

            isRouting = 1; /* routing indication */
            isEnabled = actionInfoPtr->routeUcAaAndTaMessageToCpuEn;
            break;

        default:
            skernelFatalError("should never happen\n");
            break;
    }

    if(0 == isEnabled)
    {
        return GT_TRUE;
    }

    /* the UPload uses the AA/TA messaging mechanism so I assume use this bit too */

    /* Create update message    */
    sfdbChtFdb2AuMsg(devObjPtr, index, entryPtr, &macUpdMsg[0]);

    /* Message Type */
    if(action == SFDB_CHEETAH_TRIGGER_TRANS_E)
    {
        fldValue = SFDB_UPDATE_MSG_TA_E;
    }
    else
    if(action == SFDB_CHEETAH_TRIGGER_UPLOAD_E)
    {
        fldValue = SFDB_UPDATE_MSG_FU_E;
    }
    else
    {
       fldValue = SFDB_UPDATE_MSG_AA_E;
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* Message Type */
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE,
            fldValue);

        /* Entry Index 0..8 */
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0,
            index);

        /* Entry Index 9..21 */
        if(isRouting)
        {
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX_20_9,
                index >> 9);
        }
        else
        {
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9,
                index >> 9);
        }
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Message Type */
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE,
            fldValue);

        /* Entry Index */
        if(isRouting)
        {
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX,
                index);
        }
        else
        {
            /* Entry Index 0..8 */
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0,
                index);
            /* Entry Index 9..21 */
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9,
                index >> 9);
        }
    }
    else
    {
        /* Message Type */
        SMEM_U32_SET_FIELD(macUpdMsg[0], 4, 3, fldValue);

        /* Entry Index */
        SMEM_U32_SET_FIELD(macUpdMsg[0], 7, 8, (index & 0xFF));
        SMEM_U32_SET_FIELD(macUpdMsg[3], 27, 4,((index >> 8) & 0xF));
        SMEM_U32_SET_FIELD(macUpdMsg[3], 0, 2, ((index >> 12) & 0x3));
    }

    tileId = FDB_TILE_ID_GET(devObjPtr);
    /* Send MAC update message to CPU */
    while(GT_FALSE == status)
    {
        if (action == SFDB_CHEETAH_TRIGGER_UPLOAD_E)
        {
            status = snetCht2L2iSendFuMsg2Cpu(devObjPtr, macUpdMsg);
        }
        else
        {
            status = snetChtL2iSendUpdMsg2Cpu(devObjPtr, macUpdMsg);
        }

        if(status == GT_FALSE)
        {
            if(oldWaitDuringSkernelTaskForAuqOrFua)
            {
                /* wait for SW to free buffers */
                SIM_OS_MAC(simOsSleep)(SNET_CHEETAH_NO_FDB_ACTION_BUFFERS_SLEEP_TIME);
                /* do not stack the sKernel task , re-send message to continue from this point */
            }
            else
            {
                /* state that the daemon is on hold */
                devObjPtr->fdbAgingDaemonInfo[tileId].daemonOnHold = 1;

                return GT_FALSE;
            }
        }
    }

    return GT_TRUE;
}


/**
* @internal sfdbChtFdbActionEntry function
* @endinternal
*
* @brief   MAC Table Auto/triggered Action.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] action                   - remove or not the aged out entries
* @param[in] actionInfoPtr            - (pointer to)common info need for the 'Aging daemon'
* @param[in] index                    -  in the FDB
*
* @param[out] needBreakPtr             - (pointer to) indication that the loop iteration must break
*                                      and continue from current index , by new skernel message  .
*                                       none
*/
static void sfdbChtFdbActionEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SFDB_CHEETAH_ACTION_ENT action,
    IN AGE_DAEMON_ACTION_INFO_STC *actionInfoPtr,
    IN GT_U32 index,
    IN GT_U32   tileId,
    OUT GT_BIT  *needBreakPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtFdbActionEntry);

    GT_U32 regAddr;                     /* Register address */
    GT_U32 fldValue;                    /* Register's field value */
    GT_U32 * entryPtr;                  /* MAC table entry pointer */
    GT_U32  staticEntry = 0;            /* is current entry static, must be zero by default */
    GT_U32  devNumInEntry;/* the device number of the entry */
    GT_U32  useVidxInEntry;/* the entry is for vidx */
    GT_U32  portInEntry;   /* port number in the entry */
    GT_U32  isLocalDevPort;/*is entry of type 'port' (not vidx/trunk) with 'own dev num'
                            NOTE: this is not 'local port' in terms of 'multi-port groups'
                                  but in terms of 'local to this device'*/
    GT_U32  ageOnAllDevices;/*do age on all entries (trunk and non trunk) that
                                      registered on other devices*/
    GT_U32  udbInEntry;/*user define field in the entry*/
    GT_U32  entry_isTrunk;/* the field isTrunk in the entry */
    GT_U32  entry_portTrunkNum; /* the field portTrunkNum in the entry */
    GT_BIT  isPortValid;/* indication the info is about 'port' (not trunk and not vidx) */

    SNET_CHEETAH_FDB_ENTRY_ENT   macEntryType;
    GT_BIT  treatAsAgeWithoutRemoval;
    GT_BIT  treatAsAgeWithRemoval = 0;

    GT_U32  isUcRoutingEntry = 0; /* indicates fdb entry type is one of uc routing entries,
                                     must be zero by default */
    GT_U32  isUcIpv6KeyRoutingEntry = 0; /* is Ipv6 UC address entry */

    SMEM_LION3_FDB_FDB_TABLE_FIELDS entryField;/* SIP5 and not SIP6 */
    GT_BIT  isAgeOrDelete = (SFDB_CHEETAH_TRIGGER_UPLOAD_E || SFDB_CHEETAH_TRIGGER_TRANS_E) ? 0  :1;
    GT_BOOL status;/* status of send message to the CPU */
    GT_BIT  spUnknown;
    SNET_SIP6_FDB_ENTRY_INFO sip6FdbEntryInfo;
    SNET_SIP6_FDB_ENTRY_INFO *sip6FdbEntryInfoPtr = NULL; /* to avoid  warning C4701: potentially uninitialized local variable 'sip6FdbEntryInfo' used */
    GT_U32  macEntrySize;
    GT_U32  bypassAllMasksFilters = 0;/* when 1 : indication that action would apply on the entry */
    GT_U32  macAddrWords[2];
    GT_BOOL isMultiCastEntry = GT_FALSE;

    macEntrySize = SMEM_CHT_MAC_TABLE_WORDS_MAC(devObjPtr);

    if(devObjPtr->fdbNumEntries == 0)
    {
        /* can be in sip6.30 when FDB used only by the DDE for HSR-PRP */
        __LOG(("FDB size is ZERO !!! no action supported \n"));
        return;
    }

    *needBreakPtr = 0;
    index =  index % devObjPtr->fdbNumEntries;

#ifdef SIMULATION_CHECK_AGING_TIME
    {
        static GT_U32      start_sec  = 0;
        static GT_U32      start_nsec = 0;
        GT_U32      end_sec  = 0;
        GT_U32      end_nsec = 0;
        GT_U32      diff_sec;
        GT_U32      diff_nsec;

        if(cpssOsTimeRT && (devObjPtr->portGroupId == 0))
        {
            if(index == 0)
            {
                cpssOsTimeRT(&start_sec, &start_nsec);
            }
            else if(index == (devObjPtr->fdbNumEntries - 1))
            {
                cpssOsTimeRT(&end_sec, &end_nsec);
                if(end_nsec < start_nsec)
                {
                    end_nsec += 1000000000;
                    end_sec  -= 1;
                }
                diff_sec  = end_sec  - start_sec;
                diff_nsec = end_nsec - start_nsec;

                simGeneralPrintf(" aging time  = %d sec + %d nano \n" , diff_sec , diff_nsec);
            }
        }
    }
#endif /*SIMULATION_CHECK_AGING_TIME*/

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* Get entryPtr according to entry index */
        entryPtr = SMEM_SIP6_HIDDEN_FDB_PTR(devObjPtr, index);
        regAddr = SMAIN_NOT_VALID_CNS;
    }
    else
    {
        /* Get entryPtr according to entry index */
        regAddr = SMEM_CHT_MAC_TBL_MEM(devObjPtr, index);
        entryPtr = smemMemGet(devObjPtr, regAddr);
    }

    /* Valid */
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        sfdbSip6ParseFdbEntry(devObjPtr,entryPtr,index,&sip6FdbEntryInfo);
        sip6FdbEntryInfoPtr = &sip6FdbEntryInfo;

        fldValue = sip6FdbEntryInfoPtr->valid;
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        fldValue =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
            index,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_VALID);
    }
    else
    {
        fldValue = SMEM_U32_GET_FIELD(entryPtr[0], 0, 1);
    }

    if (fldValue == 0)
    {
        return;
    }

    if(actionInfoPtr->multiHashEnable == 0)
    {
        if(sip6FdbEntryInfoPtr)
        {
            /* Skip */
            fldValue = sip6FdbEntryInfoPtr->skip;
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Skip */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                index,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SKIP);
        }
        else
        {
            /* Skip */
            fldValue = SMEM_U32_GET_FIELD(entryPtr[0], 1, 1);
        }

        if (fldValue)
        {
            return;
        }
    }

    /* mac entry type */
    if(sip6FdbEntryInfoPtr)
    {
        macEntryType = sip6FdbEntryInfoPtr->fdb_entry_type;
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        macEntryType =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
            index,
            SMEM_LION3_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE);
    }
    else
    {
        macEntryType = SMEM_U32_GET_FIELD(entryPtr[0], 3, 2);
    }

    if (SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E == macEntryType ||
        SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E == macEntryType)
    {
        if (action == SFDB_CHEETAH_TRIGGER_UPLOAD_E)
        {
            status = sfdbChtFdbAaAndTaMsgToCpu(devObjPtr, action, actionInfoPtr, index, macEntryType, tileId,entryPtr);
            if(status == GT_FALSE)
            {
                *needBreakPtr = 1;
            }
            return ;
        }

        if (action == SFDB_CHEETAH_TRIGGER_TRANS_E)
        {
            /* the IPMC entries are NOT subject transplant */
            return;
        }

        if(actionInfoPtr->IPMCAddrDelMode == 1)
        {
            /* the ipmc entries are NOT subject to next operations */
            return;
        }

        /* NOTE: non sip5 device can not reach here ! */

        if(sip6FdbEntryInfoPtr)
        {
            staticEntry = sip6FdbEntryInfoPtr->is_static;
        }
        else
        {
            staticEntry =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                index,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_STATIC);
        }

        if(action != SFDB_CHEETAH_TRIGGER_DELETE_E)
        {
            if(staticEntry)
            {
                /* aging is not subject on static entries */
                return;
            }

            if(sip6FdbEntryInfoPtr)
            {
                /* age  bit */
                fldValue = sip6FdbEntryInfoPtr->age;
            }
            else
            {
                /* age  bit */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                        index,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_AGE);
            }

            if (fldValue)
            {
                /* entry is not aged out.
                   reset age bit and continue with next entry. */
                if(sip6FdbEntryInfoPtr)
                {
                    /* age  bit */
                    SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                        index,
                        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE,
                        0);
                }
                else
                {
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                        index,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_AGE,
                        0);
                }

                return ;
            }
        }
        else /*SFDB_CHEETAH_TRIGGER_DELETE_E*/
        {
            if (actionInfoPtr->staticDelEn == 0 && staticEntry)
            {
                /* Skip entries when delete static entries disable */
                return;
            }
        }

        /* Triggered address deleting */
        sfdbChtFdbActionEntry_invalidate(devObjPtr,entryPtr,actionInfoPtr,index);

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* Write aged/deleted entry */
            smemGenericHiddenMemSet(devObjPtr, SMEM_GENERIC_HIDDEN_MEM_FDB_E, index , entryPtr, macEntrySize);
        }
        else
        {
            /* Write aged/deleted entry */
            smemMemSet(devObjPtr, regAddr, entryPtr, macEntrySize);
        }
        status = sfdbChtFdbAaAndTaMsgToCpu(devObjPtr, action, actionInfoPtr, index, macEntryType, tileId , entryPtr);
        if(status == GT_FALSE)
        {
            *needBreakPtr = 1;
        }
    }
    else if (SNET_CHEETAH_FDB_ENTRY_MAC_E != macEntryType)
    {
        isUcRoutingEntry = 1;

        if (SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E == macEntryType)
        {
            isUcIpv6KeyRoutingEntry = 1;
        }
    }
    else
    {
        if(sip6FdbEntryInfoPtr)
        {
            spUnknown = sip6FdbEntryInfoPtr->sp_unknown;
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            entryField = SMEM_LION3_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN;
            spUnknown =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr, index, entryField);
        }
        else
        {
            spUnknown = snetFieldValueGet(entryPtr,98,1);
        }

        if(spUnknown &&
           actionInfoPtr->SPEntryFastAgeEn == 0)
        {
            /* the action modify the aging bit and act according to it ...
               even for action'DELETE' !!!
            */

            /* JIRA : MT-294 : The design decides whether to delete an FDB entry based
                on age bit in case of storm prevention entry */
            treatAsAgeWithRemoval = 1;
        }
    }

    if(treatAsAgeWithRemoval &&
       action == SFDB_CHEETAH_TRIGGER_DELETE_E)
    {
        /* act on the entry as if was 'aging with removal' */
        action = SFDB_CHEETAH_TRIGGER_AGING_REMOVE_E;
    }

    if(sip6FdbEntryInfoPtr)
    {
        useVidxInEntry = isUcRoutingEntry ?
                sip6FdbEntryInfoPtr->ucRoute.use_vidx :
                sip6FdbEntryInfoPtr->multiple;

        if(0 == isUcRoutingEntry) /* The Static bit is not supported for FDB-based host route entries */
        {
            staticEntry = sip6FdbEntryInfoPtr->is_static;
        }

        devNumInEntry = isUcRoutingEntry ?
                sip6FdbEntryInfoPtr->ucRoute.trg_dev :
                sip6FdbEntryInfoPtr->dev_id;
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX :
                                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_MULTIPLE ;
        useVidxInEntry =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr, index, entryField);

        if(0 == isUcRoutingEntry) /* The Static bit is not supported for FDB-based host route entries */
        {
            staticEntry =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                index,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_STATIC);
        }

        entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV :
                                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_DEV_ID ;
        devNumInEntry =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr, index, entryField);
    }
    else
    {
        useVidxInEntry = SMEM_U32_GET_FIELD(entryPtr[2],26,1);
        staticEntry = SMEM_U32_GET_FIELD(entryPtr[2], 25, 1);
        devNumInEntry = SMEM_U32_GET_FIELD(entryPtr[2], 1, 5);

        /* check that the device in the entry exists */
        /* NOTE : even multicast entries are checked */
        if(actionInfoPtr->ownDevNum != devNumInEntry && /* not own dev num */
            /*Indexing to it is performed using the lower 5 bits of descriptor<DevNum>*/
          (snetFieldValueGet(actionInfoPtr->deviceTableBmpPtr,(devNumInEntry & 0x1f),1) == 0))
        {
            if(actionInfoPtr->removeStaticOnNonExistDev || staticEntry == 0)
            {
                /* delete the entry */
                sfdbChtFdbActionEntry_invalidate(devObjPtr,entryPtr,actionInfoPtr,index);

                /* DO NOT SEND AA message to CPU !!! */
                return;
            }
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if ((macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E ||
             macEntryType == SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E ||
             macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E) && useVidxInEntry)
        {

            isMultiCastEntry = GT_TRUE;
        }
        else if (macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E ||
                 macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E)
        {
            isMultiCastEntry = GT_TRUE;
        }
        else if (macEntryType == SNET_CHEETAH_FDB_ENTRY_MAC_E)
        {
            /* MAC ADDR */
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,
                entryPtr,
                index,
                macAddrWords);

            /* Bit 40 */
            fldValue = SMEM_U32_GET_FIELD(macAddrWords[1], 8, 1);
            if (useVidxInEntry || fldValue)
            {
                isMultiCastEntry = GT_TRUE;
            }
        }
    }
    else
    {
        if (SNET_CHEETAH_FDB_ENTRY_MAC_E != macEntryType)
        {
            isMultiCastEntry = GT_TRUE;
        }
    }

    if(0 == isUcRoutingEntry) /* fid filtering is not relevant for fdb routing */
    {
        if(sip6FdbEntryInfoPtr)
        {
            /* FID */
            fldValue = sip6FdbEntryInfoPtr->fid;
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* FID */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                index,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_FID);
        }
        else
        {
            /* Vlan ID */
            fldValue = SMEM_U32_GET_FIELD(entryPtr[0], 5, 12);
        }

        if ((fldValue & actionInfoPtr->actVlanMask) != actionInfoPtr->actVlan)
        {
            return;
        }
    }
    else
    {
        /* according to FS : When performing age/delete with restricted subset that is not supported by IPv4 and FCOE entries, the
            global control that allow age/delete for FDB-based host route entries must be disabled. Otherwise,
            all the FDB-based host route entries would be aged/deleted.

            meaning : that if mask of "Vlan-Id , UDB" is not ZERO (all bits 0) ,
            every ipv4/fcoe/Ipv6-data/ipv6 key entry would be aged/deleted
            (depend on "global control that allow age/delete")
        */
        if ((actionInfoPtr->actVlanMask!=0)||
            (actionInfoPtr->actUserDefinedMask!=0))
        {
            bypassAllMasksFilters = 1;
        }
    }

    if(isMultiCastEntry && actionInfoPtr->McAddrDelMode == 1 && isAgeOrDelete)
    {
        /* the MC entries are NOT subject to aging/delete */
        return;
    }

    if(sip6FdbEntryInfoPtr)
    {
        /* trunk  */
        entry_isTrunk = isUcRoutingEntry ?
            sip6FdbEntryInfoPtr->ucRoute.trg_is_trunk:
            sip6FdbEntryInfoPtr->is_trunk;

        if(entry_isTrunk)/*port*/
        {
            /* trunk num */
            entry_portTrunkNum = isUcRoutingEntry ?
                sip6FdbEntryInfoPtr->ucRoute.trg_trunk_id:
                sip6FdbEntryInfoPtr->trunk_num;
        }
        else
        {
            /* eport num */
            entry_portTrunkNum = isUcRoutingEntry ?
                sip6FdbEntryInfoPtr->ucRoute.trg_eport:
                sip6FdbEntryInfoPtr->eport_num;
        }
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* trunk  */
        entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK :
                                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_TRUNK ;
        entry_isTrunk =
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr, index, entryField);

        if(entry_isTrunk)/*port*/
        {
            /* trunk num */
            entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID :
                                            SMEM_LION3_FDB_FDB_TABLE_FIELDS_TRUNK_NUM ;
            entry_portTrunkNum =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr, index, entryField);
        }
        else
        {
            /* eport num */
            entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT :
                                            SMEM_LION3_FDB_FDB_TABLE_FIELDS_EPORT_NUM ;
            entry_portTrunkNum =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr, index, entryField);
        }
    }
    else
    {
        /* Trunk */
        entry_isTrunk = SMEM_U32_GET_FIELD(entryPtr[2], 13, 1);
        /* Port Num/Trunk Num */
        entry_portTrunkNum = SMEM_U32_GET_FIELD(entryPtr[2], 14, 7);
    }

    if(actionInfoPtr->trunksAgingMode == ForceAgeWithoutRemoval &&
           (isMultiCastEntry == GT_FALSE && entry_isTrunk == 1)/*trunk entry*/)
    {
        /* cq# 150852 */
        /* this entry associated with trunk , but trunk aging mode forced to
            'Age without removal'*/
        treatAsAgeWithoutRemoval = 1;
    }
    else
    {
        treatAsAgeWithoutRemoval = 0;
    }


    if(action == SFDB_CHEETAH_TRIGGER_TRANS_E)
    {
        if ((SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E == macEntryType)||
            (SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E == macEntryType))
        {
            /* Ipv6 Uc Data/Key entry is not subject to transplanting */
            return;
        }

        if(isUcRoutingEntry && 0 == actionInfoPtr->routeUcTransplantingEn)
        {
            /* uc routing entry: transplanting disabled */
            return;
        }

        /* trunk/port masking done only on unicast entries */
        if(isMultiCastEntry == GT_TRUE)
        {
            return;
        }

        if (actionInfoPtr->staticTransEn == 0 && staticEntry == 1)
        {
            /* Skip entries when transplant static entries disable */
            return;
        }
        /* DevID */
        if (devNumInEntry != actionInfoPtr->oldDev)
        {
            return;
        }

        /* Trunk */
        fldValue = entry_isTrunk;

        isLocalDevPort = 1 - fldValue;
        isPortValid = isLocalDevPort;
        if (fldValue != actionInfoPtr->oldIsTrunk)
        {
            return;
        }

        /* Port Num/Trunk Num */
        fldValue = entry_portTrunkNum;

        if (fldValue != actionInfoPtr->oldPort)
        {
            return;
        }

        if(actionInfoPtr->ownDevNum != devNumInEntry)
        {
            isLocalDevPort = 0;
        }

        /* Lookup entry is match */
        if(sip6FdbEntryInfoPtr)
        {
            /* New device */
            entryField = isUcRoutingEntry ? SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV :
                                            SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID ;
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
            index,
            entryField,
            actionInfoPtr->newDev);

            /* New Is Trunk */
            entryField = isUcRoutingEntry ? SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK :
                                            SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK ;
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
            index,
            entryField,
            actionInfoPtr->newIsTrunk);

            if(actionInfoPtr->newIsTrunk)
            {
                /* New trunk */
                entryField = isUcRoutingEntry ? SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID :
                                                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM ;
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                index,
                entryField,
                actionInfoPtr->newPort);
            }
            else
            {
                /* New port */
                entryField = isUcRoutingEntry ? SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT :
                                                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM ;
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                index,
                entryField,
                actionInfoPtr->newPort);
            }
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* New device */
            entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV :
                                            SMEM_LION3_FDB_FDB_TABLE_FIELDS_DEV_ID ;
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
            index,
            entryField,
            actionInfoPtr->newDev);

            /* New Is Trunk */
            entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK :
                                            SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_TRUNK ;
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
            index,
            entryField,
            actionInfoPtr->newIsTrunk);

            if(actionInfoPtr->newIsTrunk)
            {
                /* New trunk */
                entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID :
                                                SMEM_LION3_FDB_FDB_TABLE_FIELDS_TRUNK_NUM ;
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                index,
                entryField,
                actionInfoPtr->newPort);
            }
            else
            {
                /* New port */
                entryField = isUcRoutingEntry ? SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT :
                                                SMEM_LION3_FDB_FDB_TABLE_FIELDS_EPORT_NUM ;
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                index,
                entryField,
                actionInfoPtr->newPort);
            }
        }
        else
        {
            /* New device */
            SMEM_U32_SET_FIELD(entryPtr[2], 1, 5, actionInfoPtr->newDev);
            /* New Is Trunk */
            SMEM_U32_SET_FIELD(entryPtr[2], 13, 1, actionInfoPtr->newIsTrunk);
            /* New port */
            SMEM_U32_SET_FIELD(entryPtr[2], 14, 7, actionInfoPtr->newPort);
        }
    }
    else
    {
        if(isUcRoutingEntry && 0 == actionInfoPtr->routeUcAgingEn)
        {
            if (action == SFDB_CHEETAH_TRIGGER_AGING_REMOVE_E    ||
                action == SFDB_CHEETAH_TRIGGER_AGING_NO_REMOVE_E ||
                action == SFDB_CHEETAH_AUTOMATIC_AGING_REMOVE_E  ||
                action == SFDB_CHEETAH_AUTOMATIC_AGING_NO_REMOVE_E)
            {
                /* uc routing entry: aging disabled */
                return;
            }
        }

        if (/*!(SMEM_CHT_IS_SIP5_10_GET(devObjPtr)) &&*/
            (SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E == macEntryType)&&
            (action != SFDB_CHEETAH_TRIGGER_DELETE_E))
        {
            /* Ipv6 Uc Data entry is not subject to aging */
            /* in SIP_5_10 do aging on Data entry and not on the Key */
            return;
        }

        /* for BobCat2 B0 and above
           In case of IPv6 key or data, in order for an entry to be subject to aging
           and delete all the following should be set to 0:
           actPortMask, actIsTrunkMask, actDeviceMask.  */
        if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr)&&
           ((SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E == macEntryType)||
            (SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E == macEntryType)))
        {
            if ((actionInfoPtr->actDeviceMask!=0)||
                (actionInfoPtr->actIsTrunkMask!=0)||
                (actionInfoPtr->actPortMask!=0))
            {
                /* the ipv6 entry is not subject to aging */
                return;
            }
        }

        if (action == SFDB_CHEETAH_TRIGGER_UPLOAD_E)
        {
            ageOnAllDevices = 1;
        }
        else
        {
            ageOnAllDevices = actionInfoPtr->ageOutAllDevOnNonTrunk;
        }

       isLocalDevPort = 0;
       isPortValid = 0;

       /* trunk/port masking done only on unicast entries */
       if(isMultiCastEntry == GT_FALSE)
       {
           /* Address associated  with trunk */
           fldValue = entry_isTrunk;

           isLocalDevPort = 1 - fldValue;

            if(bypassAllMasksFilters)
            {
                /* the entry subject to the action */
            }
            else
            if ((isUcIpv6KeyRoutingEntry==0) &&
                ((fldValue & actionInfoPtr->actIsTrunkMask) != actionInfoPtr->actIsTrunk))
            {
                return;
            }

            isPortValid = isLocalDevPort;

            if (action == SFDB_CHEETAH_TRIGGER_UPLOAD_E)
            {
                ageOnAllDevices = fldValue ?
                                  actionInfoPtr->ageOutAllDevOnTrunk :
                                  1;
            }
            else
            {
                ageOnAllDevices =  fldValue ?
                                   actionInfoPtr->ageOutAllDevOnTrunk :
                                   actionInfoPtr->ageOutAllDevOnNonTrunk;
            }

            /* Port Number/ Trunk Number */
            fldValue = entry_portTrunkNum;

            if(bypassAllMasksFilters)
            {
                /* the entry subject to the action */
            }
            else
            if ((isUcIpv6KeyRoutingEntry==0) &&
                ((fldValue & actionInfoPtr->actPortMask) != actionInfoPtr->actPort))
            {
                return;
            }
        }
        else /* 'vidx' entries */
        {
            if(devObjPtr->errata.fdbAgingDaemonVidxEntries)
            {
                /* aging daemon skip fdb entries of fdb<FDBEntryType> MAC with fdb<multiple> = 1 (vidx entries) ,
                   unless (<ActDevMask > & <ActDev>) = 0 (from action 1,2 registers ) */
                if(bypassAllMasksFilters)
                {
                    /* the entry subject to the action */
                }
                else
                if ((isUcIpv6KeyRoutingEntry==0) &&
                    ((actionInfoPtr->actDeviceMask & actionInfoPtr->actDevice) != 0))
                {
                    /* don't age entries that are not registered on this device */
                    return;
                }
            }
        }

        if(actionInfoPtr->ownDevNum != devNumInEntry)
        {
            isLocalDevPort = 0;
        }

        /* Device number */
        if (ageOnAllDevices == 0)
        {
            if(bypassAllMasksFilters)
            {
                /* the entry subject to the action */
            }
            else
            if ((isUcIpv6KeyRoutingEntry==0) &&
                ((devNumInEntry & actionInfoPtr->actDeviceMask) != actionInfoPtr->actDevice) &&
                (isMultiCastEntry == GT_FALSE))
            {
                /* don't age entries that are not registered on this device */
                return;
            }
        }

        if(bypassAllMasksFilters)
        {
            /* the entry subject to the action */
        }
        else
        if (isUcRoutingEntry)
        {
            /* UDB not relevant for uc routing: do nothing */
        }
        else
        if(sip6FdbEntryInfoPtr)
        {
            if ((sip6FdbEntryInfoPtr->user_defined  & actionInfoPtr->actUserDefinedMask) !=
                actionInfoPtr->actUserDefined)
            {
                /* don't age entries that are not match on 'user defined' */
                return;
            }
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* UDB  */
            udbInEntry =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                index,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_USER_DEFINED);

            if(actionInfoPtr->srcIdLengthInFdbMode == 0)
            {
                /* the 3 MSB of the UDB used in srcId ! */
                udbInEntry &= 0x1F;
            }

            if ((udbInEntry  & actionInfoPtr->actUserDefinedMask) != actionInfoPtr->actUserDefined)
            {
                /* don't age entries that are not match on 'user defined' */
                return;
            }
        }
    }/*transplant*/


    /* NOTE that :
      1. multicast entries can be aged if not static entries !
      2. multicast entries are subject to "triggered delete" just like other
         entries.
    */

    /* Look up entry is match */
    if (action == SFDB_CHEETAH_TRIGGER_DELETE_E)
    {
        if(isUcRoutingEntry && 0 == actionInfoPtr->routeUcDeletingEn)
        {
            /* uc routing entry: deleting disabled */
            return;
        }

        if (actionInfoPtr->staticDelEn == 0 && staticEntry)
        {
            /* Skip entries when delete static entries disable */
            return;
        }
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if (actionInfoPtr->staticDelEn == 2 && !staticEntry)
            {
                /* Skip dynamic entries when static only delete is enabled */
                return;
            }
        }

        if(treatAsAgeWithoutRemoval == 0)
        {
            /* cq# 150852 */
            /* Triggered address deleting */
            sfdbChtFdbActionEntry_invalidate(devObjPtr,entryPtr,actionInfoPtr,index);
        }
    }
    else if (action != SFDB_CHEETAH_TRIGGER_TRANS_E &&
              action != SFDB_CHEETAH_TRIGGER_UPLOAD_E)
    {
        if (staticEntry)
        {
            /* don't age static entries */
            return;
        }

        /* Triggered address aging without removal of aged out entries */
        /* Triggered address aging with removal of aged out entries */
        /* Automatic address aging without removal of aged out entries */
        /* Automatic address aging with removal of aged out entries */
        if(sip6FdbEntryInfoPtr)
        {
            /* age  bit */
            fldValue = sip6FdbEntryInfoPtr->age;

            if (fldValue)
            {
                /* entry is not aged out.
                   reset age bit and continue with next entry. */
                SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                    index,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE,
                    0);

                return ;
            }
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* age  bit */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,entryPtr,
                    index,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_AGE);
            if (fldValue)
            {
                /* entry is not aged out.
                   reset age bit and continue with next entry. */
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,entryPtr,
                    index,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_AGE,
                    0);

                return ;
            }

        }
        else
        {
            if (SMEM_U32_GET_FIELD(entryPtr[0], 2, 1))
            {
                /* entry is not aged out.
                   reset age bit and continue with next entry. */
                SMEM_U32_SET_FIELD(entryPtr[0], 2, 1, 0);
                return ;
            }
        }

        /* Triggered address aging with removal of aged out entries */
        /* Automatic address aging with removal of aged out entries */
        if ((action == SFDB_CHEETAH_AUTOMATIC_AGING_REMOVE_E) ||
            (action == SFDB_CHEETAH_TRIGGER_AGING_REMOVE_E))
        {
            if(isUcRoutingEntry && actionInfoPtr->routeUcAgingEn)
            {
                /* do not remove the entry, only age_without_removal
                   is supported in case of UC routing entries*/
            }
            else if(treatAsAgeWithoutRemoval == 0)
            {
                /* cq# 150852 */
                /* the entry is aged out - set the skip bit */
                sfdbChtFdbActionEntry_invalidate(devObjPtr,entryPtr,actionInfoPtr,index);
            }
        }
    }

    if (action != SFDB_CHEETAH_TRIGGER_UPLOAD_E)/* not update the table */
    {
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* Write aged/transplanted entry */
            smemGenericHiddenMemSet(devObjPtr, SMEM_GENERIC_HIDDEN_MEM_FDB_E, index , entryPtr, macEntrySize);
        }
        else
        {
            /* Write aged/transplanted entry */
            smemMemSet(devObjPtr, regAddr, entryPtr, macEntrySize);
        }
    }

    if(devObjPtr->supportEArch == 0)
    {
        portInEntry = entry_portTrunkNum;

        if(isPortValid && /* indication that info is about 'port' */
           actionInfoPtr->maskAuFuMessageToCpuOnNonLocal_with3BitsCoreId)
        {
            if((actionInfoPtr->ownDevNum & 0xFFFFFFFE) == (devNumInEntry & 0xFFFFFFFE))
            {
                /* local port to 1 of my hemispheres */

                GT_U32   devLsBit = devNumInEntry & 1;
                GT_U32   entryPortGroupId = ((portInEntry >> 4) & 3) | (devLsBit << 2);

                if(entryPortGroupId != (devObjPtr->portGroupId & 7))
                {
                    /* checking all bits of the codeId */

                    /* AU/FU messages are filtered by HW .
                        port checked according to 3 bits comparison as follows: {device[0],port[5:4]} vs pipe_id[2:0]. */
                    return;
                }

            }
        }
        else
        if(devObjPtr->portGroupSharedDevObjPtr && actionInfoPtr->maskAuFuMessageToCpuOnNonLocal &&
           isLocalDevPort && (((portInEntry >> 4) & 3) != (devObjPtr->portGroupId & 3)))
        {
            /*When Enabled - AU/FU messages are filtered by HW if the MAC entry
               does NOT reside on the local port group, i.e. the entry port[5:4] != port group */
            return;
        }
    }

    status = sfdbChtFdbAaAndTaMsgToCpu(devObjPtr, action, actionInfoPtr, index, macEntryType, tileId,entryPtr);
    if(status == GT_FALSE)
    {
        *needBreakPtr = 1;
    }

}

/**
* @internal sfdbChtTriggerTa function
* @endinternal
*
* @brief   MAC Table Triggering Action - transplant address
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - pointer table action data
*/
static GT_VOID sfdbChtTriggerTa
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * tblActPtr,
    IN GT_U32                tileId
)
{
    GT_U32 entry;                       /* MAC table entry index */
    AGE_DAEMON_ACTION_INFO_STC actionInfo;/* common info need for the 'Aging daemon' */
    GT_BIT  needBreak = 0;
    /* get common action info */
    sfdbChtActionInfoGet(devObjPtr,tblActPtr,&actionInfo);

    entry = devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb;

    for (/**/; entry < devObjPtr->fdbNumEntries; entry++)
    {
        sfdbChtFdbActionEntry(devObjPtr,SFDB_CHEETAH_TRIGGER_TRANS_E,&actionInfo,entry,tileId,&needBreak);
        if(needBreak)
        {
            /* the action is broken and will be continued in new message */
            /* save the index */
            devObjPtr->needResendMessage = 1;
            devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = entry;
            return;
        }
        /* state that the daemon is NOT on hold */
        devObjPtr->fdbAgingDaemonInfo[tileId].daemonOnHold = 0;
    }

    devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = 0;
}

/**
* @internal sfdbChtDoAging function
* @endinternal
*
* @brief   MAC Table Triggering Action - delete address, aging without removal
*         of aged out entries, triggered aging with removal of aged out entries.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - action mode being performed on the entries
* @param[in] action                   - remove or not the aged out entries
* @param[in] firstEntryIdx            - index to the first enrty .
* @param[in] numOfEntries             - number of entries to be searched.
*/
static GT_VOID sfdbChtDoAging
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN GT_U32                * tblActPtr,
  IN GT_U32                  action,
  IN GT_U32                  firstEntryIdx,
  IN GT_U32                  numOfEntries,
  IN GT_U32                  tileId
)
{
    DECLARE_FUNC_NAME(sfdbChtDoAging);

    GT_U32 entry;                       /* MAC table entry index */
    AGE_DAEMON_ACTION_INFO_STC actionInfo;/* common info need for the 'Aging daemon' */
    GT_BIT  needBreak = 0;

    /* get common action info */
    sfdbChtActionInfoGet(devObjPtr,tblActPtr,&actionInfo);

    firstEntryIdx = devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb;

    for (entry = firstEntryIdx;
         entry < firstEntryIdx + numOfEntries ;
         entry++)
    {
        sfdbChtFdbActionEntry(devObjPtr,action,&actionInfo,entry,tileId,&needBreak);
        if(needBreak)
        {
            /* the action is broken and will be continued in new message */
            /* save the index */
            devObjPtr->needResendMessage = 1;
            devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = entry;
            return;
        }
        /* state that the daemon is NOT on hold */
        devObjPtr->fdbAgingDaemonInfo[tileId].daemonOnHold = 0;
    }

    if(devObjPtr->fdbNumEntries == 0)
    {
        /* can be in sip6.30 when FDB used only by the DDE for HSR-PRP */
        __LOG(("FDB size is ZERO !!! no action supported \n"));
        return;
    }
    devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = entry % devObjPtr->fdbNumEntries;
}

/**
* @internal sfdbChtQxMsgProcess function
* @endinternal
*
* @brief   Query Address Entry Query Response.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] msgWordPtr               - pointer to fdb message.
*/
static GT_STATUS sfdbChtQxMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtQxMsgProcess);

    GT_U32 fdbIndex;                         /* FDB entry index */
    GT_STATUS  status;                      /* function return status */
    GT_U32 fldValue;                        /* Register's field value */
    GT_U32 * entryPtr;                      /* MAC table entry pointer */
    GT_U32 macUpdMsg[SMEM_CHT_AU_MSG_WORDS];                    /* MAC update message */
    GT_U32 entryOffset;                     /* MAC address offset */
    GT_U32  fdbMsgId;                       /* FDB Message ID */

    status = parseAuMsgGetFdbEntryIndex(devObjPtr,msgWordPtr,&fdbIndex, &entryOffset);

    if (status == GT_OK)
    {
        /* entry found */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* Get FDB entry */
            entryPtr = SMEM_SIP6_HIDDEN_FDB_PTR(devObjPtr, fdbIndex);
        }
        else
        {
            /* Get FDB entry */
            entryPtr = smemMemGet(devObjPtr, SMEM_CHT_MAC_TBL_MEM(devObjPtr, fdbIndex));
        }

        /* Create update message */
        sfdbChtFdb2AuMsg(devObjPtr, fdbIndex, entryPtr, &macUpdMsg[0]);
    }
    else
    {
        /* entry was not found , need to use the info from the original AU message */
        memcpy(&macUpdMsg[0],msgWordPtr,4*SMEM_CHT_AU_MSG_WORDS);

        /* so we take the mac,vlan (or IPs,vlan) */
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* Message ID always 0x0 */
        fldValue =  0;
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID,
            fldValue);

        /* Message Type */
        fldValue = SFDB_UPDATE_MSG_QR_E;
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE,
            fldValue);

        /* Entry was found */
        fldValue = (status == GT_OK) ? 1 : 0;
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND,
            fldValue);

        fldValue = entryOffset;
        /* MAC address offset */
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET,
            fldValue);
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Message ID always 0x0 */
        fldValue =  0;
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID,
            fldValue);

        /* Message Type */
        fldValue = SFDB_UPDATE_MSG_QR_E;
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE,
            fldValue);

        /* Entry was found */
        fldValue = (status == GT_OK) ? 1 : 0;
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND,
            fldValue);

        fldValue = entryOffset;
        /* MAC address offset */
        SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsg,
            SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET,
            fldValue);
    }
    else
    {
        /* Message ID for Cht is 0x2 and for Cht2 is 0x0 (ignore DASecurity Level)*/
        fdbMsgId =  (!SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)) ? 0 : 2;
        SMEM_U32_SET_FIELD(macUpdMsg[0], 0, 4, fdbMsgId);

        /* Message Type */
        fldValue = SFDB_UPDATE_MSG_QR_E;
        SMEM_U32_SET_FIELD(macUpdMsg[0], 4, 3, fldValue);

        /* Entry was found */
        fldValue = (status == GT_OK) ? 1 : 0;
        SMEM_U32_SET_FIELD(macUpdMsg[0], 15, 1, fldValue);

        fldValue = entryOffset;
        /* MAC address offset */
        SMEM_U32_SET_FIELD(macUpdMsg[0], 7, 5, fldValue);
    }

    /* Send MAC update message to CPU */
    while(GT_FALSE == snetChtL2iSendUpdMsg2Cpu(devObjPtr, &macUpdMsg[0]))
    {
        if(oldWaitDuringSkernelTaskForAuqOrFua)
        {
            /* wait for SW to free buffers */
            SIM_OS_MAC(simOsSleep)(SNET_CHEETAH_NO_FDB_ACTION_BUFFERS_SLEEP_TIME);
        }
        else
        {
            devObjPtr->needResendMessage = 1;
            return status;/* this status is not relevant because ,
                             caller will not use it */
        }
    }

    __LOG(("Interrupt: AU_PROC_COMPLETED \n"));

    /* Processing of an AU Message received by the device is completed */
    snetChetahDoInterrupt(devObjPtr,
                      SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr),
                      SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr),
                      SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                        SMEM_LION3_AU_PROC_COMPLETED_INT :
                      SMEM_CHT_AU_PROC_COMPLETED_INT,
                      SMEM_CHT_FDB_SUM_INT(devObjPtr));

    return status;
}

typedef struct{
    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ENT      fdbField;
    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ENT   auMsgField;
}SIP6_FDB_AU_MSG_CONVERT_INFO;
/* array to convert fields from the FDB format to AU_MSG format and vice versa */
static SIP6_FDB_AU_MSG_CONVERT_INFO sip6FdbAuMsgConvertInfoArr[] = {
    /* common fields */
     {SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP                 }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE       }
    /* MAC entry / IPMC specifics */
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FID                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FID                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DEV_ID                   ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DEV_ID               }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL          ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL      }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DIP                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DIP                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_26_0                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_26_0             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL          ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL      }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_11_7       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_6_1        }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_0          }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_7           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_7       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_6_3            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_6_3        }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_2_1            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_2_1        }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SOURCE_ID_0              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_0          }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_10_7        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_10_7    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_0           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_0       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_4_1         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_4_1     }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_USER_DEFINED_6_5         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_6_5     }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_30_27                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_30_27            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VIDX                     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VIDX                 }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_TRUNK                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_TRUNK_NUM                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_EPORT_NUM                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SIP_31                   ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SIP_31               }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MULTIPLE                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_IS_STATIC                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_CMD                   ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_CMD               }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SA_CMD                   ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_CMD               }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_DA_ROUTE                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE}
    ,{SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_EPG_STREAM_ID         ,SMEM_SIP6_30_FDB_AU_MSG_TABLE_FIELDS_MAC_MC_IP_EPG_STREAM_ID    }

    /****************/
    /* MUST be last */
    /****************/
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E  }
} ;

/* array to convert fields from the FDB format to AU_MSG format and vice versa - when interface is EPORT/DEV */
static SIP6_FDB_AU_MSG_CONVERT_INFO sip6FdbAuMsgUcRoutePortConvertInfoArr[] = {
    /* common fields */
     {SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP                 }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE       }
    /* UC route specifics */
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID                                  ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID                              }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                       ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                   }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX                                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK                            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV                                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE                       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_POINTER                            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ECMP_POINTER                              }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                          ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                      }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                     }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR                                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR                             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                             ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                         }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                          }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                          }

    ,{SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_EPG_STREAM_ID                                 ,SMEM_SIP6_30_FDB_AU_MSG_TABLE_FIELDS_UC_EPG_STREAM_ID                          }

    /****************/
    /* MUST be last */
    /****************/
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E  }
};

/* array to convert fields from the FDB format to AU_MSG format and vice versa - when interface is TRUNK */
static SIP6_FDB_AU_MSG_CONVERT_INFO sip6FdbAuMsgUcRouteTrunkConvertInfoArr[] = {
    /* common fields */
     {SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP                 }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE       }
    /* UC route specifics */
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID                                  ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID                              }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                       ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                   }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX                                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK                            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID                            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_POINTER                            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ECMP_POINTER                              }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                          ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                      }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                     }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR                                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR                             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                             ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                         }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                          }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                          }
    ,{SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_EPG_STREAM_ID                                 ,SMEM_SIP6_30_FDB_AU_MSG_TABLE_FIELDS_UC_EPG_STREAM_ID                          }


    /****************/
    /* MUST be last */
    /****************/
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E  }
};

/* array to convert fields from the FDB format to AU_MSG format and vice versa - when interface is VIDX */
static SIP6_FDB_AU_MSG_CONVERT_INFO sip6FdbAuMsgUcRouteVidxConvertInfoArr[] = {
    /* common fields */
     {SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP                 }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE       }
    /* UC route specifics */
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID                                  ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID                              }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                        ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_127_106                    }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                       ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX                   }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN       }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX                                ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX                            }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX                                   ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX                               }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_POINTER                            ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ECMP_POINTER                              }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                          ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN                      }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL                     }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR                                 ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR                             }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                             ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE                         }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR                          }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                               ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT                           }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                              ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE                          }

    ,{SMEM_SIP6_30_FDB_FDB_TABLE_FIELDS_EPG_STREAM_ID                                 ,SMEM_SIP6_30_FDB_AU_MSG_TABLE_FIELDS_UC_EPG_STREAM_ID                          }

    /****************/
    /* MUST be last */
    /****************/
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E  }
};


/* array to convert fields from the FDB format to AU_MSG format and vice versa */
static SIP6_FDB_AU_MSG_CONVERT_INFO sip6FdbAuMsgIpv6KeyConvertInfoArr[] = {
    /* common fields */
     {SMEM_SIP6_FDB_FDB_TABLE_FIELDS_VALID                    ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_VALID                }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_SKIP                     ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_SKIP                 }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_AGE                      ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_AGE                  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE           ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE       }

    /* Ipv6 key specifics */
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0    ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0   }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32   ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_63_32  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64   ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_95_64  }
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96  ,SMEM_SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_105_96 }

    /****************/
    /* MUST be last */
    /****************/
    ,{SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E         ,SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E  }
};



/**
* @internal sip6FdbEntryConvertToAuMsg function
* @endinternal
*
* @brief   Sip6 : Create AU message from FDB entry
*/
static GT_VOID sip6FdbEntryConvertToAuMsg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  fdbIndex,
    IN GT_U32                * fdbEntryPtr,
    OUT GT_U32               * macUpdMsgPtr
)
{
    GT_U32           fldValue;        /* Word's field value */
    SIP6_FDB_AU_MSG_CONVERT_INFO    *convertPtr = NULL;
    SNET_CHEETAH_FDB_ENTRY_ENT       macEntryType;
    GT_U32          useVidx;
    GT_U32          isTrunk;

    memset(macUpdMsgPtr, 0, 4 * SMEM_CHT_AU_MSG_WORDS);

    /* MessageID Always set to 0x0 */
    fldValue = 0;
    SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID,
        fldValue);

    /* fdb entry type */
    macEntryType = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
        fdbIndex,
        SMEM_SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE);

    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:/*IPv4 Multicast address entry (IGMP snooping);*/
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:/*IPv6 Multicast address entry (MLD snooping);*/
            convertPtr = &sip6FdbAuMsgConvertInfoArr[0];
            break;
        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:
            /* is vidx */
            useVidx = SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX);
            if(useVidx)
            {
                convertPtr = &sip6FdbAuMsgUcRouteVidxConvertInfoArr[0];
            }
            else
            {
                /* is dev/ePort or Trunk */
                isTrunk = SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    SMEM_SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK);
                if(isTrunk)
                {
                    convertPtr = &sip6FdbAuMsgUcRouteTrunkConvertInfoArr[0];
                }
                else
                {
                    convertPtr = &sip6FdbAuMsgUcRoutePortConvertInfoArr[0];
                }
            }
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E:
            convertPtr = &sip6FdbAuMsgIpv6KeyConvertInfoArr[0];
            break;

        default:
            skernelFatalError("wrong entry type given\n");
            return;
    }


    for(/*already init*/; convertPtr->fdbField != SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E; convertPtr++)
    {
        if(convertPtr->fdbField == SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR)
        {
            GT_U32  valueArr[2];
            /* get value from 'FDB format' */
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                valueArr);
            /* set value to 'AU_MSG format' */
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_SET(devObjPtr,macUpdMsgPtr,
                valueArr);


            continue;
        }

        /* get value from 'FDB format' */
        fldValue = SMEM_SIP6_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
            fdbIndex,
            convertPtr->fdbField);
        /* set value to 'AU_MSG format' */
        SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
            convertPtr->auMsgField,
            fldValue);
    }

}

/**
* @internal sip6AuMsgConvertToFdbEntry function
* @endinternal
*
* @brief   SIP6 : Create FDB entry from AU message
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbIndex                 - index in the FDB
* @param[in] msgWordPtr               - MAC update message pointer
*
* @param[out] macEntry                 - FDB entry pointer
*                                      replace lion3AuMsgConvertToFdbEntry
*/
static GT_STATUS sip6AuMsgConvertToFdbEntry
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 fdbIndex,
    IN GT_U32                *msgWordPtr,
    OUT GT_U32               *macEntry
)
{
    DECLARE_FUNC_NAME(sip6AuMsgConvertToFdbEntry);

    GT_U32           fldValue;        /* Word's field value */
    SIP6_FDB_AU_MSG_CONVERT_INFO    *convertPtr = NULL;
    SNET_CHEETAH_FDB_ENTRY_ENT       macEntryType;
    GT_U32          useVidx;
    GT_U32          isTrunk;

    /* fdb entry type */
    macEntryType = SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
        SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE);

    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:/*IPv4 Multicast address entry (IGMP snooping);*/
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:/*IPv6 Multicast address entry (MLD snooping);*/
            convertPtr = &sip6FdbAuMsgConvertInfoArr[0];
            break;
        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:
            /* is vidx */
            useVidx = SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX);
            if(useVidx)
            {
                convertPtr = &sip6FdbAuMsgUcRouteVidxConvertInfoArr[0];
            }
            else
            {
                /* is dev/ePort or Trunk */
                isTrunk = SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
                    SMEM_SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK);
                if(isTrunk)
                {
                    convertPtr = &sip6FdbAuMsgUcRouteTrunkConvertInfoArr[0];
                }
                else
                {
                    convertPtr = &sip6FdbAuMsgUcRoutePortConvertInfoArr[0];
                }
            }
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:
            __LOG(("ipv6 'not supported' (the device not support 'by message' the ipv6 entries)"));
            return GT_FAIL;
        default:
            skernelFatalError("wrong entry type given\n");
            return GT_FAIL;
    }


    for(/*already init*/; convertPtr->fdbField != SMEM_SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E; convertPtr++)
    {
        if(convertPtr->fdbField == SMEM_SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR)
        {
            GT_U32  valueArr[2];
            /* get value from 'AU_MSG format' */
            SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,msgWordPtr,
                valueArr);
            /* set value to 'FDB format' */
            SMEM_SIP6_FDB_FDB_ENTRY_FIELD_MAC_ADDR_SET(devObjPtr,macEntry,
                fdbIndex,
                valueArr);

            continue;
        }

        /* get value from 'AU_MSG format' */
        fldValue = SMEM_SIP6_FDB_AU_MSG_ENTRY_FIELD_GET(devObjPtr,msgWordPtr,
            convertPtr->auMsgField);
        /* set value to 'FDB format' */
        SMEM_SIP6_FDB_FDB_ENTRY_FIELD_SET(devObjPtr,macEntry,
            fdbIndex,
            convertPtr->fdbField,
            fldValue);
    }

    return GT_OK;
}


/**
* @internal lion3FdbEntryConvertToAuMsg function
* @endinternal
*
* @brief   Lion3 : Create AU message from FDB entry
*/
static GT_VOID lion3FdbEntryConvertToAuMsg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  fdbIndex,
    IN GT_U32                * fdbEntryPtr,
    OUT GT_U32               * macUpdMsgPtr
)
{
    GT_U32                                  fldValue;        /* Word's field value */
    GT_U32                                  valueArr[4] = {0};
    SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC specialFields;
    GT_U32                                  macAddrWords[2];
    SNET_CHEETAH_FDB_ENTRY_ENT              macEntryType;

    ASSERT_PTR(fdbEntryPtr);
    ASSERT_PTR(macUpdMsgPtr);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("lion3FdbEntryConvertToAuMsg : function not for sip6 devices \n");
        return;
    }

    memset(macUpdMsgPtr, 0, 4 * SMEM_CHT_AU_MSG_WORDS);
    memset(&specialFields,0,sizeof(SFDB_LION3_FDB_SPECIAL_MUXED_FIELDS_STC));

    /* fdb entry type */
    macEntryType = SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE);

    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE,
        macEntryType);


    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_MAC_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV4_IPMC_BRIDGING_E:/*IPv4 Multicast address entry (IGMP snooping);*/
        case SNET_CHEETAH_FDB_ENTRY_IPV6_IPMC_BRIDGING_E:/*IPv6 Multicast address entry (MLD snooping);*/

            /* srcId          */
            /* udb            */
            /* origVid1       */
            /* daAccessLevel  */
            /* saAccessLevel  */
            sfdbLion3FdbSpecialMuxedFieldsGet(devObjPtr,fdbEntryPtr,fdbIndex,
                &specialFields);
            sfdbLion3FdbAuMsgSpecialMuxedFieldsSet(devObjPtr,macUpdMsgPtr,macEntryType,
                &specialFields);

            /* MAC ADDR */
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_MAC_ADDR_GET(devObjPtr,
                fdbEntryPtr,
                fdbIndex,
                macAddrWords);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_MAC_ADDR_SET(devObjPtr,
                macUpdMsgPtr,
                macAddrWords);

            /* VID */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_FID);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_FID,
                fldValue);

            /* Multiple */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_MULTIPLE);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE,
                fldValue);

            /* SPUnknown */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN,
                fldValue);

            /* VIDX/Port */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_VIDX);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_VIDX,
                fldValue);

            /* add application specific code */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE,
                fldValue);

            /* DevNum - the device number associated with this message */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_DEV_ID);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_DEV_ID,
                fldValue);

            /* FDB Entry Static Bit */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_IS_STATIC);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC,
                fldValue);

            break;

        /*****************************************************************/
        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:

            /* ipv4 dip */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV4_DIP);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP,
                fldValue);
            break;

        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:
            /* fcoe d_id */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID,
                fldValue);
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_KEY_E:

            /* ipv6 dip */
            SMEM_LION3_FDB_FDB_ENTRY_FIELD_IPV6_DIP_GET(devObjPtr,
                fdbEntryPtr,
                fdbIndex,
                valueArr);

            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_0,
                valueArr[0]);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_1,
                valueArr[1]);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_2,
                valueArr[2]);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_3,
                valueArr[3]);

            /* NH_DATA_BANK_NUM */
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NH_DATA_BANK_NUM,
                fdbIndex % devObjPtr->fdbNumOfBanks );
            break;

        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:
            break;

        default:
            skernelFatalError("wrong entry type given\n");
            break;
    }

    /* MessageID Always set to 0x0 */
    fldValue = 0;
    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID,
        fldValue);

    /* Age */
    fldValue =
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_AGE);
    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_AGE,
        fldValue);

    /* Skip */
    fldValue =
        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
        fdbIndex,
        SMEM_LION3_FDB_FDB_TABLE_FIELDS_SKIP);
    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_SKIP,
        fldValue);


    /* FDB ipv4 / fcoe / ipv6 data  UC Routing common fields */
    switch(macEntryType)
    {
        case SNET_CHEETAH_FDB_ENTRY_IPV4_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_FCOE_UC_ROUTING_E:
        case SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E:

            /* valid */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_VALID);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_VALID,
                fldValue);

            /* vrf_id */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_VRF_ID);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID,
                fldValue);

           if(macEntryType == SNET_CHEETAH_FDB_ENTRY_IPV6_UC_ROUTING_DATA_E)
            {
                /* ipv6 scope check */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK,
                    fldValue);

                /* ipv6 dst site id */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_IPV6_DEST_SITE_ID);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID,
                    fldValue);
            }

            /* dec ttl */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT,
                fldValue);

            /* bypass ttl */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION,
                fldValue);

            /* Ingress Mirror to Analyzer Index */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX,
                fldValue);

            /* Qos profile marking en */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN,
                fldValue);

            /* Qos profile index */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX,
                fldValue);

            /* Qos profile precedence */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE,
                fldValue);

            /* Modify UP */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MODIFY_UP);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_UP,
                fldValue);

            /* Modify DSCP */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP,
                fldValue);

            /* Counter Set index */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX,
                fldValue);

            /* Arp bc trap mirror en */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN,
                fldValue);

            /* Dip access level */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL,
                fldValue);

            /* Ecmp */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEPTION_MIRROR);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN,
                fldValue);

            /* Mtu Index */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_MTU_INDEX);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX,
                fldValue);

            /* Use Vidx*/
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_USE_VIDX);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX,
                fldValue);

            if(0 == fldValue) /* useVidx = false*/
            {
                /* Is trunk*/
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_IS_TRUNK);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK,
                    fldValue);

                if(fldValue)/*IsTrunk=true*/
                {
                    /* Trunk num */
                    fldValue =
                        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_TRUNK_ID);
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM,
                        fldValue);
                }
                else /* not trunk */
                {
                    /* Target Device */
                    fldValue =
                        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_DEV);
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE,
                        fldValue);

                    /* Eport num */
                    fldValue =
                        SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                        fdbIndex,
                        SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TRG_EPORT);
                    SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                        SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM,
                        fldValue);
                }
            }
            else /* useVidx true */
            {
                /* Evidx */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_EVIDX);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX,
                    fldValue);
            }

            /* Next Hop Evlan */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN,
                fldValue);

            /* Tunnel Start */
            fldValue =
                SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                fdbIndex,
                SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL);
            SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_START,
                fldValue);

            if(fldValue)/* tunnelStart=true */
            {
                /* Tunnel Type */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE,
                    fldValue);

                /* Tunnel Ptr */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR,
                    fldValue);
            }
            else /* not tunnel */
            {
                /* Arp Ptr */
                fldValue =
                    SMEM_LION3_FDB_FDB_ENTRY_FIELD_GET(devObjPtr,fdbEntryPtr,
                    fdbIndex,
                    SMEM_LION3_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ARP_PTR);
                SMEM_LION3_FDB_AU_MSG_ENTRY_FIELD_SET(devObjPtr,macUpdMsgPtr,
                    SMEM_LION3_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR,
                    fldValue);
            }

            break;

        default:
            break;
    }

}


/**
* @internal sfdbChtFdb2AuMsg function
* @endinternal
*
* @brief   Create AU message from FDB entry
*/
static GT_VOID sfdbChtFdb2AuMsg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  fdbIndex,
    IN GT_U32                * fdbEntryPtr,
    OUT GT_U32               * macUpdMsgPtr
)
{
    GT_U32 fldValue;                        /* Word's field value */

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        sip6FdbEntryConvertToAuMsg(devObjPtr,fdbIndex,fdbEntryPtr,macUpdMsgPtr);

        return;
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        lion3FdbEntryConvertToAuMsg(devObjPtr,fdbIndex,fdbEntryPtr,macUpdMsgPtr);

        return;
    }

    ASSERT_PTR(macUpdMsgPtr);


    memset(macUpdMsgPtr, 0, 4 * SMEM_CHT_AU_MSG_WORDS);

    if (SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr))
    {   /* MessageID Always set to 0x2 , on cht */
        SMEM_U32_SET_FIELD(macUpdMsgPtr[0], 0, 4, 2);
    }
    else
    {   /* MessageID Always set to 0x0 , on cht2 */
        SMEM_U32_SET_FIELD(macUpdMsgPtr[0], 0, 1, 0);
        /* DA security level */
        fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[3], 11, 3);
        SMEM_U32_SET_FIELD(macUpdMsgPtr[0], 1, 3, fldValue);
        /* SA security level */
        fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[3], 14, 3);
        SMEM_U32_SET_FIELD(macUpdMsgPtr[0], 12, 3, fldValue);
    }
    /* MacAddr[14:0] - the lower 16 bits of the mac addr*/
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[0], 17, 15);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[0], 16, 15, fldValue);
    /* MacAddr[15] - continue of the lower 16 bits lower */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[1], 0, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[0], 31, 1, fldValue);

    /* MacAddr[46:16] , the upper 32 bits of the mac addr */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[1], 1, 31);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[1], 0, 31, fldValue);
    /* MacAddr[47] , continue with the upper 32 bits of the mac addr */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[2], 0, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[1], 31, 1, fldValue);

    /* VID */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[0], 5, 12);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 0, 12, fldValue);

    /* Multiple */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[2], 26, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 15, 1, fldValue);

    /* SPUnknown */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[3], 2, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 14, 1, fldValue);

    /* Age */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[0], 2, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 13, 1, fldValue);

    /* Skip */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[0], 1, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 12, 1, fldValue);

    /* VIDX/Port */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[2], 13, 12);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 17, 12, fldValue);
    if (!SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr))
    {   /* add application specific code */
        fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[3], 10, 1);
        SMEM_U32_SET_FIELD(macUpdMsgPtr[2], 29, 1, fldValue);
    }

    /* SrcID , the source id associated with this message */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[2], 6, 5);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[3], 2, 5, fldValue);
    /* DevNum - the device number associated with this message */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[2], 1, 5);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[3], 7, 5, fldValue);

    /* FDB Entry Static Bit */
    fldValue = SMEM_U32_GET_FIELD(fdbEntryPtr[2], 25, 1);
    SMEM_U32_SET_FIELD(macUpdMsgPtr[3], 18, 1, fldValue);

}

/* use value 0 to keep using the value from the register */
GT_U32  numOfEntries_debug = 256;
/* debug function */
GT_STATUS sfdbChtAutoAgingNumOfEntries_debug(GT_U32 new_numOfEntries_debug)
{
    numOfEntries_debug = new_numOfEntries_debug;

    return GT_OK;
}

/**
* @internal sfdbChtAutoAging function
* @endinternal
*
* @brief   Age out MAC table entries.
*/
void sfdbChtAutoAging
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtAutoAging);

    SKERNEL_DEVICE_OBJECT * devObjPtr = deviceObjPtr;/*needed for the __LOG*/
    GT_U32  regValue;                 /* register data */
    GT_U32  fldValue;                 /* field value of register */
    GT_U32  agingTime;                /* aging time in milliseconds */
    GT_U32  ActionTimer;              /* field <ActionTimer> in HW */
    GT_U32 entryIndex = 0 ;
    GT_U32 numOfEntries = 0;
    SBUF_BUF_ID bufferId;             /* buffer */
    GT_U32 * dataPtr;                /* pointer to the data in the buffer */
    GT_U32 dataSize;                  /* data size */
    GT_U32 timeOut;
    GT_U32 totalEntries;
    GT_U8  *dataU8Ptr;                   /* pointer to the data in the buffer */
    GT_U32  regAddr;

    GT_U32  coreClk;        /* the core clock of the device */
    GT_U32  fdbBaseCoreClock; /* FDB usage of the reference core clock for the device */
    GT_U32  fdbAgingFactor; /* ch3 - 16 sec , ch1,2,xcat,2,lion,2 = 10 sec, bobcat2,lion3 */
    GT_U32  FDBAgingWindowSizeRegAddr;/* FDB Aging Window Size Register Address */
    GT_U32  currentSleepTime = 0;
    GT_U32 tileId = FDB_TILE_ID_GET(deviceObjPtr);

    /* Sleep to finish init of the simulation */
    /* (try to) create some kind of divert between the different port groups */
    deviceObjPtr = skernelSleep(deviceObjPtr,5000 + ((deviceObjPtr->portGroupId+tileId) * 111));

    if(SMEM_CHT_IS_SIP5_GET(deviceObjPtr))
    {
        regAddr = SMEM_LION3_FDB_FDB_ACTION_GENERAL_REG(deviceObjPtr);
        FDBAgingWindowSizeRegAddr = SMEM_LION3_FDB_AGING_WINDOW_SIZE_REG(deviceObjPtr);
    }
    else
    {
        regAddr = SMEM_CHT_MAC_TBL_ACTION0_REG(deviceObjPtr);
        FDBAgingWindowSizeRegAddr = 0;/* not used */
    }

    do
    {
        if(deviceObjPtr->portGroupSharedDevObjPtr)
        {
            coreClk = deviceObjPtr->portGroupSharedDevObjPtr->coreClk;
        }
        else
        {
            coreClk = deviceObjPtr->coreClk;
        }

        if(coreClk == 0)
        {
            /* device is not initialized yet -- try again later ... */
            deviceObjPtr = skernelSleep(deviceObjPtr,100);
        }
    }while(coreClk == 0); /* we wait while init part 2 is done ... we need fdbBaseCoreClock,fdbAgingFactor*/

    if(deviceObjPtr->portGroupSharedDevObjPtr)
    {
        fdbBaseCoreClock  = deviceObjPtr->portGroupSharedDevObjPtr->fdbBaseCoreClock;
        fdbAgingFactor    = deviceObjPtr->portGroupSharedDevObjPtr->fdbAgingFactor  ;
    }
    else
    {
        fdbBaseCoreClock  = deviceObjPtr->fdbBaseCoreClock;
        fdbAgingFactor    = deviceObjPtr->fdbAgingFactor  ;
    }

    while(1)
    {
        if(currentSleepTime)
        {
            /* the sleep function may replace the device object !!! due to soft reset */
            deviceObjPtr = skernelSleep(deviceObjPtr,currentSleepTime);

            /* set task type - only after SHOSTG_psos_reg_asic_task */
            /* support possible change of the device during 'soft reset' */
            SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
                SIM_OS_TASK_PURPOSE_TYPE_PP_AGING_DAEMON_E,
                &deviceObjPtr->task_sagingCookieInfo[tileId].generic);
        }

        /* check that automatic aging enabled and active by read automatic field */
        smemRegGet(deviceObjPtr, regAddr, &regValue);
        /*ActEn */
        fldValue = SMEM_U32_GET_FIELD(regValue, 0, 1);
        if(fldValue == 0)
        {
            /* daemon is not active -- try again later ... */
            currentSleepTime = 1000;
            continue;
        }
        /*AgingTrigger */
        fldValue = SMEM_U32_GET_FIELD(regValue, 1, 1);
        if(fldValue)
        {
            /* daemon is BUSY -- try again later ... */
            currentSleepTime = 1000;
            continue;
        }
        /*TriggerMode */
        fldValue = SMEM_U32_GET_FIELD(regValue, 2, 1);
        if(fldValue)
        {
            /* trigger aging */
            currentSleepTime = 1000;
            /* automatic aging is currently disabled
                                   sleep for 2 seconds and then check again if
                                   automatic aging is enabled again */
            continue;
        }
        /*ActionMode */
        fldValue = SMEM_U32_GET_FIELD(regValue, 3, 2);

        /* if TriggerMode > 1 than automatic aging disabled */
        if (fldValue > 1)
        {
            currentSleepTime = 1000;
            /* automatic aging is currently disabled
                                   sleep for 2 seconds and then check again if
                                   automatic aging is enabled again */
            continue;
        }

        numOfEntries = numOfEntries_debug;

        if(deviceObjPtr->fdbAgingDaemonInfo[tileId].daemonOnHold)
        {
            currentSleepTime = 100;
            /* the daemon is on hold (AUQ is full) */
            continue;
        }

        if(deviceObjPtr->fdbNumEntries == 0)
        {
            /* can be in sip6.30 when FDB used only by the DDE for HSR-PRP */
            __LOG(("FDB size is ZERO !!! no action supported \n"));
            currentSleepTime = 1000;
            continue;
        }

        /* Get buffer */
        bufferId = sbufAlloc(deviceObjPtr->bufPool, SFDB_AUTO_AGING_BUFF_SIZE);
        if (bufferId == NULL)
        {
            simWarningPrintf(" sfdbChtAutoAging: no buffers for automatic aging \n");
            currentSleepTime = 1000;
            /* currently no buffers for aging
                                   sleep for 2 seconds and then try again ,
                                   maybe there will be buffers free then */
            continue;
        }

        /* Get actual data pointer */
        sbufDataGet(bufferId, (GT_U8**)&dataU8Ptr, &dataSize);
        dataPtr = (GT_U32*)dataU8Ptr;


        /* copy MAC table action 1 word to buffer */
        memcpy(dataPtr, &entryIndex , sizeof(GT_U32) );
        dataPtr++;
        memcpy(dataPtr, &numOfEntries , sizeof(GT_U32) );

        /* set source type of buffer */
        bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

        /* set message type of buffer */
        bufferId->dataType = SMAIN_CPU_FDB_AUTO_AGING_E;

        if(deviceObjPtr->numOfPipes > 2)
        {
            /* needed in Falcon to identify the 'TileId' */
            bufferId->pipeId = smemGetCurrentPipeId(deviceObjPtr);
        }
        /* put buffer to queue */
        squeBufPut(deviceObjPtr->queueId, SIM_CAST_BUFF(bufferId));

        totalEntries =  entryIndex + numOfEntries;
        entryIndex = totalEntries % deviceObjPtr->fdbNumEntries ;
        /* get aging time in seconds * 10 */
        /* ActionTimer  */
        ActionTimer = SMEM_U32_GET_FIELD(regValue, 5, 6);
        if (ActionTimer == 0)
            ActionTimer++;

        if(FDBAgingWindowSizeRegAddr)
        {
            /*aging_end_counter_val*/
            smemRegGet(deviceObjPtr, FDBAgingWindowSizeRegAddr, &regValue);

            if(SMEM_CHT_IS_SIP5_10_GET(deviceObjPtr))
            {
                fdbAgingFactor = _512K;
            }
            else
            {
                fdbAgingFactor = (_256K / POWER_2(_256K/deviceObjPtr->fdbNumEntries));
            }

            /* result in milliseconds */
            /* NOTE: order of dividing is important to not overflow the GT_U32 calculations */
            agingTime = (((ActionTimer * regValue)/coreClk))  * (fdbAgingFactor / (_1_MILLION / 1000/*milliseconds*/));
        }
        else
        {
            /* in milliseconds */
            agingTime = ((fdbAgingFactor * 1000/*milliseconds*/) * ActionTimer * fdbBaseCoreClock) /
                        coreClk;
        }

        timeOut = (agingTime * numOfEntries) / deviceObjPtr->fdbNumEntries ;

        currentSleepTime = timeOut;
    }
}

/**
* @internal sfdbCht2MacTableUploadAction function
* @endinternal
*
* @brief   MAC Table Trigger Action
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tblActPtr                - pointer table action data
*/
GT_VOID sfdbCht2MacTableUploadAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * tblActPtr
)
{
    DECLARE_FUNC_NAME(sfdbCht2MacTableUploadAction);

    GT_U32 entry;                       /* MAC table entry index */
    GT_U32  *tblDataMemPtr = (GT_U32*)tblActPtr;
    AGE_DAEMON_ACTION_INFO_STC actionInfo;/* common info need for the 'Aging daemon' */
    GT_U32  regAddr;
    GT_BIT  needBreak = 0;
    GT_U32 tileId = FDB_TILE_ID_GET(devObjPtr);

    /* get common action info */
    sfdbChtActionInfoGet(devObjPtr,tblDataMemPtr,&actionInfo);

    entry = devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb;

    if(devObjPtr->fdbNumEntries == 0)
    {
        /* can be in sip6.30 when FDB used only by the DDE for HSR-PRP */
        __LOG(("FDB size is ZERO !!! no action supported \n"));
        return;
    }

    for (/**/; entry < devObjPtr->fdbNumEntries; entry++)
    {
        sfdbChtFdbActionEntry(devObjPtr,SFDB_CHEETAH_TRIGGER_UPLOAD_E,&actionInfo,entry,tileId,&needBreak);
        if(needBreak)
        {
            /* the action is broken and will be continued in new message */
            /* save the index */
            devObjPtr->needResendMessage = 1;
            devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = entry;
            return;
        }
        /* state that the daemon is NOT on hold */
        devObjPtr->fdbAgingDaemonInfo[tileId].daemonOnHold = 0;
    }

    devObjPtr->fdbAgingDaemonInfo[tileId].indexInFdb = 0;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        regAddr = SMEM_LION3_FDB_FDB_ACTION_GENERAL_REG(devObjPtr);
    }
    else
    {
        regAddr = SMEM_CHT_MAC_TBL_ACTION0_REG(devObjPtr);
    }

    /* Clear Aging Trigger */
    smemRegFldSet(devObjPtr, regAddr, 1, 1, 0);

    __LOG(("Interrupt: AGE_VIA_TRIGGER_END \n"));

    snetChetahDoInterrupt(devObjPtr,
                        SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr),
                        SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr),
                        SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                          SMEM_LION3_AGE_VIA_TRIGGER_END_INT :
                        SMEM_CHT_AGE_VIA_TRIGGER_END_INT,
                        SMEM_CHT_FDB_SUM_INT(devObjPtr));
}

/**
* @internal sfdbChtHashResultsRegistersUpdate function
* @endinternal
*
* @brief   SIP5 : update the hash result registers that relate to last AU message
*         from the CPU to the PP.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] calculatedHashArr[FDB_MAX_BANKS_CNS] - multi hash results :
*                                      array of calculated hash by the different functions
*                                      NULL means --> ignore the multi hash values
* @param[in] xorResult                -   xor result.
* @param[in] crcResult                -   CRC result.
*                                       None
*/
void sfdbChtHashResultsRegistersUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32       calculatedHashArr[FDB_MAX_BANKS_CNS],
    IN GT_U32       xorResult,
    IN GT_U32       crcResult
)
{
    GT_U32 regAddr;                         /* register address */
    GT_U32 regValue;
    GT_U32 ii;

    regValue = 0;

    if(calculatedHashArr)
    {
        for(ii = 0 ; ii < devObjPtr->fdbNumOfBanks; ii++)
        {
            SMEM_U32_SET_FIELD(regValue,16*(ii & 1),16,calculatedHashArr[ii]);

            if((ii & 1) == 1)
            {
                regAddr = SMEM_LION3_FDB_MULTI_HASH_CRC_RESULT_REG(devObjPtr,(ii / 2));
                smemRegSet(devObjPtr,regAddr,regValue);

                regValue = 0;
            }
        }
    }

    regAddr = SMEM_LION3_FDB_NON_MULTI_HASH_CRC_RESULT_REG(devObjPtr);
    smemRegSet(devObjPtr,regAddr,crcResult);

    regAddr = SMEM_LION3_FDB_NON_MULTI_HASH_XOR_HASH_REG(devObjPtr);
    smemRegSet(devObjPtr,regAddr,xorResult);

    return;
}

/**
* @internal sfdbChtHrMsgProcess function
* @endinternal
*
* @brief   SIP5 : Hash Request (HR) - called by the CPU to calculate all the HASH
*         functions that the devices can generate :
*         CRC , XOR , 16 CRC multi-hash.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] msgWordPtr               - pointer to fdb message.
*/
static GT_STATUS sfdbChtHrMsgProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                * msgWordPtr
)
{
    DECLARE_FUNC_NAME(sfdbChtHrMsgProcess);

    GT_U32 tblSize = devObjPtr->fdbNumEntries;
    GT_U32 numBitsToUse = SMEM_CHT_FDB_HASH_NUM_BITS(tblSize);
    GT_U32 calculatedHashArr[FDB_MAX_BANKS_CNS];
    enum{legacy_CRC = 1,legacy_XOR = 0, new_multi__CRC=0xFFF};
    GT_U32  vlanMode;
    GT_U32 macGlobCfgRegData;       /* Global register configuration data */
    SNET_CHEETAH_FDB_ENTRY_HASH_INFO_STC    entryHashInfo;
    GT_STATUS       status;
    GT_U32       xorResult;
    GT_U32       crcResult;

    status = parseAuMsgGetFdbEntryHashInfo(devObjPtr,msgWordPtr,&entryHashInfo);
    if(status != GT_OK)
    {
        return status;
    }

    /* Get data from MAC Table Global Configuration Register */
    smemRegGet(devObjPtr, SMEM_CHT_MAC_TBL_GLB_CONF_REG(devObjPtr), &macGlobCfgRegData);

    vlanMode  = SMEM_U32_GET_FIELD(macGlobCfgRegData, 3, 1);

    /* get multi hash CRC calculation */
    sip5MacHashCalcMultiHash(devObjPtr,new_multi__CRC, vlanMode, &entryHashInfo,numBitsToUse,
                devObjPtr->fdbNumOfBanks,calculatedHashArr);

    /* get legacy (single) hash CRC calculation */
    crcResult = cheetahMacHashCalcByStc(legacy_CRC, vlanMode, &entryHashInfo,numBitsToUse);
    /* the results come from the function as 'FDB index' */
    crcResult /= 4;

    /* get legacy (single) hash XOR calculation */
    xorResult = cheetahMacHashCalcByStc(legacy_XOR, vlanMode, &entryHashInfo,numBitsToUse);
    /* the results come from the function as 'FDB index' */
    xorResult /= 4;

    /* write the results to the registers */
    sfdbChtHashResultsRegistersUpdate(devObjPtr,calculatedHashArr,xorResult,crcResult);

    __LOG(("Interrupt: AU_PROC_COMPLETED \n"));

    /* Processing of an AU Message received by the device is completed */
    snetChetahDoInterrupt(devObjPtr,
                      SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr),
                      SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr),
                      SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                        SMEM_LION3_AU_PROC_COMPLETED_INT :
                      SMEM_CHT_AU_PROC_COMPLETED_INT,
                      SMEM_CHT_FDB_SUM_INT(devObjPtr));

    return GT_OK;
}

/**
* @internal sfdbChtBankCounterAction function
* @endinternal
*
* @brief   SIP5 : the AU message from the CPU need to be checked if to increment/decrement
*         the counters of the relevant FDB bank
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fdbIndex                 - FDB index that relate to the entry that may have bank counter action
* @param[in] counterAction            - the bank counter action.
* @param[in] client                   - triggering client
*                                       None.
*/
void sfdbChtBankCounterAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   fdbIndex,
    IN SFDB_CHT_BANK_COUNTER_ACTION_ENT     counterAction,
    IN SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_ENT  client
)
{
    DECLARE_FUNC_NAME(sfdbChtBankCounterAction);

    GT_U32  bankIndex; /*index of the bank --> calculated from fdbIndex */
    GT_CHAR *actionName;/*string (name) of action type */
    GT_CHAR *clientName;/*string (name) of client type */
    GT_BIT  overflowError = 0; /*indication for error - type overrun */
    GT_BIT  underflowError = 0;/*indication for error - type underrun */
    GT_U32  bankRankIndex;/* the rank of the bank */
    GT_U32  tmpBankIndex;/* temp - bank index */
    GT_U32  tmpCounter;  /* temp - bank counter */
    GT_U32  origBankRankIndex;/* the original rank of the bank */
    GT_U32  ii;/*iterator*/
    GT_U32  *counterPtr;/* pointer to the counter */
    GT_U32  *rankPtr;   /* pointer to the rank    */

    if((devObjPtr->multiHashEnable == 0) &&
       (SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_EXACT_BANK_E != client))
    {
        /* There is no bank management when working in none MULTI hash mode.
           Management works always by CPU direct update regardless of hash mode. */
        return;
    }

    switch(counterAction)
    {
        case SFDB_CHT_BANK_COUNTER_ACTION_NONE_E:
            return ;
        case SFDB_CHT_BANK_COUNTER_ACTION_INCREMENT_E:
            actionName = "INCREMENT";
            break;
        case SFDB_CHT_BANK_COUNTER_ACTION_DECREMENT_E:
            actionName = "DECREMENT";
            break;
        default:
            return ;
    }

    switch(client)
    {
        case SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_AU_MSG_E:
            clientName = "CPU_AU_MSG";
            break;
        case SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_CPU_EXACT_BANK_E:
            clientName = "CPU_EXACT_BANK";
            break;
        case SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_PP_AGING_DAEMON_E:
            clientName = "PP_AGING_DAEMON";
            break;
        case SFDB_CHT_BANK_COUNTER_UPDATE_CLIENT_PP_AUTO_LEARN_E:
            clientName = "PP_AUTO_LEARN";
            break;
        default:
            skernelFatalError("sfdbChtBankCounterAction: not supported client[%d] \n",client);
            return;
    }

    /* calculate the bank from the FDB index */
    bankIndex = fdbIndex % devObjPtr->fdbNumOfBanks;

    SCIB_SEM_TAKE;
    while(devObjPtr->fdbBankCounterInCpuPossess)
    {
        SCIB_SEM_SIGNAL;
        /* wait for CPU to release the counters */
        SIM_OS_MAC(simOsSleep)(50);
        SCIB_SEM_TAKE;

        if(devObjPtr->needToDoSoftReset)
        {
            /* we can not allow the thread to be stuck here */
            break;
        }
    }

    devObjPtr->fdbBankCounterUsed = 1;
    SCIB_SEM_SIGNAL;

    bankRankIndex = SMAIN_NOT_VALID_CNS;
    rankPtr = smemMemGet(devObjPtr,SMEM_LION3_FDB_BANK_RANKS_REG(devObjPtr,0));
    for( ii = 0 ; ii < devObjPtr->fdbNumOfBanks ; ii++)
    {
        if((rankPtr[ii]) == bankIndex)
        {
            bankRankIndex = ii;
            break;
        }
    }

    origBankRankIndex = bankRankIndex;

    counterPtr = smemMemGet(devObjPtr,SMEM_LION3_FDB_BANK_COUNTER_REG(devObjPtr,0));

    if(counterAction == SFDB_CHT_BANK_COUNTER_ACTION_INCREMENT_E)
    {
        if(counterPtr[bankIndex] >= (devObjPtr->fdbMaxNumEntries / devObjPtr->fdbNumOfBanks))
        {
            overflowError = 1;
        }
        else
        {
            counterPtr[bankIndex]++;

            /* re-rank the counters */
            /* check if counter's rank need to be 'upgrated' (swap with higher rank) */
            if((bankRankIndex != SMAIN_NOT_VALID_CNS) && (bankRankIndex != 0))
            {
                for( ii = (bankRankIndex - 1) ; /*break condition is not here*/ ; ii--)
                {
                    /* bank index of 'better ranked' bank */
                    tmpBankIndex = rankPtr[ii];
                    /* the counter of the 'better ranked' bank */
                    tmpCounter = counterPtr[tmpBankIndex];

                    if(counterPtr[bankIndex] > tmpCounter)
                    {
                        /*the counter of 'this' bank is higher then the counter of bank ranked higher*/
                        /*swap the ranks*/
                        rankPtr[ii] = bankIndex;
                        rankPtr[bankRankIndex] = tmpBankIndex;
                        bankRankIndex = ii;
                    }
                    else
                    {
                        /* no need to continue the search for better rank */
                        break;
                    }

                    if(ii == 0)
                    {
                        /* no more swaps can be */
                        break;
                    }
                }
            }/*rank check*/
        }
    }
    else
    {
        if(counterPtr[bankIndex] == 0)
        {
            underflowError = 1;
        }
        else
        {
            counterPtr[bankIndex]--;

            /* re-rank the counters */
            /* check if counter's rank need to be 'downgrated' (swap with lower rank) */
            if(bankRankIndex != SMAIN_NOT_VALID_CNS)
            {
                for( ii = bankRankIndex + 1 ; ii < devObjPtr->fdbNumOfBanks ; ii++)
                {
                    /* bank index of 'lower ranked' bank */
                    tmpBankIndex = rankPtr[ii];
                    /* the counter of the 'lower ranked' bank */
                    tmpCounter = counterPtr[tmpBankIndex];

                    if(counterPtr[bankIndex] < tmpCounter)
                    {
                        /*the counter of 'this' bank is lower then the counter of bank ranked lower */
                        /*swap the ranks*/
                        rankPtr[ii] = bankIndex;
                        rankPtr[bankRankIndex] = tmpBankIndex;
                        bankRankIndex = ii;
                    }
                    else
                    {
                        /* no need to continue the search for lower rank */
                        break;
                    }
                }
            }/*rank check*/
        }

    }

    devObjPtr->fdbBankCounterUsed = 0;


    if(overflowError || underflowError)
    {
        __LOG(("Interrupt: BLC (bank learn counters) overflow/underflow \n"));

        /* generate BLC (bank learn counters) overflow/underflow interrupt */
        snetChetahDoInterrupt(devObjPtr,
                              SMEM_CHT_MAC_TBL_INTR_CAUSE_REG(devObjPtr),
                              SMEM_CHT_MAC_TBL_INTR_CAUSE_MASK_REG(devObjPtr),
                              SMEM_LION3_BLC_OVERFLOW_INT,
                              SMEM_CHT_FDB_SUM_INT(devObjPtr));
    }

    if(simLogIsOpenFlag)
    {
        if(overflowError)
        {
            __LOG(("client[%s] counterAction[%s] for bank[%d] OVERFLOW ERROR ,already at value[%d] \n",
                          clientName,actionName,bankIndex,
                          counterPtr[bankIndex]));
        }
        else if(underflowError)
        {
            __LOG(("client[%s] counterAction[%s] for bank[%d] UNDERFLOW ERROR ,already at value 0 \n",
                          clientName,actionName,bankIndex));
        }
        else
        {
            __LOG(("client[%s] counterAction[%s] for bank[%d] new value[%d] \n"
                          "bank rank changed from [%d] to [%d] \n",
                          clientName,actionName,bankIndex,
                          counterPtr[bankIndex],
                          origBankRankIndex , bankRankIndex));
        }
    }

    return ;
}




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
* @file prvCpssDxChPortDpIronman.c
*
* @brief CPSS implementation for DP and resources configuration.
*        PB GPC Packet Read, TX DMA, TX FIFO, PCA Units, EPI MIF.
*
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortDpIronman.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* array address and size */
#define ARRAY_ADDR_AND_SIZE(_arr) _arr, (sizeof(_arr) / sizeof(_arr[0]))

/* included debug functions */
#define INC_DEBUG_FUNCTIONS

/* copied from prvCpssDxChPortTxPizzaResourceHawk.c and must be the same */
#define NOT_VALID_CNS 0xFFFFFFFF
/* CPU port channel */
#define PRV_IRONMAN_SDM0_CNS 56
/* maximal amount of resources (splitted slot groups) partially used by one channel */
#define PRV_IRONMAN_MAX_CHANNEL_RESOURCES_CNS 5
/* amount of slots used in pizza */
#define PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS 280
/* amount of channels including CPU present in arbiter only */
#define PRV_IRONMAN_CHANNELS_NUM_CNS 57
/* amount of pizza resources (splitted slot groups)  */
#define PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS 14
/* amount of slots in each pizza resource (splitted slot group)  */
#define PRV_IRONMAN_SLOTS_IN_RESOURCE_NUM_CNS 20
/* size of array */
#define SIZE_OF_ARR(arr)    sizeof(arr)/sizeof(arr[0])


/**
* @struct PRV_CHANNEL_RESOURCES_STC
 *
 * @brief Members of resources mapped to channel - channel never uses more than 2 resources
 * @note 14 resources (i.e. slot groups) 20 members in each
 * @note each two members of each resource separated by 13 members of other resources
*/
typedef struct
{
    /** resource index 0..13 */
    GT_U32 resourceIndex;
    /** bitmap of resource member indexes */
    GT_U32 membersBitmap;
} PRV_CHANNEL_RESOURCES_STC;

/**
* @struct PRV_CHANNEL_RESOURCES_STC
 *
 * @brief Channel info passed as parameter
*/
typedef struct
{
    /** channel number in DP */
    GT_U32                 channelNum;
    /** number of MIF unit in DP - 255 means "unknown" */
    GT_U32                 mifUnitNum;
    /** number of MIF channel in the MIF unit */
    GT_U32                 mifChannelNum;
    /** speed in Megabits per second */
    GT_U32                 speedInMbps;
    /** usage of SerDes lanes (256 units - per one lane) */
    GT_U32                 serdesUsage;
    /** GT_FALSE - regular, GT_TRUE - express or preemptive */
    GT_BOOL                preemptiveModeActivated;
    /** GT_FALSE - regular or express, GT_TRUE - preemptive */
    GT_BOOL                isPreemptive;
} PRV_CHANNEL_INFO_STC;


/**
* @internal prvCpssDxChPortDpIronmanMifChannelMapGenericSet function
* @endinternal
*
* @brief    Set channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[in] clockEnable            - channel clock enable (1) or disable (0)
* @param[in] rxPcaChannelNum        - RX PCA channel global number
* @param[in] rxPcaChannelEnable     - RX PCA channel enable (1) or disable (0)
* @param[in] rxPcaPfcEnable         - RX PCA PFC enable (1) or disable (0)
* @param[in] txPcaChannelNum        - TX PCA channel global number
* @param[in] txPcaChannelEnable     - TX PCA channel enable (1) or disable (0)
* @param[in] txPcaPfcEnable         - TX PCA PFC enable (1) or disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelMapGenericSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_U32                 clockEnable,
    IN    GT_U32                 rxPcaChannelNum,
    IN    GT_U32                 rxPcaChannelEnable,
    IN    GT_U32                 rxPcaPfcEnable,
    IN    GT_U32                 txPcaChannelNum,
    IN    GT_U32                 txPcaChannelEnable,
    IN    GT_U32                 txPcaPfcEnable
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 pcaChannelMask = 0x3F;
    GT_U32 regMask;
    GT_U32 regData;
    GT_U32 data;

    regMask = 
        (1 << 6)/*txEnable*/ | (1 << 14)/*rxEnable*/ | (1 << 18)/*clock*/
        | (1 << 7)/*txPcaPfcEnable*/ |  (1 << 15)/*rxPcaPfcEnable*/
        | pcaChannelMask/*txPcaChannelNum*/ | (pcaChannelMask << 8)/*rxPcaChannelNum*/;
    regData =
        (txPcaChannelEnable << 6)/*txEnable*/ |
        (rxPcaChannelEnable << 14)/*rxEnable*/ |
        (clockEnable << 18)/*clock*/ |
        (txPcaPfcEnable << 7)/*txPcaPfcEnable*/ |
        (rxPcaPfcEnable << 15)/*rxPcaPfcEnable*/ |
        txPcaChannelNum/*txPcaChannelNum*/ |
        (rxPcaChannelNum << 8) /*rxPcaChannelNum*/;

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((data & regMask) == regData)
    {
        return GT_OK;
    }

    data = ((data & (~ regMask)) | regData);

    return prvCpssDrvHwPpWriteRegister(devNum, regAddr, data);
}

/**
* @internal prvCpssDxChPortDpIronmanMifChannelMapGenericGet function
* @endinternal
*
* @brief    Get channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[out] clockEnablePtr        - (pointer to)channel clock enable (1) or disable (0)
* @param[out] rxPcaChannelNumPtr    - (pointer to)RX PCA channel global number
* @param[out] rxPcaChannelEnablePtr - (pointer to)RX PCA channel enable (1) or disable (0)
* @param[out] txPcaChannelNumPtr    - (pointer to)TX PCA channel global number
* @param[out] txPcaChannelEnablePtr - (pointer to)TX PCA channel enable (1) or disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelMapGenericGet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    OUT   GT_U32                 *clockEnablePtr,
    OUT   GT_U32                 *rxPcaChannelNumPtr,
    OUT   GT_U32                 *rxPcaChannelEnablePtr,
    OUT   GT_U32                 *txPcaChannelNumPtr,
    OUT   GT_U32                 *txPcaChannelEnablePtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32 regAddr;
    GT_U32 pcaChannelMask = 0x3F;
    GT_U32 regData;

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];

    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

   *clockEnablePtr          = (regData >> 18) & 1;
   *rxPcaChannelNumPtr      = (regData >>  8) & pcaChannelMask;
   *rxPcaChannelEnablePtr   = (regData >> 14) & 1;
   *txPcaChannelNumPtr      =  regData        & pcaChannelMask;
   *txPcaChannelEnablePtr   = (regData >>  6) & 1;
   return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanMifChannelMapSet function
* @endinternal
*
* @brief    Set channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[in] pcaChannelNum          - PCA channel global number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelMapSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_U32                 pcaChannelNum
)
{
    /* the "set clock only" stage removed according to PUNKT prcedure */
    /* Set and enable Rx and Tx PCA local channels*/
    return prvCpssDxChPortDpIronmanMifChannelMapGenericSet(
        devNum, unitNum, channelNum, 1/*clockEnable*/,
        pcaChannelNum/*rxPcaChannelNum*/, 1/*rxPcaChannelEnable*/, 1/*rxPcaPfcEnable*/,
        pcaChannelNum/*txPcaChannelNum*/, 1/*txPcaChannelEnable*/, 1/*txPcaPfcEnable*/);
}

/**
* @internal prvCpssDxChPortDpIronmanMifChannelMapDisable function
* @endinternal
*
* @brief    Disable channel mapping MIF to PCA.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelMapDisable
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum
)
{
    return prvCpssDxChPortDpIronmanMifChannelMapGenericSet(
        devNum, unitNum, channelNum, 0/*clockEnable*/,
        0x3F, 0/*rxPcaChannelEnable*/, 0/*rxPcaPfcEnable*/,
        0x3F, 0/*txPcaChannelEnable*/, 0/*txPcaPfcEnable*/);
}

/**
* @internal prvCpssDxChPortDpIronmanMifChannelPfcEnableGenericSet function
* @endinternal
*
* @brief    Set MIF PFC enable/disable.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[in] enableTx               - Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[in] enableRx               - Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelPfcEnableGenericSet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_BOOL                enableTx,
    IN    GT_BOOL                enableRx
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32 regAddr;
    GT_U32 regMask;
    GT_U32 regData;

    regMask = (1 << 7)/*enableTx*/ | (1 << 15)/*enableRx*/;
    regData = (BOOL2BIT_MAC(enableTx) << 7) | (BOOL2BIT_MAC(enableRx) << 15);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];

    return prvCpssDrvHwPpWriteRegBitMask(devNum, regAddr, regMask, regData);
}

/**
* @internal prvCpssDxChPortDpIronmanMifChannelPfcEnableGenericGet function
* @endinternal
*
* @brief    Get MIF PFC enable/disable.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
* @param[out] enableTx             - (Pointer to)Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[out] enableRx             - (Pointer to)Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelPfcEnableGenericGet
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum,
    IN    GT_BOOL                *enableTxPtr,
    IN    GT_BOOL                *enableRxPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32    field;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_STATUS rc;

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    field = (regData >> 7) & 1; /*enableTx*/
    *enableTxPtr = BIT2BOOL_MAC(field);
    field = (regData >> 15) & 1; /*enableRx*/
    *enableRxPtr = BIT2BOOL_MAC(field);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanMifMacToListOfChannels function
* @endinternal
*
* @brief    Get a list of MIF channels that can be mapped to the given Local port.
* @note     used to unmap channel with unknown speed/mode
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] localMac               - local MAC number (in DP)
* @param[out] mifChannelsNumPtr      - (pointer to)amount of MIF channels
* @param[out] mifUnitNumArrPtr       - (pointer to array of)MIF unit numbers
* @param[out] mifChannelNumArrPtr    - (pointer to array of)MIF channel numbers inside the unit
*
* @retval - none
*/
static void prvCpssDxChPortDpIronmanMifMacToListOfChannels
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 localMac,
    OUT   GT_U32                 *mifChannelsNumPtr,
    OUT   GT_U32                 *mifUnitNumArrPtr,
    OUT   GT_U32                 *mifChannelNumArrPtr
)
{
    static const GT_U8 macMap[4][17] =
    {
         /*         __0, __1, __2, __3, __4, __5, __6, __7, __8, __9, _10, _11, _12, _13, _14, _15, _16 */
         /* MIF0*/ {  0,   1,   2,   3,   4,   5,   6,   7,  24,  25,  26,  27,  28,  29,  30,  31,  52},
         /* MIF1*/ {  8,   9,  10,  11,  12,  13,  14,  15,  32,  33,  34,  35,  36,  37,  38,  39,  53},
         /* MIF2*/ { 16,  17,  18,  19,  20,  21,  22,  23,  40,  41,  42,  43,  44,  45,  46,  47,  54},
         /* MIF3*/ { 48,  49,  50,  51,  52,  53,  54,  55, 255, 255, 255, 255, 255, 255, 255, 255, 255}
    };
    GT_U32 unit;
    GT_U32 channel;

    GT_UNUSED_PARAM(devNum);
    *mifChannelsNumPtr = 0;
    for (unit = 0; (unit < 4); unit++)
    {
        for (channel = 0; (channel < 17); channel++)
        {
            if (macMap[unit][channel] == (GT_U8)localMac)
            {
                mifUnitNumArrPtr[*mifChannelsNumPtr]    = unit;
                mifChannelNumArrPtr[*mifChannelsNumPtr] = channel;
                (*mifChannelsNumPtr) ++;
            }
        }
    }
}

/**
* @internal prvCpssDxChPortDpIronmanMifMacToUnitAndChannel function
* @endinternal
*
* @brief    Convert Local port to MIF unit and channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] localMac               - local MAC number (in DP)
* @param[out] mifUnitNumPtr         - (pointer to)MIF unit number
* @param[out] mifChannelNumPtr      - (pointer to)MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
* @retval GT_FAIL                  - internal error
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifMacToUnitAndChannel
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 localMac,
    OUT   GT_U32                 *mifUnitNumPtr,
    OUT   GT_U32                 *mifChannelNumPtr
)
{
    GT_U32    mifChannelsNum;
    GT_U32    mifUnitNumArr[3];
    GT_U32    mifChannelNumArr[3];

    GT_STATUS rc;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT portRoleType;

    prvCpssDxChPortDpIronmanMifMacToListOfChannels(
        devNum, localMac,
        &mifChannelsNum, mifUnitNumArr, mifChannelNumArr);
    if (mifChannelsNum == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (mifChannelsNum == 1)
    {
        *mifUnitNumPtr     = mifUnitNumArr[0];
        *mifChannelNumPtr  = mifChannelNumArr[0];
        return GT_OK;
    }

    /*preemptionSupported = PRV_CPSS_PP_MAC(devNum)->preemptionSupported;*/

    /* for Iroman having one datapath only assumed:                              */
    /* 1. global MAC number equal to global Tx DMA number                        */
    /* 2. global Tx DMA number equal to local Tx DMA number                      */
    /* 3. any port supporting preemption supports preemption on some speed >= 1G */
    rc =  prvCpssDxChTxqSip6_10LocalPortActualTypeGet(
        devNum, 0/*dpIndex*/, localMac/*localDmaNum*/, 1000/*speedInMb*/, &portRoleType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portRoleType == PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_PRE_E)
    {
        /* related to ports 52,53,54 in MIF4 unit */
        *mifUnitNumPtr     = mifUnitNumArr[1];
        *mifChannelNumPtr  = mifChannelNumArr[1];
    }
    else
    {
        *mifUnitNumPtr     = mifUnitNumArr[0];
        *mifChannelNumPtr  = mifChannelNumArr[0];
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanMifChannelMapFindAndDisable function
* @endinternal
*
* @brief    Disable of MIF channels mapped to the given Local port.
* @note     used to unmap channel with unknown speed/mode
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] localMac               - local MAC number (in DP)
* @param[out] mifChannelsNumPtr      - (pointer to)amount of MIF channels
* @param[out] mifUnitNumArrPtr       - (pointer to array of)MIF unit numbers
* @param[out] mifChannelNumArrPtr    - (pointer to array of)MIF channel numbers inside the unit
*
* @retval - none
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifChannelMapFindAndDisable
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 localMac
)
{
   GT_STATUS rc;
   GT_U32    mifChannelsNum;
   GT_U32    mifUnitNumArr[3];
   GT_U32    mifChannelNumArr[3];
   GT_U32    i;
   GT_U32    clockEnable;
   GT_U32    rxPcaChannelNum;
   GT_U32    rxPcaChannelEnable;
   GT_U32    txPcaChannelNum;
   GT_U32    txPcaChannelEnable;

   prvCpssDxChPortDpIronmanMifMacToListOfChannels(
       devNum, localMac,
       &mifChannelsNum, mifUnitNumArr, mifChannelNumArr);

   for (i = 0; (i < mifChannelsNum); i++)
   {
       rc = prvCpssDxChPortDpIronmanMifChannelMapGenericGet(
           devNum, mifUnitNumArr[i], mifChannelNumArr[i],
           &clockEnable, &rxPcaChannelNum, &rxPcaChannelEnable,
           &txPcaChannelNum, &txPcaChannelEnable);
       if (rc != GT_OK)
       {
           return GT_OK;
       }
       if (clockEnable == 0) continue;
       if ((rxPcaChannelNum && (rxPcaChannelNum == localMac))
           || (txPcaChannelEnable && (txPcaChannelNum == localMac)))
       {
           rc = prvCpssDxChPortDpIronmanMifChannelMapDisable(
               devNum, mifUnitNumArr[i], mifChannelNumArr[i]);
           if (rc != GT_OK)
           {
               return GT_OK;
           }
       }
   }
   return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanMifUnitsInit function
* @endinternal
*
* @brief    Initialyze Ironman MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*
* @note - This function is a place holer for MIF units registers initialization.
* @note - according to Cider register default valies - no action needed
*/
static GT_STATUS prvCpssDxChPortDpIronmanMifUnitsInit
(
    IN    GT_U8                                devNum
)
{
    devNum = devNum; /*use parameter*/
    return GT_OK;
}

/* PCA SFF Configure and Down */

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC ironman_sff_num_of_lls[] =
{
    {{0xFFFFFFFF, 0xFFFFFFFF},     0,   2}, /* 2.5 G - 2 LLs        */
    {{0xFFFFFFFF, 0xFFFFFFFF},  5000,   3}, /* 5G, 10G, 12G - 3 LLs */
    /* invalid vales for not supported speeds */
    {{0x00EEEEEE, 0x00000000},  2501,   NOT_VALID_CNS}, /* ports 1-3,5-7,9-11,13-15,17-19,21-23 cannot be faster than 2.5G*/
    {{0xFF000000, 0x0040FFFF},  2501,   NOT_VALID_CNS}, /* ports 24-47,55 cannot be faster than 2.5G*/
    {{0x00111111, 0x00300000}, 10001,   NOT_VALID_CNS}, /* ports 0,4,8,12,16,20,52-54 cannot be faster than 10G*/
    {{0xFFFFFFFF, 0xFFFFFFFF}, 12001,   NOT_VALID_CNS}, /* no ports can be faster than 12G */
    {{0, 0},NOT_VALID_CNS, 0}
};

static const PRV_CPSS_DXCH_PIZZA_ARBITER_SPEED_ID_TO_VAL_STC ironman_sff_first_ll[] =
{
    {{0x00000001, 0x00000000},     0,   0}, /* channel  0, 0-2.5G */
    {{0x00000002, 0x00000000},     0,   3}, /* channel  1, 0-2.5G */
    {{0x00000004, 0x00000000},     0,   5}, /* channel  2, 0-2.5G */
    {{0x00000008, 0x00000000},     0,   7}, /* channel  3, 0-2.5G */
    {{0x00000010, 0x00000000},     0,   9}, /* channel  4, 0-2.5G */
    {{0x00000020, 0x00000000},     0,  12}, /* channel  5, 0-2.5G */
    {{0x00000040, 0x00000000},     0,  14}, /* channel  6, 0-2.5G */
    {{0x00000080, 0x00000000},     0,  16}, /* channel  7, 0-2.5G */
    {{0x00000100, 0x00000000},     0,  18}, /* channel  8, 0-2.5G */
    {{0x00000200, 0x00000000},     0,  21}, /* channel  9, 0-2.5G */
    {{0x00000400, 0x00000000},     0,  23}, /* channel 10, 0-2.5G */
    {{0x00000800, 0x00000000},     0,  25}, /* channel 11, 0-2.5G */
    {{0x00001000, 0x00000000},     0,  27}, /* channel 12, 0-2.5G */
    {{0x00002000, 0x00000000},     0,  30}, /* channel 13, 0-2.5G */
    {{0x00004000, 0x00000000},     0,  32}, /* channel 14, 0-2.5G */
    {{0x00008000, 0x00000000},     0,  34}, /* channel 15, 0-2.5G */
    {{0x00010000, 0x00000000},     0,  36}, /* channel 16, 0-2.5G */
    {{0x00020000, 0x00000000},     0,  39}, /* channel 17, 0-2.5G */
    {{0x00040000, 0x00000000},     0,  41}, /* channel 18, 0-2.5G */
    {{0x00080000, 0x00000000},     0,  43}, /* channel 19, 0-2.5G */
    {{0x00100000, 0x00000000},     0,  45}, /* channel 20, 0-2.5G */
    {{0x00200000, 0x00000000},     0,  48}, /* channel 21, 0-2.5G */
    {{0x00400000, 0x00000000},     0,  50}, /* channel 22, 0-2.5G */
    {{0x00800000, 0x00000000},     0,  52}, /* channel 23, 0-2.5G */
    {{0x01000000, 0x00000000},     0,  54}, /* channel 24, 0-2.5G */
    {{0x02000000, 0x00000000},     0,  56}, /* channel 25, 0-2.5G */
    {{0x04000000, 0x00000000},     0,  58}, /* channel 26, 0-2.5G */
    {{0x08000000, 0x00000000},     0,  60}, /* channel 27, 0-2.5G */
    {{0x10000000, 0x00000000},     0,  62}, /* channel 28, 0-2.5G */
    {{0x20000000, 0x00000000},     0,  64}, /* channel 29, 0-2.5G */
    {{0x40000000, 0x00000000},     0,  66}, /* channel 30, 0-2.5G */
    {{0x80000000, 0x00000000},     0,  68}, /* channel 31, 0-2.5G */
    {{0x00000000, 0x00000001},     0,  70}, /* channel 32, 0-2.5G */
    {{0x00000000, 0x00000002},     0,  72}, /* channel 33, 0-2.5G */
    {{0x00000000, 0x00000004},     0,  74}, /* channel 34, 0-2.5G */
    {{0x00000000, 0x00000008},     0,  76}, /* channel 35, 0-2.5G */
    {{0x00000000, 0x00000010},     0,  78}, /* channel 36, 0-2.5G */
    {{0x00000000, 0x00000020},     0,  80}, /* channel 37, 0-2.5G */
    {{0x00000000, 0x00000040},     0,  82}, /* channel 38, 0-2.5G */
    {{0x00000000, 0x00000080},     0,  84}, /* channel 39, 0-2.5G */
    {{0x00000000, 0x00000100},     0,  86}, /* channel 40, 0-2.5G */
    {{0x00000000, 0x00000200},     0,  88}, /* channel 41, 0-2.5G */
    {{0x00000000, 0x00000400},     0,  90}, /* channel 42, 0-2.5G */
    {{0x00000000, 0x00000800},     0,  92}, /* channel 43, 0-2.5G */
    {{0x00000000, 0x00001000},     0,  94}, /* channel 44, 0-2.5G */
    {{0x00000000, 0x00002000},     0,  96}, /* channel 45, 0-2.5G */
    {{0x00000000, 0x00004000},     0,  98}, /* channel 46, 0-2.5G */
    {{0x00000000, 0x00008000},     0, 100}, /* channel 47, 0-2.5G */
    /* the locations beow are the same as for 0-2.5G - duplicated */
    /*only to match the algorthm description and be fixed with it */
    {{0x00000001, 0x00000000},  5000,   0}, /* channel  0, 5G,10G */
    {{0x00000010, 0x00000000},  5000,   9}, /* channel  4, 5G,10G */
    {{0x00000100, 0x00000000},  5000,  18}, /* channel  8, 5G,10G */
    {{0x00001000, 0x00000000},  5000,  27}, /* channel 12, 5G,10G */
    {{0x00010000, 0x00000000},  5000,  36}, /* channel 16, 5G,10G */
    {{0x00100000, 0x00000000},  5000,  45}, /* channel 20, 5G,10G */
    /* high ports part */
    {{0x00000000, 0x00010000},     0, 102}, /* channel 48, any speed (0-12G) */
    {{0x00000000, 0x00020000},     0, 105}, /* channel 49, any speed (0-12G) */
    {{0x00000000, 0x00040000},     0, 108}, /* channel 50, any speed (0-12G) */
    {{0x00000000, 0x00080000},     0, 111}, /* channel 51, any speed (0-12G) */
    {{0x00000000, 0x00100000},     0, 114}, /* channel 52, any speed (0-12G) */
    {{0x00000000, 0x00200000},     0, 117}, /* channel 53, any speed (0-12G) */
    {{0x00000000, 0x00400000},     0, 120}, /* channel 54, any speed (0-12G) */
    {{0x00000000, 0x00800000},     0, 123}, /* channel 55, any speed (0-12G) */
    {{0, 0},NOT_VALID_CNS, 0}
};

/**
* @internal prvCpssDxChPortDpIronmanPcaSffInit function
* @endinternal
*
* @brief    Initialize SFF unit.
* @brief    Placeholder for future fixes.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaSffInit
(
    IN    GT_U8                                devNum
)
{
    GT_UNUSED_PARAM(devNum);
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaSffChannelDown function
* @endinternal
*
* @brief    Disable for PCA SFF unit channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] channelNum             - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaSffChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               channelNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    ringStart;
    GT_U32    ringSize;
    GT_U32    ringNext;
    GT_U32    ringNewNext;

    /* disable chanel if enabled */
    regAddr = regsAddrPtr->PCA_SFF[0].channelControlEnable[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (regData & 1)
    {
        regData &= (~ 1);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* found disable - not needed to restore defaults */
        return GT_OK;
    }

    regAddr = regsAddrPtr->PCA_SFF[0].channelControlConfig[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    ringStart  = (regData >> 8) & 0x7F;
    ringSize   = regData & 0x3F;
    for (ringNext = ringStart; (ringSize > 0); ringSize--)
    {
        regAddr = regsAddrPtr->PCA_SFF[0].llNextPointer[ringNext];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        ringNewNext   = regData & 0x7F;
        /* default value - self pointer */
        rc = prvCpssDrvHwPpWriteRegister(
            devNum, regAddr, ((regData & 0xFFFFFF80) | (ringNext & 0x7F)));
        if (rc != GT_OK)
        {
            return rc;
        }
        ringNext = ringNewNext;
        if (ringNext == ringStart)
        {
            /* end of ring */
            break;
        }
    }

    regAddr = regsAddrPtr->PCA_SFF[0].channelControlConfig[channelNum];
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0);
    return rc;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaSffChannelConfigure function
* @endinternal
*
* @brief    Configure PCA SFF unit channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] channelNum             - channel number
* @param[in] speedInMbps            - speed in Mega Bit Per Second
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaSffChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               channelNum,
    IN    GT_U32                               speedInMbps
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    enableRegData;
    GT_U32    regData;
    GT_U32    numOfLls;
    GT_U32    firstLl;
    GT_U32    idx;

    /* disable chanel if enabled - Configuring Channel_Control_Enable */
    regAddr = regsAddrPtr->PCA_SFF[0].channelControlEnable[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &enableRegData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (enableRegData & 1)
    {
        enableRegData &= (~ 1);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, enableRegData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Configuring LL_Next_Pointer */
    numOfLls =
        prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
            ARRAY_ADDR_AND_SIZE(ironman_sff_num_of_lls), channelNum, speedInMbps);
    if (numOfLls == NOT_VALID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    firstLl =
        prvCpssDxChTxPortSpeedPizzaResourcesValueBySpeedIdGet(
            ARRAY_ADDR_AND_SIZE(ironman_sff_first_ll), channelNum, speedInMbps);
    if (firstLl == NOT_VALID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (idx = 0; (idx < numOfLls); idx++)
    {
        regAddr = regsAddrPtr->PCA_SFF[0].llNextPointer[firstLl + idx];
        regData = ((idx + 1) < numOfLls) ? (firstLl + idx + 1) : firstLl;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Configuring channel_control_configuration */

    regAddr = regsAddrPtr->PCA_SFF[0].channelControlConfig[channelNum];
    regData = numOfLls/*max occup*/ | (firstLl << 8)/*write ptr*/ | (firstLl << 16)/*read ptr*/;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable channel - Configuring Channel_Control_Enable */
    regAddr = regsAddrPtr->PCA_SFF[0].channelControlEnable[channelNum];
    enableRegData |= 1;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, enableRegData);
    if (rc != GT_OK)
    {
        return rc;
    }

    GT_UNUSED_PARAM(speedInMbps);
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaSffDump function
* @endinternal
*
* @brief    Dump SFF Fifo of all channels.
* @note     Debug function called from command line.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChPortDpIronmanPcaSffDump
(
    IN    GT_U8   devNum
)
{
    GT_STATUS rc;
    GT_U8     llNextPtrArr[128];
    GT_U32    maxLlNextIndex;
    GT_U32    channelAmount;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    llStart;
    GT_U32    llNext;
    GT_U32    index;
    GT_U32    data;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* The GM not supports the DP related units except RX DMA */
        cpssOsPrintf("Not supported unit\n");
        return GT_OK;
    }

    cpssOsMemSet(llNextPtrArr, 0, sizeof(llNextPtrArr));

    channelAmount  = 56;
    maxLlNextIndex = 128;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* Global configuration */
    regAddr = regsAddrPtr->PCA_SFF[0].globalConfig;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
        return rc;
    }
    cpssOsPrintf(
        "global configuration: statistics collection enable %d statistics for SOP (not EOP) %d\n",
        ((data >> 24) & 1), ((data >> 23) & 1));
    cpssOsPrintf(
        "====================: statistics channel select %d\n",
        ((data >> 16) & 0x3F));

    /* load aff llNextPointer registers to llNextPtrArr */
    for (index = 0; (index < maxLlNextIndex); index++)
    {
        regAddr = regsAddrPtr->PCA_SFF[0].llNextPointer[index];
        if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            maxLlNextIndex = index;
            break;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        llNextPtrArr[index] = (GT_U8)(data & 0x7F);
    }

    /* unused FIFO - element next poiter poinits to itself */
    cpssOsPrintf("    | 000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 |\n");
    cpssOsPrintf("----|---------------------------------------------------------------------------------");
    for (index = 0; (index < maxLlNextIndex); index++)
    {
        if ((index % 20) == 0)
        {
            cpssOsPrintf("|\n%03d | ", index);
        }
        cpssOsPrintf("%03d ", llNextPtrArr[index]);
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("--------------------------------------------------------------------------------------");
    cpssOsPrintf("\n");

    /* per channel registers */
    for (channel = 0; (channel < channelAmount); channel++)
    {
        regAddr = regsAddrPtr->PCA_SFF[0].channelControlEnable[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        if ((data & 1) == 0)
        {
            /* channel disable */
            continue;
        }
        cpssOsPrintf("channel %02d ", channel);
        cpssOsPrintf(
            "Statistics Enable %d Channel Enable %d\n",
            ((data >> 1) & 1), (data & 1));

        regAddr = regsAddrPtr->PCA_SFF[0].channelControlConfig[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        cpssOsPrintf(
            "CONFIG rdPtr %d wrPtr %d MaxOccup %d\n",
            ((data >> 16) & 0x7F), ((data >> 8) & 0x7F), (data & 0x3F));
        llStart = (data >> 16) & 0x7F;
        cpssOsPrintf("FIFO ring: ");
        for (llNext = NOT_VALID_CNS, index = 0;
              ((llNext != llStart) && (index < 128));
              llNext = llNextPtrArr[llNext], index++)
        {
            if (llNext == NOT_VALID_CNS) llNext = llStart;
            cpssOsPrintf("%d ", llNext);
        }
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaSffAllocationPrint function
* @endinternal
*
* @brief    Print SFF Fifo allocations of all channels.
* @note     Debug function called from command line.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChPortDpIronmanPcaSffAllocationPrint
(
    IN    GT_U8   devNum
)
{
    GT_STATUS rc;
    GT_U8     llNextPtrArr[128];
    GT_U8     llPrintArr[128];
    GT_U32    maxLlNextIndex;
    GT_U32    channelAmount;
    GT_U32    regAddr;
    GT_U32    channel;
    GT_U32    llStart;
    GT_U32    llNext;
    GT_U32    index;
    GT_U32    data;
    const PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* The GM not supports the DP related units except RX DMA */
        cpssOsPrintf("Not supported unit\n");
        return GT_OK;
    }

    cpssOsMemSet(llNextPtrArr, 0xFF, sizeof(llNextPtrArr));
    cpssOsMemSet(llPrintArr, 0xFF, sizeof(llPrintArr));

    channelAmount  = 56;
    maxLlNextIndex = 128;

    regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);

    /* load aff llNextPointer registers to llNextPtrArr */
    for (index = 0; (index < maxLlNextIndex); index++)
    {
        regAddr = regsAddrPtr->PCA_SFF[0].llNextPointer[index];
        if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            maxLlNextIndex = index;
            break;
        }
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        llNextPtrArr[index] = (GT_U8)(data & 0x7F);
    }

    /* per channel registers */
    for (channel = 0; (channel < channelAmount); channel++)
    {
        regAddr = regsAddrPtr->PCA_SFF[0].channelControlEnable[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        if ((data & 1) == 0)
        {
            /* channel disable */
            continue;
        }

        regAddr = regsAddrPtr->PCA_SFF[0].channelControlConfig[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDrvHwPpReadRegister failed\n");
            return rc;
        }
        llStart = (data >> 16) & 0x7F;
        for (llNext = NOT_VALID_CNS, index = 0;
              ((llNext != llStart) && (index < 128));
              llNext = llNextPtrArr[llNext], index++)
        {
            if (llNext == NOT_VALID_CNS) llNext = llStart;
            llPrintArr[llNext] = (GT_U8)channel;
        }
    }

    /* unused FIFO - element next poiter poinits to itself */
    cpssOsPrintf("    | 000 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 |\n");
    cpssOsPrintf("----|---------------------------------------------------------------------------------");
    for (index = 0; (index < maxLlNextIndex); index++)
    {
        if ((index % 20) == 0)
        {
            cpssOsPrintf("|\n%03d | ", index);
        }
        if (llPrintArr[index] != 0xFF)
        {
            cpssOsPrintf("%03d ", llPrintArr[index]);
        }
        else
        {
            cpssOsPrintf("*** ");
        }
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("--------------------------------------------------------------------------------------");
    cpssOsPrintf("\n");

    return GT_OK;
}

/* PCA Arbiter */

/**
* @struct PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC
 *
 * @brief List of parameters of channels that must be equal to share the same resource.
 * @brief for members of the same channel group.
*/
typedef struct
{
    /** serdesUsage: 32 - Octo, 64 - Quadro, 256 - single MAC per serdes. */
    /** Measured in units of 1/256 of serdes lane sharing.                */
    GT_U32 serdesUsage;
    /** speed In Mbps - speeds 10M, 100M, 1G treated as the same speed b*/
    GT_U32 speedInMbps;
    /** GT_TRUE - Express or Preemptive, GT_FALSE - regular */
    GT_U32 preemptionModeActivated;

} PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC;

/**
* @struct PRV_IRONMAN_PCA_ARBITER_CHANNEL_GROUPS_LIST_STC
 *
 * @brief List of channel groups relevant to the given filter.
*/
typedef struct
{
    /** Channel filter */
    PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC filter;
    /* Amount of Channel groups */
    GT_U32                                      bitmap64ArrSize;
    /* pointer to array of 32-bit words */
    /** each pair of words is 64-bit bitmap of channels */
    const GT_U32                                *bitmap64ArrPtr;
} PRV_IRONMAN_PCA_ARBITER_CHANNEL_GROUPS_LIST_STC;

/**
* @struct PRV_IRONMAN_PCA_ARBITER_CHANNEL_RESOURCE_MEMBERS_STC
 *
 * @brief List of shifts of resource members bitmap.
 * @brief The same resource members bitmap 
 * @brief used with different shifts for channels of the same group.
*/
typedef struct
{
    /** Channel filter */
    PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC filter;
    /** bitmap of channels for additional filtering */
    GT_U32                                channel64Bitmap[2];
    /** set of left shift values coded as bitmap */
    GT_U32                                shiftsBitmap;
    /** GT_TRUE - can share resource with SDMA, GT_FALSE - cannot */
    GT_U32                                allocWithSdma;
} PRV_IRONMAN_PCA_ARBITER_CHANNEL_RESOURCE_MEMBERS_STC;

/**
* @struct PRV_IRONMAN_PCA_ARBITER_RESOURCE_SPEED_TO_MEMBERS_BMP_STC
 *
 * @brief Element of table converting speed to resource members bitmap.
*/
typedef struct
{
    /** speed in MB per second */
    GT_U32                                speedInMbps;
    /* Base Resource Members Bitmap - can be shifted */
    GT_U32                                baseMembersBitmap;
} PRV_IRONMAN_PCA_ARBITER_RESOURCE_SPEED_TO_MEMBERS_BMP_STC;

/* Channel Groups Octo mode channels 1G,100M,10M */
static const GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiter8ChannelsBmpArr[] =
{
    0x000000FF, 0x00000000, /* 0-7 */
    0xFF000000, 0x00000000, /* 24-31 */
    0x0000FF00, 0x00000000, /* 8-15 */
    0x00000000, 0x000000FF, /* 32-39 */
    0x00FF0000, 0x00000000, /* 16-23 */
    0x00000000, 0x0000FF00  /* 40-47 */
};

/* Channel Groups Quadro mode channels without preemtion 2.5G,1G,100M,10M */
/* and 2.5G with preemtion                                                */
static const GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr[] =
{
    0x000000F0, 0x00000000, /* 4-7 */
    0xF0000000, 0x00000000, /* 28-31 */
    0x0000000F, 0x00000000, /* 0-3 */
    0x0F000000, 0x00000000, /* 24-27 */
    0x0000F000, 0x00000000, /* 12-15 */
    0x00000000, 0x000000F0, /* 36-39 */
    0x00000F00, 0x00000000, /* 8-11 */
    0x00000000, 0x0000000F, /* 32-35 */
    0x00F00000, 0x00000000, /* 20-23 */
    0x00000000, 0x0000F000, /* 44-47 */
    0x000F0000, 0x00000000, /* 16-19 */
    0x00000000, 0x00000F00  /* 40-43 */
};

/* Channel Groups Quadro mode channels with preemtion 1G,100M,10M */
static const GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiter4_4ChannelsBmpArr[] =
{
    0xF00000F0, 0x00000000, /* 4-7,28-31 */
    0x0F00000F, 0x00000000, /* 0-3,24-27 */
    0x0000F000, 0x000000F0, /* 12-15,36-39 */
    0x00000F00, 0x0000000F, /* 8-11,32-35 */
    0x00F00000, 0x0000F000, /* 20-23,44-47 */
    0x000F0000, 0x00000F00  /* 16-19,40-43 */
};

/* Channel Groups Single mode channels with preemtion 2.5G,1G */
static const GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiter2_0ChannelsBmpArr[] =
{
    0x01000001, 0x00000000, /* 0,24 */
    0x10000010, 0x00000000, /* 4,28 */
    0x00000100, 0x00000001, /* 8,32 */
    0x00001000, 0x00000010, /* 12,36 */
    0x00010000, 0x00000100, /* 16,40 */
    0x00100000, 0x00001000, /* 20,44 */
    0x00000000, 0x00110000, /* 48,52 */
    0x00000000, 0x00220000, /* 49,53 */
    0x00000000, 0x00440000, /* 50,54 */
    0x00000000, 0x00880000  /* 51,55 */
};

/* Channel Groups Single mode channels with preemtion 5G */
static const GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiter2_1ChannelsBmpArr[] =
{
    0x00000000, 0x00110000, /* 48,52 */
    0x00000000, 0x00220000, /* 49,53 */
    0x00000000, 0x00440000, /* 50,54 */
    0x00000000, 0x00880000  /* 51,55 */
};

/* Table of conversion Channel Filter to List of Channel Groups */
static const PRV_IRONMAN_PCA_ARBITER_CHANNEL_GROUPS_LIST_STC 
    prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelGroupsArr[] =
{
    {{32, 1000, 0}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter8ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter8ChannelsBmpArr},
    {{32, 1000, 1}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter8ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter8ChannelsBmpArr},
    {{64, 2500, 1}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr},
    {{64, 2500, 0}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr},
    {{64, 1000, 0}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter4ChannelsBmpArr},
    {{64, 1000, 1}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter4_4ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter4_4ChannelsBmpArr},
    {{256, 2500, 1}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter2_0ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter2_0ChannelsBmpArr},
    {{256, 1000, 1}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter2_0ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter2_0ChannelsBmpArr},
    {{256, 5000, 1}, 
        SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter2_1ChannelsBmpArr),
        prvCpssDxChPortDpIronmanPcaPizzaArbiter2_1ChannelsBmpArr}
};

/* Table of conversion Channel Filer to Resource member grroup offsets */
static const PRV_IRONMAN_PCA_ARBITER_CHANNEL_RESOURCE_MEMBERS_STC 
    prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesMembersArr[] =
{
    {{ 32, 1000, 0}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x000001EF, 1}, /* bypass possible SD members 4,9,14,19 */
    {{ 32, 1000, 1}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x000001EF, 1}, /* bypass possible SD members 4,9,14,19 */
    {{ 64, 2500, 1}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x0000000F, 0},
    {{ 64, 2500, 0}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x0000000F, 0},
    {{ 64, 1000, 0}, {0x00FFFFFF, 0x00000000}, 0x0000000F, 0}, /* ch0-23  - members0-3 */
    {{ 64, 1000, 0}, {0xFF000000, 0x0000FFFF}, 0x000000F0, 0}, /* ch24-47 - members4-7 */
    {{ 64, 1000, 1}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x000000FF, 0},
    {{256, 2500, 1}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x00000003, 0},
    {{256, 1000, 1}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x00000003, 0},
    {{256, 5000, 1}, {0xFFFFFFFF, 0xFFFFFFFF}, 0x00000003, 0}
};

/* Table of conversion channel speed to resource members base bitmap */
static const PRV_IRONMAN_PCA_ARBITER_RESOURCE_SPEED_TO_MEMBERS_BMP_STC
    prvCpssDxChPortDpIronmanPcaPizzaArbiterSpeedToMembersBaseBmp[] =
{
    {  100, 0x00000001},
    { 1000, 0x00000401},
    { 2500, 0x00011111},
    { 5000, 0x00055555},
    {10000, 0x000FFFFF}
};

/* Remaping of resources - each of resource 0-13 at a new place */
static const GT_U8  prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesRemapArr[] =
    {0,1,9,2,3,10,4,5,11,6,7,12,8,13};

/* Resource 0, members 4,9,14,19 */
static const PRV_CHANNEL_RESOURCES_STC prvCpssDxChPortDpIronmanPcaPizzaArbiterSdmaMap =
    {0, ((1 << 4) | (1 << 9) | (1 << 14) | (1 << 19))};

/* the lowest priority to use - dedicated to 12G channels */
static const GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesReservedBitmap = 0x00003E00; /* 9-13 */

/* Layout or resorce members of 12G channel 48 */
static const PRV_CHANNEL_RESOURCES_STC prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch48_MapArray[] =
{
    { 9, ((1 <<  0) | (1 <<  4) | (1 <<  8) | (1 << 12) | (1 << 16))},
    {10, ((1 <<  3) | (1 <<  7) | (1 << 11) | (1 << 15) | (1 << 19))},
    {11, ((1 <<  2) | (1 <<  6) | (1 << 10) | (1 << 14) | (1 << 18))},
    {12, ((1 <<  1) | (1 <<  5) | (1 <<  9) | (1 << 13) | (1 << 17))},
    {13, ((1 <<  0) | (1 <<  4) | (1 <<  8) | (1 << 12) | (1 << 16))}
};

/* Layout or resorce members of 12G channel 49 */
static const PRV_CHANNEL_RESOURCES_STC prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch49_MapArray[] =
{
    { 9, ((1 <<  1) | (1 <<  5) | (1 <<  9) | (1 << 13) | (1 << 17))},
    {10, ((1 <<  0) | (1 <<  4) | (1 <<  8) | (1 << 12) | (1 << 16))},
    {11, ((1 <<  3) | (1 <<  7) | (1 << 11) | (1 << 15) | (1 << 19))},
    {12, ((1 <<  2) | (1 <<  6) | (1 << 10) | (1 << 14) | (1 << 18))},
    {13, ((1 <<  1) | (1 <<  5) | (1 <<  9) | (1 << 13) | (1 << 17))}
};

/* Layout or resorce members of 12G channel 50 */
static const PRV_CHANNEL_RESOURCES_STC prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch50_MapArray[] =
{
    { 9, ((1 <<  2) | (1 <<  6) | (1 << 10) | (1 << 14) | (1 << 18))},
    {10, ((1 <<  1) | (1 <<  5) | (1 <<  9) | (1 << 13) | (1 << 17))},
    {11, ((1 <<  0) | (1 <<  4) | (1 <<  8) | (1 << 12) | (1 << 16))},
    {12, ((1 <<  3) | (1 <<  7) | (1 << 11) | (1 << 15) | (1 << 19))},
    {13, ((1 <<  2) | (1 <<  6) | (1 << 10) | (1 << 14) | (1 << 18))}
};

/* Layout or resorce members of 12G channel 51 */
static const PRV_CHANNEL_RESOURCES_STC prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch51_MapArray[] =
{
    { 9, ((1 <<  3) | (1 <<  7) | (1 << 11) | (1 << 15) | (1 << 19))},
    {10, ((1 <<  2) | (1 <<  6) | (1 << 10) | (1 << 14) | (1 << 18))},
    {11, ((1 <<  1) | (1 <<  5) | (1 <<  9) | (1 << 13) | (1 << 17))},
    {12, ((1 <<  0) | (1 <<  4) | (1 <<  8) | (1 << 12) | (1 << 16))},
    {13, ((1 <<  3) | (1 <<  7) | (1 << 11) | (1 << 15) | (1 << 19))}
};

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersGet function
* @endinternal
*
* @brief    Get filer filters of already configured channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  devNum                   - physical device number
* @param[in]  channel                  - channel number
* @param[out] channelResourceFilterPtr - (pointer to) filter of channel values from DB
*
* @retval - none
*/
static void prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersGet
(
    IN    GT_U8                                       devNum,
    IN    GT_U32                                      channel,
    OUT   PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC *channelResourceFilterPtr
)
{
    PRV_CPSS_DXCH_SIP6_30_DP_CHANNEL_INFO_STC *channelCfgPtr;

    channelCfgPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->channelConfiguration;

    channelResourceFilterPtr->preemptionModeActivated = channelCfgPtr[channel].preemptiveModeActivated;
    channelResourceFilterPtr->speedInMbps             = channelCfgPtr[channel].speedInMbps;
    channelResourceFilterPtr->serdesUsage             = channelCfgPtr[channel].serdesUsage;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterNewChannelFiltersGet function
* @endinternal
*
* @brief    Get filer filters of a new configured channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  channelInfoPtr           - (pointer to) channel info structure
* @param[in]  channel                  - channel number
* @param[out] channelResourceFilterPtr - (pointer to) filter of channel values from DB
*
* @retval - none
*/
static void prvCpssDxChPortDpIronmanPcaPizzaArbiterNewChannelFiltersGet
(
    IN    PRV_CHANNEL_INFO_STC                        *channelInfoPtr,
    OUT   PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC *channelResourceFilterPtr
)
{
    channelResourceFilterPtr->speedInMbps = channelInfoPtr->speedInMbps;
    channelResourceFilterPtr->serdesUsage = channelInfoPtr->serdesUsage;
    channelResourceFilterPtr->preemptionModeActivated = 
            BOOL2BIT_MAC(channelInfoPtr->preemptiveModeActivated);
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersEqual function
* @endinternal
*
* @brief    Check that two given filters are equal.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] channelResourceFilter1Ptr - (pointer to) first  filter of channel values from DB
* @param[in] channelResourceFilter2Ptr - (pointer to) second filter of channel values from DB
*
* @retval - 1 - equal; 0 - not equal
*/
static int prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersEqual
(
    IN    const PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC *channelResourceFilter1Ptr,
    IN    const PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC *channelResourceFilter2Ptr
)
{
    GT_U32 speed1, speed2;

    speed1 = channelResourceFilter1Ptr->speedInMbps;
    speed1 = (speed1 < 1000) ? 1000 : speed1;
    speed2 = channelResourceFilter2Ptr->speedInMbps;
    speed2 = (speed2 < 1000) ? 1000 : speed2;
    if (speed1 != speed2) return 0;
    if (channelResourceFilter1Ptr->serdesUsage 
        != channelResourceFilter2Ptr->serdesUsage) return 0;
    if (channelResourceFilter1Ptr->preemptionModeActivated 
        != channelResourceFilter2Ptr->preemptionModeActivated) return 0;
    return 1;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelGroupGet function
* @endinternal
*
* @brief    Get Channel group 64-bitmap.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] channelResourceFilterPtr - (pointer to) filter of channel values from DB
* @param[in] channel                  - Channel number
* @param[out] channelGroupBmpArr      - (pointer to) 64-channels bitmap representing the given channel
*
* @retval - none
*/
static void prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelGroupGet
(
    IN    PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC *channelResourceFilterPtr,
    IN    GT_U32                                      channel,
    OUT   GT_U32                                      *channelGroupBmpArr
)
{
    GT_U32 i;
    GT_U32 j;
    GT_U32 arrSize;
    const PRV_IRONMAN_PCA_ARBITER_CHANNEL_GROUPS_LIST_STC *groupListPtr;
    const GT_U32                                          *bmp64Ptr;

    arrSize = SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelGroupsArr);
    for (i = 0; (i < arrSize); i++)
    {
        groupListPtr = &(prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelGroupsArr[i]);

        if (prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersEqual(
            channelResourceFilterPtr, &(groupListPtr->filter)) == 0) continue;

        for (j = 0;(j < groupListPtr->bitmap64ArrSize); j += 2)
        {
            bmp64Ptr = &(groupListPtr->bitmap64ArrPtr[j]);
            if (bmp64Ptr[channel / 32] & (1 << (channel % 32)))
            {
                channelGroupBmpArr[0] = bmp64Ptr[0];
                channelGroupBmpArr[1] = bmp64Ptr[1];
                return;
            }
        }
    }
    /* not found */
    channelGroupBmpArr[0] = 0;
    channelGroupBmpArr[1] = 0;
    channelGroupBmpArr[channel / 32] |= (1 << (channel % 32));
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterResourceMembersBmpGet function
* @endinternal
*
* @brief   Ger channel Resource Members bitmap.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] channelResourceFilterPtr - (pointer to) filter of channel values from DB
* @param[in] channelGroupBmpArr       - (pointer to) 64-channels bitmap representing the given channel
* @param[in] channel                  - Channel number
* @param[out] resourceMembersBmpPtr   - (pointer to) 32-bitmap of resorce members
* @param[out] canShareWithSdmaPtr     - (pointer to) 0 - cannot share resource with SDMA, other - can
*
* @retval - none
*/
static void prvCpssDxChPortDpIronmanPcaPizzaArbiterResourceMembersBmpGet
(
    IN    PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC *channelResourceFilterPtr,
    IN    GT_U32                                      *channelGroupBmpArr,
    IN    GT_U32                                      channel,
    OUT   GT_U32                                      *resourceMembersBmpPtr,
    OUT   GT_U32                                      *canShareWithSdmaPtr
)
{
    GT_U32 i;
    GT_U32 speed;
    GT_U32 index;
    GT_U32 baseMembersBmp;
    GT_U32 shiftsBmp;
    const PRV_IRONMAN_PCA_ARBITER_CHANNEL_RESOURCE_MEMBERS_STC *resourceMembersPtr;
    GT_U32 arrSize;
    GT_U32 foundBits;
    GT_U32 foundShift;

    /* invalid value, updated below */
    *resourceMembersBmpPtr = 0;
    /* default - can be updated */
    *canShareWithSdmaPtr = 0;

    speed = channelResourceFilterPtr->speedInMbps;
    baseMembersBmp = 0;
    for (i = 0; (i < SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiterSpeedToMembersBaseBmp)); i++)
    {
        if (prvCpssDxChPortDpIronmanPcaPizzaArbiterSpeedToMembersBaseBmp[i].speedInMbps >= speed)
        {
            baseMembersBmp =
                prvCpssDxChPortDpIronmanPcaPizzaArbiterSpeedToMembersBaseBmp[i].baseMembersBitmap;
            break;
        }
    }
    if (baseMembersBmp == 0)
    {
        /* unsupported speed */
        return;
    }

    shiftsBmp = 1; /* default shift bitmap for channels not sharing resource */
    arrSize = SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesMembersArr);
    for (i = 0; (i < arrSize); i++)
    {
        resourceMembersPtr = &(prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesMembersArr[i]);

        if (prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersEqual(
            channelResourceFilterPtr, &(resourceMembersPtr->filter)) == 0) continue;
        if ((resourceMembersPtr->channel64Bitmap[channel / 32] 
             & (1 << (channel % 32))) == 0) continue;

        *canShareWithSdmaPtr = resourceMembersPtr->allocWithSdma;
        shiftsBmp            = resourceMembersPtr->shiftsBitmap;
        break;
    }

    /* count channel index in group */
    index = 0;
    for (i = 0; (i < channel); i++)
    {
        if (channelGroupBmpArr[i / 32] & (1 << (i % 32)))
        {
            index ++;
        }
    }

    /* search in foundBits one-bit number "index" */
    foundShift = 0xFFFFFFFF;
    foundBits = 0;
    for (i = 0; (i < 32); i++)
    {
        if (shiftsBmp & (1 << i))
        {
            foundBits ++;
        }
        if (foundBits == (index + 1))
        {
            foundShift = i;
            break;
        }
    }
    if (foundShift == 0xFFFFFFFF)
    {
        /* not found shift for group member */
        return;
    }

    *resourceMembersBmpPtr = (baseMembersBmp << foundShift);
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterResourceMemberToSlot function
* @endinternal
*
* @brief    Convert Slot To Resource Member.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] slot               - slot (0..279)
*
* @retval - bits[31:16] - member, bits[15:0] - resource
*/
static GT_U32 prvCpssDxChPortDpIronmanPcaPizzaArbiterSlotToResourceMember
(
    IN    GT_U32     slot
)
{
    GT_U32 member = (slot / PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS);
    GT_U32 resourceIdx = (slot % PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS);
    return ((member << 16) | prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesRemapArr[resourceIdx]);
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterResourceMemberToSlot function
* @endinternal
*
* @brief    Convert Resource Member To Slot.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  devNum                 - physical device number
* @param[in]  channelInfoPtr         - (pointer to) Channel Info structure
* @param[out] resourcesMapArrSizePtr - (pointer to) size of array of channel map structures
* @param[out] resourcesMapArrPtr     - (pointer to) array of channel map structures
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelMap
(
    IN    GT_U8                      devNum,
    IN    PRV_CHANNEL_INFO_STC       *channelInfoPtr,
    OUT   GT_U32                     *resourcesMapArrSizePtr,
    OUT   PRV_CHANNEL_RESOURCES_STC  *resourcesMapArrPtr
)
{
    const PRV_CHANNEL_RESOURCES_STC  *foundResourcesMapPtr;
    GT_U32   resourceAndMember;
    GT_U8    *arbiterShadowPtr;
    GT_U32   channelNum;
    GT_U32   channel;
    GT_U32   sdmaFoundResourceIndex;
    GT_U32   slot;
    GT_U32   index;
    GT_U32   resourceUsedMap[PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS];
    GT_U8    resourceFoundChannel[PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS];
    GT_U32   resourceMember;
    PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC newChannelFilter;
    PRV_IRONMAN_PCA_ARBITER_RESOURCE_FILTER_STC foundChannelFilter;
    GT_U32   resourceMembersBmp;
    GT_U32   canShareWithSdma;
    GT_U32   newChannelGroup[2];
    GT_U32   resourceAllocated;

    channelNum = channelInfoPtr->channelNum;
    arbiterShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->pcaArbiterShadow;

    /* load resource use map from arbiter shadow */
    cpssOsMemSet(resourceUsedMap, 0, sizeof(resourceUsedMap));
    cpssOsMemSet(resourceFoundChannel, 0xFF, sizeof(resourceFoundChannel));
    sdmaFoundResourceIndex = 0xFFFFFFFF;
    for (slot = 0; (slot < PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS); slot++)
    {
        channel = arbiterShadowPtr[slot];
        if (channel == 0x3F) continue;

        resourceAndMember = 
            prvCpssDxChPortDpIronmanPcaPizzaArbiterSlotToResourceMember(slot);

        index = resourceAndMember & 0xFFFF;
        resourceMember = (resourceAndMember >> 16) & 0xFFFF;
        resourceUsedMap[index] |= (1 << resourceMember);
        if ((resourceFoundChannel[index] == 0xFF)
            && (channel != PRV_IRONMAN_SDM0_CNS))
        {
            resourceFoundChannel[index] = (GT_U8)channel;
        }
        if (channel == PRV_IRONMAN_SDM0_CNS)
        {
            sdmaFoundResourceIndex = index;
        }
    }

    if (channelNum == PRV_IRONMAN_SDM0_CNS)
    {
        if (resourceUsedMap[prvCpssDxChPortDpIronmanPcaPizzaArbiterSdmaMap.resourceIndex] 
            & prvCpssDxChPortDpIronmanPcaPizzaArbiterSdmaMap.membersBitmap)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        *resourcesMapArrSizePtr = 1;
        resourcesMapArrPtr[0] = prvCpssDxChPortDpIronmanPcaPizzaArbiterSdmaMap;
        return GT_OK;
    }

    if (channelInfoPtr->speedInMbps >= 12000)
    {
        switch (channelNum)
        {
            case 48:
                foundResourcesMapPtr = prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch48_MapArray;
                *resourcesMapArrSizePtr = SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch48_MapArray);
                break;
            case 49: /* no break */
                foundResourcesMapPtr = prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch49_MapArray;
                *resourcesMapArrSizePtr = SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch49_MapArray);
                break;
            case 50: /* no break */
                foundResourcesMapPtr = prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch50_MapArray;
                *resourcesMapArrSizePtr = SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch50_MapArray);
                break;
            case 51:
                foundResourcesMapPtr = prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch51_MapArray;
                *resourcesMapArrSizePtr = SIZE_OF_ARR(prvCpssDxChPortDpIronmanPcaPizzaArbiter12G_Ch51_MapArray);
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* copy const resource array */
        for (index = 0; (index < *resourcesMapArrSizePtr); index++)
        {
            resourcesMapArrPtr[index] = foundResourcesMapPtr[index];
        }
        /* check result not overlapping already allocated slots */
        for (index = 0; (index < *resourcesMapArrSizePtr); index++)
        {
            if (resourceUsedMap[resourcesMapArrPtr[index].resourceIndex]
                & resourcesMapArrPtr[index].membersBitmap)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }
        }

        return GT_OK;
    }

    prvCpssDxChPortDpIronmanPcaPizzaArbiterNewChannelFiltersGet(
        channelInfoPtr, &newChannelFilter);

    prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelGroupGet(
        &newChannelFilter, channelNum, newChannelGroup);

    prvCpssDxChPortDpIronmanPcaPizzaArbiterResourceMembersBmpGet(
        &newChannelFilter, newChannelGroup, channelNum,
        &resourceMembersBmp, &canShareWithSdma);
    if (resourceMembersBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    resourceAllocated = 0xFFFFFFFF;

    /* attempt to use resource used by the same group */
    for (index = 0; (index < PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS); index++)
    {
        channel = resourceFoundChannel[index];
        if (channel == 0xFF) continue;

        /* found channel not in the new channel's group */
        if ((newChannelGroup[channel / 32] & (1 << (channel % 32))) == 0) continue;

        prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersGet(
            devNum, resourceFoundChannel[index], &foundChannelFilter);

        /* found channel has different resouce filter */
        if (prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelFiltersEqual(
            &foundChannelFilter, &newChannelFilter) == 0) continue;

        resourceAllocated = index;
        break;
    }

    /*  use SDMA resource when it is yet free */
    if ((resourceAllocated == 0xFFFFFFFF) 
        && canShareWithSdma 
        && (sdmaFoundResourceIndex != 0xFFFFFFFF)
        && (resourceFoundChannel[sdmaFoundResourceIndex] == 0xFF))
    {
        resourceAllocated = sdmaFoundResourceIndex;
    }

    /* use not reserved free resource */
    if (resourceAllocated == 0xFFFFFFFF)
    {
        for (index = 0; (index < PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS); index++)
        {
            if (prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesReservedBitmap 
                & (1 << index)) continue;
            if (resourceUsedMap[index] != 0) continue;

            resourceAllocated = index;
            break;
        }
    }

    /* use reserved free resource */
    if (resourceAllocated == 0xFFFFFFFF)
    {
        for (index = 0; (index < PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS); index++)
        {
            if ((prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesReservedBitmap 
                & (1 << index)) == 0) continue;
            if (resourceUsedMap[index] != 0) continue;

            resourceAllocated = index;
            break;
        }
    }

    if (resourceAllocated == 0xFFFFFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    if (resourceUsedMap[resourceAllocated] & resourceMembersBmp)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    *resourcesMapArrSizePtr = 1;
    resourcesMapArrPtr[0].resourceIndex = resourceAllocated;
    resourcesMapArrPtr[0].membersBitmap = resourceMembersBmp;
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelConfigure function
* @endinternal
*
* @brief    Configure channel on PCA Arbiter unit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                - physical device number
* @param[in] channelInfoPtr        - (pointer to) Channel Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelConfigure
(
    IN    GT_U8                     devNum,
    IN    PRV_CHANNEL_INFO_STC      *channelInfoPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U8                                      *arbiterShadowPtr;
    PRV_CPSS_DXCH_SIP6_30_DP_CHANNEL_INFO_STC  *channelCfgPtr;
    GT_STATUS                  rc;
    GT_U32                     resourcesMapArrSize;
    PRV_CHANNEL_RESOURCES_STC  resourcesMapArr[PRV_IRONMAN_MAX_CHANNEL_RESOURCES_CNS];
    GT_U8                      resourceInMapIndexArr[PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS];
    GT_U32                     resIndexInMap;
    GT_U32                     resourceAndMember;
    GT_U32                     regAddr;
    GT_U32                     regData;
    GT_U32                     resIndex;
    GT_U32                     slotIndex;
    GT_U32                     slot;

    arbiterShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->pcaArbiterShadow;
    channelCfgPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->channelConfiguration;

    rc = prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelMap(
        devNum, channelInfoPtr, &resourcesMapArrSize, resourcesMapArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* enable channel, SDMA also enabled */
    regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].channelEnable[channelInfoPtr->channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ((regData & 1) == 0)
    {
        regData |= 1;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* array for conversion resource index to index in Map */
    for (resIndex = 0; (resIndex < PRV_IRONMAN_ARBITER_RESOURCES_NUM_CNS); resIndex++)
    {
        resourceInMapIndexArr[resIndex] = 0xFF;
    }
    for (resIndex = 0; (resIndex < resourcesMapArrSize); resIndex++)
    {
        resourceInMapIndexArr[resourcesMapArr[resIndex].resourceIndex] = resIndex;
    }

    for (slot = 0; (slot < PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS); slot++)
    {
        resourceAndMember = prvCpssDxChPortDpIronmanPcaPizzaArbiterSlotToResourceMember(slot);
        resIndex  = resourceAndMember & 0xFFFF;
        slotIndex = (resourceAndMember >> 16) & 0xFFFF;

        resIndexInMap = resourceInMapIndexArr[resIndex];
        if (resIndexInMap == 0xFF) continue;
        if ((resourcesMapArr[resIndexInMap].membersBitmap & (1 << slotIndex)) == 0) continue;

        regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].configuration[slot];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((regData & 0x3F) != 0x3F)
        {
            if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
            {
                break;
            }
            /* overlapping another channel slots */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        arbiterShadowPtr[slot] = channelInfoPtr->channelNum;
        regData &= (~ 0x3F);
        regData |= channelInfoPtr->channelNum;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    channelCfgPtr[channelInfoPtr->channelNum].speedInMbps = channelInfoPtr->speedInMbps;
    channelCfgPtr[channelInfoPtr->channelNum].serdesUsage = channelInfoPtr->serdesUsage;
    channelCfgPtr[channelInfoPtr->channelNum].preemptiveModeActivated = 
        BOOL2BIT_MAC(channelInfoPtr->preemptiveModeActivated);
    channelCfgPtr[channelInfoPtr->channelNum].isPreemtive = BOOL2BIT_MAC(channelInfoPtr->isPreemptive);

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterInit function
* @endinternal
*
* @brief    Initialize PCA Pizza Arbiter unit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaPizzaArbiterInit
(
    IN    GT_U8                                devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    channel;
    GT_U32    slot;
    GT_U8     *arbiterShadowPtr;
    PRV_CPSS_DXCH_SIP6_30_DP_CHANNEL_INFO_STC    *channelCfgPtr;
    PRV_CHANNEL_INFO_STC      channelInfo;

    arbiterShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->pcaArbiterShadow;
    channelCfgPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->channelConfiguration;

    /* pz_cycle_length and pz_cycle_en has to be configured one by one (not together) */

    /* pz_cycle_length */
    regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].control;
    regData = PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* pz_cycle_length and pz_cycle_en */
    regData |= ((1 << 12) | PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* disable all channels */
    for (channel = 0; (channel < PRV_IRONMAN_CHANNELS_NUM_CNS); channel++)
    {
        channelCfgPtr[channel].speedInMbps = 0;
        channelCfgPtr[channel].serdesUsage = 0;
        channelCfgPtr[channel].preemptiveModeActivated = 0;
        channelCfgPtr[channel].isPreemtive = 0;

        regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].channelEnable[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((regData & 1) == 0) continue;
        regData &= (~ 1);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* stamp all slots  */
    for (slot = 0; (slot < PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS); slot++)
    {
        arbiterShadowPtr[slot] = 0x3F;
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].configuration[slot];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((regData & 0x3F) == 0x3F) continue;
        regData |= 0x3F;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Allocate slots to SDMA0 - channel number only relevant for SDMA */
    cpssOsMemSet(&channelInfo, 0, sizeof(channelInfo));
    channelInfo.channelNum = PRV_IRONMAN_SDM0_CNS;
    rc = prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelConfigure(
        devNum, &channelInfo);

    return rc;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelDown function
* @endinternal
*
* @brief    Disable channel on PCA Arbiter unit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                - physical device number
* @param[in] channelInfoPtr        - (pointer to) Channel Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelDown
(
    IN    GT_U8                     devNum,
    IN    PRV_CHANNEL_INFO_STC      *channelInfoPtr
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    slot;
    GT_U8     *arbiterShadowPtr;
    PRV_CPSS_DXCH_SIP6_30_DP_CHANNEL_INFO_STC    *channelCfgPtr;

    arbiterShadowPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->pcaArbiterShadow;
    channelCfgPtr =
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0]->channelConfiguration;

    /* stamp all slots used by the channel */
    for (slot = 0; (slot < PRV_IRONMAN_PIZZA_CYCLES_NUM_CNS); slot++)
    {
        if (arbiterShadowPtr[slot] != channelInfoPtr->channelNum) continue;

        arbiterShadowPtr[slot] = 0x3F;
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].configuration[slot];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        regData |= 0x3F;
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    channelCfgPtr[channelInfoPtr->channelNum].speedInMbps = 0;
    channelCfgPtr[channelInfoPtr->channelNum].serdesUsage = 0;
    channelCfgPtr[channelInfoPtr->channelNum].preemptiveModeActivated = 0;
    channelCfgPtr[channelInfoPtr->channelNum].isPreemtive = 0;
    regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].channelEnable[channelInfoPtr->channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if ((regData & 1) != 0)
    {
        regData &= (~ 1);
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPcaPizzaArbiterDump function
* @endinternal
*
* @brief    Dump PCA Arbiter unit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPcaPizzaArbiterDump
(
    IN    GT_U8                     devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    numOfSlots;
    GT_U32    slot;
    GT_U32    channel;
    GT_U32    index;
    GT_U32    channelBmp[2];

    regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].control;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    numOfSlots = regData & 0x1FF;
    cpssOsPrintf("Arbiter enable %d numOfSlots %d\n\n", ((regData >> 12) & 1), numOfSlots);
    cpssOsPrintf("=================================================\n");
    cpssOsPrintf("    | ");
    for (index = 0; (index < 14); index++)
    {
        cpssOsPrintf("%02d ", prvCpssDxChPortDpIronmanPcaPizzaArbiterResorcesRemapArr[index]);
    }
    cpssOsPrintf("|\n");
    cpssOsPrintf("----|-------------------------------------------|\n");
    cpssOsPrintf("    | 00 01 02 03 04 05 06 07 08 09 10 11 12 13 |\n");
    cpssOsPrintf("----|-------------------------------------------");
    for (slot = 0; (slot < numOfSlots); slot++)
    {
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].configuration[slot];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        channel = regData & 0x3F;
        if ((slot % 14) == 0)
        {
            cpssOsPrintf("|\n%03d | ", slot);
        }
        cpssOsPrintf("%02d ", channel);
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("=================================================\n");

    channelBmp[0] = 0;
    channelBmp[1] = 0;
    for (channel = 0; (channel < 64); channel++)
    {
        regAddr = regsAddrPtr->PCA_PZ_ARBITER[0][0].channelEnable[channel];
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
        if ((regData & 1) == 0) continue;
        channelBmp[channel / 32] |= (1 << (channel % 32));
    }
    cpssOsPrintf(
        "Enabled channels bitmap: 0x%08X 0x%08X\n",
        channelBmp[0], channelBmp[1]);

    return GT_OK;
}

/* TX_FIFO and TX_DMA configurations */

/**
* @internal prvCpssDxChPortDpIronmanDpTxInit function
* @endinternal
*
* @brief    Initialize TX_FIFO and TX_DMA units.
* @brief    Placeholder for future fixes.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanDpTxInit
(
    IN    GT_U8                                devNum
)
{
    GT_UNUSED_PARAM(devNum);
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanDpTxChannelDown function
* @endinternal
*
* @brief    Disable for TX_FIFO and TX_DMA units channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] channelNum             - channel number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanDpTxChannelDown
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               channelNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;

    regAddr = regsAddrPtr->sip6_txDMA[0].configs.channelConfigs.channelReset[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (regData & 1)
    {
        regData &= (~ 1); /* normal state before reset */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* read to delay only */
        rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    regData |= 1; /* assert "reset" to clear credit counters */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* read to delay only */
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    regData &= (~ 1); /* put back normal state */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal prvCpssDxChPortDpIronmanDpTxChannelConfigure function
* @endinternal
*
* @brief    Configure TX_FIFO and TX_DMA units channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] channelNum             - channel number
* @param[in] speedInMbps            - speed in Mega Bit Per Second
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanDpTxChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               channelNum,
    IN    GT_U32                               speedInMbps
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    numOfCredits;
    GT_U32    interPktGap;
    GT_U32    profile;

    /* TBD: set num of credits from PCA to TXF word-send-threshold */
         if (speedInMbps < 5000)  {numOfCredits = 10;}
    else if (speedInMbps < 12000) {numOfCredits = 20;}
    else                          {numOfCredits = 22;}

    regAddr = regsAddrPtr->sip6_txFIFO[0].configs.channelConfigs.txfCredits[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    regData &= (~ 0x03FF);
    regData |= numOfCredits;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* TXD interpacket GAP - default value - 24 bytes for port without DSA tag */
    interPktGap = (24 * 8 * 32);
    regAddr = regsAddrPtr->sip6_txDMA[0].configs.channelConfigs.interGapConfigitation[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    regData &= 0x0000FFFF;
    regData |= (interPktGap << 16);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* TXD speed profile */
         if (speedInMbps <=    10)  {profile = 12;}
    else if (speedInMbps <=   100)  {profile = 11;}
    else if (speedInMbps <=  1000)  {profile = 10;}
    else if (speedInMbps <=  2500)  {profile =  9;}
    else if (speedInMbps <=  5000)  {profile =  8;}
    else if (speedInMbps <= 10000)  {profile =  7;}
    else     /* 12G */              {profile =  6;}

    regAddr = regsAddrPtr->sip6_txDMA[0].configs.channelConfigs.speedProfile[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    regData &= (~ 0xF);
    regData |= profile;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/* RX DMA  - part of configuration in "miscelanous" */

/**
* @internal prvCpssDxChPortDpIronmanDpRxInit function
* @endinternal
*
* @brief    Initialize RX_DMA units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanDpRxInit
(
    IN    GT_U8                                devNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    regNeededVal;
    GT_U32    pbXonTheshold;
    GT_U32    pbXoffTheshold;

    pbXonTheshold  = 0x3FF;
    pbXoffTheshold = 0x3FF;
    regNeededVal = pbXonTheshold | (pbXoffTheshold << 16);

    regAddr = regsAddrPtr->sip6_rxDMA[0].configs.dataAccumulator.PBFillLevelThresholds;
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (regData != regNeededVal)
    {
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regNeededVal);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanDpRxChannelConfigure function
* @endinternal
*
* @brief    Configure RX_DMA units channel.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] channelNum             - channel number
* @param[in] isPreemptive           - GT_TRUE - preemtive, GT_FALSE - other
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
*/
static GT_STATUS prvCpssDxChPortDpIronmanDpRxChannelConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_U32                               channelNum,
    IN    GT_BOOL                              isPreemptive
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_STATUS rc;
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    bitOffset;

    bitOffset = 0;
    regAddr = regsAddrPtr->sip6_rxDMA[0].configs.channelConfig.preemptedConfigs[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    regData &= (~ (1 << bitOffset));
    regData |= (BOOL2BIT_MAC(isPreemptive) << bitOffset);
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}


/* no Packet buffer configurations needed */

/* Configure units sequence per channel */

/**
* @internal prvCpssDxChPortDpIronmanDeviceChannelDown function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                - physical device number
* @param[in] channelInfoPtr        - (pointer to) Channel Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanDeviceChannelDown
(
    IN    GT_U8                     devNum,
    IN    PRV_CHANNEL_INFO_STC      *channelInfoPtr
)
{
    GT_STATUS                     rc;

    if (channelInfoPtr->mifUnitNum < 255)
    {
        rc = prvCpssDxChPortDpIronmanMifChannelMapDisable(
            devNum, channelInfoPtr->mifUnitNum, channelInfoPtr->mifChannelNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* unknown mapping */
        rc = prvCpssDxChPortDpIronmanMifChannelMapFindAndDisable(
            devNum, channelInfoPtr->channelNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChPortDpIronmanPcaSffChannelDown(
        devNum, channelInfoPtr->channelNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelDown(
        devNum, channelInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDpTxChannelDown(
        devNum, channelInfoPtr->channelNum);

    return rc;
}

/**
* @internal prvCpssDxChPortDpIronmanDeviceChannelConfigure function
* @endinternal
*
* @brief    Configure port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                - physical device number
* @param[in] channelInfoPtr        - (pointer to) Channel Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanDeviceChannelConfigure
(
    IN    GT_U8                     devNum,
    IN    PRV_CHANNEL_INFO_STC      *channelInfoPtr
)
{
    GT_STATUS              rc;

    rc = prvCpssDxChPortDpIronmanMifChannelMapSet(
        devNum, channelInfoPtr->mifUnitNum, channelInfoPtr->mifChannelNum,
        channelInfoPtr->channelNum/*localTxDma*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanPcaSffChannelConfigure(
        devNum, channelInfoPtr->channelNum, channelInfoPtr->speedInMbps);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanPcaPizzaArbiterChannelConfigure(
        devNum, channelInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDpTxChannelConfigure(
        devNum, channelInfoPtr->channelNum, channelInfoPtr->speedInMbps);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDpRxChannelConfigure(
        devNum, channelInfoPtr->channelNum, channelInfoPtr->isPreemptive);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* Channel Info from port, mode, speed */

/**
* @internal prvCpssDxChPortDpIronmanPortConfigure function
* @endinternal
*
* @brief    Configure port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  devNum                 - physical device number
* @param[in]  portNum                - physical port number
* @param[in]  failOnInvalidMode      - on invalis speed or mode GT_TRUE - return BAD PARAM,
*                                      GT_FALSE - to continue with unknown values
* @param[in]  ifMode                 - Interface mode,
*                                      not relevant for SDMA because PCA units not configured
* @param[in]  portSpeed              - port speed
* @param[out] channelInfoPtr         - (pointer to) Channel Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanPortChannelnfoGet
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    GT_BOOL                              failOnInvalidMode,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed,
    OUT   PRV_CHANNEL_INFO_STC                 *channelInfoPtr
)
{
    GT_STATUS              rc;
    GT_U32                 globalMac;
    GT_U32                 globalTxDma;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(
        devNum, portNum, globalMac);
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(
        devNum, portNum, globalTxDma);

    /* Ironman has one DP only */
    channelInfoPtr->channelNum = globalTxDma;

    channelInfoPtr->speedInMbps = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);
    channelInfoPtr->serdesUsage = prvCpssCommonPortModeEnumToSerdesUsageUnitsConvert(ifMode);

    if ((channelInfoPtr->speedInMbps != 0) && (channelInfoPtr->serdesUsage != 0))
    {
        rc = prvCpssDxChPortDpIronmanMifMacToUnitAndChannel(
            devNum, globalMac,
            &(channelInfoPtr->mifUnitNum), &(channelInfoPtr->mifChannelNum));
        if (rc != GT_OK)
        {
            return rc;
        }

        /*check if preemption was enabled for the port .
        The assumption is that Port manger enabled/disabled preemption prior to calling this function.
         So reading HW status should give the indication regarding the preemption*/
        channelInfoPtr->preemptiveModeActivated = GT_FALSE;
        rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
            devNum, portNum, channelInfoPtr->speedInMbps,
            &(channelInfoPtr->preemptiveModeActivated));
        if (rc != GT_OK)
        {
             return rc;
        }
    }
    else
    {
        if (failOnInvalidMode != GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        channelInfoPtr->mifUnitNum              = 255;
        channelInfoPtr->mifChannelNum           = 255;
        channelInfoPtr->preemptiveModeActivated = GT_FALSE;
        /* try to obtain preemtion anyway */
        prvCpssDxChTxqSip6_10PreemptionEnableGet(
            devNum, portNum, 1000 /*speedInMbps*/,
            &(channelInfoPtr->preemptiveModeActivated));
    }

    /* physical port mapped to regular or express channel */
    channelInfoPtr->isPreemptive = GT_FALSE;

    return GT_OK;
}

/* Miscelanous related to port */

/**
* @internal prvCpssDxChPortDpIronmanPortMiscConfigure function
* @endinternal
*
* @brief    Configure miscelanous features for given port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
static GT_STATUS prvCpssDxChPortDpIronmanPortMiscConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
)
{
    GT_STATUS  rc;
    GT_U32     speedInMbps;
    GT_U32     minCtByteCount;
    GT_U32     channel;
    GT_U32     globalTxDma;
    GT_U32     globalDmaNum;
    GT_BOOL    preemptiveModeActivated = GT_FALSE;
    GT_U32     preChannel = 0;/*extra channel for preemption*/

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXDMA_NUM_GET_MAC(
        devNum, portNum, globalTxDma);

    /* Ironman has one DP only */
    channel = globalTxDma;

    /* configure minimal Cut Through byte count */
    speedInMbps = prvCpssCommonPortSpeedEnumToMbPerSecConvert(portSpeed);

    /* TBD: copied from Phoenix - should be fixed */
         if (speedInMbps <=  1000)  {minCtByteCount = 512;}
    else if (speedInMbps <=  2500)  {minCtByteCount = 384;}
    else if (speedInMbps <= 10000)  {minCtByteCount = 257;}
    else     /* 12G */              {minCtByteCount = 256;}

    /*check if preemption was enabled for the port .
            The assumption is that Port manger enabled/disabled preemption prior to calling this function.
             So reading HW status should give the indication regarding the preemption*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(
        devNum, portNum, speedInMbps, &preemptiveModeActivated);
    if (rc != GT_OK)
    {
         return rc;
    }

    if (GT_TRUE == preemptiveModeActivated)
    {
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum, speedInMbps, channel, &preChannel);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
        devNum, 0/*unitIndex*/, channel, &globalDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChCutThroughPortMinCtByteCountSet(
        devNum, globalDmaNum, minCtByteCount);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssDxChCutThroughPortMinCtByteCountSet globalDmaNum %d, minCtByteCount %d\n",
            globalDmaNum, minCtByteCount);
    }

    /* source physical port should be configured also for not preemtive      */
    /* to ovrride the value that may be written to HW when it was preemptive */
    rc = prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet(
        devNum, globalDmaNum, portNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet globalDmaNum %d, portNum %d\n",
            globalDmaNum, portNum);
    }

    if (GT_TRUE == preemptiveModeActivated)
    {
        rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(
            devNum, 0/*unitIndex*/, preChannel, &globalDmaNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChCutThroughPortMinCtByteCountSet(
            devNum, globalDmaNum, minCtByteCount);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChCutThroughPortMinCtByteCountSet globalDmaNum %d, minCtByteCount %d\n",
                globalDmaNum, minCtByteCount);
        }

        rc = prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet(
            devNum, globalDmaNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChTxPortSpeedPizzaResourcesRxChannelSourcePortSet globalDmaNum %d, portNum %d\n",
                globalDmaNum, portNum);
        }
    }

    rc = prvCpssDxChFalconPortCutThroughSpeedSet(
        devNum, portNum, portSpeed);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssDxChFalconPortCutThroughSpeedSet portNum %d, speedInMbPerSec %d\n",
            portNum, speedInMbps);
    }

    return GT_OK;
}

/* DP configurations called from CPSS */


/**
* @internal prvCpssDxChPortDpIronmanDeviceInit function
* @endinternal
*
* @brief    Initialyze Ironman DP units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChPortDpIronmanDeviceInit
(
    IN    GT_U8                                devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_SIP6_30_DP_INFO_STC *dpInfoPtr;

    /* allocate per datapath DB - needed for arbiter shadow */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0] == NULL)
    {
        dpInfoPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_SIP6_30_DP_INFO_STC));
        if (dpInfoPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(dpInfoPtr, 0, sizeof(PRV_CPSS_DXCH_SIP6_30_DP_INFO_STC));
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[0] = dpInfoPtr;
    }

    rc = prvCpssDxChPortDpIronmanMifUnitsInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanPcaSffInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanPcaPizzaArbiterInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDpTxInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDpRxInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPortDown function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
)
{
    GT_STATUS                     rc;
    PRV_CHANNEL_INFO_STC          channelInfo;
    PRV_CHANNEL_INFO_STC          preemptiveInfo;
    GT_U32                        globalMac;
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode;
    CPSS_PORT_SPEED_ENT           portSpeed;

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_LOCAL_OR_REMOTE_MAC(
        devNum, portNum, globalMac);
    ifMode     = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, globalMac);
    portSpeed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, globalMac);

    /* values below also supported - needed for rollback when DB yet not updated      */
    /*(portSpeed == CPSS_PORT_SPEED_NA_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)*/

    rc = prvCpssDxChPortDpIronmanPortChannelnfoGet(
        devNum, portNum, GT_FALSE/*failOnInvalidMode*/, ifMode, portSpeed, &channelInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDeviceChannelDown(
        devNum, &channelInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_FALSE != channelInfo.preemptiveModeActivated)
    {
        preemptiveInfo = channelInfo;
        preemptiveInfo.isPreemptive = GT_TRUE;
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum, channelInfo.speedInMbps, channelInfo.channelNum, &(preemptiveInfo.channelNum));
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChPortDpIronmanDeviceChannelDown(
            devNum, &preemptiveInfo);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPortConfigure function
* @endinternal
*
* @brief    Configure port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant for SDMA because PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPortConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
)
{
    GT_STATUS              rc;
    PRV_CHANNEL_INFO_STC   channelInfo;
    PRV_CHANNEL_INFO_STC   preemptiveInfo;

    rc = prvCpssDxChPortDpIronmanPortChannelnfoGet(
        devNum, portNum, GT_TRUE/*failOnInvalidMode*/, ifMode, portSpeed, &channelInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanDeviceChannelConfigure(
        devNum, &channelInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_FALSE != channelInfo.preemptiveModeActivated)
    {
        preemptiveInfo = channelInfo;
        preemptiveInfo.isPreemptive = GT_TRUE;
        /*get additional channel*/
        rc = prvCpssDxChTxqSip6_10PreChannelGet(
            devNum, channelInfo.speedInMbps, channelInfo.channelNum, &(preemptiveInfo.channelNum));
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortDpIronmanMifMacToUnitAndChannel(
            devNum, preemptiveInfo.channelNum,
            &(preemptiveInfo.mifUnitNum),
            &(preemptiveInfo.mifChannelNum));
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortDpIronmanDeviceChannelConfigure(
            devNum, &preemptiveInfo);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChPortDpIronmanPortMiscConfigure(
        devNum, portNum, portSpeed);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFalconPortCutThroughSpeedSet(
        devNum, portNum, portSpeed);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            rc,"error in prvCpssDxChFalconPortCutThroughSpeedSet portNum %d, speedInMbPerSec %d\n",
            portNum, channelInfo.speedInMbps);
    }

    return GT_OK;
}

/* MIF utils called not from DpInit, DpConfigure, DpDown */

/**
* @internal prvCpssDxChPortDpIronmanPortMifPfcEnableSet function
* @endinternal
*
* @brief  Set MIF PFC enable/disable
*
* @param[in] devNum                - system device number
* @param[in] globalMac             - global MAC number
* @param[in] enableTx              - Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[in] enableRx              - Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpIronmanPortMifPfcEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               globalMac,
    IN  GT_BOOL              enableTx,
    IN  GT_BOOL              enableRx
)
{
    GT_STATUS rc;
    GT_U32    mifUnitNum;
    GT_U32    mifChannelNum;

    rc = prvCpssDxChPortDpIronmanMifMacToUnitAndChannel(
        devNum, globalMac, &mifUnitNum, &mifChannelNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanMifChannelPfcEnableGenericSet(
        devNum, mifUnitNum, mifChannelNum, enableTx, enableRx);

    return rc;
}


/**
* @internal prvCpssDxChPortDpIronmanPortMifPfcEnableGet function
* @endinternal
*
* @brief  Get MIF PFC enable/disable
*
* @param[in] devNum                - system device number
* @param[in] globalMac             - global MAC number
* @param[out] enableTx             - (Pointer to)Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[out] enableRx             - (Pointer to)Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpIronmanPortMifPfcEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               globalMac,
    OUT GT_BOOL              *enableTxPtr,
    OUT GT_BOOL              *enableRxPtr
)
{
    GT_STATUS rc;
    GT_U32    mifUnitNum;
    GT_U32    mifChannelNum;

    rc = prvCpssDxChPortDpIronmanMifMacToUnitAndChannel(
        devNum, globalMac, &mifUnitNum, &mifChannelNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortDpIronmanMifChannelPfcEnableGenericGet(
        devNum, mifUnitNum, mifChannelNum, enableTxPtr, enableRxPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanMifUnitChannelDump function
* @endinternal
*
* @brief    Dump MIF unit channel data.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanMifUnitChannelDump
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum
)
{
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32    regAddr;
    GT_U32    regData;
    GT_STATUS rc;

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.mapping[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("mapping: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.rxFifoReadyThershold[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxFifoReadyThershold: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.txCredit[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txCredit: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.config.statusFsmControl[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("statusFsmControl: %0x08X ", regData);

    cpssOsPrintf("\n");

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.status.txStatus[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txStatus: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.status.rxStatus[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxStatus: %0x08X ", regData);

    cpssOsPrintf("\n");

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txGoodPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txGoodPacketCount: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.rxGoodPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxGoodPacketCount: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txBadPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txBadPacketCount: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.rxBadPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxBadPacketCount: %0x08X ", regData);

    cpssOsPrintf("\n");

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txDiscardedPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txDiscardedPacketCount: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.rxDiscardedPacketCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("rxDiscardedPacketCount: %0x08X ", regData);

    regAddr = regsAddrPtr->GOP.MIF[unitNum].channel.statistics.txLinkFailCount[channelNum];
    rc = prvCpssDrvHwPpReadRegister(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("txLinkFailCount: %0x08X ", regData);

    cpssOsPrintf("\n");
    return GT_OK;
}

#ifdef INC_DEBUG_FUNCTIONS

/**
* @internal prvCpssDxChPortDpIronmanPortBitmapDown function
* @endinternal
*
* @brief    Disable set of ports on all DP, PB, PCA and MIF units.
* @note     For debugging only
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] basePortNum            - base physical port number
* @param[in] portIncBmp             - bitmap as set of nombers 0-31 to add to basePortNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPortBitmapDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 basePortNum,
    IN    GT_U32                               portIncBmp
)
{
    GT_STATUS rc;
    GT_U32    i;

    for (i = 0; (i < 32); i++)
    {
        if ((portIncBmp & (1 << i)) == 0) continue;
        rc = prvCpssDxChPortDpIronmanPortDown(
            devNum, (basePortNum + i));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortDpIronmanPortBitmapConfigure function
* @endinternal
*
* @brief    Configure set of ports on all DP, PB, PCA and MIF units.
* @note     For debugging only
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] basePortNum            - base physical port number
* @param[in] portIncBmp             - bitmap as set of nombers 0-31 to add to basePortNum
* @param[in] ifMode                 - Interface mode,
*                                     not relevant for SDMA because PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPortBitmapConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 basePortNum,
    IN    GT_U32                               portIncBmp,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
)
{
    GT_STATUS rc;
    GT_U32    i;

    for (i = 0; (i < 32); i++)
    {
        if ((portIncBmp & (1 << i)) == 0) continue;
        rc = prvCpssDxChPortDpIronmanPortConfigure(
            devNum, (basePortNum + i), ifMode, portSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

#endif /*INC_DEBUG_FUNCTIONS*/



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
* @file cpssDxChCfgInit.c
*
* @brief Core initialization of PPs and shadow data structures, and declarations
* of global variables.
*
* @version   109
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChCfg.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUp.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcam.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChConfigLog.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortBufMg.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPe.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqMemory.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/
#define DXCH_CONFIG_DIR   mainPpDrvMod.dxChConfigDir

#define PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.portDir.commonPortManagerSrc._var)

#define CONFIG_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(DXCH_CONFIG_DIR._var,_value)

#define CONFIG_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(DXCH_CONFIG_DIR._var)

#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

#define PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT \
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.prvDxChTableCpssHwAccessObj)


/* get number of entries of table that HW supports :

_tableType - one of CPSS_DXCH_TABLE_ENT
*/
#define TABLE_NUM_ENTRIES_GET_MAC(_devNum, _tableType) \
    ((_tableType < PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoSize) ?  \
        /* table in valid range of the device */                         \
        PRV_CPSS_DXCH_PP_MAC(_devNum)->accessTableInfoPtr[_tableType].maxNumOfEntries : \
    0)/* unknown table for the device*/

/*******************************************************************************
 * Internal usage variables
 ******************************************************************************/

/* number of entries in the cheetah3 mac2me table */
#define PRV_CPSS_DXCH3_MAC2ME_TABLE_MAX_ENTRIES_CNS 8
#define PRV_CPSS_SIP5_MAC2ME_TABLE_MAX_ENTRIES_CNS 128

/* max number of PBR entries for SIP6 */
#define PRV_CPSS_SIP6_MAX_PBR_ENTRIES_CNS 163840

/* max number of SIP6 PBR entries for minimum LPM configuration */
#define PRV_CPSS_SIP6_MAX_PBR_ENTRIES_FOR_MIN_LPM_CFG_CNS 25600

/* max number of PBR lines for SIP6 */
#define PRV_CPSS_SIP6_MAX_PBR_LINES_CNS PRV_CPSS_SIP6_MAX_PBR_ENTRIES_CNS/PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS

/* max number of pbr banks in hw  */
#define PRV_CPSS_SIP6_MAX_NUM_PBR_BANKS_CNS 8

/* table not valid --> 0 entries */
#define TABLE_NOT_VALID_CNS     0

/* define the number of different FDB messages queues that we may use:
    1. primary AUQ
    2. additional primary AUQ
    3. primary FUQ for FU and CNC0,1
    4. secondary AUQ for primary first AUQ
    5. secondary AUQ for primary second AUQ
    6. primary FUQ for CNC 2,3
*/
#define MESSAGE_QUEUE_NUM_CNS   6

#define QUEUE_NOT_USED_CNS  0xFF

#define DEVICE_NOT_EXISTS_CNS 0xFF

#define NUM_OF_SHADOW_TABLES_CNS 4

extern GT_STATUS prvCpssGenNetIfMiiFreeMemory
(
    IN      PRV_CPSS_GEN_NETIF_MII_POOL_TYPE_ENT    poolIdx,
    IN      CPSS_OS_FREE_FUNC                       freeFunc
);





/**
* @internal prvCpssDxChCfgCalculatePbrBanksNumberAndHwSizeCnfg function
* @endinternal
*
* @brief   This function calculates pbr banks number and pbr bank size
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] bankSize                 - size of bank in lines.
* @param[in] numberOfPbrLines         - number of pbr lines.
*
* @param[out] numberOfPbrBanksPtr     - (pointer to) number of banks intended for pbr.
* @param[out] pbrBankInitSizePtr      - (pointer to) init value pbr bank size.
* @param[out] pbrBankSizePtr          - (pointer to) pbr bank size.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChCfgCalculatePbrBanksNumberAndHwSizeCnfg
(
    IN GT_U32 bankSize,
    IN GT_U32 numberOfPbrLines,
    OUT GT_U32 *numberOfPbrBanksPtr,
    OUT GT_U32 *pbrBankInitSizePtr,
    OUT GT_U32 *pbrBankSizePtr
)
{
    GT_U32 pbrMaxBankNumber = 0;
    *pbrBankSizePtr = bankSize;

    if ( (numberOfPbrLines > PRV_CPSS_SIP6_MAX_NUM_PBR_BANKS_CNS*bankSize) ||
         (numberOfPbrLines > PRV_CPSS_SIP6_MAX_PBR_LINES_CNS) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR entries number");
    }

    if ((numberOfPbrLines > 4*bankSize) && (numberOfPbrLines <= 8*bankSize))
    {
        /* max addressable pbr banks is 8*/
        /* each bank can address 4k*/
        pbrMaxBankNumber = 8;
        *pbrBankInitSizePtr = 0;
        if (bankSize > 4*1024)
        {
            *pbrBankSizePtr = 4*1024;
        }
    }
    else
        if ((numberOfPbrLines > 2*bankSize) && (numberOfPbrLines <= 4*bankSize))
        {
            /* max addressable pbr banks is 4*/
            /* each bank can address 8k*/
            pbrMaxBankNumber = 4;
            *pbrBankInitSizePtr = 1;
            if (bankSize > 8*1024)
            {
                *pbrBankSizePtr = 8*1024;
            }
        }
        else
            if ((numberOfPbrLines > 1*bankSize) && (numberOfPbrLines <= 2*bankSize))
            {
               /* max addressable pbr banks is 2*/
               /* each bank can address 16k*/
               pbrMaxBankNumber = 2;
               *pbrBankInitSizePtr = 2;
               if (bankSize > 16*1024)
               {
                   *pbrBankSizePtr = 16*1024;
               }
            }
            else
                if (numberOfPbrLines <= 1*bankSize)
                {
                    /* max addressable pbr banks is 1*/
                    /* each bank can address 32k*/
                    pbrMaxBankNumber = 1;
                    *pbrBankInitSizePtr = 3;
                    if (bankSize > 32*1024)
                    {
                        *pbrBankSizePtr = 32*1024;
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR configuration");
                }
     *numberOfPbrBanksPtr = (numberOfPbrLines +(*pbrBankSizePtr-1))/(*pbrBankSizePtr);
     if (*numberOfPbrBanksPtr > pbrMaxBankNumber)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR configuration");
     }
     return GT_OK;
}

/**
* @internal prvCpssDxChCfgCalculatePbrIndexAndBankSize function
* @endinternal
*
* @brief   This function calculates pbr start index and pbr bank size
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - The device number.
* @param[out] pbrStartIndexPtr        - (pointer to) pbr start index.
* @param[out] pbrInitBankSizePtr      - (pointer to) init value of pbr bank size.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssDxChCfgCalculatePbrIndexAndBankSize
(
    IN GT_U8     devNum,
    OUT GT_U32   *pbrStartIndexPtr,
    OUT GT_U32   *pbrInitBankSizePtr
)
{
    GT_U32 numberOfPbrLines;
    GT_U32 bigBanksNumber;
    GT_U32 smallBanksNumber;
    GT_U32 bigBanksSize;
    GT_U32 smallBankSize = 640;
    GT_U32 numOfBigPbrBlocks = 0;
    GT_U32 numOfSmallPbrBlocks = 0;
    GT_U32 pbrBankSize=0;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;
    GT_STATUS rc = GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(pbrStartIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(pbrInitBankSizePtr);

    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
    if (moduleCfgPtr->ip.maxNumOfPbrEntries == 0)
    {
        moduleCfgPtr->ip.numOfBigBanks = moduleCfgPtr->ip.lpmSharedMemoryBankNumber;
        if (moduleCfgPtr->ip.numOfBigBanks >= 10)
        {
            moduleCfgPtr->ip.numOfSmallBanks = 30 - moduleCfgPtr->ip.numOfBigBanks;
        }
        else
        {
            moduleCfgPtr->ip.numOfSmallBanks = 20;
        }
        return GT_OK;
    }
    numberOfPbrLines = moduleCfgPtr->ip.maxNumOfPbrEntries / PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS;
    if (moduleCfgPtr->ip.maxNumOfPbrEntries%PRV_CPSS_DXCH_CFG_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS != 0)
    {
        numberOfPbrLines++;
    }
    moduleCfgPtr->ip.numberOfPbrLines = numberOfPbrLines;
     /* there is no case when big blocks and small blocks are used together for PBR: or small blocks,
       or big blocks */
    /* big banks for PBR is taken from the beginning of bank addresses, small banks from the end */
    if (moduleCfgPtr->ip.lpmSharedMemoryBankNumber == 0)
    {
        /* only small banks */
        bigBanksNumber = 0;
        bigBanksSize = 0;
        smallBanksNumber = 20;
        *pbrInitBankSizePtr = 0; /* 4K */
        rc = prvCpssDxChCfgCalculatePbrBanksNumberAndHwSizeCnfg(smallBankSize,numberOfPbrLines,
                                                                &numOfSmallPbrBlocks,pbrInitBankSizePtr,&pbrBankSize);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad lpm-pbr configuration");
        }
    }
    else
    {
        /* there are big banks */
        bigBanksNumber = moduleCfgPtr->ip.lpmSharedMemoryBankNumber;
        bigBanksSize = moduleCfgPtr->ip.lpmBankSize;
        smallBanksNumber = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared/*30*/ - bigBanksNumber;
        if(smallBanksNumber > PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared)
        {
            /* limited to 20 small banks */
            smallBanksNumber = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared;
        }


        if (numberOfPbrLines <= smallBankSize)
        {
            numOfBigPbrBlocks = 0;
            numOfSmallPbrBlocks = 1;
            *pbrInitBankSizePtr = 0; /* 4K */
            pbrBankSize = 640;
        }
        else
        {

            rc = prvCpssDxChCfgCalculatePbrBanksNumberAndHwSizeCnfg(bigBanksSize,numberOfPbrLines,
                                                                    &numOfBigPbrBlocks,pbrInitBankSizePtr,&pbrBankSize);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad lpm-pbr configuration");
            }
        }
    }


    moduleCfgPtr->ip.numOfBigBanks = bigBanksNumber - numOfBigPbrBlocks;
    moduleCfgPtr->ip.numOfSmallBanks = smallBanksNumber - numOfSmallPbrBlocks;
    moduleCfgPtr->ip.numOfBigPbrBanks = numOfBigPbrBlocks;
    moduleCfgPtr->ip.numOfSmallPbrBanks = numOfSmallPbrBlocks;
    moduleCfgPtr->ip.pbrBankSize = pbrBankSize;
    if (numOfBigPbrBlocks > 0)
    {
        /* big blocks for PBR is taken from begining */
        *pbrStartIndexPtr = 0;
    }
    if (numOfSmallPbrBlocks > 0)
    {
        /* small banks is starting from position 10 and ending on position 30.*/
        /* We have 30 banks on address lane and taking for PBR from the end  */
        *pbrStartIndexPtr = (30 - numOfSmallPbrBlocks)*0x80000;
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChCfgInitAuqEnableDisable function
* @endinternal
*
* @brief   AUQ enable/disable sequence to allow to re-write the AUQ/FUQ addresses
*   of start of queue, and not wait for the AUQ to get to full for 'rewind'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Catus; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - the port group Id
* @param[in] enable                   - enable/disable
* @param[OUT] isAuqDisabledPtr        - is the AUQ actully need to be disabled and enabled
*
*
* @retval GT_OK                    - on success
* @retval otherwise                - HW error
*/
GT_STATUS prvCpssDxChCfgInitAuqEnableDisable
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_BOOL  enable,
    OUT GT_BOOL *isAuqDisabledPtr
)
{
    GT_STATUS                      rc = GT_OK; /* return code */
    GT_U32                         regAddr;
    GT_U32                         mgUnitId;
    GT_U32                         auq_portGroupsBmp;
    GT_U32                         fuq_portGroupsBmp;
    GT_U32                         regValue,regValue1;

    *isAuqDisabledPtr = GT_FALSE;


    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum) &&
        !PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        /* nothing to do - the device not support it */
        return GT_OK;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* AUQ and FUQ */
        auq_portGroupsBmp = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp;
        fuq_portGroupsBmp = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp;

        if(0 == ((auq_portGroupsBmp | fuq_portGroupsBmp) & (1<<portGroupId)))
        {
            /* nothing to do */
            return GT_OK;
        }
    }

    rc = prvCpssDxChHwPortGroupToMgUnitConvert(devNum,portGroupId,PRV_CPSS_DXCH_MG_CLIENT_FDB_ONLY_E,
        &mgUnitId);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_hostConfig;
    if(enable == GT_TRUE)
    {
        /* Reset the NA/FU counters */
        rc = prvCpssHwPpMgSetRegField(devNum, mgUnitId, regAddr, 1, 2, 3);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpMgRegBusyWaitByMask(devNum, mgUnitId, regAddr, 1 << 1, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssHwPpMgRegBusyWaitByMask(devNum, mgUnitId, regAddr, 1 << 2, GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        /* check both AUQ and FUQ to support MGs with only FUQ (that not serve AUQ) */
        rc = prvCpssHwPpMgReadReg(devNum, mgUnitId,
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr,
            &regValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpMgReadReg(devNum, mgUnitId,
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr,
            &regValue1);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(regValue == 0 && regValue1 == 0)
        {
            /* nothing to do as the AUQ and FUQ were not initialized before */
            return GT_OK;
        }

        /* AUQ or FUQ require force 'reset' to allow rewind */
        *isAuqDisabledPtr = GT_TRUE;
    }
    rc = prvCpssHwPpMgSetRegField(devNum, mgUnitId, regAddr, 0, 1, 1 - BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal internalCfgAddrUpInit function
* @endinternal
*
* @brief   This function initializes the Core Address Update mechanism, by
*         allocating the AU descs. block.
*         supports 'per port group'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to init the Au unit for.
* @param[in] portGroupId              - the port group Id
* @param[in] auDescBlockPtr           - (pointer to)A block to allocate the Au descriptors from.
* @param[in] auDescBlockSize          - Size in bytes of Au descBlock.
* @param[in] fuDescBlockPtr           - (pointer to)A block to allocate the Fu descriptors from.
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] fuDescBlockSize          - Size in bytes of Fu descBlock.
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] useFuQueue               - GT_TRUE  - Fu queue is used - for DxCh2 devices only.
*                                      - GT_FALSE - Fu queue is unused.
* @param[in] au1DescBlockPtr          - (pointer to)A block to allocate the additional Au descriptors from.
* @param[in] au1DescBlockSize
*                                      - Size in bytes of Au1 descBlock.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
* @param[in] cnc23FuDescBlockPtr      - (pointer to)A block to allocate the Fu descriptors for the CNC2,3 upload
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] cnc23FuDescBlockSize     - Size in bytes of Fu descBlock  for the CNC2,3 upload
*                                      Valid if useFuQueue is GT_TRUE.
*/
static GT_STATUS internalCfgAddrUpInit
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U8    *auDescBlockPtr,
    IN GT_U32   auDescBlockSize,
    IN GT_U8    *fuDescBlockPtr,
    IN GT_U32   fuDescBlockSize,
    IN GT_BOOL  useFuQueue,
    IN GT_U8    *au1DescBlockPtr,
    IN GT_U32   au1DescBlockSize,
    IN GT_U8    *cnc23FuDescBlockPtr,
    IN GT_U32   cnc23FuDescBlockSize
)
{
    PRV_CPSS_AU_DESC_CTRL_STC   *descCtrl[MESSAGE_QUEUE_NUM_CNS] = {NULL};    /* Points to the desc. list. */
    PRV_CPSS_AU_DESC_CTRL_STC   *tmpDescCtrl[MESSAGE_QUEUE_NUM_CNS] = {NULL}; /* Points to the desc. list. */
    GT_UINTPTR                  phyAddr;/* The physical address of the AU block*/
    GT_STATUS                   retVal;
    GT_U32                      numOfIter = 0;/* the number of iterations in initialization */
    GT_U32                      ii;          /* iterator */
    PRV_CPSS_AU_DESC_STC        *addrUpMsgPtr = NULL;   /* update message address */
    GT_U32                      *auMemPtr = NULL;/* pointer to the SDMA memory , to start of current message */
    GT_U8                       *descBlock[MESSAGE_QUEUE_NUM_CNS] = {NULL};  /* A block to allocate the Au/Fu descriptors from.*/
    GT_U32                      descBlockSize[MESSAGE_QUEUE_NUM_CNS] = {0}; /* Size in bytes of Fu/Au descBlock. */
    GT_BOOL                     initHwPointerAfterShutdown = GT_FALSE;
    GT_BOOL                     didHwReset;/* are we after HW reset or only after 'cpss SW re-init' (without HW reset since last call of this function)*/
    GT_U32                      alignmentNumBytes;/* number of bytes for alignment movement*/
    GT_U32                      auqIndex = 0;/*index for the AU in the arrays of : descCtrl,descBlock,descBlockSize*/
    GT_U32                      auq1Index = QUEUE_NOT_USED_CNS;/*index for the additional AU in the arrays of : descCtrl,descBlock,descBlockSize*/
    GT_U32                      fuqIndex = 0;/*index for the FU in the arrays of : descCtrl,descBlock,descBlockSize*/
    GT_U32                      cnc23FuqIndex = QUEUE_NOT_USED_CNS;/*index for the CNC2,3 FU in the arrays of : descCtrl,descBlock,descBlockSize*/
    GT_U32  mgUnitArr[MESSAGE_QUEUE_NUM_CNS] = {0};       /* the MG units to serve the AU/FU/CNC */
    GT_U32  addrUpdateQueueCtrl[MESSAGE_QUEUE_NUM_CNS] = {0};
    GT_U32  addrUpdateQueueBaseAddr[MESSAGE_QUEUE_NUM_CNS]= {0};
    GT_U32  secondaryAuqIndex = QUEUE_NOT_USED_CNS;/* index in the descCtrl[] that the secondary uses ,
                                    initialized with unreachable index value */
    GT_U32  secondaryAuq1Index = QUEUE_NOT_USED_CNS;/* index in the descCtrl[] that the secondary for additional AUQ uses,
                                    initialized with unreachable index value */
    GT_U8   *secondaryAuqDescBlockPtr = NULL;/*pointer to the secondary AUQ block */
    GT_U32  secondaryAuqDescBlockSize;/*size in bytes of the secondary AUQ block */
    GT_U32  regAddr;                  /* register address */
    GT_U32 auMessageNumBytes; /* number of bytes in AU/FU message */
    CPSS_SYSTEM_RECOVERY_INFO_STC  oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_BOOL auqRecoverySkipInit = GT_FALSE;
    GT_BOOL fuqRecoverySkipInit = GT_FALSE;
    GT_BOOL doSdma;/* do SDMA or onChipFifo */
    GT_BOOL initAuq = GT_TRUE;/* indication to init the AUQ on the device */
    GT_U32  mgUnit_auq;/* MGs that serve the port group for AUQ */
    GT_U32  mgUnit_fuq;/* MGs that serve the port group for FUQ , CNC0,1*/
    GT_U32  mgUnit_fuq_cnc23=0;/* MGs that serve the port group for CNC2,3 */
    GT_BOOL isAuqDisabled = GT_FALSE;/* is the AUQ disabled by prvCpssDxChCfgInitAuqEnableDisable */


    /* save recovery state*/
    retVal = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (retVal != GT_OK)
    {
        return retVal;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    if ((oldSystemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E) &&
        (oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E))
    {
        /* recovery process is in progress */
        if(oldSystemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_TRUE)
        {
          /* auq  continues to receive packets, no init required  */
            auqRecoverySkipInit = GT_TRUE;
        }
        if(oldSystemRecoveryInfo.systemRecoveryMode.continuousFuMessages == GT_TRUE)
        {
            /* fuq  continues to receive packets, no init required  */
            fuqRecoverySkipInit = GT_TRUE;
        }
        if ((auqRecoverySkipInit == GT_FALSE) ||(fuqRecoverySkipInit == GT_FALSE))
        {
            /* in this case we need perform hw write for full initialization */
            newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
            prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
        }
    }

    auMessageNumBytes = 4 * PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;

    if((PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum)) &&
       (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                   info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E.
                       enabled == GT_FALSE))
    {
        doSdma = GT_TRUE;
    }
    else
    {
        doSdma = GT_FALSE;
    }

    if(auDescBlockPtr == NULL)
    {
        initAuq = GT_FALSE;
    }

    if(doSdma == GT_TRUE)
    {
        /* AUQ/FUQ : convert the port-group to the MG unit */
        retVal = prvCpssDxChHwPortGroupToMgUnitConvert(devNum,portGroupId,PRV_CPSS_DXCH_MG_CLIENT_FDB_ONLY_E,
            &mgUnit_auq);
        if(retVal != GT_OK)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            return retVal;
        }

        mgUnit_fuq = mgUnit_auq;/* even if no AUQ in this port group , still the 'mgUnit_auq' is valid here */

        if(cnc23FuDescBlockPtr)
        {
            /* CNC2,3 : convert the port-group to the MG unit */
            retVal = prvCpssDxChHwPortGroupToMgUnitConvert(devNum,portGroupId,PRV_CPSS_DXCH_MG_CLIENT_FUQ_FOR_CNC23_E,
                &mgUnit_fuq_cnc23);
            if(retVal != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                return retVal;
            }
        }

        if (initAuq == GT_TRUE && auqRecoverySkipInit == GT_FALSE)
        {
#if 0 /* allow Falcon to init this 'pipe' only for FUQ without AUQ */
            if(auDescBlockPtr == NULL)
            {
                /* no Hw changes needed */
                /* nothing more to do */
                return GT_OK;
            }
#endif
            auqIndex = numOfIter;

            /* AUQ registers */
            addrUpdateQueueCtrl[numOfIter] =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;
            addrUpdateQueueBaseAddr[numOfIter] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;
            mgUnitArr[numOfIter] = mgUnit_auq;

            descCtrl[auqIndex] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId]);
            descBlock[auqIndex] = auDescBlockPtr;
            descBlockSize[auqIndex] = auDescBlockSize;

            numOfIter++;

        }
        if (fuqRecoverySkipInit == GT_FALSE)
        {
            if(useFuQueue == GT_TRUE)
            {
                /* FUQ registers */
                addrUpdateQueueCtrl[numOfIter] =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
                addrUpdateQueueBaseAddr[numOfIter] =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
                mgUnitArr[numOfIter] = mgUnit_fuq;

                fuqIndex = numOfIter;
                descCtrl[fuqIndex] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId]);
                descBlock[fuqIndex] = fuDescBlockPtr;
                descBlockSize[fuqIndex] = fuDescBlockSize;

                numOfIter++;

                if(cnc23FuDescBlockPtr)
                {
                    /* the device supports CNC2,3 uploads via other MG than this used for : FU and CNC2,3 */
                    addrUpdateQueueCtrl[numOfIter] =
                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
                    addrUpdateQueueBaseAddr[numOfIter] =
                        PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
                    mgUnitArr[numOfIter] = mgUnit_fuq_cnc23;

                    cnc23FuqIndex = numOfIter;
                    descCtrl[cnc23FuqIndex] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[portGroupId]);
                    descBlock[cnc23FuqIndex] = cnc23FuDescBlockPtr;
                    descBlockSize[cnc23FuqIndex] = cnc23FuDescBlockSize;

                    numOfIter++;
                }
            }
            else
            {
                fuqIndex      = QUEUE_NOT_USED_CNS;/*not relevant*/
            }
        }

        if (initAuq == GT_TRUE && auqRecoverySkipInit == GT_FALSE)
        {
            /* Initialize primary AUQ1 descriptor block */
            /* do it only after AUQ and FUQ due to operation of prvCpssDxChCfgInitAuqEnableDisable() */
            if(au1DescBlockPtr)
            {
                auq1Index = numOfIter;

                /* AUQ registers */
                addrUpdateQueueCtrl[numOfIter] =
                    PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;

                addrUpdateQueueBaseAddr[numOfIter] = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;
                mgUnitArr[numOfIter] = mgUnit_auq;

                descCtrl[auq1Index] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl[portGroupId]);
                descBlock[auq1Index] = au1DescBlockPtr;
                descBlockSize[auq1Index] = au1DescBlockSize;

                numOfIter++;
            }

            /* check if need to set the secondary AUQ for WA */
            if(PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                 PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E) == GT_TRUE)
            {
                descCtrl[numOfIter] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId]);

                secondaryAuqDescBlockSize =
                    CPSS_DXCH_PP_FDB_AUQ_DEADLOCK_EXTRA_MESSAGES_NUM_WA_CNS *
                    auMessageNumBytes;

                if(auDescBlockSize <= secondaryAuqDescBlockSize)
                {
                    /* caller not giving enough resources */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }

                /* we need to take the needed size from the primary AUQ */
                secondaryAuqDescBlockPtr = auDescBlockPtr +
                    (auDescBlockSize - secondaryAuqDescBlockSize);

                /* update the primary AUQ size */
                auDescBlockSize -= secondaryAuqDescBlockSize;
                descBlockSize[auqIndex] = auDescBlockSize;

                /* NOTE : the auDescBlock is already aligned so we not need to align
                   the secondaryAuqDescBlockPtr
                */

                descBlock[numOfIter] = secondaryAuqDescBlockPtr;
                descBlockSize[numOfIter] = secondaryAuqDescBlockSize;

                /* use the same registers as the primary AUQ */
                addrUpdateQueueCtrl[numOfIter] = addrUpdateQueueCtrl[auqIndex];
                addrUpdateQueueBaseAddr[numOfIter] = addrUpdateQueueBaseAddr[auqIndex];
                mgUnitArr[numOfIter] = mgUnitArr[auqIndex];

                secondaryAuqIndex = numOfIter;

                numOfIter++;

                /* Initialize secondary AUQ1 descriptor block */
                if(au1DescBlockPtr)
                {
                    if(au1DescBlockSize <= secondaryAuqDescBlockSize)
                    {
                        /* caller not giving enough resources */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                    }

                    descCtrl[numOfIter] =
                        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl[portGroupId]);

                    /* we need to take the needed size from the additional primary AUQ */
                    secondaryAuqDescBlockPtr = au1DescBlockPtr +
                        (au1DescBlockSize - secondaryAuqDescBlockSize);

                    /* update the additional primary AUQ size */
                    au1DescBlockSize -= secondaryAuqDescBlockSize;
                    descBlockSize[auq1Index] = au1DescBlockSize;

                    /* NOTE : the au1DescBlock is already aligned so we don't need to align
                       the secondaryAuqDescBlockPtr
                    */
                    descBlock[numOfIter] = secondaryAuqDescBlockPtr;
                    descBlockSize[numOfIter] = secondaryAuqDescBlockSize;

                    /* use the same registers as the primary AUQ1 */
                    addrUpdateQueueCtrl[numOfIter] = addrUpdateQueueCtrl[auq1Index];
                    addrUpdateQueueBaseAddr[numOfIter] = addrUpdateQueueBaseAddr[auq1Index];
                    mgUnitArr[numOfIter] = mgUnitArr[auq1Index];

                    secondaryAuq1Index = numOfIter;

                    numOfIter++;
                }
            }
        }
        if(numOfIter == 0)
        {
            /* nothing more to do */
            goto exit_cleanly_lbl;
        }
        /* get that state of the device , is the device after HW reset and
           not did the AU/FU queues settings */
        retVal = prvCpssPpConfigDevDbHwResetGet(devNum,&didHwReset);
        if(retVal != GT_OK)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            return retVal;
        }
        /* if system recovery is in progress the following block is skipped */
        if (oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)
        {
            /* we need to get the values that were last used , when the HW didn't do reset ,
               and when the DB of descCtrl[0]->blockAddr == 0 (not initialized yet)  */
            if(descCtrl[0]->blockAddr == 0 && didHwReset == GT_FALSE)

            {
                tmpDescCtrl[0] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[portGroupId]);
                tmpDescCtrl[1] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[portGroupId]);
                tmpDescCtrl[2] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[portGroupId]);
                tmpDescCtrl[3] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl[portGroupId]);
                tmpDescCtrl[4] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl[portGroupId]);
                tmpDescCtrl[5] = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[portGroupId]);

                /* get the info to synch with current HW state
                   NOTE: the 'save' of this info was done in
                   prvCpssPpConfigDevDbPrvInfoSet(...) */
                retVal = prvCpssPpConfigDevDbPrvInfoGet(devNum,portGroupId,
                                                        tmpDescCtrl[0], /*  auqInternalInfoPtr,            */
                                                        tmpDescCtrl[1], /*  fuqInternalInfoPtr,            */
                                                        tmpDescCtrl[2], /*  secondaryAuqInternalInfoPtr,   */
                                                        tmpDescCtrl[3], /*  auq1InternalInfoPtr,           */
                                                        tmpDescCtrl[4], /*  secondaryAuq1InternalInfoPtr,  */
                                                        tmpDescCtrl[5], /*  cnc23FuqInternalInfoPtr,       */
                                                        &PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId],
                                                        &PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroupId]);
                if (retVal != GT_OK)
                {
                     prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                     return retVal;
                }


                if(auqIndex < MESSAGE_QUEUE_NUM_CNS)
                {
                    descCtrl[auqIndex] = tmpDescCtrl[0];
                }

                if(fuqIndex < MESSAGE_QUEUE_NUM_CNS)
                {
                    descCtrl[fuqIndex] = tmpDescCtrl[1];
                }

                if(secondaryAuqIndex < MESSAGE_QUEUE_NUM_CNS)
                {
                    descCtrl[secondaryAuqIndex] = tmpDescCtrl[2];
                }

                if(auq1Index < MESSAGE_QUEUE_NUM_CNS)
                {
                    descCtrl[auq1Index] = tmpDescCtrl[3];
                }

                if(secondaryAuq1Index < MESSAGE_QUEUE_NUM_CNS)
                {
                    descCtrl[secondaryAuq1Index] = tmpDescCtrl[4];
                }

                if(cnc23FuqIndex < MESSAGE_QUEUE_NUM_CNS)
                {
                    descCtrl[cnc23FuqIndex] = tmpDescCtrl[5];
                }

             }
        }
        if(fuqRecoverySkipInit == GT_FALSE)
        {
            if (oldSystemRecoveryInfo.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                for(ii = 0; ii < numOfIter; ii++)
                {
                    /* enable/disable  FU queue */
                    retVal = prvCpssHwPpMgSetRegField(devNum,
                           mgUnitArr[ii],
                           PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl,
                           31,1,
                           (useFuQueue == GT_TRUE) ? 1 : 0);
                    if (retVal != GT_OK)
                    {
                         prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                         return retVal;
                    }
                }
            }
        }


        for(ii = 0; ii < numOfIter; ii++)
        {
            /* if device is not after HW reset (meaning we already set the device
               with AU/FU queue size and base address , and we cant set those
               values again to HW , we only need to clean the AU/FU queues from
               leftovers messages) */
            if(( didHwReset == GT_FALSE)&&(oldSystemRecoveryInfo.systemRecoveryProcess!=CPSS_SYSTEM_RECOVERY_PROCESS_HA_E))
            {
                /* the pointer to start of 'next message to handle'  */
                auMemPtr = (GT_U32 *)(descCtrl[ii]->blockAddr + (auMessageNumBytes * descCtrl[ii]->currDescIdx));

                 /* point descPtr to the current AU descriptor in the queue */
                addrUpMsgPtr = (PRV_CPSS_AU_DESC_STC*)auMemPtr;

                /* handle AUQ or FUQ (when no CNC used) */
                if((ii == auqIndex)|| (ii == auq1Index) || /*AUQ*/
                    (ii == fuqIndex /*FUQ*/&& (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum)) &&
                     0 == descCtrl[ii]->unreadCncCounters)||
                    (ii == secondaryAuqIndex) || (ii == secondaryAuq1Index)/*secondary AUQ*/)
                {
                    /* Clear the AU queue until the HW pointer */
                    while (!AU_DESC_IS_NOT_VALID(addrUpMsgPtr))
                    {
                        descCtrl[ii]->currDescIdx =
                                    ((descCtrl[ii]->currDescIdx + 1) %
                                                    descCtrl[ii]->blockSize);
                        AU_DESC_RESET_MAC(addrUpMsgPtr);

                        auMemPtr = (GT_U32 *)(descCtrl[ii]->blockAddr + (auMessageNumBytes * descCtrl[ii]->currDescIdx));

                        /* prevent infinite loop */
                        if(descCtrl[ii]->currDescIdx == 0)
                        {
                            /* HW pointer at the end of AU/FU queue =>
                              Need to rewind it */
                            initHwPointerAfterShutdown = GT_TRUE;
                            break;
                        }
                    }
                }
                else if(ii == fuqIndex || ii == cnc23FuqIndex)/* relevant to CNC counters only */
                {
                    if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum) &&
                       descCtrl[ii]->unreadCncCounters)
                    {
                        /*since we may have CNC messages on queue , clear it also */

                        if(descCtrl[ii]->unreadCncCounters & 1)
                        {
                            /* make the number even , because 2 counters in an entry */
                            descCtrl[ii]->unreadCncCounters ++;
                        }

                        while(descCtrl[ii]->unreadCncCounters)
                        {
                            descCtrl[ii]->unreadCncCounters -=2;/* 2 CNC counters in 'Single entry' */

                            descCtrl[ii]->currDescIdx =
                                        ((descCtrl[ii]->currDescIdx + 1) %
                                                        descCtrl[ii]->blockSize);
                            AU_DESC_RESET_MAC(addrUpMsgPtr);

                            auMemPtr = (GT_U32 *)(descCtrl[ii]->blockAddr + (auMessageNumBytes * descCtrl[ii]->currDescIdx));

                            /* prevent infinite loop */
                            if(descCtrl[ii]->currDescIdx == 0)
                            {
                                /* HW pointer at the end of AU/FU queue =>
                                  Need to rewind it */
                                initHwPointerAfterShutdown = GT_TRUE;
                                break;
                            }
                        }

                        descCtrl[ii]->unreadCncCounters = 0;
                    }
                }

                if (initHwPointerAfterShutdown == GT_FALSE)
                {
                    /* go to the next iteration */
                    continue;
                }
           }

            /* Set the descBlock to point to an aligned start address. */
            if((((GT_UINTPTR)descBlock[ii]) % auMessageNumBytes) != 0)
            {
                alignmentNumBytes =  (auMessageNumBytes -
                                 (((GT_UINTPTR)descBlock[ii]) % auMessageNumBytes));
                descBlockSize[ii] -= alignmentNumBytes;/* update size according to new buffer alignment*/
                descBlock[ii]     += alignmentNumBytes;/* update buffer according to alignment */
            }

            descCtrl[ii]->currDescIdx = 0;
            descCtrl[ii]->blockAddr = (GT_UINTPTR)descBlock[ii];
            descCtrl[ii]->blockSize = descBlockSize[ii] / auMessageNumBytes;/*num of entries in the block*/
            descCtrl[ii]->unreadCncCounters = 0;
            descCtrl[ii]->mgUnitId = mgUnitArr[ii];

            cpssOsMemSet(descBlock[ii],0,sizeof(GT_U8) * descBlockSize[ii]);

            if((ii == secondaryAuqIndex) || (ii == secondaryAuq1Index))
            {
                /* no more to do for this queue as we not do any HW writings for
                    this queue */
                continue;
            }

            if(ii == 0)
            {
                /* check if need to reset the AUQ and/or FUQ in the MG  */
                /* this in case that HW did not so reset of the MG units */
                /* Stop the AUQ - allow it to be set with new AUQ/FUQ parameters */
                retVal  = prvCpssDxChCfgInitAuqEnableDisable(devNum, portGroupId,GT_FALSE/*disable*/,
                    &isAuqDisabled);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
            }

            retVal = prvCpssHwPpMgSetRegField(devNum,
                   mgUnitArr[ii],
                   addrUpdateQueueCtrl[ii],0,31,
                   descCtrl[ii]->blockSize);/*num of entries in the block*/
            if (retVal != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                return retVal;
            }
            retVal = cpssOsVirt2Phy(descCtrl[ii]->blockAddr,/*OUT*/&phyAddr);
            if (retVal != GT_OK)
            {
                return retVal;
            }

            #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
                if (0 != (phyAddr & 0xffffffff00000000L))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
            #endif

            retVal = prvCpssHwPpMgWriteReg(devNum,mgUnitArr[ii],
                                       addrUpdateQueueBaseAddr[ii],(GT_U32)phyAddr);
            if (retVal != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                return retVal;
            }

            if(isAuqDisabled == GT_TRUE &&
               (ii     == fuqIndex  ||/* we just did FUQ (comes after auq) */
                (ii+1) == auq1Index ||/* next is AUQ1 (so there was no FUQ) */
                (ii+1) == numOfIter))  /* no more queues so no FUQ and no AUQ1 */
            {
                /* need to reanable as it was disabled */
                /* this need to be done after setting the base and size of AUQ and FUQ
                   and before the AUQ1 */

                /* Re-enable the AUQ - after seting new AUQ/FUQ parameters */
                retVal  = prvCpssDxChCfgInitAuqEnableDisable(devNum, portGroupId,GT_TRUE/*enable*/,&isAuqDisabled);
                if (retVal != GT_OK)
                {
                    return retVal;
                }
                /* make sure that not entering here , any more */
                isAuqDisabled = GT_FALSE;
            }

            if ( (oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E ) ||
                 (oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) )
            {
                 /* if HSU or HA process is in progress */

                if ((auqRecoverySkipInit == GT_FALSE) ||(fuqRecoverySkipInit == GT_FALSE))
                {
                    /* if non-continuous mode is chosen */
                    /* clear all possible FIFO messages in AUQ */
                    if(ii == auqIndex)
                    {
                        cpssOsTimerWkAfter(1);
                        /* the pointer to start of 'next message to handle'  */
                        addrUpMsgPtr = (PRV_CPSS_AU_DESC_STC *)(descCtrl[ii]->blockAddr + (auMessageNumBytes * descCtrl[ii]->currDescIdx));

                        /* Clear the AU queue until the HW pointer */
                        while (!AU_DESC_IS_NOT_VALID(addrUpMsgPtr))
                        {
                            descCtrl[ii]->currDescIdx =
                                        ((descCtrl[ii]->currDescIdx + 1) %
                                                        descCtrl[ii]->blockSize);
                            AU_DESC_RESET_MAC(addrUpMsgPtr);

                            addrUpMsgPtr = (PRV_CPSS_AU_DESC_STC*)(descCtrl[ii]->blockAddr + (auMessageNumBytes * descCtrl[ii]->currDescIdx));
                        }
                    }
                }
            }
        }
    }
    else
    {

        /* use FIFO registers not PCI.
           change auQBaseAddr register address to use FIFO.
           do not change auQControl register address. */
        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr =
                PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBAddrUpdateMsgs.AUMsgToCPU.AUMsgToCPU;
        }
        else if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr= 0x0B000034;
        }
        else
        {
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr= 0x06000034;
        }

        if(PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(devNum))
        {
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
            }
            else
            {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bridgeRegs.macControlReg;
        }
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.globalControl;
        }

       /* For DxCh1 and DxCh2 disable PCI interface
        in the Global Control register.
        For DxCh3 and above devices disable "AUMsgsToCPUIF"
        in FDB Global Configuration register. */
        retVal = prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,
                 regAddr,
                 20,1,0);
        if (retVal != GT_OK)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            return retVal;
        }
    }

exit_cleanly_lbl:
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);

    return GT_OK;
}

/**
* @internal internal_cpssDxChCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU , and
*         messages (AUQ) , prior to this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgDevRemove
(
    IN GT_U8   devNum
)
{
    GT_STATUS                                   rc;
    CPSS_PP_FAMILY_TYPE_ENT                     devFamily;
    GT_U32                                      ii;
    CPSS_PP_DEVICE_TYPE                         devType;
    GT_BOOL                                     useMii;
    CPSS_DXCH_TABLE_ENT                         hwTable;
    PRV_CPSS_DXCH_BRG_L2_DLB_WA_LTT_USAGE_STC   *dlbWaUsageNodePtr, *dlbWaUsageNodeNextPtr;

    /* check that the device still exists */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /*store values needed for later use */
    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;
    devType = PRV_CPSS_PP_MAC(devNum)->devType;

    useMii =((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
             (PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
             PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum) ? GT_TRUE : GT_FALSE);

    rc = prvCpssDxChHwRegAddrDbRemove(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* free allocations are 'DXCH' specific allocations (not generic) */
    /* need to free :
       1. pools
       2. dynamic allocations
       3. semaphores/mutexes
    */
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.enhancedInfoPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyArr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.frameLatencyFracArr);

    for(ii = 0;ii < PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->numOfUplinkPorts ;ii++)
    {
        FREE_PTR_MAC(PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr[ii].internalConnectionsArr);
    }
    for(ii = 0;ii < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS ;ii++)
    {
        FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMibShadowArr[ii]);
    }
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->uplinkInfoArr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_PORT_GROUPS_DEBUG_INFO_PTR_MAC(devNum)->internalTrunksPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_GE_PORT_DISABLE_WA_E.
            portTxQueuesBmpPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.
            pcsLoopbackBmpPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_XCAT_GE_PORT_UNIDIRECT_WA_E.
            portEnableBmpPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.
            portForceLinkDownBmpPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
            info_PRV_CPSS_DXCH_MISCONFIG_100BASEX_AN_DISABLE_WA_E.
            portPcsRxErMaskDisableBmpPtr);

    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesCfgDbArrPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->extMemory.mvHwsDevTopologyMapPtr);

    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.keyModePerFidBmpPtr);
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr);
    {
        /* free memoried pointed by array of pointers */
        GT_U32 dp;
        for (dp = 0; (dp < PRV_CPSS_MAX_SIP6_30_DP_CNS); dp++)
        {
            FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.sip6_30_dpInfoPtr[dp]);
        }
    }
    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr);

    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->port.ptpPortSpeedDelayValArrPtr);

    for(hwTable = 0; hwTable < CPSS_DXCH_TABLE_LAST_E; hwTable++)
    {
        if(prvCpssDxChDiagDataIntegrityIsMultipleNames(hwTable,NULL))
        {
            /* the memory belongs to some one else */
            PRV_CPSS_DXCH_PP_MAC(devNum)->shadowInfoArr[hwTable].hwTableMemPtr = NULL;
        }
        else
        {
            FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->shadowInfoArr[hwTable].hwTableMemPtr);
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->shadowInfoArr[hwTable].hwTableMemSize = 0;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {   /*destroy scheduler*/
        rc = prvCpssSip6TxqUtilsDevClose(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.info_PRV_CPSS_DXCH_BRG_L2_DLB_TIMER_COUNTER_WA_E.dlbWaEnable == GT_TRUE)
        {
            dlbWaUsageNodePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                info_PRV_CPSS_DXCH_BRG_L2_DLB_TIMER_COUNTER_WA_E.headOfDlbWaUsageList;
            while(dlbWaUsageNodePtr)
            {
                dlbWaUsageNodeNextPtr = dlbWaUsageNodePtr->nextPointer;
                FREE_PTR_MAC(dlbWaUsageNodePtr);
                dlbWaUsageNodePtr = dlbWaUsageNodeNextPtr;
            }
        }
    }
    prvCpssGenericSrvCpuRemove(devNum, 0xFF);

    /* next part -- release the 'generic' DB */
    rc = prvCpssPpConfigDevDbRelease(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************************************************/
    /* NOTE :  PRV_CPSS_PP_MAC(devNum) == NULL at this point !!! */
    /*************************************************************/

    /* check that the family of the cheetah is empty , so we can release the
       global needed DB , relate to the cheetah family */
    if(HWINIT_GLOVAR(prvCpssFamilyInfoArray[devFamily]) == NULL)
    {
        if (useMii)
        {
            /* only the RX need the free because uses osMalloc
               (see function ethPrtInitRx())

               the TX uses 'DMA' memory that is up to application to manage.
               (was not alloc by CPSS)
            */

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
            rc = prvCpssGenNetIfMiiFreeMemory(PRV_CPSS_GEN_NETIF_MII_RX_SEG_POOL_E,cpssOsFree_MemoryLeakageDbg);
#else
            rc = prvCpssGenNetIfMiiFreeMemory(PRV_CPSS_GEN_NETIF_MII_RX_SEG_POOL_E,cpssOsFree);
#endif
            if(rc != GT_OK)
            {
                return rc;
            }
        }

    }

    /* reset the HWS */
    switch(devType)
    {
        case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
             hwsHooperIfClose(devNum);
            break;
        case CPSS_BOBCAT2_ALL_DEVICES_CASES_MAC:
            hwsBobcat2IfClose(devNum);
            break;
        default:/* bobk and above (also AC5) */
            mvHwsDeviceClose(devNum);/* call generic function of HWS */
            break;
        case PRV_CPSS_DXCH_XCAT3_DEVICES_CASES_MAC:
            hwsAlleycat3IfClose(devNum);
            break;
        case CPSS_LION2_PORT_GROUPS_01234567_DEVICES_CASES_MAC:
            hwsLion2IfClose(devNum);
            break;
    }
   /* prvCpssGenericSrvCpuRemove(devNum, SRVCPU_IPC_CHANNEL_ALL);*/

    return GT_OK;
}

/**
* @internal cpssDxChCfgDevRemove function
* @endinternal
*
* @brief   Remove the device from the CPSS.
*         This function we release the DB of CPSS that associated with the PP ,
*         and will "know" that the device is not longer valid.
*         This function will not do any HW actions , and therefore can be called
*         before or after the HW was actually removed.
*         After this function is called the devNum is no longer valid for any API
*         until this devNum is used in the initialization sequence for a "new" PP.
*         NOTE: 1. Application may want to disable the Traffic to the CPU , and
*         messages (AUQ) , prior to this operation (if the device still exists).
*         2. Application may want to a reset the HW PP , and there for need
*         to call the "hw reset" function (before calling this function)
*         and after calling this function to do the initialization
*         sequence all over again for the device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to remove.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - wrong dev
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssDxChCfgDevRemove
(
    IN GT_U8   devNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDevRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    PRV_CPSS_INT_SCAN_LOCK();
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChCfgDevRemove(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    PRV_CPSS_INT_SCAN_UNLOCK();
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
* @param[in] enable                   - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgDevEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;       /* return code */
    GT_U32  bit;        /* The bit to be written to the register    */
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    bit = BOOL2BIT_MAC(enable);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_TBD_BOOKMARK_EARCH/* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                               dfxUnits.server.resetControl;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, bit);
    }
    else
    {
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                              DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
                                                    devNum, regAddr, 0, 1, bit);
    }

    return rc;
}

/**
* @internal cpssDxChCfgDevEnable function
* @endinternal
*
* @brief   This routine sets the device state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
* @param[in] enable                   - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgDevEnable
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDevEnable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCfgDevEnable(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
*
* @param[out] enablePtr                - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChCfgDevEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    bit;
    GT_U32    regAddr;    /* register address */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_TBD_BOOKMARK_EARCH/* current GM doesn't support DFX unit so far */
        *enablePtr = GT_TRUE;
        return GT_OK;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                                               dfxUnits.server.resetControl;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &bit);
    }
    else /* PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) || CPSS_PP_FAMILY_DXCH_BOBCAT2_E */
    {
        regAddr =
            PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                              DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;
        rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
                                                   devNum, regAddr, 0, 1, &bit);
    }

    if(GT_OK != rc)
    {
        return rc;
    }

    *enablePtr =  BIT2BOOL_MAC(bit);

    return rc;
}

/**
* @internal cpssDxChCfgDevEnableGet function
* @endinternal
*
* @brief   This routine gets the device state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to set.
*
* @param[out] enablePtr                - GT_TRUE device enable, GT_FALSE disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCfgDevEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDevEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChCfgDevEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
static GT_STATUS internal_cpssDxChCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
{
    GT_U8 num;

    /* device should be in the allowed range but not necessary exist */
    if ((devNum > PRV_CPSS_MAX_PP_DEVICES_CNS) && (devNum != DEVICE_NOT_EXISTS_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (nextDevNumPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if(NULL != cpssSharedGlobalVarsPtr)
    {
        if (devNum == 0xFF)
            num = 0;
        else
            num = (GT_U8)(devNum+1);

        for (; num<PRV_CPSS_MAX_PP_DEVICES_CNS; num++)
        {
            if (PRV_CPSS_PP_MAC(num) &&
                (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(num)->devFamily)))
            {
                *nextDevNumPtr = num;
                return GT_OK;
            }
        }
    }

    /* no more devices found */
    *nextDevNumPtr = DEVICE_NOT_EXISTS_CNS;
    return /* it's not error for log */ GT_NO_MORE;
}

/**
* @internal cpssDxChCfgNextDevGet function
* @endinternal
*
* @brief   Return the number of the next existing device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number to start from. For the first one  should be 0xFF.
*
* @param[out] nextDevNumPtr            - number of next device after devNum.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_MORE               - devNum is the last device. nextDevNumPtr will be set to 0xFF.
* @retval GT_BAD_PARAM             - devNum > max device number
* @retval GT_BAD_PTR               - nextDevNumPtr pointer is NULL.
*/
GT_STATUS cpssDxChCfgNextDevGet
(
    IN  GT_U8 devNum,
    OUT GT_U8 *nextDevNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgNextDevGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nextDevNumPtr));

    rc = internal_cpssDxChCfgNextDevGet(devNum, nextDevNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nextDevNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChCfgAddrUpInit function
* @endinternal
*
* @brief   This function initializes the Core Address Update mechanism, by
*         allocating the AU descs. block.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to init the Au unit for.
* @param[in] auDescBlockPtr           - (pointer to)A block to allocate the Au descriptors from.
* @param[in] auDescBlockSize          - Size in bytes of Au descBlock.
* @param[in] fuDescBlockPtr           - (pointer to)A block to allocate the Fu descriptors from.
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] fuDescBlockSize          - Size in bytes of Fu descBlock.
*                                      Valid if useFuQueue is GT_TRUE.
* @param[in] useFuQueue               - GT_TRUE  - Fu queue is used - for DxCh2 devices only.
*                                      - GT_FALSE - Fu queue is unused.
* @param[in] useDoubleAuq             - Support configuration of two AUQ memory regions.
*                                      GT_TRUE - CPSS manages two AU Queues with the same size:
* @param[in] auDescBlockSize          / 2.
*                                      GT_FALSE - CPSS manages single AU Queue with size:
* @param[in] auDescBlockSize
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvCpssDxChCfgAddrUpInit
(
    IN GT_U8    devNum,
    IN GT_U8    *auDescBlockPtr,
    IN GT_U32   auDescBlockSize,
    IN GT_U8    *fuDescBlockPtr,
    IN GT_U32   fuDescBlockSize,
    IN GT_BOOL  useFuQueue,
    IN GT_BOOL  useDoubleAuq
)
{
    GT_STATUS   rc = GT_OK;                     /* return code */
    GT_U32      portGroupId;                    /* port group ID */
    GT_U16      numActivePortGroups;            /* number of active port groups */
    GT_U16      auq_numActivePortGroups;        /* number of active port groups with AUQ */
    GT_U16      fuq_numActivePortGroups;        /* number of active port groups with FUQ */
    GT_U32      perPortGroupAuSize;             /* AUQ size per port group */
    GT_U32      perPortGroupFuSize;             /* FUQ size per port group */
    GT_U32      auq_index=0;                    /* AUQ instance index */
    GT_U32      fuq_index=0;                    /* FUQ instance index */
    GT_U8       *perPortGroupAuDescBlockPtr;    /* pointer to AUQ descriptors block per port group */
    GT_U8       *perPortGroupFuDescBlockPtr;    /* pointer to FUQ descriptors block per port group */
    GT_U8       *perPortGroupCnc23FuDescBlockPtr;/* pointer to FUQ descriptors block per port group that serves CNC2,3 upload*/
    GT_U8       *perPortGroupAu1DescBlockPtr;   /* pointer to AUQ1 descriptors block per port group */
    GT_U8       *au1DescBlockPtr = 0;           /* pointer to AUQ1 descriptors first block */
    GT_U32      auqSize = auDescBlockSize;      /* Temporary AUQ size */
    CPSS_SYSTEM_RECOVERY_INFO_STC  oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_U32      portGroupsBmp;/* multi-port groups support */
    GT_U32      auq_portGroupsBmp;/* multi-port groups with AUQ support */
    GT_U32      fuq_portGroupsBmp;/* multi-port groups with FUQ support */
    GT_BOOL     useCnc23Fuq = GT_FALSE;

    if((PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum)) &&
       (PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                   info_PRV_CPSS_DXCH_XCAT_FDB_AU_FIFO_CORRUPT_WA_E.
                       enabled == GT_FALSE))
    {
        if(auDescBlockPtr == NULL)
        {
            /* no Hw changes needed */
            /* nothing more to do */
            return GT_OK;
        }
    }

    /* save recovery state*/
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;

    if ( oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
        if ( (useFuQueue == GT_TRUE) && (oldSystemRecoveryInfo.systemRecoveryMode.continuousFuMessages == GT_FALSE) )
        {
            /* This init is under system recovery process after HA event. PP starts to send  */
            /* messages to newly reprogramed FUQ only if it has indication that "old" one is full. */
            /* After HA event there is no knowledge about FUQ status. So WA on previously defined queue is performed*/
            rc = prvCpssDxChHaFuqNonContinuousMsgModeHandle(devNum);
            if (rc != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                return rc;
            }
        }

        if(oldSystemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE)
        {
            /* This init is under system recovery process after HA event. PP starts to send  */
            /* messages to newly reprogramed AUQ only if it has indication that "old" one is full. */
            /* After HA event there is no knowledge about AUQ status. So WA on previously defined queue is performed*/
            rc = prvCpssDxChHaAuqNonContinuousMsgModeHandle(devNum);
            if (rc != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                return rc;
            }
        }

        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    }
    else
    {
        if ( oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E )
        {
            if (oldSystemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE)
            {
                newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
                prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

                rc = prvCpssDxChDisableAuqAndSaveAuqCurrentStatus(devNum);
                if (rc != GT_OK)
                {
                    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                    return rc;
                }
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            }
        }
    }

    /* Memory allocated for two AUQ */
    if(useDoubleAuq == GT_TRUE)
    {
        if(auDescBlockPtr == NULL)
        {
            /* avoid access to NULL pointer */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "specified 'double AUQ' but NULL AUQ pointer");
        }

        auqSize /= 2;
        au1DescBlockPtr = &auDescBlockPtr[auqSize];
    }

    if(useFuQueue == GT_TRUE && fuDescBlockPtr == NULL)
    {
        /* avoid access to NULL pointer */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, "specified 'use FUQ' but NULL FUQ pointer");
    }

    if(useFuQueue == GT_TRUE &&
       PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits > 2)
    {
        useCnc23Fuq = GT_TRUE;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        auq_portGroupsBmp = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp;
        fuq_portGroupsBmp = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp;
        auq_numActivePortGroups = (GT_U16)prvCpssPpConfigBitmapNumBitsGet(auq_portGroupsBmp);
        fuq_numActivePortGroups = (GT_U16)prvCpssPpConfigBitmapNumBitsGet(fuq_portGroupsBmp);

        portGroupsBmp = auq_portGroupsBmp | fuq_portGroupsBmp;

    }
    else
    {
        /* calculate number of port groups */
        rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_CPSS_DXCH_UNIT_MG_E,&numActivePortGroups);
        if(rc != GT_OK)
        {
            return rc;
        }

        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_MG_E);

        auq_numActivePortGroups = numActivePortGroups;
        fuq_numActivePortGroups = numActivePortGroups;

        auq_portGroupsBmp = portGroupsBmp;
        fuq_portGroupsBmp = portGroupsBmp;
    }

    if(useCnc23Fuq == GT_TRUE)
    {
        fuq_numActivePortGroups *= 2;/* one for the main FU/CNC0,1 and one for the CNC2,3 */
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
    {
        /* all port groups get the same AUQ size */
        perPortGroupAuSize = auqSize / auq_numActivePortGroups;
        /* all port groups get the same FUQ size */
        perPortGroupFuSize = fuDescBlockSize / fuq_numActivePortGroups;

        perPortGroupAuDescBlockPtr = auDescBlockPtr ?
            &auDescBlockPtr[perPortGroupAuSize * auq_index] : NULL;

        perPortGroupFuDescBlockPtr = useFuQueue ?
            &fuDescBlockPtr[perPortGroupFuSize * fuq_index] : NULL;

        perPortGroupAu1DescBlockPtr = au1DescBlockPtr ?
            &au1DescBlockPtr[perPortGroupAuSize * auq_index] : NULL;

        if(0 == (auq_portGroupsBmp & (1<<portGroupId)))
        {
            perPortGroupAuDescBlockPtr  = NULL;
            perPortGroupAu1DescBlockPtr = NULL;
        }
        else
        {
            auq_index++;
        }

        perPortGroupCnc23FuDescBlockPtr = NULL;

        if(0 == (fuq_portGroupsBmp & (1<<portGroupId)))
        {
            perPortGroupFuDescBlockPtr = NULL;
        }
        else
        {
            fuq_index++;

            if(useCnc23Fuq == GT_TRUE)
            {
                perPortGroupCnc23FuDescBlockPtr = &fuDescBlockPtr[perPortGroupFuSize * fuq_index];
                fuq_index++;
            }
        }


        /* Set primary AUQ index */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[portGroupId] = 0;
        /* Set primary AUQ init state - 'FULL';
        When WA triggered for the first time - all primary AUQs are full */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId].primaryState =
            PRV_CPSS_AUQ_STATE_ALL_FULL_E;
        /* Set secondary AUQ index */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId].activeSecondaryAuqIndex = 0;
        /* Set secondary AUQ state - 'EMPTY' */
        PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[portGroupId].secondaryState =
            PRV_CPSS_AUQ_STATE_ALL_EMPTY_E;

        rc = internalCfgAddrUpInit(devNum,
                    portGroupId,
                    perPortGroupAuDescBlockPtr,
                    perPortGroupAuSize,
                    perPortGroupFuDescBlockPtr,
                    perPortGroupFuSize,
                    useFuQueue,
                    perPortGroupAu1DescBlockPtr,
                    perPortGroupAuSize,
                    perPortGroupCnc23FuDescBlockPtr,
                    perPortGroupFuSize);

        if(rc != GT_OK)
        {
            return rc;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

    /* for 'multi port groups' device -- state that PP ready only after all port groups finished */
    /* state to the special DB that the device finished PP logical init */
    (void)prvCpssPpConfigDevDbHwResetSet(devNum,GT_FALSE);

    if (oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E)
    {
        if (oldSystemRecoveryInfo.systemRecoveryMode.continuousAuMessages == GT_FALSE)
        {
            newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
            prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

            rc = prvCpssDxChRestoreAuqCurrentStatus(devNum);
            if (rc != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                return rc;
            }
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
        }
    }

    return GT_OK;
}

/**
* @internal sharedCncPortGroupLogicalInit function
* @endinternal
*
* @brief   This function initialize the support for CNC uploads in device with shared
*         CNC pipes into single FUQ in MG unit.
*
* @note   APPLICABLE DEVICES:      Bobcat3.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X ; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed.
*                                       GT_FAIL otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sharedCncPortGroupLogicalInit
(
    IN      GT_U8                           devNum
)
{
    GT_STATUS   rc;
    GT_U32  portGroupId;
    GT_U32  auMessageNumBytes;
    GT_U32  cncBlocksInFuq;
    GT_U32  sizeOfMalloc,numBitsInBmp;
    GT_VOID *mallocPtr;
    GT_U32  mgUnit_portGroupId;/* current MG unit port group that relate to current CNC unit port group */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC *currCncInfoPtr;
    GT_U32  sharedCncPortGroupsBmpPerMg = 3;/*BIT_0 | BIT_1*/

    /* number of bytes in each FUQ message */
    auMessageNumBytes = 4 * PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        /* get MG port group that serves the current CNC unit port group */
        rc = prvCpssDxChHwPortGroupBetweenUnitsConvert(devNum,
            PRV_CPSS_DXCH_UNIT_CNC_0_E, portGroupId,
            PRV_CPSS_DXCH_UNIT_MG_E   , &mgUnit_portGroupId);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* number of CNC blocks that can fit into FUQ */
        cncBlocksInFuq = 1 + (PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[mgUnit_portGroupId].blockSize * auMessageNumBytes) /
            (PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncBlockNumEntries * 8/*each counter 8 bytes*/);
        /* number of 'words' that need to represent the 'cncBlocksInFuq' as 'bitmap' */
        sizeOfMalloc = 1 + (cncBlocksInFuq / 32);

        /* save the number of bits in the bmp */
        numBitsInBmp = 32 * sizeOfMalloc;

        /* multiple by the size of the BMP element */
        sizeOfMalloc *= sizeof(GT_U32);

        mallocPtr = cpssOsMalloc(sizeOfMalloc);
        if (mallocPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(mallocPtr, 0, sizeOfMalloc);

        currCncInfoPtr = PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,portGroupId);

        currCncInfoPtr->numBitsInBmp = numBitsInBmp;

        /* bind the allocation to the 'per port group' */
        currCncInfoPtr->isMyCncUploadTriggeredBmpArr = mallocPtr;

        currCncInfoPtr->mgPortGroupId = mgUnit_portGroupId;
        currCncInfoPtr->currCncDescIdxInTrigger = 0;

        /* the CNC in bobcat3 must not be triggered in parallel on the 2 pipes
           because both pipe use the same MG unit with the same FUQ
        */
        currCncInfoPtr->otherSharedPortGroupsOnMyMgUnit =
            /* all shared port groups of my MG */
            (sharedCncPortGroupsBmpPerMg << mgUnit_portGroupId) -
            /* remove 'me' from the bitmap of 'others' */
            (1<<portGroupId);
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl = GT_TRUE;

    return GT_OK;
}

/* macro to set the register value for shared table */
#define FILL_REG_INFO_MAC(regValue_0,regValue_1)    ((regValue_0) | ((regValue_1) << 8))

/* macro to check if second client of the register is free  */
#define IS_SECOND_FREE_CHECK_REG_INFO_MAC(regValue)  (((regValue) >> 8) ? 0 : 1)

/* macro to set the register value for shared table */
#define FILL_SECOND_CLIENT_REG_INFO_MAC(secondClientValue,regValue)    FILL_REG_INFO_MAC(regValue,secondClientValue)

/* the ARP always use 0x11,0x12 */
#define ARP_REG_VALUE   FILL_REG_INFO_MAC(0x11,0x12)

/* the falcon hold 9 'supper' SBMs  */
#define FALCON_SUPER_SBM_NUM        9
/* the Hawk hold 10 'supper' SBMs  */
#define HAWK_SUPER_SBM_NUM         10

/* max number of 'supper' SBMs  */
#define MAX_SUPER_SBM_NUM          10

/* the falcon hold each 'supper' SBMs with 4 SMBs */
#define FALCON_SBM_IN_SUPER_NUM     4
#define FALCON_SBM_NUM              (FALCON_SUPER_SBM_NUM * FALCON_SBM_IN_SUPER_NUM)
#define HAWK_SBM_NUM                (HAWK_SUPER_SBM_NUM   * FALCON_SBM_IN_SUPER_NUM)

/* max number of SBMs  */
#define MAX_SBM_NUM                (MAX_SUPER_SBM_NUM * FALCON_SBM_IN_SUPER_NUM)


#define CLIENT_TYPE_AND_NAME_MAC(_client)   _client,#_client
/* the SBM is connected to 2 'parts' (part 1 , part 2) */
#define FALCON_PART_NUM     2
/* FALCON : enum for the clients that may occupy the SBMs */
typedef enum
{
    SBM_CLIENT_TYPE_LPM_0_E = 1,/* must not be 0 */
    SBM_CLIENT_TYPE_LPM_1_E ,
    SBM_CLIENT_TYPE_LPM_2_E ,
    SBM_CLIENT_TYPE_LPM_3_E ,
    SBM_CLIENT_TYPE_LPM_4_E ,
    SBM_CLIENT_TYPE_LPM_5_E ,
    SBM_CLIENT_TYPE_LPM_6_E ,
    SBM_CLIENT_TYPE_LPM_7_E ,
    SBM_CLIENT_TYPE_LPM_8_E ,
    SBM_CLIENT_TYPE_LPM_9_E ,
    SBM_CLIENT_TYPE_LPM_10_E,
    SBM_CLIENT_TYPE_LPM_11_E,
    SBM_CLIENT_TYPE_LPM_12_E,
    SBM_CLIENT_TYPE_LPM_13_E,
    SBM_CLIENT_TYPE_LPM_14_E,
    SBM_CLIENT_TYPE_LPM_15_E,
    SBM_CLIENT_TYPE_LPM_16_E,
    SBM_CLIENT_TYPE_LPM_17_E,
    SBM_CLIENT_TYPE_LPM_18_E,
    SBM_CLIENT_TYPE_LPM_19_E,
    SBM_CLIENT_TYPE_LPM_20_E,
    SBM_CLIENT_TYPE_LPM_21_E,
    SBM_CLIENT_TYPE_LPM_22_E,
    SBM_CLIENT_TYPE_LPM_23_E,
    SBM_CLIENT_TYPE_LPM_24_E,
    SBM_CLIENT_TYPE_LPM_25_E,
    SBM_CLIENT_TYPE_LPM_26_E,
    SBM_CLIENT_TYPE_LPM_27_E,

    SBM_CLIENT_TYPE_FDB_0_E,
    SBM_CLIENT_TYPE_FDB_1_E,
    SBM_CLIENT_TYPE_FDB_2_E,
    SBM_CLIENT_TYPE_FDB_3_E,
    SBM_CLIENT_TYPE_FDB_4_E,
    SBM_CLIENT_TYPE_FDB_5_E,
    SBM_CLIENT_TYPE_FDB_6_E,
    SBM_CLIENT_TYPE_FDB_7_E,
    SBM_CLIENT_TYPE_FDB_8_E,
    SBM_CLIENT_TYPE_FDB_9_E,
    SBM_CLIENT_TYPE_FDB_10_E,
    SBM_CLIENT_TYPE_FDB_11_E,
    SBM_CLIENT_TYPE_FDB_12_E,
    SBM_CLIENT_TYPE_FDB_13_E,
    SBM_CLIENT_TYPE_FDB_14_E,
    SBM_CLIENT_TYPE_FDB_15_E,

    SBM_CLIENT_TYPE_EM_0_E,
    SBM_CLIENT_TYPE_EM_1_E,
    SBM_CLIENT_TYPE_EM_2_E,
    SBM_CLIENT_TYPE_EM_3_E,
    SBM_CLIENT_TYPE_EM_4_E,
    SBM_CLIENT_TYPE_EM_5_E,
    SBM_CLIENT_TYPE_EM_6_E,
    SBM_CLIENT_TYPE_EM_7_E,
    SBM_CLIENT_TYPE_EM_8_E,
    SBM_CLIENT_TYPE_EM_9_E,
    SBM_CLIENT_TYPE_EM_10_E,
    SBM_CLIENT_TYPE_EM_11_E,
    SBM_CLIENT_TYPE_EM_12_E,
    SBM_CLIENT_TYPE_EM_13_E,
    SBM_CLIENT_TYPE_EM_14_E,
    SBM_CLIENT_TYPE_EM_15_E,

    SBM_CLIENT_TYPE_ARP_E   ,/* single name for all ARP clients */

    SBM_CLIENT_TYPE__LAST__E

}SBM_CLIENT_TYPE_ENT;
/* struct to hold info about the client that occupy the supper SBM */
typedef struct
{
    SBM_CLIENT_TYPE_ENT client;         /* the id */
    GT_CHAR             *clientName;    /* the name (for debug prints) */
    GT_U32              regValue_0;     /* value of part 1 (into the register) */
    GT_U32              regValue_1;     /* value of part 2 (into the register) */
}SUPPER_SBM_INFO_STC;
/* value 0 is not valid in the regValue_0,regValue_1 */
#define UNUSED_HW_VALUE 0
/* Falcon supper SBM 0 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_0_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 0  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 8  ),  0x3            ,0x4},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 2   ),  0x5            ,0x6},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 4   ),  0x7            ,0x8},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 4  ),  0x9            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 12 ),  0xa            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 14 ),  0xb            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 26 ),  0xc            ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 1 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_1_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 1  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 9  ),  0x3            ,0x4},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 3   ),  0x5            ,0x6},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 5   ),  0x7            ,0x8},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 5  ),  0x9            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 13 ),  0xa            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 15 ),  0xb            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 27 ),  0xc            ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 2 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_2_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 2  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 10 ),  0x3            ,0x4},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 2   ),  0x5            ,0x6},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 8   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 12  ),  0x9            ,0xa},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 6  ),  0xb            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 14 ),  0xc            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 20 ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 21 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 24 ),  0xf            ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 3 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_3_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 3  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 11 ),  0x3            ,0x4},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 3   ),  0x5            ,0x6},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 9   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 13  ),  0x9            ,0xa},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 7  ),  0xb            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 15 ),  0xc            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 22 ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 23 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 25 ),  0xf            ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 4 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_4_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 0  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 4  ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 12 ),  0x5            ,0x6},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 0   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 10  ),  0x9            ,0xa},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 14  ),  0xb            ,0xc},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 8  ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 16 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 10 ),  0xf            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 20 ),  0x10           ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 5 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_5_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 1  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 5  ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 13 ),  0x5            ,0x6},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 1   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 11  ),  0x9            ,0xa},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 15  ),  0xb            ,0xc},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 9  ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 17 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 11 ),  0xf            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 21 ),  0x10           ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 6 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_6_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 2  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 6  ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 14 ),  0x5            ,0x6},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 0   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 6   ),  0x9            ,0xa},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 2   ),  0xb            ,0xc},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 10 ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 18 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 22 ),  0xf            ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 7 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_7_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 3  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 7  ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 15 ),  0x5            ,0x6},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 1   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 7   ),  0x9            ,0xa},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 3   ),  0xb            ,0xc},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 11 ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 19 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 23 ),  0xf            ,UNUSED_HW_VALUE},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon supper SBM 8 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC falcon_super_sbm_8_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 0   ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 1   ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 2   ),  0x5            ,0x6},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 3   ),  0x7            ,0x8},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 0  ),  0x9            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 1  ),  0xa            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 2  ),  0xb            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 3  ),  0xc            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 4  ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 5  ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 6  ),  0xf            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 7  ),  0x10           ,UNUSED_HW_VALUE},

    /* !!! ARP special case !!! , can appear 0..4 times in supper bank 8 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};

/* Falcon ALL supper SBMs */
static const SUPPER_SBM_INFO_STC* const Falcon_super_SBM_support[FALCON_SUPER_SBM_NUM] =
{
     &falcon_super_sbm_0_support[0]
    ,&falcon_super_sbm_1_support[0]
    ,&falcon_super_sbm_2_support[0]
    ,&falcon_super_sbm_3_support[0]
    ,&falcon_super_sbm_4_support[0]
    ,&falcon_super_sbm_5_support[0]
    ,&falcon_super_sbm_6_support[0]
    ,&falcon_super_sbm_7_support[0]
    ,&falcon_super_sbm_8_support[0]
};

/* hawk supper SBM 6 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC hawk_super_sbm_6_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 2  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 6  ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 14 ),  0x5            ,0x6},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 0   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 6   ),  0x9            ,0xa},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 2   ),  0xb            ,0xc},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 10 ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 18 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 22 ),  0xf            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E +  8 ),  0x10           ,UNUSED_HW_VALUE},/* not exists in Falcon */

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* hawk supper SBM 7 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC hawk_super_sbm_7_support[] =
{
    /*      client                  regValue_0 , regValue_1 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 3  ),  0x1            ,0x2},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 7  ),  0x3            ,0x4},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_FDB_0_E + 15 ),  0x5            ,0x6},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 1   ),  0x7            ,0x8},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 7   ),  0x9            ,0xa},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_EM_0_E + 3   ),  0xb            ,0xc},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 11 ),  0xd            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 19 ),  0xe            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E + 23 ),  0xf            ,UNUSED_HW_VALUE},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_LPM_0_E +  9 ),  0x10           ,UNUSED_HW_VALUE},/* not exists in Falcon */

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Hawk supper SBM 9 - list of all optional clients and their info */
static const SUPPER_SBM_INFO_STC hawk_super_sbm_9_support[] =
{
    /* !!! ARP special case !!! , can appear 0..4 times in supper bank 9 */
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},
    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE_ARP_E        ),  0x11           ,0x12},

    {CLIENT_TYPE_AND_NAME_MAC(SBM_CLIENT_TYPE__LAST__E     ),UNUSED_HW_VALUE  ,UNUSED_HW_VALUE}
};
/* Falcon ALL supper SBMs */
static const SUPPER_SBM_INFO_STC* const Hawk_super_SBM_support[HAWK_SUPER_SBM_NUM] =
{
     &falcon_super_sbm_0_support[0]
    ,&falcon_super_sbm_1_support[0]
    ,&falcon_super_sbm_2_support[0]
    ,&falcon_super_sbm_3_support[0]
    ,&falcon_super_sbm_4_support[0]
    ,&falcon_super_sbm_5_support[0]
    ,&hawk_super_sbm_6_support[0]
    ,&hawk_super_sbm_7_support[0]
    ,&falcon_super_sbm_8_support[0]
    ,&hawk_super_sbm_9_support[0]
};

/*debug function to allow printings in the logic of setting the shared tables*/
void dxChSharedTabled_DebugEnable
(
    IN GT_U32  enableDebug
)
{
    CONFIG_GLOBAL_VAR_SET(debug_sharedTables  , enableDebug);
}

/* FALCON : LPM : full mode (14K) : those in the list comes at 'second' loop on
    the clients
*/
static const GT_U32   falconLpmClientsPriorityLowArr_fullMode[] =
    {10,11,14,15,23,24,25,26,27,GT_NA};

/* FALCON : LPM : half mode (7K) : those in the list comes at 'second' loop on
    the clients
*/
static const GT_U32   falconLpmClientsPriorityLowArr_halfMode[] =
    {10,11,22,23,GT_NA};

/* FALCON : EM : those in the list comes at 'second' loop on the clients */
static const GT_U32   falconEmClientsPriorityLowArr[] =
    {0,1,2,3,GT_NA};

/* FALCON : FDB : those in the list comes at 'second' loop on the clients */
static const GT_U32   falconFdbClientsPriorityLowArr[] =
    {0,1,2,3,GT_NA};

/**
* @internal getRepresentativeClient function
* @endinternal
*
* @brief   function to return representative ID for the client.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in] client  - the client ID
*
*
* @retval   - the representative ID for the client.
*/
static SBM_CLIENT_TYPE_ENT getRepresentativeClient(
    IN SBM_CLIENT_TYPE_ENT client
)
{
    if(client >= SBM_CLIENT_TYPE_LPM_0_E &&
       client <= SBM_CLIENT_TYPE_LPM_27_E)
    {
        return  SBM_CLIENT_TYPE_LPM_0_E;
    }

    if(client >= SBM_CLIENT_TYPE_FDB_0_E &&
       client <= SBM_CLIENT_TYPE_FDB_15_E)
    {
        return  SBM_CLIENT_TYPE_FDB_0_E;
    }

    if(client >= SBM_CLIENT_TYPE_EM_0_E &&
       client <= SBM_CLIENT_TYPE_EM_15_E)
    {
        return  SBM_CLIENT_TYPE_EM_0_E;
    }

    if(client == SBM_CLIENT_TYPE_ARP_E)
    {
        return SBM_CLIENT_TYPE_ARP_E;
    }

    return SBM_CLIENT_TYPE__LAST__E;
}

/**
* @internal getRepresentativeClientName function
* @endinternal
*
* @brief   function to return representative NAME (string) for the client.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in] client  - the client ID
*
*
* @retval   - the representative NAME (string) for the client.
*/
static const char* getRepresentativeClientName(
    IN SBM_CLIENT_TYPE_ENT client
)
{
    static const char* clientNamesArr[] = {
        STR(SBM_CLIENT_TYPE_LPM_0_E),
        STR(SBM_CLIENT_TYPE_FDB_0_E),
        STR(SBM_CLIENT_TYPE_EM_0_E),
        STR(SBM_CLIENT_TYPE_ARP_E),
        STR(unknown),
        };

    GT_U32  index = 0;
    if(client >= SBM_CLIENT_TYPE_LPM_0_E &&
       client <= SBM_CLIENT_TYPE_LPM_27_E)
    {
        return  clientNamesArr[index];
    }
    index++;

    if(client >= SBM_CLIENT_TYPE_FDB_0_E &&
       client <= SBM_CLIENT_TYPE_FDB_15_E)
    {
        return  clientNamesArr[index];
    }
    index++;

    if(client >= SBM_CLIENT_TYPE_EM_0_E &&
       client <= SBM_CLIENT_TYPE_EM_15_E)
    {
        return  clientNamesArr[index];
    }
    index++;

    if(client == SBM_CLIENT_TYPE_ARP_E)
    {
        return  clientNamesArr[index];
    }
    index++;

    return  clientNamesArr[index];
}

/**
* @internal hawkNumberOfClientsArrInit function
* @endinternal
*
* @brief   sip6_10 : function to initialize hawkPriorityArr[].
*          (using hawkNumberOfClientsArr[] that also initialized here)
*
* @note   APPLICABLE DEVICES:      sip6_10.
* @note   NOT APPLICABLE DEVICES:  non-sip6_10.
*
* @param[in] devNum  - the device number
*
*
* @retval  GT_OK on success, or
*          GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed.
*/
static GT_STATUS hawkNumberOfClientsArrInit(IN GT_U32    devNum)
{
    const SUPPER_SBM_INFO_STC*    currSupperSbmInfoPtr;
    SBM_CLIENT_TYPE_ENT representativeClient;
    GT_U32  ii,jj;
    GT_U32  startIndex , lastIndex;
    GT_U32  minValue;
    GT_U32  numOfSuperSbm;
    const SUPPER_SBM_INFO_STC* const *    devSupperSbmInfoArr;

    devSupperSbmInfoArr = Hawk_super_SBM_support;
    numOfSuperSbm = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;

    CONFIG_GLOBAL_VAR_SET(hawkNumberOfClientsArr_initDone , GT_TRUE);

    if(CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr) == NULL)
    {
        CONFIG_GLOBAL_VAR_SET(hawkNumberOfClientsArr , cpssOsMalloc(sizeof(GT_U32)*SBM_CLIENT_TYPE__LAST__E));
        if(CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr) == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"hawkNumberOfClientsArr allocation failed");
        }
        cpssOsMemSet(CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr),0,
            sizeof(GT_U32)*SBM_CLIENT_TYPE__LAST__E);
    }
    if(CONFIG_GLOBAL_VAR_GET(hawkPriorityArr) == NULL)
    {
        CONFIG_GLOBAL_VAR_SET(hawkPriorityArr , cpssOsMalloc(sizeof(GT_U32)*SBM_CLIENT_TYPE__LAST__E));
        if(CONFIG_GLOBAL_VAR_GET(hawkPriorityArr) == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"hawkPriorityArr allocation failed");
        }
        cpssOsMemSet(CONFIG_GLOBAL_VAR_GET(hawkPriorityArr),0,
            sizeof(GT_U32)*SBM_CLIENT_TYPE__LAST__E);
    }

    /* count how many times each client exists */
    for(ii = 0 ; ii < numOfSuperSbm ; ii++)
    {
        currSupperSbmInfoPtr = devSupperSbmInfoArr[ii];
        for(jj = 0 ; currSupperSbmInfoPtr[jj].client != SBM_CLIENT_TYPE__LAST__E; jj++)
        {
            CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr)[currSupperSbmInfoPtr[jj].client]++;
        }
    }

    /* according to count , give priorities to each client */
    for(ii = SBM_CLIENT_TYPE_LPM_0_E ; ii < SBM_CLIENT_TYPE__LAST__E ; ii++)
    {
        minValue = GT_NA;
        startIndex = ii;
        lastIndex = ii;
        /* find client of this representative that hold the max number of occurrences */
        representativeClient = getRepresentativeClient(ii);
        for(jj = ii ; jj < SBM_CLIENT_TYPE__LAST__E ; jj++)
        {
            if(representativeClient != getRepresentativeClient(jj))
            {
                /* we are done with this family of clients */
                ii = jj-1;/* -1 due to '++' in the loop of ii */
                lastIndex = ii;
                break;
            }

            if(minValue  > CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr)[jj])
            {
                minValue = CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr)[jj];
            }
        }

        for(jj = startIndex ; jj <= lastIndex ; jj++)
        {
            CONFIG_GLOBAL_VAR_GET(hawkPriorityArr)[jj] = CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr)[jj] - minValue;
        }
    }

    return GT_OK;
}


/* check if the smbIndex can hold ARP client  */
/* if yes , it fill it's needed neighbors too */
static GT_BOOL  sip6_10CheckArpClientValidityAndSetAsUsed(
    IN    GT_U32   sbmIndex ,
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM],
    OUT   GT_U32    sbmIndexBmp[2]
)
{
    GT_U32  ii;
    GT_U32  relativeIndex     = sbmIndex % FALCON_SBM_IN_SUPER_NUM;/*0..3*/
    GT_U32  firstIndexOfGroup = (sbmIndex & 0x10) ;/* 0 or 16 */

    if(ARP_REG_VALUE != infoArr[sbmIndex] &&
       0             != infoArr[sbmIndex])
    {
        return GT_FALSE;
    }

    sbmIndexBmp[0] = sbmIndexBmp[1] = 0;

    if(sbmIndex < (8 * FALCON_SBM_IN_SUPER_NUM))
    {
        /* the arp is NOT limited to single in super SBM. but should be in the
           same relative SBM index in the 4 super SBMs : 0..3 and in 3..7

           next taken from the MAS doc of SHM :
           8.5.1 Super SBM 0 till 3 and Super SBM 4 till 7
           ARP must be connected to same SBM (0 till 3) on ALL 4 Super SBM.
           Example: ARP connected to SBM1 in Super SBM 0,1,2,3 and SBM2 in Super SBM 4,5,6,7
        */
        firstIndexOfGroup += relativeIndex;

        for(ii = 0 ; ii < FALCON_SBM_IN_SUPER_NUM ; ii++)
        {
            if(sbmIndex == (firstIndexOfGroup + 4*ii))
            {
                /* no check for 'current index'*/
                continue;
            }

            if(0 != infoArr[firstIndexOfGroup + 4*ii])
            {
                /* the 'neighbor' used for something else */
                break;
            }
        }

        if(ii ==  FALCON_SBM_IN_SUPER_NUM)
        {
            /* make sure we can use the other 3 for this ARP too */
            infoArr[firstIndexOfGroup + 4*0] = ARP_REG_VALUE;
            infoArr[firstIndexOfGroup + 4*1] = ARP_REG_VALUE;
            infoArr[firstIndexOfGroup + 4*2] = ARP_REG_VALUE;
            infoArr[firstIndexOfGroup + 4*3] = ARP_REG_VALUE;

            clientTypeArr[firstIndexOfGroup + 4*0][0] = SBM_CLIENT_TYPE_ARP_E;
            clientTypeArr[firstIndexOfGroup + 4*1][0] = SBM_CLIENT_TYPE_ARP_E;
            clientTypeArr[firstIndexOfGroup + 4*2][0] = SBM_CLIENT_TYPE_ARP_E;
            clientTypeArr[firstIndexOfGroup + 4*3][0] = SBM_CLIENT_TYPE_ARP_E;

            sbmIndexBmp[0] |= 0x1111 << firstIndexOfGroup/*0..3 or 16..19*/;
        }
        else
        {
            /* the ARP can't be  at this index*/
            if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
            {
                cpssOsPrintf("super SBMs[%d..%d] need to hold 'ARP' at relative SBM[%d] , but collide with SBM[%d] \n",
                    firstIndexOfGroup/FALCON_SBM_IN_SUPER_NUM,
                    firstIndexOfGroup/FALCON_SBM_IN_SUPER_NUM + 3,
                    relativeIndex ,
                    firstIndexOfGroup + 4*ii);
            }

            return GT_FALSE;
        }
    }
    else
    {
        /* index in Supper SBM = 8 or 9 */
        firstIndexOfGroup = sbmIndex & ~ 0x3 ;/* 32 or 36 */
        /*
        8.5.2        Super SBM 8 and 9
        ARP must be connected or not to all 4 SBMs of this Super SBM.
        Example: ARP connected to SBM0,1,2,3 in Super SBM8
        */
        for(ii = 0 ; ii < FALCON_SBM_IN_SUPER_NUM ; ii++)
        {
            if(sbmIndex == (firstIndexOfGroup + ii))
            {
                /* no check for 'current index'*/
                continue;
            }

            if(0 != infoArr[firstIndexOfGroup + ii])
            {
                /* the 'neighbor' used for something else */
                break;
            }
        }

        if(ii ==  FALCON_SBM_IN_SUPER_NUM)
        {
            /* make sure we can use the other 3 for this ARP too */
            infoArr[firstIndexOfGroup + 0] = ARP_REG_VALUE;
            infoArr[firstIndexOfGroup + 1] = ARP_REG_VALUE;
            infoArr[firstIndexOfGroup + 2] = ARP_REG_VALUE;
            infoArr[firstIndexOfGroup + 3] = ARP_REG_VALUE;

            clientTypeArr[firstIndexOfGroup + 0][0] = SBM_CLIENT_TYPE_ARP_E;
            clientTypeArr[firstIndexOfGroup + 1][0] = SBM_CLIENT_TYPE_ARP_E;
            clientTypeArr[firstIndexOfGroup + 2][0] = SBM_CLIENT_TYPE_ARP_E;
            clientTypeArr[firstIndexOfGroup + 3][0] = SBM_CLIENT_TYPE_ARP_E;

            sbmIndexBmp[1] |= 0xf << (firstIndexOfGroup & 0x1f)/* 0 or 4 */;
        }
        else
        {
            /* the ARP can't be  at this index*/
            if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
            {
                cpssOsPrintf("super SBM[%d] need to hold 0 or 4 'ARPs' , but collide with SBM[%d]\n",
                    firstIndexOfGroup/FALCON_SBM_IN_SUPER_NUM,
                    firstIndexOfGroup + ii);
            }

            return GT_FALSE;
        }
    }

    return GT_TRUE;
}

/* check if the LPM client is 'forced' to be first in register , because it always open new register */
static GT_BOOL   isLpmClientsForceStartAsFirst(
    IN      GT_U8                devNum,
    IN      SBM_CLIENT_TYPE_ENT  clientType
)
{
    static const GT_U32   hawkLpmClientsForceStartAsFirst[] =
        {4,6,8,GT_NA};
    GT_U32  ii;

    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return GT_FALSE;
    }

    for(ii = 0 ; hawkLpmClientsForceStartAsFirst[ii] != GT_NA ; ii++)
    {
        if(hawkLpmClientsForceStartAsFirst[ii] == (GT_U32)(clientType-SBM_CLIENT_TYPE_LPM_0_E))
        {
            return GT_TRUE;
        }
    }

    return GT_FALSE;/* not found as 'force as first' */

}



/* the max depth of recursive of the 'sharedTableAddClient' comes from :
   (END_RECURSIVE_LEVEL - START_RECURSIVE_LEVEL) - 1
*/
#define START_RECURSIVE_LEVEL   0
#define END_RECURSIVE_LEVEL     2


typedef struct{
    GT_U32 fdb_numOfSbm ;
    GT_U32 em_numOfSbm  ;
    GT_U32 lpm_numOfSbm ;
    GT_U32 arp_numOfSbm ;
}SHM_GLOBAL_INFO_STC;

/**
* @internal sharedTableAddClient function
* @endinternal
*
* @brief   function to add a client into one of the free places in the array of SBMs.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in] recursiveLevel  - the recursive Level (depth)
*            clientType      - the client to add to the array
*            isDoubleClientMode - is double client mode (for LPM this is 7K/14K mode)
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*            lastSupperSbmFoundPtr - hold index in infoArr[] and clientTypeArr[] of the previous iteration
*                                   ignored when NULL
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about the 'clientType'
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about the 'clientType'
*            lastSupperSbmFoundPtr - hold index in infoArr[] and clientTypeArr[] of the previous iteration
*                               hold the index that was used by current function
*                                   ignored when NULL
*
* @retval   - GT_OK - added the client properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the client.
*/
static GT_STATUS sharedTableAddClient(
    IN      GT_U8                devNum,
    IN      SHM_GLOBAL_INFO_STC  *globalInfoPtr,
    IN      GT_U32               recursiveLevel,
    IN      SBM_CLIENT_TYPE_ENT  clientType,
    IN      GT_BOOL              isDoubleClientMode,/* for LPM 7K mode */
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM],
    INOUT   GT_U32               *lastSupperSbmFoundPtr
)
{
    GT_STATUS   rc;
    GT_U32  tmp_supperSbmIndex,supperSbmIndex,sbmIndex;
    const SUPPER_SBM_INFO_STC*    currSupperSbmInfoPtr;
    GT_BOOL supperSbmFound;
    GT_CHAR*    clientName = NULL;
    GT_U32  startSupperSmbOffset;
    GT_BOOL foundPlace = GT_FALSE;
    GT_U32  partIndex = 1;
    GT_U32  save_sbmFirstIndex = GT_NA;
    const SUPPER_SBM_INFO_STC*    save_currSupperSbmInfoPtr = NULL;
    GT_BOOL  supperSbmWithArp[MAX_SUPER_SBM_NUM] = {0};
    SBM_CLIENT_TYPE_ENT representativeClient;
    GT_U32  numOfSuperSbm;
    GT_U32  maxSbmNum;
    const SUPPER_SBM_INFO_STC* const *    devSupperSbmInfoArr;
    GT_U32    sbmIndexBmp[2] = {0,0};
    GT_BOOL   forceFirstIndex = GT_FALSE;

    if(lastSupperSbmFoundPtr == NULL || ((*lastSupperSbmFoundPtr) == GT_NA))
    {
        startSupperSmbOffset = 0;
    }
    else
    {
        startSupperSmbOffset = (*lastSupperSbmFoundPtr) + 1;

        if(clientType == (SBM_CLIENT_TYPE_LPM_0_E + 20))
        {
            startSupperSmbOffset = 0x4;/* instead match on 0x2 */
        }
    }

    devSupperSbmInfoArr = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? Hawk_super_SBM_support : Falcon_super_SBM_support;
    numOfSuperSbm = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;

    representativeClient = getRepresentativeClient(clientType);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        if(representativeClient == SBM_CLIENT_TYPE_EM_0_E)
        {
            if(globalInfoPtr->fdb_numOfSbm == 4 &&
               globalInfoPtr->em_numOfSbm == 16)
            {
                if(clientType == SBM_CLIENT_TYPE_EM_0_E ||
                   clientType == SBM_CLIENT_TYPE_EM_1_E )
                {
                    startSupperSmbOffset = 8;
                }
                else
                if(clientType == SBM_CLIENT_TYPE_EM_2_E ||
                   clientType == SBM_CLIENT_TYPE_EM_3_E )
                {
                    startSupperSmbOffset = 6;
                }
            }
            if(globalInfoPtr->fdb_numOfSbm == 16 &&
               globalInfoPtr->em_numOfSbm == 4 &&
               globalInfoPtr->arp_numOfSbm == 12)
            {
                if(clientType == SBM_CLIENT_TYPE_EM_0_E ||
                   clientType == SBM_CLIENT_TYPE_EM_1_E )
                {
                    startSupperSmbOffset = 6;
                }
                else
                if(clientType == SBM_CLIENT_TYPE_EM_2_E ||
                   clientType == SBM_CLIENT_TYPE_EM_3_E )
                {
                    startSupperSmbOffset = 2;
                }
            }
        }
        else
        if(representativeClient == SBM_CLIENT_TYPE_ARP_E)
        {
            if(lastSupperSbmFoundPtr == NULL || *lastSupperSbmFoundPtr == GT_NA)
            {
                /* Supper SBM 9 is for ARPs only , so start with it */
                startSupperSmbOffset = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm - 1;
            }
        }
        else
        if(representativeClient == SBM_CLIENT_TYPE_FDB_0_E)
        {
            if(globalInfoPtr->lpm_numOfSbm ==28)
            {
                startSupperSmbOffset = 0;
            }
            if(globalInfoPtr->fdb_numOfSbm == 4 &&
               globalInfoPtr->em_numOfSbm >= 16)
            {
                if(clientType == SBM_CLIENT_TYPE_FDB_0_E)
                {
                    startSupperSmbOffset = 0;
                }
                else
                if(clientType == SBM_CLIENT_TYPE_FDB_2_E)
                {
                    startSupperSmbOffset = 6;
                }
                else
                if(clientType == SBM_CLIENT_TYPE_FDB_3_E)
                {
                    startSupperSmbOffset = 7;
                }
            }
            else
            if(globalInfoPtr->fdb_numOfSbm > 8 ||
              (globalInfoPtr->fdb_numOfSbm + globalInfoPtr->em_numOfSbm) >= 16)
            {
                startSupperSmbOffset = 0;
            }
            else
            {
                startSupperSmbOffset = 4;
            }
        }
        else
        if(representativeClient == SBM_CLIENT_TYPE_LPM_0_E)
        {
            if(clientType == SBM_CLIENT_TYPE_LPM_0_E)
            {
                startSupperSmbOffset = 8;
            }
            else
            if(clientType >= SBM_CLIENT_TYPE_LPM_4_E &&
               clientType <= SBM_CLIENT_TYPE_LPM_7_E)
            {
                if(isDoubleClientMode == GT_TRUE)
                {
                    startSupperSmbOffset = 0;
                    if(globalInfoPtr->em_numOfSbm == 0)
                    {
                        /* when no EM only LPM 0..7 will use it */
                        startSupperSmbOffset = 8;
                    }
                }
                else
                {
                    startSupperSmbOffset = 8;
                }
            }
            else
            if(clientType == SBM_CLIENT_TYPE_LPM_10_E ||
               clientType == SBM_CLIENT_TYPE_LPM_11_E )
            {
                if(globalInfoPtr->fdb_numOfSbm == 4 &&
                   globalInfoPtr->em_numOfSbm == 16)
                {
                    startSupperSmbOffset = 4;
                    forceFirstIndex = GT_TRUE;
                }
            }
        }
    }


    tmp_supperSbmIndex = startSupperSmbOffset;
    /* look for supper SBMs that the client can be places in */
    for(/* continue*/ ;tmp_supperSbmIndex < (numOfSuperSbm+startSupperSmbOffset);
        tmp_supperSbmIndex++)
    {
        /* support 'wraparound' of the supperSbmIndex */
        supperSbmIndex = (tmp_supperSbmIndex % numOfSuperSbm);

        supperSbmFound = GT_FALSE;
        currSupperSbmInfoPtr = devSupperSbmInfoArr[supperSbmIndex];
        while(currSupperSbmInfoPtr->client != SBM_CLIENT_TYPE__LAST__E)
        {
            if(clientType == currSupperSbmInfoPtr->client)
            {
                /* client can be in this supper SBM */
                clientName = currSupperSbmInfoPtr->clientName;
                supperSbmFound = GT_TRUE;
                break;
            }

            currSupperSbmInfoPtr++;
        }

        if(supperSbmFound == GT_FALSE)
        {
            continue;
        }

        if(clientType == SBM_CLIENT_TYPE_ARP_E &&
           (0 == ((1<<supperSbmIndex) & PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.bmpSbmSupportMultipleArp)))/* last supper SBM supports multiple ARP clients */
        {
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
            }
            else
            {

                for(sbmIndex =    supperSbmIndex * FALCON_SBM_IN_SUPER_NUM;
                    sbmIndex < (1+supperSbmIndex)* FALCON_SBM_IN_SUPER_NUM;
                    sbmIndex++)
                {
                    if(ARP_REG_VALUE == infoArr[sbmIndex])
                    {
                        supperSbmWithArp[supperSbmIndex] = GT_TRUE;

                        /* this super SBM can not hold the 'new ARP' , we need other supper SBM */
                        break;
                    }
                }

                if(sbmIndex != ((1+supperSbmIndex)* FALCON_SBM_IN_SUPER_NUM))
                {
                    if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
                    {
                        cpssOsPrintf("super SBM [%d] can not hold the 'new ARP' , we need other supper SBM \n",
                            supperSbmIndex);
                    }

                    /* this super SBM can not hold the 'new ARP' , we need other supper SBM */
                    continue;
                }
            }
        }


        /* check if this supper SBM can hold the client (or already 'full') */
        for(sbmIndex =    supperSbmIndex * FALCON_SBM_IN_SUPER_NUM;
            sbmIndex < (1+supperSbmIndex)* FALCON_SBM_IN_SUPER_NUM;
            sbmIndex++)
        {
            if (infoArr[sbmIndex] == 0)
            {
                /* found empty place for it */
                if(isDoubleClientMode == GT_TRUE &&
                    GT_FALSE == isLpmClientsForceStartAsFirst(devNum,clientType))
                {
                    /* need to keep looking for sbm that may hold this client as 'second clien' and not open new index */
                    if(save_sbmFirstIndex == GT_NA)
                    {
                        save_sbmFirstIndex = sbmIndex;/* save the index ... to be used if not find 'better' index */
                        save_currSupperSbmInfoPtr = currSupperSbmInfoPtr;
                    }
                    continue;
                }

                if(clientType == SBM_CLIENT_TYPE_ARP_E &&
                   PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) &&
                   GT_FALSE == sip6_10CheckArpClientValidityAndSetAsUsed(sbmIndex,infoArr,
                   clientTypeArr,sbmIndexBmp))
                {
                    /* the ARP not belong into this index */
                    continue;
                }

                /*forceNewRegister_lbl:*/

                /* fill it with needed info */
                infoArr[sbmIndex] =  FILL_REG_INFO_MAC(
                    currSupperSbmInfoPtr->regValue_0,
                    currSupperSbmInfoPtr->regValue_1);

                foundPlace = GT_TRUE;
            }
            else
            if(isDoubleClientMode == GT_TRUE && /* LPM 7K mode */
               forceFirstIndex == GT_FALSE &&
               IS_SECOND_FREE_CHECK_REG_INFO_MAC(infoArr[sbmIndex])) /* register hold single client ... but can hold 2 clients */
            {
                /* found empty second place for it */
                infoArr[sbmIndex] =  FILL_SECOND_CLIENT_REG_INFO_MAC(
                    currSupperSbmInfoPtr->regValue_0,
                    infoArr[sbmIndex]);

                foundPlace = GT_TRUE;
                partIndex  = 2;
            }

            if(foundPlace == GT_TRUE)
            {
treatFoundIndex_lbl:

                if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables) && clientName)
                {
                    if(isDoubleClientMode == GT_FALSE)
                    {
                        if(clientType == SBM_CLIENT_TYPE_ARP_E &&
                               PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                        {
                            GT_U32  ii;
                            for(ii = 0 ; ii < numOfSuperSbm * FALCON_SBM_IN_SUPER_NUM; ii++)
                            {
                                if(0 == (sbmIndexBmp[ii>>5] & (1 << (ii & 0x1f))))
                                {
                                    continue;

                                }
                                cpssOsPrintf("client[%s] :into SBM [%d] (supper SBM[%d])\n",
                                    clientName,
                                    ii,
                                    ii/FALCON_SBM_IN_SUPER_NUM);
                            }
                        }
                        else
                        {
                            cpssOsPrintf("client[%s] :into SBM [%d] (supper SBM[%d])\n",
                                clientName,
                                sbmIndex,
                                supperSbmIndex);
                        }
                    }
                    else
                    {
                        cpssOsPrintf("client[%s] :into SBM [%d] part[%d] (supper SBM[%d])\n",
                            clientName,
                            sbmIndex,
                            partIndex,
                            supperSbmIndex);
                    }
                }

                if(lastSupperSbmFoundPtr)
                {
                    if(isDoubleClientMode == GT_TRUE) /* LPM 7K mode */
                    {
                        /* indicate the caller to 're-visit' current supper bloack */
                        if(supperSbmIndex == 0)
                        {
                            *lastSupperSbmFoundPtr = GT_NA;
                        }
                        else
                        {
                            *lastSupperSbmFoundPtr = supperSbmIndex - 1;
                        }
                    }
                    else
                    {
                        *lastSupperSbmFoundPtr = supperSbmIndex;
                    }
                }

                clientTypeArr[sbmIndex][partIndex-1] = clientType;
                return GT_OK;
            }

        }
    }/* loop on supperSbmIndex */

    if(save_sbmFirstIndex != GT_NA)
    {
        sbmIndex        = save_sbmFirstIndex;
        supperSbmIndex  = save_sbmFirstIndex / FALCON_SBM_IN_SUPER_NUM;

        if(save_currSupperSbmInfoPtr)
        {
            currSupperSbmInfoPtr = save_currSupperSbmInfoPtr;

            /* fill it with needed info */
            infoArr[sbmIndex] =  FILL_REG_INFO_MAC(
                currSupperSbmInfoPtr->regValue_0,
                currSupperSbmInfoPtr->regValue_1);
        }

        goto treatFoundIndex_lbl;
    }

    maxSbmNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm *
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;

    /******************/
    /* retry to move one client and replace with own */
    /* the ARP/LPM when last one ... need some client to make room for it (if possible) */
    /******************/
    if(recursiveLevel < END_RECURSIVE_LEVEL && /* 0 or 1 */
       (representativeClient == SBM_CLIENT_TYPE_ARP_E ||  /* replace with LPM/EM */
        representativeClient == SBM_CLIENT_TYPE_LPM_0_E)) /* replace with LPM/EM */
    {
        SBM_CLIENT_TYPE_ENT currClientType,currRepresentativeClient;

        supperSbmIndex = 0;
        for(sbmIndex = 0;
            sbmIndex < maxSbmNum;
            sbmIndex++)
        {
            if((sbmIndex % FALCON_SBM_IN_SUPER_NUM == 0))
            {
                supperSbmIndex = sbmIndex / FALCON_SBM_IN_SUPER_NUM;

                if (supperSbmIndex >= MAX_SUPER_SBM_NUM)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                if(supperSbmWithArp[supperSbmIndex])
                {
                    /* already have ARP in this supper SBM */
                    sbmIndex += FALCON_SBM_IN_SUPER_NUM-1;
                    continue;
                }
            }

            currClientType = clientTypeArr[sbmIndex][partIndex-1];
            currRepresentativeClient = getRepresentativeClient(currClientType);

            if(currRepresentativeClient == representativeClient)
            {
                continue;
            }

            if(currClientType == clientType)
            {
                continue;
            }

            /* look for other place for 'currClientType' */
            if(currRepresentativeClient == SBM_CLIENT_TYPE_EM_0_E ||
               currRepresentativeClient == SBM_CLIENT_TYPE_LPM_0_E ||
               currRepresentativeClient == SBM_CLIENT_TYPE_FDB_0_E)
            {
                GT_U32  origValue = infoArr[sbmIndex];

                if(clientTypeArr[sbmIndex][partIndex])
                {
                    /* The LPM uses double mode */
                    continue;
                }

                /* need to check if the client can be in this supper SMB */

                supperSbmFound = GT_FALSE;
                currSupperSbmInfoPtr = devSupperSbmInfoArr[supperSbmIndex];
                while(currSupperSbmInfoPtr->client != SBM_CLIENT_TYPE__LAST__E)
                {
                    if(clientType == currSupperSbmInfoPtr->client)
                    {
                        /* client can be in this supper SBM */
                        supperSbmFound = GT_TRUE;
                        break;
                    }

                    currSupperSbmInfoPtr++;
                }

                if(supperSbmFound == GT_FALSE)
                {
                    continue;
                }

                infoArr[sbmIndex] =  FILL_REG_INFO_MAC(
                    currSupperSbmInfoPtr->regValue_0,
                    currSupperSbmInfoPtr->regValue_1);

                clientTypeArr[sbmIndex][partIndex-1] = clientType;

                /* do recursive call after we replaced client from current index
                   with the one we need to place ... now we check if the moved
                   client can find other place to be */
                rc = sharedTableAddClient(devNum,
                    globalInfoPtr,
                    recursiveLevel + 1,currClientType,
                    GT_FALSE,
                    infoArr,
                    clientTypeArr,
                    NULL/*lastSupperSbmFoundPtr*/);
                if(rc != GT_OK)
                {
                    /* restore */
                    infoArr[sbmIndex] = origValue;
                    clientTypeArr[sbmIndex][partIndex-1] = currClientType;
                    /* we continue ... to check other clients */
                }
                else
                {
                    /* we managed to swap ! */
                    if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables) && clientName)
                    {
                        cpssOsPrintf("client[%s] :into SBM [%d] (supper SBM[%d]) (replaced with %s + %d)\n",
                            clientName,
                            sbmIndex,
                            supperSbmIndex,
                            getRepresentativeClientName(currClientType),
                            currClientType - currRepresentativeClient);
                    }
                    return GT_OK;
                }

            }

        }
    }


    /************************************************/
    /* if we got here we have error 'GT_NO_RESOURCE'*/
    /************************************************/

    if(recursiveLevel != START_RECURSIVE_LEVEL)
    {
        /* due to 'iterations' ... no need to 'error' it */
        return /* this is not error for the ERROR LOG */ GT_NO_RESOURCE;
    }
    else
    if(clientName)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "client[%s] : not have place in the array",
            clientName);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "client [%d] : not have place in the array",
            clientType);
    }
}

/**
* @internal wrap_sharedTableAddClient function
* @endinternal
*
* @brief   function to allow call to sharedTableAddClient with several iterations
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            clientType      - the client to add to the array
*            dupOnHalf       - indication that need to call sharedTableAddClient twice
*            isDoubleClientMode - is double client mode (for LPM this is 7K/14K mode)
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*            lastSupperSbmFoundPtr - hold index in infoArr[] and clientTypeArr[] of the previous iteration
*                                   ignored when NULL
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about the 'clientType'
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about the 'clientType'
*            lastSupperSbmFoundPtr - hold index in infoArr[] and clientTypeArr[] of the previous iteration
*                               hold the index that was used by current function
*                                   ignored when NULL
*
* @retval   - GT_OK - added the client properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the client.
*/
static GT_STATUS wrap_sharedTableAddClient(
    IN      GT_U8                devNum,
    IN      SHM_GLOBAL_INFO_STC  *globalInfoPtr,
    IN      SBM_CLIENT_TYPE_ENT  clientType,
    IN      GT_BOOL              dupOnHalf,
    IN      GT_BOOL              isDoubleClientMode,/* for LPM 7K mode */
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM],
    INOUT   GT_U32               *lastSupperSbmFoundPtr
)
{
    GT_STATUS rc;

    if(dupOnHalf == GT_TRUE)
    {
        /* first time */
        rc = sharedTableAddClient(devNum,
            globalInfoPtr,
            START_RECURSIVE_LEVEL,clientType,isDoubleClientMode,infoArr,clientTypeArr,lastSupperSbmFoundPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* second time */
    }

    rc = sharedTableAddClient(devNum,
            globalInfoPtr,
            START_RECURSIVE_LEVEL,clientType,isDoubleClientMode,infoArr,clientTypeArr,lastSupperSbmFoundPtr);

    return rc;
}

/**
* @internal isSecondPriority function
* @endinternal
*
* @brief   function to check if the index is in the list of 'second priority'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            currIndex      - the index to check
*            secondPriorityPtr - pointer to list of indexes of second priority
*               (ended by 'GT_NA')
* @param[out]
*           NONE.
*
* @retval   - GT_TRUE  -     part of the list (second priority).
* @retval   - GT_FALSE - not part of the list (first  priority).
*/
static GT_BOOL  isSecondPriority(
    IN GT_U32   currIndex,
    IN const GT_U32  *secondPriorityPtr
)
{
    GT_U32  jj = 0;
    while(secondPriorityPtr[jj] != GT_NA)
    {
        if(secondPriorityPtr[jj] == currIndex)
        {
            break;
        }
        jj++;
    }

    if(secondPriorityPtr[jj] != GT_NA)
    {
        /* found to be 'second priority' */
        return GT_TRUE;
    }

    return GT_FALSE;
}


/**
* @internal getPriority function
* @endinternal
*
* @brief   function to get priority for the index is in the list of clients
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            currIndex      - the index to check
*            clientsListPtr - pointer to list of clients
* @param[out]
*           NONE.
*
* @retval   - priority value.
*/
static GT_U32  getPriority(
    IN GT_U32   currIndex,
    IN GT_U32  *clientsListPtr,
    IN GT_U32   representativeClient,
    IN GT_U32   clientsNum
)
{
    GT_U32  ii;
    GT_U32  lowPrio = GT_NA;
    for(ii = representativeClient ; ii < representativeClient + clientsNum; ii++)
    {
        if(lowPrio > clientsListPtr[ii])
        {
            lowPrio = clientsListPtr[ii];
        }
    }
    /* normalize the priority according to the actual available clients */
    /* like in EM the first 4 clients have much more places than others that makes them priority 2,3,
       but should be treated as priority 0,1 if only those 4 exists */
    return clientsListPtr[currIndex] - lowPrio;
}


/**
* @internal getNextIterator function
* @endinternal
*
* @brief   function to get next index for iteration
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            currIndexPtr   - (pointer to) the current index
*                               value of GT_NA means (iteration just stared)
*            reverseOrder - indication that iteration in revere (-- instead of ++)
*            maxValue     -  the max value to iterate.
*                           when reverseOrder == GT_FALSE --> iterate till max value
*                           when reverseOrder == GT_TRUE  --> iterate start from max value
* @param[out]
*           NONE.
*
* @retval   - GT_OK      - the function return new valid iteration index.
* @retval   - GT_NO_MORE - no more iterations available
*/
static GT_STATUS    getNextIterator(
    IN GT_U32   *currIndexPtr,
    IN GT_BOOL  reverseOrder,
    IN GT_U32   maxValue
)
{
    if(reverseOrder == GT_FALSE)
    {
        if((*currIndexPtr) == GT_NA)
        {
            (*currIndexPtr) = 0;
        }
        else
        if((*currIndexPtr) >= (maxValue-1))
        {
            return /* not error to the LOG */ GT_NO_MORE;
        }
        else
        {
            (*currIndexPtr)++;
        }
    }
    else
    {
        if((*currIndexPtr) == GT_NA)
        {
            (*currIndexPtr) = maxValue-1;
        }
        else
        if((*currIndexPtr) == 0)
        {
            return /* not error to the LOG */ GT_NO_MORE;
        }
        else
        {
            (*currIndexPtr)--;
        }
    }

    return GT_OK;
}

/* FALCON : the max number of SMBs for the LPM */
#define FALCON_MAX_SBM_LPM      28
/* FALCON : the max number of SMBs for the FDB */
#define FALCON_MAX_SBM_FDB      32
/* FALCON : the max number of SMBs for the EM */
#define FALCON_MAX_SBM_EM       16
/* FALCON : the max number of SMBs for the ARP */
#define FALCON_MAX_SBM_ARP       8
/* HAWK : the max number of SMBs for the EM */
#define HAWK_MAX_SBM_EM         32
/* HAWK : the max number of SMBs for the ARP */
#define HAWK_MAX_SBM_ARP        24

/* macro to check parameter for 'max value' and return error via the 'error LOG' of the CPSS*/
#define CHECK_PARAM_TO_MAX_VALUE(value,max_value)    \
    if((value) > (max_value))               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"[%s] with value [%d] is more than max[%s] [%d]",    \
            #value,value,#max_value,max_value)

/* values for high and low priorities in parameter 'priorityLevel' */
#define PRIORITY_HIGH   0
#define PRIORITY_LOW    1
#define MAX_PRIORITY_LEVEL  4

/**
* @internal sharedTablesAddTable function
* @endinternal
*
* @brief   function to call wrap_sharedTableAddClient/sharedTableAddClient for
*           the 'num of clients' needed starting from 'start client'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            clientType      - the 'start' client to add to the array
*            clientsNum      - the number of clients (from the 'start')
*            isDoubleClientMode - is double client mode (for LPM this is 7K/14K mode)
*            priorityLevel   - indication about the priority : PRIORITY_HIGH / PRIORITY_LOW
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the 'clientType + clientsNum'
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the 'clientType + clientsNum'
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS sharedTablesAddTable(
    IN      GT_U8                devNum,
    IN      SHM_GLOBAL_INFO_STC  *globalInfoPtr,
    IN      SBM_CLIENT_TYPE_ENT  clientType,
    IN      GT_U32               clientsNum,
    IN      GT_BOOL              isDoubleClientMode,/* for LPM 7K mode */
    IN      GT_U32               priorityLevel,/* PRIORITY_HIGH , PRIORITY_LOW */
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_STATUS rc;
    GT_U32  ii;
    GT_BOOL dupOnHalf = GT_FALSE;
    const GT_U32  *secondPriorityPtr = NULL;
    GT_U32  lastSupperSbmFound = GT_NA;
    GT_U32  *lastSupperSbmFoundPtr = NULL;
    GT_BOOL isReverseOrder = GT_TRUE;
    SBM_CLIENT_TYPE_ENT representativeClient;

    representativeClient = getRepresentativeClient(clientType);

    /* ARP is special client */
    if(representativeClient == SBM_CLIENT_TYPE_ARP_E)
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            lastSupperSbmFoundPtr = &lastSupperSbmFound;
        }
        for(ii = 0 ; ii < clientsNum ; ii++)
        {

            rc = sharedTableAddClient(devNum,
                globalInfoPtr,
                START_RECURSIVE_LEVEL,clientType/* the same client again*/,isDoubleClientMode,infoArr,clientTypeArr,lastSupperSbmFoundPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                /* the function sip6_10CheckArpClientValidityAndSetAsUsed(...)
                   is making 4 ARP client at once ! so need to consider here the extra 3
                */
                ii += 3;
                lastSupperSbmFound += 3;
            }


        }
        return GT_OK;
    }

    if(representativeClient == SBM_CLIENT_TYPE_EM_0_E)
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            secondPriorityPtr = NULL;
        }
        else
        {
            secondPriorityPtr = falconEmClientsPriorityLowArr;
        }
        isReverseOrder = GT_FALSE;/* EM3 hold the most locations (more than EM0,EM1,EM2)*/
        if(clientsNum == 32)
        {
            /* special case of : 256K EM (like in FDB) */
            /* we need 2 loops on FDB_0..FDB_15 */
            dupOnHalf  = GT_TRUE;
            clientsNum /= 2;
        }
    }
    else
    if(representativeClient == SBM_CLIENT_TYPE_LPM_0_E)
    {
        if(isDoubleClientMode == GT_TRUE)
        {
            /* use the x SBMs to hold double the LPM clients */
            clientsNum *= 2;/* less than 28 ! */
            CHECK_PARAM_TO_MAX_VALUE(clientsNum,FALCON_MAX_SBM_LPM);

            secondPriorityPtr = falconLpmClientsPriorityLowArr_halfMode;
        }
        else
        {
            secondPriorityPtr = falconLpmClientsPriorityLowArr_fullMode;
        }

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            secondPriorityPtr = NULL;
        }


        lastSupperSbmFoundPtr = &lastSupperSbmFound;
        isReverseOrder = GT_FALSE;

        if(clientsNum == FALCON_MAX_SBM_LPM)
        {
            secondPriorityPtr = NULL;
        }
    }
    else
    if(representativeClient == SBM_CLIENT_TYPE_FDB_0_E)
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            secondPriorityPtr = NULL;
        }
        else
        {
            secondPriorityPtr = falconFdbClientsPriorityLowArr;
        }

        if(clientsNum == 32)
        {
            /* special case of : 256K FDB */
            /* we need 2 loops on FDB_0..FDB_15 */
            dupOnHalf  = GT_TRUE;
            clientsNum /= 2;
        }
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* do not skip */
    }
    else
    if(secondPriorityPtr == NULL && priorityLevel != PRIORITY_HIGH)
    {
        /* all indexes done in the 'priority high' loop */
        return GT_OK;
    }

    /* start iterations */
    ii = GT_NA;

    /* first priority */
    while(GT_OK == getNextIterator(&ii,isReverseOrder,clientsNum))
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            GT_U32  client_priorityLevel = getPriority(clientType + ii,CONFIG_GLOBAL_VAR_GET(hawkPriorityArr),representativeClient,clientsNum);

            if(secondPriorityPtr)
            {
                if(GT_TRUE == isSecondPriority(ii,secondPriorityPtr))
                {
                    /* consider the priority lower than it should be */
                    client_priorityLevel += 1;
                }
            }

            if(client_priorityLevel > MAX_PRIORITY_LEVEL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "client_priorityLevel[%d] > 'max'[%d]",
                    client_priorityLevel,MAX_PRIORITY_LEVEL);
            }

            if(priorityLevel != client_priorityLevel)
            {
                continue;
            }
        }
        else
        {
            if(secondPriorityPtr)
            {
                GT_BOOL isSecond = isSecondPriority(ii,secondPriorityPtr);

                if(GT_TRUE == isSecond && priorityLevel == PRIORITY_HIGH)
                {
                    /* found to be 'second priority' */
                    continue;/* skip to next ii */
                }

                if(GT_FALSE == isSecond && priorityLevel == PRIORITY_LOW)
                {
                    /* not found to be 'second priority' ... meaning already done */
                    continue;/* skip to next ii */
                }
            }
        }

        rc = wrap_sharedTableAddClient(devNum,
            globalInfoPtr,
            clientType + ii,dupOnHalf,isDoubleClientMode,infoArr,clientTypeArr,lastSupperSbmFoundPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/* each 8K EM/FDB entries needs SBM */
#define FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR     _8K

/**
* @internal sharedTablesResourceSet function
* @endinternal
*
* @brief   function to fill the SMBs with info about the clients that relate to a 'table'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            tableType      - the type of table
*            tableSize      - the number of entries/lines that the table hold
*            priorityLevel   - indication about the priority : PRIORITY_HIGH / PRIORITY_LOW
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for the table
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for the table
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS sharedTablesResourceSet(
    IN      GT_U8                devNum,
    IN      SHM_GLOBAL_INFO_STC  *globalInfoPtr,
    IN      CPSS_DXCH_CFG_TABLES_ENT    tableType,
    IN      GT_U32                      tableSize,
    IN      GT_U32                      priorityLevel,
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_U32              numOfSbm;
    SBM_CLIENT_TYPE_ENT clientType;
    GT_BOOL             isDoubleClientMode = GT_FALSE;/* for LPM 7K mode */

    if(tableSize == 0)
    {
        /* LPM,EM expected to support it */
        return GT_OK;
    }

    switch(tableType)
    {
        case CPSS_DXCH_CFG_TABLE_FDB_E:
            numOfSbm   = tableSize / FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
            clientType = SBM_CLIENT_TYPE_FDB_0_E;
            CHECK_PARAM_TO_MAX_VALUE(numOfSbm,FALCON_MAX_SBM_FDB);
            break;
        case CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E:
            numOfSbm = tableSize / FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
            clientType = SBM_CLIENT_TYPE_EM_0_E;
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CHECK_PARAM_TO_MAX_VALUE(numOfSbm,HAWK_MAX_SBM_EM);
            }
            else
            {
                CHECK_PARAM_TO_MAX_VALUE(numOfSbm,FALCON_MAX_SBM_EM);
            }
            break;
        case CPSS_DXCH_CFG_TABLE_LPM_RAM_E:
            numOfSbm = tableSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;
            clientType = SBM_CLIENT_TYPE_LPM_0_E;
            CHECK_PARAM_TO_MAX_VALUE(numOfSbm,FALCON_MAX_SBM_LPM);
            if(numOfSbm < ((FALCON_MAX_SBM_LPM/2) + 1))
            {
                isDoubleClientMode = GT_TRUE;
            }
            break;
        case CPSS_DXCH_CFG_TABLE_ARP_E:
            numOfSbm = tableSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;
            clientType = SBM_CLIENT_TYPE_ARP_E;
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CHECK_PARAM_TO_MAX_VALUE(numOfSbm,HAWK_MAX_SBM_ARP);
            }
            else
            {
                CHECK_PARAM_TO_MAX_VALUE(numOfSbm,FALCON_MAX_SBM_ARP);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(tableType);
    }

    return sharedTablesAddTable(devNum,
        globalInfoPtr,
        clientType,numOfSbm,isDoubleClientMode,priorityLevel,infoArr,clientTypeArr);
}

/**
* @internal sharedTablesResourceRemove function
* @endinternal
*
* @brief   function to remove the SMBs from info about the clients that relate to a 'table'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            removedTable      - the type of table
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info after the clients of the table removed
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info after the clients of the table removed
*
* @retval   - GT_OK - always
*/
static GT_STATUS sharedTablesResourceRemove(
    IN      GT_U8                devNum,
    IN      CPSS_DXCH_CFG_TABLES_ENT  removedTable,
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_U32  sbmIndex;
    SBM_CLIENT_TYPE_ENT  currClientType;
    GT_U32  partIndex = 1;
    SBM_CLIENT_TYPE_ENT  representativeClient;
    SBM_CLIENT_TYPE_ENT  removedRepresentativeClient;
    GT_U32  maxSbmNum;

    switch(removedTable)
    {
        case CPSS_DXCH_CFG_TABLE_FDB_E:
            removedRepresentativeClient = SBM_CLIENT_TYPE_FDB_0_E;
            break;
        case CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E:
            removedRepresentativeClient = SBM_CLIENT_TYPE_EM_0_E;
            break;
        case CPSS_DXCH_CFG_TABLE_LPM_RAM_E:
            removedRepresentativeClient = SBM_CLIENT_TYPE_LPM_0_E;
            break;
        case CPSS_DXCH_CFG_TABLE_ARP_E:
            removedRepresentativeClient = SBM_CLIENT_TYPE_ARP_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(removedTable);
    }

    maxSbmNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm *
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;

    for(sbmIndex = 0; sbmIndex < maxSbmNum; sbmIndex++)
    {
        currClientType = clientTypeArr[sbmIndex][partIndex-1];
        representativeClient = getRepresentativeClient(currClientType);

        if(representativeClient == removedRepresentativeClient)
        {
            /*********************/
            /* remove the client */
            /*********************/
            if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
            {
                cpssOsPrintf("Removed [%s + %d] from SBM [%d] supper SBM[%d] \n",
                    getRepresentativeClientName(currClientType),
                    currClientType - representativeClient,
                    sbmIndex,
                    sbmIndex / FALCON_SBM_IN_SUPER_NUM);
            }

            infoArr[sbmIndex] = 0;
            clientTypeArr[sbmIndex][partIndex-1] = 0;
        }
    }

    return GT_OK;
}

/**
* @internal sharedTablesResourceRetry function
* @endinternal
*
* @brief   function to retry to fill the SMBs with info about the clients that relate to list of 'tables'
*           the function first remove clients of those tables , and then add then
*           one by one again with 'pririty' high and low
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            retryArr[]      - array of type of tables
*            sizeArr[]       - array of type the number of entries/lines that each table hold
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for the table
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for the table
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS sharedTablesResourceRetry(
    IN      GT_U8                devNum,
    IN      SHM_GLOBAL_INFO_STC  *globalInfoPtr,
    IN      GT_U32                 minPriority,
    IN CPSS_DXCH_CFG_TABLES_ENT    retryArr[],
    IN GT_U32                      sizeArr[],
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32      prio;

    /* remove tables */
    for(ii = 0 ; retryArr[ii] != CPSS_DXCH_CFG_TABLE_LAST_E ; ii++)
    {
        rc = sharedTablesResourceRemove(devNum,retryArr[ii],infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* add tables */
    for(prio = PRIORITY_HIGH ; prio <= minPriority ; prio++)
    {
        for(ii = 0 ; retryArr[ii] != CPSS_DXCH_CFG_TABLE_LAST_E ; ii++)
        {
            /* insert tables */
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                retryArr[ii],sizeArr[ii],prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}


/**
* @internal falcon_sharedTablesAllResourceSet function
* @endinternal
*
* @brief   function to fill ALL tables : for each the SMBs with info about the
*           clients that relate to a 'table'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            fdbSize        - the number of FDB entries (needed from the shared tables)
*            emSize         - the number of EM  entries (needed from the shared tables)
*            lpmSize        - the number of LPM entries (needed from the shared tables)
*            arpSize        - the number of ARP entries (needed from the shared tables)
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for all the tables
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for all the tables
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS falcon_sharedTablesAllResourceSet(
    IN      GT_U8                devNum,
    IN      GT_U32               fdbSize,
    IN      GT_U32               emSize,
    IN      GT_U32               lpmSize,
    IN      GT_U32               arpSize,
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_STATUS   rc;
    GT_U32      prio;
    SHM_GLOBAL_INFO_STC globalInfo;
    SHM_GLOBAL_INFO_STC *globalInfoPtr = &globalInfo;
    GT_BOOL     arpDone = GT_FALSE;
    GT_BOOL     isLpmLast = GT_FALSE;
    GT_U32 fdb_numOfSbm = fdbSize / FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
    GT_U32 em_numOfSbm  = emSize  / FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
    GT_U32 lpm_numOfSbm = lpmSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;
    GT_U32 arp_numOfSbm = arpSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;

    if((fdb_numOfSbm + em_numOfSbm) >= 12 && arp_numOfSbm == 8 && lpm_numOfSbm >= 12)
    {
        isLpmLast = GT_TRUE;
    }

    globalInfoPtr->fdb_numOfSbm = fdb_numOfSbm ;
    globalInfoPtr->em_numOfSbm  = em_numOfSbm  ;
    globalInfoPtr->lpm_numOfSbm = lpm_numOfSbm ;
    globalInfoPtr->arp_numOfSbm = arp_numOfSbm ;

    for(prio = PRIORITY_HIGH ; prio <= PRIORITY_LOW ; prio++)
    {
        /* FDB table */
        rc = sharedTablesResourceSet(devNum,
            globalInfoPtr,
            CPSS_DXCH_CFG_TABLE_FDB_E,fdbSize,prio,infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(arp_numOfSbm == 8 && lpmSize)
    {
        /* must fill  the ARPs 0..7 */
        rc = sharedTablesResourceSet(devNum,
            globalInfoPtr,
            CPSS_DXCH_CFG_TABLE_ARP_E,arpSize,PRIORITY_HIGH,infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        arpDone = GT_TRUE;
    }

    /* set tables according to 'priority' */
    for(prio = PRIORITY_HIGH ; prio <= PRIORITY_LOW ; prio++)
    {
        if(isLpmLast == GT_FALSE)
        {
            /* LPM Table */
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_LPM_RAM_E,lpmSize,prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* EM table */
        rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E,emSize,prio,infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            if(prio == PRIORITY_LOW && rc == GT_NO_RESOURCE)
            {
                GT_STATUS   rc1;
                CPSS_DXCH_CFG_TABLES_ENT    retryArr[] = {CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E,CPSS_DXCH_CFG_TABLE_FDB_E,CPSS_DXCH_CFG_TABLE_LAST_E};
                GT_U32                      sizeArr[]  = {0,0,0};

                /* need to reorder FDB ... maybe EM can fit */
                /* we keep the ARP/LPM in same position */
                sizeArr[0] = fdbSize;/* match indexes at retryArr[]*/
                sizeArr[1] =  emSize;/* match indexes at retryArr[]*/

                rc1 = sharedTablesResourceRetry(devNum,
                    globalInfoPtr,
                    PRIORITY_LOW,retryArr,sizeArr,infoArr,clientTypeArr);
                if(rc1 == GT_OK)
                {
                    /* we continue to do the ARP table */
                    break;
                }
            }

            return rc;
        }
    }

    /* last ARP */
    if(arpDone == GT_FALSE)
    {
        rc = sharedTablesResourceSet(devNum,
            globalInfoPtr,
            CPSS_DXCH_CFG_TABLE_ARP_E,arpSize,PRIORITY_HIGH,infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(isLpmLast == GT_TRUE)
    {
        for(prio = PRIORITY_HIGH ; prio <= PRIORITY_LOW ; prio++)
        {
            /* LPM Table */
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_LPM_RAM_E,lpmSize,prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }


    return GT_OK;
}

/**
* @internal sharedTablesResourceRemove function
* @endinternal
*
* @brief   function to clear the info in infoArr[] , clientTypeArr[][]
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[out]
*            infoArr[]       - the array with all values 0
*            clientTypeArr[][] - the array with all values 0
*
* @retval   - GT_OK - always
*/
static void sharedTablesResourceRemoveAll(
    OUT   GT_U32               infoArr[MAX_SBM_NUM],
    OUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_U32  ii;
    for(ii = 0 ; ii < MAX_SBM_NUM ; ii++)
    {
        infoArr[ii] = 0;
        clientTypeArr[ii][0] = clientTypeArr[ii][1] = 0;
    }
    return ;
}

#define FORCE_FIRST_TABLE   BIT_30
#define FORCE_LAST_TABLE    BIT_31
/**
* @internal hawk_sharedTablesAllResourceSet function
* @endinternal
*
* @brief   function to fill ALL tables : for each the SMBs with info about the
*           clients that relate to a 'table'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            fdbSize        - the number of FDB entries (needed from the shared tables)
*            emSize         - the number of EM  entries (needed from the shared tables)
*            lpmSize        - the number of LPM entries (needed from the shared tables)
*            arpSize        - the number of ARP entries (needed from the shared tables)
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for all the tables
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for all the tables
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS hawk_sharedTablesAllResourceSet_byOrder(
    IN      GT_U8                devNum,
    IN      SHM_GLOBAL_INFO_STC  *globalInfoPtr,
    IN      GT_U32               fdbSize,
    IN      GT_U32               emSize,
    IN      GT_U32               lpmSize,
    IN      GT_U32               arpSize,

    IN      GT_U32               fdbTableOrder,
    IN      GT_U32               emTableOrder,
    IN      GT_U32               lpmTableOrder,
    IN      GT_U32               arpTableOrder,

    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_STATUS   rc;
    GT_U32      prio;
    GT_U32  minPriority = MAX_PRIORITY_LEVEL;/* must be 4 (not 3) , due to client_priorityLevel += 1; in function sharedTablesAddTable(...) */

    CPSS_DXCH_CFG_TABLES_ENT    tablesOrederArr[4];
    GT_U32                      tablesSizes[4];
    GT_BOOL                     isArpFirst = GT_FALSE;
    GT_BOOL                     isLpmLast  = GT_FALSE;
    GT_BOOL                     isLpmFirst = GT_FALSE;
    GT_BOOL                     isFdbFirst = GT_FALSE;
    GT_BOOL                     isEmFirst  = GT_FALSE;
    GT_U32                      lpmIndex;
    GT_U32                      arpIndex;
    GT_U32                      fdbIndex;
    GT_U32                      emIndex;

    /* clear leftovers if any from last failing iteration ! */
    sharedTablesResourceRemoveAll(infoArr,clientTypeArr);

    lpmIndex =  lpmTableOrder & ~ (FORCE_LAST_TABLE | FORCE_FIRST_TABLE);
    if(lpmTableOrder & FORCE_LAST_TABLE)
    {
        isLpmLast = GT_TRUE;
    }
    if(lpmTableOrder & FORCE_FIRST_TABLE)
    {
        isLpmFirst = GT_TRUE;
    }
    arpIndex = arpTableOrder & ~ FORCE_FIRST_TABLE;
    if(arpTableOrder & FORCE_FIRST_TABLE)
    {
        isArpFirst = GT_TRUE;
    }

    fdbIndex = fdbTableOrder & ~ FORCE_FIRST_TABLE;
    if(fdbTableOrder & FORCE_FIRST_TABLE)
    {
        isFdbFirst = GT_TRUE;
    }

    emIndex = emTableOrder & ~ FORCE_FIRST_TABLE;
    if(emTableOrder & FORCE_FIRST_TABLE)
    {
        isEmFirst = GT_TRUE;
    }

    if ((fdbIndex >= 4) || (emIndex >= 4) || (lpmIndex >= 4) || (arpIndex >= 4))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    tablesOrederArr[fdbIndex ] = CPSS_DXCH_CFG_TABLE_FDB_E;
    tablesOrederArr[emIndex  ] = CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E;
    tablesOrederArr[lpmIndex ] = CPSS_DXCH_CFG_TABLE_LPM_RAM_E;
    tablesOrederArr[arpIndex ] = CPSS_DXCH_CFG_TABLE_ARP_E;

    tablesSizes[fdbIndex] = isFdbFirst == GT_TRUE ? GT_NA : fdbSize;
    tablesSizes[emIndex ] = isEmFirst  == GT_TRUE ? GT_NA : emSize;
    tablesSizes[lpmIndex] = isLpmFirst == GT_TRUE || isLpmLast == GT_TRUE ? GT_NA : lpmSize;
    tablesSizes[arpIndex] = isArpFirst == GT_TRUE ? GT_NA : arpSize;

    if(isArpFirst == GT_TRUE)
    {
        rc = sharedTablesResourceSet(devNum,
            globalInfoPtr,
            CPSS_DXCH_CFG_TABLE_ARP_E,arpSize,PRIORITY_HIGH,infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(isFdbFirst == GT_TRUE)
    {
        for(prio = PRIORITY_HIGH ; prio <= minPriority ; prio++)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_FDB_E,fdbSize,prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    if(isLpmFirst == GT_TRUE)
    {
        for(prio = PRIORITY_HIGH ; prio <= minPriority ; prio++)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_LPM_RAM_E,lpmSize,prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    if(isEmFirst == GT_TRUE)
    {
        for(prio = PRIORITY_HIGH ; prio <= minPriority ; prio++)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E,emSize,prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for(prio = PRIORITY_HIGH ; prio <= minPriority ; prio++)
    {
        if(tablesSizes[0] != GT_NA)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                tablesOrederArr[0],tablesSizes[0],prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(tablesSizes[1] != GT_NA)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                tablesOrederArr[1],tablesSizes[1],prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(tablesSizes[2] != GT_NA)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                tablesOrederArr[2],tablesSizes[2],prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(tablesSizes[3] != GT_NA &&
           tablesOrederArr[3] != CPSS_DXCH_CFG_TABLE_ARP_E)
        {
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                tablesOrederArr[3],tablesSizes[3],prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(tablesOrederArr[3] == CPSS_DXCH_CFG_TABLE_ARP_E &&
       isArpFirst == GT_FALSE)
    {
        rc = sharedTablesResourceSet(devNum,
            globalInfoPtr,
            CPSS_DXCH_CFG_TABLE_ARP_E,arpSize,PRIORITY_HIGH,infoArr,clientTypeArr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(isLpmLast == GT_TRUE)
    {
        for(prio = PRIORITY_HIGH ; prio <= minPriority ; prio++)
        {
            /* LPM Table */
            rc = sharedTablesResourceSet(devNum,
                globalInfoPtr,
                CPSS_DXCH_CFG_TABLE_LPM_RAM_E,lpmSize,prio,infoArr,clientTypeArr);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }


    return GT_OK;
}

/*
    calculate  infoArr[] from clientTypeArr[][]
*/
static GT_STATUS calcShmRegValueFromClients(
    IN      GT_U8                devNum,
    OUT   GT_U32               infoArr[MAX_SBM_NUM],
    IN    SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_U32  ii;
    const SUPPER_SBM_INFO_STC*    currSupperSbmInfoPtr;
    const SUPPER_SBM_INFO_STC* const *    devSupperSbmInfoArr;
    GT_U32  numOfSuperSbm = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;

    devSupperSbmInfoArr = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? Hawk_super_SBM_support : Falcon_super_SBM_support;

    for(ii = 0 ; ii < numOfSuperSbm * FALCON_SBM_IN_SUPER_NUM ; ii++)
    {
        currSupperSbmInfoPtr = devSupperSbmInfoArr[ii/FALCON_SBM_IN_SUPER_NUM];
        while(currSupperSbmInfoPtr->client != SBM_CLIENT_TYPE__LAST__E)
        {
            if(clientTypeArr[ii][0] == currSupperSbmInfoPtr->client)
            {
                /* fill it with needed info */
                infoArr[ii] =  FILL_REG_INFO_MAC(
                    currSupperSbmInfoPtr->regValue_0,
                    currSupperSbmInfoPtr->regValue_1);
                if(clientTypeArr[ii][1] != 0)
                {
                    /* 2 clients : LPM mode */
                    /* find the regValue of the second register */
                    currSupperSbmInfoPtr = devSupperSbmInfoArr[ii/FALCON_SBM_IN_SUPER_NUM];
                    while(currSupperSbmInfoPtr->client != SBM_CLIENT_TYPE__LAST__E)
                    {
                        if(clientTypeArr[ii][1] == currSupperSbmInfoPtr->client)
                        {
                            /* update the register value */
                            infoArr[ii] =  FILL_SECOND_CLIENT_REG_INFO_MAC(
                                currSupperSbmInfoPtr->regValue_0,
                                infoArr[ii]);
                            break;
                        }

                        currSupperSbmInfoPtr++;
                    }
                }
                break;
            }

            currSupperSbmInfoPtr++;
        }
    }

    return GT_OK;
}


/**
* @internal hawk_sharedTablesAllResourceSet function
* @endinternal
*
* @brief   function to fill ALL tables : for each the SMBs with info about the
*           clients that relate to a 'table'
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            fdbSize        - the number of FDB entries (needed from the shared tables)
*            emSize         - the number of EM  entries (needed from the shared tables)
*            lpmSize        - the number of LPM entries (needed from the shared tables)
*            arpSize        - the number of ARP entries (needed from the shared tables)
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*
* @param[out]
*            infoArr[]       - the array of value for the registers of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for all the tables
*            clientTypeArr[][] - the array of 'already' occupied clients (with empty places for others)
*                               hold the info about ALL the clients for all the tables
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS hawk_sharedTablesAllResourceSet(
    IN      GT_U8                devNum,
    IN      GT_U32               fdbSize,
    IN      GT_U32               emSize,
    IN      GT_U32               lpmSize,
    IN      GT_U32               arpSize,
    INOUT   GT_U32               infoArr[MAX_SBM_NUM],
    INOUT   SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]
)
{
    GT_STATUS rc;
    SHM_GLOBAL_INFO_STC globalInfo;
    SHM_GLOBAL_INFO_STC *globalInfoPtr = &globalInfo;
    GT_U32 fdb_numOfSbm;
    GT_U32 em_numOfSbm;
    GT_U32 arpTableOrder = 3 | FORCE_FIRST_TABLE;/* default : first table*/
    GT_U32 lpm_numOfSbm = lpmSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;
    GT_U32 arp_numOfSbm = arpSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;
    char*   debugString = NULL;

    fdb_numOfSbm = fdbSize / FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
    em_numOfSbm  = emSize  / FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;

    if(fdb_numOfSbm == 4 && em_numOfSbm == 16 && lpm_numOfSbm == 8 && arp_numOfSbm == 12)
    {
        static const SBM_CLIENT_TYPE_ENT final_clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM]=
        {
            {SBM_CLIENT_TYPE_FDB_0_E},   {SBM_CLIENT_TYPE_EM_4_E} , {SBM_CLIENT_TYPE_LPM_12_E, SBM_CLIENT_TYPE_LPM_14_E},  {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_FDB_1_E},   {SBM_CLIENT_TYPE_EM_2_E} , {SBM_CLIENT_TYPE_LPM_13_E, SBM_CLIENT_TYPE_LPM_15_E},  {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_FDB_2_E},   {SBM_CLIENT_TYPE_EM_8_E} , {SBM_CLIENT_TYPE_EM_12_E},                             {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_FDB_3_E},   {SBM_CLIENT_TYPE_EM_9_E} , {SBM_CLIENT_TYPE_EM_13_E},                             {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_EM_10_E} ,  {SBM_CLIENT_TYPE_EM_14_E}, {SBM_CLIENT_TYPE_LPM_8_E, SBM_CLIENT_TYPE_LPM_10_E} ,  {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_EM_11_E} ,  {SBM_CLIENT_TYPE_EM_15_E}, {SBM_CLIENT_TYPE_LPM_9_E, SBM_CLIENT_TYPE_LPM_11_E} ,  {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_EM_0_E },   {SBM_CLIENT_TYPE_EM_2_E} , {SBM_CLIENT_TYPE_EM_6_E},                              {SBM_CLIENT_TYPE_ARP_E},
            {SBM_CLIENT_TYPE_EM_1_E },   {SBM_CLIENT_TYPE_EM_3_E} , {SBM_CLIENT_TYPE_EM_7_E},                              {SBM_CLIENT_TYPE_ARP_E},

            {SBM_CLIENT_TYPE_LPM_0_E,SBM_CLIENT_TYPE_LPM_4_E},  {SBM_CLIENT_TYPE_LPM_1_E,SBM_CLIENT_TYPE_LPM_5_E},  {SBM_CLIENT_TYPE_LPM_2_E, SBM_CLIENT_TYPE_LPM_6_E},  {SBM_CLIENT_TYPE_LPM_3_E, SBM_CLIENT_TYPE_LPM_7_E},
            {SBM_CLIENT_TYPE_ARP_E},    {SBM_CLIENT_TYPE_ARP_E},    {SBM_CLIENT_TYPE_ARP_E},  {SBM_CLIENT_TYPE_ARP_E},
        };

        cpssOsMemCpy(clientTypeArr,final_clientTypeArr,sizeof(final_clientTypeArr));

        return calcShmRegValueFromClients(devNum,infoArr,clientTypeArr);
    }


    globalInfoPtr->fdb_numOfSbm = fdb_numOfSbm ;
    globalInfoPtr->em_numOfSbm  = em_numOfSbm  ;
    globalInfoPtr->lpm_numOfSbm = lpm_numOfSbm ;
    globalInfoPtr->arp_numOfSbm = arp_numOfSbm ;

    if(em_numOfSbm == 0 && lpm_numOfSbm == 0)
    {
        /* only FDB (32) and ARPs (8)  */
        /* let FDB be first  (case 19) */
        arpTableOrder &= ~ FORCE_FIRST_TABLE;
    }

    if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
    {
        if(0 == (arpTableOrder & FORCE_FIRST_TABLE))
        {
            debugString = "(case1) Try tables order : {FDB , EM , LPM} , ARP \n";
            /* let FDB be first */
            cpssOsPrintf(debugString);
        }
        else
        if(em_numOfSbm > fdb_numOfSbm)
        {
            debugString = "(case2) Try tables order : ARP , {EM , FDB , LPM} \n";
            /* let EM be second (after ARP) */
            cpssOsPrintf(debugString);
        }
        else
        {
            debugString = "(case3) Try tables order : ARP , {FDB , EM , LPM}  \n";
            /* let FDB be second (after ARP) */
            cpssOsPrintf(debugString);
        }
    }
    rc = hawk_sharedTablesAllResourceSet_byOrder(devNum,
        globalInfoPtr,
        fdbSize,emSize,lpmSize,arpSize,
        em_numOfSbm > fdb_numOfSbm ? 1:0/*fdb order*/,
        em_numOfSbm > fdb_numOfSbm ? 0:1/*em order*/,2/*lpm order*/,
        arpTableOrder/*arp order*/,
        infoArr,clientTypeArr);
    if(rc == GT_OK && debugString)
    {
        cpssOsPrintf("PASSED : %s",debugString);
    }

    if(rc != GT_OK)
    {
        if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
        {
            debugString = "(case4) Retry from scratch in other tables order : ARP , {FDB , LPM , EM} \n";
            cpssOsPrintf(debugString);
        }
        rc = hawk_sharedTablesAllResourceSet_byOrder(devNum,
            globalInfoPtr,
            fdbSize,emSize,lpmSize,arpSize,
            0/*fdb order*/,2/*em order*/,1/*lpm order*/,arpTableOrder/*arp order*/,
            infoArr,clientTypeArr);
        if(rc == GT_OK && debugString)
        {
            cpssOsPrintf("PASSED : %s",debugString);
        }
    }

    if(rc != GT_OK)
    {
        if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
        {
            debugString = "(case5) Retry from scratch in other tables order : ARP , {LPM , FDB , EM} \n";
            cpssOsPrintf(debugString);
        }
        rc = hawk_sharedTablesAllResourceSet_byOrder(devNum,
            globalInfoPtr,
            fdbSize,emSize,lpmSize,arpSize,
            1/*fdb order*/,2/*em order*/,0/*lpm order*/,arpTableOrder/*arp order*/,
            infoArr,clientTypeArr);
        if(rc == GT_OK && debugString)
        {
            cpssOsPrintf("PASSED : %s",debugString);
        }
    }

    if(rc != GT_OK)
    {
        if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
        {
            debugString = "(case6) Retry from scratch in other tables order : ARP , {FDB , EM} , LPM  \n";
            cpssOsPrintf(debugString);
        }
        rc = hawk_sharedTablesAllResourceSet_byOrder(devNum,
            globalInfoPtr,
            fdbSize,emSize,lpmSize,arpSize,
            0/*fdb order*/,1/*em order*/,2 | FORCE_LAST_TABLE/*lpm order*/,arpTableOrder/*arp order*/,
            infoArr,clientTypeArr);
        if(rc == GT_OK && debugString)
        {
            cpssOsPrintf("PASSED : %s",debugString);
        }
    }

    return rc;
}



/**
* @internal sharedTablesParamsCheck_lpm function
* @endinternal
*
* @brief   function to check if the LPM table sizes exceed the HW capabilities (according to device matrix)
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum         - the device number
*            lpmSize        - the number of LPM entries (needed from the shared tables)
*
* @param[out]
*           newLpmPrefixNumPtr - (pointer to) the number of LPM prefixes need (non-shared + shared)
*
* @retval   - GT_OK - the parameter not oversize the HW capabilities (according to device matrix)
*             GT_BAD_PARAM - the parameter oversize the HW capabilities (according to device matrix)
*/
static GT_STATUS sharedTablesParamsCheck_lpm(
    IN  GT_U8   devNum,
    IN  GT_U32  lpmSize,
    OUT GT_U32  *newLpmPrefixNumPtr
)
{
    GT_U32  lpmTotalNeededLines;
    GT_U32  lpm_numOfSbm;
    GT_U32  lpm_numClients_inSbm,lpm_numOfBlocksNonShared;

    lpm_numOfSbm = lpmSize / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    if(lpm_numOfSbm < ((FALCON_MAX_SBM_LPM/2) + 1))
    {
        /* we need double clients in the SBMs (and rest in non-shared) */
        lpm_numClients_inSbm = lpm_numOfSbm * 2;
    }
    else
    {
        lpm_numClients_inSbm = lpm_numOfSbm;
    }

    CHECK_PARAM_TO_MAX_VALUE(lpm_numClients_inSbm,PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared);

    /* fill the banks with memory from the non shared memory */
    lpm_numOfBlocksNonShared = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared - lpm_numClients_inSbm;
    if(lpm_numOfBlocksNonShared > PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared)
    {
        lpm_numOfBlocksNonShared = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared;
    }

    lpmTotalNeededLines = lpm_numOfSbm * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock +
        lpm_numOfBlocksNonShared *
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared;

    *newLpmPrefixNumPtr =
        PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(lpmTotalNeededLines);

    return GT_OK;
}

/**
* @internal sharedTablesParamsCheck function
* @endinternal
*
* @brief   function to check if any of the table sizes exceed the HW capabilities (according to device matrix)
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum         - the device number
*            fdbSize        - the number of FDB entries (needed from the shared tables)
*            emSize         - the number of EM  entries (needed from the shared tables)
*            lpmSize        - the number of LPM entries (needed from the shared tables)
*            arpSize        - the number of ARP entries (needed from the shared tables)
*
* @param[out]
*           newLpmPrefixNumPtr - (pointer to) the number of LPM prefixes need (non-shared + shared)
*
* @retval   - GT_OK - the parameters not oversize the HW capabilities (according to device matrix)
*             GT_BAD_PARAM - one of the parameters oversize the HW capabilities (according to device matrix)
*/
static GT_STATUS sharedTablesParamsCheck(
    IN  GT_U8   devNum,
    IN  GT_U32  fdbSize,
    IN  GT_U32  emSize,
    IN  GT_U32  lpmSize,
    IN  GT_U32  arpSize,
    OUT GT_U32  *newLpmPrefixNumPtr
)
{
    GT_STATUS rc;

    /* check the LPM parameter */
    rc = sharedTablesParamsCheck_lpm(devNum,lpmSize,newLpmPrefixNumPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check the FDB parameter */
    CHECK_PARAM_TO_MAX_VALUE(fdbSize,
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb);

    /* check the EM  parameter */
    CHECK_PARAM_TO_MAX_VALUE(emSize,
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum);

    /* check the ARP parameter */
    CHECK_PARAM_TO_MAX_VALUE(arpSize,
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp);

    return GT_OK;
}

/**
* @internal sharedTablesNewParamsSet function
* @endinternal
*
* @brief   function to set the new table sizes as new HW capabilities (into fine tuning)
*          the function also update the EM,FDB size in the HW (in EM,FDB unit)
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum         - the device number
*            fdbSize        - the number of FDB entries
*            emSize         - the number of EM  entries
*            lpmPrefixNum   - the number of LPM prefixes
*            arpSize        - the number of ARP entries
*
* @param[out]
*       None
*
* @retval GT_OK         - on success
* @retval GT_BAD_PARAM  - on bad device or FDB table size
*
*/
static GT_STATUS sharedTablesNewParamsSet(
    IN  GT_U8   devNum,
    IN  GT_U32  fdbSize,
    IN  GT_U32  emSize,
    IN  GT_U32  lpmPrefixNum,
    IN  GT_U32  arpSize
)
{
    GT_STATUS   rc;
    GT_U32    arpToTsRatio = 0;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart)
    {
        arpToTsRatio = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp /
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart;
    }

    /* update the <fdb> (involve updating other SW parameters and update the HW)*/
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb     */
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size    */
    rc = prvCpssDxChBrgFdbSizeSet(devNum,fdbSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm)
    {
        /* update the <emSize> (involve updating other SW parameters and update the HW)*/
        /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum      */
        rc = prvCpssDxChCfgEmSizeSet(devNum,emSize);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* update the <lpmRam> */
    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam    = lpmPrefixNum;
    /* update the <routerArp> */
    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp = arpSize;
    /* update the <tunnelStart> */
    if(arpToTsRatio != 0)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart =  arpSize  / arpToTsRatio;
    }

    return GT_OK;
}

/* macro to allow easy print of parameter in HEX format */
#define PRINT_PARAM_MAC(param)  cpssOsPrintf("%s = [0x%x] \n", #param , param)

/**
* @internal falcon_sharedTablesConnectTablesToMemory_rawSbmSize function
* @endinternal
*
* @brief   function to fill into the 'shared tables' HW the needed configurations,
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum           - the device number
*            fdbNumSmb        - the number of FDB SBMs needed (needed from the shared tables)
*            emNumSmb         - the number of EM  SBMs needed (needed from the shared tables)
*            lpmNumSmb        - the number of LPM SBMs needed (needed from the shared tables)
*            arpNumSmb        - the number of ARP SBMs needed (needed from the shared tables)
*            isDebugMode      - indication for debug printings
*
* @param[out]
*            None.
*
* @retval   - GT_OK - added the clients properly into infoArr[], clientTypeArr[][]
*             GT_NO_RESOURCE - could not add the clients.
*/
static GT_STATUS falcon_sharedTablesConnectTablesToMemory_rawSbmSize(
    IN  GT_U8   devNum,
    IN  GT_U32  fdbNumSmb,
    IN  GT_U32  emNumSmb,
    IN  GT_U32  lpmNumSmb,
    IN  GT_U32  arpNumSmb,
    IN  GT_BOOL isDebugMode
)
{
    GT_STATUS   rc;
    GT_U32      regAddr,value;
    GT_U32      n;
    GT_U32      infoArr[MAX_SBM_NUM];
    SBM_CLIENT_TYPE_ENT clientTypeArr[MAX_SBM_NUM][FALCON_PART_NUM];
    GT_U32      fdbSize,emSize,lpmSize,arpSize;/* hw number of lines/entries needed in the shared tables */
    GT_U32      lpmPrefixNum;/* lpm number of prefixes needed for the LPM (will be saved into fine tuning) */
    GT_U32      reg_LPM_size;/*  0x1 = SMALL; SMALL; LPM size is small, each LPM bank is connected to small dedicate memory for LPM; LPM size is small, each LPM bank is connected to small dedicate memory for LPM
                             0x2 = MEDIUM; MEDIUM; LPM size is medium, each LPM bank is connected to half SBM; LPM size is medium, each LPM bank is connected to half SBM
                             0x4 = BIG; BIG; LPM size is big, each LPM bank is connected to full SBM; LPM size is big, each LPM bank is connected to full SBM
                         */
    GT_U32  reg_FDB_size;/*0x1 = SingleSBM; SingleSBM; FDB size for each MHT is 1 SBM; FDB size for each MHT is 1 SBM
                           0x2 = TwoSBM; TwoSBM; FDB size for each MHT is 2 SBMs; FDB size for each MHT is 2 SBMs
                         */
    GT_U32  reg_EM_size;/*0x1 = NoSBM; NoSBM; EM size for each MHT is 0 SBM; EM size for each MHT is 0 SBM
                          0x2 = SingleSBM; SingleSBM; EM size for each MHT is 1 SBM; EM size for each MHT is 1 SBM
                        */
    GT_U32  reg_ARP_size;/*0x1 = FourSBM; FourSBM; ARP size is 4 SBMs; ARP size is 4 SBMs
                           0x4 = EightSBM; EightSBM; ARP size is 8 SBMs; ARP size is 8 SBMs
                         */
    GT_U32  PBR_Bank_Size;/*0x0 = 4K; 4K; offset addr = {2'd0,iu_lpm_ipvx2lpm_req_next_bucket[17:15],3'b0,iu_lpm_ipvx2lpm_req_next_bucket[14:3]}
                            0x1 = 8K; 8K; offset addr = {3'd0,iu_lpm_ipvx2lpm_req_next_bucket[17:16],2'b0,iu_lpm_ipvx2lpm_req_next_bucket[15:3]}
                            0x2 = 16K; 16K; offset addr ={4'd0,iu_lpm_ipvx2lpm_req_next_bucket[17],1'b0,iu_lpm_ipvx2lpm_req_next_bucket[16:3]}
                            0x3 = 32K; 32K; offset addr ={5'd0,iu_lpm_ipvx2lpm_req_next_bucket[17:3] */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
    GT_U32  maxSbmNum;
    GT_U32  didError = 0;

    GT_U32   shm_engines_size_mode_value;
    GT_U32   numOfSwFdbEmEntriesPerBank;
    GT_U32   numOfSwLpmLinesInBlock;
    GT_U32   numOfSwArpLinesInBlock;
    CPSS_PP_DEVICE_TYPE devType;
    GT_U32   fdbSwSize,emSwSize,lpmSwSize,arpSwSize; /* SW number of lines/entries needed in the shared tables */

    numOfSwFdbEmEntriesPerBank = FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
    numOfSwLpmLinesInBlock = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;
    numOfSwArpLinesInBlock = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;

    devType = PRV_CPSS_PP_MAC(devNum)->devType;
    if (devType == CPSS_98DX4504_CNS ||
        devType == CPSS_98DX4504M_CNS)
    {
        numOfSwFdbEmEntriesPerBank = _2K;
        numOfSwArpLinesInBlock = _4K;
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) &&
       CONFIG_GLOBAL_VAR_GET(hawkNumberOfClientsArr_initDone) == GT_FALSE)
    {
        rc = hawkNumberOfClientsArrInit(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(fdbNumSmb > 16)
    {
        reg_FDB_size =  0x2; /*TwoSBM*/
    }
    else
    {
        reg_FDB_size =  0x1; /*SingleSBM*/
    }

    if(emNumSmb == 0)
    {
        reg_EM_size =  0x1; /*NoSBM*/
    }
    else
    {
        reg_EM_size =  0x2; /*SingleSBM*/
    }

    if(lpmNumSmb >= 14)
    {
        reg_LPM_size = 0x4  ;/*BIG*/
        PBR_Bank_Size= 0x2  ;/*16K*/
        moduleCfgPtr->ip.lpmSharedMemoryBankNumber = lpmNumSmb;
        moduleCfgPtr->ip.lpmBankSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;
    }
    else
    if(lpmNumSmb > 0)
    {
        reg_LPM_size = 0x2  ;/*MEDIUM*/
        PBR_Bank_Size= 0x1  ;/* 8K*/
        moduleCfgPtr->ip.lpmSharedMemoryBankNumber = lpmNumSmb*2;
        moduleCfgPtr->ip.lpmBankSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock / 2;
    }
    else/*0*/
    {
        reg_LPM_size = 0x1  ;/*SMALL*/
        PBR_Bank_Size= 0x0  ;/* 4K*/
        moduleCfgPtr->ip.lpmSharedMemoryBankNumber = 0;
        moduleCfgPtr->ip.lpmBankSize = 0;
    }

    if(arpNumSmb == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"ARP table 0 SBMs is not supported");
    }
    else
    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))/*Falcon*/
    {
        if(arpNumSmb > 4)
        {
            reg_ARP_size = 0x4  ;/*EightSBM*/
        }
        else
        {
            reg_ARP_size = 0x1  ;/*FourSBM*/
        }
    }
    else /* Hawk */
    {
        switch(arpNumSmb)
        {
            case  4: reg_ARP_size =  0x1; break;
            case  8: reg_ARP_size =  0x2; break;
            case 12: reg_ARP_size =  0x4; break;
            case 16: reg_ARP_size =  0x8; break;
            case 20: reg_ARP_size = 0x10; break;
            case 24: reg_ARP_size = 0x20; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(arpNumSmb);
        }
    }

    maxSbmNum = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSbmInSuperSbm *
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;

    /* check that the sum of SBMs needed is not more than exists */
    CHECK_PARAM_TO_MAX_VALUE((fdbNumSmb + emNumSmb + lpmNumSmb + arpNumSmb),maxSbmNum);

    fdbSize = fdbNumSmb  * FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
    emSize  = emNumSmb   * FALCON_EM_AND_FDB_ENTRIES_TO_SBM_FACTOR;
    lpmSize = lpmNumSmb  * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;
    arpSize = arpNumSmb  * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;

    /* Set SW table sizes */
    fdbSwSize = fdbNumSmb  * numOfSwFdbEmEntriesPerBank;
    emSwSize  = emNumSmb   * numOfSwFdbEmEntriesPerBank;
    lpmSwSize = lpmNumSmb  * numOfSwLpmLinesInBlock;
    arpSwSize = arpNumSmb  * numOfSwArpLinesInBlock;

    /* check the parameters with the device matrix */
    rc = sharedTablesParamsCheck(devNum,fdbSwSize,emSwSize,lpmSwSize,arpSwSize,&lpmPrefixNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(infoArr,0,sizeof(infoArr));
    cpssOsMemSet(clientTypeArr,0,sizeof(clientTypeArr));

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* fill the array of infoArr[] that need to write to the registers : */
        rc = hawk_sharedTablesAllResourceSet(devNum,fdbSize,emSize,lpmSize,arpSize,
                infoArr,clientTypeArr);
    }
    else
    {
        /* fill the array of infoArr[] that need to write to the registers : */
        rc = falcon_sharedTablesAllResourceSet(devNum,fdbSize,emSize,lpmSize,arpSize,
                infoArr,clientTypeArr);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    if(isDebugMode == GT_TRUE)
    {
        /* in debug mode ... do not update the fine tuning */
        PRINT_PARAM_MAC(lpmPrefixNum);
    }
    else
    {
        /* set the new parameters into the 'fine tuning' (and FDB HW) */
        /* the function also update the EM,FDB size in the HW (in EM,FDB unit) */
        rc = sharedTablesNewParamsSet(devNum,fdbSwSize,emSwSize,lpmPrefixNum,arpSwSize);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    shm_engines_size_mode_value = (reg_LPM_size <<  0) |
                                  (reg_FDB_size <<  4) |
                                  (reg_EM_size  <<  8) |
                                  (reg_ARP_size << 12) ;


    regAddr = regsAddrPtr->SHM.shm_engines_size_mode;
    value   = shm_engines_size_mode_value;

    if(isDebugMode == GT_TRUE)
    {
        /* we are NOT writing into the registers ... only calculating them */
        PRINT_PARAM_MAC(reg_LPM_size);
        PRINT_PARAM_MAC(reg_FDB_size);
        PRINT_PARAM_MAC(reg_EM_size );
        PRINT_PARAM_MAC(reg_ARP_size);
        PRINT_PARAM_MAC(shm_engines_size_mode_value);
        PRINT_PARAM_MAC(PBR_Bank_Size);
    }
    else
    {
        if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))/*Falcon*/
        {
            rc = prvCpssHwPpSetRegField(devNum,regAddr,0,15,value);
        }
        else /* Hawk : 18 bits instead of 15 */
        {
            rc = prvCpssHwPpSetRegField(devNum,regAddr,0,18,value);
        }

        if (rc != GT_OK)
        {
            return rc;
        }

        {
            for(n = 0 ; n < maxSbmNum ; n++) {
                if(getRepresentativeClient(clientTypeArr[n][0]) == SBM_CLIENT_TYPE__LAST__E)
                {
                    /* the SBM is not in use */
                    CPSS_LOG_ERROR_MAC("SMB[%d] : is not used",
                        n);
                    didError++;
                }
            }

            if(didError)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"got [%d] SMBs (out of[%d]) that are not bound to any client \n",
                    didError,maxSbmNum);
            }
        }
    }

    if(isDebugMode == GT_TRUE)
    {
        SBM_CLIENT_TYPE_ENT client,client2,representativeClient;
        char const*   clientName;

        cpssOsPrintf("SBMs request of :  \n");
        PRINT_PARAM_MAC(fdbNumSmb);
        PRINT_PARAM_MAC(emNumSmb);
        PRINT_PARAM_MAC(lpmNumSmb);
        PRINT_PARAM_MAC(arpNumSmb);

        cpssOsPrintf("\n start list all clients in SBMs \n");

        for(n = 0 ; n < maxSbmNum ; n++) {
            client = clientTypeArr[n][0];

            representativeClient = getRepresentativeClient(client);

            if(representativeClient == SBM_CLIENT_TYPE__LAST__E)
            {
                /* the SBM is not in use */
                cpssOsPrintf("SMB[%d] : is not used \n",
                    n);

                didError++;
                continue;
            }


            clientName = getRepresentativeClientName(client);

            client2 = clientTypeArr[n][1];

            if(representativeClient == SBM_CLIENT_TYPE_LPM_0_E &&
               (client2 != 0)/* valid 2 clients */)
            {
                cpssOsPrintf("SMB[%d] : [%s + %d , %d] \n",
                    n ,
                    clientName,
                    client  - representativeClient,
                    client2 - representativeClient);
            }
            else
            if(representativeClient != SBM_CLIENT_TYPE_ARP_E)
            {
                cpssOsPrintf("SMB[%d] : [%s + %d] \n",
                    n ,
                    clientName,
                    client - representativeClient);
            }
            else /* ARP */
            {
                cpssOsPrintf("SMB[%d] : [%s] \n",
                    n ,
                    clientName);
            }
        }

        cpssOsPrintf(" Ended list of clients in SBMs \n");
    }

    for(n = 0 ; n < maxSbmNum ; n++) {
        regAddr = regsAddrPtr->SHM.block_port_select[n];
        value   = infoArr[n];

        if(isDebugMode == GT_TRUE)
        {
            /* we are NOT writing into the registers ... only calculating them */
            cpssOsPrintf("register[%d] with value [0x%x] \n",
                n,value);
        }
        else
        {
            rc = prvCpssHwPpWriteRegister(devNum,regAddr,value);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }/* end of loop n */


    /************************************/
    /* set the LPM with proper PBR mode */
    /************************************/
    if(isDebugMode == GT_TRUE)
    {
        cpssOsPrintf("LPM : <PBR Bank Size> = [%d] \n",
            PBR_Bank_Size);
    }
    else
    {
        rc = prvCpssDxChLpmPbrBankSizeSet(devNum,PBR_Bank_Size);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /******************************************/
    /* set the FDB and EM to proper MHT value */
    /******************************************/
    /* value is the MHT : 4/8/16 */
    value = fdbNumSmb <=  4 ?  4 :
            fdbNumSmb >= 16 ? 16 :
            8;

    if(isDebugMode == GT_TRUE)
    {
        cpssOsPrintf("FDB : <Number of Multiple Hash Tables> = [%d] \n",
            value);
    }
    else
    {
        /* will update HW and DB : PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank      */
        rc = prvCpssDxChBrgFdbMhtSet(devNum,value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* value is the MHT : 4/8/16 */
    value = emNumSmb <=  4 ?  4 :
            emNumSmb >= 16 ? 16 :
            8;

    if(isDebugMode == GT_TRUE)
    {
        cpssOsPrintf("EM  : <Number of Multiple Hash Tables> = [%d] \n",
            value);
    }
    else
    {
        /* will update HW and DB : PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks  */
        rc = prvCpssDxChCfgEmMhtSet(devNum,value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(isDebugMode == GT_TRUE)
    {
        /* we are NOT writing into the registers ... only calculating them */
        cpssOsPrintf("\n\n");

        if(didError)
        {
            /* */
            cpssOsPrintf(" Error was recognized \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }


    return GT_OK;
}

/**
* @internal falcon_sharedTablesConnectTablesToMemory function
* @endinternal
*
* @brief   function to fill into the 'shared tables' HW the needed configurations,
*           according to simple 'enum' of predefined cases.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum           - the device number
*            sharedTableMode  - the simple predefined cases of 'shared tables'
*
* @param[out]
*            None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
*
*/
static GT_STATUS falcon_sharedTablesConnectTablesToMemory(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT sharedTableMode
)
{
    GT_STATUS   rc;
    GT_U32  fdbNumSmb,emNumSmb,lpmNumSmb,arpNumSmb;/* number of SBMs each client need */
    CPSS_PP_DEVICE_TYPE     devType;

    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) /* Falcon */
    {
        switch(sharedTableMode)
        {
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                fdbNumSmb = 4;  /* 32K*/
                emNumSmb  = 0;
                lpmNumSmb = 28; /*392K*/
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                fdbNumSmb = 32; /*256K*/
                emNumSmb  = 0;
                lpmNumSmb = 0;
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                fdbNumSmb = 16; /*128K*/
                emNumSmb  = 4 ; /* 32K*/
                lpmNumSmb = 12; /*168K*/
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                fdbNumSmb = 4;  /* 32K*/
                emNumSmb  = 16; /*128K*/
                lpmNumSmb = 12; /*168K*/
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                fdbNumSmb = 8;  /* 64K*/
                emNumSmb  = 16; /*128K*/
                lpmNumSmb = 8;  /* 90K*/
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E:
                fdbNumSmb = 16; /*128K*/
                emNumSmb  = 0; /* 32K*/
                lpmNumSmb = 16;
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E:
                fdbNumSmb = 16; /*128K*/
                emNumSmb  = 8; /* 64K*/
                lpmNumSmb = 8;
                arpNumSmb = 4;  /* 96K*/
                break;
            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E:
                fdbNumSmb = 8;  /*64K*/
                emNumSmb  = 8; /* 64K*/
                lpmNumSmb = 12;
                arpNumSmb = 8;  /* 192K*/
                break;

            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E:
                fdbNumSmb = 16;
                emNumSmb  = 16;
                lpmNumSmb = 0;
                arpNumSmb = 4;
                break;

            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E:
                fdbNumSmb = 8;
                emNumSmb  = 0;
                lpmNumSmb = 24;
                arpNumSmb = 4;
                break;

            case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E:
                    fdbNumSmb = 8;
                    emNumSmb  = 0;
                    lpmNumSmb = 20;
                    arpNumSmb = 8;
                    break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sharedTableMode);
        }
    }
    else    /* Hawk */
    {
        devType = PRV_CPSS_PP_MAC(devNum)->devType;

        if (devType == CPSS_98DX4504_CNS ||
            devType == CPSS_98DX4504M_CNS)
        {
            fdbNumSmb = 16; /* 32K the EM/FDB engine uses 2K entries per bank */
            emNumSmb  = 16; /* 32K the EM/FDB engine uses 2K entries per bank */
            lpmNumSmb = 4;
            arpNumSmb = 4;  /* 96K */
        }
        else
        {
            switch(sharedTableMode)
            {
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                    fdbNumSmb = 4;  /* 32K*/
                    emNumSmb  = 0;
                    lpmNumSmb = 28; /*280K. Falcon : 392K*/
                    arpNumSmb = 8;  /*128K. Falcon :  92K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                    fdbNumSmb = 32; /*256K*/
                    emNumSmb  = 0;
                    lpmNumSmb = 0;
                    arpNumSmb = 8;  /*128K. Falcon :  92K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                    fdbNumSmb = 16; /*128K*/
                    emNumSmb  = 4 ; /* 32K*/
                    lpmNumSmb = 12; /*120K. Falcon : 168K*/
                    arpNumSmb = 8;  /*128K. Falcon :  92K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                    fdbNumSmb = 4;  /* 32K*/
                    emNumSmb  = 16; /*128K*/
                    lpmNumSmb = 12; /*120K. Falcon : 168K*/
                    arpNumSmb = 8;  /*128K. Falcon :  92K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                    fdbNumSmb = 8;  /* 64K*/
                    emNumSmb  = 16; /*128K*/
                    lpmNumSmb = 8;  /* 80K*/
                    arpNumSmb = 8;  /*128K. Falcon :  92K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E:
                    fdbNumSmb = 16; /*128K*/
                    emNumSmb  = 0;
                    lpmNumSmb = 20;
                    arpNumSmb = 4;  /* 96K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E:
                    fdbNumSmb = 16; /*128K*/
                    emNumSmb  = 8;  /* 64K*/
                    lpmNumSmb = 8;
                    arpNumSmb = 8;  /* 96K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E:
                    fdbNumSmb = 8;  /*64*/
                    emNumSmb  = 8;  /* 64K*/
                    lpmNumSmb = 12;
                    arpNumSmb = 12;  /* 192K*/
                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E:
                    fdbNumSmb = 16;
                    emNumSmb  = 16;
                    lpmNumSmb = 0;
                    arpNumSmb = 8;
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E:
                    fdbNumSmb = 8;
                    emNumSmb  = 0;
                    lpmNumSmb = 24;
                    arpNumSmb = 8;
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E:
                    fdbNumSmb = 8;
                    emNumSmb  = 0;
                    lpmNumSmb = 20;
                    arpNumSmb = 12;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sharedTableMode);
            }
        }
    }

    /* call the 'raw' function that gets info in 'SBMs' */
    rc = falcon_sharedTablesConnectTablesToMemory_rawSbmSize(devNum,
        fdbNumSmb,emNumSmb,lpmNumSmb,arpNumSmb,
        GT_FALSE/* not debug mode */);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* Get Iinfo about Shared Blocks mapping  */

/**
* @internal sharedTablesType2BlockPortSelectValuesToClientConvert function
* @endinternal
*
* @brief   function converts port select valueus of SMB Type2 block to clients.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum           - the device number
*            smbBlockIndex    - index of SMB block
*            smbPort0Select   - port0-select field of Port Select register
*            smbPort1Select   - port1-select field of Port Select register
*
* @param[out]
*            client0Ptr       - (pointer to) client0 enum value.
*            client1Ptr       - (pointer to) client1 enum value (relevant for LPM only).
*
* @retval GT_OK               - success.
* @retval GT_BAD_PARAM        - on wrong parameters.
* @retval GT_BAD_STATE        - port select values that cannot be converted.
*
*/
static GT_STATUS sharedTablesType2BlockPortSelectValuesToClientConvert(
    IN   GT_U8                           devNum,
    IN   GT_U32                          smbBlockIndex,
    IN   GT_U32                          smbPort0Select,
    IN   GT_U32                          smbPort1Select,
    OUT  SBM_CLIENT_TYPE_ENT             *client0Ptr,
    OUT  SBM_CLIENT_TYPE_ENT             *client1Ptr
)
{
    const SUPPER_SBM_INFO_STC*    currSupperSbmInfoPtr;
    const SUPPER_SBM_INFO_STC* const *    devSupperSbmInfoArr;
    GT_U32  numOfSuperSbm;
    GT_U32  sbmIndex;
    const SUPPER_SBM_INFO_STC*    supperSbmInfo0Ptr;
    const SUPPER_SBM_INFO_STC*    supperSbmInfo1Ptr;

    sbmIndex = (smbBlockIndex / FALCON_SBM_IN_SUPER_NUM);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        devSupperSbmInfoArr = Hawk_super_SBM_support;
        numOfSuperSbm = HAWK_SUPER_SBM_NUM;
    }
    else
    {
        devSupperSbmInfoArr = Falcon_super_SBM_support;
        numOfSuperSbm = FALCON_SUPER_SBM_NUM;
    }

    if (sbmIndex >= numOfSuperSbm)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* first attempt - pattern as is */
    currSupperSbmInfoPtr = devSupperSbmInfoArr[sbmIndex];
    while (currSupperSbmInfoPtr->client != SBM_CLIENT_TYPE__LAST__E)
    {
        if ((currSupperSbmInfoPtr->regValue_0 == smbPort0Select)
            && (currSupperSbmInfoPtr->regValue_1 == smbPort1Select))
        {
            *client0Ptr = currSupperSbmInfoPtr->client;
            *client1Ptr = UNUSED_HW_VALUE;
            return GT_OK;
        }
        currSupperSbmInfoPtr ++;
    }

    /* second attempt (LPM only) - composed */
    supperSbmInfo0Ptr = (SUPPER_SBM_INFO_STC*)NULL;
    supperSbmInfo1Ptr = (SUPPER_SBM_INFO_STC*)NULL;
    currSupperSbmInfoPtr = devSupperSbmInfoArr[sbmIndex];
    while (currSupperSbmInfoPtr->client != SBM_CLIENT_TYPE__LAST__E)
    {
        if ((supperSbmInfo0Ptr == NULL)
            && (currSupperSbmInfoPtr->regValue_0 == smbPort0Select)
            && (currSupperSbmInfoPtr->regValue_1 == UNUSED_HW_VALUE))
        {
            supperSbmInfo0Ptr = currSupperSbmInfoPtr;
        }
        if ((supperSbmInfo1Ptr == NULL)
            && (currSupperSbmInfoPtr->regValue_0 == smbPort1Select)
            && (currSupperSbmInfoPtr->regValue_1 == UNUSED_HW_VALUE))
        {
            supperSbmInfo1Ptr = currSupperSbmInfoPtr;
        }
        if ((supperSbmInfo0Ptr != NULL) && (supperSbmInfo1Ptr != NULL))
        {
            /* in these cases the configuring function swaps values */
            /* here done the same                                   */
            *client0Ptr = supperSbmInfo1Ptr->client;
            *client1Ptr = supperSbmInfo0Ptr->client;
            return GT_OK;
        }
        currSupperSbmInfoPtr ++;
    }

    /* not found data in DB for needed conversion */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

/**
* @internal sharedTablesType2BlockClientsGet function
* @endinternal
*
* @brief   function gets clients of SMB Type2 block.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum           - the device number
*            smbBlockIndex    - index of SMB block
*
* @param[out]
*            client0Ptr       - (pointer to) client0 enum value.
*            client1Ptr       - (pointer to) client1 enum value (relevant for LPM only).
*
* @retval GT_OK               - success.
* @retval GT_BAD_PARAM        - on wrong parameters.
* @retval GT_BAD_STATE        - port select values that cannot be converted.
* @retval GT_HW_ERROR         - on hardware error.
*
*/
static GT_STATUS sharedTablesType2BlockClientsGet
(
    IN   GT_U8                           devNum,
    IN   GT_U32                          smbBlockIndex,
    OUT  SBM_CLIENT_TYPE_ENT             *client0Ptr,
    OUT  SBM_CLIENT_TYPE_ENT             *client1Ptr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numOfSuperSbm = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;
    GT_U32  sbmIndex;
    GT_U32  regAddr;
    GT_U32  data;
    GT_U32  smbPort0Select;
    GT_U32  smbPort1Select;

    sbmIndex = (smbBlockIndex / FALCON_SBM_IN_SUPER_NUM);
    if (sbmIndex >= numOfSuperSbm)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = regsAddrPtr->SHM.block_port_select[smbBlockIndex];
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &data);
    if (rc != GT_OK)
    {
        return rc;
    }

    smbPort0Select = (data & 0x1F);
    smbPort1Select = ((data >> 8) & 0x1F);

    return sharedTablesType2BlockPortSelectValuesToClientConvert(
        devNum, smbBlockIndex, smbPort0Select, smbPort1Select,
        client0Ptr, client1Ptr);
}

/**
* @internal prvCpssDxChCfgSharedTablesArpSmbIndexGet function
* @endinternal
*
* @brief   function returns serial number of ARP SMB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X, Bobcat3; Aldrin2.
*
* @param[in]
*            devNum           - the device number
*            smbNum           - number of ARP's SMB to calculate serial number
*
* @retval serial number of SMB between all ARP SMBs with number less than smbNum.
*         0xFFFFFFFF - in case of wrong input parameters
*
*/
GT_U32 prvCpssDxChCfgSharedTablesArpSmbIndexGet
(
    IN   GT_U8      devNum,
    IN   GT_U32     smbNum
)
{
    GT_STATUS           rc;
    GT_U32              smbBlock;  /* iterator of SMB blocks */
    GT_U32              index = 0; /* serial number of ARP's SMB block */
    SBM_CLIENT_TYPE_ENT client0;   /* client connected to port 0 */
    SBM_CLIENT_TYPE_ENT client1;   /* client connected to port 1 */

    for (smbBlock = 0; smbBlock < smbNum; smbBlock++)
    {
        rc = sharedTablesType2BlockClientsGet(devNum, smbBlock, &client0, &client1);
        if (rc != GT_OK)
        {
            return 0xFFFFFFFF;
        }

        /* ARP use both ports, check port 0 */
        if (client0 == SBM_CLIENT_TYPE_ARP_E)
        {
            index++;
        }
    }
    return index;
}

/**
* @internal prvCpssDxChCfgSharedTablesType2BlockTableIdGet function
* @endinternal
*
* @brief   function gets Id of table using the given SMB Type2 block.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X, Bobcat3; Aldrin2.
*
* @param[in]
*            devNum           - the device number
*            smbBlockIndex    - index of SMB block
*            smbBlockPort     - port of SMB block - 0 or 1
*
* @param[out]
*            tableIdPtr       - (pointer to) Id of the table using the given SMB Type2 block.
*            clientOffsetPtr  - (pointer to) Id of the client's bank using the given SMB Type2 block.
*
* @retval GT_OK               - success.
* @retval GT_BAD_PARAM        - on wrong parameters.
* @retval GT_BAD_STATE        - port select values that cannot be converted.
* @retval GT_HW_ERROR         - on hardware error.
*
*/
GT_STATUS prvCpssDxChCfgSharedTablesType2BlockTableIdGet
(
    IN   GT_U8                           devNum,
    IN   GT_U32                          smbBlockIndex,
    IN   GT_U32                          smbBlockPort,
    OUT  CPSS_DXCH_TABLE_ENT             *tableIdPtr,
    OUT  GT_U32                          *clientOffsetPtr
)
{
    GT_STATUS rc;
    SBM_CLIENT_TYPE_ENT  client0;
    SBM_CLIENT_TYPE_ENT  client1;
    SBM_CLIENT_TYPE_ENT  client;
    GT_U32  numOfLinesInBlock;
    GT_U32  pbrStartLpmLine;
    GT_U32  pbrNumOfLpmLines;
    GT_U32  pbrStartLpmClient;
    GT_U32  pbrNumOfLpmClients;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;
    GT_U32  totalNumOfLinesInBlockIncludingGap;

    rc = sharedTablesType2BlockClientsGet(
        devNum, smbBlockIndex, &client0, &client1);
    if (rc != GT_OK)
    {
        return rc;
    }
    client = (smbBlockPort == 0) ? client0 : client1;
    if (client == UNUSED_HW_VALUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    if ((client >= SBM_CLIENT_TYPE_LPM_0_E) && (client <= SBM_CLIENT_TYPE_LPM_27_E))
    {
        moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);

        numOfLinesInBlock = moduleCfgPtr->ip.lpmBankSize;

        /* amount of PBR clients */
        pbrNumOfLpmLines = moduleCfgPtr->ip.numberOfPbrLines;

        pbrNumOfLpmClients =
            (pbrNumOfLpmLines + numOfLinesInBlock - 1)
            / (numOfLinesInBlock);

        /* start PBR client */
        totalNumOfLinesInBlockIncludingGap =
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;

        pbrStartLpmLine = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;

        pbrStartLpmClient  = SBM_CLIENT_TYPE_LPM_0_E +
            ((pbrStartLpmLine + totalNumOfLinesInBlockIncludingGap - 1)
            / totalNumOfLinesInBlockIncludingGap);

        /* PBR table is a part of LPM table, the same indexes used for access */
        /* Line indexes relevant for PBR-lines-range invalid for LPM table    */
        /* Line indexes relevant for LPM-lines-range invalid for PBR table    */
        /* LPM table has 1 entry per line, PBR table has 5 entries per line   */
        /* Client offset is common                                            */
        *clientOffsetPtr = client - SBM_CLIENT_TYPE_LPM_0_E;
        if (((GT_U32)client >= pbrStartLpmClient)
            && ((GT_U32)client < (pbrStartLpmClient + pbrNumOfLpmClients)))
        {
            *tableIdPtr = CPSS_DXCH_SIP6_TABLE_PBR_E;
        }
        else
        {
            *tableIdPtr = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
        }
        return GT_OK;
    }
    if ((client >= SBM_CLIENT_TYPE_FDB_0_E) && (client <= SBM_CLIENT_TYPE_FDB_15_E))
    {
        *tableIdPtr = CPSS_DXCH_TABLE_FDB_E;
        *clientOffsetPtr = client - SBM_CLIENT_TYPE_FDB_0_E;
        return GT_OK;
    }
    if ((client >= SBM_CLIENT_TYPE_EM_0_E) && (client <= SBM_CLIENT_TYPE_EM_15_E))
    {
        *tableIdPtr = CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E;
        *clientOffsetPtr = client - SBM_CLIENT_TYPE_EM_0_E;
        return GT_OK;
    }
    if (client == SBM_CLIENT_TYPE_ARP_E)
    {
        *tableIdPtr = CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E;
        *clientOffsetPtr = prvCpssDxChCfgSharedTablesArpSmbIndexGet(devNum, smbBlockIndex);
        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG); /* should never occur */
}

/**
* @internal dump_prvCpssDxChCfgSharedTablesType2BlockTableIds function
* @endinternal
*
* @brief   function prints Id of table using the all SMB Type2 blocks.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X, Bobcat3; Aldrin2.
*
* @param[in]
*            devNum           - the device number
*
* @param[out]
*
* @retval none.
*
*/
void dump_prvCpssDxChCfgSharedTablesType2BlockTableIds
(
    IN   GT_U8                           devNum
)
{
    GT_STATUS           rc;
    GT_U32              smbBlock;
    GT_U32              numOfSuperSbm = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfSuperSbm;
    GT_U32              numOfSmbBlocks = numOfSuperSbm * FALCON_SBM_IN_SUPER_NUM;
    GT_U32              smbBlockPort;
    CPSS_DXCH_TABLE_ENT tableId;
    GT_U32              clientOffset;

    for (smbBlock = 0; (smbBlock < numOfSmbBlocks); smbBlock++)
    {
        cpssOsPrintf("block[%02d] ", smbBlock);
        for (smbBlockPort = 0; (smbBlockPort < 2); smbBlockPort++)
        {
            rc = prvCpssDxChCfgSharedTablesType2BlockTableIdGet(
                devNum, smbBlock, smbBlockPort,
                &tableId, &clientOffset);
            if (rc != GT_OK) continue;
            switch (tableId)
            {
            case CPSS_DXCH_SIP6_TABLE_PBR_E:
                cpssOsPrintf("PBR");
                break;
            case CPSS_DXCH_TABLE_FDB_E:
                cpssOsPrintf("FDB");
                break;
            case CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E:
                cpssOsPrintf("ARP");
                break;
            case CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E:
                cpssOsPrintf("EXACT_MATCH");
                break;
            case CPSS_DXCH_SIP5_TABLE_LPM_MEM_E:
                cpssOsPrintf("LPM");
                break;
            default:
                break;
            }
            cpssOsPrintf("[%d] ", clientOffset);
        }
        cpssOsPrintf("\n");
    }
}

/**
* @internal sharedTablesParamsGet function
* @endinternal
*
* @brief   function to get the table sizes of HW capabilities (from fine tuning)
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum         - the device number
*
* @param[out]
*            fdbSize        - the number of FDB entries
*            emSize         - the number of EM  entries
*            lpmPrefixNum   - the number of LPM prefixes
*            arpSize        - the number of ARP entries
*
* @retval   - GT_OK - always
*
*/
static GT_STATUS sharedTablesParamsGet(
    IN  GT_U8   devNum,
    OUT  GT_U32  *fdbSizePtr,
    OUT  GT_U32  *emSizePtr,
    OUT  GT_U32  *lpmPrefixNumPtr,
    OUT  GT_U32  *arpSizePtr
)
{
    /* get the <fdb> */
    *fdbSizePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb;
    /* get the <emNum> */
    *emSizePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum;
    /* get the <lpmRam> */
    *lpmPrefixNumPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam;
    /* get the <routerArp> */
    *arpSizePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp;

    return GT_OK;
}

/* max lines in sip6 : Falcon/AC5P */
#define SIP6_MAX_LPM_CNS(_devNum) \
        ((28 * PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.lpm.numOfLinesInBlock)+ \
         (2  * PRV_CPSS_DXCH_PP_MAC(_devNum)->hwInfo.lpm.sip6_numLinesPerBlockNonShared))

/********************/
/* structure for test cases of number of SBMs needed per client */
/********************/
typedef struct
{
    /* FDB : the number of needed SBMs */
    GT_U32  fdb_num_sbm;
    /* EM  : the number of needed SBMs */
    GT_U32  em_num_sbm;
    /* ARP : the number of needed SBMs */
    GT_U32  arp_num_sbm;
    /* LPM : the number of needed SBMs */
    GT_U32  lpm_num_sbm;

}TEST_SMBS_FOR_CLIENTS_STC;

static const TEST_SMBS_FOR_CLIENTS_STC falcon_sharedMemoryModes_validCasesArr[] = {
/*  Falcon : valid cases
    values are number of SBMs per client
                    FDB        EM        ARP        LPM        total    */

/* test case 1 : */ {32      ,  0     ,   4     ,   0   /* ,    36  */ } /*CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E*/

/* test case 2 : */,{16      ,  16    ,    4    ,    0  /* ,     36 */   }

/* test case 3 : */,{16      ,  8     ,   8     ,   4   /* ,    36  */   }
/* test case 4 : */,{16      ,  8     ,   4     ,   8   /* ,    36  */   }

/* test case 5 : */,{16      ,  4     ,   8     ,   8   /* ,    36  */   }
/* test case 6 : */,{16      ,  4     ,   4     ,   12  /* ,     36 */   } /*CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E*/

/* test case 7 : */,{16      ,  0     ,   8     ,   12  /* ,     36 */   }
/* test case 8 : */,{16      ,  0     ,   4     ,   16  /* ,     36 */   }

/* test case 9 : */,{8       , 16     ,   8     ,   4   /* ,    36  */   }
/* test case 10: */,{8       , 16     ,   4     ,   8   /* ,    36  */   }

/* test case 11: */,{8       , 8      ,  8      ,  12   /* ,    36  */   }
/* test case 12: */,{8       , 8      ,  4      ,  16   /* ,    36  */   }

/* test case 13: */,{4       , 16     ,   8     ,   8   /* ,    36  */   }
/* test case 14: */,{4       , 16     ,   4     ,   12  /* ,     36 */   } /*CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E*/

/* test case 15: */,{4       , 8      ,  8      ,  16   /* ,    36  */   }
/* test case 16: */,{4       , 8      ,  4      ,  20   /* ,    36  */   }

/* test case 17: */,{4       , 4      ,  8      ,  20   /* ,    36  */   }
/* test case 18: */,{4       , 4      ,  4      ,  24   /* ,    36  */   }

/* test case 19: */,{4       , 0      ,  8      ,  24   /* ,    36  */   }
/* test case 20: */,{4       , 0      ,  4      ,  28   /* ,    36  */   } /*CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E*/

    /* must be last */
    ,{GT_NA   ,GT_NA   , GT_NA   , GT_NA}
};/*end of validCasesArr[] */
static const TEST_SMBS_FOR_CLIENTS_STC hawk_sharedMemoryModes_validCasesArr[] = {
/*  Falcon : valid cases
    values are number of SBMs per client
                     FDB        EM     ARP        LPM                   total  */
/* test case 1   */  {32      ,  0     ,4         , 4    /    1     /*   40    */  }
/* test case 2   */ ,{32      ,  0     ,4         , 8    /    2     /*   40    */  }
/* test case 3   */ ,{32      ,  4     ,4         , 0    /    1     /*   40    */  }
/* test case 4   */ ,{16      ,  0     ,4         , 20   /     1    /*    40   */  }
/* test case 5   */ ,{16      ,  4     ,4         , 16   /     1    /*    40   */  }
/* test case 6   */ ,{16      ,  8     ,4         , 12   /     1    /*    40   */  }
/* test case 7   */ ,{16      ,  8     ,4         , 24   /     2    /*    40   */  }
/* test case 8   */ ,{16      ,  16    ,4         ,  4   /     1    /*    40   */  }
/* test case 9   */ ,{16      ,  16    ,4         ,  8   /     2    /*    40   */  }
/* test case 10  */ ,{8       ,  0     ,4         , 28   /     1    /*    40   */  }
/* test case 11  */ ,{8       ,  4     ,4         , 24   /     1    /*    40   */  }
/* test case 12  */ ,{8       ,  8     ,4         , 20   /     1    /*    40   */  }
/* test case 13  */ ,{8       ,  16    ,4         ,  12  /      1   /*     40  */  }
/* test case 14  */ ,{8       ,  16    ,4         ,  24  /      2   /*     40  */  }
/* test case 15  */ ,{4       ,  4     ,4         , 28   /     1    /*    40   */  }
/* test case 16  */ ,{4       ,  8     ,4         , 24   /     1    /*    40   */  }
/* test case 17  */ ,{4       ,  16    ,4         ,  16  /      1   /*     40  */  }
/* test case 18  */ ,{4       ,  32    ,4         ,  0   /     1    /*    40   */  }
/* test case 19  */ ,{32      ,  0     ,8         , 0    /    1     /*   40    */  }
/* test case 20  */ ,{16      ,  0     ,8         , 16   /     1    /*    40   */  }
/* test case 21  */ ,{16      ,  4     ,8         , 12   /     1    /*    40   */  }
/* test case 22  */ ,{16      ,  4     ,8         , 24   /     2    /*    40   */  }
/* test case 23  */ ,{16      ,  8     ,8         , 8    /    1     /*   40    */  }
/* test case 24  */ ,{16      ,  8     ,8         , 16   /     2    /*    40   */  }
/* test case 25  */ ,{16      ,  16    ,8         ,  0   /     1    /*    40   */  }
/* test case 26  */ ,{8       , 0      ,8         ,24    /    1     /*   40    */  }
/* test case 27  */ ,{8       , 4      ,8         ,20    /    1     /*   40    */  }
/* test case 28  */ ,{8       , 8      ,8         ,16    /    1     /*   40    */  }
/* test case 29  */ ,{8       , 16     ,8         , 8    /    1     /*   40    */  }
/* test case 30  */ ,{8       , 16     ,8         , 16   /     2    /*    40   */  }
/* test case 31  */ ,{4       , 0      ,8         ,28    /    1     /*   40    */  }
/* test case 32  */ ,{4       , 4      ,8         ,24    /    1     /*   40    */  }
/* test case 33  */ ,{4       , 8      ,8         ,20    /    1     /*   40    */  }
/* test case 34  */ ,{4       , 16     ,8         , 12   /     1    /*    40   */  }
/* test case 35  */ ,{4       , 16     ,8         , 24   /     2    /*    40   */  }
/* test case 36  */ ,{ 16     ,   0    ,12        ,  12  /      1   /*     40  */  }
/* test case 37  */ ,{ 16     ,   0    ,12        ,  24  /      2   /*     40  */  }
/* test case 38  */ ,{ 16     ,   4    ,12        ,  8   /     1    /*    40   */  }
/* test case 39  */ ,{ 16     ,   4    ,12        ,  16  /      2   /*     40  */  }
/* test case 40  */ ,{ 16     ,   8    ,12        ,  4   /     1    /*    40   */  }
/* test case 41  */ ,{ 16     ,   8    ,12        ,  8   /     2    /*    40   */  }
/* test case 42  */ ,{ 8      ,  0     ,12        , 20   /     1    /*    40   */  }
/* test case 43  */ ,{ 8      ,  4     ,12        , 16   /     1    /*    40   */  }
/* test case 44  */ ,{ 8      ,  8     ,12        , 12   /     1    /*    40   */  }
/* test case 45  */ ,{ 8      ,  8     ,12        , 24   /     2    /*    40   */  }
/* test case 46  */ ,{ 8      ,  16    ,12        ,  4   /     1    /*    40   */  }
/* test case 47  */ ,{ 8      ,  16    ,12        ,  8   /     2    /*    40   */  }
/* test case 48  */ ,{ 4      ,  0     ,12        , 24   /     1    /*    40   */  }
/* test case 49  */ ,{ 4      ,  4     ,12        , 20   /     1    /*    40   */  }
/* test case 50  */ ,{ 4      ,  8     ,12        , 16   /     1    /*    40   */  }
/* test case 51  */ ,{ 4      ,  16    ,12        ,  8   /     1    /*    40   */  }
/* test case 52  */ ,{ 4      ,  16    ,12        ,  16  /      2   /*     40  */  }
/* test case 53  */ ,{ 16     ,   0    ,16        ,  8   /     1    /*    40   */  }
/* test case 54  */ ,{ 16     ,   0    ,16        ,  16  /      2   /*     40  */  }
/* test case 55  */ ,{ 16     ,   4    ,16        ,  4   /     1    /*    40   */  }
/* test case 56  */ ,{ 16     ,   4    ,16        ,  8   /     2    /*    40   */  }
/* test case 57  */ ,{ 16     ,   8    ,16        ,  0   /     1    /*    40   */  }
/* test case 58  */ ,{ 8      ,  0     ,16        , 16   /     1    /*    40   */  }
/* test case 59  */ ,{ 8      ,  4     ,16        , 12   /     1    /*    40   */  }
/* test case 60  */ ,{ 8      ,  4     ,16        , 24   /     2    /*    40   */  }
/* test case 61  */ ,{ 8      ,  8     ,16        , 8    /    1     /*   40    */  }
/* test case 62  */ ,{ 8      ,  8     ,16        , 16   /     2    /*    40   */  }
/* test case 63  */ ,{ 8      ,  16    ,16        ,  0   /     1    /*    40   */  }
/* test case 64  */ ,{ 4      ,  0     ,16        , 20   /     1    /*    40   */  }
/* test case 65  */ ,{ 4      ,  4     ,16        , 16   /     1    /*    40   */  }
/* test case 66  */ ,{ 4      ,  8     ,16        , 12   /     1    /*    40   */  }
/* test case 67  */ ,{ 4      ,  8     ,16        , 24   /     2    /*    40   */  }
/* test case 68  */ ,{ 4      ,  16    ,16        ,  4   /     1    /*    40   */  }
/* test case 69  */ ,{ 4      ,  16    ,16        ,  8   /     2    /*    40   */  }
/* test case 70  */ ,{ 16     ,   0    ,20        ,  4   /     1    /*    40   */  }
/* test case 71  */ ,{ 16     ,   0    ,20        ,  8   /     2    /*    40   */  }
/* test case 72  */ ,{ 16     ,   4    ,20        ,  0   /     1    /*    40   */  }
/* test case 73  */ ,{ 8      ,  0     ,20        , 12   /     1    /*    40   */  }
/* test case 74  */ ,{ 8      ,  0     ,20        , 24   /     2    /*    40   */  }
/* test case 75  */ ,{ 8      ,  4     ,20        , 8    /    1     /*   40    */  }
/* test case 76  */ ,{ 8      ,  4     ,20        , 16   /     2    /*    40   */  }
/* test case 77  */ ,{ 8      ,  8     ,20        , 4    /    1     /*   40    */  }
/* test case 78  */ ,{ 8      ,  8     ,20        , 8    /    2     /*   40    */  }
/* test case 79  */ ,{ 4      ,  0     ,20        , 16   /     1    /*    40   */  }
/* test case 80  */ ,{ 4      ,  4     ,20        , 12   /     1    /*    40   */  }
/* test case 81  */ ,{ 4      ,  4     ,20        , 24   /     2    /*    40   */  }
/* test case 82  */ ,{ 4      ,  8     ,20        , 8    /    1     /*   40    */  }
/* test case 83  */ ,{ 4      ,  8     ,20        , 16   /     2    /*    40   */  }
/* test case 84  */ ,{ 4      ,  16    ,20        ,  0   /     1    /*    40   */  }
/* test case 85  */ ,{ 16     ,   0    ,24        ,  0   /     1    /*    40   */  }
/* test case 86  */ ,{ 8      ,  0     ,24        , 8    /    1     /*   40    */  }
/* test case 87  */ ,{ 8      ,  0     ,24        , 16   /     2    /*    40   */  }
/* test case 88  */ ,{ 8      ,  4     ,24        , 4    /    1     /*   40    */  }
/* test case 89  */ ,{ 8      ,  4     ,24        , 8    /    2     /*   40    */  }
/* test case 90  */ ,{ 8      ,  8     ,24        , 0    /    1     /*   40    */  }
/* test case 91  */ ,{ 4      ,  0     ,24        , 12   /     1    /*    40   */  }
/* test case 92  */ ,{ 4      ,  0     ,24        , 24   /     2    /*    40   */  }
/* test case 93  */ ,{ 4      ,  4     ,24        , 8    /    1     /*   40    */  }
/* test case 94  */ ,{ 4      ,  4     ,24        , 16   /     2    /*    40   */  }
/* test case 95  */ ,{ 4      ,  8     ,24        , 8    /    2     /*   40    */  }
/* test case 96  */ ,{ 4      ,  8     ,24        , 4    /    1     /*   40    */  }

    /* must be last */
    ,{GT_NA   ,GT_NA   , GT_NA   , GT_NA }
};
/**
* @internal testCasesFor_falcon_sharedTablesConnectTablesToMemory_rawSbmSize function
* @endinternal
*
* @brief   debug function to check all valid cases of falcon to fill the 'shared tables'.
*          NOTE: the function NOT modify the HW values and NOT modify the SW values.
*                the function only 'check' and do printings
*          the function will print 'failed' cases (if any)
*          the function cause each tested case to print it's filling procedure.
*
* @note   APPLICABLE DEVICES:      sip6.
* @note   NOT APPLICABLE DEVICES:  non-sip6.
*
* @param[in]
*            devNum           - the device number
*
* @param[out]
*            None.
*
* @retval GT_OK          - always
*
*/
GT_STATUS testCasesFor_falcon_sharedTablesConnectTablesToMemory_rawSbmSize(
    IN GT_U8    devNum)
{
    GT_STATUS rc = GT_FAIL;
    const TEST_SMBS_FOR_CLIENTS_STC*   currCasePtr;
    GT_U32                  caseNum = 1;
    GT_U32                  didFail = 0;
    GT_U32                  orig_fdbSize,orig_emSize,orig_lpmPrefixNum,orig_arpSize;
    GT_U32                  max_fdbSize,max_emSize,max_lpmPrefixNum,max_arpSize;
    const TEST_SMBS_FOR_CLIENTS_STC    *devSharedMemValidCasesPtr;
    GT_STATUS   *statusArray;/* hold status per entry in
                        hawk_sharedMemoryModes_validCasesArr or
                        falcon_sharedMemoryModes_validCasesArr
                        */
    GT_U32  arraySize;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    devSharedMemValidCasesPtr = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ?
                                hawk_sharedMemoryModes_validCasesArr :
                                falcon_sharedMemoryModes_validCasesArr;
    arraySize = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ?
                sizeof(hawk_sharedMemoryModes_validCasesArr)  / sizeof (TEST_SMBS_FOR_CLIENTS_STC):
                sizeof(falcon_sharedMemoryModes_validCasesArr)/ sizeof (TEST_SMBS_FOR_CLIENTS_STC);

    dxChSharedTabled_DebugEnable(1);

    rc = sharedTablesParamsGet(devNum,&orig_fdbSize,&orig_emSize,&orig_lpmPrefixNum,&orig_arpSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        max_fdbSize = _256K;
        max_emSize =  _128K;
        max_lpmPrefixNum = PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(SIP6_MAX_LPM_CNS(devNum));
        max_arpSize = FALCON_MAX_SBM_ARP/*8*/ * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;/* 8 * 24K = 192K*/
    }
    else
    {
        max_fdbSize = _256K;
        max_emSize =  _256K;/* EM can be 256K (more than Falcon!)*/
        max_lpmPrefixNum = PRV_CPSS_DXCH_SIP6_LPM_RAM_GET_NUM_OF_PREFIXES_MAC(SIP6_MAX_LPM_CNS(devNum));/*(less than Falcon!)*/
        max_arpSize = HAWK_MAX_SBM_ARP/*24*/ * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.numOfArpsPerSbm;/* 24 * 16K = 384K (more than Falcon!)*/
    }

    statusArray = cpssOsMalloc(arraySize*sizeof(GT_STATUS));
    if (statusArray == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"statusArray allocation failed");

    currCasePtr = &devSharedMemValidCasesPtr[0];
    while(currCasePtr->fdb_num_sbm != GT_NA)
    {
        PRINT_PARAM_MAC(caseNum);
        PRINT_PARAM_MAC(currCasePtr->fdb_num_sbm);
        PRINT_PARAM_MAC(currCasePtr->em_num_sbm);
        PRINT_PARAM_MAC(currCasePtr->lpm_num_sbm);
        PRINT_PARAM_MAC(currCasePtr->arp_num_sbm);


        /* AC5P : reduce duplications due to LPM mode 1 or 2 */
        if(currCasePtr != &devSharedMemValidCasesPtr[0])
        {
            if(currCasePtr->fdb_num_sbm == (currCasePtr-1)->fdb_num_sbm &&
               currCasePtr->em_num_sbm  == (currCasePtr-1)->em_num_sbm  &&
               currCasePtr->lpm_num_sbm == (currCasePtr-1)->lpm_num_sbm &&
               currCasePtr->arp_num_sbm == (currCasePtr-1)->arp_num_sbm)
            {
                statusArray[caseNum-1] = GT_OK;

                cpssOsPrintf("skip duplicated case with the previous one \n");

                currCasePtr++;
                caseNum++;

                continue;
            }
        }



        /* set MAX values into the 'fine tuning' , to allow to check the case */
        /* the function also update the EM,FDB size in the HW (in EM,FDB unit) */
        rc = sharedTablesNewParamsSet(devNum,max_fdbSize,max_emSize,max_lpmPrefixNum,max_arpSize);
        if (rc != GT_OK)
        {
            cpssOsFree(statusArray);
            return rc;
        }

        rc = falcon_sharedTablesConnectTablesToMemory_rawSbmSize(devNum,
            currCasePtr->fdb_num_sbm,
            currCasePtr->em_num_sbm,
            currCasePtr->lpm_num_sbm,
            currCasePtr->arp_num_sbm,
            GT_TRUE/* we debug the cases */);
        if(rc != GT_OK)
        {
            cpssOsPrintf("Case [%d] : FAILED ! \n",caseNum);
            didFail++;
        }

        statusArray[caseNum-1] = rc;

        currCasePtr++;
        caseNum++;
    }

    dxChSharedTabled_DebugEnable(0);

    /* restore the parameters into the 'fine tuning' */
    /* the function also update the EM,FDB size in the HW (in EM,FDB unit) */
    rc = sharedTablesNewParamsSet(devNum,orig_fdbSize,orig_emSize,orig_lpmPrefixNum,orig_arpSize);
    if (rc != GT_OK)
    {
        cpssOsFree(statusArray);
        return rc;
    }

    if(didFail)
    {
        cpssOsPrintf("failed [%d] out of [%d]\n",didFail,caseNum-1);

        cpssOsPrintf("test fdb   em  lpm   arp \n");
        currCasePtr = &devSharedMemValidCasesPtr[0];
        caseNum = 1;
        while(currCasePtr->fdb_num_sbm != GT_NA)
        {
            cpssOsFree(statusArray);
            if(statusArray[caseNum-1] == GT_OK)
            {
                currCasePtr++;
                caseNum++;
                continue;
            }

            cpssOsPrintf(" %2d   %2d   %2d   %2d   %2d \n",
                caseNum,
                currCasePtr->fdb_num_sbm,
                currCasePtr->em_num_sbm,
                currCasePtr->lpm_num_sbm,
                currCasePtr->arp_num_sbm);

            currCasePtr++;
            caseNum++;
        }

    }
    else
    {
        cpssOsPrintf("ALL [%d] tests PASS \n", caseNum-1);
    }

    cpssOsFree(statusArray);


    return GT_OK;
}

/**
* @internal phoenix_non_sharedTablesInit function
* @endinternal
*
* @brief   collect tables update as done inside falcon_sharedTablesConnectTablesToMemory(...)
*               that update the units that are not 'SHM' (shared memory unit).
*               this also update 'DB' info that the CPSS hold
*
* @note   APPLICABLE DEVICES:      AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P.
*
* @param[in]
*            devNum           - the device number
*            sharedTableMode  - the simple predefined cases of 'shared tables'
*
* @param[out]
*            None.
*
* @retval GT_OK          - success to set the needed configurations
* @retval GT_NO_RESOURCE - one of the clients of one of the tables could not
*                           find place to be added.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
*
*/
static GT_STATUS phoenix_non_sharedTablesInit(
    IN  GT_U8                           devNum
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;/* pointer to the module
                                                configure of the PP's database*/
    GT_U32  PBR_Bank_Size;

    /* Configure the module configruation struct.   */
    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);

    /* the device not supports shared memory , but for generic coding considered with 28 banks for the shared memory */
    moduleCfgPtr->ip.lpmSharedMemoryBankNumber = /* 28 */
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared - /* 30 = shared + non-shared */
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksNonShared;   /*  2 = non-shared          */

    moduleCfgPtr->ip.lpmBankSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    /* update the <fdb> (involve updating other SW parameters and update the HW)*/
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb     */
    /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.size    */
    rc = prvCpssDxChBrgFdbSizeSet(devNum,PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm)
    {
        /* update the <emSize> (involve updating other SW parameters and update the HW)*/
        /* will update : PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum      */
        rc = prvCpssDxChCfgEmSizeSet(devNum,PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    PBR_Bank_Size= 0x2; /* use same value that used for Shared memory when working in 28 SBMs for LPM */
    rc = prvCpssDxChLpmPbrBankSizeSet(devNum,PBR_Bank_Size);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* will update HW and DB : PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank      */
    rc = prvCpssDxChBrgFdbMhtSet(devNum,16/* max MHT */);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm)
    {
        /* will update HW and DB : PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks  */
        rc = prvCpssDxChCfgEmMhtSet(devNum ,16/* max MHT */);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/* align an FUQ/AUQ descriptors  */
static GT_STATUS auqDescBlockPtrAlign(
    IN GT_U8    devNum,
    INOUT GT_U8    **descBlockPtrPtr,
    INOUT GT_U32   *descBlockSizePtr,
    IN GT_U32   numberOfDividers
)
{
    GT_U32 alignmentNumBytes;
    GT_U32 auMessageNumBytes = 4 * PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.auMessageNumOfWords;
    GT_U8  *descBlockPtr = *descBlockPtrPtr;
    GT_U32 descBlockSize = *descBlockSizePtr;

    if(descBlockPtr == NULL || descBlockSize == 0)
    {
        /* no update needed */
        return GT_OK;
    }

    if((((GT_UINTPTR)descBlockPtr) % auMessageNumBytes) != 0)
    {
        alignmentNumBytes =  (auMessageNumBytes -
                         (((GT_UINTPTR)descBlockPtr) % auMessageNumBytes));
        /* the address of the start of the block is now aligned */
        descBlockPtr     += alignmentNumBytes;/* update buffer according to alignment          */
        /* we need to apply alignment also on the number of bytes in the block */
        descBlockSize    -= alignmentNumBytes;
        /* align the size according to number of dividers */
        descBlockSize    /= numberOfDividers*auMessageNumBytes;
        descBlockSize    *= numberOfDividers*auMessageNumBytes;


        /* need to update the caller */
        *descBlockPtrPtr  = descBlockPtr;
        *descBlockSizePtr = descBlockSize;

        return GT_OK;
    }

    /* no update needed */
    return GT_OK;
}
/* align an FUQ/AUQ descriptors  */
static GT_STATUS alignPointersAndNumberOfElements
(
    IN GT_U8    devNum,
    INOUT GT_U8    **auDescBlockPtrPtr,
    INOUT GT_U32   *auDescBlockSizePtr,
    INOUT GT_U8    **fuDescBlockPtrPtr,
    INOUT GT_U32   *fuDescBlockSizePtr,
    IN GT_BOOL  useFuQueue,
    IN GT_BOOL  useDoubleAuq
)
{
    GT_STATUS   rc;
    GT_U32      auNumDividers = 1;/* number of AUQ dividers : spiting the AUQ for several port groups and secondary .... */
    GT_U32      fuNumDividers = 1;/* number of FUQ dividers : spiting the FUQ for several port groups and CNC2,3    .... */
    GT_U32      auq_numActivePortGroups;
    GT_U32      fuq_numActivePortGroups;

    /* logic MUST be the same as in prvCpssDxChCfgAddrUpInit(...) */
    if(useDoubleAuq == GT_TRUE)
    {
        auNumDividers *= 2;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GT_U32  auq_portGroupsBmp = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6AuqPortGroupBmp;
        GT_U32  fuq_portGroupsBmp = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6FuqPortGroupBmp;
        auq_numActivePortGroups = (GT_U16)prvCpssPpConfigBitmapNumBitsGet(auq_portGroupsBmp);
        fuq_numActivePortGroups = (GT_U16)prvCpssPpConfigBitmapNumBitsGet(fuq_portGroupsBmp);
    }
    else
    {
        GT_U16  numActivePortGroups;
        /* calculate number of port groups */
        rc = prvCpssPortGroupsNumActivePortGroupsInBmpGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            PRV_CPSS_DXCH_UNIT_MG_E,&numActivePortGroups);
        if(rc != GT_OK)
        {
            return rc;
        }

        auq_numActivePortGroups = numActivePortGroups;
        fuq_numActivePortGroups = numActivePortGroups;
    }

    auNumDividers *= auq_numActivePortGroups;

    rc = auqDescBlockPtrAlign(devNum,auDescBlockPtrPtr,auDescBlockSizePtr,auNumDividers);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(useFuQueue == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_CNC_MAC(devNum).cncUnits > 2)
        {
            fuNumDividers *= 2; /* one for the main FU/CNC0,1 and one for the CNC2,3 */
        }

        fuNumDividers *= fuq_numActivePortGroups;

        rc = auqDescBlockPtrAlign(devNum,fuDescBlockPtrPtr,fuDescBlockSizePtr,fuNumDividers);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChCfgPpLogicalInit function
* @endinternal
*
* @brief   This function Performs PP RAMs divisions to memory pools, according to
*         the supported modules in system.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] ppConfigPtr              - (pointer to)includes PP specific logical initialization
*                                      params.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_PP_MEM         - If the given configuration can't fit into the given
*                                       PP's memory capabilities, or
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function should perform all PP initializations, which
*       includes:
*       -  Init the Driver's part (by calling driverPpInit()).
*       -  Ram structs init, and Ram size check.
*       -  Initialization of the interrupt mechanism data structure.
*       2. When done, the function should leave the PP in disable state, and
*       forbid any packets forwarding, and any interrupts generation.
*       3. The execution flow for preparing for the initialization of core
*       modules is described below:
*       +-----------------+
*       | Init the 3 RAMs |  This initialization includes all structs
*       | conf. structs  |  fields but the base address field.
*       |         |
*       +-----------------+
*       |
*       (Ram is big    |         +-------------------------+
*       enough to hold  +--(No)-----------| return GT_OUT_OF_PP_MEM |
*       all tables?)   |         +-------------------------+
*       |
*       (Yes)
*       |
*       V
*       +-------------------+
*       | Set the Rams base |
*       | addr. according  |
*       | to the location  |
*       | fields.      |
*       +-------------------+
*       |
*       |
*       V
*       +----------------------------------+
*       | Init the             |
*       | PRV_CPSS_DXCH_MODULE_CONFIG_STC |
*       | struct, according        |
*       | to the info in RAM        |
*       | conf. struct.          |
*       +----------------------------------+
*       |
*       |
*       |
*       V
*       +---------------------------------+
*       | set the prepared        |
*       | module Cfg struct        |
*       | in the appropriate       |
*       | PRV_CPSS_DXCH_PP_CONFIG_STC   |
*       | struct.             |
*       +---------------------------------+
*
*/
static GT_STATUS internal_cpssDxChCfgPpLogicalInit
(
    IN      GT_U8                           devNum,
    IN      CPSS_DXCH_PP_CONFIG_INIT_STC    *ppConfigPtr
)
{
    GT_STATUS         rc = GT_OK;
    GT_U32            value;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;/* pointer to the module
                                                configure of the PP's database*/
    GT_U32              indexOfLastMemory;/* index of start of last memory in the LPM */
    GT_U32              indexForPbr;/* index of start of PBR in the LPM */
    GT_U32              numOfPbrBlocks ;/* number of blocks PBR needs */
    GT_U32              blockIndexForPbr;/* block index of start of PBR in the LPM */
    GT_U32              numOfIndexesInFirstBlockNotUsedForPbr;/* amount need to be skipped from first PBR block */
    GT_U32              lpmRamNumOfLines;
    GT_U32              numOfLinesInBlock;/* number of entries in each block memory */
    GT_U32              pbrInitBankSize;
    GT_U32              index;
    GT_U32              numberOfPbrLines = 0;
    GT_U32              totalLinesInBlock = 0;
    CPSS_DXCH_TABLE_ENT hwTable = CPSS_DXCH_TABLE_LAST_E;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ppConfigPtr);

    /* Configure the module configruation struct.   */
    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)
        {
            /* collect tables update as done inside falcon_sharedTablesConnectTablesToMemory(...)
                that update the units that are not 'SHM' (shared memory unit)
               this also update 'DB' info that the CPSS hold
            */
            rc = phoenix_non_sharedTablesInit(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* fill into the 'shared tables' HW the needed configurations, according
               to simple 'enum' of predefined cases. */
            rc = falcon_sharedTablesConnectTablesToMemory(devNum,
                ppConfigPtr->sharedTableMode);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* allow to test the cases with 'ERROR LOG' of CPSS */
            /*
             testCasesFor_falcon_sharedTablesConnectTablesToMemory_rawSbmSize(devNum);
            */

            moduleCfgPtr->ip.sharedTableMode = ppConfigPtr->sharedTableMode;
        }
    }

    /* as the function internalCfgAddrUpInit(...) may do alignment on some of
       the pointers this will cause reduce in the number of entries in a specific
       'port group' , but on other port group there maybe not such issue so the
       original pointer will be and the number of entries will differ from the
       changed 'port group'

       so to avoid this we must first align the start of the pointers to align to
       'auMessageNumBytes'
    */
    alignPointersAndNumberOfElements(devNum,
                                  &moduleCfgPtr->auCfg.auDescBlock,
                                  &moduleCfgPtr->auCfg.auDescBlockSize,
                                  &moduleCfgPtr->fuCfg.fuDescBlock,
                                  &moduleCfgPtr->fuCfg.fuDescBlockSize,
                                  moduleCfgPtr->fuqUseSeparate,
                                  moduleCfgPtr->useDoubleAuq);

    /* Init the AU and FU (if fuqUseSeparate is GT_TRUE) mechanism according
     to the data. Set in the module configuration structure. */
    /* Init the AU mechanism according to the data  */
    /* set in the module configuration structure.   */
    rc = prvCpssDxChCfgAddrUpInit(devNum,moduleCfgPtr->auCfg.auDescBlock,
                                  moduleCfgPtr->auCfg.auDescBlockSize,
                                  moduleCfgPtr->fuCfg.fuDescBlock,
                                  moduleCfgPtr->fuCfg.fuDescBlockSize,
                                  moduleCfgPtr->fuqUseSeparate,
                                  moduleCfgPtr->useDoubleAuq);
    if (rc != GT_OK)
        return rc;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E &&
       PRV_CPSS_IS_MULTI_PIPES_DEVICE_MAC(devNum)/* the GM not support it */ )
    {
        /* initialize the support for CNC uploads in device with shared CNC pipes
           into single FUQ in MG unit.
        */
        rc = sharedCncPortGroupLogicalInit(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        switch(ppConfigPtr->routingMode)
        {
            case CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E:
                value = 0;
                break;
            case CPSS_DXCH_TCAM_ROUTER_BASED_E:
                value = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssHwPpSetRegField(devNum,
                                       PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipRegs.routerAdditionalCtrlReg,
                                       0,1,value);
        if (rc != GT_OK)
            return rc;
    }

    moduleCfgPtr->ip.routingMode = ppConfigPtr->routingMode;
    moduleCfgPtr->ip.maxNumOfPbrEntries = ppConfigPtr->maxNumOfPbrEntries;
    totalLinesInBlock = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
    /*default mode for all devices*/
    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode = PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    if ( (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) &&
         (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) )
    {
        if((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE)&&/*Bobcat3 */
            /* current GM doesn't support DFX unit so far */
        (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE))
        {

            switch(ppConfigPtr->lpmMemoryMode)
            {
                case CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode = PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E;
                    value = 0;
                    break;
                case CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E:
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode = PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
                    value = 1;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }


            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum,
                                        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl15,
                                        25, 1, value);

            if (rc != GT_OK)
            {
                return rc;
            }

        }

        lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam);
        numOfLinesInBlock = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

        /*if we are working in half memory mode then do all the calculations for half memory size */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
           lpmRamNumOfLines/= 2;
           numOfLinesInBlock/=2;
        }

        if (moduleCfgPtr->ip.maxNumOfPbrEntries >= lpmRamNumOfLines)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if( numOfLinesInBlock !=
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)
        {
            /* this is indication that memory is not consecutive */

            if(moduleCfgPtr->ip.maxNumOfPbrEntries >
                numOfLinesInBlock)
            {
                /* need to calculate how many blocks the pbr needs */
                numOfPbrBlocks = (moduleCfgPtr->ip.maxNumOfPbrEntries +
                                  (numOfLinesInBlock-1))/
                                  (numOfLinesInBlock);

                /* find the index of the block were the pbr entries will start */
                blockIndexForPbr = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks - numOfPbrBlocks)*
                                    (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap);

                /* update the index according to the amount of pbr residing in the first block */
                numOfIndexesInFirstBlockNotUsedForPbr = (numOfLinesInBlock*numOfPbrBlocks)-
                                                        moduleCfgPtr->ip.maxNumOfPbrEntries;
                indexForPbr = blockIndexForPbr + numOfIndexesInFirstBlockNotUsedForPbr;

            }
            else
            {
                indexOfLastMemory =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap *
                    (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks - 1);

                indexForPbr = indexOfLastMemory +
                    (numOfLinesInBlock -
                        moduleCfgPtr->ip.maxNumOfPbrEntries);
            }
        }
        else
        {
            indexForPbr = lpmRamNumOfLines - moduleCfgPtr->ip.maxNumOfPbrEntries;
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr = indexForPbr;

        rc = prvCpssHwPpSetRegField(devNum,
                                    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.directAccessMode,
                                    0, PRV_CPSS_SIP_6_CHECK_MAC(devNum) ? 20 : 19,
                                    indexForPbr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (ppConfigPtr->lpmMemoryMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong LPM memory mode");
        }

        if (ppConfigPtr->maxNumOfPbrEntries > PRV_CPSS_SIP6_MAX_PBR_ENTRIES_CNS)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR entries number");
        }
        if (ppConfigPtr->sharedTableMode == CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E)
        {
            if (ppConfigPtr->maxNumOfPbrEntries > PRV_CPSS_SIP6_MAX_PBR_ENTRIES_FOR_MIN_LPM_CFG_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR entries number");
            }
        }
        indexForPbr = 0;
        pbrInitBankSize = 0;
        /* calculate pbr index and bank size */
        rc = prvCpssDxChCfgCalculatePbrIndexAndBankSize(devNum, &indexForPbr, &pbrInitBankSize);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (ppConfigPtr->maxNumOfPbrEntries > 0)
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr = indexForPbr;
            /*  init of PBR start index in LPM line addresses */
            rc = prvCpssHwPpSetRegField(devNum,
                                        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.directAccessMode,
                                        0, 20,indexForPbr/PRV_CPSS_DXCH_LPM_RAM_FALCON_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            /* set pbr bank size */
            rc = prvCpssHwPpSetRegField(devNum,
                                        PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.directAccessMode,
                                        24, 2,pbrInitBankSize);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            /* update PBR base in direct access table entry */
            numOfPbrBlocks = (moduleCfgPtr->ip.numOfBigPbrBanks >0) ? moduleCfgPtr->ip.numOfBigPbrBanks: moduleCfgPtr->ip.numOfSmallPbrBanks;

            numberOfPbrLines = moduleCfgPtr->ip.numberOfPbrLines;
            numberOfPbrLines = ((numberOfPbrLines-1)/moduleCfgPtr->ip.pbrBankSize)*totalLinesInBlock +/*num entries in 'full blocks' */
                                1 + ((numberOfPbrLines-1) % moduleCfgPtr->ip.pbrBankSize);/* num 'entries' in partial last block */
            rc = prvCpssDxChPbrTableConfigSet(devNum, indexForPbr, numberOfPbrLines);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        for (index = 0; index < NUM_OF_SHADOW_TABLES_CNS; index++)
        {
            hwTable = CPSS_DXCH_TABLE_LAST_E;
            switch(index)
            {
                case 0:
                    if (PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.doShadowLpm)
                    {
                        hwTable = CPSS_DXCH_SIP5_TABLE_LPM_MEM_E;
                    }
                    break;
                case 1:
                    if (PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.doShadowTs)
                    {
                        hwTable = CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E;
                    }
                    break;
                case 2:
                    /* Exact Match is optional table, check fine tuning */
                    if (PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.doShadowEm &&
                        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum)
                    {
                        hwTable = CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E;
                    }
                    break;
                case 3:
                    if (PRV_CPSS_DXCH_PP_MAC(devNum)->diagInfo.doShadowPbr)
                    {
                        hwTable = CPSS_DXCH_SIP6_TABLE_PBR_E;
                    }
                    break;
                default:
                    break;
            }
            if (hwTable == CPSS_DXCH_TABLE_LAST_E)
            {
                continue;
            }
            rc = prvCpssInitHwTableShadow(devNum, hwTable, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* indication that the function finished successfully */
    PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.wasDone_cpssDxChCfgPpLogicalInit = GT_TRUE;

    return GT_OK;
}

/**
* @internal cpssDxChCfgPpLogicalInit function
* @endinternal
*
* @brief   This function Performs PP RAMs divisions to memory pools, according to
*         the supported modules in system.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] ppConfigPtr              - (pointer to)includes PP specific logical initialization
*                                      params.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_PP_MEM         - If the given configuration can't fit into the given
*                                       PP's memory capabilities, or
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function should perform all PP initializations, which
*       includes:
*       -  Init the Driver's part (by calling driverPpInit()).
*       -  Ram structs init, and Ram size check.
*       -  Initialization of the interrupt mechanism data structure.
*       2. When done, the function should leave the PP in disable state, and
*       forbid any packets forwarding, and any interrupts generation.
*       3. The execution flow for preparing for the initialization of core
*       modules is described below:
*       +-----------------+
*       | Init the 3 RAMs |  This initialization includes all structs
*       | conf. structs  |  fields but the base address field.
*       |         |
*       +-----------------+
*       |
*       (Ram is big    |         +-------------------------+
*       enough to hold  +--(No)-----------| return GT_OUT_OF_PP_MEM |
*       all tables?)   |         +-------------------------+
*       |
*       (Yes)
*       |
*       V
*       +-------------------+
*       | Set the Rams base |
*       | addr. according  |
*       | to the location  |
*       | fields.      |
*       +-------------------+
*       |
*       |
*       V
*       +----------------------------------+
*       | Init the             |
*       | PRV_CPSS_DXCH_MODULE_CONFIG_STC |
*       | struct, according        |
*       | to the info in RAM        |
*       | conf. struct.          |
*       +----------------------------------+
*       |
*       |
*       |
*       V
*       +---------------------------------+
*       | set the prepared        |
*       | module Cfg struct        |
*       | in the appropriate       |
*       | PRV_CPSS_DXCH_PP_CONFIG_STC   |
*       | struct.             |
*       +---------------------------------+
*
*/
GT_STATUS cpssDxChCfgPpLogicalInit

(
    IN      GT_U8                           devNum,
    IN      CPSS_DXCH_PP_CONFIG_INIT_STC    *ppConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgPpLogicalInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ppConfigPtr));

    rc = internal_cpssDxChCfgPpLogicalInit(devNum, ppConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ppConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
* @param[in] modifyEnable             - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 modifyEnable
)
{
    GT_U32  regAddr;     /* register address */
    GT_U32  regValue;    /* register value */
    GT_U32  fieldOffset; /* The start bit number in the register */
    GT_U32  portGroupId; /*the port group Id - support multi port groups device */
    GT_U32   localPort;   /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    regValue = (modifyEnable  == GT_TRUE) ? 1 : 0;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChWriteTableEntryField(devNum,
                                               CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                               portNum,
                                               PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                               SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E, /* field name */
                                               PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                               regValue);
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        fieldOffset = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                     PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum) :
                     localPort;

        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                routerDevIdModifyEnReg[OFFSET_TO_WORD_MAC(fieldOffset)];

        return prvCpssHwPpPortGroupSetRegField(devNum,portGroupId,
                                         regAddr, OFFSET_TO_BIT_MAC(fieldOffset), 1, regValue);
    }
}

/**
* @internal cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet function
* @endinternal
*
* @brief   Sets device ID modification for Routed packets.
*         Enables/Disables FORWARD DSA tag modification of the <source device>
*         and <source port> fields of packets routed by the local device.
*         The <source device> is set to the local device ID and the <source port>
*         is set to 61 (the virtual router port).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
* @param[in] modifyEnable             - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 modifyEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modifyEnable));

    rc = internal_cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet(devNum, portNum, modifyEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modifyEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet function
* @endinternal
*
* @brief   Gets device ID modification for Routed packets.
*         See Set Api comments.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
*
* @param[out] modifyEnablePtr          - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *modifyEnablePtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;     /* register address */
    GT_U32    regValue;    /* register value */
    GT_U32    fieldOffset; /* The start bit number in the register */
    GT_U32    portGroupId; /*the port group Id - support multi port groups device */
    GT_U32     localPort;   /* local port - support multi-port-groups device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(modifyEnablePtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChReadTableEntryField(devNum,
                                            CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
                                            portNum,
                                            PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                            SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E, /* field name */
                                            PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                            &regValue);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        /* convert the 'Physical port' to portGroupId,local port -- supporting multi port groups device */
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

        fieldOffset = (localPort == CPSS_CPU_PORT_NUM_CNS) ?
                     PRV_CPSS_DXCH_PP_HW_INFO_HA_CPU_PORT_BIT_INDEX_MAC(devNum) :
                     localPort;

        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                routerDevIdModifyEnReg[OFFSET_TO_WORD_MAC(fieldOffset)];

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum,portGroupId,
                                         regAddr, OFFSET_TO_BIT_MAC(fieldOffset), 1, &regValue);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    *modifyEnablePtr = (regValue == 0) ? GT_FALSE : GT_TRUE;

    return rc;
}

/**
* @internal cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet function
* @endinternal
*
* @brief   Gets device ID modification for Routed packets.
*         See Set Api comments.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Port number (or CPU port)
*
* @param[out] modifyEnablePtr          - Boolean value of the FORWARD DSA tag modification:
*                                      GT_TRUE  -  Device ID Modification is Enabled.
*                                      GT_FALSE -  Device ID Modification is Disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *modifyEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modifyEnablePtr));

    rc = internal_cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet(devNum, portNum, modifyEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modifyEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDsaTagSrcDevPortModifySet function
* @endinternal
*
* @brief   Enables/Disables Device ID Modification upon packet sending to another
*         stack unit.
*         When Connecting DxCh Devices to SOHO in a Value Blade model to enable
*         DxCh grade Capabilities for FE Ports, in a staking system, we must be
*         able to overcome the 32 devices limitation.
*         To do that, SOHO Device Numbers are not unique and packets received
*         from the SOHO by the DxCh and are relayed to other stack units
*         have their Device ID changed to the DxCh Device ID.
*         On Upstream (packet from SOHO to DxCh):
*         The SOHO Sends a packet to the DxCh and the packet is sent back
*         to the SOHO. In this case we don't want to change the Device ID in
*         the DSA Tag so that the SOHO is able to filter the Src Port and is
*         able to send back the packet to its source when doing cross chip
*         flow control.
*         On Downsteam (packet from SOHO to SOHO):
*         The SOHO receives a packet from another SOHO in this case we
*         want to change the Device ID so that the packet is not filtered.
*         On top of the above, for forwarding packets between the DxCh devices
*         and for Auto Learning, the Port Number must also be changed.
*         In addition Changing the Device ID is needed only for FORWARD DSA Tag.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] modifedDsaSrcDev         - Boolean value of Enables/Disables Device ID
*                                      Modification:
*                                      GT_TRUE  -  DSA<SrcDev> is modified to the
*                                      DxCh Device ID and DSA<SrcPort>
*                                      is modified to the DxCh Ingress
*                                      port if all of the following are
*                                      conditions are met:
*                                      - packet is received with a
*                                      non-extended DSA Tag FORWARD    and
*                                      - DSA<SrcIsTrunk> = 0           and
*                                      - packet is transmitted with an
*                                      extended DSA Tag FORWARD.
*                                      GT_FALSE -  DSA<SrcDev> is not modified when the
*                                      packet is sent to another stac unit
*                                      via an uplink.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
static GT_STATUS internal_cpssDxChCfgDsaTagSrcDevPortModifySet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     modifedDsaSrcDev
)
{
    GT_U32  regAddr;     /* register address */
    GT_U32  regValue;    /* register value */
    GT_U32  startBit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    regValue = (modifedDsaSrcDev == GT_TRUE) ? 1 : 0;

    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                    eggrDSATagTypeConf[0];
        startBit = 28;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                    hdrAltGlobalConfig;
        startBit = 16;
    }

    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, regValue);
}

/**
* @internal cpssDxChCfgDsaTagSrcDevPortModifySet function
* @endinternal
*
* @brief   Enables/Disables Device ID Modification upon packet sending to another
*         stack unit.
*         When Connecting DxCh Devices to SOHO in a Value Blade model to enable
*         DxCh grade Capabilities for FE Ports, in a staking system, we must be
*         able to overcome the 32 devices limitation.
*         To do that, SOHO Device Numbers are not unique and packets received
*         from the SOHO by the DxCh and are relayed to other stack units
*         have their Device ID changed to the DxCh Device ID.
*         On Upstream (packet from SOHO to DxCh):
*         The SOHO Sends a packet to the DxCh and the packet is sent back
*         to the SOHO. In this case we don't want to change the Device ID in
*         the DSA Tag so that the SOHO is able to filter the Src Port and is
*         able to send back the packet to its source when doing cross chip
*         flow control.
*         On Downsteam (packet from SOHO to SOHO):
*         The SOHO receives a packet from another SOHO in this case we
*         want to change the Device ID so that the packet is not filtered.
*         On top of the above, for forwarding packets between the DxCh devices
*         and for Auto Learning, the Port Number must also be changed.
*         In addition Changing the Device ID is needed only for FORWARD DSA Tag.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] modifedDsaSrcDev         - Boolean value of Enables/Disables Device ID
*                                      Modification:
*                                      GT_TRUE  -  DSA<SrcDev> is modified to the
*                                      DxCh Device ID and DSA<SrcPort>
*                                      is modified to the DxCh Ingress
*                                      port if all of the following are
*                                      conditions are met:
*                                      - packet is received with a
*                                      non-extended DSA Tag FORWARD    and
*                                      - DSA<SrcIsTrunk> = 0           and
*                                      - packet is transmitted with an
*                                      extended DSA Tag FORWARD.
*                                      GT_FALSE -  DSA<SrcDev> is not modified when the
*                                      packet is sent to another stac unit
*                                      via an uplink.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortModifySet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     modifedDsaSrcDev
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDsaTagSrcDevPortModifySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modifedDsaSrcDev));

    rc = internal_cpssDxChCfgDsaTagSrcDevPortModifySet(devNum, modifedDsaSrcDev);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modifedDsaSrcDev));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDsaTagSrcDevPortModifyGet function
* @endinternal
*
* @brief   Get status of Device ID Modification upon packet sending to another
*         stack unit. See Set Api comments.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] modifedDsaSrcDevPtr      - Boolean value of Enables/Disables Device ID
*                                      Modification:
*                                      GT_TRUE  -  DSA<SrcDev> is modified to the
*                                      DxCh Device ID and DSA<SrcPort>
*                                      is modified to the DxCh Ingress
*                                      port if all of the following are
*                                      conditions are met:
*                                      - packet is received with a
*                                      non-extended DSA Tag FORWARD    and
*                                      - DSA<SrcIsTrunk> = 0           and
*                                      - packet is transmitted with an
*                                      extended DSA Tag FORWARD.
*                                      GT_FALSE -  DSA<SrcDev> is not modified when the
*                                      packet is sent to another stac unit
*                                      via an uplink.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
static GT_STATUS internal_cpssDxChCfgDsaTagSrcDevPortModifyGet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     *modifedDsaSrcDevPtr
)
{
    GT_STATUS rc;
    GT_U32    regAddr;     /* register address */
    GT_U32    regValue;    /* register value */
    GT_U32    startBit;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(modifedDsaSrcDevPtr);

    if (0 == PRV_CPSS_DXCH_LION_FAMILY_CHECK_MAC(devNum))
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                    eggrDSATagTypeConf[0];
        startBit = 28;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->haRegs.
                    hdrAltGlobalConfig;
        startBit = 16;
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    *modifedDsaSrcDevPtr = (regValue == 0) ? GT_FALSE : GT_TRUE;

    return rc;
}

/**
* @internal cpssDxChCfgDsaTagSrcDevPortModifyGet function
* @endinternal
*
* @brief   Get status of Device ID Modification upon packet sending to another
*         stack unit. See Set Api comments.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] modifedDsaSrcDevPtr      - Boolean value of Enables/Disables Device ID
*                                      Modification:
*                                      GT_TRUE  -  DSA<SrcDev> is modified to the
*                                      DxCh Device ID and DSA<SrcPort>
*                                      is modified to the DxCh Ingress
*                                      port if all of the following are
*                                      conditions are met:
*                                      - packet is received with a
*                                      non-extended DSA Tag FORWARD    and
*                                      - DSA<SrcIsTrunk> = 0           and
*                                      - packet is transmitted with an
*                                      extended DSA Tag FORWARD.
*                                      GT_FALSE -  DSA<SrcDev> is not modified when the
*                                      packet is sent to another stac unit
*                                      via an uplink.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This field is relevant for GigE Ports Only.
*       This change is in Addition to the SrcDev Change for Routed packets.
*
*/
GT_STATUS cpssDxChCfgDsaTagSrcDevPortModifyGet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     *modifedDsaSrcDevPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDsaTagSrcDevPortModifyGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modifedDsaSrcDevPtr));

    rc = internal_cpssDxChCfgDsaTagSrcDevPortModifyGet(devNum, modifedDsaSrcDevPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modifedDsaSrcDevPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*         when several type of entries like ARP and tunnelStart resize in the same
*         table (memory) , the function returns the number of entries for the least
*         denominator type of entry --> in this case number of ARP entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
*
* @param[out] numEntriesPtr            - (pointer to) number of entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgTableNumEntriesGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_CFG_TABLES_ENT    table,
    OUT GT_U32                      *numEntriesPtr
)
{
    GT_U32  numEntries;/* tmp num entries value */
    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(numEntriesPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE &&
       (0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed))
    {
        /*  sip6 :
            check that the shared tables mode was properly set by function cpssDxChCfgPpLogicalInit

            otherwise the values may hold combination of those table sizes that
            can not coexists together !
        */

        switch(table)
        {
            case CPSS_DXCH_CFG_TABLE_LPM_RAM_E:
            case CPSS_DXCH_CFG_TABLE_FDB_E:
            case CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E:
            case CPSS_DXCH_CFG_TABLE_ARP_E:
            case CPSS_DXCH_CFG_TABLE_TUNNEL_START_E:
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.wasDone_cpssDxChCfgPpLogicalInit == GT_FALSE)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                        "SIP6: the Shared Tables [FDB,EM,ARP/TS,LPM] sizes must be first defined by cpssDxChCfgPpLogicalInit (see param sharedTableMode)");
                }
                break;
            default:
                break;
        }
    }

    switch(table)
    {
        case CPSS_DXCH_CFG_TABLE_VLAN_E:
            numEntries =
                1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VLAN_MAC(devNum);
            break;
        case CPSS_DXCH_CFG_TABLE_FDB_E:
            if(GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfoValid)
            {
                /* only from sip6.30 */
                numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeBaseIndexInFdbTable;
            }
            else
            {
                numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb;
            }
            break;
        case CPSS_DXCH_CFG_TABLE_DDE_PARTITION_E:
            if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DDE_PARTITION table : relevant only from SIP6.30");
            }
            if(GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfoValid)
            {
                numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hsrPrp.ddeInfo.ddeNumEntriesInFdbTable;
            }
            else
            {
                numEntries = 0;
            }
            break;
        case CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E:
            if(!PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "FDB_WITH_DDE_PARTITION table : relevant only from SIP6.30");
            }
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.fdb;
            break;
        case CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "EM table : relevant only from SIP6");
            }
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum;
            break;
        case CPSS_DXCH_CFG_TABLE_PCL_ACTION_E:
        case CPSS_DXCH_CFG_TABLE_PCL_TCAM_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                /* num of 10B rules */
                numEntries =  PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws;
            }
            else
            {
                numEntries =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws * 4;
            }

            /* the number of 'standard' rules (same number of actions) */

            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E:
        case CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.router;
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.ecmpQos;
            break;
        case CPSS_DXCH_CFG_TABLE_TTI_TCAM_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;
            break;
        case CPSS_DXCH_CFG_TABLE_MLL_PAIR_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.mllPairs;
            break;
        case CPSS_DXCH_CFG_TABLE_POLICER_METERS_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum;
            break;
        case CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E:
            numEntries = (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) ?
                            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum :
                            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum ;
            break;
        case CPSS_DXCH_CFG_TABLE_VIDX_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vidxNum;

            if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
            {
                GT_BOOL isPE;
                GT_STATUS rc = cpssDxChBrgPeEnableGet(devNum,&isPE);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if(isPE == GT_TRUE)
                {
                    /* fix JIRA : CPSS-5764 : cpssDxChCfgTableNumEntriesGet return wrong value
                       in case BPE is enabled */
                    numEntries =
                        PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[CPSS_DXCH_TABLE_MULTICAST_E].maxNumOfEntries;
                }
            }
            break;
        case CPSS_DXCH_CFG_TABLE_ARP_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp;
            break;
        case CPSS_DXCH_CFG_TABLE_TUNNEL_START_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart;
            break;
        case CPSS_DXCH_CFG_TABLE_STG_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.stgNum;
            break;
        case CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E:
            numEntries = (GT_U32)(PRV_CPSS_DXCH_QOS_PROFILE_MAX_MAC(devNum));
            break;
        case CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E:
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_mac2me_numEntries)
            {
                numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_mac2me_numEntries;
            }
            else
            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                numEntries = PRV_CPSS_SIP5_MAC2ME_TABLE_MAX_ENTRIES_CNS;
            }
            else /* xCat3 and Lion2 */
            {
                numEntries = PRV_CPSS_DXCH3_MAC2ME_TABLE_MAX_ENTRIES_CNS;
            }
            break;

        case CPSS_DXCH_CFG_TABLE_CNC_E:
            /* the number of CNC counters in the device */
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlocks *
                         PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlockNumEntries;
            break;

        case CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E:
            /* the number of counters in CNC block */
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.cncBlockNumEntries;
            break;

        case CPSS_DXCH_CFG_TABLE_TRUNK_E:
            /* the max number of trunks that the device supports */
            /* the value is regardless to the actual number of trunks that the */
            /* cpssDxChTrunkInit() was initialized with */
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
            break;

        case CPSS_DXCH_CFG_TABLE_LPM_RAM_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
            {
                numEntries = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam);
            }
            else
            {
                numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam;
            }
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E:

            if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                return 0;
            }

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                numEntries = TABLE_NUM_ENTRIES_GET_MAC(devNum,CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E);
                /* 2 entries in HW line */
                numEntries *= 2;
            }
            else
            {
                numEntries = TABLE_NUM_ENTRIES_GET_MAC(devNum,CPSS_DXCH_SIP5_TABLE_ECMP_E);
            }
            break;

        case CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            numEntries = 1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MLL_MAC(devNum);
            break;

        case CPSS_DXCH_CFG_TABLE_EPORT_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            numEntries = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);
            break;

        case CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E:
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            numEntries = PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum);
            break;

        case CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E:
            numEntries = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
            break;

        case CPSS_DXCH_CFG_TABLE_SOURCE_ID_E:
            numEntries = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_SRC_ID_MAC(devNum);
            break;

        case CPSS_DXCH_CFG_TABLE_OAM_E:
            numEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.oamEntriesNum;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* return the caller with needed info */
    *numEntriesPtr = numEntries;

    return GT_OK;
}

/**
* @internal cpssDxChCfgTableNumEntriesGet function
* @endinternal
*
* @brief   the function return the number of entries of each individual table in
*         the HW
*         when several type of entries like ARP and tunnelStart resize in the same
*         table (memory) , the function returns the number of entries for the least
*         denominator type of entry --> in this case number of ARP entries.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
*
* @param[out] numEntriesPtr            - (pointer to) number of entries
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgTableNumEntriesGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_CFG_TABLES_ENT    table,
    OUT GT_U32                      *numEntriesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgTableNumEntriesGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, table, numEntriesPtr));

    rc = internal_cpssDxChCfgTableNumEntriesGet(devNum, table, numEntriesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, table, numEntriesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChTableValidIndexGetNext function
* @endinternal
*
* @brief   the function return the index of next valid table entry
*         starting a given index (inclusive)
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; BobK; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
* @param[in,out] entryIndexPtr            - pointer to search start index
* @param[in,out] entryIndexPtr            - pointer to valid index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE               - no more valid indices
*/
static GT_STATUS internal_cpssDxChTableValidIndexGetNext
(
    IN      GT_U8                       devNum,
    IN      CPSS_DXCH_CFG_TABLES_ENT    table,
    INOUT   GT_U32                      *entryIndexPtr
)
{
    GT_U32  numEntries;/* tmp num entries value */
    GT_U32  index;
    GT_STATUS rc;
    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(entryIndexPtr);

    rc = internal_cpssDxChCfgTableNumEntriesGet(devNum, table, &numEntries);
    if (rc != GT_OK)
        return rc;
    index = *entryIndexPtr;
    if (index >= numEntries)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);

    switch(table)
    {
        case CPSS_DXCH_CFG_TABLE_VLAN_E:
            while (index < numEntries)
            {
                GT_BOOL isValid;
                CPSS_PORTS_BMP_STC d0, d1;
                CPSS_DXCH_BRG_VLAN_INFO_STC d2;
                CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC d3;

                rc = cpssDxChBrgVlanEntryRead(devNum, (GT_U16)index, &d0, &d1, &d2, &isValid, &d3);
                if (rc != GT_OK || isValid == GT_TRUE)
                {
                    *entryIndexPtr = index;
                    return rc;
                }
                index++;
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
        case CPSS_DXCH_CFG_TABLE_FDB_E:
            while (index < numEntries)
            {
                GT_BOOL valid, skip, aged;
                GT_HW_DEV_NUM   hwDevNum;
                CPSS_MAC_ENTRY_EXT_STC entry;
                rc = cpssDxChBrgFdbMacEntryRead(devNum, index,
                            &valid, &skip, &aged,
                            &hwDevNum, &entry);
                if (rc != GT_OK || (valid == GT_TRUE && skip == GT_FALSE))
                {
                    *entryIndexPtr = index;
                    return rc;
                }
                index++;
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
        case CPSS_DXCH_CFG_TABLE_PCL_ACTION_E:
        case CPSS_DXCH_CFG_TABLE_PCL_TCAM_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E:
        case CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_TTI_TCAM_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_MLL_PAIR_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_POLICER_METERS_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_VIDX_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_ARP_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_TUNNEL_START_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_STG_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_CNC_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_TRUNK_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_LPM_RAM_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_EPORT_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        case CPSS_DXCH_CFG_TABLE_OAM_E:
            /* TODO */
            *entryIndexPtr = index;
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChTableValidIndexGetNext function
* @endinternal
*
* @brief   the function return the index of next valid table entry
*         starting a given index (inclusive)
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; BobK; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
* @param[in] table                    - type of the specific table
* @param[in,out] entryIndexPtr            - pointer to search start index
* @param[in,out] entryIndexPtr            - pointer to valid index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE               - no more valid indices
*/
GT_STATUS cpssDxChTableValidIndexGetNext
(
    IN      GT_U8                       devNum,
    IN      CPSS_DXCH_CFG_TABLES_ENT    table,
    INOUT   GT_U32                      *entryIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChTableValidIndexGetNext);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, table, entryIndexPtr));

    rc = internal_cpssDxChTableValidIndexGetNext(devNum, table, entryIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, table, entryIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. it is the responsibility of application to update the 'devNum' of
*         HW entries from 'oldDevNum' to 'newDevNum' using appropriate API's
*         such as 'Trunk members','Fdb entries','NH entries','PCE entries'...
*         2. it's application responsibility to update the HW device number !
*         see API cpssDxChCfgHwDevNumSet
*         3. no HW operations involved in this API
*         NOTE:
*         this function MUST be called under 'Interrupts are locked' and under
*         'Tasks lock'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device number
* @param[in] newDevNum                - new device number (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum value > 0x1f (0..31)
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChCfgReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
{
    GT_STATUS   rc;
    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(oldDevNum);

    /*validate the new device number*/
    if(newDevNum >= BIT_5)
    {
        /*device number limited to 5 bits */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(oldDevNum == newDevNum)
    {
        /* nothing more to do */
        return GT_OK;
    }

    /* swap the special DB - for re-init support */
    rc = prvCpssPpConfigDevDbRenumber(oldDevNum,newDevNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* swap the cpss DB */
    if(PRV_CPSS_PP_CONFIG_ARR_MAC[newDevNum] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PP_CONFIG_ARR_MAC[newDevNum] = PRV_CPSS_PP_CONFIG_ARR_MAC[oldDevNum];
    PRV_CPSS_PP_CONFIG_ARR_MAC[oldDevNum] = NULL;

    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[newDevNum])=
        PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[oldDevNum]);

    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[oldDevNum])= NULL;


    /* let the cpssDriver also 'SWAP' pointers */
    rc = prvCpssDrvHwPpRenumber(oldDevNum,newDevNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(newDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        || (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(newDevNum))
        || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(newDevNum)))
    {
        /* swap the HWS DB - for re-init support.
           should be done after prvCpssDrvHwPpRenumber because
           mvHwsReNumberDevNum access registers for newDevNum */
        rc = mvHwsReNumberDevNum(oldDevNum,newDevNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(newDevNum))
        {
           /* enable port manager for new devNum */
            PRV_SHARED_PORT_DIR_COMMON_PORT_MANAGER_SRC_GLOBAL_VAR(isPortMgrEnable)[newDevNum] = GT_TRUE;
        }
    }

    /* swap the MAC PHY DB - for re-init support */
    rc = prvCpssDxChCfg88e1690ReDevNum(oldDevNum,newDevNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(newDevNum))
    {
        /* UN-bind the old device from the DSMA Tx completed callback to the driver */
        rc = prvCpssDrvInterruptPpTxEndedCbBind(oldDevNum,NULL);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* bind the DSMA Tx completed callback to the driver - to the new device */
        rc = prvCpssDrvInterruptPpTxEndedCbBind(newDevNum,
                                    prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* update network interface DB*/
    HWINIT_GLOVAR(sysGenGlobalInfo.prvMiiDevNum) = newDevNum;

    return GT_OK;
}

/**
* @internal cpssDxChCfgReNumberDevNum function
* @endinternal
*
* @brief   function allow the caller to modify the DB of the cpss ,
*         so all info that was 'attached' to 'oldDevNum' will be moved to 'newDevNum'.
*         NOTE:
*         1. it is the responsibility of application to update the 'devNum' of
*         HW entries from 'oldDevNum' to 'newDevNum' using appropriate API's
*         such as 'Trunk members','Fdb entries','NH entries','PCE entries'...
*         2. it's application responsibility to update the HW device number !
*         see API cpssDxChCfgHwDevNumSet
*         3. no HW operations involved in this API
*         NOTE:
*         this function MUST be called under 'Interrupts are locked' and under
*         'Tasks lock'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - old device number
* @param[in] newDevNum                - new device number (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - the device oldDevNum not exist
* @retval GT_OUT_OF_RANGE          - the device newDevNum value > 0x1f (0..31)
* @retval GT_ALREADY_EXIST         - the new device number is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on error
*
* @note This API is "ISR safe".The interrupt are disabled on API entry and enabled on API exit
*
*/
GT_STATUS cpssDxChCfgReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgReNumberDevNum);

    CPSS_API_LOCK_MAC(oldDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_API_LOCK_MAC(newDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    PRV_CPSS_INT_SCAN_LOCK();

    CPSS_LOG_API_ENTER_MAC((funcId, oldDevNum, newDevNum));

    rc = internal_cpssDxChCfgReNumberDevNum(oldDevNum, newDevNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, oldDevNum, newDevNum));
    PRV_CPSS_INT_SCAN_UNLOCK();
    CPSS_API_UNLOCK_MAC(newDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_API_UNLOCK_MAC(oldDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDevInfoGet function
* @endinternal
*
* @brief   the function returns device static information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_CFG_DEV_INFO_STC   *devInfoPtr
)
{
    GT_STATUS   rc;

    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(devInfoPtr);

    /* fill in generic part of device info */
    rc = prvCpssPpConfigDevInfoGet(devNum,&(devInfoPtr->genDevInfo));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        devInfoPtr->genDevInfo.hwDevNumMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E;
    }

    devInfoPtr->genDevInfo.numOfLedInfPerPortGroup =
        prvCpssLedStreamNumOfInterfacesInPortGroupGet(devNum);

    return GT_OK;
}

/**
* @internal cpssDxChCfgDevInfoGet function
* @endinternal
*
* @brief   the function returns device static information
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_DXCH_CFG_DEV_INFO_STC   *devInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDevInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, devInfoPtr));

    rc = internal_cpssDxChCfgDevInfoGet(devNum, devInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, devInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCfgBindPortPhymacObject function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssDxChCfgBindPortPhymacObject
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr
)
{
    /* call the generic function (all validity checks done there) */
    return prvCpssPpConfigPortToPhymacObjectBind(devNum,portNum,macDrvObjPtr);
}

/**
* @internal cpssDxChCfgBindPortPhymacObject function
* @endinternal
*
* @brief   The function binds port phymac pointer
*         to 1540M PHYMAC object
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      macDrvObjPtr   - port phymac object pointer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChCfgBindPortPhymacObject(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_MACDRV_OBJ_STC * const macDrvObjPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgBindPortPhymacObject);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macDrvObjPtr));

    rc = internal_cpssDxChCfgBindPortPhymacObject(devNum, portNum, macDrvObjPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macDrvObjPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgPortDefaultSourceEportNumberSet function
* @endinternal
*
* @brief   Set Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ePort                    -  number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number.
*       2. Can be overridden by TTI Action/RBridge.
*       3. The 'default eport table' will be accessed at (source ePort)%512.
*          (APPLICABLE DEVICES: FALCON. %1K)
*       But the rest of eport tables are accessed by source ePort.
*
*/
static GT_STATUS internal_cpssDxChCfgPortDefaultSourceEportNumberSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         ePort
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,ePort);

    /* write to TTI-Physical-Port-Attribute table */
    return prvCpssDxChWriteTableEntryField(devNum,
                                             CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                             portNum,/*global port*/
                                             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                             SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER_E, /* field name */
                                             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                             ePort);
}

/**
* @internal cpssDxChCfgPortDefaultSourceEportNumberSet function
* @endinternal
*
* @brief   Set Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ePort                    -  number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. Default value is physical port number.
*       2. Can be overridden by TTI Action/RBridge.
*       3. The 'default eport table' will be accessed at (source ePort)%512.
*          (APPLICABLE DEVICES: FALCON. %1K)
*       But the rest of eport tables are accessed by source ePort.
*
*/
GT_STATUS cpssDxChCfgPortDefaultSourceEportNumberSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_PORT_NUM                         ePort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgPortDefaultSourceEportNumberSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ePort));

    rc = internal_cpssDxChCfgPortDefaultSourceEportNumberSet(devNum, portNum, ePort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ePort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgPortDefaultSourceEportNumberGet function
* @endinternal
*
* @brief   Get Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ePortPtr                 - pointer to ePort number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Can be overridden by TTI Action/RBridge
*
*/
static GT_STATUS internal_cpssDxChCfgPortDefaultSourceEportNumberGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *ePortPtr
)
{
    GT_STATUS   rc;     /* function return code */
    GT_U32  value;      /* value to write */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(ePortPtr);

    /* read from TTI-Physical-Port-Attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                     CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                                     portNum,
                                     PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                     SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER_E, /* field name */
                                     PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                     &value);

    *ePortPtr = value;

    return rc;
}

/**
* @internal cpssDxChCfgPortDefaultSourceEportNumberGet function
* @endinternal
*
* @brief   Get Port default Source ePort number.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] ePortPtr                 - pointer to ePort number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Can be overridden by TTI Action/RBridge
*
*/
GT_STATUS cpssDxChCfgPortDefaultSourceEportNumberGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT GT_PORT_NUM                         *ePortPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgPortDefaultSourceEportNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ePortPtr));

    rc = internal_cpssDxChCfgPortDefaultSourceEportNumberGet(devNum, portNum, ePortPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ePortPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgEgressHashSelectionModeSet function
* @endinternal
*
* @brief   Set the egress hash bit selection mode.
*          The egress pipe selects the range of bits from ingress hash
*          to be used as egress hash(EQ-->TXQ)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - hash selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgEgressHashSelectionModeSet
(
    IN GT_U8                                                        devNum,
    IN CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT    selectionMode
)
{
    GT_U32    regAddr;      /* value to write */
    GT_U32    value;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    switch(selectionMode)
    {
        case CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E:
            value = (5  << 5) | /* last bit 5 */
                    (0 << 0);   /* start bit 0 */
            break;
        case CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E:
            value = (11 << 5) | /* last bit 11 */
                    (6 << 0);   /* start bit 6 */
            break;
        case CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E:
            value = (11 << 5) | /* last bit 11 */
                    (0 << 0);   /* start bit 0 */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.cscdTrunkHashBitSelectionConfig;

    /* set the first and last bit of hash based on selection mode */
    return prvCpssHwPpSetRegField(devNum,regAddr ,0 ,10 ,value);
}

/**
* @internal cpssDxChCfgEgressHashSelectionModeSet function
* @endinternal
*
* @brief   Set the egress hash bit selection mode
*          The egress pipe selects the range of bits from ingress hash
*          to be used as egress hash(EQ-->TXQ)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] selectionMode            - hash selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgEgressHashSelectionModeSet
(
    IN GT_U8                                                        devNum,
    IN CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT    selectionMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgEgressHashSelectionModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, selectionMode));

    rc = internal_cpssDxChCfgEgressHashSelectionModeSet(devNum, selectionMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, selectionMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgEgressHashSelectionModeGet function
* @endinternal
*
* @brief   Get the egress hash bit selection mode
*          returns how egress pipe selects the range of bits from ingress hash
*          to be used as egress hash(EQ-->TXQ)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to)hash selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgEgressHashSelectionModeGet
(
    IN  GT_U8                                                       devNum,
    OUT CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT   *selectionModePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      hwData;
    GT_U32      regAddr;

    /* check parameters */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(selectionModePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.cscdTrunkHashBitSelectionConfig;

    rc = prvCpssHwPpGetRegField(devNum,regAddr ,0 ,10 ,&hwData);

    if(rc != GT_OK)
        return rc;

    switch(hwData)
    {
        case (5 << 5) | /* last bit 5 */
             (0 << 0):
            *selectionModePtr = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E;
            break;
        case (11 << 5) | /* last bit 11 */
              (6 << 0):   /* start bit 6 */
            *selectionModePtr = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E;
            break;
        case (11 << 5) | /* last bit 11 */
             ( 0 << 0):   /* start bit 0 */
            *selectionModePtr = CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssDxChCfgEgressHashSelectionModeGet function
* @endinternal
*
* @brief   Get the egress hash bit selection mode
*          returns how egress pipe selects the range of bits from ingress hash
*          to be used as egress hash(EQ-->TXQ)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] selectionModePtr         - (pointer to)hash selection mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgEgressHashSelectionModeGet
(
    IN  GT_U8                                                       devNum,
    OUT CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT   *selectionModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgEgressHashSelectionModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, selectionModePtr));

    rc = internal_cpssDxChCfgEgressHashSelectionModeGet(devNum, selectionModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, selectionModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChCfgIngressDropCntrModeSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mode                     - Ingress Drop Counter Mode.
* @param[in] portNum                  - port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[in] vlan                     - VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_OUT_OF_RANGE          - on wrong portNum or vlan.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgIngressDropCntrModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode,
    IN  GT_PORT_NUM                                 portNum,
    IN  GT_U16                                      vlan
)
{
    GT_U32    regAddr;     /* register address */
    GT_U32    value;       /* register field value */
    GT_U32    sip6EportModeValue = 0; /* port field start in falcon in bit 15. sip6 only*/
    GT_U32    fieldLength; /* the number of bits to be written to register */
    GT_U32    portGroupId,portPortGroupId;/*the port group Id - support multi-port-groups device */
    GT_PORT_NUM     localPort;   /* local port - support multi-port-groups device */
    GT_STATUS rc;          /* return code */

    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            ingrDropCntr.ingrDropCntrConfig;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
            eqBlkCfgRegs.ingressDropCntrConfReg;
    }

    switch(mode)
    {
        case CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E:
            fieldLength = 2;
            value = 0;
            break;

        case     CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E:
            if (vlan > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                fieldLength = 15;
            }
            else
            {
                fieldLength = 14;
            }
            value = 1 | (GT_U32)vlan << 2;
            break;

        case     CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E:
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                PRV_CPSS_HW_DEV_NUM_MAC(devNum),portNum);

            localPort = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                PRV_CPSS_HW_DEV_NUM_MAC(devNum),
                portNum);

            if(localPort > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
                {
                    fieldLength = 17;
                }
                else
                {
                    fieldLength = 15;
                }
            }
            else
            {
                fieldLength = 14;
            }

            if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                value = 2 | localPort << 2;
            }
            else
            {
               value = 2;
               fieldLength = 2; /* set only counter mode field */
               /* port field start in falcon in bit 15*/
               sip6EportModeValue = localPort;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    if((mode == CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E) &&
       (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE))
    {
        /* hold 'port filter' for not eArch devices.
           The eArch devices uses ePort for filter.
           Therefore same ePort value is in all port groups */

        /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
        portPortGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
        /* loop on all port groups :
            on the port group that 'own' the port , set the needed configuration
            on other port groups put 'NULL port'
        */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

        {
            if((CPSS_PORT_GROUP_UNAWARE_MODE_CNS == portPortGroupId) || (portPortGroupId == portGroupId))
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
            }
            else
            {
                localPort = PRV_CPSS_DXCH_NULL_PORT_NUM_CNS;
            }

            value = 2 | localPort << 2;

            rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr ,0 ,fieldLength ,value);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

        rc = GT_OK;
    }
    else
    {
        portPortGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldLength ,value);
    }
    if(rc == GT_OK)
    {
        /* save the info for 'get' configuration and 'read' counters */
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
                cfgIngressDropCntrMode.portGroupId = portPortGroupId;
    }
    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum))&&(mode == CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E) )
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr ,15 ,15 ,sip6EportModeValue);
        if(rc != GT_OK)
        {
            return rc;
        }

    }


    return rc;

}

/**
* @internal cpssDxChCfgIngressDropCntrModeSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mode                     - Ingress Drop Counter Mode.
* @param[in] portNum                  - port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[in] vlan                     - VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum or mode.
* @retval GT_OUT_OF_RANGE          - on wrong portNum or vlan.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrModeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT mode,
    IN  GT_PORT_NUM                                 portNum,
    IN  GT_U16                                      vlan
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgIngressDropCntrModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mode, portNum, vlan));

    rc = internal_cpssDxChCfgIngressDropCntrModeSet(devNum, mode, portNum, vlan);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mode, portNum, vlan));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgIngressDropCntrModeGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] modePtr                  - pointer to Ingress Drop Counter Mode.
* @param[out] portNumPtr               - pointer to port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[out] vlanPtr                  - pointer to VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - illegal state of configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgIngressDropCntrModeGet
(
    IN   GT_U8                                       devNum,
    OUT  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT *modePtr,
    OUT  GT_PORT_NUM                                 *portNumPtr,
    OUT  GT_U16                                      *vlanPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      value;       /* register field value */
    GT_U32      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_STATUS   rc;          /* return code */
    GT_U32      fieldOffset; /* The number of bits to be written to register */


    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            ingrDropCntr.ingrDropCntrConfig;
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            fieldOffset = 30;
        }
        else
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            fieldOffset = 17;
        }
        else
        {
            fieldOffset = 15;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
            eqBlkCfgRegs.ingressDropCntrConfReg;
        fieldOffset = 14;
    }

    portGroupId = PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.
            cfgIngressDropCntrMode.portGroupId;

    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr ,0 ,fieldOffset ,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(value & 0x3)
    {
        case 0:
            *modePtr = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E;
            break;
        case 1:
            *modePtr = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E;
            if (vlanPtr != NULL)
            {
                if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
                {
                    *vlanPtr = (GT_U16)((value >> 2) & 0x1FFF);
                }
                else
                {
                    *vlanPtr = (GT_U16)((value >> 2) & 0xFFF);
                }
            }

            break;
        case 2:
            *modePtr = CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E;
            if (portNumPtr != NULL)
            {
                if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
                {
                    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        *portNumPtr = (value >> 15) & 0x3FFF;
                    }
                    else
                    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
                    {
                        *portNumPtr = (value >> 2) & 0x7FFF;
                    }
                    else
                    {
                        *portNumPtr = (value >> 2) & 0x1FFF;
                    }
                }
                else
                {
                    *portNumPtr = (value >> 2) & 0xFFF;

                    if(portGroupId != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
                    {
                        /* support multi-port-groups device , convert local port to global port */
                        *portNumPtr = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum,portGroupId,(*portNumPtr));
                    }
                }
            }

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;

}

/**
* @internal cpssDxChCfgIngressDropCntrModeGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] modePtr                  - pointer to Ingress Drop Counter Mode.
* @param[out] portNumPtr               - pointer to port number monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E
*                                      counter mode.
* @param[out] vlanPtr                  - pointer to VLAN ID monitored by Ingress Drop Counter.
*                                      This parameter is applied upon
*                                      CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E
*                                      counter mode.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - illegal state of configuration
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrModeGet
(
    IN   GT_U8                                       devNum,
    OUT  CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT *modePtr,
    OUT  GT_PORT_NUM                                 *portNumPtr,
    OUT  GT_U16                                      *vlanPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgIngressDropCntrModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modePtr, portNumPtr, vlanPtr));

    rc = internal_cpssDxChCfgIngressDropCntrModeGet(devNum, modePtr, portNumPtr, vlanPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modePtr, portNumPtr, vlanPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgIngressDropCntrSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] counter                  - Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgIngressDropCntrSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      counter
)
{
    GT_U32      regAddr;     /* register address */

    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* next devices hold the counter as 'read only,clear' so can't 'set' to it*/
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E
                                          | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
        eqBlkCfgRegs.ingressDropCntrReg;

    return prvCpssPortGroupsBmpCounterSet(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              regAddr, 0, 32, counter);
}

/**
* @internal cpssDxChCfgIngressDropCntrSet function
* @endinternal
*
* @brief   Set the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] counter                  - Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      counter
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgIngressDropCntrSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counter));

    rc = internal_cpssDxChCfgIngressDropCntrSet(devNum, counter);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counter));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCfgIngressDropCntrGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgIngressDropCntrGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
)
{
    GT_U32      regAddr;     /* register address */

    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* validate the pointer */
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).
            ingrDropCntr.ingrDropCntr;
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.
            eqBlkCfgRegs.ingressDropCntrReg;
    }

    return prvCpssPortGroupsBmpCounterSummary(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                       regAddr, 0, 32,
                                                       counterPtr, NULL);
}

/**
* @internal cpssDxChCfgIngressDropCntrGet function
* @endinternal
*
* @brief   Get the Ingress Drop Counter value.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
*
* @param[out] counterPtr               - (pointer to) Ingress Drop Counter value
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgIngressDropCntrGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgIngressDropCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterPtr));

    rc = internal_cpssDxChCfgIngressDropCntrGet(devNum, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* prvCpssDxChSip6GlobalEportMaskCheck
*
* DESCRIPTION:
*       Check that the Eport Mask can be supported by SIP6 in terms of 'min-max values'
*       doing that by check that mask bits are continues.
*
* APPLICABLE DEVICES:
*       Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       devNum  - Device number.
*       mask    - the mask of the global eport
*       pattern - the pattern of the global eport
*
* OUTPUTS:
*       minValuePtr - pointer to min value.
*       maxValuePtr - pointer to max value.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong mask.
*
* COMMENTS:
*
*******************************************************************************/
/*extern*/ GT_STATUS prvCpssDxChSip6GlobalEportMaskCheck
(
    IN  GT_U8   devNum,
    IN  GT_U32  mask,
    IN  GT_U32  pattern,
    OUT GT_U32  *minValuePtr,
    OUT GT_U32  *maxValuePtr
)
{
    GT_U32    ii;
    GT_U32    maxBits = PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).ePort;
    GT_U32    startBit = 0;
    /* check that mask bits are continues */
    for(ii = 0 ; ii < maxBits ; ii++)
    {
        if(mask & (1<<ii))
        {
            /* found first bit set in mask */
            startBit = ii;

            for(/*ii continue*/;ii < maxBits;ii++)
            {
                if(0 == (mask & (1<<ii)))
                {
                    /* found first bit that breaks the mask */

                    /* check that no more bits that are set */
                    for(/* ii continue */ ; ii < maxBits ; ii++)
                    {
                        if(mask & (1<<ii))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the MASK[0x%5.5x] must be with continues bits",
                                mask);
                        }
                    }

                    break;
                }
            }
            break;
        }
    }

    *minValuePtr = (pattern & mask);
    *maxValuePtr = (*minValuePtr) + ((1<<startBit) - 1)/*bits outside the mask*/;

    return GT_OK;
}


/**
* @internal sip6_cpssDxChCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman..
*
* @param[in] devNum                   - device number
* @param[in] usePatternMask           - indication to use {pattern,mask} or {min,max}
*                                      GT_TRUE  - use {pattern,mask}
*                                      GT_FALSE - use {min,max}
* @param[in] globalPtr                - (pointer to) Global EPorts for the Bridge,L2Mll to use.
*                                      In the Bridge Used for:
*                                      a) Whether the bridge engine looks at the device number
*                                      when performing MAC SA lookup or local switching check.
*                                      b) That the local device ID (own device) is learned in the
*                                      FDB entry with the global ePort
*                                      In the L2MLL Used for:
*                                      Enabling the L2MLL replication engine to ignore the
*                                      device ID when comparing source and target interfaces
*                                      for source filtering.
*                                      Typically configured to include global ePorts
*                                      representing a trunk or an ePort ECMP group
*                                      NOTE: For defining the EPorts that used for 'Multi-Target ePort to eVIDX Mapping'
*                                      use API cpssDxChL2MllMultiTargetPortSet(...)
*                                      (see cpssDxChL2Mll.h file for more related functions)
* @param[in] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*                                      Used for: The 'Primary ePorts' that need to be converted to
*                                      one of their 'Secondary ePorts'.
* @param[in] l2DlbPtr                 - (pointer to) L2 DLB Primary ePorts.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_cpssDxChCfgGlobalEportSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         usePatternMask,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32    regAddr;  /* register address */
    GT_U32    ePortMask;/* mask of ePort */
    GT_U32    sip6_global_minValue = 0,sip6_global_maxValue = 0;/* global : min and max values for sip 6 */
    GT_U32    sip6_l2Ecmp_minValue = 0,sip6_l2Ecmp_maxValue = 0;/* L2 ecmp : min and max values for sip 6 */
    GT_U32    sip6_l2Dlb_minValue = 0,sip6_l2Dlb_maxValue = 0;/* L2 DLb : min and max values for sip 6 */
    GT_U32    fieldSize;    /* register field size */
    GT_U32    fieldValue;   /* register field value */
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev; /* pp configuration structure */

    ePortMask = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum); /* 0x1FFFF */
    fieldSize = 17;
    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if(globalPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E ||
       l2EcmpPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E ||
       l2DlbPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        /* global eport range is based on min/max configuration */
        usePatternMask = GT_FALSE;
    }

    if(usePatternMask == GT_TRUE)
    {
        /* check ECMP pattern & mask values */
        if( CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E == l2EcmpPtr->enable )
        {
            if( (l2EcmpPtr->mask > ePortMask) ||
                ((l2EcmpPtr->pattern & l2EcmpPtr->mask) != l2EcmpPtr->pattern) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* check that can convert mask to range */
            rc = prvCpssDxChSip6GlobalEportMaskCheck(
                devNum,
                l2EcmpPtr->mask,
                l2EcmpPtr->pattern,
                &sip6_l2Ecmp_minValue,
                &sip6_l2Ecmp_maxValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /*these values guaranties NO Match.*/
            sip6_l2Ecmp_minValue = 1;
            sip6_l2Ecmp_maxValue = 0;
        }

        /* check DLB pattern & mask values */
        if( CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E == l2DlbPtr->enable )
        {
            if( (l2DlbPtr->mask > ePortMask) ||
                ((l2DlbPtr->pattern & l2DlbPtr->mask) != l2DlbPtr->pattern) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* check that can convert mask to range */
            rc = prvCpssDxChSip6GlobalEportMaskCheck(
                devNum,
                l2DlbPtr->mask,
                l2DlbPtr->pattern,
                &sip6_l2Dlb_minValue,
                &sip6_l2Dlb_maxValue);

            if(rc != GT_OK)
            {
                return rc;
            }

            if( CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E == l2EcmpPtr->enable)
            {
               if((sip6_l2Ecmp_minValue <= sip6_l2Dlb_maxValue) &&
                       (sip6_l2Dlb_minValue <= sip6_l2Ecmp_maxValue))
               {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DLB ePort Ranges [%d:%d] Overlaps with ECMP eport Ranges [%d:%d]",
                       sip6_l2Dlb_minValue, sip6_l2Dlb_maxValue,
                       sip6_l2Ecmp_minValue, sip6_l2Ecmp_maxValue);
               }
            }
        }
        else
        {
            /*these values guaranties NO Match.*/
            sip6_l2Dlb_minValue = 1;
            sip6_l2Dlb_maxValue = 0;
        }
        /* check global pattern & mask values */
        if( CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E == globalPtr->enable )
        {
            if( (globalPtr->mask > ePortMask) ||
                ((globalPtr->pattern & globalPtr->mask) != globalPtr->pattern) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* check that can convert mask to range */
            rc = prvCpssDxChSip6GlobalEportMaskCheck(
                devNum,
                globalPtr->mask,
                globalPtr->pattern,
                &sip6_global_minValue,
                &sip6_global_maxValue);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /*these values guaranties NO Match.*/
            sip6_global_minValue = 1;
            sip6_global_maxValue = 0;
        }
    }
    else /* Min/Max ranges values*/
    {
        if(globalPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E ||
           l2EcmpPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E ||
           l2DlbPtr->enable  == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Mask/value configurations and min/max configuration type is not simultaneously supported"
                "global type [%d], L2ECMP type [%d], L2DLB type [%d]\n",
                globalPtr->enable, l2EcmpPtr->enable, l2DlbPtr->enable);
        }

        if(globalPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
        {
            if((globalPtr->minValue > ePortMask)||
               (globalPtr->maxValue > ePortMask)||
               (globalPtr->minValue > globalPtr->maxValue))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Global eport minValue [0x%x], maxValue [0x%x], allowed range [0x%x]",
                                              globalPtr->minValue, globalPtr->maxValue, ePortMask);
            }
            sip6_global_minValue = globalPtr->minValue;
            sip6_global_maxValue = globalPtr->maxValue;
        }
        else
        {
            sip6_global_minValue = 1;
            sip6_global_maxValue = 0;
        }

        if(l2EcmpPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
        {
            if((sip6_l2Ecmp_minValue > ePortMask)||
               (sip6_l2Ecmp_maxValue > ePortMask)||
               (sip6_l2Ecmp_minValue > sip6_l2Ecmp_maxValue))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "L2-ECMP eport minValue [0x%x], maxValue [0x%x], allowed range [0x%x]",
                                              sip6_l2Ecmp_minValue, sip6_l2Ecmp_maxValue, ePortMask);
            }
            sip6_l2Ecmp_minValue = l2EcmpPtr->minValue;
            sip6_l2Ecmp_maxValue = l2EcmpPtr->maxValue;
        }
        else
        {
            sip6_l2Ecmp_minValue = 1;
            sip6_l2Ecmp_maxValue = 0;
        }

        if(l2DlbPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
        {
            if((sip6_l2Dlb_minValue > ePortMask)||
               (sip6_l2Dlb_maxValue > ePortMask)||
               (sip6_l2Dlb_minValue > sip6_l2Dlb_maxValue))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "L2-DLB eport minValue [0x%x], maxValue [0x%x], allowed range [0x%x]",
                                              sip6_l2Dlb_minValue, sip6_l2Dlb_maxValue, ePortMask);
            }
            sip6_l2Dlb_minValue  = l2DlbPtr->minValue;
            sip6_l2Dlb_maxValue  = l2DlbPtr->maxValue;

            if(l2EcmpPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
            {
                if((sip6_l2Ecmp_minValue <= sip6_l2Dlb_maxValue) &&
                    (sip6_l2Dlb_minValue <= sip6_l2Ecmp_maxValue))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "DLB ePort Ranges [%d:%d] Overlaps with ECMP eport Ranges [%d:%d]",
                        sip6_l2Dlb_minValue, sip6_l2Dlb_maxValue,
                        sip6_l2Ecmp_minValue, sip6_l2Ecmp_maxValue);
                }
            }
        }
        else
        {
            sip6_l2Dlb_minValue  = 1;
            sip6_l2Dlb_maxValue  = 0;
        }
    }

    /* write bridge global ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
        globalEportConifguration.globalEportMinValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_global_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write bridge global ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                globalEportConifguration.globalEportMaxValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_global_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write MLL global ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.L2mllGlobalEPortMinValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_global_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write MLL global ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.L2mllGlobalEPortMaxValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_global_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /***********************************/
    /* NEW in SIP6.0 -- Enable/disable */
    /***********************************/
    if(globalPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E ||
       globalPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        fieldValue = 1;
    }
    else
    {
        fieldValue = 0;
    }
    /* write <L2MLL Global ePort Enable> */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLGlobalCtrl;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,22 ,1 ,fieldValue);
    if( GT_OK != rc )
    {
        return rc;
    }


    /* write EQ-ECMP ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortMinimum;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_l2Ecmp_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write EQ-ECMP global ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortMaximum;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_l2Ecmp_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write EQ-DLB ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbEportMinValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_l2Dlb_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write EQ-DLB ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbEportMaxValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,sip6_l2Dlb_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* store per global eport type configuration mode */
    pDev->ePortCfgTypeInfo.globalEPortCfgType = globalPtr->enable;
    pDev->ePortCfgTypeInfo.l2ecmpEPortCfgType = l2EcmpPtr->enable;
    pDev->ePortCfgTypeInfo.l2DlbEPortCfgType = l2DlbPtr->enable;

    return GT_OK;
}


/**
* @internal internal_cpssDxChCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalPtr                - (pointer to) Global EPorts for the Bridge,L2Mll to use.
*                                      In the Bridge Used for:
*                                      a) Whether the bridge engine looks at the device number
*                                      when performing MAC SA lookup or local switching check.
*                                      b) That the local device ID (own device) is learned in the
*                                      FDB entry with the global ePort
*                                      In the L2MLL Used for:
*                                      Enabling the L2MLL replication engine to ignore the
*                                      device ID when comparing source and target interfaces
*                                      for source filtering.
*                                      Typically configured to include global ePorts
*                                      representing a trunk or an ePort ECMP group
*                                      NOTE: For defining the EPorts that used for 'Multi-Target ePort to eVIDX Mapping'
*                                      use API cpssDxChL2MllMultiTargetPortSet(...)
*                                      (see cpssDxChL2Mll.h file for more related functions)
* @param[in] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*                                      Used for: The 'Primary ePorts' that need to be converted to
*                                      one of their 'Secondary ePorts'.
* @param[in] l2DlbPtr              - (pointer to) L2 DLB ePorts
*                                    (APPLICABLE DEVICE: Falcon)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgGlobalEportSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32    regAddr;  /* register address */
    GT_U32    ecmpPattern, ecmpMask, globalPattern, globalMask;/* registers values */
    GT_U32    ePortMask;/* mask of ePort */
    GT_U32    noMatchPattern,noMatchMask;/*these values guaranties NO Match.*/
    GT_U32    fieldSize;    /* register field size */
    /*these values guaranties NO Match.
    because in formula ((ePort & Mask) == Pattern) ? global : not_global
    then if bit set in pattern and not set in mask cause that no ePort can be
    treated as 'global' */
    noMatchPattern = 1;
    noMatchMask = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(l2EcmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(globalPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(l2DlbPtr);

        return sip6_cpssDxChCfgGlobalEportSet(devNum,
            GT_TRUE/* BWC mode : use {pattern,mask} */,
            globalPtr,
            l2EcmpPtr,
            l2DlbPtr);
    }

    ePortMask = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum);

    /* check ECMP pattern & mask values */
    if( CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E  == l2EcmpPtr->enable )
    {
        if( (l2EcmpPtr->mask > ePortMask) ||
            ((l2EcmpPtr->pattern & l2EcmpPtr->mask) != l2EcmpPtr->pattern) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        ecmpPattern = l2EcmpPtr->pattern;
        ecmpMask    = l2EcmpPtr->mask;
    }
    else
    {
        ecmpPattern = noMatchPattern;
        ecmpMask    = noMatchMask;
    }

    /* check global pattern & mask values */
    if( CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E  == globalPtr->enable )
    {
        if( (globalPtr->mask > ePortMask) ||
            ((globalPtr->pattern & globalPtr->mask) != globalPtr->pattern) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        globalPattern = globalPtr->pattern;
        globalMask    = globalPtr->mask;
    }
    else
    {
        globalPattern = noMatchPattern;
        globalMask    = noMatchMask;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        fieldSize = 15;
    }
    else
    {
        fieldSize = 13;
    }

    /* write ECMP global ePort range value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,ecmpPattern);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write ECMP global ePort range mask */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortMask;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,ecmpMask);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write bridge global ePort range value */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
        globalEportConifguration.globalEPortValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,globalPattern);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write bridge global ePort range mask */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                globalEportConifguration.globalEPortMask;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,fieldSize ,globalMask);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write L2 MLL global ePort range value */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.globalEPortRangeConfig;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,20 ,globalPattern);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write L2 MLL global ePort range mask */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.globalEPortRangeMask;
    rc = prvCpssHwPpSetRegField(devNum, regAddr ,0 ,20 ,globalMask);
    if( GT_OK != rc )
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChCfgGlobalEportSet function
* @endinternal
*
* @brief   Set Global ePorts ranges configuration.
*         Global ePorts are ePorts that are global in the entire system, and are
*         not unique per device as regular ePorts are.
*         Global ePorts are used for Representing:
*         1. a Trunk.(map target ePort to trunkId by the E2PHY)
*         2. a ePort ECMP Group.(map 'primary' target ePort to 'secondary' target ePort)
*         3. an ePort Replication Group (eVIDX).(map target ePort to eVIDX group)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalPtr                - (pointer to) Global EPorts for the Bridge,L2Mll to use.
*                                      In the Bridge Used for:
*                                      a) Whether the bridge engine looks at the device number
*                                      when performing MAC SA lookup or local switching check.
*                                      b) That the local device ID (own device) is learned in the
*                                      FDB entry with the global ePort
*                                      In the L2MLL Used for:
*                                      Enabling the L2MLL replication engine to ignore the
*                                      device ID when comparing source and target interfaces
*                                      for source filtering.
*                                      Typically configured to include global ePorts
*                                      representing a trunk or an ePort ECMP group
*                                      NOTE: For defining the EPorts that used for 'Multi-Target ePort to eVIDX Mapping'
*                                      use API cpssDxChL2MllMultiTargetPortSet(...)
*                                      (see cpssDxChL2Mll.h file for more related functions)
* @param[in] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
*                                      Used for: The 'Primary ePorts' that need to be converted to
*                                      one of their 'Secondary ePorts'.
* @param[in] l2DlbPtr              - (pointer to) L2 DLB ePorts
*                                    (APPLICABLE DEVICE: Falcon)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgGlobalEportSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    IN  CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgGlobalEportSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalPtr, l2EcmpPtr, l2DlbPtr));

    rc = internal_cpssDxChCfgGlobalEportSet(devNum, globalPtr, l2EcmpPtr, l2DlbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalPtr, l2EcmpPtr, l2DlbPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*******************************************************************************
* prvCpssDxChSip6GlobalEportMaskGet
*
* DESCRIPTION:
*       Get the Eport Mask and pattern for 'min-max values'.
*
* APPLICABLE DEVICES:
*       Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       minValue - min value.
*       maxValue - max value.
*
* OUTPUTS:
*       patternPtr - (pointer to) the pattern of the global eport
*       maskPtr - (pointer to) the mask of the global eport
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_BAD_PARAM             - on wrong min,max.
*
* COMMENTS:
*
*******************************************************************************/
/*extern*/ GT_STATUS prvCpssDxChSip6GlobalEportMaskGet
(
    IN  GT_U32  minValue,
    IN  GT_U32  maxValue,
    IN  GT_U32  *patternPtr,
    IN  GT_U32  *maskPtr
)
{
    GT_U32    ii;
    GT_U32    maxBits = 17;
    GT_U32    lastBit=0;

    if(minValue == 0 && maxValue == 0)
    {
        *patternPtr = 0;
        *maskPtr    = 0;

        return GT_OK;
    }


    /* check last bit set in max value */
    for(ii = (maxBits-1) ;  ; ii--)
    {
        if(maxValue & (1<<ii))
        {
            /* found MSBit set in max value */
            lastBit = ii;
            break;
        }

        if(ii == 0)
        {
            /* not found ? --> maxValue is 0 ? */
            break;
        }
    }

    *patternPtr  = minValue;
    /* mask of exact match on high bits of the minimum value */
    *maskPtr     =  BIT_MASK_0_31_MAC((lastBit+1)) - (maxValue - minValue);

    return GT_OK;
}

/**
* @internal sip6_cpssDxChCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] usePatternMask           - indication to use {pattern,mask} or {min,max}
*                                      GT_TRUE  - use {pattern,mask}
*                                      GT_FALSE - use {min,max}
*
* @param[out] globalPtr                - (pointer to) Global EPorts for the Bridge,L2MLL to use.
* @param[out] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
* @param[out] l2DlbPtr                 - (pointer to) L2 DLB Primary ePorts.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip6_cpssDxChCfgGlobalEportGet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         usePatternMask,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32    regAddr;  /* register address */
    GT_U32    sip6_global_minValue,sip6_global_maxValue;/* global : min and max values for sip 6 */
    GT_U32    sip6_global_minValue_1,sip6_global_maxValue_1;/* global : min and max values for sip 6 */
    GT_U32    sip6_l2Ecmp_minValue,sip6_l2Ecmp_maxValue;/* L2 ecmp : min and max values for sip 6 */
    GT_U32    sip6_l2Dlb_minValue,sip6_l2Dlb_maxValue;/* L2 dlb : min and max values for sip 6 */
    GT_U32    fieldSize;    /* register field size */
    CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT globalEPortCfgType ; /* global per eport type configuration mode */
    CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT l2ecmpEPortCfgType ; /* l2ecmp per eport type configuration mode */
    CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT l2DlbEPortCfgType ;  /* l2dlb per eport type configuration mode */
    PRV_CPSS_DXCH_PP_CONFIG_STC          *pDev; /* pp configuration structure */

    fieldSize = 17;
    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    globalEPortCfgType = pDev->ePortCfgTypeInfo.globalEPortCfgType;
    l2ecmpEPortCfgType = pDev->ePortCfgTypeInfo.l2ecmpEPortCfgType;
    l2DlbEPortCfgType  = pDev->ePortCfgTypeInfo.l2DlbEPortCfgType;

    /* read bridge global ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
        globalEportConifguration.globalEportMinValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_global_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* read bridge global ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                globalEportConifguration.globalEportMaxValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_global_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write MLL global ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.L2mllGlobalEPortMinValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_global_minValue_1);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* write MLL global ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.L2mllGlobalEPortMaxValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_global_maxValue_1);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(sip6_global_maxValue_1 != sip6_global_maxValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Bridge hold max eport[%d] but L2MLL hold [%d] \n",
            sip6_global_maxValue_1 ,
            sip6_global_maxValue);
    }

    if(sip6_global_minValue_1 != sip6_global_minValue)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Bridge hold min eport[%d] but L2MLL hold [%d] \n",
            sip6_global_minValue_1 ,
            sip6_global_minValue);
    }

    /* read EQ-ECMP ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortMinimum;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_l2Ecmp_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* read EQ-ECMP ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortMaximum;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_l2Ecmp_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Read EQ-DLB ePort range min value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbEportMinValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_l2Dlb_minValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Read EQ-DLB ePort range max value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.dlbEportMaxValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&sip6_l2Dlb_maxValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(globalEPortCfgType == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E ||
       l2ecmpEPortCfgType == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E ||
       l2DlbEPortCfgType == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E)
    {
        /* global eport range is based on min/max configuration */
        usePatternMask = GT_FALSE;
    }

    if(usePatternMask == GT_TRUE)
    {
        globalPtr->minValue = 0x0;
        globalPtr->maxValue = 0x0;
        if(sip6_global_minValue > sip6_global_maxValue)
        {
            globalPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
            globalPtr->pattern = 0x0;
            globalPtr->mask = 0x0;
        }
        else
        {
            globalPtr->enable  = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
            prvCpssDxChSip6GlobalEportMaskGet(sip6_global_minValue,sip6_global_maxValue,
                &globalPtr->pattern,
                &globalPtr->mask);
        }

        l2EcmpPtr->minValue = 0x0;
        l2EcmpPtr->maxValue = 0x0;
        if(sip6_l2Ecmp_minValue > sip6_l2Ecmp_maxValue)
        {
            l2EcmpPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
            l2EcmpPtr->pattern = 0x0;
            l2EcmpPtr->mask = 0x0;
        }
        else
        {
            l2EcmpPtr->enable  = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
            prvCpssDxChSip6GlobalEportMaskGet(sip6_l2Ecmp_minValue,sip6_l2Ecmp_maxValue,
                &l2EcmpPtr->pattern,
                &l2EcmpPtr->mask);
        }

        l2DlbPtr->minValue = 0x0;
        l2DlbPtr->maxValue = 0x0;
        if(sip6_l2Dlb_minValue > sip6_l2Dlb_maxValue)
        {
            l2DlbPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
            l2DlbPtr->pattern = 0x0;
            l2DlbPtr->mask = 0x0;
        }
        else
        {
            l2DlbPtr->enable  = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
            prvCpssDxChSip6GlobalEportMaskGet(sip6_l2Dlb_minValue,sip6_l2Dlb_maxValue,
                &l2DlbPtr->pattern,
                &l2DlbPtr->mask);
        }
    }
    else/* RANGE based global ePort config mode */
    {
        if(sip6_global_minValue > sip6_global_maxValue)
        {
            globalPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
            globalPtr->minValue = 0x0;
            globalPtr->maxValue = 0x0;
        }
        else
        {
            globalPtr->enable  = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
            globalPtr->minValue = sip6_global_minValue;
            globalPtr->maxValue = sip6_global_maxValue;
        }

        if(sip6_l2Ecmp_minValue > sip6_l2Ecmp_maxValue)
        {
            l2EcmpPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
            l2EcmpPtr->minValue = 0x0;
            l2EcmpPtr->maxValue = 0x0;
        }
        else
        {
            l2EcmpPtr->enable  = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
            l2EcmpPtr->minValue = sip6_l2Ecmp_minValue;
            l2EcmpPtr->maxValue = sip6_l2Ecmp_maxValue;
        }

        if(sip6_l2Dlb_minValue > sip6_l2Dlb_maxValue)
        {
            l2DlbPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
            l2DlbPtr->minValue = 0x0;
            l2DlbPtr->maxValue = 0x0;
        }
        else
        {
            l2DlbPtr->enable  = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
            l2DlbPtr->minValue  = sip6_l2Dlb_minValue;
            l2DlbPtr->maxValue  = sip6_l2Dlb_maxValue;
        }

        globalPtr->pattern = 0x0;
        globalPtr->mask = 0x0;
        l2EcmpPtr->pattern = 0x0;
        l2EcmpPtr->mask = 0x0;
        l2DlbPtr->pattern  = 0x0;
        l2DlbPtr->mask  = 0x0;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] globalPtr                - (pointer to) Global EPorts for the Bridge,L2MLL to use.
* @param[out] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
* @param[out] l2DlbPtr                 - (pointer to) L2 DLB Primary ePorts.
*                                        (APPLICABLE DEVICE: Falcon)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgGlobalEportGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
    GT_STATUS rc;       /* return code */
    GT_U32    regAddr;  /* register address */
    GT_U32    pattern, mask; /* sip5 : Global ePort registers values */
    GT_U32    l2MllPattern, l2MllMask; /* sip5 : Global ePort registers values */
    GT_U32    fieldSize;    /* register field size */
    CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT       l2MllEnable;  /* L2MLL logic for value & mask */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(l2EcmpPtr);
    CPSS_NULL_PTR_CHECK_MAC(globalPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(l2DlbPtr);

        return sip6_cpssDxChCfgGlobalEportGet(devNum,
            GT_TRUE/* BWC mode : use {pattern,mask} */,
            globalPtr,
            l2EcmpPtr,
            l2DlbPtr);
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        fieldSize = 15;
    }
    else
    {
        fieldSize = 13;
    }

    /* read ECMP global ePort range value */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&pattern);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* read ECMP global ePort range mask */
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).L2ECMP.ePortECMPEPortMask;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&mask);
    if( GT_OK != rc )
    {
        return rc;
    }

    if ((pattern & mask) != pattern)
    {
        l2EcmpPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
        l2EcmpPtr->pattern = 0x0;
        l2EcmpPtr->mask = 0x0;
    }
    else
    {
        l2EcmpPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
        l2EcmpPtr->pattern = pattern;
        l2EcmpPtr->mask = mask;
    }
    l2EcmpPtr->minValue = 0x0;
    l2EcmpPtr->maxValue = 0x0;

    /* read bridge global ePort range value */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                globalEportConifguration.globalEPortValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&pattern);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* read bridge global ePort range mask */
    regAddr = PRV_DXCH_REG1_UNIT_L2I_MAC(devNum).
                globalEportConifguration.globalEPortMask;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,fieldSize ,&mask);
    if( GT_OK != rc )
    {
        return rc;
    }

    if ((pattern & mask) != pattern)
    {
        globalPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
        globalPtr->pattern = 0x0;
        globalPtr->mask = 0x0;
    }
    else
    {
        globalPtr->enable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
        globalPtr->pattern = pattern;
        globalPtr->mask = mask;
    }
    globalPtr->minValue = 0x0;
    globalPtr->maxValue = 0x0;

    /* read L2 MLL global ePort range value */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.globalEPortRangeConfig;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,20 ,&l2MllPattern);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* read L2 MLL global ePort range mask */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.globalEPortRangeMask;
    rc = prvCpssHwPpGetRegField(devNum, regAddr ,0 ,20 ,&l2MllMask);
    if( GT_OK != rc )
    {
        return rc;
    }

    if ((l2MllPattern & l2MllMask) != l2MllPattern)
    {
        l2MllEnable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;
    }
    else
    {
        l2MllEnable = CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E;
    }

    /* compare register values from bridge and from L2Mll */
    if( (l2MllPattern != pattern) ||
        (l2MllMask != mask) )
    {
        /* compare if both MLL and L2I are disabled - good result */
        if ( (globalPtr->enable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E) && (l2MllEnable == CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E) )
        {
            /* return the L2 global pattern */
            return GT_OK;
        }
        else
        {
            /* bridge and L2Mll are not synchronized */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChCfgGlobalEportGet function
* @endinternal
*
* @brief   Get Global ePorts ranges configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] globalPtr                - (pointer to) Global EPorts for the Bridge,L2MLL to use.
* @param[out] l2EcmpPtr                - (pointer to) L2 ECMP Primary ePorts.
* @param[out] l2DlbPtr                 - (pointer to) L2 DLB  Primary ePorts.
*                                        (APPLICABLE DEVICE: Falcon)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - failed to write to hardware
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgGlobalEportGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *globalPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2EcmpPtr,
    OUT CPSS_DXCH_CFG_GLOBAL_EPORT_STC  *l2DlbPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgGlobalEportGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalPtr, l2EcmpPtr, l2DlbPtr));

    rc = internal_cpssDxChCfgGlobalEportGet(devNum, globalPtr, l2EcmpPtr, l2DlbPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalPtr, l2EcmpPtr, l2DlbPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChCfgHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cfgAccessObjPtr          - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - unbind callback routines.
*
* @retval GT_OK                    - on success
*/
static GT_STATUS internal_cpssDxChCfgHwAccessObjectBind
(
    IN CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC *cfgAccessObjPtr,
    IN GT_BOOL                     bind
)
{
    if(bind)
        cpssOsMemCpy( &PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT, cfgAccessObjPtr, sizeof(CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC));
    else
        cpssOsMemSet( &PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT, 0, sizeof(CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC));

    return GT_OK;
}

/**
* @internal cpssDxChCfgHwAccessObjectBind function
* @endinternal
*
* @brief   The function binds/unbinds a callback routines for HW access.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cfgAccessObjPtr          - HW access object pointer.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - unbind callback routines.
*
* @retval GT_OK                    - on success
*/
GT_STATUS cpssDxChCfgHwAccessObjectBind
(
    IN CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC *cfgAccessObjPtr,
    IN GT_BOOL                     bind
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgHwAccessObjectBind);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, cfgAccessObjPtr, bind));

    rc = internal_cpssDxChCfgHwAccessObjectBind(cfgAccessObjPtr, bind);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, cfgAccessObjPtr, bind));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write methood.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  hitless write methood is enable. In this case write operation
*                                      would be performed only if writing data is not equal with
*                                      affected memory contents.
*                                      GT_FALSE: hitless write methood is disable. In this case write operation
*                                      is done at any rate.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgHitlessWriteMethodEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable = enable;
    return GT_OK;
}

/**
* @internal cpssDxChCfgHitlessWriteMethodEnableSet function
* @endinternal
*
* @brief   Enable/disable hitless write methood.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE:  hitless write methood is enable. In this case write operation
*                                      would be performed only if writing data is not equal with
*                                      affected memory contents.
*                                      GT_FALSE: hitless write methood is disable. In this case write operation
*                                      is done at any rate.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgHitlessWriteMethodEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_BOOL                             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgHitlessWriteMethodEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChCfgHitlessWriteMethodEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgPortRemotePhyMacBind function
* @endinternal
*
* @brief   Creation and Bind PHY-MAC device (like: PHY '88E1690') to the DX device
*         on specific portNum.
*         NOTE:
*         The portNum should be the 'physical port num' of the DX port connection
*         to the PHY-MAC device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] infoPtr                  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgPortRemotePhyMacBind
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr
)
{

    switch(infoPtr->phyMacType)
    {
        case CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E:
            return prvCpssDxChCfgPort88e1690Bind(devNum,portNum,infoPtr);
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

}

/**
* @internal cpssDxChCfgPortRemotePhyMacBind function
* @endinternal
*
* @brief   Creation and Bind PHY-MAC device (like: PHY '88E1690') to the DX device
*         on specific portNum.
*         NOTE:
*         The portNum should be the 'physical port num' of the DX port connection
*         to the PHY-MAC device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] infoPtr                  - (pointer to) The needed info about the remote device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgPortRemotePhyMacBind
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC   *infoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgPortRemotePhyMacBind);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum , infoPtr));

    rc = internal_cpssDxChCfgPortRemotePhyMacBind(devNum, portNum , infoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum , infoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgRemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] modeFcHol                - CPSS_DXCH_PORT_FC_E  : set Flow Control mode
*                                      CPSS_DXCH_PORT_HOL_E : set HOL system mode
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internal_cpssDxChCfgRemoteFcModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                              tcBitmap
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM                portNum;
    PRV_CPSS_DXCH_PORT_INFO_STC                 *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC  *remotePhyMacInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_CAELUM_E);

    /* Walk over all remote cascading ports to configure according to new mode. */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    for(portNum = 0 ; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[portNum];
        if(remotePhyMacInfoPtr == NULL)
        {
            continue;
        }
        switch(remotePhyMacInfoPtr->connectedPhyMacInfo.phyMacType)
        {
            case CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E:
                rc = prvCpssDxChCfg88e1690RemoteFcModeSet(devNum,
                                                            portNum,
                                                            modeFcHol,
                                                            profileSet,
                                                            tcBitmap);
                if (rc != GT_OK)
                {
                    return rc;
                }
                break;
            default:
                break;
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChCfgRemoteFcModeSet function
* @endinternal
*
* @brief   Set Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   -device number
* @param[in] modeFcHol                - CPSS_DXCH_PORT_FC_E  : set Flow Control mode
*                                      CPSS_DXCH_PORT_HOL_E : set HOL system mode
* @param[in] profileSet               - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[in] tcBitmap                 - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                      bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChCfgRemoteFcModeSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PORT_HOL_FC_ENT           modeFcHol,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                              tcBitmap
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgRemoteFcModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modeFcHol, profileSet, tcBitmap));

    rc = internal_cpssDxChCfgRemoteFcModeSet(devNum, modeFcHol, profileSet, tcBitmap);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modeFcHol, profileSet, tcBitmap));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgRemoteFcModeGet function
* @endinternal
*
* @brief   Get Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[out] profileSet           - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[out] tcBitmap             - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                    bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
* @param[out] modeFcHolPtr         - (Pointer to) Flow Control or HOL Mode.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS internal_cpssDxChCfgRemoteFcModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PORT_HOL_FC_ENT          *modeFcHolPtr,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT  *profileSetPtr,
    OUT GT_U32                             *tcBitmapPtr
)
{
    GT_STATUS                                   rc;
    GT_PHYSICAL_PORT_NUM                        portNum;
    PRV_CPSS_DXCH_PORT_INFO_STC                *portPtr;
    PRV_CPSS_DXCH_PORT_REMOTE_PHY_MAC_INFO_STC *remotePhyMacInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(modeFcHolPtr);
    CPSS_NULL_PTR_CHECK_MAC(profileSetPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcBitmapPtr);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_CAELUM_E);

    /* Walk over all remote cascading ports to configure according to new mode. */
    portPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->port;
    for(portNum = 0 ; portNum < PRV_CPSS_MAX_MAC_PORTS_NUM_CNS; portNum++)
    {
        remotePhyMacInfoPtr = portPtr->remotePhyMacInfoArr[portNum];
        if(remotePhyMacInfoPtr == NULL)
        {
            continue;
        }
        switch(remotePhyMacInfoPtr->connectedPhyMacInfo.phyMacType)
        {
            case CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E:
                rc = prvCpssDxChCfg88e1690RemoteFcModeGet(devNum, portNum,
                                           profileSetPtr, tcBitmapPtr, modeFcHolPtr);
                return rc;
            default:
                break;
        }
    }

    /* No remote ports configured, return GT_NOT_INITIALIZED */
    if(portNum == PRV_CPSS_MAX_MAC_PORTS_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssDxChCfgRemoteFcModeGet function
* @endinternal
*
* @brief   Get Flow Control or HOL system mode for Remote ports on the specified device.
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[out] profileSet           - the associated Drop Profile Set (Relevant for CPSS_DXCH_PORT_FC_E mode).
* @param[out] tcBitmap             - bit map of CN Aware TCs (Relevant for CPSS_DXCH_PORT_FC_E mode):
*                                    bit#n set if traffic class #n is CN aware, bit#n is clear otherwise.
* @param[out] modeFcHolPtr         - (Pointer to) Flow Control or HOL Mode.
*
* @retval GT_OK                    - on success,
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDxChCfgRemoteFcModeGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PORT_HOL_FC_ENT          *modeFcHolPtr,
    OUT CPSS_PORT_TX_DROP_PROFILE_SET_ENT  *profileSetPtr,
    OUT GT_U32                             *tcBitmapPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgRemoteFcModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, modeFcHolPtr, profileSetPtr, tcBitmapPtr));

    rc = internal_cpssDxChCfgRemoteFcModeGet(devNum, modeFcHolPtr, profileSetPtr, tcBitmapPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, modeFcHolPtr, profileSetPtr, tcBitmapPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sharedTablesIllegalClientAccessInfoGet_PerTileIdPerTable function
* @endinternal
*
* @brief   a debug API to check and get errors caused in the shared memory engine ,
*          by wrong access to it from one or more of it's clients : LPM , LPM aging ,
*          FDB , EM , ARP.
*          NOTEs:
*           if the function     found 'illegal access' it will return GT_OK and the info will be in (*clientInfoPtr)
*           if the function not found 'illegal access' it will return GT_NO_MORE.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum               - device number
* @param[in]  tileId               - the tile Id to check
* @param[in]  tableToServe         - the table to check
* @param[out] clientInfoPtr        - (pointer to) the client info to retrieve.
*
* @retval GT_OK                    - on success , and there is 'illegal' info to retrieve.
* @retval GT_NO_MORE               - on success , but there are no more 'illegal' info to retrieve.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sharedTablesIllegalClientAccessInfoGet_PerTileIdPerTable
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              tileId,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT tableToServe,
    OUT CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC *clientInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    GT_U32  regAddr,regValue;
    GT_U32  portGroupId;
    GT_U32  localPipeIdInTile;
    GT_U32  bankIndex,lineIndex;


    if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
    {
        PRINT_PARAM_MAC(tileId);
        PRINT_PARAM_MAC(tableToServe);
    }

    switch(tableToServe)
    {
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E        :
            regAddr = regsAddrPtr->SHM.lpm_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E         :
            regAddr = regsAddrPtr->SHM.em_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_FDB_E        :
            regAddr = regsAddrPtr->SHM.fdb_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ARP_E        :
            regAddr = regsAddrPtr->SHM.arp_illegal_address;
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E  :
            regAddr = regsAddrPtr->SHM.lpm_aging_illegal_address;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(tableToServe);
    }

    /* representative portGroupId */
    portGroupId = tileId * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

    /* read register value */
    rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId , regAddr ,&regValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    bankIndex = 0;

    switch(tableToServe)
    {
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E        :
            /*<valid>*/
            if(0 == (regValue & BIT_0))
            {
                return /* not error to the log */ GT_NO_MORE;
            }

            /*<bank>*/
            bankIndex = U32_GET_FIELD_MAC(regValue , 4 ,5);

            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E         :
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_FDB_E        :
            /*<ch_a_valid >*/
            if((0 == (regValue & BIT_0)) &&
            /*<ch_b_valid >*/
               (0 == (regValue & BIT_4)))
            {
                return /* not error to the log */ GT_NO_MORE;
            }

            /*MHT*/
            bankIndex = U32_GET_FIELD_MAC(regValue , 8 , 5);
            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ARP_E        :
            /*<ch_a_valid >*/
            if((0 == (regValue & BIT_0)) &&
            /*<ch_b_valid >*/
               (0 == (regValue & BIT_4)))
            {
                return /* not error to the log */ GT_NO_MORE;
            }

            break;
        case CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E  :
            /*<valid>*/
            if(0 == (regValue & BIT_0))
            {
                return /* not error to the log */ GT_NO_MORE;
            }

            break;
        default:
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(tableToServe);
    }

    /*****************************/
    /* we have valid info to use */
    /*****************************/

    /*<pipe>*/
    localPipeIdInTile = U32_GET_FIELD_MAC(regValue , 12 ,1);
    /*<address>*/
    lineIndex = U32_GET_FIELD_MAC(regValue , 16 ,16);

    if((1<<tileId) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        localPipeIdInTile = (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile - 1) -
            localPipeIdInTile;
    }

    clientInfoPtr->portGroupId = portGroupId + localPipeIdInTile;
    clientInfoPtr->clientId    = tableToServe;
    clientInfoPtr->bankIndex   = bankIndex;
    clientInfoPtr->lineIndex   = lineIndex;

    if(CONFIG_GLOBAL_VAR_GET(debug_sharedTables))
    {
        PRINT_PARAM_MAC(clientInfoPtr->portGroupId);
        PRINT_PARAM_MAC(clientInfoPtr->clientId);
        PRINT_PARAM_MAC(clientInfoPtr->bankIndex);
        PRINT_PARAM_MAC(clientInfoPtr->lineIndex);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChCfgSharedTablesIllegalClientAccessInfoGet function
* @endinternal
*
* @brief   a debug API to check and get errors caused in the shared memory engine ,
*          by wrong access to it from one or more of it's clients : LPM , LPM aging ,
*          FDB , EM , ARP.
*          NOTEs:
*           if the function     found 'illegal access' it will return GT_OK and the info will be in (*clientInfoPtr)
*           if the function not found 'illegal access' it will return GT_NO_MORE.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in]  devNum               - device number
* @param[out] clientInfoPtr        - (pointer to) the client info to retrieve.
*
* @retval GT_OK                    - on success , and there is 'illegal' info to retrieve.
* @retval GT_NO_MORE               - on success , but there are no more 'illegal' info to retrieve.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgSharedTablesIllegalClientAccessInfoGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC        *clientInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32    portGroupId;    /* the port group Id - support multi-port-groups device  */
    GT_U32    origPortGroupId;/* the original port group that we started with */
    GT_U32    tileId;         /* the tile Id - support multi-tiles device  */
    GT_U32    origTileId;     /* the tile Id - support multi-tiles device  */
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT tableToServe;
    CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT origTableToServe;
    GT_BOOL   infoFound;/* indication that there is info about 'illegal access' */
    GT_BOOL   origPipeFirstTime = GT_TRUE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E |
        CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(clientInfoPtr);

    /* The AC5X not hold shared memory ... so the API not relevant */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.sip6_sbmInfo.sharedMemoryNotUsed)/* needed because CPSS_AC5X_E is not used yet */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "The device hold no shared tables");
    }

    origPortGroupId  = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextPortGroupToServe;
    origTableToServe = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextTableToServe;

    portGroupId = origPortGroupId;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile)
    {
        origTileId = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
    }
    else
    {
        origTileId = 0;
    }

    tileId       = origTileId;
    tableToServe = origTableToServe;

    do {

        loopTablesInTile_lbl:
        do{
            /* check if the table in current tile hold error */
            rc = sharedTablesIllegalClientAccessInfoGet_PerTileIdPerTable(
                devNum,tileId,tableToServe,clientInfoPtr);

            if(rc == GT_OK)
            {
                infoFound = GT_TRUE;
            }
            else
            if(rc == GT_NO_MORE)
            {
                infoFound = GT_FALSE;
            }
            else/* error */
            {
                return rc;
            }

            tableToServe++;
            if(tableToServe == CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS___LAST___E)
            {
                if(origPipeFirstTime == GT_FALSE || infoFound == GT_TRUE)
                {
                    tableToServe = 0;
                }
            }

            PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextTableToServe = tableToServe;

            /* NOTE : at this point the
                PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextPortGroupToServe
                is also with proper value */

            if(infoFound == GT_TRUE)
            {
                return GT_OK;
            }

            /* next can happen when origPipeFirstTime == GT_TRUE */
            if(tableToServe == CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS___LAST___E)
            {
                /* we are done with the tables at the 'orig tile' */
                /* we need to look at tables in other tiles ... and only if all
                   of them not hold valid info ... we will start with tables at
                   'orig tile' (till 'orig table') */
                break;
            }

            if(tileId == origTileId && tableToServe == origTableToServe)
            {
                break;
            }


        }while(1);  /* loop on tables in current tile */

        if(origPipeFirstTime == GT_FALSE)
        {
            break;
        }


        /* NOTE: if we are here ... info was NOT FOUND in current tile ... need to jump to next tile */

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile)
        {
            /* jump to next tile */
            portGroupId = (tileId + 1) * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
        }

        if(portGroupId > PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.lastActivePortGroup )
        {
            portGroupId = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.firstActivePortGroup;
        }

        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextPortGroupToServe = portGroupId;
        /*rest the start table , bacause hold meaning only in the first 'tables iteration' (in first tile to serve) */
        PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.sharedTablesIllegalAccess.nextTableToServe     = 0;
        tableToServe = 0;/*rest the start table*/

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile)
        {
            tileId  = portGroupId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;
        }


        if(tileId == origTileId && origPipeFirstTime == GT_TRUE)
        {
            /* we need to return to the orig tile and look in the tables till the orig table */
            origPipeFirstTime = GT_FALSE;
            goto loopTablesInTile_lbl;
        }

    }while (tileId != origTileId);  /* loop on tiles */

    return /* not error to the 'error log' */ GT_NO_MORE;
}
/**
* @internal cpssDxChCfgSharedTablesIllegalClientAccessInfoGet function
* @endinternal
*
* @brief   a debug API to check and get errors caused in the shared memory engine ,
*          by wrong access to it from one or more of it's clients : LPM , LPM aging ,
*          FDB , EM , ARP.
*          NOTEs:
*           if the function     found 'illegal access' it will return GT_OK and the info will be in (*clientInfoPtr)
*           if the function not found 'illegal access' it will return GT_NO_MORE.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in]  devNum               - device number
* @param[out] clientInfoPtr        - (pointer to) the client info to retrieve.
*
* @retval GT_OK                    - on success , and there is 'illegal' info to retrieve.
* @retval GT_NO_MORE               - on success , but there are no more 'illegal' info to retrieve.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChCfgSharedTablesIllegalClientAccessInfoGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC        *clientInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgSharedTablesIllegalClientAccessInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, clientInfoPtr));

    rc = internal_cpssDxChCfgSharedTablesIllegalClientAccessInfoGet(devNum, clientInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, clientInfoPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgReplicationCountersGet function
* @endinternal
*
* @brief   Get the value of Replication engine counters.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum                 - device number
* @param[in] counterType            - type of replication counter
* @param[out] counterPtr            - (pointer to) value of counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
static GT_STATUS internal_cpssDxChCfgReplicationCountersGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT   counterType,
    OUT GT_U32                                        *counterPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    cpssOsMemSet(counterPtr, 0, sizeof(*counterPtr));

    switch (counterType)
    {
        case CPSS_DXCH_CFG_QCN_REPLICATION_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.qcnOutgoingCounter;
            break;
        case CPSS_DXCH_CFG_SNIFF_REPLICATION_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.sniffOutgoingCounter;
            break;
        case CPSS_DXCH_CFG_TRAP_REPLICATION_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.trapOutgoingCounter;
            break;
        case CPSS_DXCH_CFG_MIRROR_REPLICATION_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.mirrorOutgoingCounter;
            break;
        case CPSS_DXCH_CFG_OUTGOING_FORWARD_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.hbuPacketsOutgoingForwardCounter;
            break;
        case CPSS_DXCH_CFG_QCN_FIFO_DROP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.qcnFifoFullDroppedPacketsCounter;
            break;
        case CPSS_DXCH_CFG_SNIFF_FIFO_DROP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.sniffFifoFullDroppedPacketsCounter;
            break;
        case CPSS_DXCH_CFG_TRAP_FIFO_DROP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.trapFifoFullDroppedPacketsCounter;
            break;
        case CPSS_DXCH_CFG_MIRROR_FIFO_DROP_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.counters.mirrorFifoFullDroppedPacketsCounter;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPortGroupsCounterSummary(
        devNum, regAddr,
        0 /*fieldOffset*/, 32 /*fieldLength*/,
        counterPtr, NULL /*counter64bitValuePtr*/);

    return rc;
}

/**
* @internal cpssDxChCfgReplicationCountersGet function
* @endinternal
*
* @brief   Get the value of Replication engine counters.
*
* @note   APPLICABLE DEVICES:     Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] counterType              - type of replication counter
* @param[out] counterPtr              - (pointer to) value of counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
GT_STATUS cpssDxChCfgReplicationCountersGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT   counterType,
    OUT GT_U32                                        *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgReplicationCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, counterPtr));

    rc = internal_cpssDxChCfgReplicationCountersGet(devNum, counterType, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgProbePacketConfigSet function
*
* @brief Set the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] probeCfgPtr           - (Pointer to)Probe Packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChCfgProbePacketConfigSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_CFG_PROBE_PACKET_STC *probeCfgPtr
)
{
    GT_STATUS rc;               /* return code                  */
    GT_U32    regAddr;          /* register address             */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(probeCfgPtr);

    if( probeCfgPtr->bitLocation > 11)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EQ.ingrDropCodeLatching.ingrDropCodeLatching;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,1,BOOL2BIT_MAC(probeCfgPtr->enable));

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpSetRegField(devNum,regAddr,1,4,probeCfgPtr->bitLocation);

    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.dropCodeConfigs;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,0,1,BOOL2BIT_MAC(probeCfgPtr->enable));

    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssHwPpSetRegField(devNum,regAddr,1,4,probeCfgPtr->bitLocation);

    return rc;
}

/**
* @internal cpssDxChCfgProbePacketConfigSet function
*
* @brief Set the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] probeCfgPtr           - (Pointer to)Probe Packet configuration.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCfgProbePacketConfigSet
(
    IN GT_U8                          devNum,
    IN CPSS_DXCH_CFG_PROBE_PACKET_STC *probeCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgProbePacketConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, probeCfgPtr));

    rc = internal_cpssDxChCfgProbePacketConfigSet(devNum, probeCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, probeCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgProbePacketConfigGet function
*
* @brief Get the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number.
* @param[out] probeCfgPtr          - (pointer to) Probe packet configuration
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCfgProbePacketConfigGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_CFG_PROBE_PACKET_STC  *probeCfgPtr
)
{
    GT_STATUS rc;
    GT_U32      regAddr;
    GT_U32      hwData;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(probeCfgPtr);

    /* Since both ingresss and egress have the same configuration, we use ingress register */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EQ.ingrDropCodeLatching.ingrDropCodeLatching;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwData);

    if (rc != GT_OK)
    {
        return rc;
    }

    probeCfgPtr->enable = BIT2BOOL_MAC(hwData);

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 4, &probeCfgPtr->bitLocation);

    return rc;
}

/**
* @internal cpssDxChCfgProbePacketConfigGet function
*
* @brief Get the Probe packet configuration
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number.
* @param[in] stage                 - ingress/egress stage.
* @param[out] probeCfgPtr          - (pointer to) Probe Packet Configuration
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCfgProbePacketConfigGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_CFG_PROBE_PACKET_STC  *probeCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,cpssDxChCfgProbePacketConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, probeCfgPtr));

    rc = internal_cpssDxChCfgProbePacketConfigGet(devNum,  probeCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, probeCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
*
* @internal internal_cpssDxChCfgProbePacketDropCodeGet function
*
* @brief Get the last probe packet Drop Code
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number
* @param[in] direction             - ingress/egress direction
* @param[out] dropCodePtr          - (pointer to) last probe packet drop code
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCfgProbePacketDropCodeGet
(
    IN GT_U8                     devNum,
    IN CPSS_DIRECTION_ENT        direction,
    OUT CPSS_NET_RX_CPU_CODE_ENT *dropCodePtr
)
{
    GT_STATUS                         rc;
    GT_U32                            regAddr;
    GT_U32                            portGroupId;
    GT_PORT_GROUPS_BMP                portGroupsBmp;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode = PRV_CPSS_DXCH_NET_DSA_TAG_MAX_E;
    GT_U32                            hw_dsaCpuCode = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(dropCodePtr);

    switch (direction)
    {
        case CPSS_DIRECTION_INGRESS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EQ.ingrDropCodeLatching.lastDropCode;
            break;
        case CPSS_DIRECTION_EGRESS_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EREP.configurations.dropCodeStatus;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    portGroupId = 0;
    /* loop on all port groups to get the drop code */
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)
    {
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 0, 8, &hw_dsaCpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (hw_dsaCpuCode)
            break;
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
        devNum, portGroupsBmp, portGroupId)

    dsaCpuCode = hw_dsaCpuCode;

    return prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,dropCodePtr);
}

/**
*
* @internal cpssDxChCfgProbePacketDropCodeGet function
*
* @brief Get the last probe packet Drop Code
*
* @note APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device Number
* @param[in] direction             - ingress/egress direction
* @param[out] dropCodePtr          - (pointer to) last probe packet drop code
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PTR               - one of the parameter is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChCfgProbePacketDropCodeGet
(
    IN GT_U8                     devNum,
    IN CPSS_DIRECTION_ENT        direction,
    OUT CPSS_NET_RX_CPU_CODE_ENT *dropCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,cpssDxChCfgProbePacketDropCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, direction, dropCodePtr));

    rc = internal_cpssDxChCfgProbePacketDropCodeGet(devNum, direction, dropCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, direction, dropCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgDevDbInfoGet function
* @endinternal
*
* @brief   function to get the init parameters given during initialization.
*         it is needed for application that need to do re-init without HW reset ,
*         and want to use same init parameters, that relate to DMAs .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_SUPPORTED         - device not hold DMA info
* @retval GT_NOT_INITIALIZED       - device was not yet initialized with DMA info
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
static GT_STATUS internal_cpssDxChCfgDevDbInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_IF_CFG_STC        **prevCpssInit_netIfCfgPtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_auqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_fuqUseSeparatePtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_fuqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_useMultiNetIfSdmaPtrPtr,
    OUT CPSS_MULTI_NET_IF_CFG_STC  **prevCpssInit_multiNetIfCfgPtrPtr
)
{
    GT_STATUS   rc;

    /* validate the device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* fill in generic part of device info */
    rc = prvCpssPpConfigDevDbInfoGet(devNum,
        prevCpssInit_netIfCfgPtrPtr,
        prevCpssInit_auqCfgPtrPtr,
        prevCpssInit_fuqUseSeparatePtrPtr,
        prevCpssInit_fuqCfgPtrPtr,
        prevCpssInit_useMultiNetIfSdmaPtrPtr,
        prevCpssInit_multiNetIfCfgPtrPtr);

    return rc;
}

/**
* @internal cpssDxChCfgDevDbInfoGet function
* @endinternal
*
* @brief   function to get the init parameters given during initialization.
*         it is needed for application that need to do re-init without HW reset ,
*         and want to use same init parameters, that relate to DMAs .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_SUPPORTED         - device not hold DMA info
* @retval GT_NOT_INITIALIZED       - device was not yet initialized with DMA info
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS cpssDxChCfgDevDbInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_IF_CFG_STC        **prevCpssInit_netIfCfgPtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_auqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_fuqUseSeparatePtrPtr,
    OUT CPSS_AUQ_CFG_STC           **prevCpssInit_fuqCfgPtrPtr,
    OUT GT_BOOL                    **prevCpssInit_useMultiNetIfSdmaPtrPtr,
    OUT CPSS_MULTI_NET_IF_CFG_STC  **prevCpssInit_multiNetIfCfgPtrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgDevDbInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, prevCpssInit_netIfCfgPtrPtr,
                            prevCpssInit_auqCfgPtrPtr, prevCpssInit_fuqUseSeparatePtrPtr, prevCpssInit_fuqCfgPtrPtr,
                            prevCpssInit_useMultiNetIfSdmaPtrPtr, prevCpssInit_multiNetIfCfgPtrPtr));

    rc = internal_cpssDxChCfgDevDbInfoGet(devNum,
                                          prevCpssInit_netIfCfgPtrPtr,
                                          prevCpssInit_auqCfgPtrPtr,
                                          prevCpssInit_fuqUseSeparatePtrPtr,
                                          prevCpssInit_fuqCfgPtrPtr,
                                          prevCpssInit_useMultiNetIfSdmaPtrPtr,
                                          prevCpssInit_multiNetIfCfgPtrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgIngressDropEnableSet function
* @endinternal
*
* @brief   Enable/disable packet drop in ingress processing pipe
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - The device number.
* @param[in] pktDropEnable         -  Enable/disable packet drop in ingress processing pipe
*                                     GT_TRUE - drop packet in ingress pipe, egress
*                                               pipe does not get the dropped packets
*                                     GT_FALSE- pass dropped packets to egress
*                                               pipe, drop done in egress pipe
*
*Note: In egress filter, only the ingress drops are forwarded.
*      The drops in egress filtering logic is dropped in egress filter itself.
*
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - wrong device Number
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChCfgIngressDropEnableSet
(
    IN  GT_U8         devNum,
    IN  GT_BOOL       pktDropEnable
)
{

    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E| CPSS_BOBCAT2_E | CPSS_CAELUM_E|
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 1, BOOL2BIT_MAC(pktDropEnable));
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersGlobal;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 16, 1, BOOL2BIT_MAC(pktDropEnable));

    return rc;
}
/**
* @internal cpssDxChCfgIngressDropEnableSet function
* @endinternal
*
* @brief   Enable/disable packet drop in ingress processing pipe
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - The device number.
* @param[in] pktDropEnable         -  Enable/disable packet drop in ingress processing pipe
*                                     GT_TRUE - drop packet in ingress pipe, egress
*                                               pipe does not get the dropped packets
*                                     GT_FALSE- pass dropped packets to egress
*                                               pipe, drop done in egress pipe
*
*Note: In egress filter, only the ingress drops are forwarded.
*      The drops in egress filtering logic is dropped in egress filter itself.
*
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - wrong device Number
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS cpssDxChCfgIngressDropEnableSet
(
    IN  GT_U8         devNum,
    IN  GT_BOOL       pktDropEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgIngressDropEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pktDropEnable));

    rc = internal_cpssDxChCfgIngressDropEnableSet(devNum, pktDropEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pktDropEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChCfgIngressDropEnableGet function
* @endinternal
*
* @brief   Get packet drop status in ingress processing pipe
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 -  The device number.
*
* @param[out] pktDropEnablePtr      -  (pointer to) the enabling status of drop in ingress processing pipe
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - wrong device Number
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_PTR                - one of the pointers is NULL
*/
static GT_STATUS internal_cpssDxChCfgIngressDropEnableGet
(
    IN   GT_U8          devNum,
    OUT  GT_BOOL        *pktDropEnablePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr;
    GT_U32      eqDropEnable, eftDropEnable;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E| CPSS_BOBCAT2_E | CPSS_CAELUM_E|
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    CPSS_NULL_PTR_CHECK_MAC(pktDropEnablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.preEgrEngineGlobalConfig2;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 1, &eqDropEnable);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(devNum).egrFilterConfigs.egrFiltersGlobal;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 16, 1, &eftDropEnable);
    if(rc != GT_OK)
    {
        return rc;
    }

    *pktDropEnablePtr = BIT2BOOL_MAC(eqDropEnable && eftDropEnable);

    return rc;
}

/**
* @internal cpssDxChCfgIngressDropEnableGet function
* @endinternal
*
* @brief   Get packet drop status in ingress processing pipe
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                 -  The device number.
*
* @param[out] pktDropEnablePtr      -  (pointer to) the enabling status of drop in ingress processing pipe
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - wrong device Number
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_PTR                - one of the pointers is NULL
*/
GT_STATUS cpssDxChCfgIngressDropEnableGet
(
    IN   GT_U8          devNum,
    OUT  GT_BOOL        *pktDropEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChCfgIngressDropEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pktDropEnablePtr));

    rc = internal_cpssDxChCfgIngressDropEnableGet(devNum, pktDropEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pktDropEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*
    temp function to allow test prvTgfBrgSrcIdScalableSgt to set ' SRC_ID+ desc<copy_reserved> mode'
    to get BWC. although the Ironman should work in the new EPG mode.
*/
GT_STATUS internal_cpssDxChCfgEpclTrgPortModeSet
(
    IN   GT_U8          devNum,
    OUT  GT_U32         mode/* 0 = EPG mode , 1 = SRC_ID+ desc<copy_reserved> mode */
)
{
    GT_STATUS                      rc;
    GT_U32                         hwValue;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E| CPSS_BOBCAT2_E | CPSS_CAELUM_E|
                                          CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E |
                                          CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E);
    if(mode > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mode[%d] must be 0 (EPG mode) or 1 (SRC_ID+ desc<copy_reserved> mode) ",
            mode);
    }

    hwValue = mode;
    /* sip6.30 */
    rc = prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_EPCL_MAC(devNum).egrPolicyGlobalConfig,
        25, 1, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    hwValue = (mode == 0) ? 3 /*default of IM-L */ : 0;
    /* <sgt_trg_eport_msb_ignore_len> */
    rc = prvCpssHwPpSetRegField(devNum,
        PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).preEgrEngineGlobalConfig.scalableSgtSupport,
        10, 3, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

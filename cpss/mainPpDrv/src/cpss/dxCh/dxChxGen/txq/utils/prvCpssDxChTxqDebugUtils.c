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
* @file prvCpssDxChTxqDebugUtils.c
*
* @brief CPSS SIP6 TXQ debug functions
*
* @version   1
********************************************************************************
*/


/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS

#define MAX_PORTS_PER_LINE 9

#define ITERATION_STOP(_iteration,_devNum) (MAX_PORTS_PER_LINE*(_iteration+1)>CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(_devNum))?\
    CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(_devNum):(MAX_PORTS_PER_LINE*(_iteration+1))

#define ITERATION_START(_iteration) (MAX_PORTS_PER_LINE*_iteration)

/*_1K  counters per block ,each queue need 2 counters.So 512 queues per block*/

#define MAX_QUEUES_PER_CNC_BLOCK 512
#define PRV_TXQ_LOG_STRING_BUFFER_SIZE_CNS       1024
#define PRV_TXQ_LOG_STRING_ALLOWED_SIZE_CNS      (PRV_TXQ_LOG_STRING_BUFFER_SIZE_CNS-64)



#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPfcc.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqFcGopUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPsi.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdx.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/pipe/prvCpssAc5pTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/pipe/prvCpssAc5xTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/pipe/prvCpssHarrierTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPds.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedEligPrioFunc.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSched.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqDebugUtils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqTailDropUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define PRV_NON_SHARED_TXQ_DIR_TXQ_DBG_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.txqDir.txqDbgSrc._var,_value)

#define PRV_NON_SHARED_TXQ_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.txqDir.txqDbgSrc._var)

#define PRV_TXQ_CIDER_INFO_GET()\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.txqDbgDir.txqDbgSrc.ciderInfo)

#define PRV_TXQ_CIDER_INFO_INIT_MARK_DONE(_done)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.txqDbgDir.txqDbgSrc.ciderInfoInitDone,_done)

#define PRV_TXQ_CIDER_INFO_INIT_GET()\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.txqDbgDir.txqDbgSrc.ciderInfoInitDone)


#define PRV_TXQ_SIP_6_DBG_SATURATION_VALUE 0xFFFF

#define TXQ_DEBUG_PREEMPTION_TYPE_PRINT( _preemptionType) \
    do \
    {\
       switch(_preemptionType)\
        {\
            case PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_EXP_E:\
                cpssOsPrintf("preemptionType = EXP\n");\
                break;\
            case PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_PRE_E:\
                cpssOsPrintf("preemptionType = PRE\n");\
                break;\
            case PRV_DXCH_TXQ_SIP6_PREEMTION_A_NODE_TYPE_NONE_E:\
                cpssOsPrintf("preemptionType = NONE\n");\
                break;\
        }\
     }while(0);
extern GT_U32 prvCpssTxqUtilsCreditSizeGet
(
    IN GT_U8 devNum
);
extern  GT_STATUS prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 dpNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 * indexPtr,
    IN GT_BOOL isErrorForLog
);


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_HWINFO_STC
 *
 * @brief txq hw info for debug
*/
typedef struct{

    /**number of tiles*/
    GT_U32 numberOfTiles;
    /**number of data path*/
    GT_U32                  numberOfDp;
    /**number of local ports*/
    GT_U32                  numberOfLocalPorts;
    /**number of queues per DP*/
    GT_U32                  numberOfDpQueues;
    /**PB size*/
    GT_U32                  pbSize;
    /**PDQ ports in PSI size*/
    GT_U32                  pdqNumPorts;
    /**Shaper frequency*/
    GT_U32                  freq;
} PRV_CPSS_DXCH_TXQ_SIP_6_HWINFO_STC;

extern GT_STATUS     prvSchedRmDebugDumpAllChunk(PRV_CPSS_SCHED_HANDLE hndl);

#define PRV_CPSS_TXQ_DEBUG_PORT_MAPPING_CNS  0x1
#define PRV_CPSS_TXQ_DEBUG_PORT_STATUS_CNS   0x2
#define PRV_CPSS_TXQ_DEBUG_PORT_SHAPING_CNS  0x4
#define PRV_CPSS_TXQ_DEBUG_PORT_SCHED_CNS    0x8
#define PRV_CPSS_TXQ_DEBUG_PORT_LINK_STATE_CNS    0x10
#define PRV_CPSS_TXQ_DEBUG_PORT_BR_STATE_CNS    0x20
#define PRV_CPSS_TXQ_DEBUG_PORT_SEMI_ELIG_THRESHOLDS_STATE_CNS    0x40

#define PRV_TXQ_DBG_DEFAULT_TD_CODE_CNS CPSS_NET_FIRST_USER_DEFINED_E

#define PRV_CPSS_TXQ_DEBUG_PREEMPTION_OFFSET_CNC_CNS    8


#define MAX_CP_IN_TILE                       2

#define PRV_PREQ_DEBUG_MAX_REG_NUM 10
#define PRV_TXQ_DEBUG_MAX_REG_NUM 64

#define PRV_PREQ_REG_DUMP_BIND_TO_INDEX(_name,_index) \
           registersToDump[_index] = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).preq_##_name;\
           registerNames[_index]   = #_name;

#define PRV_TXQ_REG_DUMP_BIND_TO_INDEX(_name,_index,_loop) \
           registersToDumpPtr[_index] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TX##_name;\
           registerNamesPtr[_index]   = #_name;\
           loopPtr[_index]   = _loop;\
           _index++;

#define PRV_BR_BIND_TO_INDEX(_devNum,_portMacNum,_name,_index) \
           registersToDumpPtr[_index] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->GOP.MTI[_portMacNum].MTI100##_name;\
           registerNamesPtr[_index]   = #_name;\
           _index++;

#define PRV_TXQ_A_NODE_Q_SIZE(_aNode) _aNode##Ptr->queuesData.pdqQueueLast -_aNode##Ptr->queuesData.pdqQueueFirst+1

#define PRV_TXQ_PDS_UNIT_ID 0
#define PRV_TXQ_SDQ_UNIT_ID 1


#define PRV_TXQ_PDS_DUMP_FILE_NAME "txqPdsDump.txt"
#define PRV_TXQ_SDQ_DUMP_FILE_NAME "txqSdqDump.txt"
#define PRV_TXQ_LOG_TRIGGER_FILE_NAME "txq.dbg"



#define PRV_TXQ_QUEUE_STAT_CNC_BLOCK_FIRST  8
#define PRV_TXQ_DROP_REASON_CNC_BLOCK  7


#define PRV_TXQ_QUEUE_STAT_CNC_BLOCK_DEFAULT_FORMAT CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E
#define PRV_TXQ_QUEUE_STAT_BLOCKS_NUM_MAX_REQUIRED  0xFFFFFF

#define PRV_CPSS_DXCH_RXDMA_2_PHYS_OFFS_CNS 0
#define PRV_CPSS_DXCH_RXDMA_2_PHYS_LEN_CNS(devNum)  MAX(8,PRV_CPSS_DXCH_PP_HW_INFO_FLEX_FIELD_NUM_BITS_SUPPORTED_MAC(devNum).phyPort)


extern GT_STATUS prvCpssFalconTxqUtilsGetCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 remotePortNum,
    IN GT_BOOL remote,
    OUT GT_U32 *casCadePortNumPtr,
    OUT GT_U32 *pNodeIndPtr
);

extern GT_STATUS prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum,
    IN GT_U32 localPortStart,
    IN GT_U32 localPortEnd,
    OUT GT_U32 *sourcePortPtr
);

extern GT_STATUS prvCpssDxChTxqDumpAnodeShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 aNodeStart,
    IN GT_U32 aNodeEnd
);


extern GT_STATUS prvCpssFalconTxqUtilsNumberOfConsumedLanesGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  *numberOfConsumedLanesPtr
);

extern GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);

extern GT_U32  prvCpssFalconTxqUtilsStartQueueIndexForDpGet
(
    GT_U8  devNum,
    GT_U32  tileNum,
    GT_U32  dp,
    GT_U32  *queueIndexPtr
);

GT_STATUS prvCpssSip6TxqDebugRequiredBlockNumGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *requiredBlockNumPtr
);

extern GT_STATUS prvCpssDxChTxqFalconDumpSchedErrors
(
    IN GT_U8 devNum
);

GT_STATUS prvCpssDxChTxqDebugDumpQueueShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
)
{
        GT_U32 i;
        GT_U32 rc;
        GT_U32    divExp;
        GT_U32    token;
        GT_U32    res;
        GT_U32    burst;
        GT_BOOL shapingEnabled;
        GT_U32     shaperDec;
        GT_U32     perInter;
        GT_U32     perEn,bw = 0,freqInKHz;
        double tmp;
        int ret;



       rc = prvCpssSip6TxqPdqPerLevelShapingParametersGet(devNum,tileNum,PRV_CPSS_PDQ_LEVEL_Q_E,&shaperDec,&perInter,&perEn);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqPdqPerLevelShapingParametersGet  failed  ");
        }

        cpssOsPrintf("\nQ level parameters:\n\nPeriodic interval %d\n\nshaperDec %d\n\nperEn %d\n",perInter,shaperDec,perEn);

        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
        cpssOsPrintf("\n| Q  |shp| divExp|token  |res  | burst |CIR in Kbit|");
        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        for(i=queueStart; (i<=queueEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM);i++)
        {
            rc = prvCpssFalconTxqPdqShapingEnableGet(devNum,tileNum,i,PRV_CPSS_PDQ_LEVEL_Q_E,&shapingEnabled);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqShapingEnableGet  failed for q  %d  ",i);
            }

            rc = prvCpssFalconTxqPdqGetQueueShapingParameters(devNum,tileNum,i,PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT,
                &divExp,&token,&res,&burst);

             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueShapingParameters  failed for q  %d  ",i);
            }

            ret = prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency(devNum,&freqInKHz,0);

            rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,0);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssTxqSchedulerGetOptimalShaperUpdateFrequency  failed  ");
            }
            freqInKHz/=1000;

            tmp =(double) ( token*(1<<res))/(perInter*(1<<divExp));
            bw = (GT_U32)((freqInKHz*8)*tmp);


             cpssOsPrintf("\n|%4d|%3c|%7d|%7d|%5d|%7d|%11d|",i,shapingEnabled?'y':'n',divExp,token,res,burst,bw);

             cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        }

        cpssOsPrintf("\n");


        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
        cpssOsPrintf("\n| Q  |mbw| divExp|token  |res  | burst |EIR in Kbit|");
        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        for(i=queueStart; (i<=queueEnd) && (i<CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM);i++)
        {
            rc = prvCpssFalconTxqPdqMinBwEnableGet(devNum,tileNum,i,&shapingEnabled);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqShapingEnableGet  failed for q  %d  ",i);
            }

            rc = prvCpssFalconTxqPdqGetQueueShapingParameters(devNum,tileNum,i,PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT,
                &divExp,&token,&res,&burst);


             if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueShapingParameters  failed for q  %d  ",i);
            }

            tmp =(double) ( token*(1<<res))/(perInter*(1<<divExp));
            bw = (GT_U32)((200000*8)*tmp); /*shaper update freq is 200 Mhz*/


             cpssOsPrintf("\n|%4d|%3c|%7d|%7d|%5d|%7d|%11d|",i,shapingEnabled?'y':'n',divExp,token,res,burst,bw);

             cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");

        }

        cpssOsPrintf("\n");

        return GT_OK;


}



/**
* @internal  prvCpssDxChTxqDebugDumpPfccTable  function
* @endinternal
*
* @brief   Dump PFCC unit configuration and PFCC table
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number.
* @param[in] tileNum              Tile number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqDebugDumpPfccTableExt
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   tileNum,
    IN  GT_U32                   *metadataPtr
)
{   GT_STATUS                    rc = GT_OK;
    GT_U32                       i,numberOfTiles;
    PRV_CPSS_PFCC_CFG_ENTRY_STC  entry;
    PRV_CPSS_PFCC_TILE_INIT_STC  pfccDb;
    GT_U32 globalTcIndex;
    GT_U32 globalTcHrIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    globalTcIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);
    globalTcHrIndex = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)*CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum)+8;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    if(tileNum>=numberOfTiles)
    {

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Tile number bigger or equal[ %d ] then number of tiles  number of tiles [%d ]",tileNum,PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    rc = prvCpssFalconTxqPfccUnitCfgGet(devNum,tileNum,&pfccDb);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccUnitCfgGet fail for tile %d",tileNum);
    }

    if(NULL==metadataPtr)
    {

        cpssOsPrintf("\n pfccEnable = %d",pfccDb.pfccEnable);
        cpssOsPrintf("\n isMaster = %d",pfccDb.isMaster);
        cpssOsPrintf("\n pfccLastEntryIndex = %d",pfccDb.pfccLastEntryIndex);
        cpssOsPrintf("\n globalTcIndex = %d",globalTcIndex);
        cpssOsPrintf("\n globalTcHrIndex = %d",globalTcHrIndex);

        cpssOsPrintf("\n+-----+-------+------+----+-------------------+---------------+----------------+");
        cpssOsPrintf("\n|Index|  Type | DMA  | TC | Number Of Bubbles | TC bit vector | Pfc msg trigger|");
        cpssOsPrintf("\n+-----+-------+------+----+-------------------+---------------+----------------+");

        for(i = 0;i<=pfccDb.pfccLastEntryIndex;i++)
        {
          rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,tileNum,i,&entry);

          if(rc!=GT_OK)
          {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntryGet fail for entry %d",i);
          }

          switch(entry.entryType)
          {
            case PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT:
              cpssOsPrintf("\n|%5d|  Port |%6d| NA |        NA         |%.15x|%16d|",i,entry.sourcePort,entry.tcBitVecEn,entry.pfcMessageTrigger);
              break;
            case PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC:
              cpssOsPrintf("\n|%5d|   TC  |  NA  |%4d|        NA         |     NA        |       NA       |",i,entry.globalTc);
              break;
            case PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR:
              cpssOsPrintf("\n|%5d| HR-TC |  NA  |%4d|        NA         |     NA        |       NA       |",i,entry.globalTc);
              break;
            default:
              cpssOsPrintf("\n|%5d| Bubble|  NA  | NA |        %5d      |     NA        |       NA       |",i,entry.numberOfBubbles);
              break;
          }
          cpssOsPrintf("\n+-----+-------+------+----+-------------------+---------------+----------------+");
          cpssOsTimerWkAfter(50);

        }

        cpssOsPrintf("\n");
    }
    else
    {
        *metadataPtr=pfccDb.pfccLastEntryIndex|globalTcIndex<<9|globalTcHrIndex<<18|pfccDb.pfccEnable<<30|pfccDb.isMaster<<31;
    }

    return rc;
}

GT_STATUS prvCpssDxChTxqDebugDumpPfccTable
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   tileNum
)
{
    return prvCpssDxChTxqDebugDumpPfccTableExt(devNum,tileNum,NULL);
}


/**
* @internal prvCpssDxChTxqDebugDumpFlowControlConfig function
* @endinternal
*
* @brief   Dump flow control configuration of specific port (txQ and L1)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           -PP's device number.
* @param[in] portNum          -physical port number
* @param[in] pfcModeEnablePtr -(Pointer to)If equals GT_TRUE,the Core generates and processes PFC control frames according
*  to the Priority Flow Control Interface signals. If equals GT_FALSE (0 - Reset Value), the Core operates in legacy Pause Frame
*  mode and generates and processes standard Pause Frames.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqDebugDumpFlowControlConfig
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_FC_MODE_ENT fcMode;
    GT_BOOL fcuTxEnable,fcuRxEnable,pfcMode,pauseFwd,pauseIgn;
    GT_U32 quanta,i;
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_U32 portMacNum; /* MAC number */
    GT_U32 rxPauseStatus,tcPaused;
    GT_U32 macStatus;
    GT_U32 regAddr;
    GT_U32 xoffOverride;
    GT_BOOL                 fcuIsSegmentedChannel;
    GT_U32                  fcuChannelId;
    GT_U32                  txXoffStatus;


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    rc = prvCpssDxChPortSip6FlowControlModeGet(devNum,portNum,&fcMode);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6FlowControlModeGet  failed for portNum  %d  ",portNum);
    }

    /*only Falcon has FCU*/
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)==GT_FALSE)
    {
      rc = prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet(devNum,portNum,&fcuTxEnable,&fcuIsSegmentedChannel,&fcuChannelId);
      if(rc!=GT_OK)
      {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcConfigFcuChannelControlTxEnableGet  failed for portNum  %d  ",portNum);
      }
      cpssOsPrintf("\n TX -> fcuIsSegmentedChannel %d  fcuChannelId %d\n",fcuIsSegmentedChannel,fcuChannelId);

      rc = prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet(devNum,portNum,&fcuRxEnable);

     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcConfigFcuChannelControlRxEnableGet  failed for portNum  %d  ",portNum);
     }
    }
    else
    {
        fcuTxEnable =  GT_TRUE;
        fcuRxEnable = GT_TRUE;
    }


     rc = prvCpssDxChPortSip6PfcModeEnableGet(devNum,portNum,&pfcMode,&regAddr);

     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcModeEnableGet  failed for portNum  %d  ",portNum);
     }

     cpssOsPrintf("\n COM CONF addr is 0x%x\n",regAddr);

     rc = prvCpssDxChPortSip6PfcPauseFwdEnableGet(devNum,portNum,&pauseFwd);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcPauseFwdEnableGet  failed for portNum  %d  ",portNum);
     }

     rc = prvCpssDxChPortSip6PfcPauseIgnoreEnableGet(devNum,portNum,&pauseIgn);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcPauseIgnoreEnableGet  failed for portNum  %d  ",portNum);
     }



    cpssOsPrintf("\n+---+-----------+--------+--------+------------+------------------+------------------+");
    cpssOsPrintf("\n| # |TXQ FC type| FCU TX | FCU RX |COM CONF PFC|COM CONF PAUSE FWD|COM CONF PAUSE IGN|");
    cpssOsPrintf("\n+---+-----------+--------+--------+------------+------------------+------------------+");

    cpssOsPrintf("\n|%3d|%11s|%8d|%8d|%12d|%18d|%18d|",portNum,
        fcMode==CPSS_DXCH_PORT_FC_MODE_802_3X_E?"FC":(fcMode==CPSS_DXCH_PORT_FC_MODE_PFC_E?"PFC":"DISABLE"),
        fcuTxEnable?1:0,fcuRxEnable?1:0,pfcMode?1:0,pauseFwd?1:0,pauseIgn?1:0);
     cpssOsPrintf("\n+---+-----------+--------+--------+------------+------------------+------------------+");
    cpssOsPrintf("\n");

    cpssOsPrintf("Quanta - MAC TYPE %s\n",
        (macType == PRV_CPSS_PORT_MTI_100_E)?"MTI 100":(macType == PRV_CPSS_PORT_MTI_400_E)?"MTI 400":(macType == PRV_CPSS_PORT_MTI_USX_E)?"MTI USX":(macType == PRV_CPSS_PORT_MTI_CPU_E)?"MTI CPU":"Unknown");

    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+");
    cpssOsPrintf("\n|TC0   |TC1   |TC2   |TC3   |TC4   |TC5   |TC6   |TC7   |");
    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+\n");

    for(i=0;i<8;i++)
    {
     rc = prvCpssDxChPortSip6PfcPauseQuantaGet(devNum,portNum,i,&quanta);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcModeEnableGet  failed for portNum  %d  ",portNum);
     }
     cpssOsPrintf("| %5x",quanta);
    }
    cpssOsPrintf("|\n+------+------+------+------+------+------+------+------+\n");

    cpssOsPrintf("Threshold - MAC TYPE %s\n",
        (macType == PRV_CPSS_PORT_MTI_100_E)?"MTI 100":(macType == PRV_CPSS_PORT_MTI_400_E)?"MTI 400":(macType == PRV_CPSS_PORT_MTI_USX_E)?"MTI USX":(macType == PRV_CPSS_PORT_MTI_CPU_E)?"MTI CPU":"Unknown");

    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+");
    cpssOsPrintf("\n|TC0   |TC1   |TC2   |TC3   |TC4   |TC5   |TC6   |TC7   |");
    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+\n");

    for(i=0;i<8;i++)
    {
     rc = prvCpssDxChPortSip6PfcQuantaThreshGet(devNum,portNum,i,&quanta);
     if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcQuantaThreshGet  failed for portNum  %d  ",portNum);
     }
     cpssOsPrintf("| %5x",quanta);
    }
    cpssOsPrintf("|\n+------+------+------+------+------+------+------+------+\n");



    rc = prvCpssDxChPortSip6PfcRxPauseStatusGet(devNum,portNum,&rxPauseStatus,&regAddr);
    if(rc!=GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChPortSip6PfcQuantaThreshGet  failed for portNum  %d  ",portNum);
     }

     cpssOsPrintf("Rx Pause Status - MAC TYPE % s regAddr 0x%x\n",
        (macType == PRV_CPSS_PORT_MTI_100_E)?"MTI 100":(macType == PRV_CPSS_PORT_MTI_400_E)?"MTI 400":(macType == PRV_CPSS_PORT_MTI_USX_E)?"MTI USX":(macType == PRV_CPSS_PORT_MTI_CPU_E)?"MTI CPU":"Unknown",
        regAddr);

    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+");
    cpssOsPrintf("\n|TC0   |TC1   |TC2   |TC3   |TC4   |TC5   |TC6   |TC7   |");
    cpssOsPrintf("\n+------+------+------+------+------+------+------+------+\n");

    for(i=0;i<8;i++)
    {
     tcPaused=(rxPauseStatus>>i)&1;
     cpssOsPrintf("| %5x",tcPaused);
    }
    cpssOsPrintf("|\n+------+------+------+------+------+------+------+------+\n");


    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.status;
    }
    else
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " unsupported mac type %d for portNum  %d \n ",macType, portNum);
    }

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&macStatus);

    if(rc==GT_OK)
    {
        cpssOsPrintf("MAC status address 0x%x is 0x%x\n",regAddr, macStatus);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDrvHwPpPortGroupReadRegister  failed for portNum  %d  ",portNum);
    }

    rc = prvCpssDxChPortSip6XoffOverrideEnableGet(devNum, portNum,&xoffOverride);

    if(rc==GT_OK)
    {
        cpssOsPrintf("MAC XOFF Override is %d\n",xoffOverride);
    }

    rc = prvCpssDxChPortSip6XoffStatusGet(devNum, portNum,&regAddr,&txXoffStatus);

    if(rc==GT_OK)
    {
        cpssOsPrintf("MAC TX XOFF status  is 0x%x adress  0x%x\n",txXoffStatus,regAddr);
    }


    return rc;
}

/**
* @internal  prvCpssDxChTxqDebugDumpQfcConfig  function
* @endinternal
*
* @brief   Dump QFC  unit configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number.
* @param[in] tileNum              Tile number
* @param[in] qfcNum              Local QFC number [0..7]
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssDxChTxqDebugDumpQfcConfig
(
  IN  GT_U8                    devNum,
  IN  GT_U32                   tileNum,
  IN  GT_U32                   qfcNum
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_QFC_GLOBAL_PFC_CFG_STC  qfcCfg;
    GT_STATUS                                      rc;
    GT_U32                                         numberOfTiles;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(qfcNum);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    if(tileNum>=numberOfTiles)
    {

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Tile number bigger or equal[ %d ] then number of tiles  number of tiles [%d ]",tileNum,PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
    }

    rc = prvCpssFalconTxqQfcGlobalPfcCfgGet(devNum,tileNum,qfcNum,&qfcCfg);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcGlobalPfcCfgGet  failed for qfc  %d  ",qfcNum);
    }

    cpssOsPrintf("\n globalPfcEnable             = %d",qfcCfg.globalPfcEnable);
    cpssOsPrintf("\n ingressAgregatorTcPfcBitmap = %d",qfcCfg.ingressAgregatorTcPfcBitmap);
    cpssOsPrintf("\n pbAvailableBuffers          = %d",qfcCfg.pbAvailableBuffers);
    cpssOsPrintf("\n pbCongestionPfcEnable       = %d",qfcCfg.pbCongestionPfcEnable);
    cpssOsPrintf("\n pdxCongestionPfcEnable      = %d",qfcCfg.pdxCongestionPfcEnable);
    cpssOsPrintf("\n hrCountingEnable            = %d\n",qfcCfg.hrCountingEnable);

    return GT_OK;

}

/**
* @internal  prvCpssFalconTxqDebugDumpPortInfo  function
* @endinternal
*
* @brief   Dump port configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] portNum              physical port number
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
static GT_STATUS prvCpssFalconTxqDebugDumpPortMapping
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  printDebug,
    IN  GT_BOOL                 isCascade,
    OUT GT_U32                  *tileNumPtr,
    OUT GT_U32                  *dpNumPtr,
    OUT GT_U32                  *localdpPortNumPtr
)
{
    GT_STATUS rc;
    GT_U32 tileNum,numberOfConsumedLanes,aNodeIndex=0,pNodeIndex;
    GT_U32 localdpPortNum,dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    rc = prvCpssFalconTxqUtilsNumberOfConsumedLanesGet(devNum,tileNum,dpNum,localdpPortNum,&numberOfConsumedLanes);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }

    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum,portNum,NULL,&aNodeIndex);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ",portNum);
        }
    }

    rc = prvCpssSip6TxqUtilsPnodeIndexGet(devNum,dpNum,localdpPortNum,&pNodeIndex);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet  failed for local port  %d  ",localdpPortNum);
    }

    if(printDebug)
    {

        cpssOsPrintf("\n+---------------+------+----+------------+----------------------+------+------+");
        cpssOsPrintf("\n| physical port | tile | DP | local port | numberOfConsumedLanes|P node|A node|");
        cpssOsPrintf("\n+---------------+------+----+------------+----------------------+------+------+");
        cpssOsPrintf("\n|%15d|%6d|%4d|%12d|%22d|%6d|%6d|",portNum,tileNum,dpNum,localdpPortNum,numberOfConsumedLanes,pNodeIndex,isCascade?0:aNodeIndex);
        cpssOsPrintf("\n+---------------+------+----+------------+----------------------+------+------+");
        cpssOsPrintf("\n");
    }

    *tileNumPtr = tileNum;
    *dpNumPtr = dpNum;
    *localdpPortNumPtr = localdpPortNum;

    return GT_OK;


}

/**
* @internal  prvCpssFalconTxqTailDropPoolMappingValidityCheck  function
* @endinternal
*
* @brief   Validate tc4Pfc  SW shadow.Check that  <pool id>  field from EGF_QAG_TC_DP_MAPPER table
*      is synced to pfcc tc to pool vector.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqTailDropPoolMappingValidityCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum
)
{
    GT_U32 poolId;
    GT_U32 tc4Pfc;
    GT_STATUS rc;
    GT_U32 i,value;
    GT_U32 pfccTcMapping,pfccPoolId,tmp;
    GT_BOOL headRoom;


    rc = prvCpssFalconTxqPfccTcMapVectorGet(devNum,tileNum,&pfccTcMapping);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcMapVectorGet failed  ");
    }

    tmp=pfccTcMapping;

    for(i=0;i<4;i++)
    {
       cpssOsPrintf("PFC mapping POOL_%d vector 0x%x\n",i,tmp&0xFF);
       tmp>>=8;
    }

    for(i=0;i<_4K;i++)
    {
        rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,i,&value);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChReadTableEntry EGF_QAG_TC_DP_MAPPER  failed  for  index %d  ",i);
        }

        tc4Pfc = U32_GET_FIELD_MAC(value,6,3);
        poolId = U32_GET_FIELD_MAC(value,9,1);

        if(pfccTcMapping&(1<<tc4Pfc))
        {
            pfccPoolId =0;
        }
        else if(pfccTcMapping&(1<<(tc4Pfc+8)))
        {
            pfccPoolId =1;
        }
        else if(pfccTcMapping&(1<<(tc4Pfc+16)))
        {
            pfccPoolId =2;
        }
        else if(pfccTcMapping&(1<<(tc4Pfc+24)))
        {
            pfccPoolId =3;
        }
        else
        {
            pfccPoolId =4;
        }

        if(pfccPoolId>0)
        {
            pfccPoolId =1;
        }

        if(poolId!=pfccPoolId)
        {
            cpssOsPrintf("FAIL for index %d for Vector 0x%x tc4Pfc %d poolId %d\n",i,pfccTcMapping,tc4Pfc,poolId);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqTailDropPoolMappingValidityCheck failed  for  index %d  ",i);
        }
    }


    /*Check  coherency of TC_to_pool_CFG and pool_CFG*/
    for(i=0;i<8;i++)
    {
        rc =prvCpssFalconTxqPfccTcResourceModeGet(devNum,tileNum,(GT_U8)i,&tmp,&headRoom);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPfccTcMapVectorGet failed  ");
        }

        if(pfccTcMapping&(1<<i))
        {
            pfccPoolId =0;
        }
        else if(pfccTcMapping&(1<<(i+8)))
        {
            pfccPoolId =1;
        }
        else if(pfccTcMapping&(1<<(i+16)))
        {
            pfccPoolId =2;
        }
        else if(pfccTcMapping&(1<<(i+24)))
        {
            pfccPoolId =3;
        }
        else
        {
            pfccPoolId =4;
        }

        if(tmp!=PRV_TC_MAP_PB_MAC)
        {
            cpssOsPrintf("TC %d DBA POOL %d  PFC MAP %d HR %d\n",i,tmp,pfccPoolId,headRoom);

            if(tmp!=pfccPoolId)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " Coherency of TC_to_pool_CFG and pool_CFG failed for tc %d ",i);
            }
        }
        else
        {
            cpssOsPrintf("TC %d DBA POOL PB PFC MAP %d HR %d\n",i,pfccPoolId,headRoom);
        }
    }

    tmp =0;
    /*Check that all Tcs are mapped*/
    for(i=0;i<4;i++)
    {
        tmp |= (pfccTcMapping&0xFF);
        pfccTcMapping>>=8;
    }

    if(tmp!=0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "pfccTcMapping vector validation failed.Unmapped TC found 0x%x",tmp);
    }

    cpssOsPrintf("ALL PASS  \n");

    return GT_OK;
}

/**
* @internal  prvCpssFalconTxqDebugDumpPort  function
* @endinternal
*
* @brief   Debug function for port diagnostic
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] portNum               physical number.
* @param[in] debugMode         desired debug information bitmap
*                                                     [0x1 -mapping ,0x2 - status ,0x3 shaping configuration ,0x4 scheduling configuration]
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpPort
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  debugMode
)
{
    GT_STATUS rc;
    GT_U32    tileNum;
    GT_U32    dpNum;
    GT_U32    localPortNum,dpQueueOffset;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr=NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pnodePtr;
    GT_U32    queueStart,queueEnd,i;
    GT_U32    activeSliceMap[40];
    GT_BOOL   sliceValid[40];
    GT_U32    activeSliceNum;
    GT_U32    pNodeIndex,txDma,aNodeListSize,pNodeNum;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE range;
    GT_BOOL isCascadePort = GT_FALSE;
    GT_U32         hwValue[3];
    GT_BOOL        selectListEnable[2];
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portEgfLinkStatusState;
    PRV_TXQ_GOP_UTILS_BR_PARAMS_STC            brParams;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES qAttr;
    GT_CHAR_PTR stateNames []= {"INITIAL","TXQ_PORT_ENABLED","TXD_RESOURCE_CONFIGURED","FULL"};

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
    if(rc != GT_OK)
    {
         return rc;
    }

    if(GT_FALSE == isCascadePort)
    {
        rc =prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed  ");
        }
     }

    rc =prvCpssFalconTxqDebugDumpPortMapping(devNum,portNum,(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_MAPPING_CNS)?GT_TRUE:GT_FALSE,isCascadePort,
        &tileNum,&dpNum,&localPortNum);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqDebugDumpPortMapping failed  ");
    }
    rc = prvCpssFalconPsiMapPortToPdqQueuesGet(devNum,tileNum,dpNum,localPortNum,&dpQueueOffset,hwValue,hwValue+1,hwValue+2);
    if(rc!=GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

    }
    if(GT_FALSE == isCascadePort)
    {
        if(aNodePtr==NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "NULL pointer\n");
        }
        queueStart = aNodePtr->queuesData.pdqQueueFirst- dpQueueOffset;
        queueEnd   = aNodePtr->queuesData.pdqQueueLast- dpQueueOffset;
    }
    else
    {
        pNodeNum = dpNum + (MAX_DP_IN_TILE(devNum))*localPortNum;
        PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pnodePtr,devNum,tileNum,pNodeNum);

        queueStart = pnodePtr->aNodelist[0].queuesData.queueBase;
        aNodeListSize = pnodePtr->aNodeListSize;
        queueEnd = queueStart+(pnodePtr->aNodelist[aNodeListSize-1].queuesData.pdqQueueLast -pnodePtr->aNodelist[0].queuesData.pdqQueueFirst);

    }
            /*Mapping*/
    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_MAPPING_CNS)
    {

        cpssOsPrintf("\n");
        rc = prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump(devNum,portNum,portNum);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump failed  ");
        }
        cpssOsPrintf("\n");




        cpssOsPrintf("\nPSI configurations\n");
        cpssOsPrintf("\n+-----+--------------+-----------------+---------------------+--------------------+");
        cpssOsPrintf("\n| SDQ | localPortNum | DP queue offset |Internal offset start|Internal offset stop|");
        cpssOsPrintf("\n+-----+--------------+-----------------+---------------------+--------------------+");
        cpssOsPrintf("\n|%5d|%14d|%17d|%20d|%21d|",dpNum,localPortNum,dpQueueOffset,queueStart,queueEnd);

        cpssOsPrintf("\n+-----+--------------+-----------------+---------------------+--------------------+\n");

        rc = prvCpssFalconTxqUtilsSdqPortRangeDump(devNum,tileNum,dpNum,GT_TRUE,localPortNum);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        rc = prvCpssFalconTxqSdqSelectListGet(devNum,tileNum,dpNum,localPortNum,&range);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        cpssOsPrintf("\nSelect List Pointers\n");
        cpssOsPrintf("\n+-----+--------------+---------------+---------------+----------------+-------------+");
        cpssOsPrintf("\n| SDQ | localPortNum | prio0HighLimit| prio0LowLimit | prio1HighLimit |prio1LowLimit|");
        cpssOsPrintf("\n+-----+--------------+---------------+---------------+----------------+-------------+");
        cpssOsPrintf("\n|%5d|%14d|%15d|%15d|%16d|%13d|",dpNum,localPortNum,range.prio0HighLimit,range.prio0LowLimit,
            range.prio1HighLimit,range.prio1LowLimit);
        cpssOsPrintf("\n+-----+--------------+---------------+---------------+----------------+-------------+\n");


        rc =prvCpssFalconTxqUtilsSdqQueueAttributesDump(devNum,tileNum,dpNum,queueStart,
            queueEnd-queueStart+1,NULL);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)==GT_TRUE)
        {
            rc = prvCpssFalconTxqSdqSelectListEnableGet(devNum,tileNum,dpNum,localPortNum,GT_TRUE,selectListEnable);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }
            rc = prvCpssFalconTxqSdqSelectListEnableGet(devNum,tileNum,dpNum,localPortNum,GT_FALSE,selectListEnable+1);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }

            cpssOsPrintf("\nSelect List enable: Prio 0 - %d , Prio 1 - %d\n",selectListEnable[0],selectListEnable[1]);
        }

        cpssOsPrintf("\nQFC configurations\n");
        rc =prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping(devNum,tileNum,dpNum,localPortNum,localPortNum,&txDma);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_STATUS_CNS)
    {
        /*Queue status*/

        rc = prvCpssFalconTxqSdqQueueStatusDump(devNum,tileNum,dpNum,queueStart,
            queueEnd-queueStart+1,NULL);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc =prvCpssDxChTxqFalconDescCounterDump(devNum,tileNum,dpNum,queueStart,queueEnd,NULL);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }

        /*pizza is supported only for Falcon*/
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)==GT_FALSE)
        {
          cpssOsPrintf("\nPDX Pizza arbiter\n");

          cpssOsPrintf("\n+----+-----+-------+-----+----------+");
          cpssOsPrintf("\n|PDX | PDS | slice |valid|assignment|");
          cpssOsPrintf("\n+----+-----+-------+-----+----------+");

          rc = prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet(devNum,tileNum,dpNum,&activeSliceNum,activeSliceMap,sliceValid);

          if(rc!=GT_OK)
          {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

          }


          for(i=0;i<activeSliceNum;i++)
          {
            cpssOsPrintf("\n|%4d|%5d|%7d|%5s|%10c|",tileNum,dpNum,i,(sliceValid[i]==GT_TRUE)?"true":"false",(sliceValid[i]==GT_TRUE)?'0'+activeSliceMap[i]:' ');
          }

          cpssOsPrintf("\n+----+-----+-------+-----+----------+");
        }
        if (prvCpssDxChPortRemotePortCheck(devNum, portNum) == GT_FALSE)
        {
            /*Flow control*/
            rc = prvCpssDxChTxqDebugDumpFlowControlConfig(devNum,portNum);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }
       }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_SHAPING_CNS)
    {
        /*no shaping for cascade*/
        if(GT_FALSE == isCascadePort)
        {

            /*Shaping*/
            rc = prvCpssDxChTxqDumpAnodeShapingParameters(devNum,tileNum,aNodePtr->aNodeIndex,aNodePtr->aNodeIndex);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }

            rc = prvCpssDxChTxqDebugDumpQueueShapingParameters(devNum,tileNum,aNodePtr->queuesData.pdqQueueFirst,
                aNodePtr->queuesData.pdqQueueLast);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

            }
        }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_SCHED_CNS)
    {
        /*Scheduling*/
        rc = prvCpssSip6TxqUtilsPnodeIndexGet(devNum,dpNum,localPortNum,&pNodeIndex);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet  failed for local port  %d  ",localPortNum);
        }

        cpssOsPrintf("\nDump scheduling PDQ configurations\n");
        rc = prvCpssFalconTxqPdqDumpPortHw(devNum,tileNum,pNodeIndex);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

        }
    }



    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_LINK_STATE_CNS)
    {

        rc = cpssDxChBrgEgrFltPortLinkEnableGet(devNum, portNum, &portEgfLinkStatusState);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChBrgEgrFltPortLinkEnableGet failed=%d ", rc);
        }

        cpssOsPrintf("\nLINK state in EGF %s\n",
            portEgfLinkStatusState==CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E?
            "LINK_UP_E":
            "LINK_DOWN_E");
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_BR_STATE_CNS)
    {

        rc = prvCpssDxChPortSip6BrDataGet(devNum, portNum, &brParams);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSip6BrDataGet failed=%d ", rc);
        }
        if(GT_TRUE == brParams.paramValid)
        {
            cpssOsPrintf("\nBR parameters are BR_CONTROL 0x%X ,BR_STATUS 0x%X \n",
           brParams.control ,brParams.status);
        }
        else
        {
            cpssOsPrintf("\nBR parameters are N/A \n");
        }


        if(GT_FALSE == isCascadePort)
        {
            rc =prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed  ");
            }

            cpssOsPrintf("Anode  preemption parameters:\n");
            cpssOsPrintf("partner A node index =%d\n",aNodePtr->partnerIndex);
            cpssOsPrintf("preemption activated =%d\n",aNodePtr->preemptionActivated);
            cpssOsPrintf("implicit allocation =%d\n",aNodePtr->implicitAllocation);
            cpssOsPrintf("gOq =%d\n",aNodePtr->queuesData.queueGroupIndex);
            TXQ_DEBUG_PREEMPTION_TYPE_PRINT(aNodePtr->preemptionType)


            if( aNodePtr->partnerIndex!=CPSS_SIP6_TXQ_INVAL_DATA_CNS)
            {
                rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,tileNum,aNodePtr->partnerIndex,&pNodeNum);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodePtr->partnerIndex);
                }
                PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pnodePtr,devNum,tileNum,pNodeNum);
                aNodePtr= &(pnodePtr->aNodelist[0]);
                cpssOsPrintf("partner physical port =%d\n",aNodePtr->physicalPort);
                cpssOsPrintf("partner goQ=%d\n",aNodePtr->queuesData.queueGroupIndex);
                cpssOsPrintf("partner ");
                TXQ_DEBUG_PREEMPTION_TYPE_PRINT(aNodePtr->preemptionType);
                cpssOsPrintf("parter implicit allocation =%d\n",aNodePtr->implicitAllocation);
                cpssOsPrintf("parter preemption activated =%d\n",aNodePtr->preemptionActivated);
            }

        }
    }

    if(debugMode&PRV_CPSS_TXQ_DEBUG_PORT_SEMI_ELIG_THRESHOLDS_STATE_CNS)
    {
        if(GT_FALSE == isCascadePort)
        {
            rc =prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&aNodePtr);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber failed  ");
            }

           cpssOsPrintf("\nsemiEligBmp  0x%X \n",aNodePtr->queuesData.semiEligBmp);
           cpssOsPrintf("Data path state %s \n",stateNames[aNodePtr->queuesData.dataPathState]);

           for(i=aNodePtr->queuesData.queueBase;i<aNodePtr->queuesData.queueBase + PRV_TXQ_A_NODE_Q_SIZE(aNode);i++)
           {
                rc = prvCpssFalconTxqSdqQueueAttributesGet(devNum,aNodePtr->queuesData.tileNum,
                          aNodePtr->queuesData.dp,i,&qAttr);
                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);

                }
                cpssOsPrintf("Q %d : semi elig threshold %d \n",i,qAttr.semiEligThreshold);
           }


        }

    }

    return GT_OK;
}

/**
* @internal  prvCpssFalconTxqDebugDumpPort  function
* @endinternal
*
* @brief   Dump function for QFC flow control status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] tileNum                tile number.
* @param[in] qfcNum               QFC number

* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpQfcStatus
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  qfcNum
)
{
    GT_STATUS                                rc;
    GT_U32                                   globalPfcStatusBmp;
    GT_U32                                   portPfcStatusBmp[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
    GT_U32                                   portTcPfcStatusBmp[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
    GT_U32                                   pfcMessageCounter;

    GT_U32                                   pbLimitCrossed;
    GT_U32                                   pdxLimitCrossed;
    GT_U32                                   iaLimitCrossed;
    GT_U32                                   globalLimitCrossedBmp;
    GT_U32                                   portLimitCrossed[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
    GT_U32                                   i,sequenceNum,iteration;
    GT_U32                                   firstPort,lastPort,numberOfLocalPorts;

    rc = prvCpssFalconTxqQfcPfcStatusGet(devNum,tileNum,qfcNum,&globalPfcStatusBmp,portPfcStatusBmp,
        portTcPfcStatusBmp,&pfcMessageCounter);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcStatusGet failed\n");

    }

     rc = prvCpssFalconTxqQfcPfcGlobalStatusParse(devNum,globalPfcStatusBmp,&pbLimitCrossed,&pdxLimitCrossed,
        &iaLimitCrossed,&globalLimitCrossedBmp);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcStatusGet failed\n");

    }

    cpssOsPrintf("\npfcMessageCounter = %d\n",pfcMessageCounter);

    cpssOsPrintf("\nGlobal Pfc Status\n");
    cpssOsPrintf("\n+----+-----+---------------+-----+");
    cpssOsPrintf("\n| PB | PDX |Global TC(hex) |IT TC|");
    cpssOsPrintf("\n+----+-----+---------------+-----+");
    cpssOsPrintf("\n|%4d|%5d|%15x|%5d|",pbLimitCrossed,pdxLimitCrossed,globalLimitCrossedBmp,iaLimitCrossed);
    cpssOsPrintf("\n+----+-----+---------------+-----+\n");

    rc = prvCpssFalconTxqQfcPfcPortStatusParse(devNum,portPfcStatusBmp,GT_FALSE,portLimitCrossed);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcPortStatusParse failed\n");

    }

    numberOfLocalPorts =  CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);
    sequenceNum =(numberOfLocalPorts/MAX_PORTS_PER_LINE)+((numberOfLocalPorts%MAX_PORTS_PER_LINE)?1:0);

    iteration =0;

    do
    {
        firstPort = ITERATION_START(iteration);
        lastPort  = ITERATION_STOP(iteration,devNum);

        cpssOsPrintf("\nPort Pfc Status\n");
        cpssOsPrintf("\n+-------+-------+-------+-------+-------+-------+-------+-------+-------+\n|");
        for(i=firstPort;i<lastPort;i++)
        {
            cpssOsPrintf("Port %2d|",i);
        }

        cpssOsPrintf("\n+-------+-------+-------+-------+-------+-------+-------+-------+-------+\n|");

        for(i=firstPort;i<lastPort;i++)
        {
            cpssOsPrintf("%7d|",portLimitCrossed[i]);
        }

        cpssOsPrintf("\n+-------+-------+-------+-------+-------+-------+-------+-------+-------+\n|");


        rc = prvCpssFalconTxqQfcPfcPortStatusParse(devNum,portTcPfcStatusBmp,GT_TRUE,portLimitCrossed);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcPortStatusParse failed\n");

        }

        cpssOsPrintf("\nPort TC Pfc Status (hex format)\n");
       cpssOsPrintf("\n+-------+-------+-------+-------+-------+-------+-------+-------+-------+\n|");

        for(i=firstPort;i<lastPort;i++)
        {
            cpssOsPrintf("Port %2d|",i);
        }

        cpssOsPrintf("\n+-------+-------+-------+-------+-------+-------+-------+-------+-------+\n|");

        for(i=firstPort;i<lastPort;i++)
        {
            cpssOsPrintf("%7x|",portLimitCrossed[i]);
        }

        cpssOsPrintf("\n+-------+-------+-------+-------+-------+-------+-------+-------+-------+\n|");

        sequenceNum--;
        iteration++;
     }
     while(sequenceNum>0);


    return GT_OK;

}/**
* @internal  prvCpssFalconTxqDebugDumpHrEvents  function
* @endinternal
*
* @brief   Dump HR crossed events
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpHrEvents
(
    IN  GT_U8                   devNum
)
{
     GT_U32                  tileNum,numberOfTiles;
     GT_U32                  qfcNum,i;
     GT_STATUS               rc;
     GT_U32                               numOfEvents,totalNumOfEvents=0;
     PRV_CPSS_DXCH_HR_EVENT_INFO_STC      eventsArr[CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC*CPSS_TC_RANGE_CNS];


     switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
     {
          case 0:
          case 1:
              numberOfTiles = 1;
              break;
         case 2:
         case 4:
              numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
              break;
          default:
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
              break;
     }

    cpssOsPrintf("\n+--+----+-----+----+--+");
    cpssOsPrintf("\n|# |Tile| QFC |Port|TC|");
    cpssOsPrintf("\n+--+----+-----+----+--+");

     for(tileNum=0;tileNum<numberOfTiles;tileNum++)
     {
        for(qfcNum=0;qfcNum<CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC;qfcNum++)
        {
            rc = prvCpssFalconTxqQfcHeadroomInterruptStatusGet(devNum,tileNum,qfcNum,&numOfEvents,eventsArr);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcMaxMinHeadroomCounterGet failed\n");
            }

            for(i=0;i<numOfEvents;i++)
            {
                cpssOsPrintf("\n|%2d|%4d|%5d|%4d|%2d|",totalNumOfEvents+i,tileNum,qfcNum,eventsArr[i].localPortNum,eventsArr[i].trafficClass);
                cpssOsPrintf("\n+--+----+-----+----+--+");
            }
            totalNumOfEvents+=numOfEvents;
        }
     }

     return GT_OK;
}

/**
* @internal  prvCpssFalconTxqDebugHrEventsSet  function
* @endinternal
*
* @brief   Enable HR threshold crossed interrupt on device
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugHrEventsSet
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32 numberOfTiles,tileNum,qfcNum;
    GT_STATUS rc;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }



    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
       for(qfcNum=0;qfcNum<CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC;qfcNum++)
       {
           rc = prvCpssFalconTxqQfcHeadroomInterruptEnableSet(devNum,tileNum,qfcNum,enable);

           if(rc!=GT_OK)
           {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcMaxMinHeadroomCounterGet failed\n");
           }
        }
     }

     return GT_OK;
}

GT_STATUS prvCpssFalconTxqUtilsDumpBindPortsToPdsDb
(
    IN GT_U8  devNum,
    IN GT_U32 lengthAdjust
)
{
    GT_U32 i,j,tmp,portNum,shift,num=0;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC *profilePtr;

    profilePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles;

    if(lengthAdjust)
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)==GT_FALSE)
        {

          cpssOsPrintf("Length adjust binding is supported only for SIP 6.10\n");
          return GT_OK;
        }
        else
        {
            profilePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.lengthAdjustProfiles;
        }
    }


    for(i = 0;i<CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM ;i++,profilePtr++)
    {
            cpssOsPrintf(" \n%s Profile %d binded ports (num %d , speed in G : %d): \n",lengthAdjust?"Length adjust":"PDS",i,
                profilePtr->numberOfBindedPorts,
                 profilePtr->speed_in_G);

            for(j=0;j<32;j++)
            {
                tmp= profilePtr->bindedPortsBmp[j];
                if(tmp )
                {
                    shift =0;
                    while(tmp)
                    {
                        if(tmp&0x1)
                        {
                            portNum = j*32+shift;

                            cpssOsPrintf(" %3d ,",portNum);
                            num++;

                            if(num==8)
                            {
                                cpssOsPrintf(" \n");
                                num=0;
                            }

                        }
                        tmp>>=1;
                        shift++;
                    }
                }
            }

    }

     return GT_OK;
}

/**
* @internal  prvCpssFalconTxqDumpPoolCounters  function
* @endinternal
*
* @brief   Dump PFCC pool counters for all the pools and all the tiles
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number.
* @param[in] tileNum              Tile number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDumpPoolCounters
(
    IN  GT_U8                   devNum
)
{
    GT_U32 numberOfTiles,tileNum,poolInd,regValue;
    GT_U32 regAddr,tileOffset;
    GT_STATUS rc;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }
    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
        for(poolInd=0;poolInd<4;poolInd++)
        {
           regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pfcc.Pool_counter[poolInd];

           tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);
           regAddr+=tileOffset;

           rc =prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

           if(rc!=GT_OK)
           {
               CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssHwPpReadRegister failed for pool %d\n",poolInd);
           }

           cpssOsPrintf("\nTile %d Pool %d Register value %d",tileNum,poolInd,regValue);
        }
    }

    cpssOsPrintf(" \n");

     return GT_OK;
}



GT_STATUS prvCpssFalconTxqDebugEligFuncSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  eligFunc
)
{
    int             ret = 0;
    GT_STATUS       rc;
    PRV_CPSS_SCHED_HANDLE hndl;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum]==NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(eligFunc >PRV_SCHED_ELIG_Q_LAST)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    hndl = PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum];

    if(eligFunc<PRV_SCHED_ELIG_Q_LAST)
    {
        ret = prvSchedLowLevelSetQLevelEligPrioFuncEntry(hndl,(uint16_t) eligFunc);
    }
    else
    {
        ret =prvSchedLowLevelSetQLevelEligPrioFuncAllTable(hndl);
    }
    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,tileNum);

    return rc;
}
/**
* @internal  prvCpssFalconTxqDebugDumpSdqGlobalConfig  function
* @endinternal
*
* @brief   Dump SDQ global features enable/disable status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpSdqGlobalConfig
(
    IN  GT_U8                   devNum
)
{
    GT_U32 tileNum,dpNum,numberOfTiles;
    GT_STATUS rc;
    GT_U32 regAddr,tileOffset,regData;
    GT_U32 semiElig,aging,msgCancel,qbvScanEn;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
        for(dpNum=0;dpNum<MAX_DP_IN_TILE(devNum);dpNum++)
        {
            regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[dpNum].global_config;
            tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);
            regAddr+=tileOffset;
            rc =prvCpssHwPpReadRegister(devNum,regAddr,&regData);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error reading global config for TILE/SDQ - %d/%d ",tileNum,dpNum);
            }

            semiElig = U32_GET_FIELD_MAC(regData,
                            TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_SELIG_EN_FIELD_OFFSET),
                            TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_SELIG_EN_FIELD_SIZE));
            aging = U32_GET_FIELD_MAC(regData,
                            TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_AGING_EN_FIELD_OFFSET),
                            TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_AGING_EN_FIELD_SIZE));
            msgCancel = U32_GET_FIELD_MAC(regData,
                            TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_MSG_CNCL_EN_FIELD_OFFSET),
                            TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_MSG_CNCL_EN_FIELD_SIZE));

            cpssOsPrintf("tile %d dp %d RAW 0x%08x [semiElig %d , aging %d , msgCancel %d ",tileNum,dpNum,regData,semiElig,
                aging,msgCancel);

            if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
            {
                qbvScanEn = U32_GET_FIELD_MAC(regData,
                            TXQ_IRONMAN_SDQ_GLOBAL_CONFIG_QBV_SCAN_EN_FIELD_OFFSET,
                            TXQ_IRONMAN_SDQ_GLOBAL_CONFIG_QBV_SCAN_EN_FIELD_SIZE);

                cpssOsPrintf("qbvScanEn %d ",qbvScanEn);
            }

            cpssOsPrintf("]\n");

        }
    }

    return GT_OK;

}

/**
* @internal  prvCpssFalconTxqDebugDumpSdqGlobalConfig  function
* @endinternal
*
* @brief   Debug function that can override default configuration of semi elig feature.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] enable                enable/disable semi elig feature
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugSdqSemiEligEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 enable,
    IN  GT_BOOL                 enable4Dry

)
{
    GT_U32 tileNum,dpNum,numberOfTiles;
    GT_STATUS rc;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
        for(dpNum=0;dpNum<MAX_DP_IN_TILE(devNum);dpNum++)
        {
         rc = prvCpssFalconTxqSdqSemiEligEnableSet(devNum,tileNum,dpNum,enable,enable4Dry,
            (PRV_CPSS_SIP_6_20_CHECK_MAC(devNum) == GT_TRUE)?GT_TRUE:GT_FALSE);
         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqSemiEligEnableSet failed for dp  %d \n",dpNum );
         }
        }
    }

    return GT_OK;

}
/**
* @internal  prvCpssFalconTxqDebugPreqCountersDump  function
* @endinternal
*
* @brief   Dump PREQ debug counters for all the pipes  and all the tiles
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugPreqCountersDump
(
    IN  GT_U8                   devNum
)
{
    GT_U32 numberOfTiles,tileNum,pipeNum;
    GT_U32 regAddr,tileOffset,regData;
    GT_U32 pipeOffset;
    GT_STATUS rc;
    GT_U32    registersToDump[PRV_PREQ_DEBUG_MAX_REG_NUM]={0},i;
    GT_CHAR_PTR registerNames [PRV_PREQ_DEBUG_MAX_REG_NUM]={0};

    PRV_PREQ_REG_DUMP_BIND_TO_INDEX(debug_registers.mark_ecn_pkts_cnt,0)
    PRV_PREQ_REG_DUMP_BIND_TO_INDEX(debug_registers.mirrored_pkts_cnt,1)
    PRV_PREQ_REG_DUMP_BIND_TO_INDEX(debug_registers.ingress_packet_cnt,2)
    PRV_PREQ_REG_DUMP_BIND_TO_INDEX(debug_registers.egress_packet_cnt,3)
    PRV_PREQ_REG_DUMP_BIND_TO_INDEX(debug_registers.qcn_pkts_cnt,4)


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

   for(i=0;i<PRV_PREQ_DEBUG_MAX_REG_NUM;i++)
   {
        /*done*/
        if(!registerNames[i])
        {
            break;
        }

        cpssOsPrintf("%s\n=============================\n",registerNames[i]);
        for(tileNum=0;tileNum<numberOfTiles;tileNum++)
        {
            for(pipeNum=0;pipeNum<MAX_CP_IN_TILE;pipeNum++)
            {
                regAddr = registersToDump[i];
                tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);
                regAddr+=tileOffset;
                pipeOffset = prvCpssSip6OffsetFromFirstInstanceGet(devNum,pipeNum,PRV_CPSS_DXCH_UNIT_PREQ_E);
                regAddr+=pipeOffset;
                rc =prvCpssHwPpReadRegister(devNum,regAddr,&regData);
                if(rc!=GT_OK)
                {
                   CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqDebugPreqCountersDump failed for tile/cp  %d/%d \n",tileNum,pipeNum);
                }

                cpssOsPrintf("   tile %d , pipe %d , addr 0x%8x ,data %d\n",tileNum,pipeNum,regAddr,regData);
         }
        }
    }

    return GT_OK;

}
/**
* @internal  prvCpssFalconTxqDebugPreqCountersEnableSet  function
* @endinternal
*
* @brief   Debug function that enable PREQ debug counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] enable                enable/disable sPREQ debug counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugPreqCountersEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 enable
)
{
    GT_U32 regAddr;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).preq_debug_registers.debug_cntr_en;
    rc =prvCpssHwPpWriteRegister(devNum,regAddr,enable?0xFF:0x0);
    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqDebugPreqCountersEnableSet failed  \n");
    }

    return GT_OK;

}
/**
* @internal  prvCpssFalconTxqDebugDumpQfcFunctionalInterrupt  function
* @endinternal
*
* @brief   Debug function that edump QFC functional interrupt register
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] enable                enable/disable sPREQ debug counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpQfcFunctionalInterrupt
(
    IN  GT_U8                   devNum
)
{
    GT_U32 tileNum,dpNum,numberOfTiles,regAddr[2],regData=0,regMask =0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    for(tileNum=0;tileNum<numberOfTiles;tileNum++)
    {
        for(dpNum=0;dpNum<MAX_DP_IN_TILE(devNum);dpNum++)
        {
           regAddr[0] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[dpNum].QFC_Interrupt_functional_Cause;
           rc =prvCpssHwPpReadRegister(devNum,regAddr[0],&regData);
           if(rc!=GT_OK)
           {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssHwPpReadRegister failed  \n");
           }
           regAddr[1] = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[dpNum].QFC_Interrupt_functional_Mask;
           rc =prvCpssHwPpReadRegister(devNum,regAddr[1],&regMask);
           if(rc!=GT_OK)
           {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssHwPpReadRegister failed  \n");
           }

           cpssOsPrintf("   Cause -> tile %d , dp %d , addr 0x%8x ,data %d\n",tileNum,dpNum,regAddr[0],regData);
           cpssOsPrintf("   Mask  -> tile %d , dp %d , addr 0x%8x ,data %d\n\n",tileNum,dpNum,regAddr[1],regMask);
        }
    }

     return GT_OK;
}

/**
* @internal  prvCpssTxqSip6DebugDiagnoseExt  function
* @endinternal
*
* @brief   Debug function that check all device available ports .
*          If buffers detected at the queue there is a print.
*           Also there is an option for multiple iterations with sleep time between them
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] iterationNum                the number of required iterations
* @param[in] sleepTimeInMilisec        sleep time between iterations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssTxqSip6DebugDiagnoseExt
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  iterationNum,
    IN  GT_U32                  sleepTimeInMilisec
)
{
    GT_STATUS rc;
    GT_U32 tileNum,pdsNum,numberOfTiles,pdsDescCount;
    GT_U32  i,total;
    GT_U32 physicalPort;


    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    do
    {
        for(tileNum=0;tileNum<numberOfTiles;tileNum++)
        {
            for(pdsNum=0;pdsNum<MAX_DP_IN_TILE(devNum);pdsNum++)
            {
                rc = prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet(devNum,tileNum,pdsNum,&pdsDescCount);
                if(rc!=GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ",tileNum,pdsNum);
                }

                /*Some ports are stuck*/
                if(pdsDescCount>0)
                {
                    cpssOsPrintf("\nDetected tile %d dp %d that contain buffers.\n",tileNum,pdsNum);

                    for(i=0;i<CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);i++)
                    {

                        /*skip reserved ports*/
                        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
                        {
                            if(PRV_CPSS_TXQ_HARRIER_RESERVED_PORTS_BMP_MAC&(1<<i))
                            {
                                continue;
                            }
                        }

                        rc = prvCpssFalconTxqQfcBufNumberGet(devNum,tileNum,pdsNum,i,GT_TRUE,&total);
                        if(rc!=GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcBufNumberGet  failed for tile  %d pds %d ",tileNum,pdsNum);
                        }

                        if(total>0)
                        {
                            /*Find physical port*/
                            rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum,tileNum,pdsNum,i,&physicalPort,NULL);
                            if(rc!=GT_OK)
                            {
                                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortGet  failed for tile  %d pds %d \n",tileNum,pdsNum);
                            }

                            cpssOsPrintf("\n   Detected local dp port %d that contain %d buffers in tile %d dp %d (belong to phys port %d).\n",
                                    i,total,tileNum,pdsNum,physicalPort);
                        }
                    }

                }
                else
                {
                    cpssOsPrintf("*");
                }
            }
       }

      if(iterationNum)
      {
        iterationNum--;

        cpssOsPrintf("\n >>>>Sleep %d msec\n",sleepTimeInMilisec);

        if(sleepTimeInMilisec>0)
        {
            cpssOsTimerWkAfter(sleepTimeInMilisec);
        }
      }
      cpssOsPrintf("\n");
    }
    while(iterationNum);

   return GT_OK;
}

/**
* @internal  prvCpssTxqSip6DebugDiagnose  function
* @endinternal
*
* @brief   Debug function that check all device available ports .
*          If buffers detected at the queue there is a print.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] enable                enable/disable sPREQ debug counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssTxqSip6DebugDiagnose
(
    IN  GT_U8                   devNum
)
{
    return prvCpssTxqSip6DebugDiagnoseExt(devNum,0,0);
}


/**
* @internal  prvCpssFalconTxqDebugDumpQfcHrIntStatusAdresses  function
* @endinternal
*
* @brief   Debug function that print headroom crossed status register addreses
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] enable                enable/disable sPREQ debug counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugDumpQfcHrIntStatusAdresses
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  pipe,
    IN  GT_U32                  dp,
    IN  GT_U32                  registerInd
)
{
    GT_U32 regAddr,tileOffset;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr =  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.qfc[pipe*4+dp].HR_Crossed_Threshold_Reg_Cause[registerInd];
    regAddr+=tileOffset;
    cpssOsPrintf("Tile %d ,pipe %d , dp %d ,regInd %d ->HR_Crossed_Threshold_Reg_Cause addr 0x%x\n",tileNum,pipe,dp,registerInd,regAddr);

    return GT_OK;
}

/**
* @internal  prvCpssTxqSip6DebugDropReason  function
* @endinternal
*
* @brief   Debug function that print drop reason
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
* @param[in] blockNum                 CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] phase - for preparation set to 0, for getting counters set non 0 value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssTxqSip6DebugDropReason
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  blockNum,
    IN  GT_U32                  phase
)
{
    GT_STATUS rc;
    GT_U64                     indexRangesBmp;
    GT_U32                     i;
    CPSS_DXCH_CNC_COUNTER_STC  cnt;
    GT_U32                     counterBase;

    if(0==phase)
    {

        indexRangesBmp.l[0] = 0x1;
        indexRangesBmp.l[1] = 0x0;


        rc = cpssDxChCncBlockClientEnableSet(devNum,blockNum,CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
            GT_TRUE);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientEnableSet  failed for blockNum  %d",blockNum);
        }

        rc = cpssDxChCncBlockClientRangesSet(devNum,blockNum,CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,&indexRangesBmp);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientRangesSet  failed for blockNum  %d",blockNum);
        }

        rc = cpssDxChPortTxTailDropCommandSet(devNum,CPSS_PACKET_CMD_DROP_HARD_E,PRV_TXQ_DBG_DEFAULT_TD_CODE_CNS);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientRangesSet  failed for blockNum  %d",blockNum);
        }

        cpssOsPrintf("\nPhase 0 done for block %d \n",blockNum);
    }
    else
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(PRV_TXQ_DBG_DEFAULT_TD_CODE_CNS, &counterBase);
        if (rc != GT_OK)
        {
            return rc;
        }
        for(i=0;i<7;i++)
        {
            rc = cpssDxChCncCounterGet(devNum,blockNum,counterBase+(CPSS_PACKET_CMD_DROP_HARD_E<<8)+i,
                CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E,&cnt);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncCounterGet  failed for blockNum  %d",blockNum);
            }
            switch(i)
            {
                case 0:
                  cpssOsPrintf("\nGlobal limit drop  packet          %d \n",cnt.packetCount);
                  break;
                case 1:
                  cpssOsPrintf("\nPDX burst fifo global drop packet  %d \n",cnt.packetCount);
                  break;
                case 2:
                  cpssOsPrintf("\nPool limit drop packet             %d \n",cnt.packetCount);
                  break;
                case 3:
                  cpssOsPrintf("\nMC limit drop packet               %d \n",cnt.packetCount);
                  break;
                case 4:
                  cpssOsPrintf("\nPDX burst fifo port  drop packet   %d \n",cnt.packetCount);
                  break;
                case 5:
                  cpssOsPrintf("\nQ max  limit drop packet           %d \n",cnt.packetCount);
                  break;
                default:
                  cpssOsPrintf("\nPort/Q  limit drop packet          %d \n",cnt.packetCount);
                  break;
            }
        }
    }

    return GT_OK;
}

/**
* @internal  prvCpssTxqSip6DebugDropReason  function
* @endinternal
*
* @brief   Debug function that validate SW/HW mapping sync
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum             PP's device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssTxqSip6DebugValidateQgrpTable
(
    IN  GT_U8                   devNum
)
{
    GT_U32 i,tableSize;
    GT_U32 tileNum,aNodeNum;
    GT_STATUS rc,rc1 ;
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC hwData;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE                     *anodePtr=NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE                     *pNodePtr = NULL;
    GT_U32      numOfValidatedEntries =0,numOfValidatedCscdEntries=0;
    GT_BOOL     isCascade;
    GT_U32     phyPortNum;

    tableSize = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups;


    for(i=0;i<tableSize;i++)
    {
        isCascade = GT_FALSE;
        anodePtr = NULL;

        /*get port number*/

        rc = prvCpssDxChTxqFalconPdxQueueGroupMapGet(devNum,i,&hwData);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " prvCpssDxChTxqFalconPdxQueueGroupMapGet  failed for index  %d\n",i);
        }

        rc = prvCpssSip6TxqGoQToPhysicalPortGet(devNum,hwData.queuePdxIndex,i,&phyPortNum);

        if(rc!=GT_OK||phyPortNum==CPSS_SIP6_TXQ_INVAL_DATA_CNS)
        {
            CPSS_LOG_INFORMATION_MAC(" queue group index is unmapped %d rc = %d\n",i,rc);
            continue;
        }


        if (phyPortNum >= PRV_CPSS_MAX_PP_PORTS_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum,phyPortNum,&tileNum,&aNodeNum);

        if(rc ==GT_NOT_FOUND )
        {
            /*check if this CSCD*/
            rc1 = prvCpssFalconTxqUtilsIsCascadePort(devNum,phyPortNum,&isCascade,&pNodePtr);
            if(rc1!=GT_OK||pNodePtr==NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for index  %d\n",i);
            }

            if(GT_TRUE==isCascade)
            {
                anodePtr=&(pNodePtr->aNodelist[0]);
            }
        }

        if(rc==GT_OK||GT_TRUE==isCascade)
        {
            if(NULL==anodePtr)
            {
                rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,phyPortNum,&anodePtr);
                if(rc!=GT_OK ||anodePtr==NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for index  %d\n",i);
                }
            }

            if(hwData.queuePdxIndex!= anodePtr->queuesData.tileNum)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Validation of tile failed for index  %d [%d - %d]\n",i,hwData.queuePdxIndex,tileNum);
            }

            if(hwData.queuePdsIndex!= anodePtr->queuesData.dp)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Validation of dp failed for index  %d [%d - %d]\n",i,hwData.queuePdsIndex,anodePtr->queuesData.dp);
            }

            if(hwData.queueBase!= anodePtr->queuesData.queueBase)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Validation of queueBase failed for index  %d [%d - %d]\n",i,hwData.queueBase,anodePtr->queuesData.queueBase);
            }

            if(hwData.dpCoreLocalTrgPort!= anodePtr->queuesData.localPort)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Validation of local port failed for index  %d [%d - %d]\n",i,hwData.queuePdsIndex,anodePtr->queuesData.dp);
            }

            if(GT_TRUE ==isCascade)
            {
                numOfValidatedCscdEntries++;
            }
            else
            {
                numOfValidatedEntries++;
            }
        }
    }

     cpssOsPrintf("\n%d regular entries validated ,%d cscd entries validated.\n",numOfValidatedEntries,numOfValidatedCscdEntries);

     return GT_OK;

}

GT_STATUS prvCpssFalconTxqUtilsSdqPortCreditDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 sdqNum,
    IN  GT_U32 startPort,
    IN  GT_U32 size,
    IN  CPSS_OS_FILE_TYPE_STC * file
)
{
    GT_U32           i;
    GT_STATUS        rc;
    GT_U32 balance;
    GT_U32 qCount;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);


    cpssOsFprintf(file->fd,"\nPort credit balance tile %d SDQ %d\n",tileNumber,sdqNum);
    cpssOsFprintf(file->fd,"\n+-------------+-------+------+");
    cpssOsFprintf(file->fd,"\n| PortNumber  |balance|qCount|");
    cpssOsFprintf(file->fd,"\n+-------------+-------+------+");


    for(i =startPort;i<(startPort+size);i++)
    {
        rc = prvCpssFalconTxqSdqPortCreditBalanceGet(devNum,tileNumber,sdqNum,i,&balance,&qCount);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqPortCreditBalanceGet\n",rc );
            return rc;
        }
        cpssOsFprintf(file->fd,"\n|%13d|%7d|%6d|",i,balance,qCount);

    }

    cpssOsFprintf(file->fd,"\n+-------------+-------+------+\n");

    return GT_OK;


}



GT_STATUS prvCpssFalconTxqUtilsSdqSelListPtrDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 sdqNum,
    IN  CPSS_OS_FILE_TYPE_STC * file
)
{
    GT_U32           i;
    GT_STATUS        rc;
    GT_U32 ptrVal;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);

    cpssOsFprintf(file->fd,"\nSdqSelListPtr tile %d SDQ %d\n",tileNumber,sdqNum);
    cpssOsFprintf(file->fd,"\n+------+----------+");
    cpssOsFprintf(file->fd,"\n| ind  |   value  |");
    cpssOsFprintf(file->fd,"\n+------+----------+");


    for(i =0;i<18;i++)
    {
        rc = prvCpssFalconTxqSdqSelectListPtrGet(devNum,tileNumber,sdqNum,i,&ptrVal);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqPortCreditBalanceGet\n",rc );
            return rc;
        }
        cpssOsFprintf(file->fd,"\n|%6d|0x%08x|",i,ptrVal);
    }
    cpssOsFprintf(file->fd,"\n+------+----------+\n");

    return GT_OK;

}







GT_U32 prvCpssTxqSip6DebugSdqInitDumpRegisters
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  *registersToDumpPtr,
    IN  GT_CHAR_PTR             *registerNamesPtr,
    IN  GT_U32                  *loopPtr
)
{
    GT_U32  regInd = 0;

    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].global_config,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].SDQ_Metal_Fix,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].SDQ_Interrupt_Functional_Cause,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].sdq_interrupt_functional_mask,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].SDQ_Interrupt_Summary_Mask,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].SDQ_Interrupt_Summary_Cause,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].SDQ_Interrupt_Debug_Mask,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].SDQ_Interrupt_Debug_Cause,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].QCN_Config,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].Sdq_Idle,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].Illegal_Bad_Address,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].QCN_Message_Drop_Counter,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].Queue_Dryer_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].PDQ_Message_Output_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].QCN_Message_Drop_Counter_Control,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].QCN_Message_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].QCN_Message_FIFO_Fill_Level,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].Elig_Func_Error_Capture,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.sdq[0].Select_Func_Error_Capture,regInd,0)


    return regInd;
}


GT_U32 prvCpssTxqSip6DebugPdsInitDumpRegisters
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  *registersToDumpPtr,
    IN  GT_CHAR_PTR             *registerNamesPtr,
    IN  GT_U32                  *loopPtr
)
{
    GT_U32  regInd = 0;

    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Max_PDS_size_limit_for_pdx,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Metal_Fix,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Global_PDS_CFG,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].FIFOs_Limits,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].FIFOs_DQ_Disable,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Max_PDS_size_limit_for_PB,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Max_Num_Of_Long_Queues,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Tail_Size_for_PB_Wr,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PID_Empty_Limit_for_PDX,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PID_Empty_Limit_for_PB,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].NEXT_Empty_Limit_for_PDX,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].NEXT_Empty_Limit_for_PB,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Full_Limit,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Interrupt_Debug_Cause,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Interrupt_Debug_Mask,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Last_Address_Violation,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Interrupt_Summary_Mask,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Interrupt_Summary_Cause,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Interrupt_Functional_Mask,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Interrupt_Functional_Cause,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Total_PDS_Counter,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Cache_Counter,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Idle_Register,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Free_Next_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Free_Next_FIFO_Min_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Free_PID_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Free_PID_FIFO_Min_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Data_Read_Order_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Head_Answer_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Read_Line_Buff_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Read_Req_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Write_Req_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Write_Reply_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Read_FIFO_State_Return_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Tail_Ans_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Wr_Line_Buff_Ctrl_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Wr_Line_Buff_Data_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_FIFO_State_Read_Order_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Frag_On_The_Air_Cntr_Queue_Status,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Datas_BMX_Addr_Out_Of_Range_Port,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Write_FIFO_State_Return_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PB_Read_Write_Order_FIFO_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].FIFO_State_Latency_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].FIFO_State_Latency_Min_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Long_Queue_Counter,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Long_Queue_Count_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].PDS_Cache_Count_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Total_PDS_Count_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Active_Queue_Counter,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Active_Queue_Count_Max_Peak,regInd,0)
    PRV_TXQ_REG_DUMP_BIND_TO_INDEX(Q.pds[0].Queue_Fragment_On_The_Air_Counter[0],regInd,399)

    return regInd;
}





GT_STATUS prvCpssTxqSip6DebugUnitHwStatus
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  printToFile,
    IN  GT_U32                  unitId,
    OUT CPSS_OS_FILE_TYPE_STC   **fileOutPtr
)
{
    GT_U32 numberOfTiles,i,regInd=0,tileNum,j;
    GT_U32 tileOffset,regAddr,pipeOffset,dp,regData;
    GT_U32    registersToDump[PRV_TXQ_DEBUG_MAX_REG_NUM];
    GT_CHAR_PTR registerNames [PRV_TXQ_DEBUG_MAX_REG_NUM]={0};
    GT_U32    loop[PRV_TXQ_DEBUG_MAX_REG_NUM];
    GT_STATUS rc;
    CPSS_OS_FILE_TYPE_STC  *file = NULL;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);

    if(PRV_TXQ_PDS_UNIT_ID == unitId)
    {
        regInd = prvCpssTxqSip6DebugPdsInitDumpRegisters(devNum,registersToDump,registerNames,loop);
    }
    else   if(PRV_TXQ_SDQ_UNIT_ID == unitId)
    {
        regInd = prvCpssTxqSip6DebugSdqInitDumpRegisters(devNum,registersToDump,registerNames,loop);
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected unit id  - %d ",unitId);
    }

    CPSS_NULL_PTR_CHECK_MAC(*fileOutPtr);
    *fileOutPtr = NULL;


     switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }


   if(printToFile)
   {
        *fileOutPtr = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
        if (*fileOutPtr== NULL)
        {
            cpssOsPrintf("Error: failed to open  file\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        file = *fileOutPtr;

        file->type = CPSS_OS_FILE_REGULAR;
        if(PRV_TXQ_PDS_UNIT_ID == unitId)
        {
            file->fd = cpssOsFopen(PRV_TXQ_PDS_DUMP_FILE_NAME, "w",file);
        }
        else if(PRV_TXQ_SDQ_UNIT_ID == unitId)
        {
            file->fd = cpssOsFopen(PRV_TXQ_SDQ_DUMP_FILE_NAME, "w",file);
        }

        if (file->fd==CPSS_OS_FILE_INVALID)
        {
            cpssOsPrintf("Error: failed to open  file\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        else
        {
            cpssOsPrintf("opened file for write - %s \n",(PRV_TXQ_SDQ_UNIT_ID == unitId)?PRV_TXQ_SDQ_DUMP_FILE_NAME:PRV_TXQ_PDS_DUMP_FILE_NAME);
        }
   }

   for(i=0;i<regInd;i++)
   {

        cpssOsFprintf(file->fd,"\n");
        cpssOsFprintf(file->fd,"%s \n=============================\n",registerNames[i]+9);

        for(tileNum=0;tileNum<numberOfTiles;tileNum++)
        {
            for(dp=0;dp<MAX_DP_IN_TILE(devNum);dp++)
            {
                for(j=0;j<=loop[i];j++)
                {
                    regAddr = registersToDump[i]+j*4;
                    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum/*tileId*/);
                    regAddr+=tileOffset;
                    if(PRV_TXQ_PDS_UNIT_ID == unitId)
                    {
                      pipeOffset = prvCpssSip6OffsetFromFirstInstanceGet(devNum,dp,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E);
                    }
                    else
                    {
                        pipeOffset = prvCpssSip6OffsetFromFirstInstanceGet(devNum,dp,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E);
                    }

                    regAddr+=pipeOffset;
                    rc =prvCpssHwPpReadRegister(devNum,regAddr,&regData);
                    if(rc!=GT_OK)
                    {
                       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqDebugPreqCountersDump failed for tile/dp  %d/%d \n",tileNum,dp);
                    }

                    if(loop[i]==0)
                    {
                       cpssOsFprintf(file->fd,"   tile %d , dp %d , addr 0x%08x ,data 0x%08x\n",tileNum,dp,regAddr,regData);
                    }
                    else
                    {
                        cpssOsFprintf(file->fd," ind[%d]  tile %d , dp %d , addr 0x%08x ,data 0x%08x\n",j,tileNum,dp,regAddr,regData);
                    }
                }
         }
        }
    }

   return GT_OK;
}


GT_STATUS prvCpssTxqSip6DebugPdsHwStatus
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  printToFile
)
{
    GT_STATUS rc;
    CPSS_OS_FILE_TYPE_STC  *file=NULL;

    GT_U32    numberOfTiles,i,j,k;
    GT_U32    desc[2];
    CPSS_OS_FILE_TYPE_STC   stdOut;
    stdOut.type=CPSS_OS_FILE_INVALID;



    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    rc = prvCpssTxqSip6DebugUnitHwStatus(devNum,printToFile,PRV_TXQ_PDS_UNIT_ID,&file);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssTxqSip6DebugPdsHwStatus failed\n");
    }

     if(NULL==file)
     {
         stdOut.type=CPSS_OS_FILE_STDOUT;
         stdOut.fd  = (CPSS_OS_FILE)stdout;
         file=&stdOut;
     }
     for(i=0;i<numberOfTiles;i++)
     {
         for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
         {
             rc = prvCpssDxChTxqFalconDescCounterDump(devNum,i,j,0,399,file);
             if(rc!=GT_OK)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueStatusDump failed\n");
             }

             for(k=0;k<=TXQ_PDS_MAX_DESC_NUMBER_MAC;k++)
             {
                  rc = prvCpssDxChTxqFalconPdsDataStorageGet(devNum,i,j,k,desc);
                  if(rc!=GT_OK)
                  {
                      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssTxqSip6DebugPdsHwStatus failed\n");
                  }

                  cpssOsFprintf(file->fd,"Data Storage ind %d word1 0x%08x word0 0x%08x\n",k,desc[1],desc[0]);
             }
         }
     }



    if(file!=NULL&&stdOut.type==CPSS_OS_FILE_INVALID)
    {   cpssOsPrintf("close file %s \n",PRV_TXQ_PDS_DUMP_FILE_NAME);
        cpssOsFclose(file);
    }

    return GT_OK;
}

GT_STATUS prvCpssTxqSip6DebugSdqHwStatus
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  printToFile
)
{   GT_STATUS rc;
    GT_U32    numberOfTiles,i,j;
    CPSS_OS_FILE_TYPE_STC  *file=NULL;


    rc =prvCpssTxqSip6DebugUnitHwStatus(devNum,printToFile,PRV_TXQ_SDQ_UNIT_ID,&file);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssTxqSip6DebugSdqHwStatus failed\n");
    }


    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    for(i=0;i<numberOfTiles;i++)
    {
        for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
        {
            rc = prvCpssFalconTxqSdqQueueStatusDump(devNum,i,j,0,400,file);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueStatusDump failed\n");
            }

            rc = prvCpssFalconTxqUtilsSdqQueueAttributesDump(devNum,i,j,0,400,file);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqQueueStatusDump failed\n");
            }

            rc =prvCpssFalconTxqUtilsSdqPortCreditDump(devNum,i,j,0,9,file);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsSdqPortCreditDump failed\n");
            }

            rc = prvCpssFalconTxqUtilsSdqSelListPtrDump(devNum,i,j,file);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsSdqPortCreditDump failed\n");
            }
        }
    }

    if(file!=NULL)
    {   cpssOsPrintf("close file %s \n",PRV_TXQ_SDQ_DUMP_FILE_NAME);
        cpssOsFclose(file);
    }

    return GT_OK;


}

GT_STATUS prvCpssFalconTxqDebugFindPortDump
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  dp,
    IN GT_U32  localPort
)
{
    GT_PHYSICAL_PORT_NUM portNum;
    GT_STATUS rc;

    rc = prvCpssFalconTxqUtilsPhysicalPortNumberGet(devNum,tileNum,dp,localPort,&portNum);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalPortNumberGet failed\n");
    }

    cpssOsPrintf("\nTile %d DP %d local port %d contain physical port %d\n",tileNum,dp,localPort,portNum);

    return GT_OK;

}

/**
* @internal prvCpssFalconTxqDebugBlockClientEnableAndBindSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number
*                                      valid range see in datasheet of specific device.
* @param[in] client                   - CNC client
* @param[in] enable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*                                      It is forbidden to enable update the same
*                                      block by more than one client. When an
*                                      application enables some client it is responsible
*                                      to disable all other clients it enabled before
* @param[in] index - block index
* @param[in] format - block format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqDebugBlockClientEnableAndBindSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              blockNum,
    IN  CPSS_DXCH_CNC_CLIENT_ENT            client,
    IN GT_BOOL                              enable,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT    format,
    IN GT_BOOL                              validateNotTaken
)
{
    GT_STATUS   status;
    GT_U32      i;
    GT_BOOL     updateEnable;
    GT_U64      indexRangesBmp;

    if(GT_TRUE==enable&&GT_TRUE==validateNotTaken)
    {
    /*check that no one took this block before*/
        for(i=0;i<CPSS_DXCH_CNC_CLIENT_LAST_E;i++)
        {
            if(client==(CPSS_DXCH_CNC_CLIENT_ENT)i)
            {
                continue;
            }
            status = cpssDxChCncBlockClientEnableGet(devNum, blockNum,(CPSS_DXCH_CNC_CLIENT_ENT)i,&updateEnable);
            if((GT_OK==status)&&(GT_TRUE==updateEnable))
            {

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
     }

    status = cpssDxChCncBlockClientEnableSet(devNum, blockNum,client,enable);
    if (status != GT_OK)
    {
        return status;
    }

    if(GT_TRUE==enable)
    {

        indexRangesBmp.l[0] = (1<<index);
        indexRangesBmp.l[1] = 0x0;

        status = cpssDxChCncBlockClientRangesSet(devNum, blockNum, client, &indexRangesBmp);
        if (GT_OK != status)
        {
            return status;
        }

        status = cpssDxChCncCounterFormatSet( devNum,blockNum, format);
        if (GT_OK != status)
        {
            return status;
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssFalconTxqDebugCncQueueCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    INOUT CPSS_DXCH_CNC_COUNTER_STC       *cntPtr /*should be array of 2*/
)
{
    GT_STATUS   status;
    GT_U32      queueBase = 0,blockInternalIndex;

    CPSS_DXCH_CNC_COUNTER_STC  localCnt;

    status = cpssDxChCncPortQueueGroupBaseGet(devNum,port,CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,&queueBase);
    if (GT_OK != status)
    {
        return status;
    }

    blockInternalIndex = queueBase>>9;

    cpssOsMemSet(&localCnt,0,sizeof(CPSS_DXCH_CNC_COUNTER_STC));

    status = cpssDxChCncCounterGet(devNum,PRV_TXQ_QUEUE_STAT_CNC_BLOCK_FIRST+blockInternalIndex,((queueBase&0x1FF)+queue)<<1,
                PRV_TXQ_QUEUE_STAT_CNC_BLOCK_DEFAULT_FORMAT,&localCnt);
    if (GT_OK != status)
    {
        return status;
    }

    if(NULL ==cntPtr)
    {
     cpssOsPrintf("PASS cnt.packetCount l[0] %d l[1] %d \n",localCnt.packetCount.l[0],localCnt.packetCount.l[1]);
     cpssOsPrintf("PASS cnt.byteCount l[0] %d l[1] %d \n",localCnt.byteCount.l[0],localCnt.byteCount.l[1]);
    }
    else
    {
        *cntPtr = localCnt;
        cntPtr++;
    }



    status = cpssDxChCncCounterGet(devNum,PRV_TXQ_QUEUE_STAT_CNC_BLOCK_FIRST+blockInternalIndex,(((queueBase&0x1FF)+queue)<<1)+1,
                PRV_TXQ_QUEUE_STAT_CNC_BLOCK_DEFAULT_FORMAT,&localCnt);
    if (GT_OK != status)
    {
        return status;
    }

    if(NULL ==cntPtr)
    {
        cpssOsPrintf("DROP cnt.packetCount l[0] %d l[1] %d \n",localCnt.packetCount.l[0],localCnt.packetCount.l[1]);
        cpssOsPrintf("DROP cnt.byteCount l[0] %d l[1] %d \n",localCnt.byteCount.l[0],localCnt.byteCount.l[1]);
    }
    else
    {
        *cntPtr = localCnt;
    }

    return GT_OK;
}
/**
* @internal prvCpssSip6TxqDebugCncCounterGet function
* @endinternal
*
* @brief   This function get  PASS/DROP CNC counters for specific queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                                -device number
* @param[in]    port                                       -port number
* @param[in]    queue                                    -queue number
* @param[in]    preemptiveMode                 -whether or not the queue is configured to preemptive mode
* @param[out]    passPcktsPtr                      -(pointer to)pass packet count
* @param[out]    droppedPcktsPtr               -(pointer to)drop packet count
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqDebugCncCounterGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    IN  GT_BOOL                           preemptiveMode,
    OUT GT_U32                            *passPcktsPtr,
    OUT GT_U32                            *droppedPcktsPtr,
    OUT GT_U32                            *passPcktsMsbPtr,
    OUT GT_U32                            *droppedPcktsMsbPtr
)
{
    CPSS_DXCH_CNC_COUNTER_STC cnt[2];
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(passPcktsPtr);
    CPSS_NULL_PTR_CHECK_MAC(droppedPcktsPtr);

    preemptiveMode = preemptiveMode&&TXQ_IS_PREEMPTIVE_DEVICE(devNum);


    rc = prvCpssFalconTxqDebugCncQueueCounterGet(devNum,port,queue+(preemptiveMode?PRV_CPSS_TXQ_DEBUG_PREEMPTION_OFFSET_CNC_CNS:0),cnt);
    if (GT_OK != rc)
    {
          return rc;
    }

    *passPcktsPtr =cnt[0].packetCount.l[0];
    *droppedPcktsPtr =cnt[1].packetCount.l[0];
    if(passPcktsMsbPtr)
    {
        *passPcktsMsbPtr = cnt[0].packetCount.l[1];
    }

    if(droppedPcktsMsbPtr)
    {
        *droppedPcktsMsbPtr = cnt[1].packetCount.l[1];
    }


    return GT_OK;
}

/**
* @internal prvCpssFalconTxqDebugDumpCncQueueCounter function
* @endinternal
*
* @brief   This function dump  PASS/DROP CNC counters for specific queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                              - device number
* @param[in]    port                                     -port number
* @param[in]    queue                                  - queue number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqDebugDumpCncQueueCounter
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue
)
{
    return prvCpssFalconTxqDebugCncQueueCounterGet(devNum,port,queue,NULL);
}

/**
* @internal prvCpssFalconQueueStatisticDeInitSpecificBlock function
* @endinternal
*
* @brief   This function deconfigure  CNC counters  and disable client binding for specific number of blocks.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                              - device number
* @param[in]    firstBlock                             -first block to initialize
* @param[in]    numberOfBlocks                 - number of  blocks to initialize
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconQueueStatisticDeInitSpecificBlock
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            firstBlock,
    IN  GT_U32                            numberOfBlocks
)
{
    GT_STATUS rc;
    GT_U32    i;


    if(PRV_TXQ_QUEUE_STAT_BLOCKS_NUM_MAX_REQUIRED == numberOfBlocks)
    {
        /*calculate number of required blocks*/
        rc = prvCpssSip6TxqDebugRequiredBlockNumGet(devNum,&numberOfBlocks);
        if(rc != GT_OK)
        {
          return rc;
        }
    }

    /*Disable  queue statistic counter */

    for(i=0;i<numberOfBlocks;i++)
    {
        rc = prvCpssFalconTxqDebugBlockClientEnableAndBindSet(devNum,firstBlock+i,
            CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,GT_FALSE,i,PRV_TXQ_QUEUE_STAT_CNC_BLOCK_DEFAULT_FORMAT,GT_FALSE);
        if(rc != GT_OK)
        {
          return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssFalconQueueStatisticInitSpecificBlock function
* @endinternal
*
* @brief   This function initialize  CNC counters  and enable client binding for specific number of blocks.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                              - device number
* @param[in]    firstBlock                             -first block to initialize
* @param[in]    numberOfBlocks                 - number of  blocks to initialize
* @param[in]    validateNotTaken                - whether or not to validate that block is free .Error is thrown in case the block is taken
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconQueueStatisticInitSpecificBlock
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            firstBlock,
    IN  GT_U32                            numberOfBlocks,
    IN GT_BOOL                            validateNotTaken
)
{
    GT_STATUS rc;
    GT_U32    i;

    rc = cpssDxChCncEgressQueueClientModeSet(devNum,CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E);
    if(rc != GT_OK)
    {
      return rc;
    }

    if(PRV_TXQ_QUEUE_STAT_BLOCKS_NUM_MAX_REQUIRED == numberOfBlocks)
    {
        /*calculate number of required blocks*/
        rc = prvCpssSip6TxqDebugRequiredBlockNumGet(devNum,&numberOfBlocks);
        if(rc != GT_OK)
        {
          return rc;
        }
    }

    /*Enable  queue statistic counter */

    for(i=0;i<numberOfBlocks;i++)
    {
        rc = prvCpssFalconTxqDebugBlockClientEnableAndBindSet(devNum,firstBlock+i,
            CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E,GT_TRUE,i,PRV_TXQ_QUEUE_STAT_CNC_BLOCK_DEFAULT_FORMAT,validateNotTaken);
        if(rc != GT_OK)
        {
          return rc;
        }
    }

    return rc;
}
/**
* @internal prvCpssFalconQueueStatisticInit function
* @endinternal
*
* @brief   This function initialize  CNC counters  and enable client binding.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                              - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconQueueStatisticInit
(
    IN  GT_U8                             devNum
)
{
    return prvCpssFalconQueueStatisticInitSpecificBlock(devNum,PRV_TXQ_QUEUE_STAT_CNC_BLOCK_FIRST,
        PRV_TXQ_QUEUE_STAT_BLOCKS_NUM_MAX_REQUIRED,GT_TRUE);
}

/**
* @internal prvCpssFalconQueueStatisticDeInit function
* @endinternal
*
* @brief   This function deconfigure CNC counters .Disable client binding.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                              - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconQueueStatisticDeInit
(
    IN  GT_U8                             devNum
)
{
    return prvCpssFalconQueueStatisticDeInitSpecificBlock(devNum,PRV_TXQ_QUEUE_STAT_CNC_BLOCK_FIRST,
        PRV_TXQ_QUEUE_STAT_BLOCKS_NUM_MAX_REQUIRED);
}



GT_STATUS prvCpssFalconTxqDebugDumpPoolStatus
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  poolId
)
{
    GT_STATUS rc;
    GT_U32    cnt;

    rc = prvCpssSip6TxqPoolCounterGet(devNum,poolId,&cnt);

    if(GT_OK==rc)
    {
        cpssOsPrintf("POOL [%d] occupied buffers %d \n",poolId,cnt);
    }

    return rc;
}

GT_STATUS prvCpssFalconTxqDebugDumpLevelEligPrioFunc
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  level,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  index
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    cpssOsPrintf("Format  0:HW\n");

    ret = prvSchedDebugDumpEligFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],
        level,index,0);

    cpssOsPrintf("Format  1:SW\n");

    ret = prvSchedDebugDumpEligFunc(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum],
        level,index,1);

    rc = FALCON_XEL_TO_CPSS_ERR_CODE(ret,tileNum);

    return rc;
}
/**
* @internal prvCpssSip6TxqDebugRequiredBlockNumGet function
* @endinternal
*
* @brief   This function get the number of required CNC blocks in order to track statististicks for all the queues,
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in]    devNum                              - device number
* @param[out] requiredBlockNumPtr       - (pointer to) required number of blocks
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqDebugRequiredBlockNumGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *requiredBlockNumPtr
)
{
    GT_U32    i;
    GT_U32    numOfTiles,numOfQueues,total=0;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(requiredBlockNumPtr);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            numOfTiles = 1;
            break;
       case 2:
       case 4:
            numOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);\
            break;
    }

    for(i=0;i<numOfTiles;i++)
    {
        rc = prvCpssFalconTxqUtilsMappedQueueNumGet(devNum,i,&numOfQueues);
        if(rc != GT_OK)
        {
          return rc;
        }
        total+=numOfQueues;
    }
    *requiredBlockNumPtr = (total/MAX_QUEUES_PER_CNC_BLOCK)+((total%MAX_QUEUES_PER_CNC_BLOCK)?1:0);

    return GT_OK;
}


/**
* @internal prvCpssFalconTxqDebugDumpTreeUsage function
* @endinternal
*
* @brief   Debug function that print usage of queues in the device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqDebugDumpTreeUsage
(
    IN  GT_U8                   devNum
)
{
    GT_U32    i,j,actualPnodes;
    GT_U32    numOfTiles,numOfQueues,total=0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION *tilePtr;
    PRV_CPSS_SCHED_HW_PARAMS_STC          *pdqSchedHwParamsPtr;
    PRV_CPSS_SCHED_HW_PARAMS_STC           pdqSchedHwParams;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            numOfTiles = 1;
            break;
       case 2:
       case 4:
            numOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);\
            break;
    }

    pdqSchedHwParamsPtr= &pdqSchedHwParams;

    rc = prvCpssSip6TxqPdqSchedulerHwParamsGet(devNum, pdqSchedHwParamsPtr);
    if(rc != GT_OK)
    {
      return rc;
    }

    for(i=0;i<numOfTiles;i++)
    {
        tilePtr = PRV_CPSS_TILE_HANDLE_GET_MAC(devNum,i);
        actualPnodes = 0;

        rc = prvCpssFalconTxqUtilsMappedQueueNumGet(devNum,i,&numOfQueues);
        if(rc != GT_OK)
        {
          return rc;
        }

        cpssOsPrintf("Tile %d: Number of queues allocated in scheduler %d (supported by hw %d)\n",i,tilePtr->mapping.searchTable.qNodeIndexToAnodeIndexSize,
            pdqSchedHwParamsPtr->maxQueues);
        cpssOsPrintf("Tile %d: Number of Anodes allocated in scheduler %d (supported by hw %d)\n",i,tilePtr->mapping.searchTable.aNodeIndexToPnodeIndexSize,
            pdqSchedHwParamsPtr->maxAnodes);
        cpssOsPrintf("Tile %d: Number of Pnodes allocated in scheduler %d (supported by hw %d)\n",i,tilePtr->mapping.size,
            pdqSchedHwParamsPtr->maxPorts);
        cpssOsPrintf("Tile %d: Number of Bnodes allocated in scheduler %d (supported by hw %d)\n",i,tilePtr->mapping.size,
            pdqSchedHwParamsPtr->maxBnodes);
        cpssOsPrintf("Tile %d: Number of Cnodes allocated in scheduler %d (supported by hw %d)\n",i,tilePtr->mapping.size,
            pdqSchedHwParamsPtr->maxCnodes);

        cpssOsPrintf("Tile %d: Number of mapped queues %d\n",i,numOfQueues);
        total+=numOfQueues;

        for(j=0;j<tilePtr->mapping.size;j++)
        {
            if(tilePtr->mapping.pNodeMappingConfiguration[j]!=NULL)
            {
                actualPnodes++;
            }
        }

        cpssOsPrintf("Tile %d: Number of mapped Pnodes %d out of allocated %d\n\n",i,actualPnodes,tilePtr->mapping.size);
    }

    cpssOsPrintf("Total per device : Number of mapped queues %d\n",total);

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqDebugSyncDump
* @endinternal
*
* @brief  This function print device syncronization status
*
*
* @note   APPLICABLE DEVICES:      Falcon; Hawk;AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                      - physical device number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqDebugSyncDump
(
    IN  GT_U8                   devNum
)
{
    GT_BOOL sync;
    GT_STATUS rc;
    GT_U32    tile,dp;

    rc = prvCpssFalconTxqQfcValidateDpSyncronization(devNum,&tile,&dp,&sync);
    if(rc != GT_OK)
    {
     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcValidateDpSyncronization failed");
    }
    if(GT_TRUE==sync)
    {
        cpssOsPrintf("All data paths are synced on global PFC threshold\n");
    }
    else
    {
        cpssOsPrintf("FAIL :Data path %d on tile %d  is not synced on global PFC threshold\n",dp,tile);
    }
    return rc;

}

/**
* @internal  prvCpssFalconTxqDebugQfcGlobalStatusGet  function
* @endinternal
*
* @brief   Get QFC global status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] tileNum                tile number.
* @param[in] qfcNum               QFC number
* @param[out] globalPfcStatusBmpPtr              (Pointer to)Global PFC status
* @param[out] pbLimitCrossedPtr                 (Pointer to)PB limit status
* @param[out] pdxLimitCrossedPtr               (Pointer to) PDX  limit status
* @param[out] iaLimitCrossedPtr                   (Pointer to)IA limit status
* @param[out] globalLimitCrossedBmpPtr                   (Pointer to)Global  TC limit status bitmap
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugQfcGlobalStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  qfcNum,
    OUT GT_U32                  *pbLimitCrossedPtr,
    OUT GT_U32                  *pdxLimitCrossedPtr,
    OUT GT_U32                  *iaLimitCrossedPtr,
    OUT GT_U32                  *globalTcLimitCrossedBmpPtr
)
{
    GT_STATUS                                rc;
    GT_U32                                   globalPfcStatusBmp;


    rc = prvCpssFalconTxqQfcPfcStatusGet(devNum,tileNum,qfcNum,&globalPfcStatusBmp,NULL,
        NULL,NULL);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcStatusGet failed\n");

    }

     rc = prvCpssFalconTxqQfcPfcGlobalStatusParse(devNum,globalPfcStatusBmp,pbLimitCrossedPtr,pdxLimitCrossedPtr,
        iaLimitCrossedPtr,globalTcLimitCrossedBmpPtr);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcGlobalStatusParse failed\n");

    }

    return GT_OK;

}


/**
* @internal  prvCpssFalconTxqDebugQfcPortStatusGet  function
* @endinternal
*
* @brief   Get port specific QFC status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note  NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               PP's device number.
* @param[in] tileNum                tile number.
* @param[in] qfcNum               QFC number
* @param[in] localPortNum              Local port number
* @param[out] portPfcStatusPtr                 (Pointer to)Port congestion status
* @param[out] portTcPfcStatusBmpPtr               (Pointer to) Port/TC  congestion status
*
* @retval GT_OK                    -        on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              - if write failed
*/
GT_STATUS prvCpssFalconTxqDebugQfcPortStatusGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  qfcNum,
    IN  GT_U32                  localPortNum,
    OUT  GT_U32                 *portPfcStatusPtr,
    OUT  GT_U32                 *portTcPfcStatusBmpPtr
)
{
    GT_STATUS                                rc;
    GT_U32                                   portPfcStatusBmp[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
    GT_U32                                   portTcPfcStatusBmp[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];
    GT_U32                                   portLimitCrossed[CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_DEV_LESS_MAC];


    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    CPSS_NULL_PTR_CHECK_MAC(portPfcStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(portTcPfcStatusBmpPtr);

    rc = prvCpssFalconTxqQfcPfcStatusGet(devNum,tileNum,qfcNum,NULL,portPfcStatusBmp,
        portTcPfcStatusBmp,NULL);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcStatusGet failed\n");

    }

    rc = prvCpssFalconTxqQfcPfcPortStatusParse(devNum,portPfcStatusBmp,GT_FALSE,portLimitCrossed);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcPortStatusParse failed\n");

    }

    *portPfcStatusPtr = portLimitCrossed[localPortNum];

    rc = prvCpssFalconTxqQfcPfcPortStatusParse(devNum,portTcPfcStatusBmp,GT_TRUE,portLimitCrossed);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqQfcPfcPortStatusParse failed\n");

    }

    *portTcPfcStatusBmpPtr =portLimitCrossed[localPortNum];

    return GT_OK;

}




GT_STATUS prvCpssFalconTxqPfccCfgTableEntryRawGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 index,
    OUT GT_U32 *numOfBubblesPtr,
    OUT GT_U32 *metaDataPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PFCC_CFG_ENTRY_STC entry;

    CPSS_NULL_PTR_CHECK_MAC(numOfBubblesPtr);

    rc = prvCpssFalconTxqPfccCfgTableEntryGet(devNum,tileNum,index,&entry);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPfccCfgTableEntryGet fail for entry %d",index);
    }

    if(entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_BUBBLE)
    {
        *numOfBubblesPtr =entry.numberOfBubbles;
    }
    else
    {
        *numOfBubblesPtr = 0;
        if(entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC)
        {
            *metaDataPtr =0x1|entry.globalTc<<8;
        }
        else if(entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_GLOBAL_TC_HR)
        {
            *metaDataPtr =0x3|entry.globalTc<<8;
        }
        else if(entry.entryType == PRV_CPSS_PFCC_CFG_ENTRY_TYPE_PORT)
        {
            *metaDataPtr =0x0|entry.pfcMessageTrigger<<2|entry.sourcePort<<10|entry.tcBitVecEn<<24;
        }
    }

    return rc;


}




GT_U32 prvCpssTxqSip6DebugBrInitRegisters
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                  *registersToDumpPtr,
    IN  GT_CHAR_PTR             *registerNamesPtr
)
{
    GT_U32  regInd = 0;

    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.commandConfig,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_commandConfig,regInd);

    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl01QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl01QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl23QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl23QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl23QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl45QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl67QuantaThresh,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl67QuantaThresh,regInd);

    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl01PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl01PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl23PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl23PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl45PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl45PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.cl67PauseQuanta,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_cl67PauseQuanta,regInd);

    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.macAddr0,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_macAddr0,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.macAddr1,regInd);
    PRV_BR_BIND_TO_INDEX(devNum,portMacNum,_MAC.EMAC.emac_macAddr1,regInd);

    return regInd;
}



GT_STATUS prvCpssTxqSip6DebugBrSyncDump
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
)
{
    GT_U32 regAddr,regData[2];
    GT_U32 iterator;
    GT_STATUS rc;
    GT_U32    registersToDump[PRV_TXQ_DEBUG_MAX_REG_NUM],i;
    GT_CHAR_PTR registerNames [PRV_TXQ_DEBUG_MAX_REG_NUM]={0};


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

   iterator =  prvCpssTxqSip6DebugBrInitRegisters(devNum,portNum,registersToDump,registerNames);
   for(i=0;i<iterator;i+=2)
   {

      regAddr = registersToDump[i];
      if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
      {
        cpssOsPrintf("Preemption is not supported on port %d\n",portNum);
        break;/*we are done*/
      }

      rc =prvCpssHwPpReadRegister(devNum,regAddr,regData);
      if(rc!=GT_OK)
      {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
      }

      regAddr = registersToDump[i+1];

      rc =prvCpssHwPpReadRegister(devNum,regAddr,regData+1);
      if(rc!=GT_OK)
      {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
      }

      if(regData[0]==regData[1])
      {
        cpssOsPrintf("Same content for %s 0x%x\n",registerNames[i]+5,regData[0]);
      }
      else
      {
        cpssOsPrintf("Different content for %s [e 0x%x vs p 0x%x]\n",registerNames[i]+5,regData[0],regData[1]);
      }


    }
    return GT_OK;

}


GT_STATUS prvCpssTxqSip6DebugCiderVersionHandler
(
    IN  CPSS_PP_FAMILY_TYPE_ENT              devFamily,
    IN  PRV_CPSS_TXQ_CIDER_INFO_ACTION_ENT   action,
    OUT GT_U32                               *outParamPtr
)
{
    GT_U32 i;
    GT_STATUS rc = GT_OK;
    GT_U32 size;

    PRV_CPSS_TXQ_CIDER_INFO_STC initInfo [] =
        {       /*AC5P*/
                {
                    CPSS_PP_FAMILY_DXCH_AC5P_E,1,7,7
                },
                /*AC5X*/
                {
                    CPSS_PP_FAMILY_DXCH_AC5X_E,2,7,7
                },
                /*Harrier*/
                {
                    CPSS_PP_FAMILY_DXCH_HARRIER_E,3,4,4
                },
                /*Ironman*/
                {
                    CPSS_PP_FAMILY_DXCH_IRONMAN_E,4,3,4
                }
        };

    PRV_CPSS_TXQ_CIDER_INFO_STC *infoPtr =PRV_TXQ_CIDER_INFO_GET();

    size = sizeof(initInfo)/sizeof(initInfo[0]);

    if(GT_FALSE == PRV_TXQ_CIDER_INFO_INIT_GET())
    {
        cpssOsMemCpy(infoPtr,initInfo,sizeof(PRV_CPSS_TXQ_CIDER_INFO_STC)*size);
        PRV_TXQ_CIDER_INFO_INIT_MARK_DONE(GT_TRUE);
    }

    if(PRV_CPSS_TXQ_CIDER_INFO_ACTION_CHECK_LAST_ENT==action)
    {
        CPSS_NULL_PTR_CHECK_MAC(outParamPtr);
    }

    for(i=0;i<sizeof(initInfo)/sizeof(initInfo[0]);i++)
    {
        if(initInfo[i].family==devFamily)
        {
            switch(action)
            {
                case PRV_CPSS_TXQ_CIDER_INFO_ACTION_SHOW_ENT:
                    cpssOsPrintf("Current version 7.%d.%d\n",infoPtr[i].revision,infoPtr[i].currentVersion);
                    cpssOsPrintf("Last version 7.%d.%d\n",infoPtr[i].revision,infoPtr[i].lastVersion);
                    break;
                case PRV_CPSS_TXQ_CIDER_INFO_ACTION_UPGRADE_ENT:
                    if(infoPtr[i].currentVersion<infoPtr[i].lastVersion)
                    {
                        infoPtr[i].currentVersion++;
                        cpssOsPrintf("Current version 7.%d.%d\n",infoPtr[i].revision,infoPtr[i].currentVersion);
                    }
                    else
                    {
                        cpssOsPrintf("Already at last version %d\n",infoPtr[i].lastVersion);
                    }
                    break;
                case PRV_CPSS_TXQ_CIDER_INFO_ACTION_CHECK_LAST_ENT:
                    if(infoPtr[i].currentVersion==infoPtr[i].lastVersion)
                    {
                        *outParamPtr =1;
                    }
                    else
                    {
                        *outParamPtr =0;
                    }
                    break;
                default:
                    cpssOsPrintf("Unknown action %d\n",action);
                    rc=GT_FAIL;
                    break;
            }
            break;
        }
    }

    return rc;
}

GT_STATUS prvCpssTxqSip6DebugCiderVersionIsLast
(
    IN  GT_U8                   devNum,
    OUT GT_BOOL                 *isLastVersionPtr
)
{
    GT_STATUS rc;
    GT_U32 isLast = 0;

    CPSS_NULL_PTR_CHECK_MAC(isLastVersionPtr);

    rc = prvCpssTxqSip6DebugCiderVersionHandler(PRV_CPSS_PP_MAC(devNum)->devFamily,
        PRV_CPSS_TXQ_CIDER_INFO_ACTION_CHECK_LAST_ENT,&isLast);
    if(rc!=GT_OK)
    {
        return rc;
    }

    *isLastVersionPtr = (isLast==1)?GT_TRUE:GT_FALSE;

    return rc;

}

GT_STATUS prvCpssTxqSip6DebugCiderVersionIncrease
(
    IN  GT_CHAR *              input
)
{
    CPSS_PP_FAMILY_TYPE_ENT              devFamily;

   if(0==cpssOsStrCmp("CPSS_PP_FAMILY_DXCH_AC5P_E",input))
   {
        devFamily = CPSS_PP_FAMILY_DXCH_AC5P_E;
   }
   else if (0==cpssOsStrCmp("CPSS_PP_FAMILY_DXCH_AC5X_E",input))
   {
        devFamily = CPSS_PP_FAMILY_DXCH_AC5X_E;
   }
   else if (0==cpssOsStrCmp("CPSS_PP_FAMILY_DXCH_HARRIER_E",input))
   {
        devFamily = CPSS_PP_FAMILY_DXCH_HARRIER_E;
   }
   else if (0==cpssOsStrCmp("CPSS_PP_FAMILY_DXCH_IRONMAN_E",input))
   {
        devFamily = CPSS_PP_FAMILY_DXCH_IRONMAN_E;
   }
   else
   {
        cpssOsPrintf("Unsupported family %d\n",input);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
   }

    return prvCpssTxqSip6DebugCiderVersionHandler(devFamily,PRV_CPSS_TXQ_CIDER_INFO_ACTION_UPGRADE_ENT,NULL);
}


GT_STATUS prvCpssTxqDebugSdqQueueAttributesSet
(
    IN GT_U8  devNum,
    IN GT_U32 portNum,
    IN GT_U32 queueNumber,
    IN GT_U32 negativeCreditThreshold,
    IN GT_U32 lowCreditTheshold,
    IN GT_U32 highCreditThreshold
)
{
    GT_STATUS rc;
    GT_U32    tileNum,dpNum,localdpPortNum,dummy,pNodeNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_BOOL   isCascade;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    anodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *    pnodePtr;
    GT_U32                              queueToSet;
    GT_U32 regValue[2],value,size;


    /*According to Cider : "This threshold is 256B granule and is signed number defined by the msb"*/

    if((highCreditThreshold>>PRV_TH_GRANULARITY_SHIFT)>= (GT_U32)(1<<TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((lowCreditTheshold>>PRV_TH_GRANULARITY_SHIFT)>= (GT_U32)(1<<TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((negativeCreditThreshold>>PRV_TH_GRANULARITY_SHIFT)>= (GT_U32)(1<<TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsPrintf("negativeCreditThreshold %d\n",negativeCreditThreshold);
    cpssOsPrintf("lowCreditTheshold       %d\n",lowCreditTheshold);
    cpssOsPrintf("highCreditThreshold     %d\n",highCreditThreshold);

    /*Let's find the port*/
    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }


    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascade, NULL);
        if (rc != GT_OK)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ", portNum);
        }

        if (isCascade == GT_TRUE)
        {
            pNodeNum    = dpNum + (MAX_DP_IN_TILE(devNum)) * localdpPortNum;
            PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pnodePtr,devNum,tileNum,pNodeNum);
            queueToSet = pnodePtr->aNodelist[0].queuesData.queueBase + queueNumber;

        }
        else
        {
            rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &dummy);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
            }
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
            }
            queueToSet = anodePtr->queuesData.queueBase+ queueNumber;

        }

        cpssOsPrintf("Tile %d DP %d queueToSet %d\n",tileNum,dpNum,queueToSet);

        rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+dpNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChReadTableEntry  failed for portNum  %d  ", portNum);
        }

        cpssOsPrintf("Current register value word0 0x%x word1 0x%x\n",regValue[0],regValue[1]);

        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET),
                                TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE),
                                negativeCreditThreshold>PRV_TH_GRANULARITY_SHIFT);



        value = U32_GET_FIELD_MAC(lowCreditTheshold>>PRV_TH_GRANULARITY_SHIFT,0,32 - TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET));
        size = 32 - TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET);

        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET),
                                   size,
                                   value);

        value = U32_GET_FIELD_MAC(lowCreditTheshold>>PRV_TH_GRANULARITY_SHIFT,
                                   32 - TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET),
                                   TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE) - (32 - TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET)));

       size = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE - (32 - TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET));

       U32_SET_FIELD_MASKED_MAC(regValue[1],0,
                                   size,
                                  value);



       U32_SET_FIELD_MASKED_MAC(regValue[1],TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET)-32,
                                  TXQ_SDQ_FIELD_GET(devNum,QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE),
                                  highCreditThreshold>>PRV_TH_GRANULARITY_SHIFT);



       rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+dpNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);

       if (rc != GT_OK)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChWriteTableEntry  failed for portNum  %d  ", portNum);
       }

        rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+dpNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChReadTableEntry  failed for portNum  %d  ", portNum);
        }

        cpssOsPrintf("After modification register value word0 0x%x word1 0x%x\n",regValue[0],regValue[1]);

    }

    return GT_OK;
}


GT_STATUS prvCpssTxqSip6RxDMA2LocalPhysDump
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  rxDmaNum
)
{
    GT_STATUS rc;
    GT_U32      regAddr;            /* register's address */
    GT_U32 dpIdx,localDmaNum,physPort;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);


    physPort = CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS;

    rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,rxDmaNum,/*OUT*/&dpIdx,&localDmaNum);
    if (GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* it is enough to get data from single pipe mapping, since them both are configured same */

    regAddr = PRV_DXCH_REG1_UNIT_SIP6_RXDMA_MAC(devNum).configs.channelConfig.channelToLocalDevSourcePort[rxDmaNum];

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, 0, regAddr,
                                            PRV_CPSS_DXCH_RXDMA_2_PHYS_OFFS_CNS,
                                            PRV_CPSS_DXCH_RXDMA_2_PHYS_LEN_CNS(devNum),
                                            &physPort);

    if (rc == GT_OK)
    {
      cpssOsPrintf("RxDma %d is mapped to physical port %d\n",rxDmaNum,physPort);
    }

    return rc;
}

GT_STATUS prvCpssSip6TxqUtilsDataPathEventLogEnableSet
(
   IN GT_U8 devNum,
   IN GT_BOOL enable
)
{
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.dataPathEventLog = enable;
    return GT_OK;
}


GT_BOOL prvCpssSip6TxqUtilsDataPathEventLogGet
(
   GT_U8 devNum
)
{
    return PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.dataPathEventLog;
}


GT_STATUS prvCpssSip6TxqTimerTest
(
   GT_VOID
)
{
      GT_STATUS rc;
      GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec;

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    if(rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    cpssOsTimerWkAfter(1);

    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }

    cpssOsPrintf("Sleep  1ms finished after: %d sec., %d nanosec.\n",seconds, nanoSec);

    return GT_OK;

}

/**
* @internal prvCpssSip6TxqDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in txQ modules
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqDebugInterruptDisableSet
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32    numberOfTiles,numberOfDp;
    GT_U32  i,j;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    numberOfDp = MAX_DP_IN_TILE(devNum);

    for(i=0;i<numberOfTiles;i++)
    {
        rc = prvCpssSip6TxqPfccDebugInterruptDisableSet(devNum,i);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPfccDebugInterruptDisableSet fail for tile - %d",i);
        }

        rc = prvCpssSip6TxqPdxDebugInterruptDisableSet(devNum,i);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPdxDebugInterruptDisableSet fail for tile - %d",i);
        }

        rc =prvCpssSip6TxqPsiDebugInterruptDisableSet(devNum,i);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPsiDebugInterruptDisableSet fail for tile - %d",i);
        }

        for(j=0;j<numberOfDp;j++)
        {
            rc = prvCpssSip6TxqSdqDebugInterruptDisableSet(devNum,i,j);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqSdqDebugInterruptDisableSet fail for tile - %d data path %d",i,j);
            }

            rc = prvCpssSip6TxqPdsDebugInterruptDisableSet(devNum,i,j);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPdsDebugInterruptDisableSet fail for tile - %d data path %d",i,j);
            }

            rc = prvCpssSip6TxqQfcDebugInterruptDisableSet(devNum,i,j);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqQfcDebugInterruptDisableSet fail for tile - %d data path %d",i,j);
            }
        }
    }

    return GT_OK;
}
/**
* @internal prvCpssSip6TxqDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in txQ modules
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - device number
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqDebugInterruptDump
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32    numberOfTiles,numberOfDp;
    GT_U32  i,j;
    GT_U32    functional,debug,pac[2];
    GT_BOOL   printSchedErrors = GT_FALSE;

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
         case 0:
         case 1:
             numberOfTiles = 1;
             break;
        case 2:
        case 4:
             numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
             break;
    }

    numberOfDp = MAX_DP_IN_TILE(devNum);

    for(i=0;i<numberOfTiles;i++)
    {
        /*PSI*/
        rc = prvCpssSip6TxqPsiDebugInterruptGet(devNum,i,&functional);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPsiDebugInterruptGet fail for tile - %d ",i);
        }
        if(0!=functional)
        {
            cpssOsPrintf("Tile %d PSI Interrupt cause 0x%08x\n",i,functional);

            if(functional&(prvCpssSip6TxqPsiSchedErrorBmp(devNum)))
            {
                printSchedErrors = GT_TRUE;
            }
        }

         /*PFCC*/
        rc = prvCpssSip6TxqPfccDebugInterruptGet(devNum,i,&functional);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPfccDebugInterruptGet fail for tile - %d ",i);
        }
        if(0!=functional)
        {
            cpssOsPrintf("Tile %d PFCC Interrupt cause 0x%08x\n",i,functional);
        }

        /*PDX*/
        rc = prvCpssSip6TxqPdxDebugInterruptGet(devNum,i,&functional,&debug,&pac[0],&pac[1]);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPsiDebugInterruptGet fail for tile - %d ",i);
        }
        if(0!=functional)
        {
            cpssOsPrintf("Tile  %d PDX Functional Interrupt cause 0x%08x\n",i,functional);
        }

        if(0!=debug)
        {
            cpssOsPrintf("Tile  %d PDX Debug Interrupt cause 0x%08x\n",i,debug);
        }

        if(0!=pac[0])
        {
            cpssOsPrintf("Tile %d PDX PAC 0 Interrupt cause 0x%08x\n",i,pac[0]);
        }

        if(0!=pac[1])
        {
            cpssOsPrintf("Tile %d PDX PAC 1 Interrupt cause 0x%08x\n",i,pac[1]);
        }

        /*SDQ*/
        for(j=0;j<numberOfDp;j++)
        {
            rc = prvCpssSip6TxqSdqDebugInterruptGet(devNum,i,j,&functional,&debug);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqSdqDebugInterruptGet fail for tile - %d data path %d",i,j);
            }

            if(0!=functional)
            {
                cpssOsPrintf("Tile %d SDQ %d Functional 0x%08x\n",i,j, functional);

                if(functional&(prvCpssSip6TxqSdqUnMapPortErrorBmp(devNum)))
                {
                    rc = prvCpssSip6TxqSdqErrorCaptureDump(devNum,i,j);
                    if(rc!=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPfccDebugInterruptGet fail for tile - %d ",i);
                    }
                }
            }
            if(0!=debug)
            {
                cpssOsPrintf("Tile %d SDQ %d Debug 0x%x\n",i,j, debug);
            }
        }

        /*PDS*/
        for(j=0;j<numberOfDp;j++)
        {
            rc = prvCpssSip6TxqPdsDebugInterruptGet(devNum,i,j,&functional,&debug);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqSdqDebugInterruptGet fail for tile - %d data path %d",i,j);
            }

            if(0!=functional)
            {
                cpssOsPrintf("Tile %d PDS %d Functional 0x%08x\n",i,j, functional);
            }
            if(0!=debug)
            {
                cpssOsPrintf("Tile %d PDS %d Debug 0x%08x\n",i,j, debug);
            }
        }
        /*QFC*/
        for(j=0;j<numberOfDp;j++)
        {
            rc = prvCpssSip6TxqQfcDebugInterruptGet(devNum,i,j,&functional,&debug);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqQfcDebugInterruptGet fail for tile - %d data path %d",i,j);
            }

            if(0!=functional)
            {
                cpssOsPrintf("Tile %d QFC %d Functional 0x%08x\n",i,j, functional);
            }
            if(0!=debug)
            {
                cpssOsPrintf("Tile %d QFC %d Debug 0x%08x\n",i,j, debug);
            }
        }
    }

    if(GT_TRUE == printSchedErrors)
    {
        cpssOsPrintf("\nScheduler errors \n");
        rc = prvCpssDxChTxqFalconDumpSchedErrors(devNum);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChTxqFalconDumpSchedErrors fail.");
        }
    }
    return GT_OK;
}


GT_STATUS prvCpssSip6TxqUtilsPortDwrrBmpSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN GT_U32 bmp
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32 pdqNum;
    int    ret;

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    pdqNum = aNodePtr->queuesData.tileNum;

    ret = prvSchedLowLevelAnodeDwrrSet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodePtr->aNodeIndex,bmp);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    return rc;
}


GT_STATUS prvCpssSip6TxqUtilsPortDwrrBmpDump
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodePtr;
    GT_U32 pdqNum;
    int    ret;
    uint8_t bmp;

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    pdqNum = aNodePtr->queuesData.tileNum;

    ret = prvSchedLowLevelAnodeDwrrGet(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[pdqNum],aNodePtr->aNodeIndex,&bmp);
    rc  = FALCON_XEL_TO_CPSS_ERR_CODE(ret, pdqNum);

    if(rc == GT_OK)
    {
        cpssOsPrintf("Physical %d - Tile %d A node %d DWRR bmp  0x%x\n",physicalPortNum,pdqNum, aNodePtr->aNodeIndex,bmp);
    }

    return rc;
}

GT_STATUS prvCpssTxqSip6DropReasonInit
(
    IN  GT_U8                   devNum,
    IN CPSS_PACKET_CMD_ENT       cmd,
    IN CPSS_NET_RX_CPU_CODE_ENT  tailDropCode
)
{
    GT_STATUS rc;
    GT_U64                     indexRangesBmp;
    GT_BOOL   updateEnable;
    GT_U32    i;

    indexRangesBmp.l[0] = 0x1;
    indexRangesBmp.l[1] = 0x0;


    /*check that no one took this block before*/
    for(i=0;i<CPSS_DXCH_CNC_CLIENT_LAST_E;i++)
    {
        if(CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E==(CPSS_DXCH_CNC_CLIENT_ENT)i)
        {
            continue;
        }
        rc = cpssDxChCncBlockClientEnableGet(devNum, PRV_TXQ_DROP_REASON_CNC_BLOCK,(CPSS_DXCH_CNC_CLIENT_ENT)i,&updateEnable);
        if((GT_OK==rc)&&(GT_TRUE==updateEnable))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    rc = cpssDxChCncBlockClientEnableSet(devNum,PRV_TXQ_DROP_REASON_CNC_BLOCK,CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
        GT_TRUE);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientEnableSet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    rc = cpssDxChCncBlockClientRangesSet(devNum,PRV_TXQ_DROP_REASON_CNC_BLOCK,CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,&indexRangesBmp);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientRangesSet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    rc = cpssDxChPortTxTailDropCommandSet(devNum,cmd /*CPSS_PACKET_CMD_DROP_HARD_E*/,tailDropCode/*CPSS_NET_FIRST_USER_DEFINED_E*/);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientRangesSet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    return GT_OK;
}
GT_STATUS prvCpssTxqSip6DropReasonDeInit
(
    IN  GT_U8                   devNum
)
{
    GT_STATUS rc;

    rc = cpssDxChCncBlockClientEnableSet(devNum,PRV_TXQ_DROP_REASON_CNC_BLOCK,CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
        GT_FALSE);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientEnableSet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    /*return to default*/
    rc = cpssDxChPortTxTailDropCommandSet(devNum,CPSS_PACKET_CMD_DROP_HARD_E,CPSS_NET_FIRST_USER_DEFINED_E);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientRangesSet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    return GT_OK;
}

GT_STATUS prvCpssTxqSip6DropReasonCountGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  reason,
    OUT GT_U32                  *packetCntLsbPtr,
    OUT GT_U32                  *packetCntMsbPtr,
    OUT GT_U32                  *counterPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_CNC_COUNTER_STC  cnt;
    GT_U32                     counterBase;
    CPSS_PACKET_CMD_ENT        cmd;
    CPSS_NET_RX_CPU_CODE_ENT   tailDropCode;

    CPSS_NULL_PTR_CHECK_MAC(packetCntLsbPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetCntMsbPtr);
    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    if(reason>6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "reason  %d >6",reason);
    }

    /*get currently configured command*/

    rc = cpssDxChPortTxTailDropCommandGet(devNum,&cmd ,&tailDropCode);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChPortTxTailDropCommandGet  failed\n");
    }


    rc = cpssDxChPortTxTailDropCommandGet(devNum,&cmd /*CPSS_PACKET_CMD_DROP_HARD_E*/,&tailDropCode/*CPSS_NET_FIRST_USER_DEFINED_E*/);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncBlockClientRangesSet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    rc = prvCpssDxChNetIfCpuToDsaCode(tailDropCode, &counterBase);
    if (rc != GT_OK)
    {
        return rc;
    }

    *counterPtr = counterBase+(cmd<<8)+reason;

    rc = cpssDxChCncCounterGet(devNum,PRV_TXQ_DROP_REASON_CNC_BLOCK, *counterPtr,
                CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E,&cnt);
    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChCncCounterGet  failed for blockNum  %d",PRV_TXQ_DROP_REASON_CNC_BLOCK);
    }

    *packetCntLsbPtr = cnt.packetCount.l[0];
    *packetCntMsbPtr = cnt.packetCount.l[1];

    return GT_OK;
}

/**
 * @internal prvCpssSip6TxqUtilsDumpSchedDbProfile function
 * @endinternal
 *
 * @brief   Print  schedule profile attributes
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 */
GT_STATUS prvCpssSip6TxqUtilsDumpSchedDbProfile
(
    IN GT_U8 devNum,
    IN GT_U32 profile
)
{
    GT_U32 j;

     cpssOsPrintf("Profile %2d\n", profile + 1 /*profile start from 1*/);

     for (j = 0; j < 8; j++)
     {
         cpssOsPrintf(" WRR enable word %d  0x%08x\n",j, PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[profile].wrrEnableBmp[j]);
         cpssOsPrintf(" WRR group  word %d  0x%08x\n\n",j, PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[profile].wrrGroupBmp[j]);
     }

     cpssOsPrintf("\n WRR weights :\n");

     for (j = 0; j < 256; j+=4)
     {
         cpssOsPrintf("tc %d  -> %d ,tc %d  -> %d,tc %d  -> %d,tc %d  -> %d\n",j,
             PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[profile].weights[j] / CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum),
             j+1,PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[profile].weights[j+1] / CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum),
             j+2,PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[profile].weights[j+2] / CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum),
             j+3,PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[profile].weights[j+3] / CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum));
     }



    return GT_OK;
}


/**
 * @internal prvCpssSip6TxqUtilsDumpSchedDbProfiles function
 * @endinternal
 *
 * @brief   Print  schedule profile attributes
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 */
GT_STATUS prvCpssSip6TxqUtilsDumpSchedDbProfiles
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    GT_U32 i ;

    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM; i++)
    {
      rc = prvCpssSip6TxqUtilsDumpSchedDbProfile(devNum,i);
      if(rc!=GT_OK)
      {
          break;
      }
    }

    return rc;
}

/**
 * @internal prvCpssSip6TxqUtilsDumpBindPortsToSchedDb function
 * @endinternal
 *
 * @brief   Print port to schedule profile binding
 *
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - physical device number
 */
GT_STATUS prvCpssSip6TxqUtilsDumpBindPortsToSchedDb
(
    IN GT_U8 devNum
)
{
    GT_U32 i, j, tmp, portNum, shift, num = 0;
    for (i = 0; i < CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM; i++)
    {
        cpssOsPrintf(" \nProfile %d binded ports: \n", i + 1 /*Profiles start from 1*/);
        for (j = 0; j < 32; j++)
        {
            tmp = PRV_CPSS_DXCH_PP_MAC(devNum)->port.schedProfiles[i].bindedPortsBmp[j];
            if (tmp )
            {
                shift = 0;
                while (tmp)
                {
                    if (tmp & 0x1)
                    {
                        portNum = j * 32 + shift;
                        cpssOsPrintf(" %3d ,", portNum);
                        num++;
                        if (num == 8)
                        {
                            cpssOsPrintf(" \n", portNum);
                            num = 0;
                        }
                    }
                    tmp >>= 1;
                    shift++;
                }
            }
        }
    }
    return GT_OK;
}
GT_STATUS prvCpssSip6TxqUtilsDumpTxQueueOffsetMap
(
    IN GT_U8 devNum
)
{
    GT_U32 i,queueOffset4bit;
    GT_STATUS rc;

    if (GT_TRUE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        for(i=0;i<256;i++)
        {
            rc = cpssDxChPortTxQueueOffsetMapGet(devNum,i,&queueOffset4bit);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet failed ");
            }
            cpssOsPrintf("[%3d] %d \n", i,queueOffset4bit);
        }
    }

    return GT_OK;
}


GT_STATUS prvCpssTxqSip6DebugFailureCountSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_TXQ_FAILURE_TYPE_ENT failureType,
    IN  PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_ENT action
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr;
    GT_STATUS                       rc;
    /*find Pnode*/

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,NULL,&pNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet failed ");
    }

    if(pNodePtr)
    {
        switch(failureType)
        {
            case PRV_CPSS_TXQ_FAILURE_QUEUE_DRAIN_E:
                if(action == PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_INCREASE_E)
                {
                    if(pNodePtr->alertCounters.txQDrainFailureCount<PRV_TXQ_SIP_6_DBG_SATURATION_VALUE)
                    {
                        pNodePtr->alertCounters.txQDrainFailureCount++;
                    }
                }
                else
                {
                    pNodePtr->alertCounters.txQDrainFailureCount = 0;
                }
                break;
            case PRV_CPSS_TXQ_FAILURE_MAC_FIFO_DRAIN_E:
                if(action == PRV_CPSS_TXQ_FAILURE_COUNT_ACTION_INCREASE_E)
                {
                    if(pNodePtr->alertCounters.txQDrainFailureCount<PRV_TXQ_SIP_6_DBG_SATURATION_VALUE)
                    {
                        pNodePtr->alertCounters.macFifoDrainFailureCount++;
                    }
                }
                else
                {
                    pNodePtr->alertCounters.macFifoDrainFailureCount = 0;
                }
                break;
            default:
                break;
        }
     }

    return GT_OK;
}

GT_STATUS prvCpssTxqSip6DebugFailureCountDump
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr;
    GT_STATUS                       rc;
    /*find Pnode*/

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,NULL,&pNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet failed ");
    }

    if(pNodePtr)
    {
        cpssOsPrintf("Pnode = %d (CSCD %d)\n",pNodePtr->pNodeIndex,pNodePtr->isCascade);
        cpssOsPrintf("txQDrainFailureCount = %d\n",pNodePtr->alertCounters.txQDrainFailureCount);
        cpssOsPrintf("macFifoDrainFailureCount = %d\n",pNodePtr->alertCounters.macFifoDrainFailureCount);
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqFalconDescCounterDump function
 * @endinternal
 *
 * @brief   Print TxQ descriptor counters for specific tile and PDS :
 *                1. Per port decriptor count at PDX PAC unit (Port numbering is local to PDS).
 *                2. Total PDS decriptor count.
 *                3. Per queue decriptor count
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
 *
 * @param[in] devNum                   - physical device number
 * @param[in] tileNum                   - tile number (0..3)
 * @param[in] pdsNum                   - pds number  (0..7)
 * @param[in] queueStart                 - first queue to dump number  (0..399)
 * @param[in] queueEnd                   - last queue to dump number  (0..399)
 *
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssDxChTxqFalconDescCounterDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd,
    IN CPSS_OS_FILE_TYPE_STC * file
)
{
    GT_U32                          pdxPacDescCount;
    GT_U32                          pdsDescCount, pdsDescCacheCount;
    GT_U32                          i;
    GT_STATUS                       rc;
    PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC    perQueueDescCount;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);
    cpssOsFprintf(file->fd, "\nDescriptor counters for PDX %d PDS %d (PDX_PAC_X_PORT_DESC_COUNTER) \n", tileNum, pdsNum);
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        cpssOsFprintf(file->fd, "\n+------+--------------------+");
        cpssOsFprintf(file->fd, "\n| Port | PDX PAC desc count |");
        cpssOsFprintf(file->fd, "\n+------+--------------------+");
        for (i = 0; i < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); i++)
        {
            rc = prvCpssDxChTxqFalconPdxPacPortDescCounterGet(devNum, tileNum, CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum) * pdsNum + i, &pdxPacDescCount);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdxPacPortDescCounterGet  failed for pdx  %d port %d ", tileNum, i);
            }
            cpssOsFprintf(file->fd, "\n|%6d|%20d|", i, pdxPacDescCount);
            cpssOsFprintf(file->fd, "\n+------+--------------------+");
        }
    }
    cpssOsFprintf(file->fd, "\nTotal_PDS_Counter/PDS_Cache_Counter for PDX %d PDS %d  \n", tileNum, pdsNum);
    cpssOsFprintf(file->fd, "\n+----+------------+----------------------------+");
    cpssOsFprintf(file->fd, "\n| PDS| desc total | desc cache(not include PB) |");
    cpssOsFprintf(file->fd, "\n+----+------------+----------------------------+");
    rc = prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet(devNum, tileNum, pdsNum, &pdsDescCount);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ", tileNum, pdsNum);
    }
    rc = prvCpssDxChTxqFalconPdsCacheDescCounterGet(devNum, tileNum, pdsNum, &pdsDescCacheCount);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsCacheDescCounterGet  failed for tile  %d pds %d ", tileNum, pdsNum);
    }
    cpssOsFprintf(file->fd, "\n|%4d|%12d|%28d|", pdsNum, pdsDescCount, pdsDescCacheCount);
    cpssOsFprintf(file->fd, "\n+----+------------+----------------------------+\n");
    cpssOsFprintf(file->fd, "\nPER_QUEUE_COUNTERS for PDX %d PDS %d  \n", tileNum, pdsNum);
    cpssOsFprintf(file->fd, "\n+----+------------+------------+------------+------------+------------+");
    cpssOsFprintf(file->fd, "\n| Q  |tail counter|frag counter|head counter|frag   index|long  queue |");
    cpssOsFprintf(file->fd, "\n+----+------------+------------+------------+------------+------------+");
    for (i = queueStart; (i <= queueEnd) && (i < CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC); i++)
    {
        rc = prvCpssDxChTxqFalconPdsPerQueueCountersGet(devNum, tileNum, pdsNum, i, &perQueueDescCount);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet  failed for tile  %d pds %d ", tileNum, i);
        }
        cpssOsFprintf(file->fd, "\n|%4d|%12d|%12d|%12d|%12d|%12d|", i, perQueueDescCount.tailCounter,
                  perQueueDescCount.fragCounter, perQueueDescCount.headCounter, perQueueDescCount.fragIndex, perQueueDescCount.longQueue);
        cpssOsFprintf(file->fd, "\n+----+------------+------------+------------+------------+------------+");
    }
    cpssOsFprintf(file->fd, "\n");
    return GT_OK;
}
GT_STATUS prvCpssDxChTxqFalconDumpSchedErrors
(
    IN GT_U8 devNum
)
{
    GT_U32      numberOfTiles, i;
    GT_STATUS   rc;
    GT_U32      errorCounter;
    GT_U32      exceptionCounter;
    GT_U32      errorStatus;
    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        numberOfTiles = 1;
        break;
    case 2:
    case 4:
        numberOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d  ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    cpssOsPrintf("\n+------+----------+--------------+-------------+");
    cpssOsPrintf("\n| Tile | Error cnt|Exception cnt |Error status |");
    cpssOsPrintf("\n+------+----------+--------------+-------------+");
    for (i = 0; i < numberOfTiles; i++)
    {
        rc = prvCpssFalconTxqPdqGetErrorStatus(devNum, i, &errorCounter, &exceptionCounter, &errorStatus);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetErrorStatus  failed for tile  %d  ", i);
        }
        cpssOsPrintf("\n|%6d|%10d|%14d|%13d|", i, errorCounter, exceptionCounter, errorStatus);
        cpssOsPrintf("\n+------+----------+--------------+-------------+");
    }
    cpssOsPrintf("\n");
    cpssOsPrintf("\n\nError meaning :\n\n");
    cpssOsPrintf("FIFOOvrflowErr (Bit 5)- Tm to TM backpressure FIFO has over flown.Backpressure Xon/Xoff indication lost.\n");
    cpssOsPrintf("TBNegSat (Bit 4) - A Token Bucket level has reached its maximal negative value and is saturated at this value.\n");
    cpssOsPrintf("BPBSat (Bit 3)- Byte Per Burst value (internal parameter used for Port DWRR) is saturated.\n");
    cpssOsPrintf("UncECCErr(Bit 2) - Two or more bits were corrupted in internal memory protected by ECC and could not be corrected\n");
    cpssOsPrintf("CorrECCErr(Bit 1) - One corrupted bit in internal memory was detected and corrected by ECC.\n");
    cpssOsPrintf("ForcedErr(Bit 0) - A forced error was ordered by writing to the Force Error configuration register.\n");
    return GT_OK;
}
GT_STATUS prvCpssDxChTxqDumpQueueStatus
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 queueStart,
    IN GT_U32 queueEnd
)
{
    GT_U32  i;
    GT_U32  rc;
    GT_32   level;
    GT_U32  dificit;
    cpssOsPrintf("Level - Bucket level field is a signed value in bytes. Min Token Bucket is Conforming if Value greater than 0.\n");
    cpssOsPrintf("MinLvl[bit 22] - Read Error. Set in case read is failed.  \n\n\n");
    cpssOsPrintf("\n+----+-------+-------+");
    cpssOsPrintf("\n| Q  | level |dificit|");
    cpssOsPrintf("\n+----+-------+-------+");
    for (i = queueStart; (i <= queueEnd) && (i < CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM); i++)
    {
        rc = prvCpssFalconTxqPdqGetQueueStatus(devNum, tileNum, i, &level, &dificit);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueStatus  failed for tile  %d  ", i);
        }
        cpssOsPrintf("\n|%4d|%7d|%7d|", i, level, dificit);
        cpssOsPrintf("\n+----+-------+-------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}
GT_STATUS prvCpssDxChTxqDumpAlevelStatus
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 alevelStart,
    IN GT_U32 alevelEnd
)
{
    GT_U32  i;
    GT_U32  rc;
    GT_32   level;
    GT_U32  dificit;
    cpssOsPrintf("Level - Bucket level field is a signed value in bytes. Min Token Bucket is Conforming if Value greater than 0.\n");
    cpssOsPrintf("MinLvl[bit 22] - Read Error. Set in case read is failed.  \n\n\n");
    cpssOsPrintf("\n+----+-------+-------+");
    cpssOsPrintf("\n| A  | level |dificit|");
    cpssOsPrintf("\n+----+-------+-------+");
    for (i = alevelStart; (i <= alevelEnd) && (i < CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC); i++)
    {
        rc = prvCpssFalconTxqPdqGetAlevelStatus(devNum, tileNum, i, &level, &dificit);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueStatus  failed for tile  %d  ", i);
        }
        cpssOsPrintf("\n|%4d|%7d|%7d|", i, level, dificit);
        cpssOsPrintf("\n+----+-------+-------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}
GT_STATUS prvCpssDxChTxqDumpAnodeShapingParameters
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 aNodeStart,
    IN GT_U32 aNodeEnd
)
{
    GT_U32  i;
    GT_U32  rc;
    GT_U32  divExp;
    GT_U32  token;
    GT_U32  res;
    GT_U32  burst;
    GT_BOOL shapingEnabled;
    GT_U32  shaperDec;
    GT_U32  perInter;
    GT_U32  perEn, bw = 0, shaperFreq;
    double  tmp;
    rc = prvCpssFalconTxqPdqShaperInputFreqGet(devNum, tileNum, &shaperFreq);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqPdqPerLevelShapingParametersGet  failed  ");
    }
    /*move to KHz*/
    shaperFreq  /= 1000;
    rc      = prvCpssSip6TxqPdqPerLevelShapingParametersGet(devNum, tileNum, PRV_CPSS_PDQ_LEVEL_A_E, &shaperDec, &perInter, &perEn);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqPdqPerLevelShapingParametersGet  failed  ");
    }
    cpssOsPrintf("\nA level parameters:\n\nPeriodic interval %d\n\nshaperDec %d\n\nperEn %d\n", perInter, shaperDec, perEn);
    cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
    cpssOsPrintf("\n| A  |shp| divExp|token  |res  | burst |BW in Kbit |");
    cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
    for (i = aNodeStart; (i <= aNodeEnd) && (i < CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC); i++)
    {
        rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileNum, i, PRV_CPSS_PDQ_LEVEL_A_E, &shapingEnabled);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqShapingEnableGet  failed for q  %d  ", i);
        }
        rc = prvCpssSip6TxqPdqNodeShapingParametersGet(devNum, tileNum, i, GT_TRUE,&divExp, &token, &res, &burst);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqPdqGetQueueShapingParameters  failed for q  %d  ", i);
        }
        tmp = (double)( token * (1 << res)) / (perInter * (1 << divExp));
        bw  = (GT_U32)(((shaperFreq / 4) * 8) * tmp); /*shaper updated every 4 cycles*/
        cpssOsPrintf("\n|%4d|%3c|%7d|%7d|%5d|%7d|%11d|", i, shapingEnabled ? 'y' : 'n', divExp, token, res, burst, bw);
        cpssOsPrintf("\n+----+---+-------+-------+-----+-------+-----------+");
    }
    cpssOsPrintf("\n");
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump function
 * @endinternal
 *
 * @brief   Debug function that dump physical port to P node and A node mapping.
 *         Also PDQ queue numbers that are mapped to A node are dumped
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] firstPort                   -first port to dump
 * @param[in] lastPort                   -last port to dump
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump
(
    IN GT_U8 devNum,
    IN GT_U32 firstPort,
    IN GT_U32 lastPort
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *        aNode;
    GT_STATUS                   rc;
    GT_U32                      i, j, k;
    GT_U32                  currentPortTile = 0;
    GT_U32                  remote      = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *    pNodePtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    GT_U32                  aNodeIndex = 0, pNodeIndex = 0, tmp, qNodeFirst = 0, qNodeLast = 0, globalDmaNum, cascadePort = 0;
    GT_BOOL                 isCascadePort;
    GT_U32                  aNodeGlobalQOffset;
    cpssOsPrintf("\nPhysical port to scheduler nodes table - SW shadow");
    cpssOsPrintf("\n+---+------+-------+------------+----------+------+----------------+------------+");
    cpssOsPrintf("\n| # |P node| A node| PDQ Q first|PDQ Q Last|Remote|Cascade port num|Global Q ind|");
    cpssOsPrintf("\n+---+------+-------+------------+----------+------+----------------+------------+");
    for (i = firstPort; i <= lastPort; i++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i, /*OUT*/ &portMapShadowPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (portMapShadowPtr->valid)
        {
            aNodeGlobalQOffset  = 0;
            rc          = prvCpssFalconTxqUtilsIsCascadePort(devNum, i, &isCascadePort, &pNodePtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (GT_FALSE == isCascadePort)
            {
                rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, i, &currentPortTile, &aNodeIndex);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /*found corresponding P node*/
                if (prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, i, &aNode) == GT_OK)
                {
                    aNodeIndex      = aNode->aNodeIndex;
                    rc  = prvCpssSip6TxqAnodeToPnodeIndexGet(devNum,currentPortTile,aNodeIndex,&pNodeIndex);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqAnodeToPnodeIndexGet failed for index %d",aNodeIndex);
                    }

                    qNodeFirst      = aNode->queuesData.pdqQueueFirst;
                    qNodeLast       =  aNode->queuesData.pdqQueueLast;
                    aNodeGlobalQOffset  = aNode->qGlobalOffset;
                    if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
                    {
                        remote  = 1;
                        rc  = prvCpssFalconTxqUtilsGetCascadePort(devNum, i, GT_TRUE, &cascadePort, NULL);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
            else
            {
                currentPortTile = pNodePtr->aNodelist[0].queuesData.tileNum;

                rc      = prvCpssFalconTxqUtilsGetCascadePort(devNum, i, GT_FALSE, &cascadePort, &pNodeIndex);
                if (rc != GT_OK)
                {
                    return rc;
                }
                qNodeFirst  = pNodePtr->aNodelist[0].queuesData.pdqQueueFirst;
                aNodeIndex  = pNodePtr->aNodeListSize - 1;
                qNodeLast   = pNodePtr->aNodelist[aNodeIndex].queuesData.pdqQueueLast;
                currentPortTile = pNodePtr->aNodelist[aNodeIndex].queuesData.tileNum;
            }
            cpssOsPrintf("\n|%3d|%7d|%6d|%12d|%10d|%6d|%16d|%12d|", i, pNodeIndex, isCascadePort ? 0 : aNodeIndex, qNodeFirst, qNodeLast, remote, cascadePort, aNodeGlobalQOffset);
            cpssOsPrintf("\n+---+------+-------+------------+----------+------+----------------+------------+");
        }
        /*dump dma mapping only if single port requested*/
        if (firstPort != lastPort)
        {
            continue;
        }
        cpssOsPrintf("\nDMA to Pnode mapping - SW shadow");
        cpssOsPrintf("\n+---+------+-------+------------+");
        cpssOsPrintf("\n| DP |Port | P node| Global DMA |");
        cpssOsPrintf("\n+---+------+-------+------------+");
        for (k = 0; k < 8; k++)
        {
            for (j = 0; j < 9; j++)
            {
                rc = prvCpssFalconTxqUtilsFindByLocalPortAndDpInTarget(devNum, currentPortTile, k, j, &tmp, GT_FALSE);
                if (rc == GT_OK)
                {
                    rc = prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed(devNum, currentPortTile,
                                                              k, j, &globalDmaNum);
                    if (tmp == pNodeIndex)
                    {
                        cpssOsPrintf("\n|%3d|%6d|%7d|%12d|", k, j, pNodeIndex, globalDmaNum);
                        cpssOsPrintf("\n+---+------+-------+------------+");
                    }
                }
            }
        }
    }

    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsSdqQueueAttributesDump function
 * @endinternal
 *
 * @brief   Debug function that dump queue attributes
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                    PP's device number.
 * @param[in]  tileNum                    tile number.(APPLICABLE RANGES:0..3)
 * @param[in] sdqNum                    global SDQ number (APPLICABLE RANGES:0..31)
 * @param[in] startQ                     -queue to start from in the dump
 * @param[in] size                     -     number of queues to dump
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsSdqQueueAttributesDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 sdqNum,
    IN GT_U32 startQ,
    IN GT_U32 size,
    IN CPSS_OS_FILE_TYPE_STC * file
)
{
    GT_U32                      i;
    GT_STATUS                   rc;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES queueAttributes;
    GT_U32                      balance;
    GT_U32                      eligState;
    GT_BOOL                     sp;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);
    cpssOsFprintf(file->fd, "\nQueueAttributes tile %d SDQ %d\n", tileNumber, sdqNum);
    cpssOsFprintf(file->fd, "\n+-------------+------+--+--------+--------+--------+--------+--------+--+");
    cpssOsFprintf(file->fd, "\n| queueNumber |enable|tc|  high  |  low   |  neg   |balance | eligSt |SP|");
    cpssOsFprintf(file->fd, "\n+-------------+------+--+--------+--------+--------+--------+--------+--+");
    for (i = startQ; i < (startQ + size); i++)
    {
        rc = prvCpssFalconTxqSdqQueueAttributesGet(devNum, tileNumber, sdqNum, i, &queueAttributes);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n", rc );
            return rc;
        }
        rc = prvCpssFalconTxqSdqQCreditBalanceGet(devNum, tileNumber, sdqNum, i, &balance);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n", rc );
            return rc;
        }
        rc = prvCpssFalconTxqSdqEligStateGet(devNum, tileNumber, sdqNum, i, &eligState);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n", rc );
            return rc;
        }
        rc = prvCpssFalconTxqSdqQueueStrictPriorityGet(devNum, tileNumber, sdqNum, i, &sp);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueStrictPriorityGet\n", rc );
            return rc;
        }
        cpssOsFprintf(file->fd, "\n|%13d|%6d|%2d|%8d|%8d|%8d|%8d|%8d|%2d|", i, queueAttributes.enable, queueAttributes.tc,
                  queueAttributes.highCreditThreshold, queueAttributes.lowCreditTheshold, queueAttributes.negativeCreditThreshold, balance, eligState, sp);
    }
    cpssOsFprintf(file->fd, "\n+-------------+------+--+--------+--------+--------+--------+--------+--+\n");
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsPsiConfigDump function
 * @endinternal
 *
 * @brief   Debug function that dump PDQ queue to SDQ/PDS mapping
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] psiNum                   - tile number (APPLICABLE RANGES:0..3)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPsiConfigDump
(
    IN GT_U8 devNum,
    IN GT_U32 psiNum
)
{
    GT_U32      i;
    GT_U32      pdqLowQueueNum, localPortNum;
    GT_STATUS   rc;
    GT_U32      dpFromHw = 0, localPortFromHw = 0, valid = 0;
    cpssOsPrintf("\nPSI sdq to pdq map table \n");
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        cpssOsPrintf("\n+-----+--------------+-----------------+");
        cpssOsPrintf("\n| SDQ | localPortNum | queue   offset  |");
        cpssOsPrintf("\n+-----+--------------+-----------------+");
    }
    else
    {
        cpssOsPrintf("\n+-----+--------------+-----------------+--------+---------------+-----+");
        cpssOsPrintf("\n| SDQ | localPortNum | queue   offset  |dpFromHw|localPortFromHw|valid|");
        cpssOsPrintf("\n+-----+--------------+-----------------+--------+---------------+-----+");
    }
    for (i = 0; i < MAX_DP_IN_TILE(devNum); i++)
    {
        for (localPortNum = 0; localPortNum < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); localPortNum++)
        {
            rc = prvCpssFalconPsiMapPortToPdqQueuesGet(devNum, psiNum, i, localPortNum, &pdqLowQueueNum,
                                   &dpFromHw, &localPortFromHw, &valid);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            cpssOsPrintf("\n|%5d|%14d|%17d|", i, localPortNum, pdqLowQueueNum);
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
            {
                cpssOsPrintf("%8d|%14d|%5d|", dpFromHw, localPortFromHw, valid);
            }
        }
    }
    cpssOsPrintf("\n+-----+----------------+----------------+");
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
    {
        cpssOsPrintf("--------+---------------+-----+");
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsPdxTablesDump function
 * @endinternal
 *
 * @brief   Debug function that dump PDX routing table and PDX pizza arbiter
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 *                                      psiNum       - tile number (APPLICABLE RANGES:0..3)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsPdxTablesDump
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 lastEntryNum,
    IN GT_BOOL dumpPizza
)
{
    GT_U32                          i, j;
    PRV_CPSS_DXCH_FALCON_TXQ_PDX_PHYSICAL_PORT_MAP_STC  txPortMap;
    GT_STATUS                       rc;
    GT_U32                          activeSliceNum;
    GT_U32                          activeSliceMap[40];
    GT_BOOL                         sliceValid[40];
    TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(devNum, lastEntryNum);
    cpssOsPrintf("\nPDX routing table \n");
    cpssOsPrintf("\n+-----------------+-----+------+----------+-------------------+");
    cpssOsPrintf("\n| queueGroupIndex | PDX | PDS  | queueBase| dpCoreLocalTrgPort|");
    cpssOsPrintf("\n+-----------------+-----+------+----------+-------------------+");
    for (i = 0; i <= lastEntryNum; i++)
    {
        rc = prvCpssDxChTxqFalconPdxQueueGroupMapGet(devNum, i, &txPortMap);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        cpssOsPrintf("\n|%17d|%5d|%6d|%10d|%19d|", i,
                 txPortMap.queuePdxIndex, txPortMap.queuePdsIndex, txPortMap.queueBase, txPortMap.dpCoreLocalTrgPort);
    }
    cpssOsPrintf("\n+-----------------+-----+------+----------+-------------------+\n\n\n");
    cpssOsTimerWkAfter(50);
    if (GT_FALSE == dumpPizza)
    {
        return GT_OK;
    }
    cpssOsPrintf("\nPDX Pizza arbiter\n");
    cpssOsPrintf("\n+----+-----+-------+-----+----------+");
    cpssOsPrintf("\n|PDX | PDS | slice |valid|assignment|");
    cpssOsPrintf("\n+----+-----+-------+-----+----------+");
    for (i = 0; i < 8; i++)
    {
        rc = prvCpssDxChTxqFalconPdxBurstFifoPdsArbitrationMapGet(devNum, pdxNum, i, &activeSliceNum, activeSliceMap, sliceValid);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        for (j = 0; j < activeSliceNum; j++)
        {
            cpssOsPrintf("\n|%4d|%5d|%7d|%5s|%10c|", pdxNum, i, j, (sliceValid[j] == GT_TRUE) ? "true" : "false", (sliceValid[j] == GT_TRUE) ? '0' + activeSliceMap[j] : ' ');
        }
        cpssOsPrintf("\n+----+-----+-------+-----+----------+");
    }
    return GT_OK;
}
/**
 * @internal prvCpssFalconTxqUtilsSdqPortRangeDump function
 * @endinternal
 *
 * @brief   Debug function that dump mapping of local port to queues at SDQ
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                     PP's device number.
 * @param[in] tileNumber                tile number (APPLICABLE RANGES:0..3)
 * @param[in] sdqNum                   -global SDQ number (APPLICABLE RANGES:0..31)
 * @param[in] portSpecific               if equal GT_TRUE then print range for specific local port,else print for all ports.
 * @param[in] port                   -       local port number
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsSdqPortRangeDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 sdqNum,
    IN GT_BOOL portSpecific,
    IN GT_U32 port
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE range;
    GT_U32                      i,data;
    GT_STATUS                   rc;
    GT_BOOL                     enable;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  portAttributes;
    cpssOsPrintf("\nPortRange and BP thresholds in SDQ %d\n", sdqNum);
    cpssOsPrintf("\n+-----------+-------+---+----+-----------+------------+");
    cpssOsPrintf("\n| localPort |enable |low|high|BP high th | BP low th  |");
    cpssOsPrintf("\n+-----------+-------+---+----+-----------+------------+");
    for (i = portSpecific ? port : 0; i < CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum); i++)
    {
        PRV_CPSS_TXQ_UTILS_SKIP_RESERVED_MAC(devNum,i);

        rc = prvCpssFalconTxqSdqLocalPortEnableGet(devNum, tileNumber, sdqNum, i, &enable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssFalconTxqSdqLocalPortQueueRangeGet(devNum, tileNumber, sdqNum, i, &range);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssFalconTxqSdqPortAttributesGet(devNum, tileNumber, sdqNum, i, &portAttributes);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqPortAttributesGet  failed for port %d  ", i);
        }
        cpssOsPrintf("\n|%11d|%7d|%3d|%4d|%11d|%12d|", i, enable, range.lowQueueNumber, range.hiQueueNumber,
                 portAttributes.portBackPressureHighThreshold, portAttributes.portBackPressureLowThreshold);
        if (GT_TRUE == portSpecific)
        {
            break;
        }
    }
    cpssOsPrintf("\n+-----------+-------+---+----+-----------+------------+");
    cpssOsPrintf("\n");

    if(GT_TRUE==portSpecific)
    {
        rc = prvCpssSip6TxqSdqLocalPortConfigGet(devNum,tileNumber,sdqNum,port,&data);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqSdqLocalPortConfigGet  failed for port %d  ", port);
        }

        cpssOsPrintf("RAW 0x%08x \n",data);
    }
    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Dump queue to profile PDS binding
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] tileNumber           -tile number to dump
 * @param[in] pdsNum                      - pds number to dump
 * @param[in] size                      - amount of queues to dump
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note
 */
GT_STATUS prvCpssFalconTxqUtilsPdqQueueProfilesBindDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 pdsNum,
    IN GT_U32 size
)
{
    GT_U32      i, profile[2] = { 0 };
    GT_STATUS   rc;
    cpssOsPrintf("\nQueueAttributes PDS %d\n", pdsNum);
    cpssOsPrintf("\n+-------------+--------+------------------------------------+");
    cpssOsPrintf("\n| queueNumber |profile |length adjust profile(SIP 6_10 only)|");
    cpssOsPrintf("\n+-------------+--------+------------------------------------+");
    for (i = 0; i < size; i++)
    {
        rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum, tileNumber, pdsNum, i, profile);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsQueueProfileMapGet\n", rc );
            return rc;
        }
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
        {
            rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet(devNum, tileNumber, pdsNum, i, profile + 1);
            if (rc != GT_OK)
            {
                cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsQueueProfileMapGet\n", rc );
                return rc;
            }
            cpssOsPrintf("\n|%13d|%8d|%36d|", i, profile[0], profile[1]);
        }
        else
        {
            cpssOsPrintf("\n|%13d|%8d|NA                   (SIP 6_10 only)|", i, profile[0]);
        }
    }
    cpssOsPrintf("\n+-------------+--------+------------------------------------+\n");
    return GT_OK;
}
/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Dump  PDS queue profile attributes
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] tileNumber           -tile number to dump
 * @param[in] pdsNum                      - pds number to dump
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 * @note
 */
GT_STATUS prvCpssFalconTxqUtilsPdqQueueProfilesAttributesDump
(
    IN GT_U8 devNum,
    IN GT_U32 tileNumber,
    IN GT_U32 pdsNum
)
{
    GT_U32                          i;
    GT_STATUS                       rc;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC        profile;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC     longQ;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC  lengthAdjust;
    profile.longQueueParametersPtr      = &longQ;
    profile.lengthAdjustParametersPtr   = &lengthAdjust;
    cpssOsPrintf("\n PDS queue profile tile %d  pds %d\n", tileNumber, pdsNum);
    cpssOsPrintf("\n+----------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n| profile  | longQueueEnable |   headEmptyLimit| longQueueLimit  |lengthAdjEnable  |lengthAdjSubstrct|lengthAdjByteCnt |");
    cpssOsPrintf("\n+----------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+");
    for (i = 0; i < PDS_PROFILE_MAX_MAC; i++)
    {
        rc = prvCpssDxChTxqFalconPdsProfileGet(devNum, tileNumber, pdsNum, i, GT_FALSE, &profile);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsProfileGet\n", rc );
            return rc;
        }
        cpssOsPrintf("\n|%10d|%17d|%17d|%17d|%17d|%17d|%17d|", i,
                 profile.longQueueParametersPtr->longQueueEnable,
                 profile.longQueueParametersPtr->headEmptyLimit,
                 profile.longQueueParametersPtr->longQueueLimit,
                 profile.lengthAdjustParametersPtr->lengthAdjustEnable,
                 profile.lengthAdjustParametersPtr->lengthAdjustSubstruct,
                 profile.lengthAdjustParametersPtr->lengthAdjustByteCount);
    }
    cpssOsPrintf("\n+----------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+");
    cpssOsPrintf("\n");
    return GT_OK;
}

/**
 * @internal prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping function
 * @endinternal
 *
 * @brief The function dump         local_port_%p_source_port_config table
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 * @param[in] tileNum                    -tile number
 * @param[in] qfcNum                   - qfc number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 *
 */
GT_STATUS prvCpssDxChTxqDumpQfcLocalPortToSourcePortMapping
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 qfcNum,
    IN GT_U32 localPortStart,
    IN GT_U32 localPortEnd,
    OUT GT_U32 *sourcePtr
)
{
    GT_U32              i;
    GT_STATUS           rc;
    GT_U32              sourcePort = 0;
    CPSS_DXCH_PORT_FC_MODE_ENT  portType;
    cpssOsPrintf("Tile %d  qfcNum %d\n", tileNum, qfcNum);
    cpssOsPrintf("\n+----+-------+--------+");
    cpssOsPrintf("\n|Locl|Source |portType|");
    cpssOsPrintf("\n+----+-------+--------+");
    for (i = localPortStart; i <= localPortEnd; i++)
    {
        rc = prvCpssFalconTxqQfcLocalPortToSourcePortGet(devNum, tileNum, qfcNum, i, &sourcePort, &portType);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqQfcLocalPortToSourcePortGet  failed for tile  %d  ", i);
        }
        cpssOsPrintf("\n|%4d|%7d|%7d|", i, sourcePort, portType);
        cpssOsPrintf("\n+----+-------+--------+");
    }
    cpssOsPrintf("\n");
    if (sourcePtr != NULL)
    {
        *sourcePtr = sourcePort;
    }
    return GT_OK;
}

GT_STATUS prvCpssTxqSip6DebugHwInfoGet
(
    IN  GT_U8                   devNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_HWINFO_STC *platformPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum,platformPtr->numberOfTiles)
    platformPtr->numberOfDp= MAX_DP_IN_TILE(devNum);
    platformPtr->numberOfLocalPorts= CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum);
    platformPtr->numberOfDpQueues = CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC;
    platformPtr->pbSize = platformPtr->numberOfTiles*prvCpssTxqUtilsPbSizeInBuffersGet(devNum);
    platformPtr->pdqNumPorts= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts;

    rc = prvCpssFalconTxqPdqShaperInputFreqGet(devNum, 0, &(platformPtr->freq));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqPdqPerLevelShapingParametersGet  failed  ");
    }

    return GT_OK;
}

GT_STATUS prvCpssTxqSip6DebugTileNumGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                   *tileNumPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_HWINFO_STC platform;

    rc =prvCpssTxqSip6DebugHwInfoGet(devNum,&platform);
    if (rc != GT_OK)
    {
        cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsProfileGet\n", rc );
        return rc;
    }

    *tileNumPtr = platform.numberOfTiles;

    return GT_OK;
}




GT_STATUS prvCpssTxqSip6DumpChunk
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum
)
{
    cpssOsPrintf("\nTile %d chunks\n",tileNum);
    cpssOsPrintf("==============================\n");
    return prvSchedRmDebugDumpAllChunk(PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.schedHandle[tileNum]);
}


GT_STATUS prvCpssTxqSip6DebugHwInfoDump
(
    IN  GT_U8                   devNum,
    IN  GT_BOOL                 dumpTreeUsage
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_HWINFO_STC platform;
    GT_U32    i;


    rc =prvCpssTxqSip6DebugHwInfoGet(devNum,&platform);
    if (rc != GT_OK)
    {
        cpssOsPrintf("%d = prvCpssDxChTxqFalconPdsProfileGet\n", rc );
        return rc;
    }
    cpssOsPrintf("Number of tiles         : %5d\n",platform.numberOfTiles);
    cpssOsPrintf("Number of DPs           : %5d\n",platform.numberOfDp);
    cpssOsPrintf("Number of local ports   : %5d\n",platform.numberOfLocalPorts);
    cpssOsPrintf("Number of queues per DP : %5d\n",platform.numberOfDpQueues);
    cpssOsPrintf("PB size (all tiles) :   : %5d\n",platform.pbSize);
    cpssOsPrintf("SDQ2PDQ ports           : %5d\n",platform.pdqNumPorts);
    cpssOsPrintf("Shaper frequency(Mhz)   : %5d\n",platform.freq/1000000);

    if(GT_TRUE == dumpTreeUsage)
    {
        rc = prvCpssFalconTxqDebugDumpTreeUsage(devNum);
        if (rc != GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqDebugDumpTreeUsage\n", rc );
            return rc;
        }

        for(i=0;i<platform.numberOfTiles;i++)
        {
            rc = prvCpssTxqSip6DumpChunk(devNum,i);
            if (rc != GT_OK)
            {
                cpssOsPrintf("%d = prvCpssTxqSip6DumpChunk\n", rc );
                return rc;
            }
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssTxqSip6DebugPdsProfileGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U32                      *pdsProfileIndexPtr,
    OUT GT_U32                      *speedInGPtr,
    OUT GT_BOOL                     *longQueueEnablePtr,
    OUT GT_U32                      *headEmptyLimitPtr,
    OUT GT_U32                      *longQueueLimitPtr,
    OUT GT_BOOL                      *txCreditValidPtr
)
{
    GT_STATUS rc;
    GT_U32    firstQueue =0 ;
    GT_U32    tileNum =0 ;
    GT_U32    dpNum =0 ;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC lengthAdjustParameters;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC        longQueueParameters;
    GT_BOOL isCascade = GT_FALSE;

    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;
    CPSS_NULL_PTR_CHECK_MAC(pdsProfileIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(speedInGPtr);
    CPSS_NULL_PTR_CHECK_MAC(longQueueEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(headEmptyLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(longQueueLimitPtr);
    CPSS_NULL_PTR_CHECK_MAC(txCreditValidPtr);

    profile.lengthAdjustParametersPtr = &lengthAdjustParameters;
    profile.longQueueParametersPtr = &longQueueParameters;

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,&isCascade,&pNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &aNodePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
        }
    }
    else
    {
        aNodePtr = &(pNodePtr->aNodelist[0]);
    }

    firstQueue = aNodePtr->queuesData.queueBase;
    tileNum = aNodePtr->queuesData.tileNum;
    dpNum = aNodePtr->queuesData.dp;

    rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum,tileNum,dpNum,firstQueue,pdsProfileIndexPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueProfileMapGet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssDxChTxqFalconPdsProfileGet(devNum,tileNum,dpNum,*pdsProfileIndexPtr,GT_FALSE,&profile);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsProfileGet  failed for portNum  %d  ", portNum);
    }

    *speedInGPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.pdsProfiles[*pdsProfileIndexPtr].speed_in_G;
    *longQueueEnablePtr = profile.longQueueParametersPtr->longQueueEnable;
    *headEmptyLimitPtr = profile.longQueueParametersPtr->headEmptyLimit;
    *longQueueLimitPtr = profile.longQueueParametersPtr->longQueueLimit;

    rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet(devNum, portNum, txCreditValidPtr);

    return rc;
}



GT_STATUS prvCpssTxqSip6DebugLengthAdjustProfileGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U32                      *profileIndexPtr,
    OUT GT_BOOL                     *lengthAdjustEnablePtr,
    OUT GT_U32                      *lengthAdjustSubstructPtr,
    OUT GT_U32                      *lengthAdjustByteCountPtr,
    OUT GT_BOOL                      *txCreditValidPtr
)
{
    GT_STATUS rc;
    GT_U32    firstQueue =0 ;
    GT_U32    tileNum =0 ;
    GT_U32    dpNum =0 ;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC profile;
    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC lengthAdjustParameters;
    GT_BOOL isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr = NULL;
    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(lengthAdjustEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(lengthAdjustSubstructPtr);
    CPSS_NULL_PTR_CHECK_MAC(lengthAdjustByteCountPtr);
    CPSS_NULL_PTR_CHECK_MAC(txCreditValidPtr);

    profile.lengthAdjustParametersPtr = &lengthAdjustParameters;
    profile.longQueueParametersPtr = NULL;

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,&isCascade,&pNodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &aNodePtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
        }
    }
    else
    {
        aNodePtr = &(pNodePtr->aNodelist[0]);
    }

    firstQueue = aNodePtr->queuesData.queueBase;
    tileNum = aNodePtr->queuesData.tileNum;
    dpNum = aNodePtr->queuesData.dp;

    /*For SIP 6.10 and higher length adjust is not coupled with long queue profile*/
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
    {
      rc = prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet(devNum,tileNum,dpNum,firstQueue,profileIndexPtr);
      if(rc !=GT_OK)
      {
         return rc;
      }
    }
    else
    {
        rc = prvCpssDxChTxqFalconPdsQueueProfileMapGet(devNum,tileNum,dpNum,firstQueue,profileIndexPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsQueueProfileMapGet  failed for portNum  %d  ", portNum);
        }
     }

    rc = prvCpssDxChTxqFalconPdsProfileGet(devNum,tileNum,dpNum,*profileIndexPtr,GT_TRUE,&profile);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqFalconPdsProfileGet  failed for portNum  %d  ", portNum);
    }

    *lengthAdjustEnablePtr = profile.lengthAdjustParametersPtr->lengthAdjustEnable;
    *lengthAdjustSubstructPtr = profile.lengthAdjustParametersPtr->lengthAdjustSubstruct?1:0;
    *lengthAdjustByteCountPtr = profile.lengthAdjustParametersPtr->lengthAdjustByteCount;

    rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapGet(devNum, portNum, txCreditValidPtr);

    return rc;
}


GT_STATUS prvCpssTxqSip6DebugMacStatusGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U32                      *adressPtr,
    OUT GT_U32                      *rawRegisterValuePtr,
    OUT GT_U32                      *rxFifoEmptyPtr,
    OUT GT_U32                      *txFifoEmptyPtr
)
{
    PRV_CPSS_PORT_TYPE_ENT macType;
    GT_U32                 portMacNum;/*mac number*/
    GT_STATUS              rc;
    GT_U32                 regAddr,rawData;


    CPSS_NULL_PTR_CHECK_MAC(adressPtr);
    CPSS_NULL_PTR_CHECK_MAC(rawRegisterValuePtr);
    CPSS_NULL_PTR_CHECK_MAC(rxFifoEmptyPtr);
    CPSS_NULL_PTR_CHECK_MAC(txFifoEmptyPtr);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);

    if (macType == PRV_CPSS_PORT_MTI_100_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI100_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_400_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI400_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_USX_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_USX_MAC.status;
    }
    else if(macType == PRV_CPSS_PORT_MTI_CPU_E)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.MTI[portMacNum].MTI_CPU_MAC.status;
    }
    else
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " unsupported mac type %d for portNum  %d \n ",macType, portNum);
    }

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,
                PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                            portMacNum),
                    regAddr,&rawData);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssTxqSip6DebugMacStatusGet  failed for portNum  %d  ", portNum);
    }

    *adressPtr = regAddr;
    *rawRegisterValuePtr = rawData;

     /*PMAC */

    *rxFifoEmptyPtr = ((rawData>>6)&0x1);
    *txFifoEmptyPtr = ((rawData>>5)&0x1);

    if (TXQ_IS_PREEMPTIVE_DEVICE(devNum)&& (macType == PRV_CPSS_PORT_MTI_100_E))
    {
        /*EMAC - bit 21 is EMAC_TX_EMPTY ,bit 22 is EMAC_RX_EMPTY */
        *rxFifoEmptyPtr|= (((rawData>>22)&0x1)<<1);
        *txFifoEmptyPtr|=  (((rawData>>21)&0x1)<<1);
    }


    return rc;
}


GT_VOID prvCpssTxqSip6DebugLog
(
    IN  GT_U8                       devNum,
    IN  const char *                file,
    IN  const char *                 function,
    IN  GT_U32                      line,
    IN  const char *                format,
    IN  GT_U32                      param0,
    IN  GT_U32                      param1,
    IN  GT_U32                      param2,
    IN  GT_U32                      param3
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_CHAR_PTR bufferPtr = NULL ;
    GT_U32      length;
    GT_U32      pid,tid;
    GT_STATUS   rc;

    bufferPtr = PRV_NON_SHARED_TXQ_DIR_TXQ_DBG_SRC_GLOBAL_VAR_GET(logBuffer);

    length = cpssOsStrlen(format);

#ifdef CPSS_LOG_ENABLE

    if (PRV_NON_SHARED_GLOBAL_VAR_LOG_GET() == GT_TRUE)
    {
        prvCpssLogInformation(function, file, line, format,param0,param1,param2,param3);
    }
    else
#endif
        file = file;
    {
        tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0]);

        if(bufferPtr&&GT_TRUE==tileConfigsPtr->general.logEnable)
        {
            if(length>PRV_TXQ_LOG_STRING_ALLOWED_SIZE_CNS)
            {
                cpssOsSnprintf(bufferPtr,PRV_TXQ_LOG_STRING_ALLOWED_SIZE_CNS,format,param0,param1,param2,param3);
                bufferPtr[PRV_TXQ_LOG_STRING_ALLOWED_SIZE_CNS]='\0';
            }
            else
            {
                cpssOsSprintf(bufferPtr,format,param0,param1,param2,param3);
            }

            /*add PID.May be useful for shared lib*/
            pid = cpssOsTaskGetPid();

            if(GT_NOT_IMPLEMENTED !=pid)
            {
               cpssOsPrintf("PID :%d ",pid);
            }

            rc = cpssOsTaskGetSelf(&tid);

            if(GT_OK == rc )
            {
               cpssOsPrintf("TID index:%d ",tid);
            }
            cpssOsPrintf("Func:%s line %d msg:%s\n",function,line,bufferPtr);
        }
    }
}

GT_STATUS prvCpssTxqSip6DebugLogEnable
(
    IN  GT_U8          devNum,
    IN  GT_U32         enable
)
{
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[0].general.logEnable = enable?GT_TRUE:GT_FALSE;
    return GT_OK;
}

GT_VOID prvCpssTxqSip6DebugLogInit
(
    IN  GT_U8          devNum
)
{
    CPSS_OS_FILE_TYPE_STC file;
    GT_BOOL found = GT_FALSE;

    file.type = CPSS_OS_FILE_REGULAR;
    file.fd = cpssOsFopen(PRV_TXQ_LOG_TRIGGER_FILE_NAME, "r",&file);

    if (file.fd!=CPSS_OS_FILE_INVALID)
    {
        found = GT_TRUE;
        cpssOsPrintf("File:%s found\n",PRV_TXQ_LOG_TRIGGER_FILE_NAME);
        cpssOsFclose(&file);
    }

   prvCpssTxqSip6DebugLogEnable(devNum,found);
}

GT_STATUS  prvCpssTxqSip6DebugSchedulerErrorMappingDump
(
    GT_VOID
)
{
    GT_U32 i;

    for(i=TM_MIN_ERROR_CODE;i<=TM_CONF_MAX_ERROR;i++)
    {
        prvSchedToCpssErrCodeConvertDump(i);
    }

    return GT_OK;
}
GT_STATUS  prvCpssTxqSip6DebugBrStatDump
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    CPSS_PORT_MAC_MTI_BR_STAT_COUNTER_ENT i;
    GT_STATUS rc;
    GT_U32 val;
    GT_CHAR_PTR counterNames[] =
      {
        "CPSS_MTI_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_E",
        "CPSS_MTI_BR_RX_FRAG_COUNT_RX_FRAG_COUNT_E",
        "CPSS_MTI_BR_TX_HOLD_COUNT_TX_HOLD_COUNT_E",
        "CPSS_MTI_BR_RX_SMD_ERR_COUNT_RX_SMD_ERR_COUNT_E",
        "CPSS_MTI_BR_RX_ASSY_ERR_COUNT_RX_ASSY_ERR_COUNT_E",
        "CPSS_MTI_BR_RX_ASSY_OK_COUNT_RX_ASSY_OK_COUNT_E",
        "CPSS_MTI_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_GOOD_E",
        "CPSS_MTI_BR_RX_VERIFY_COUNT_RX_VERIFY_COUNT_BAD_E",
        "CPSS_MTI_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_GOOD_E",
        "CPSS_MTI_BR_RX_RESPONSE_COUNT_RX_RESP_COUNT_BAD_E",
        "CPSS_MTI_BR_TX_VERIF_COUNT_TX_VERIF_COUNT_E",
        "CPSS_MTI_BR_TX_VERIF_COUNT_TX_RESP_COUNT_E"
      };

    for(i=CPSS_MTI_BR_TX_FRAG_COUNT_TX_FRAG_COUNT_E;i<=CPSS_MTI_BR_TX_VERIF_COUNT_TX_RESP_COUNT_E;
        i++)
    {
        rc = cpssDxChPortMacPreemptionStatCounterstGet(devNum,portNum,i,&val);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " cpssDxChPortMacPreemptionStatCounterstGet  failed for portNum  %d  ", portNum);
        }

        cpssOsPrintf("%s = %d\n",counterNames[i],val);

    }

    return GT_OK;
}



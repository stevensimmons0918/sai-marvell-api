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
* @file simLog.h
*
* @brief simulation logger
*
* @version   24
********************************************************************************
*/
#ifndef __simLog_h__
#define __simLog_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <os/simTypes.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SCIB/scib.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahIngress.h>

#define SIM_LOG_IMPORTANT_INFO_STR(infoStr) "\n\n" " ########################################## " #infoStr " \n\n"

/* start processing incoming packet */
#define SIM_LOG_IN_PACKET_STR           SIM_LOG_IMPORTANT_INFO_STR(SIM_LOG_IN_PACKET_STR)
/* end processing incoming packet */
#define SIM_LOG_ENDED_IN_PACKET_STR     SIM_LOG_IMPORTANT_INFO_STR(SIM_LOG_ENDED_IN_PACKET_STR)

/* end processing out going replication */
#define SIM_LOG_OUT_REPLICATION_STR     SIM_LOG_IMPORTANT_INFO_STR(SIM_LOG_OUT_REPLICATION_STR)

/* ingress packet from CPU DMA */
#define SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_STR     "Start processing the ingress packet from DMA of CPU"

/* ingress packet from CPU DMA - ENDED */
#define SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_ENDED_STR     "Ended processing the ingress packet from DMA of CPU"

/* ingress packet from SLAN */
#define SIM_LOG_INGRESS_PACKET_FROM_SLAN_STR        "ingress packet from SLAN["
/* end of packet dump */
#define SIM_LOG_END_OF_PACKET_DUMP_STR              "end of packet dump"
/* DMA operation */
#define SIM_LOG_DMA_OPERATION_STR                   "DMA Operation : ["

/* DMA operation 'write to'*/
#define SIM_LOG_DMA_OPERATION_WRITE_TO_STR          "Write_To"
/* DMA operation 'read from'*/
#define SIM_LOG_DMA_OPERATION_READ_FROM_STR         "Read_From"

/* ready to start info to the log */
#define SIM_LOG_READY_TO_START_LOG_STR  "************ Ready to start LOG *************"

/* port link status changed - prefix */
#define SIM_LOG_PORT_LINK_STATUS_CHANGED_PREFIX_STR "port link status changed : "
/* port link status changed - details */
#define SIM_LOG_PORT_LINK_STATUS_CHANGED_DETAILS_STR "device[ %d ] core[ %d ], port[ %d ] linkState[ %d ]"


/* slan bind info - prefix */
#define SIM_LOG_SLAN_BIND_PREFIX_STR "SLAN bind operation: bind "
#define SIM_LOG_SLAN_BIND_OPERATION_STR "slan [ %s ]"
#define SIM_LOG_SLAN_UNBIND_PREFIX_STR "SLAN bind operation: unbind "
/* slan bind info - details */
#define SIM_LOG_SLAN_BIND_DETAILS_STR "device[ %d ] core[ %d ] port[ %d ] Rx[ %d ] Tx[ %d ]"



/* maximum device filters */
#define SIM_LOG_MAX_DF  32

/* device filters size */
#define SIM_LOG_DF_SIZE 3

/* macro for log function name, file name, line */
#define SIM_LOG_FUNC_NAME_MAC(funcName) #funcName,__FILE__,__LINE__

/* declare local name of function (for known function name) -
   each function that calls the logger (for packet info only)
   must add this line to start of function */
#define DECLARE_FUNC_NAME(funcName) \
  static GT_CHAR* functionNameString = #funcName

/* the macro is 'shorter' version to log messages for type 'SIM_LOG_INFO_TYPE_PACKET_E'
    NOTE:
   1. the variable functionNameString must be known (see use of DECLARE_FUNC_NAME)
   2. the variable devObjPtr must be known .
   3. the call to __LOG must be with double parentheses :
        __LOG((" the string[%d] \n" , the_Params));
*/
#define __LOG(x)                                                       \
    if(simLogIsOpenFlag)                                               \
    {                                                                  \
        /* lock the operations :                                       \
           needed to create atomic operation on next 2 functions       \
           simLogInfoSave() and  simLogInternalLog()                   \
            */                                                         \
        scibAccessLock();                                              \
                                                                       \
        /* save info for the use of simLogInternalLog */               \
        simLogInfoSave(functionNameString,__FILE__,__LINE__,           \
            devObjPtr,SIM_LOG_INFO_TYPE_PACKET_E);                     \
                                                                       \
        simLogInternalLog x;                                           \
                                                                       \
        /* UnLock the operations */                                    \
        scibAccessUnlock();                                            \
    }

/* the same as __LOG macro, but uses tcam info type */
#define __LOG_TCAM(x)                                                  \
    if(simLogIsOpenFlag)                                               \
    {                                                                  \
        scibAccessLock();                                              \
        simLogInfoSave(functionNameString,__FILE__,__LINE__,           \
            devObjPtr,SIM_LOG_INFO_TYPE_TCAM_E);                       \
        simLogInternalLog x;                                           \
        scibAccessUnlock();                                            \
    }

/* __LOG() for parameter with it's value */
#define __LOG_PARAM(param)                                      \
    __LOG(("[%s] = [0x%x] \n",                                  \
        #param ,/* name of the parameter (field name)   */      \
        (param)))/*value of the parameter (field value) */


/* __LOG() for parameter with name different then parameter */
#define __LOG_PARAM_WITH_NAME(paramNameStr,param)               \
    __LOG(("[%s] = [0x%x] (from variable[%s])\n",               \
        paramNameStr ,/* name of the parameter (field name)  */ \
        (param),#param))/*value of the parameter (field value) */



/*  __LOG a message WITHOUT details of 'line' 'file' 'function' , etc
    NOTE: assume we are under scibAccessLock() */
#define __LOG_NO_LOCATION_META_DATA(x)                              \
    if(simLogIsOpenFlag && !simLogIsDevFiltered(devObjPtr))           \
    {                                                               \
        simLogInfoSave(NULL,NULL,0,NULL,SIM_LOG_INFO_TYPE_PACKET_E);\
        simLogInternalLog x ;                                       \
    }

/* __LOG() for parameter with it's value
    __LOG a message WITHOUT details of 'line' 'file' 'function' , etc
    NOTE: assume we are under scibAccessLock() */
#define __LOG_PARAM_NO_LOCATION_META_DATA(param)                \
    __LOG_NO_LOCATION_META_DATA(("[%s] = [0x%x] \n",            \
        #param ,/* name of the parameter (field name)   */      \
        (param)))/*value of the parameter (field value) */

/* like __LOG_NO_LOCATION_META_DATA , but under scib lock/unlock */
#define __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(x)          \
    if(simLogIsOpenFlag && !simLogIsDevFiltered(devObjPtr))       \
    {                                                           \
        scibAccessLock();                                       \
        __LOG_NO_LOCATION_META_DATA(x);                         \
        /* UnLock the operations */                             \
        scibAccessUnlock();                                     \
    }

/* LOG info about pointer : valid or not valid */
#define __LOG_IS_PTR_VALID(paramPtr)                            \
    if(simLogIsOpenFlag&& !simLogIsDevFiltered(devObjPtr))        \
    {                                                           \
        scibAccessLock();                                       \
        simLogInfoSave(NULL,NULL,0,NULL,SIM_LOG_INFO_TYPE_PACKET_E);    \
        simLogInternalLog ("The pointer [%s] is [%s] \n",        \
            #paramPtr ,                                          \
            paramPtr ? "Valid" : "NULL"); \
        /* UnLock the operations */                             \
        scibAccessUnlock();                                     \
    }

#define LOG_FIELD_VALUE(_fieldsNameStr , _fieldValue) \
    __LOG(("<%s> = [0x%x]\n" , _fieldsNameStr , _fieldValue))

#define SET_LOG_FIELD_VALUE(_fieldsNameStr , _fieldValue) \
    __LOG(("set <%s> to [0x%x]\n" , _fieldsNameStr , _fieldValue))

/* pring bmp array */
#define LOG_BMP_ARR(name,bitmapArr,numOfBits)  \
        simLogPrintBitmapArr(devObjPtr,__FILE__,functionNameString, \
            "",name,bitmapArr,numOfBits)

#define __LOG_POINTER(_pointer, num_bytes)                           \
    /* add LOG info */                                              \
    if(simLogIsOpenFlag)                                            \
    {                                                               \
        __LOG(("[%s] with num of bytes[%d] \n",                     \
            #_pointer , num_bytes));                                 \
        {                                                           \
            GT_U32  ii;                                             \
            __LOG(("bytes:"));                                      \
            for(ii = 0 ; ii < num_bytes && ii < 16 ;ii++)           \
            {                                                       \
                __LOG(("%2.2x ",(_pointer)[ii]));                  \
            }                                                       \
            if(num_bytes > 16)                                      \
            {                                                       \
                __LOG(("(only first 16 bytes) \n"));                \
            }                                                       \
            else                                                    \
            {                                                       \
                __LOG(("\n"));                                      \
            }                                                       \
        }                                                           \
    }                                                               \



typedef enum{
    SIM_LOG_FRAME_COMMAND_TYPE___MUST_BE_FIRST___E,

    SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E,
    SIM_LOG_FRAME_COMMAND_TYPE_FROM_CPU_E,

    SIM_LOG_FRAME_COMMAND_TYPE___LAST___E
}SIM_LOG_FRAME_COMMAND_TYPE_ENT;

typedef enum{
    SIM_LOG_FRAME_UNIT___MUST_BE_FIRST___E,

    SIM_LOG_FRAME_UNIT_PHY_INGRESS_E ,
    SIM_LOG_FRAME_UNIT_PHY_EGRESS_E,

    /* ingress Pipe */
    SIM_LOG_FRAME_UNIT_PORT_MAC_INGRESS_E ,
    SIM_LOG_FRAME_UNIT_RXDMA_E,
    SIM_LOG_FRAME_UNIT_TTI_E,
    SIM_LOG_FRAME_UNIT_PPU_E,
    SIM_LOG_FRAME_UNIT_IPCL_E,
    SIM_LOG_FRAME_UNIT_L2I_E,
    SIM_LOG_FRAME_UNIT_IPVX_E,
    SIM_LOG_FRAME_UNIT_IOAM_E,
    SIM_LOG_FRAME_UNIT_SMU_E,
    SIM_LOG_FRAME_UNIT_IPLR_E,
    SIM_LOG_FRAME_UNIT_MLL_E,
    SIM_LOG_FRAME_UNIT_EQ_E,

    SIM_LOG_FRAME_UNIT_PIPE_PCP_E, /* PIPE device : PCP unit */


    SIM_LOG_FRAME_UNIT_INGRESS_PIPE_E,/* the whole ingress pipe */

    /* egress Pipe */
    SIM_LOG_FRAME_UNIT_EGF_EFT_SHT_QAG_E,/* egress filtering */
    SIM_LOG_FRAME_UNIT_TXQ_E, /*dq */
    SIM_LOG_FRAME_UNIT_HA_E,
    SIM_LOG_FRAME_UNIT_EPCL_E,
    SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_QUEUE_MAPPER_E,/*TM Q-Mapper Unit (bobcat2)*/
    SIM_LOG_FRAME_UNIT_EPLR_E,
    SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_DROP_AND_STATISTICS_E,/*TM Drop and Statistics Unit(bobcat2)*/
    SIM_LOG_FRAME_UNIT_ERMRK_E,/*final egress remark*/
    SIM_LOG_FRAME_UNIT_EREP_E,
    SIM_LOG_FRAME_UNIT_PREQ_E,
    SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_ENGINE_E,/* TM engine (bobcat2)*/

    SIM_LOG_FRAME_UNIT_TXDMA_E,
    SIM_LOG_FRAME_UNIT_TXFIFO_E,
    SIM_LOG_FRAME_UNIT_PORT_MAC_EGRESS_E,

    SIM_LOG_FRAME_UNIT_PHA_E,         /* PHA unit : PIPE/Falcon.. */

    SIM_LOG_FRAME_UNIT_EGRESS_PIPE_E,/* the whole egress pipe */

    /* general units */
    SIM_LOG_FRAME_UNIT_CNC_E,
    SIM_LOG_FRAME_UNIT_TCAM_E,

    SIM_LOG_FRAME_UNIT_GENERAL_E,/* ALL the general units */

    SIM_LOG_FRAME_UNIT_ALL_E,/* ALL the units */


    /************************* special values *****************/

    SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E,/* used for code to state that currently
                                we not want 'unit filter' to filter the printings to
                                the log ---> so it is used as 'wildcard' */
    SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E, /* indication to restore previous unit after use of :
                                SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E
                                SIM_LOG_FRAME_UNIT_CNC_E
                                SIM_LOG_FRAME_UNIT_TCAM_E
                                */

    /* the end */
    SIM_LOG_FRAME_UNIT___LAST___E
}SIM_LOG_FRAME_UNIT_ENT;

/* is unit belong to ingress pipe */
#define SIM_LOG_FRAME_UNIT_IS_INGRESS_PIPE_MAC(logUnitId)           \
    ((((logUnitId) > SIM_LOG_FRAME_UNIT___MUST_BE_FIRST___E) &&     \
      ((logUnitId) < SIM_LOG_FRAME_UNIT_INGRESS_PIPE_E)) ? 1 : 0)

/* is unit belong to egress pipe */
#define SIM_LOG_FRAME_UNIT_IS_EGRESS_PIPE_MAC(logUnitId)            \
    ((((logUnitId) > SIM_LOG_FRAME_UNIT_INGRESS_PIPE_E) &&          \
      ((logUnitId) < SIM_LOG_FRAME_UNIT_EGRESS_PIPE_E)) ? 1 : 0)

/* is unit belong to general units */
#define SIM_LOG_FRAME_UNIT_IS_GENERAL_MAC(logUnitId)                \
    ((((logUnitId) > SIM_LOG_FRAME_UNIT_EGRESS_PIPE_E) &&           \
      ((logUnitId) < SIM_LOG_FRAME_UNIT_GENERAL_E)) ? 1 : 0)


/* info about current frame that generate the 'Packet walkthrough' */
typedef struct SIM_LOG_FRAME_INFO_STCT{
    SIM_LOG_FRAME_COMMAND_TYPE_ENT commandType;
    SIM_LOG_FRAME_UNIT_ENT         unitType;
    SIM_LOG_FRAME_UNIT_ENT         prevUnitType;
}SIM_LOG_FRAME_INFO_STC;


/******************** config vars **********************/
extern GT_BOOL simLogOutputToLogfile;
extern GT_BOOL simLogOutputToStdout;

/* logger file pointer */
extern GT_CHAR simLogFileName[];

/* info type flags */
extern GT_BOOL simLogInfoTypePacket;
extern GT_BOOL simLogInfoTypeDevice;
extern GT_BOOL simLogInfoTypeTcam;
extern GT_BOOL simLogInfoTypeMemory;

/* thread filters */
extern GT_BOOL simLogFilterTypeCpuApplication;
extern GT_BOOL simLogFilterTypeCpuIsr;
extern GT_BOOL simLogFilterTypePpAgingDaemon;
extern GT_BOOL simLogFilterTypePpPipeProcessingDaemon;
extern GT_BOOL simLogFilterTypePpPipeSdmaQueueDaemon;
extern GT_BOOL simLogFilterTypePpPipeOamKeepAliveDaemon;
extern GT_BOOL simLogFilterTypePpPipeGeneralPurpose;
extern GT_BOOL simLogFilterTypePpPipeSoftReset;
extern GT_BOOL simLogFilterTypePreqSrfDaemon;

/**
* @struct SIM_LOG_DEV_PORT_GROUP_FILTERS_STC
 *
 * @brief Describe a device - port group filters
*/
typedef struct{

    /** task Id */
    GT_U32 filterId;

    /** device number */
    GT_U32 devNum;

    /** @brief port group
     *  Comments:
     */
    GT_U32 portGroup;

} SIM_LOG_DEV_PORT_GROUP_FILTERS_STC;

/* dev/port group filters */
extern SIM_LOG_DEV_PORT_GROUP_FILTERS_STC simLogDevPortGroupFilters[SIM_LOG_MAX_DF];

/**
* @enum SIM_LOG_INFO_TYPE_ENT
 *
 * @brief simulation logger information types
*/
typedef enum{

    /** packet specific information */
    SIM_LOG_INFO_TYPE_PACKET_E,

    /** device specific information */
    SIM_LOG_INFO_TYPE_DEVICE_E,

    /** tcam  specific information */
    SIM_LOG_INFO_TYPE_TCAM_E,

    /** memory specific information */
    SIM_LOG_INFO_TYPE_MEMORY_E

} SIM_LOG_INFO_TYPE_ENT;

/* func for opening log file */
GT_STATUS simLogFileOpen(GT_VOID);

/* empty call - it needs to link simLogRuntimeConfig.c correctly */
GT_VOID simLogEmptyFunc(GT_VOID);

/* empty call - it needs to link simLogToRuntime.c correctly */
GT_VOID simLogToRuntimeEmptyFunc(GT_VOID);

/* is the 'log to runtime' active */
extern GT_BIT simLogToRuntimeIsActive;

/**
* @enum SIM_LOG_OUTPUT_TYPE_ENT
 *
 * @brief output config enumeration
*/
typedef enum{

    /** the logger output is disabled */
    SIM_LOG_OUTPUT_DISABLED_E,

    /** the logger output is logfile */
    SIM_LOG_OUTPUT_LOGFILE_E,

    /** the logger output is STDOUT */
    SIM_LOG_OUTPUT_STDOUT_E,

    /** the logger output is logfile and STDOUT */
    SIM_LOG_OUTPUT_BOTH_E

} SIM_LOG_OUTPUT_TYPE_ENT;

/* internal function */
extern void simLogInternalLog
(
    IN const GT_CHAR *formatStringPtr,
    IN ...
);

/* internal function */
extern void simLogInfoSave
(
    IN GT_CHAR               const *funcName,
    IN GT_CHAR               const *fileName,
    IN GT_U32                       lineNum,
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        logInfoType
);

/**
* @internal simLogMessage function
* @endinternal
*
* @brief   This routine logs message.
*
* @retval GT_OK                    - Logging has been done successfully.
* @retval GT_BAD_PARAM             - Wrong parameter.
* @retval GT_FAIL                  - General failure error. Should never happen.
*
* @note Usage example:
*       simLogMessage(SIM_LOG_FUNC_NAME_MAC(snetChtL2i), devObjPtr,
*       SIM_LOG_INFO_TYPE_PACKET_E, "value is: %d\n", 123);
*
*/
GT_STATUS simLogMessage
(
    IN GT_CHAR               const *funcName,
    IN GT_CHAR               const *fileName,
    IN GT_U32                       lineNum,
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        logInfoType,
    IN const GT_CHAR               *formatStringPtr,
    IN ...
);

/**
* @internal simLogClose function
* @endinternal
*
* @brief   This routine closes logger.
*/
GT_STATUS simLogClose();

/**
* @internal simLogInit function
* @endinternal
*
* @brief   Init logger
*
* @retval GT_OK                    - Logger has been successfully initialized.
* @retval GT_FAIL                  - General failure error. Should never happen.
*/
GT_STATUS simLogInit();

/**
* @internal simLogDump function
* @endinternal
*
* @brief   Dumps content of pointer of buffers (by length)
*
* @retval GT_OK                    - success
* @retval GT_BAD_PTR               - wrong pointer
* @retval GT_BAD_PARAM             - wrong params
* @retval GT_FAIL                  - general failure error
*/
GT_STATUS simLogDump
(
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SIM_LOG_INFO_TYPE_ENT        infoType,
    IN GT_PTR                       dataPtr,
    IN GT_U32                       length
);

/**
* @internal simLogSetThreadTypeFilter function
* @endinternal
*
* @brief   This routine manages threads types in runtime.
*
* @param[in] threadType               - type of the thread
* @param[in] enable                   - value
*
* @retval GT_OK                    - success
* @retval GT_BAD_PARAM             - wrong param given
*/
GT_STATUS simLogSetThreadTypeFilter
(
    IN SIM_OS_TASK_PURPOSE_TYPE_ENT  threadType,
    IN GT_BOOL                       enable
);

/**
* @internal simLogSetInfoTypeEnable function
* @endinternal
*
* @brief   This routine manages information types in runtime.
*
* @param[in] infoType                 - type of the info
* @param[in] enable                   - value
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PARAM             -  wrong param given
*/
GT_STATUS simLogSetInfoTypeEnable
(
    IN SIM_LOG_INFO_TYPE_ENT infoType,
    IN GT_BOOL               enable
);

/**
* @internal simLogSetDevPortGroupFilter function
* @endinternal
*
* @brief   This routine manages dev num and specific port group in runtime.
*
* @param[in] devNum                   - device number
* @param[in] portGroup                -  number
* @param[in] enable                   - value
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PARAM             -  wrong param given
*/
GT_STATUS simLogSetDevPortGroupFilter
(
    IN GT_U32     devNum,
    IN GT_U32     portGroup,
    IN GT_BOOL    enable
);

/**
* @internal simLogSetOutput function
* @endinternal
*
* @brief   This routine set ouput configuration.
*
* @param[in] outputType               - type of the output
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PARAM             -  wrong param given
*/
GT_STATUS simLogSetOutput
(
    IN SIM_LOG_OUTPUT_TYPE_ENT outputType
);

/**
* @internal simLogSetFileName function
* @endinternal
*
* @brief   This routine set log file name.
*
* @param[in] fnamePtr                 - (pointer to) fileName.
*
* @retval GT_OK                    -  success
* @retval GT_BAD_PTR               -  wrong pointer
* @retval GT_FAIL                  -  General failure error. Should never happen.
*/
GT_STATUS simLogSetFileName
(
     IN GT_CHAR *fnamePtr
);

/**
* @internal simLogIsOpen function
* @endinternal
*
* @brief   Is the Logger open ?
*
* @retval 0                        - No. logger is NOT open.
* @retval else                     - Yes. logger is open.
*/
GT_U32 simLogIsOpen(void);

/**
* @internal simLogUseLineNumber function
* @endinternal
*
* @brief   state if the 'line numbers' should appear in the logger.
*         by default if this function is not called --> no 'line numbers' indication.
*
* @retval GT_OK                    - on all cases
*/
GT_STATUS simLogUseLineNumber(
    IN GT_U32   useLineNumber
);

/**
* @internal simLogFilterAllowedSpecificStringSet function
* @endinternal
*
* @brief   set filter to allow only lines that 'contain' the 'allowString' (up to 256 characters) .
*         up to 16 strings
* @param[in] allowString              - the allowed string.
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on all cases
*/
GT_STATUS   simLogFilterAllowedSpecificStringSet(
    IN GT_CHAR* allowString,
    IN GT_U32   index
);

/**
* @internal simLogFilterAllowedSpecificFunctionSet function
* @endinternal
*
* @brief   set filter to allow only lines that 'contain' the 'allowFuncName' (up to 256 characters) .
*         up to 16 strings(functions)
* @param[in] allowFuncName            - the allowed function name.
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on all cases
*/
GT_STATUS   simLogFilterAllowedSpecificFunctionSet(
    IN GT_CHAR* allowFuncName,
    IN GT_U32   index
);


/**
* @internal simLogFilterAllowedSpecificUnitSet function
* @endinternal
*
* @brief   set filter to allow only lines that 'belongs' to the 'allowedUnitName'
*         (up to 256 characters) .
*         up to 16 strings (units)
* @param[in] allowedUnitName          - the allowed unit name. (see SIM_LOG_FRAME_UNIT_ENT)
*                                      so names are (lower case ):
*                                      "tti" , "ipcl" , .. "ingress_pipe" ,
*                                      "txq" , "ha" , "epcl" .. "egress_pipe" ,
*                                      "cnc" , "tcam" , .. "general"
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown unit name
*/
GT_STATUS   simLogFilterAllowedSpecificUnitSet(
    IN GT_CHAR* allowedUnitName,
    IN GT_U32   index
);

/**
* @internal simLogFilterDenySpecificPacketCommandSet function
* @endinternal
*
* @brief   set filter to DENY lines that 'belongs' to the 'PacketCommandName'
*         (up to 256 characters) .
*         up to 16 strings (units)
* @param[in] deniedPacketCommandName  - the denied unit name. (see SIM_LOG_FRAME_COMMAND_TYPE_ENT)
*                                      so names are (lower case ):
*                                      "from_cpu"
*                                      NULL means --> remove the filter.
* @param[in] index                    - filter  (0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown packet command name
*/
GT_STATUS   simLogFilterDenySpecificPacketCommandSet(
    IN GT_CHAR* deniedPacketCommandName,
    IN GT_U32   index
);

/**
* @internal simLogToRuntime function
* @endinternal
*
* @brief   Engine of read LOG file and act according to the info:
*         1. do 'write from CPU'
*         2. do 'Read from CPU' (may effect active memory)
*         3. inject packets from the SLAN
*         NOTEs:
*         a. packets from the CPU are ignored as those are byproduct of 'write from CPU'
*         b. packet from 'loopback' ports are ignored because are byproduct of original packet (slan/CPU)
* @param[in] logFileName              - the file of the log to parse.
*                                       None.
*/
void simLogToRuntime(
    IN GT_CHAR*    logFileName
);


/**
* @internal simLogToRuntime_scibDmaRead function
* @endinternal
*
* @brief   Allow the LOG parser to emulate the DMA.
*
* @param[in] clientName               - the DMA client name
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - the number of words/bytes (according to dataIsWords)
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*                                      RETURN:
*                                      COMMENTS:
*/
void simLogToRuntime_scibDmaRead
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal simLogToRuntime_scibDmaWrite function
* @endinternal
*
* @brief   Allow the LOG parser to emulate the DMA.
*
* @param[in] clientName               - the DMA client name
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void simLogToRuntime_scibDmaWrite
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal simLogToRuntime_scibSetInterrupt function
* @endinternal
*
* @brief   Allow the LOG parser to emulate the interrupt
*
* @param[in] deviceId                 - ID of device, which is equal to PSS Core API device ID.
*/
void simLogToRuntime_scibSetInterrupt
(
    IN  GT_U32        deviceId
);

/**
* @internal simLogLinkStateNotify function
* @endinternal
*
* @brief   handle indications about link change from the SLANs
*
* @param[in] deviceObjPtr             - pointer to device object
* @param[in] port                     -  number.
* @param[in] linkState                - link state (0 - down, 1 - up)
*/
void simLogLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT      *     deviceObjPtr,
    IN GT_U32      port,
    IN GT_U32      linkState
);

/**
* @internal simLogAddDevicesInfo function
* @endinternal
*
* @brief   send to log all the info about the devices.
*/
void simLogAddDevicesInfo(void);

/**
* @internal simLogSlanBind function
* @endinternal
*
* @brief   For the LOG to indicate the ports that do Bind/Unbind to slan.
*
* @param[in] slanNamePtr              - (pointer to) slan name , if NULL -->'unbind'
* @param[in] deviceObjPtr             (pointer to) the device object
* @param[in] portNumber               - port number
* @param[in] bindRx                   - bind to Rx direction ? GT_TRUE - yes , GT_FALSE - no
* @param[in] bindTx                   - bind to Tx direction ? GT_TRUE - yes , GT_FALSE - no
*                                       None
*/
void simLogSlanBind (
    IN char                         *slanNamePtr,
    IN SKERNEL_DEVICE_OBJECT        *deviceObjPtr,
    IN GT_U32                       portNumber,
    IN GT_BOOL                      bindRx,
    IN GT_BOOL                      bindTx
);

/*  check if the device if filtered from the LOG ...
    in this case do not put any descriptor change indication */
GT_BOOL simLogIsDevFiltered(
    IN SKERNEL_DEVICE_OBJECT        const *devObjPtr
);

/* Log is opened flag */
extern GT_BOOL simLogIsOpenFlag;

void startSimulationLog(void);
void stopSimulationLog(void);
void startSimulationLogFull(void);
void startSimulationLogWithFromCpu(void);
void setSimulationLogFullPathName(const char* fullPathName);
void startSimulationLogWithFileName(IN char* explicitUniqueName);

GT_STATUS simLogDevFilterAllButOneSet(
    IN GT_U32   simDevNum ,
    IN GT_BOOL enableFilter
);
GT_STATUS simLogDevFilterSet(
    IN GT_U32   simDevNum ,
    IN GT_BOOL enableFilter
);
GT_STATUS simLogDevFilterAllSet(
    IN GT_BOOL enableFilter
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __simLog_h__ */


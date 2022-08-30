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
* @file prvCpssDxChTxqMain.h
*
* @brief CPSS SIP6 TXQ definitions
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChTxqMain
#define __prvCpssDxChTxqMain

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/generic/tm/cpssTmPublicDefs.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPdq.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/generic/port/cpssPortTx.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxUburst.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqDebugUtils.h>


#define TXQ_U32_SET_FIELD_MASKED_CHECK_OBSOLETE_MAC(data,offset,length,val)           \
   if (offset!=CPSS_SIP6_TXQ_INVAL_DATA_CNS&&length!=CPSS_SIP6_TXQ_INVAL_DATA_CNS)\
     U32_SET_FIELD_MASKED_MAC(data,offset,length,val)

#define PRV_CPSS_DXCH_PORT_NUM_MAPPING_INVALID_PORT_CNS ((GT_U32)(~0))


#define PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC 48

#define PRV_CPSS_DXCH_SIP_6_TXQ_PROFILE_BMP_WORDS_NUM_MAC 32


/*Eligeble function mapping*/

#define  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_A_NODE(_TC)  CPSS_PDQ_SCHED_ELIG_N_FP##_TC##_E
#define  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_A_NODE_WITH_SHAPING(_TC) CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP##_TC##_E

#define PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_B_NODE CPSS_PDQ_SCHED_ELIG_N_FP1_E /*TM_ELIG_N_PRIO1*/
#define PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_B_NODE_WITH_SHAPING CPSS_PDQ_SCHED_ELIG_N_MIN_SHP_FP1_E

#define PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_C_NODE CPSS_PDQ_SCHED_ELIG_N_FP1_E /*TM_ELIG_N_PRIO1*/
#define PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_P_NODE CPSS_PDQ_SCHED_ELIG_N_FP1_E /*TM_ELIG_N_PRIO1*/


#define  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_Q_NODE(_TC)  CPSS_PDQ_SCHED_ELIG_Q_PRIO##_TC##_E
#define  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_Q_NODE_WITH_SHAPING_AND_MIN_BW(_TC) CPSS_PDQ_SCHED_ELIG_Q_SHP_SCHED##_TC##0_PROP##_TC##0
#define  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_Q_NODE_WITH_SHAPING(_TC) CPSS_PDQ_SCHED_ELIG_Q_MIN_SHP_PRIO##_TC##_E
#define  PRV_CPSS_DEFAULT_ELIG_FUNC_FOR_Q_NODE_MIN_BW(_TC) CPSS_PDQ_SCHED_ELIG_Q_SCHED##_TC##0_PROP##_TC##0

#define PRV_CPSS_NUM_OF_TILE_GET_MAC(_devNum,_numberOfTiles)\
    switch(PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles)\
    {\
        case 0:\
        case 1:\
            _numberOfTiles = 1;\
            break;\
       case 2:\
       case 4:\
            _numberOfTiles =PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles;\
            break;\
        default:\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles);\
            break;\
    }

#define PRV_CPSS_TILE_HANDLE_GET_MAC(_devNum,_tileNum) (&(PRV_CPSS_DXCH_PP_MAC(_devNum)->port.tileConfigsPtr[_tileNum]))



#define PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(_file) \
    CPSS_OS_FILE_TYPE_STC   stdOut;\
    do\
    {\
        if(NULL==file)\
        {\
            stdOut.type=CPSS_OS_FILE_STDOUT;\
            stdOut.fd  = (CPSS_OS_FILE)stdout;\
            _file=&stdOut;\
        }\
    }while(0);\

#define CPSS_TXQ_DEBUG_LOG(_format,_param0,_param1,_param2,_param3) \
            prvCpssTxqSip6DebugLog(devNum, __FILENAME__,__FUNCNAME__, __LINE__,_format,_param0,_param1,_param2,_param3)

#define CPSS_TXQ_DEBUG_EXT_LOG(_format,_funcname,_param0,_param1,_param2,_param3) \
            prvCpssTxqSip6DebugLog(devNum, __FILENAME__,_funcname, __LINE__,_format,_param0,_param1,_param2,_param3)


#define PRV_CPSS_DXCH_SIP6_TXQ_MAX_NUM_OF_SUPPORTED_SPEEDS_CNS             14


#define PRV_CPSS_DXCH_SIP6_TXQ_ALL_Q_MAC             0xFF

#define TXQ_IS_PREEMPTIVE_DEVICE(_devNum) (PRV_CPSS_PP_MAC(_devNum)->preemptionSupported)

#define TXQ_IS_AC5P_DEVICE(_devNum) (PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)

/* the number of DPs in tile */
#define MAX_DP_IN_TILE(devNum)                                              \
    (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles ?                        \
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp /         \
         PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles):                    \
     PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp)



/*SIP 6 maximal PDS per tile number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC      MAX_DP_PER_TILE_CNS

/*SIP 6 maximal SDQ per tile number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC      MAX_DP_PER_TILE_CNS

/*SIP 6 maximal QFC per tile number*/
#define CPSS_DXCH_SIP_6_MAX_LOCAL_QFC_NUM_MAC      MAX_DP_PER_TILE_CNS


/*SIP 6 maximal global PDQ  number*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_NUM                4

/*SIP 6 maximal global PDX  number*/
#define CPSS_DXCH_SIP_6_MAX_PDX_NUM_MAC            4

/* 2 MACROS to remove the use of 'sip_6_10' and 'sip_6_15' in TXQ code */
#define PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(_dev) (PRV_CPSS_PP_MAC(_dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)

#define PRV_CPSS_TXQ_HARRIER_RESERVED_PORTS_BMP_MAC 0x3FF0000 /*ports 16 -25*/

/*SIP 6 maximal local port  number (reserved ports included)*/                  /* 9 in Falcon , 27 in Hawk */
#define CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(_dev)  \
    ((PRV_CPSS_PP_MAC(_dev)->devFamily != CPSS_PP_FAMILY_DXCH_HARRIER_E) ?  \
    PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts : \
    27/*like Hawk*/)

/*SIP 6 maximal global PDS  number*/
#define CPSS_DXCH_SIP_6_MAX_GLOBAL_PDS_NUM_MAC     32

/*SIP 6 maximal global SDQ  number*/
#define CPSS_DXCH_SIP_6_MAX_GLOBAL_SDQ_NUM_MAC     32


/*SIP 6 maximal  number of sceduler A nodes*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC                      512


/*SIP 6 maximal  number of sceduler port nodes*/                 /* 72 in Falcon , 128 in Hawk */
#define CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(_dev)                   PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.multiDataPath.pdqNumPorts
/*SIP 6 maximal  number of profiles at PDS*/
#define CPSS_DXCH_SIP_6_MAX_PDS_PROFILE_NUM                      16

/*SIP 6 maximal  number of profiles at scheduler*/
#define CPSS_DXCH_SIP_6_MAX_SCHED_PROFILE_NUM                      16

/*SIP 6 maximal  number PDQ queues*/
#define CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM                        _2K
/*SIP 6 maximal  number of data path  queues*/
#define CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC              PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues

/*SIP 6 maximal  number of queue group*/
#define  CPSS_DXCH_SIP_6_MAX_Q_GROUP_SIZE_MAC                    _1K
#define  CPSS_DXCH_SIP_6_10_MAX_Q_GROUP_SIZE_MAC                 128


/*SIP 6  max TC*/
#define CPSS_DXCH_SIP_6_MAX_TC_QUEUE_NUM_MAC                    16

#define PRV_TC_MAP_PB_MAC    0x4

/*SIP 6 maximal  number of uburst profiles*/
#define CPSS_DXCH_SIP_6_MAX_UBURST_PROFILE_NUM_MAC                    32


/*macro define maximal number of configured slices*/
#define TXQ_PDX_MAX_SLICE_NUMBER_MAC 33

/*macro define minimal number of configured slices*/
#define TXQ_PDX_MIN_SLICE_NUMBER_MAC 32
/*SIP 6 macro for getting field offset by unit/sub-unit/register/filed name*/
#define GET_REGISTER_FIELD_OFFSET_MAC(_UNIT,_SUB_UNIT,_REG,_FIELD)   _UNIT##_##_SUB_UNIT##_##_REG##_##_FIELD##_FIELD_OFFSET
/*SIP 6 macro for getting field size by unit/sub-unit/register/filed name*/
#define GET_REGISTER_FIELD_SIZE_MAC(_UNIT,_SUB_UNIT,_REG,_FIELD)   _UNIT##_##_SUB_UNIT##_##_REG##_##_FIELD##_FIELD_SIZE

/*macro define max descriptor number in pds*/
#define TXQ_PDS_MAX_DESC_NUMBER_MAC 3071

#define DEFAULT_LENGTH_ADJUST_PROFILE 0

#define PRV_TXQ_LOG_NO_INDEX 0xFEFEFEFE

#define PDS_PROFILE_MAX_MAC 16

#define PRV_CPSS_DXCH_CIDER_INFO_SIZE 4



/*SIP 6 macro for checking validity of value vs field length*/
#define TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(_fieldValue,_fieldLength)\
    do\
      {\
        if(_fieldValue>=(GT_U32)(1<<_fieldLength))\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "%s must be less than [%d],field lenght is %d",#_fieldValue,(1<<_fieldLength),_fieldLength);\
        }\
      }while(0)


/*SIP 6 macro for checking validity of local PDS  number*/
#define TXQ_SIP_6_CHECK_TILE_NUM_MAC(_tileNum)\
    do\
      {\
        GT_U32 numOfTilesInMacro;\
        switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)\
        {\
            case 0:\
            case 1:\
                numOfTilesInMacro = 1;\
                break;\
           case 2:\
           case 4:\
                numOfTilesInMacro =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;\
                break;\
            default:\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);\
                break;\
        }\
        if(_tileNum>=numOfTilesInMacro)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tile number [%d] must be less than [%d]",_tileNum,numOfTilesInMacro);\
        }\
      }while(0)


/*SIP 6 macro for checking validity of local PDS  number*/
#define TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(_pds)\
    do\
      {\
        if(_pds>=MAX_DP_IN_TILE(devNum))\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local pds[%d] must be less than [%d]",_pds,MAX_DP_IN_TILE(devNum));\
        }\
      }while(0)
/*SIP 6 macro for checking validity of global PDS  number*/
#define TXQ_SIP_6_CHECK_GLOBAL_PDS_NUM_MAC(_pds)\
            do\
              {\
                if(_pds>=CPSS_DXCH_SIP_6_MAX_GLOBAL_PDS_NUM_MAC)\
                {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "global pds[%d] must be less than [%d]",_pds,CPSS_DXCH_SIP_6_MAX_GLOBAL_PDS_NUM_MAC);\
                }\
              }while(0)
/*SIP 6 macro for checking validity of PDX*/
#define TXQ_SIP_6_CHECK_PDX_NUM_MAC(_pdx)\
            do\
              {\
                if(_pdx>=CPSS_DXCH_SIP_6_MAX_PDX_NUM_MAC)\
                {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pdx[%d] must be less than [%d]",_pdx,CPSS_DXCH_SIP_6_MAX_PDX_NUM_MAC);\
                }\
              }while(0)

                /*SIP 6 macro for checking validity of global port*/
 #define TXQ_SIP_6_CHECK_GLOBAL_PORT_NUM_MAC(_dev,_portNum)\
    do\
      {\
        if(_portNum>= CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(_dev))\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " portNum[%d] must be less than [%d]",_portNum,CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(_dev));\
        }\
      }while(0);


 #define TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(_dev,_portNum)\
          do\
          {\
            if(_portNum>= CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(_dev))\
            {\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d] must be less than [%d]",_portNum,CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(_dev));\
            }\
            if(PRV_CPSS_PP_MAC(_dev)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)\
            {\
                if(PRV_CPSS_TXQ_HARRIER_RESERVED_PORTS_BMP_MAC&(1<<_portNum))\
                {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d]considered reserved - can not be used",_portNum);\
                }\
            }\
          }while(0);

/*SIP 6 macro for checking validity of queue group*/
#define TXQ_SIP_6_CHECK_QUEUE_GROUP_NUM_MAC(_dev,_queueGroup)\
           do\
             {\
               if((_queueGroup>= PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.multiDataPath.pdxNumQueueGroups))\
               {\
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "queueGroup[%d] must be less than [%d]",_queueGroup,PRV_CPSS_DXCH_PP_MAC(_dev)->hwInfo.multiDataPath.pdxNumQueueGroups);\
               }\
             }while(0);

/*SIP 6 macro for checking validity of global sdq num*/
#define TXQ_SIP_6_CHECK_GLOBAL_SDQ_NUM_MAC(_sdq)\
           do\
             {\
               if(_sdq>=CPSS_DXCH_SIP_6_MAX_GLOBAL_SDQ_NUM_MAC)\
               {\
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "global sdq[%d] must be less than [%d]",_sdq,CPSS_DXCH_SIP_6_MAX_GLOBAL_SDQ_NUM_MAC);\
               }\
             }while(0)
/*SIP 6 macro for checking validity of  pdq num*/
#define TXQ_SIP_6_CHECK_PDQ_NUM_MAC(_pdq)\
           do\
             {\
               if(_pdq>=CPSS_DXCH_SIP_6_MAX_PDQ_NUM)\
               {\
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pdq[%d] must be less than [%d]",_pdq,CPSS_DXCH_SIP_6_MAX_PDQ_NUM);\
               }\
             }while(0)

/*SIP 6 macro for checking validity of local sdq num*/
#define TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(_sdq)\
       do\
         {\
           if(_sdq>=MAX_DP_IN_TILE(devNum))\
           {\
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local sdq[%d] must be less than [%d]",_sdq,MAX_DP_IN_TILE(devNum));\
           }\
         }while(0)

           /*SIP 6 macro for checking validity of local qfc num*/
#define TXQ_SIP_6_CHECK_LOCAL_QFC_NUM_MAC(_qfc)\
          do\
            {\
              if(_qfc>=MAX_DP_IN_TILE(devNum))\
              {\
                  CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local qfc[%d] must be less than [%d]",_qfc,MAX_DP_IN_TILE(devNum));\
              }\
            }while(0)

#define PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(_pNodePtr,_device,_tile,_pNodeInd)\
       do \
       { \
          _pNodePtr = prvCpssSip6TxqUtilsPnodeInstanceGet(_device,_tile,_pNodeInd);\
      \
          if(NULL == _pNodePtr)\
          {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "P node is NULL pointer\n");\
          }\
       }while(0);\

#define PRV_TXQ_SIP_6_PNODE_INSTANCE_NO_ERROR_GET(_pNodePtr,_device,_tile,_pNodeInd)\
       do \
       { \
          _pNodePtr = prvCpssSip6TxqUtilsPnodeInstanceGet(_device,_tile,_pNodeInd);\
       }while(0);\

#define PRV_TXQ_LOG_STRING_BUFFER_SIZE_CNS       1024


#define PRV_CPSS_TXQ_UTILS_SKIP_RESERVED_MAC(_devNum,_localPortNum) \
        if(PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)\
        {\
            if(PRV_CPSS_TXQ_HARRIER_RESERVED_PORTS_BMP_MAC&(1<<_localPortNum))\
            {\
               continue;\
            }\
        }



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_ENT
*
* @brief Type of Port/TC buffer counter
*/

  typedef enum{

      /**occupied buffers*/
      PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_OCCUPIED_BUFFERS,

      /** headroom count*/
      PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_HEADROOM_BUFFERS,

      /** PFC count*/
      PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_PFC_BUFFERS

  } PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_ENT;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_ENT
*
* @brief Type of global headroom counter
*/
typedef enum{

      /**Global TC*/
      PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_GLOBAL_TC,

      /** Pool */
      PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_POOL

  }PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_ENT;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_DMA_TO_PNODE_MAP_STC
*
* @brief Describe Pnode
*/
  typedef struct
  {
       /** index of Pnode[0..71]*/
      GT_U32              pNodeInd;

       /** index of tile[0..3]*/
      GT_U32              tileInd;

  } PRV_CPSS_DXCH_TXQ_SIP_6_DMA_TO_PNODE_MAP_STC;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PHYSICAL_PORT_TO_ANODE_MAP_STC
 *
 * @brief  Describe Pnode
*/
  typedef struct
  {
      /** index of aNodeInd[0..511] */
      GT_U32              aNodeInd;
          /** index of tile[0..3] */
      GT_U32              tileInd;

  } PRV_CPSS_DXCH_TXQ_SIP_6_PHYSICAL_PORT_TO_ANODE_MAP_STC;



/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC
*
* @brief The profile parameters regsrding long queue managment.
*  Applicable devices : Falcon
*/

typedef struct
{

     /** @brief
   *Enables writing a queue to the PB when it is getting long
   */

     GT_BOOL longQueueEnable;

       /** @brief
    * When the head counter is below this limit, the head is considered empty  and fragment read from PB is triggered
    */

    GT_U32 headEmptyLimit;

     /** @brief
    * When the total queue counter reaches this limit, a long queue is opened.
    */
    GT_U32 longQueueLimit;

} PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC;


/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC
 *
 * @brief The profile parameters regsrding length adjust managment.
 *  Applicable devices : Falcon
*/

typedef struct
{
      /** @brief
    *If equals GT_TRUE the length adjust is enabled ,else disabled.
    */

        GT_BOOL lengthAdjustEnable;


    /** @brief
    *If equals GT_TRUE the value of constant byte count field is subtracted from the descriptor byte count ,else
    * the value of constant byte count field is added to the descriptor byte count. Applicable only if lengthAdjustEnable set to GT_TRUE.
    */

     GT_BOOL lengthAdjustSubstruct;

     /** @brief
    *Sets the value which will be decremented or incremented from the packet's byte count.Applicable only if lengthAdjustEnable set to GT_TRUE.
    */
     GT_U32 lengthAdjustByteCount;


} PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC;


/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC
 *
 * @brief The profile parameters regarding queue profile.
 *  Applicable devices : Falcon
*/

typedef struct
{

    /** @brief
    * Parameters regarding length adjust.
    */
     PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *lengthAdjustParametersPtr;

     /** @brief
    * Parameters regarding long queue managment.
    */

    PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC         *longQueueParametersPtr;

    GT_U32                                                   numberOfBindedPorts;

    GT_U32                                                   speed_in_G;

        /** @brief Binded ports bitmap .Each bit represent port (if bit is set then port is binded to profile).
   */

     GT_U32 bindedPortsBmp[PRV_CPSS_DXCH_SIP_6_TXQ_PROFILE_BMP_WORDS_NUM_MAC];

} PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_STC;



/************start of mapping configurations***************/






/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PORT_BW_DATA
 *
 * @brief Port speed shadow.
*/
typedef struct{

      /** port speed */

    GT_U32 speed;

} PRV_CPSS_DXCH_TXQ_SIP_6_PORT_BW_DATA;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE
 *
 * @brief Describe list of A node connected to local port
*/
typedef struct{

    /** @brief size of A node list [0..CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC].
     *  0 means that local port is disabled
     */
    GT_U32 aNodeListSize;

    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *aNodelist;

    /** the number of the first queue in this range */
    GT_U32 sdqQueueFirst;

    /** the number of the last queue in this range */
    GT_U32 sdqQueueLast;

     /** if equal GT_TRUE then then Pnode represent cascade port,GT_FALSE otherwise */
    GT_BOOL isCascade;

      /** if isCascade equal GT_TRUE then this  parameter represent cascade port physical number */
    GT_U32 cascadePhysicalPort;
    /*Index of a P node*/
    GT_U32 pNodeIndex;
    /*Debug counters to count failures*/
    PRV_CPSS_DXCH_TXQ_SIP_6_ALERT_COUNTERS alertCounters;
} PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE;



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC
 *
 * @brief Data base for quick search in txQ mapping data base
*/
typedef struct{

    /** A node to Pnode mapping */
    GT_U32 * aNodeIndexToPnodeIndex;
    GT_U32   aNodeIndexToPnodeIndexSize;

     /** Q node to Anode mapping */
    GT_U32 *qNodeIndexToAnodeIndex;
    GT_U32 qNodeIndexToAnodeIndexSize;

     /** group of queues to physical port mapping*/
    GT_U32 *goqToPhyPortIndex;
    GT_U32 goqToPhyPortSize;

} PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PDX_PIZZA_CONFIG_STC
 *
 * @brief Describe PDX pizza mapping
*/
typedef struct{

     /** number of slices per DP (dp without CPU port get less slices*/
    GT_U32 pdxPizzaNumberOfSlices[CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC];

} PRV_CPSS_DXCH_TXQ_SIP_6_PDX_PIZZA_CONFIG_STC;



/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION
 *
 * @brief Describe mapping of P node to A node and physical port
*/
typedef struct{

    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE **pNodeMappingConfiguration;

    /** size of pNodeMappingConfiguration array */
    GT_U32 size;

    /** Data base for quick search in txQ mapping data base */
    PRV_CPSS_DXCH_TXQ_SIP6_SEARCH_STC searchTable;

    /** Describe PDX pizza mapping */
    PRV_CPSS_DXCH_TXQ_SIP_6_PDX_PIZZA_CONFIG_STC pdxPizza;

    GT_U32                                       firstQInDp[MAX_DP_PER_TILE_CNS];

    GT_U32                                       mappingMode;

    /** Control alligned mapping of A nodes ,if equal GT_TRUE then a nodes indexes are alligned to L2 cluster size */
    GT_BOOL                                      aNodeAllignedMapping;

    /** Number of mapped queues in the tile */
    GT_U32                                      mappedQNum;

} PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION;

typedef struct{

    GT_BOOL schedLogWriteRequests;
    GT_BOOL schedLogReadRequests;
} PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_DBG_STC;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION
 *
 * @brief General configuration of tile
*/
typedef struct{

    GT_U32                    pdxMapping[4];
    /** GT_TRUE if tx library  is  initialized ,GT_FALSE otherwise*/
    GT_BOOL                   txLibInitDone;
    /** Number of ports mapped at the device*/
    GT_U32                   numberOfMappedPorts;
    GT_VOID                 *addressSpacePtr;
    /* CPSS-11271 */
    GT_U32                                poolLimitWaBmp;
    GT_BOOL                               logEnable;
    GT_BOOL                               dataPathEventLog;
    GT_BOOL                               burstOptimization;
    GT_BOOL                               logShapingResults;
    PRV_CPSS_DXCH_TXQ_SIP_6_SCHED_DBG_STC schedDbg;


} PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION;


/************end of mapping configurations***************/

/************start of BW configurations***************/




/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE_LENGTH_ADJUST
 *
 * @brief Describe SDQ profile length adjust on deque
*/

typedef struct
{
    GT_BOOL enable;
    GT_BOOL substruct;
    GT_U32  byteCount;
} PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE_LENGTH_ADJUST;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE
 *
 * @brief Describe SDQ profile
*/
typedef struct{

    /** Aging threshold in clock cycles [0 */
    GT_BOOL longQueueEnable;

    /** Credit High Threshold[0 */
    GT_U32 headEmptyLimit;

    /** Credit Low Threshold[0 */
    GT_U32 longQueueLimit;

    /** Minimum credit budget for selection.[0 */
    PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE_LENGTH_ADJUST lengthAdjust;

} PRV_CPSS_DXCH_TXQ_SIP_6_PDS_PROFILE;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_NODE_SHAPING_PROFILE_STC
 *
 * @brief Txq profiles
*/
typedef struct{

    /** GT_TRUE if shaping profile is valid ,GT_FALSE otherwise */
    GT_BOOL valid;

    /** Shaping profile */
    CPSS_PDQ_SCHED_SHAPING_PROFILE_PARAMS_STC profile;

} PRV_CPSS_DXCH_TXQ_SIP_6_NODE_SHAPING_PROFILE_STC;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_QUEUE_DATA
 *
 * @brief Queue description
*/
typedef struct{

    /** traffic class [0 */
    GT_U32 tc;

    /** index of sdq profiles [0 */
    GT_U32 sdqProfileIndex;

    /** index of shaping profiles[0 */
    GT_U32 shapingProfilePtr;

    /** Aging threshold in clock cycles [0 */
    GT_U32 agingTh;

    /** @brief Credit High Threshold[0
     *  highCreditTh           - Credit Low Threshold[0 - 524287]
     */
    GT_U32 highCreditTh;

    GT_U32 lowCreditTh;

    /** Minimum credit budget for selection.[0 */
    GT_U32 negativeCreditTh;

} PRV_CPSS_DXCH_TXQ_SIP_6_QUEUE_DATA;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_BW_DATA
 *
 * @brief A-Node Parameters Data Structure.
*/
typedef struct{

    /** @brief Index of Shaping profile
     *  (APPLICABLE RANGES: 1..CPSS_DXCH_SIP_6_MAX_PDQ_A_NODES_MAC,
     *  CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS)
     *  number of tree levels nodes can be retrived with cpssTmTreeParamsGet.
     */
    GT_U32 shapingProfilePtr;

    /** @brief DWRR Quantum in resolution of CPSS_TM_NODE_QUANTUM_UNIT_CNS (256) bytes
     *  (APPLICABLE RANGES: 0x40..0x3FFF).
     *  Quantum Limits depend on system MTU and can be retrieved with
     *  cpssTmNodeQuantumLimitsGet API.
     *  schdModeArr[8]      - RR/DWRR Priority for A-Node Scheduling
     *  (APPLICABLE RANGES: (CPSS_TM_SCHD_MODE_ENT) 0..1).
     */
    GT_U32 quantum;

    CPSS_PDQ_SCHD_MODE_ENT schdModeArr[8];

} PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE_BW_DATA;

/************************************start  of global configurations**************************/

typedef struct
{
    GT_BOOL monitorActive;
    GT_U32  activeDp;
} PRV_CPSS_DXCH_HR_DB_INFO_STC;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION
 *
 * @brief SIP 6 mapping and bandwith related txq configuration
*/
typedef struct{

    /** General configuration of tile */
    PRV_CPSS_DXCH_TXQ_SIP_6_GENERAL_CONFIGURATION           general;

    /** Describe mapping of P node to A node and physical port */
    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION           mapping;

    /** Save last uburst events  received from this tile*/
    PRV_CPSS_DXCH_HR_DB_INFO_STC                            headRoomDb;

} PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION;


/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC
 *
 * @brief SIP 6 loging information structure
*/
typedef struct{

    /** whether log or not*/
    GT_BOOL  log;

    /** Register name */
    GT_CHAR_PTR  regName;

    /** Register index*/
    GT_U32  regIndex;

    /** unit name*/
    GT_CHAR_PTR  unitName;

    /** unit index*/
    GT_U32                            unitIndex;

} PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC;

/**
* @struct PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC
 *
 * @brief SIP 6 loging information structure
*/
typedef struct{

    /** whether log or not*/
    GT_BOOL  log;

    /** table name */
    GT_CHAR_PTR  tableName;

    /** tile index */
    GT_U32  tileIndex;

    /** unit name*/
    GT_CHAR_PTR  unitName;

    /** unit (data path) index*/
    GT_U32   unitIndex;

    /** entry index */
    GT_U32  entryIndex;

} PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC;


typedef struct{

    /** Device family*/
    CPSS_PP_FAMILY_TYPE_ENT family;

    /** Cider revision*/
    GT_U32  revision;
    /** Current code version*/
    GT_U32  currentVersion;
    /** Last availeble version*/
    GT_U32  lastVersion;

} PRV_CPSS_TXQ_CIDER_INFO_STC;


/**
* @internal prvCpssFalconTxqUtilsConvertToDb function
* @endinternal
*
* @brief   Convert to txq database format from CPSS_DXCH_PORT_MAP_STC .
*         Create mapping between P nodes /A nodes and physical port numbers
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] sourcePtr                - Pointer to CPSS_DXCH_PORT_MAP_STC to be converted into scheduler tree
* @param[in] size                     - of source array.Number of ports to map, array size
* @param[in] tileNum                  -  The number of tile
*
* @param[out] targetPtr                - Pointer to schedualler mappping configuration
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqUtilsConvertToDb
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_PORT_MAP_STC * sourcePtr,
    IN GT_U32 size,
    IN GT_U32 tileNum,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  *  targetPtr
);

/**
* @internal prvCpssFalconTxqUtilsPdxQroupMapTableInit function
* @endinternal
*
* @brief   Initialize PDX DX_QGRPMAP table (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] mappingPtr               - pointer to SW mapping shadow
* @param[out] entriesAddedPtr               - pointer to number of added entries
*
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsPdxQroupMapTableInit
(
    IN  GT_U8 devNum,
    IN  PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION  * mappingPtr,
    IN GT_U32 tileNum,
    OUT GT_U32 * entriesAddedPtr
);

/**
 * @internal prvCpssFalconTxqUtilsInitSdq function
 * @endinternal
 *
 * @brief   Initialize SDQ (write to HW) accoring to SW txq shadow
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 * @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
 * @param[in] totalTiles                  -Number of tiles in the device (Applicable range 1..4)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong pdx number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsInitSdq
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 totalTiles
);

/**
* @internal prvCpssFalconTxqUtilsInitPds function
* @endinternal
*
* @brief   Initialize PDS .Set length adjust parameters for all the queues in perticular PDS.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   PP's device number.
* @param[in] tileNum                  -Then number of tile (Applicable range 0)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitPds
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);

/**
* @internal prvCpssFalconTxqUtilsInitPsi function
* @endinternal
*
* @brief   Initialize PSI (write to HW) accoring to SW txq shadow
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  -Then number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS prvCpssFalconTxqUtilsInitPsi
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);
/**
* @internal prvCpssFalconTxqUtilsBuildPdqTree function
* @endinternal
*
* @brief   Build default schedualing tree
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNumber               -Then number of tile (Applicable range 0..3)
* @param[in] configPtr                - pointer to SW mapping shadow
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsBuildPdqTree
(
    IN GT_U8 devNum,
    IN  GT_U32 tileNumber,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION  * configPtr
);

/**
 * @internal prvCpssFalconTxqUtilsInitDb function
 * @endinternal
 *
 * @brief   Initialize txq database
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] tileConfigsPtr             - pointer to SW  shadow
 * @param[in] tileNumber                 - the number of the tile(APPLICABLE RANGES:0..3)
  * @param[in] numberOfTiles                 - the number of the tiles on device(APPLICABLE RANGES:1..4)
 */
GT_VOID  prvCpssFalconTxqUtilsInitDb
(
    IN GT_U8 devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr,
    IN GT_U32 tileNumber,
    IN GT_U32 numberOfTiles
);

/**
* @internal prvCpssFalconTxqUtilsInitDmaToPnodeDb function
* @endinternal
 *
* @brief   Initialize DmaToPnode database
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
*/

GT_VOID  prvCpssFalconTxqUtilsInitDmaToPnodeDb
(
    IN GT_U8 devNum
);


/**
* @internal prvCpssFalconTxqUtilsPsiConfigDump function
* @endinternal
*
* @brief   Debug function that dump PDQ queue to SDP/PDS mapping
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
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum
);
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
*                                      tileNum      - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump
(
    IN  GT_U8 devNum,
    IN  GT_U32 firstPort,
    IN  GT_U32 lastPort
);

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
    IN  GT_U8  devNum,
    IN  GT_U32 pdxNum,
    IN  GT_U32 lastEntryNum,
    IN  GT_BOOL dumpPizza
);
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
    IN  GT_U8   devNum,
    IN  GT_U32  tileNumber,
    IN  GT_U32  sdqNum,
    IN  GT_BOOL portSpecific,
    IN  GT_U32  port
);

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
    IN  GT_U8  devNum,
    IN  GT_U32 tileNumber,
    IN  GT_U32 sdqNum,
    IN  GT_U32 startQ,
    IN  GT_U32 size,
    IN  CPSS_OS_FILE_TYPE_STC * file
);


/**
* @internal prvCpssDxChTxQFalconPizzaArbiterInitPerTile function
* @endinternal
*
* @brief   Initialize the number of silices ta each PDX PDS pizza arbiter,also initialize all the slices to unused
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                    -PP's device number.
* @param[in] pdxNum                    -tile number
* @param[in] pdxPizzaNumberOfSlicesArr -number of slices per dp
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerTile
(
    IN GT_U8 devNum,
    IN GT_U32 pdxNum,
    IN GT_U32 * pdxPizzaNumberOfSlicesArr
);

/**
* @internal prvCpssFalconTxqUtilsInitPdx function
* @endinternal
*
* @brief   Initialize PDX unit to default values
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] tileNum                  - tile number.(APPLICABLE RANGES:0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsInitPdx
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
);


/**
* @internal prvCpssFalconTxqUtilsPortTxBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated per specified port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
*                                      portNum  - physical or CPU port number
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsPortTxBufNumberGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_U32 * numPtr
);
/**
* @internal prvCpssFalconTxqUtilsQueueBufPerTcNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numPtr                   - number of buffers
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsCounterPerPortTcGet
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_TXQ_SIP_6_COUNTER_TYPE_ENT    counterType,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  GT_U8                                       trafClass,
    OUT GT_U32                                      *numPtr
);

/**
* @internal prvCpssFalconTxqUtilsQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical or CPU port number.
* @param[in] tcQueue                  - traffic class queue on this device (0..7).
* @param[in] enable                   - if GT_TRUE queue  is enabled, otherwise disabled.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsQueueEnableSet
(
     IN GT_U8 devNum,
     IN  GT_PHYSICAL_PORT_NUM portNum,
     IN  GT_U8     tcQueue,
     IN GT_BOOL enable
);
/**
* @internal prvCpssFalconTxqUtilsQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  device number
* @param[in] portNum                  - physical or CPU port number.
* @param[in] tcQueue                  - traffic class queue on this device (0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqUtilsQueueEnableGet
(
     IN GT_U8 devNum,
     IN  GT_PHYSICAL_PORT_NUM portNum,
     IN  GT_U8     tcQueue,
     IN GT_BOOL * enablePtr
);
/**
* @internal prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb function
* @endinternal
 *
* @brief  Initialize physical port to A nodedatabase
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                  -  device number
*/

GT_VOID  prvCpssFalconTxqUtilsInitPhysicalPortToAnodeDb
(
    IN GT_U8 devNum
);

/**
* @internal prvCpssFalconTxqUtilsGetQueueGroupAttributes function
* @endinternal
 *
* @brief  Get attributes of queue group
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] queueGroup           - queue group index
* @param[out] tilePtr                   - tile of the queue group
* @param[out] dpPtr  - dp of the queue group
* @param[out]  queueBaseInDpPtr - local port of the queue group
* @param[out] queueBaseInTilePtr - global queue index (per PDQ)
*/

GT_STATUS  prvCpssFalconTxqUtilsGetQueueGroupAttributes
(
    IN GT_U8 devNum,
    IN GT_U32 queueGroup,
    OUT GT_U32 * tilePtr,
    OUT GT_U32 * dpPtr,
    OUT GT_U32 * localPortPtr,
    OUT GT_U32 * queueBaseInDpPtr,
    OUT GT_U32 * queueBaseInTilePtr
);

/**
* @internal prvCpssDxChTxQFalconPizzaArbiterInitPerDp function
* @endinternal
 *
* @brief   Initialize the number of silices to each PDS pizza arbiter,also initialize all the slices to unused
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -PP's device number.
*@param[in] pdxNum                   -tile number
*@param[in] pdsNum                   -pdsNum number
*@param[in] pdxPizzaNumberOfSlicesArr-number of slices per dp
*/

GT_STATUS prvCpssDxChTxQFalconPizzaArbiterInitPerDp
(
    IN  GT_U8                       devNum,
    IN GT_U32                      pdxNum,
    IN GT_U32                      pdsNum,
    IN GT_U32                      pdxPizzaNumberOfSlices
);

/**
* @internal prvCpssDxChTxQFalconPizzaConfigurationSet function
* @endinternal
 *
* @brief   Update pizza arbiter
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                   -PP's device number.
*@param[in] pdxNum                   -tile number
*@param[in] pdsNum                   -pdsNum number
*@param[in] localChannelIndex -the slice owner[0-7]
*@param[in] portPizzaSlicesBitMap -the slice owner[0-7]
*@param[in] enable -equal GT_TRUE in case this is owned ,GT_FALSE otherwise
*@param[in] isCpu -equal GT_TRUE in case this is slice for CPU ,GT_FALSE otherwise
*/

GT_STATUS prvCpssDxChTxQFalconPizzaConfigurationSet
(
    IN  GT_U8                       devNum,
    IN GT_U32                      pdxNum,
    IN GT_U32                      pdsNum,
    IN  GT_U32                     localChannelIndex,
    IN  GT_U32                     portPizzaSlicesBitMap,
    IN  GT_BOOL                 enable,
    IN  GT_BOOL                 isCpu
);
/**
* @internal prvCpssFalconTxqUtilsIsCascadePort function
* @endinternal
 *
* @brief  Check if port has queues mapped at TxQ
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[out] isCascadePtr  - if equal GT_TRUE this port is cascade,GT_FALSE otherwise

*/

GT_STATUS prvCpssFalconTxqUtilsIsCascadePort
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    OUT GT_BOOL * isCascadePtr,
    OUT PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE **pNodePtr
);


/**
* @internal prvCpssFalconTxqGlobalTxEnableSet function
* @endinternal
 *
* @brief   Enable/Disable transmission of specified device.
*                Set enable/disable on all tiles (1/2/4)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
*/
GT_STATUS   prvCpssFalconTxqGlobalTxEnableSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);
/**
* @internal prvCpssFalconTxqGlobalTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*                Note this function check the status in all availeble tiles (The status should be the SAME)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE, enable transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS   prvCpssFalconTxqGlobalTxEnableGet
(
    IN GT_U8    devNum,
    IN GT_BOOL  * enablePtr
);


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
    IN  CPSS_OS_FILE_TYPE_STC * file
);

/**
 * @internal prvCpssDxChTxqBindQueueToProfileSet function
 * @endinternal
 *
 * @brief Bind queue to queue  profile( Long queue and Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[in] profileIndex           - index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToProfileSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    IN GT_U32  profileIndex
);

/**
 * @internal prvCpssDxChTxqBindQueueToLengthAdjustProfileGet function
 * @endinternal
 *
 * @brief Get binded  queue  profile index( Lenght adjust)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] tc                      - traffic class (0..7)
 * @param[out] profileIndexPtr           - (pointer to)index of profile (0..15)
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
 */
GT_STATUS prvCpssDxChTxqBindQueueToLengthAdjustProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U8                tc,
    OUT GT_U32  * profileIndexPtr
);

/**
 * @internal prvCpssDxChTxqSetPortSdqThresholds function
 * @endinternal
 *
 * @brief Set port and queue thresholds (credit allocation requests to PDQ)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqSetPortSdqThresholds
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
   IN CPSS_PORT_SPEED_ENT speed
);



/**
 * @internal prvCpssDxChTxqBindPortQueuesToPdsProfile function
 * @endinternal
 *
 * @brief Bind port queues to PDS profile  (depend on port speed)
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                   - device number
 * @param[in] portNum           -physical port number
 * @param[in] speed                      - port speed
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */

GT_STATUS prvCpssDxChTxqBindPortQueuesToPdsProfile
(
   IN  GT_U8                                                  devNum,
   IN  GT_PHYSICAL_PORT_NUM                                   portNum,
   IN  CPSS_PORT_SPEED_ENT                                    speed,
   IN  PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *adjustAttributesPtr
);

/**
 * @internal prvCpssDxChTxqPfccTableInit function
 * @endinternal
 *
 * @brief The function initialize PFCC CFG table and Global_pfcc_CFG register
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */
GT_STATUS prvCpssDxChTxqPfccTableInit
(
    IN GT_U8 devNum
);

/**
 * @internal prvCpssDxChTxqQfcUnitsInit function
 * @endinternal
 *
 * @brief Enable local PFC generation at all QFCs.PFC generation will be controlled at PFCC
 *
 * @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2;
 *
 * @param[in] devNum                  - device number
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 */
GT_STATUS prvCpssDxChTxqQfcUnitsInit
(
    IN GT_U8 devNum
);


/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function read from PDX pizza and configure PFCC table according to number of lanes consumed by port
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                 data path[0..7]
* @param[in] localPort                         local port number[0..8]
* @param[in] tcBmp                           traffic class that is set for flow control,0xFF mean flow control on port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableSyncSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  tcBmp
);
/**
* @internal prvCpssFalconTxqPffcTableSyncSet function
* @endinternal
*
* @brief   This function  PFCC table according to given configuration of consumed lanes
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                data path[0..7]
* @param[in] localPort                        local port number[0..8]
* @param[in] portPizzaSlices-           desired PDX pizza configuration
* @param[in] tcBmp                            traffic class that is set for flow control,0xFF mean flow control on port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPffcTableExplicitSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    IN  GT_U32                  portPizzaSlices,
    IN  GT_U32                  tcBmp
);

/**
* @internal prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber function
* @endinternal
*
* @brief   Find dp index/local port of physical port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] physPort                 - physical port number
*
* @param[out] tileNumPtr               - Number of the tile (APPLICABLE RANGES:0..3).
* @param[out] dpNumPtr                 - Number of the dp (APPLICABLE RANGES:0..7).
* @param[out] localPortNumPtr - Number of the local port (APPLICABLE RANGES:0..8).
* @param[out] mappingTypePtr           - mapping type
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong tile number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber
(
    IN  GT_U8 devNum,
    IN  GT_PHYSICAL_PORT_NUM physPort,
    OUT GT_U32      * tileNumPtr,
    OUT GT_U32      * dpNumPtr,
    OUT GT_U32      * localdpPortNumPtr,
    OUT CPSS_DXCH_PORT_MAPPING_TYPE_ENT * mappingTypePtr
);



/**
* @internal prvCpssFalconTxqUtilsQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] queueNumber                                              local queue offset [0..15]
* @param[in] tcForPfcResponse                       -      Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsQueueTcSet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queueNumber,
    IN GT_U32                  tc
);
/**
* @internal prvCpssFalconTxqUtilsQueueTcGet function
* @endinternal
*
* @brief   Gets PFC TC to  queue map.Meaning this table define which Q should be paused on reception of
*               perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] queueNumber           local queue offset [0..15]
* @param[in] tcPtr                      -        (pointer to)Traffic class[0..15]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsQueueTcGet
(
    IN GT_U8                   devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    IN GT_U32                  queueNumber,
    IN GT_U32                  *tcPtr
);

/**
* @internal prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed function
* @endinternal
*
* @brief   Falcon : convert the local DMA number in the Local DataPath (DP index in tile),
*         to global DMA number in the device.
*         the global DMA number must be registered in 'port mapping' at this stage
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The PP's device number.
* @param[in] tileId                   - The tile Id
* @param[in] localDpIndexInTile       - the Data Path (DP) Index (local DP in the tile !!!)
* @param[in] localDmaNumInDp          - the DMA local number (local DMA in the DP !!!)
*
* @param[out] globalDmaNumPtr          - (pointer to) the DMA global number in the device.
*                                       GT_OK on success
*/
GT_STATUS prvCpssSip6TxqUtilsTileLocalDpLocalDmaNumToGlobalDmaNumInTileConvertWithMuxed
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileId,
    IN  GT_U32  localDpIndexInTile,
    IN  GT_U32  localDmaNumInDp,
    OUT GT_U32  *globalDmaNumPtr
);



/**
* @internal prvCpssFalconTxqUtilsPortEnableSet function
* @endinternal
*
* @brief  Set port to enable in SDQ.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[in] enable                                                             Enable/disable "credit ignore" mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPortEnableSet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_BOOL                              enable
);

GT_STATUS prvCpssFalconTxqUtilsPortEnableGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_BOOL * enablePtr
);

/**
* @internal prvCpssFalconTxqUtilsPortEnableSet function
* @endinternal
*
* @brief  Flush queues of remote ports.Wait until the queues are empty.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsRemotePortFlush
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum
);


/**
* @internal prvCpssFalconTxqUtilsUburstEventInfoGet function
* @endinternal
*
* @brief  Scan  all available DP for micro burst event
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[out] uBurstDataPtr                   -         `               (pointer to)micro  burst event
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE - no more new events found
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsUburstEventInfoGet
(
    IN    GT_U8                                devNum,
    INOUT GT_U32                               *uBurstEventsSizePtr,
    OUT   CPSS_DXCH_UBURST_INFO_STC            *uBurstDataPtr
);
/**
* @internal prvCpssFalconTxqUtilsUburstEnableSet
* @endinternal
*
* @brief   Enable/disable micro burst event generation per port/queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   local  queue offset (APPLICABLE RANGES:0..15).
* @param[in] enable                                             Enable/disable micro burst feature
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsUburstEnableSet
(
   IN  GT_U8                    devNum,
   IN  GT_PHYSICAL_PORT_NUM     physicalPortNumber,
   IN  GT_U32                   queueOffset,
   IN  GT_BOOL                  enable
);

/**
* @internal prvCpssFalconTxqUtilsUburstEnableGet
* @endinternal
*
* @brief   Get enable/disable configuration micro burst event generation per port/queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   local  queue offset (APPLICABLE RANGES:0..15).
* @param[out] enable                                             Enable/disable micro burst feature
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsUburstEnableGet
(
   IN  GT_U8                    devNum,
   IN  GT_PHYSICAL_PORT_NUM     physicalPortNumber,
   IN  GT_U32                   queueOffset,
   OUT GT_BOOL                 *enablePtr
);
/**
* @internal prvCpssFalconTxqUtilsUburstProfileBindSet
* @endinternal
*
* @brief  Set queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                       - physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   port queue offset (APPLICABLE RANGES:0..15).
* @param[in] profileNum                                      profile number[0..31]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsUburstProfileBindSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               physicalPortNumber,
    IN  GT_U32                             queueOffset,
    IN  GT_U32                             profileNum
);

/**
* @internal prvCpssFalconTxqUtilsUburstProfileBindGet
* @endinternal
*
* @brief  Set queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                       - physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   port queue offset (APPLICABLE RANGES:0..15).
* @param[out] profileNum                                   (pointer to)profile number[0..31]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsUburstProfileBindGet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               physicalPortNumber,
    IN  GT_U32                             queueOffset,
    OUT GT_U32                             *profileNumPtr
);

/**
* @internal prvCpssFalconTxqUtilsHeadroomMonitorSwDbSet
* @endinternal
*
* @brief  Set monitored (for headroom peak conters)  tile and DP to software DB.
*            This is used in order to query from correct tile and dp when getting peak counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          - physical device number
* @param[in] tileNum                  -          current tile id(APPLICABLE RANGES:0..3).
* @param[in] localDpNum                   -  DP (APPLICABLE RANGES:0..7).

* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS  prvCpssFalconTxqUtilsHeadroomMonitorSwDbSet
(
    IN  GT_U8                              devNum,
    IN  GT_U32                             tileNum,
    IN  GT_U32                             localDpNum
);

/**
* @internal prvCpssFalconTxqUtilsHeadroomMonitorSwDbGet
* @endinternal
*
* @brief  Get monitored (for headroom peak conters)  tile and DP to software DB.
*            This is used in order to query from correct tile and dp when getting peak counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          -       physical device number
* @param[in] tileNumPtr                  -          (pointer to)current tile id(APPLICABLE RANGES:0..3).
* @param[in] localDpNumPtr                   -  (pointer to)DP (APPLICABLE RANGES:0..7).

* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND -             in case no tile/DP is assigned for monitoring
*
*
*/
GT_STATUS  prvCpssFalconTxqUtilsHeadroomMonitorSwDbGet
(
    IN  GT_U8                               devNum,
    OUT  GT_U32                             *tileNumPtr,
    OUT  GT_U32                             *localDpNumPtr
);

/**
* @internal prvCpssFalconTxqUtilsHeadroomThresholdSet
* @endinternal
*
* @brief  Set threshold for headroom size interrupt.Once one  or more of the Port. TC has
*   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
*   to the HR crossed Threshold Status.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] threshold                   Headroom threshold
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsHeadroomThresholdSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  GT_U32                                      threshold
);

/**
* @internal prvCpssFalconTxqUtilsHeadroomThresholdGet
* @endinternal
*
* @brief  Set threshold for headroom size interrupt.Once one  or more of the Port. TC has
*   crossed the configured Threshold, an interrupt is asserted and the Status of the Port. TC is written
*   to the HR crossed Threshold Status.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[out] thresholdPtr                   (pointer to)Headroom threshold
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsHeadroomThresholdGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  GT_U32                                      *thresholdPtr
);

/**
* @internal prvCpssFalconTxqUtilsLowLatencySet  function
* @endinternal
*
* @brief   Enable/disble low latency mode on queue.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] physicalPortNum                  - physical or CPU port number
* @param[in] queueOffset                  - traffic class queue on this device (0..7)
* @param[in] enable                   - GT_TRUE,  enable queue  low latency mode,
*                                                         GT_FALSE, disable queue  low latency mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
*                                                contain latency queue.
*/
GT_STATUS prvCpssFalconTxqUtilsLowLatencySet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        physicalPortNum,
    IN  GT_U32                                      queueOffset,
    IN  GT_BOOL                                     enable
);

/**
**
* @internal prvCpssFalconTxqUtilsLowLatencyGet  function
* @endinternal
*
* @brief   Get enable/disble low latency mode on queue.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] physicalPortNum                  - physical or CPU port number
* @param[in] queueOffset                  - traffic class queue on this device (0..7)
* @param[out] enablePtr                   -(pointer to) GT_TRUE,  enable queue  low latency mode,
*                                                         GT_FALSE, disable queue  low latency mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
*                                                contain latency queue.
*/
GT_STATUS prvCpssFalconTxqUtilsLowLatencyGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        physicalPortNum,
    IN  GT_U32                                      queueOffset,
    OUT  GT_BOOL                                     *enablePtr
);
/**
* @internal prvCpssFalconTxqUtilsInitPdsProfilesDb function
* @endinternal
*
* @brief   Initialize SW db of PDS profiles.For SIP 6.10 also write the default profile to HW and bind all the ports.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -PP's device number.
* @param[in] portMappedToDefault  - number of ports that should be mapped to default
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssFalconTxqUtilsInitPdsProfilesDb
(
    IN GT_U8 devNum,
    IN GT_U32 portMappedToDefault
);
/**
* @internal prvCpssFalconTxqUtilsGlobalFlowControlEnableGet  function
* @endinternal
*
* @brief   Sets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] bcValue                  - number of bytes added to the length of every frame
*                                      for shaping/scheduling rate calculation.
*                                      (APPLICABLE RANGES:  0..63)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
*                                                contain latency queue.
*/
GT_STATUS prvCpssFalconTxqPdsByteCountChangeValueSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_32     bcValue
);

/**
* @internal prvCpssFalconTxqPdsByteCountChangeValueGet function
* @endinternal
*
* @brief   Gets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] bcValuePtr               - (pointer to) number of bytes added to the length of
*                                      every frame for shaping/scheduling rate calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqPdsByteCountChangeValueGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    OUT  GT_32                    *bcValue
);


/**
 * @internal prvCpssDxChTxqBindPortQueuesToLengthAdjustProfile function
 * @endinternal
 *
 * @brief Bind port queues to length adjust profile
 *
 * @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Bobcat3; AC3X;Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman;
 *
 * @param[in] devNum                                            -device number
 * @param[in] portNum                                          -physical port number
 * @param[in] adjustAttributesPtr                      -(pointer to)length adjust attributes
 *
 * @retval GT_OK                       - on success
 * @retval GT_BAD_PTR                  - on NULL pointer
 * @retval GT_BAD_PARAM                - wrong value in any of the parameters
 * @retval GT_HW_ERROR                 - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*
 * @note   NOTE:      Remote ports are not handled.The configuration for remote ports should be done while configuring cascade port.
 */
GT_STATUS prvCpssDxChTxqBindPortQueuesToLengthAdjustProfile
(
   IN  GT_U8                                                  devNum,
   IN  GT_PHYSICAL_PORT_NUM                                   portNum,
   IN  PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LENGTH_ADJUST_STC *adjustAttributesPtr
);

GT_STATUS prvCpssFalconTxqUtilsUpdateSelectListParamsSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      tileNum,
    IN  GT_U32                                      dpNum,
    IN  GT_U32                                      localPortNum,
    IN  GT_U32                                      firstQ,
    IN  GT_U32                                      lastQ,
    IN  GT_BOOL                                     hiPrioExist
);
/**
* @internal prvCpssFalconTxqUtilsPortQueueOffsetValidGet function
* @endinternal
*
* @brief  This function check that queue offset is within port range
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] offset                  -       traffic class queue on this device (0..15)
* @param[out] validPtr                  -  equals GT_TRUE in case the offset is within alowed range,GT_FALSE otherwise
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsPortQueueOffsetValidGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   offset,
    OUT GT_BOOL                  *validPtr
);


/**
* @internal prvCpssFalconTxqUtilsPhysicalPortNumberGet function
* @endinternal
*
* @brief  Find mapped physical port by tile/dp/local port
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[in] tileNum                  -  Number of the tile (APPLICABLE RANGES:0..4).
* @param[in] dpNum                    - Number of the dp (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - Number of the local port (APPLICABLE RANGES:0..8).
* @param[out] portNumPtr             -Physical port number.
* @retval GT_OK                    -            on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               -    on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsPhysicalPortNumberGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  dp,
    IN GT_U32  localPort,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
);

/**
* @internal prvCpssFalconTxqUtilsPhysicalPortNumberGet function
* @endinternal
*
* @brief  Find  physical port that triggered headroom crossed interrupt by info
*       delivered by event
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[in] tileNum                  -  Number of the tile (APPLICABLE RANGES:0..4).
* @param[in] pipe                     -       Number of control pipe (APPLICABLE RANGES:0..1).
* @param[in] dpNum                    - Local (per pipe) Number of the dp (APPLICABLE RANGES:0..3).
* @param[in]reg                              Register index (APPLICABLE RANGES:0..3).
* @param[in] port             -             Port index in register(APPLICABLE RANGES:0..2).
* @param[out] portNumPtr             -Physical port number.
* @retval GT_OK                    -            on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               -    on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsHeadroomTriggerPortGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  pipe,
    IN GT_U32  dp,
    IN GT_U32  reg,
    IN GT_U32  port,
    OUT GT_PHYSICAL_PORT_NUM *portNumPtr
);


/**
* @internal prvCpssTxqUtilsTreeIsReadyGet function
* @endinternal
*
* @brief  Gheck if scheduling tree is build for all device
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[out] readyPtr             -equals GT_TRUE is scheduling tree is ready,GT_FALSE otherwise
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssTxqUtilsTreeIsReadyGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *readyPtr
);


/**
* @internal prvCpssFalconTxqUtilsMappedQueueNumGet function
* @endinternal
*
* @brief  Get number of mapped queues per tile
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -  physical device number
* @param[in] tileNum                   -  tile number
* @param[out] numOfMappedPortsPtr             number of mapped ports
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsMappedQueueNumGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               tileNum,
    OUT GT_U32                               *numOfMappedPtr
);


/**
* @internal prvCpssFalconTxqUtilsPortGlobalUniqueIndexGet function
* @endinternal
*
* @brief  Get global index of the first queue mapped to physical port
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                  physical device number
* @param[in] physicalPortNum                   -    physical port number
* @param[out] globalUniqueIndexPtr             global index of the first queue mapped to physical port
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsPortGlobalUniqueIndexGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     physicalPortNum,
    OUT GT_U32                   *globalUniqueIndexPtr
);


/**
 * @internal prvCpssSip6TxqLibInitDoneSet function
 * @endinternal
 *
 * @brief  Set txQ library init finished
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[in] initDone                                          GT_TRUE if txQ library init is done,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssSip6TxqLibInitDoneSet
(
    IN GT_U8 devNum,
    IN GT_BOOL initDone
);
/**
 * @internal prvCpssSip6TxqLibInitDoneSet function
 * @endinternal
 *
 * @brief  Set txQ library init finished
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[out] initDonePtr                                 (pointer to)GT_TRUE if txQ library init is done,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssSip6TxqLibInitDoneGet
(
    IN GT_U8 devNum,
    OUT GT_BOOL *initDonePtr
);

GT_STATUS prvCpssSip6TxQUtilsOptimizedThroughputEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       tcQueue,
    IN  GT_BOOL                     enable
);

GT_STATUS prvCpssSip6TxQUtilsOptimizedThroughputEnableGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8                       tcQueue,
    IN  GT_BOOL                     *enablePtr
);

GT_STATUS prvCpssSip6TxqUtilsDataPathEvent
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM physicalPortNum,
    IN PRV_CPSS_PDQ_A_NODE_DATA_PATH_EVENT_ENT event,
    IN GT_BOOL              preemtedChannelEvent
);


GT_STATUS prvCpssSip6TxqUtilsPnodeIndexGet
(
    GT_U32 devNum,
    GT_U32 dpNum,
    GT_U32 localdpPortNum,
    GT_U32 *pNodeIndPtr
);



PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * prvCpssSip6TxqUtilsPnodeInstanceGet
(
    GT_U32 devNum,
    GT_U32 tileNum,
    GT_U32 index
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxq */


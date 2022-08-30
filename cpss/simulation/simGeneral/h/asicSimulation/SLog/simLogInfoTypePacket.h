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
* @file simLogInfoTypePacket.h
*
* @brief simulation logger packet info type functions
*
* @version   9
********************************************************************************
*/
#ifndef __simLogPacket_h__
#define __simLogPacket_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smain/smain.h>
#include <os/simTypes.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>

/* save packet descriptor */
#define SIM_LOG_PACKET_DESCR_SAVE                                   \
    {                                                               \
        SKERNEL_FRAME_CHEETAH_DESCR_STC *__descrPtrSavedPtr=NULL;   \
        SKERNEL_FRAME_CHEETAH_DESCR_STC __descrPtrSaved;            \
        if(simLogIsOpenFlag)                                          \
        {                                                           \
            __descrPtrSaved = *descrPtr;                            \
            __descrPtrSavedPtr = &__descrPtrSaved;                  \
        }

/* log packet descriptor changes -
   (to be called to NOT terminate scope of SIM_LOG_PACKET_DESCR_SAVE) */
#define SIM_LOG_PACKET_DESCR_COMPARE_AND_KEEP_FUNCTION(func)                 \
        if(__descrPtrSavedPtr)                                  \
        {                                                       \
            simLogPacketDescrCompare(devObjPtr, __descrPtrSavedPtr, descrPtr, func);   \
        }

/* terminate scope of 'SIM_LOG_PACKET_DESCR_SAVE') */
#define SIM_LOG_PACKET_DESCR_COMPARE_END                 \
    }

/* log packet descriptor changes -
    (to be called when need terminate scope of 'SIM_LOG_PACKET_DESCR_SAVE') */
#define SIM_LOG_PACKET_DESCR_COMPARE(func)                      \
        SIM_LOG_PACKET_DESCR_COMPARE_AND_KEEP_FUNCTION(func)    \
        SIM_LOG_PACKET_DESCR_COMPARE_END



/* declare ports BMP */
#define SIM_LOG_TARGET_BMP_PORTS_DECLARE(portsBmpNamePtr)           \
    /* unique name for the pointer */                           \
    SKERNEL_PORTS_BMP_STC *__saved_obj_ptr_##portsBmpNamePtr=NULL; \
    /* unique name for the object */                            \
    SKERNEL_PORTS_BMP_STC __saved_obj_##portsBmpNamePtr

/* save ports BMP */
#define SIM_LOG_TARGET_BMP_PORTS_SAVE(portsBmpNamePtr)              \
    if(simLogIsOpenFlag)                                          \
    {                                                           \
        __saved_obj_##portsBmpNamePtr = *portsBmpNamePtr;        \
        __saved_obj_ptr_##portsBmpNamePtr = &__saved_obj_##portsBmpNamePtr;\
    }

/* log packet descriptor changes */
#define SIM_LOG_TARGET_BMP_PORTS_COMPARE(portsBmpNamePtr)       \
    if(__saved_obj_ptr_##portsBmpNamePtr)                        \
    {                                                           \
        simLogPortsBmpCompare(devObjPtr,                        \
            __FILE__,                                           \
            functionNameString,/*the name of the calling function*/ \
            #portsBmpNamePtr,                                   \
            __saved_obj_ptr_##portsBmpNamePtr->ports,/*old*/     \
            portsBmpNamePtr->ports);/*new*/                     \
    }




/* declare ports BMP */
#define SIM_LOG_TARGET_ARR_PORTS_DECLARE(portsArrNamePtr)           \
    /* unique name for the pointer */                           \
    SKERNEL_PORTS_BMP_STC *__saved_obj_ptr_##portsArrNamePtr=NULL; \
    /* unique name for the object */                            \
    SKERNEL_PORTS_BMP_STC __saved_obj_##portsArrNamePtr

/* save ports BMP */
#define SIM_LOG_TARGET_ARR_PORTS_SAVE(portsArrNamePtr)          \
    if(simLogIsOpenFlag)                                          \
    {                                                           \
        memcpy(__saved_obj_##portsArrNamePtr.ports,portsArrNamePtr,sizeof(SKERNEL_PORTS_BMP_STC));        \
        __saved_obj_ptr_##portsArrNamePtr = &__saved_obj_##portsArrNamePtr;\
    }

/* log packet descriptor changes */
#define SIM_LOG_TARGET_ARR_PORTS_COMPARE(portsArrNamePtr)       \
    if(__saved_obj_ptr_##portsArrNamePtr)                        \
    {                                                           \
        simLogPortsBmpCompare(devObjPtr,                        \
            __FILE__,                                           \
            functionNameString,/*the name of the calling function*/ \
            #portsArrNamePtr,                                   \
            __saved_obj_ptr_##portsArrNamePtr->ports,/*old*/     \
            portsArrNamePtr);/*new*/                            \
    }


/* declare Array of ports */
#define SIM_LOG_TARGET_ARR_DECLARE(arrNamePtr)                  \
    /* unique name for the pointer */                           \
    SKERNEL_PORTS_BMP_STC *__saved_obj_ptr_##arrNamePtr=NULL;   \
    /* unique name for the object */                            \
    SKERNEL_PORTS_BMP_STC __saved_obj_##arrNamePtr

/* save Array of ports */
#define SIM_LOG_TARGET_ARR_SAVE(arrNamePtr)                            \
    if(simLogIsOpenFlag)                                                 \
    {                                                                  \
        /* loop on the array */                                        \
        GT_U32  _ii;                                                   \
        for(_ii = 0; _ii < SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS; _ii++) \
        {                                                              \
            if(arrNamePtr[_ii])                                        \
            {                                                          \
                /* the port is in the array --> set it to the BMP */   \
                SKERNEL_PORTS_BMP_ADD_PORT_MAC((&__saved_obj_##arrNamePtr),_ii);\
            }                                                          \
            else                                                        \
            {                                                           \
                /* the port is in NOT in the the array --> unset it to the BMP */    \
                SKERNEL_PORTS_BMP_DEL_PORT_MAC((&__saved_obj_##arrNamePtr),_ii);         \
            }                                                           \
        }                                                              \
        __saved_obj_ptr_##arrNamePtr = &__saved_obj_##arrNamePtr;      \
    }

/* log packet descriptor changes */
#define SIM_LOG_TARGET_ARR_COMPARE(arrNamePtr)                          \
    if(__saved_obj_ptr_##arrNamePtr)                                    \
    {                                                                   \
        SKERNEL_PORTS_BMP_STC _newBmp;                                  \
        /* loop on the array */                                         \
        GT_U32  _ii;                                                    \
        for(_ii = 0; _ii < SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS; _ii++)  \
        {                                                               \
            if(arrNamePtr[_ii])                                         \
            {                                                           \
                /* the port is in the array --> set it to the BMP */    \
                SKERNEL_PORTS_BMP_ADD_PORT_MAC((&_newBmp),_ii);         \
            }                                                           \
            else                                                        \
            {                                                           \
                /* the port is in NOT in the the array --> unset it to the BMP */    \
                SKERNEL_PORTS_BMP_DEL_PORT_MAC((&_newBmp),_ii);         \
            }                                                           \
        }                                                               \
        simLogPortsBmpCompare(devObjPtr,                                \
            __FILE__,                                                   \
            functionNameString,/*the name of the calling function*/     \
            #arrNamePtr,                                                \
            __saved_obj_ptr_##arrNamePtr->ports,/*old*/                 \
            _newBmp.ports);/*new*/                                      \
    }


/* SOHO : save packet descriptor */
#define SIM_SOHO_LOG_PACKET_DESCR_SAVE                                   \
    {                                                               \
        SKERNEL_FRAME_SOHO_DESCR_STC *__descrPtrSavedPtr=NULL;   \
        SKERNEL_FRAME_SOHO_DESCR_STC __descrPtrSaved;            \
        if(simLogIsOpenFlag)                                          \
        {                                                           \
            __descrPtrSaved = *descrPtr;                            \
            __descrPtrSavedPtr = &__descrPtrSaved;                  \
        }

/* SOHO : log packet descriptor changes -
   (to be called to NOT terminate scope of SIM_SOHO_LOG_PACKET_DESCR_SAVE) */
#define SIM_SOHO_LOG_PACKET_DESCR_COMPARE_AND_KEEP_FUNCTION(func)                 \
        if(__descrPtrSavedPtr)                                  \
        {                                                       \
            simLogSohoPacketDescrCompare(devObjPtr, __descrPtrSavedPtr, descrPtr, func);   \
        }

/* SOHO : terminate scope of 'SIM_SOHO_LOG_PACKET_DESCR_SAVE') */
#define SIM_SOHO_LOG_PACKET_DESCR_COMPARE_END                 \
    }

/* SOHO : log packet descriptor changes -
    (to be called when need terminate scope of 'SIM_SOHO_LOG_PACKET_DESCR_SAVE') */
#define SIM_SOHO_LOG_PACKET_DESCR_COMPARE(func)                      \
        SIM_SOHO_LOG_PACKET_DESCR_COMPARE_AND_KEEP_FUNCTION(func)    \
        SIM_SOHO_LOG_PACKET_DESCR_COMPARE_END


/* PIPE : save packet descriptor */
#define SIM_PIPE_LOG_PACKET_DESCR_SAVE                               \
    {                                                                \
        SKERNEL_FRAME_PIPE_DESCR_STC *pipe__descrPtrSavedPtr=NULL;   \
        SKERNEL_FRAME_PIPE_DESCR_STC pipe__descrPtrSaved;            \
        SKERNEL_FRAME_CHEETAH_DESCR_STC *cht__descrPtrSavedPtr=NULL; \
        SKERNEL_FRAME_CHEETAH_DESCR_STC cht__descrPtrSaved;          \
        if(simLogIsOpenFlag)                                         \
        {                                                            \
            pipe__descrPtrSaved = *pipe_descrPtr;                    \
            pipe__descrPtrSavedPtr = &pipe__descrPtrSaved;           \
            cht__descrPtrSaved = *(pipe_descrPtr->cheetah_descrPtr); \
            cht__descrPtrSavedPtr = &cht__descrPtrSaved;             \
        }

/* PIPE : log packet descriptor changes -
   (to be called to NOT terminate scope of SIM_PIPE_LOG_PACKET_DESCR_SAVE) */
#define SIM_PIPE_LOG_PACKET_DESCR_COMPARE_AND_KEEP_FUNCTION(func)                 \
        if(pipe__descrPtrSavedPtr)                              \
        {                                                       \
            simLogPipePacketDescrCompare(devObjPtr, pipe__descrPtrSavedPtr, cht__descrPtrSavedPtr, pipe_descrPtr, func);   \
        }

/* PIPE : terminate scope of 'SIM_SOHO_LOG_PACKET_DESCR_SAVE') */
#define SIM_PIPE_LOG_PACKET_DESCR_COMPARE_END                 \
    }

/* PIPE : log packet descriptor changes -
    (to be called when need terminate scope of 'SIM_PIPE_LOG_PACKET_DESCR_SAVE') */
#define SIM_PIPE_LOG_PACKET_DESCR_COMPARE(func)                      \
        SIM_PIPE_LOG_PACKET_DESCR_COMPARE_AND_KEEP_FUNCTION(func)    \
        SIM_PIPE_LOG_PACKET_DESCR_COMPARE_END


/**
* @internal simLogPacketDescrCompare function
* @endinternal
*
* @brief   log changes between saved packet descriptor and given
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] old                      -  packet descriptor pointer
* @param[in] new                      -  packet descriptor pointer
* @param[in] funcName                 - pointer to function name
*                                       None.
*/
GT_VOID simLogPacketDescrCompare
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *old,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *new,
    IN GT_CHAR                         *funcName
);
/**
* @internal simLogSohoPacketDescrCompare function
* @endinternal
*
* @brief   log changes between saved packet descriptor and given
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] old                      -  packet descriptor pointer
* @param[in] new                      -  packet descriptor pointer
* @param[in] funcName                 - pointer to function name
*                                       None.
*/
GT_VOID simLogSohoPacketDescrCompare
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC *old,
    IN SKERNEL_FRAME_SOHO_DESCR_STC *new,
    IN GT_CHAR                         *funcName
);
/**
* @internal simLogPipePacketDescrCompare function
* @endinternal
*
* @brief   PIPE : log changes between saved packet descriptor and given
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] old                      -  packet descriptor pointer
* @param[in] old_cheeath              - the 'cheetah' part of the 'old'
* @param[in] new                      -  packet descriptor pointer
* @param[in] funcName                 - pointer to function name
*                                       None.
*/
GT_VOID simLogPipePacketDescrCompare
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC     *old,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *old_cheeath,/* new is in 'new->cheetah_descrPtr'*/
    IN SKERNEL_FRAME_PIPE_DESCR_STC     *new,
    IN GT_CHAR                         *funcName
);

/**
* @internal simLogPacketDescrFrameDump function
* @endinternal
*
* @brief   log frame dump
*/
GT_VOID simLogPacketDescrFrameDump
(
    IN SKERNEL_DEVICE_OBJECT           const *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC const *descrPtr
);
/**
* @internal simLogSohoPacketDescrFrameDump function
* @endinternal
*
* @brief   log frame dump
*/
GT_VOID simLogSohoPacketDescrFrameDump
(
    IN SKERNEL_DEVICE_OBJECT           const *devObjPtr,
    IN SKERNEL_FRAME_SOHO_DESCR_STC const *descrPtr
);

/* print packetCmd enum value */
GT_VOID simLogPacketDescrPacketCmdDump
(
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT packetCmd
);
/* print cpu code enum value */
GT_VOID simLogPacketDescrCpuCodeDump
(
    IN SKERNEL_DEVICE_OBJECT const *devObjPtr,
    IN SNET_CHEETAH_CPU_CODE_ENT    cpuCode
);

/**
* @internal simLogPortsBmpCompare function
* @endinternal
*
* @brief   ports BMP changes between saved one (old) and given one (new)
*         dump to log the DIFF
* @param[in] devObjPtr                - pointer to device object.
* @param[in] fileNameString           - the name (string) of the calling file
* @param[in] functionNameString       - the name (string) of the calling function
* @param[in] variableNamePtr          - the name (string) of the variable
* @param[in] oldArr                   - old ports BMP pointer (pointer to the actual words)
* @param[in] newArr                   - new ports BMP pointer (pointer to the actual words)
*                                       None.
*/
GT_VOID simLogPortsBmpCompare
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_CHAR                         *fileNameString,
    IN GT_CHAR                         *functionNameString,
    IN GT_CHAR                         *variableNamePtr,
    IN GT_U32                           *oldArr,
    IN GT_U32                           *newArr
);


/**
* @internal simLogPacketDump function
* @endinternal
*
* @brief   log frame dump
*/
GT_VOID simLogPacketDump
(
    IN SKERNEL_DEVICE_OBJECT        const *devObjPtr,
    IN GT_BOOL                      ingressDirection,
    IN GT_U32                       portNum,
    IN GT_U8                        *startFramePtr,
    IN GT_U32                       byteCount
);

/**
* @internal simLogPacketFrameUnitSet function
* @endinternal
*
* @brief   Set the unit Id of current thread (own thread).
*
* @param[in] frameUnit                -  the unit id
*                                       None.
*/
GT_VOID simLogPacketFrameUnitSet(
    IN SIM_LOG_FRAME_UNIT_ENT frameUnit
);

/**
* @internal simLogPacketFrameCommandSet function
* @endinternal
*
* @brief   Set the command of current thread (own thread).
*
* @param[in] frameCommand             -  the command id
*                                       None.
*/
GT_VOID simLogPacketFrameCommandSet(
    IN SIM_LOG_FRAME_COMMAND_TYPE_ENT frameCommand
);

/*******************************************************************************
* simLogPacketFrameMyLogInfoGet
*
* DESCRIPTION:
*       Get the pointer to the log info of current thread (own thread)
*
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the log info of current thread (own thread)
*       (may be NULL)
*
* COMMENTS:
*
*
*******************************************************************************/
SIM_LOG_FRAME_INFO_STC * simLogPacketFrameMyLogInfoGet(
    GT_VOID
);

/**
* @internal simLogPrintBitmapArr function
* @endinternal
*
* @brief   print a bitmap array .
*
* @param[in] devObjPtr                - device object
* @param[in] fileNameString           - the name (string) of the calling file
* @param[in] functionNameString       - the name (string) of the calling function
* @param[in] variableNamePtr          - prefix for the name (string) of the variable
* @param[in] variableNamePtr          - the name (string) of the variable
* @param[in] bitmapArr                - pointer to bitmap array
* @param[in] numOfBits                - number of bits in bitmapArr (set or unset)
*                                       number of ports in the bmp
*
* @note based on printBitmapArr(...) from cpss
*
*/
GT_U32 simLogPrintBitmapArr
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_CHAR                         *fileNameString,
    IN GT_CHAR                         *functionNameString,
    IN GT_CHAR                         *prefix_variableNamePtr,
    IN GT_CHAR                         *variableNamePtr,
    IN GT_U32                          *bitmapArr,
    IN GT_U32                          numOfBits
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __simLogPacket_h__ */



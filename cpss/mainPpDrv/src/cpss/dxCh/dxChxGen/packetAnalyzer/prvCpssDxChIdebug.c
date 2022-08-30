/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChIdebug.c
*
* @brief iDebug APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebugAuto.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**************************** Register adress macros*****************************/

#define PRV_DFX_XSB_MATCH_SAMPLE_REG_OFFSET_MAC(_devNum)    (PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)->\
                                                                     DFX_XSB.Match_Sample)
#define PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(_devNum)        (PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)->\
                                                                     DFX_XSB.General_Configurations)
#define PRV_DFX_XSB_INT_MASK_REG_OFFSET_MAC(_devNum)        (PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)->\
                                                                     DFX_XSB.iDebug_Interrupt_Mask)
#define PRV_DFX_XSB_MATCH_MASK_REG_OFFSET_MAC(_devNum)       (PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)->\
                                                                    DFX_XSB.Match_Mask)
#define PRV_DFX_XSB_MATCH_COUNTER_REG_OFFSET_MAC(_devNum)   (PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)->\
                                                                     DFX_XSB.Match_Counter)
#define PRV_DFX_XSB_MATCH_KEY_REG_OFFSET_MAC(_devNum)       (PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(_devNum)->\
                                                                     DFX_XSB.Match_Key)

/* max interface length in words */
#define CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS 50
#define CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS 48
#define CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS (CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS*2)


#define PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS      0
#define PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS       1


#define PRV_CPSS_DXCH_IDEBUG_CP_0_TOKEN_CNS                    "tile0_pipe0"
#define PRV_CPSS_DXCH_IDEBUG_CP_1_TOKEN_CNS                    "tile0_pipe1"

#define PRV_CPSS_DXCH_IDEBUG_DP_0_TOKEN_CNS                    "_i0_"
#define PRV_CPSS_DXCH_IDEBUG_DP_1_TOKEN_CNS                    "_i1_"
#define PRV_CPSS_DXCH_IDEBUG_DP_2_TOKEN_CNS                    "_i2_"
#define PRV_CPSS_DXCH_IDEBUG_DP_3_TOKEN_CNS                    "_i3_"

#define PRV_CPSS_DXCH_IDEBUG_IA_0_TOKEN_CNS                    "_ia_0"
#define PRV_CPSS_DXCH_IDEBUG_IA_1_TOKEN_CNS                    "_ia_1"

#define PRV_CPSS_DXCH_IDEBUG_DP_IA_0_TOKEN_CNS                    "_desc0"
#define PRV_CPSS_DXCH_IDEBUG_DP_IA_1_TOKEN_CNS                    "_desc1"
#define PRV_CPSS_DXCH_IDEBUG_DP_IA_2_TOKEN_CNS                    "_desc2"
#define PRV_CPSS_DXCH_IDEBUG_DP_IA_3_TOKEN_CNS                    "_desc3"

#define PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_0_TOKEN_CNS                    "pipe0_2"
#define PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_1_TOKEN_CNS                    "pipe1_2"

/*macro for checking validity of tile  number*/
#define PRV_IDEBUG_CHECK_TILE_NUM_MAC(_devNum,_tileNum)\
    do\
      {\
        GT_U32 numOfTilesInMacro;\
        if(PRV_CPSS_SIP_6_CHECK_MAC(_devNum))\
        {\
            switch(PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles)\
            {\
                case 0:\
                case 1:\
                    numOfTilesInMacro = 1;\
                    break;\
               case 2:\
               case 4:\
                    numOfTilesInMacro =PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles;\
                    break;\
                default:\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles);\
                    break;\
            }\
            if(_tileNum>=numOfTilesInMacro)\
            {\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "tile number [%d] must be less than [%d]",_tileNum,numOfTilesInMacro);\
            }\
         }\
      }while(0);

#define PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(_devNum,_portGroupsBmp,_tileInd)\
    do{\
            if(PRV_CPSS_SIP_6_CHECK_MAC(_devNum))\
            {\
                switch(_portGroupsBmp)\
                {\
                    case 0x1/*tile 0*/:\
                        _tileInd=0;\
                        break;\
                    case 0x4/*tile 1*/:\
                        _tileInd=1;\
                        break;\
                    case 0x10/*tile 2*/:\
                         _tileInd=2;\
                         break;\
                    case 0x40/*tile 3*/:\
                         _tileInd=3;\
                         break;\
                    case CPSS_PORT_GROUP_UNAWARE_MODE_CNS/*all tiles*/:\
                        _tileInd=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;\
                        break;\
                    default:\
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not valid bmp 0x%x", _portGroupsBmp);\
                        break;\
                 }\
                 if(_portGroupsBmp!=CPSS_PORT_GROUP_UNAWARE_MODE_CNS)\
                 {\
                    PRV_IDEBUG_CHECK_TILE_NUM_MAC(_devNum,_tileInd)\
                 }\
            }\
            else\
            {\
                PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(_devNum,_portGroupsBmp)\
            }\
       }\
    while(0);

/**************************** Private macro definitions *************************/

#define LENGTH_TO_NUM_WORDS_MAC(_len)     ((_len+31) >> 5)
#define LENGTH_TO_NUM_BITS_MAC(_len)      ((_len) < 32 ? (_len + 1) : 32)


/****************************** Enum definitions ********************************/

static const char* idebugStageStrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1] = {
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E"
};

#define PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.idebugDir.idebugSrc.idebugDB._var)

#define PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR()\
        &(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.idebugDir.idebugSrc.idebugDB)) ;

/* check that DFX server is not in power save mode */
#define PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC() \
    if (PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_VAR_GET(powerSaveEnable) == GT_TRUE)           \
    {                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DFX server is in power save mode");\
    }

/**
* @internal prvCpssDxChIdebugMuxDbInit function
* @endinternal
*
* @brief   The function set default configuration to mux
*          priority data base
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
*
*/
static GT_STATUS prvCpssDxChIdebugMuxDbInit
(
    IN    GT_U8                                         devNum
);

static GT_STATUS prvCpssDxChIdebugPortGroupInterfaceUpperMatchEnableSet
(
    IN    GT_U8       devNum,
    IN    GT_U32      instanceDfxBaseAddr,
    IN    GT_BOOL     enable
);

/**
* @internal prvCpssDxChIdebugPipeDpGet function
* @endinternal
*
* @brief   The function parse control pipe/data path from
*          instance name.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;.
*
* @param[in] currentBusName            - interface name
* @param[out]pipe                      - (pointer to) control pipe index[0..1]
* @param[out]dataPath                  - (pointer to) data path index[0..3]
* @param[in] dpInfoPresentPtr          - (pointer to) dp info present
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS prvCpssDxChIdebugPipeDpGet
(
    IN  const GT_CHAR                           *currentBusName,
    OUT GT_U32                                  *pipePtr,
    OUT GT_U32                                  *dpPtr,
    OUT GT_BOOL                                 *dpInfoPresentPtr
);

/**
* @internal prvCpssDxChIdebugCompareMetaData function
* @endinternal
*
* @brief   The function compare two meta data fields
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2.
*
* @param[in] devNum                   - device number
*
* @retval GT_TRUE                     - meta data equal
* @retval GT_FALSE                    - meta data not equal
*
*/
static GT_BOOL prvCpssDxChIdebugCompareMetaData
(
    IN   GT_U32                                           metaDataSource,
    IN   GT_U32                                           metaDataTarget
);

/**
* @internal mxmlInstanceInfoSegmentIdGet function
* @endinternal
*
* @brief  Get segment index from interface name
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
*
* @param[in] devNum               - device number
* @param[in] interfaceName        - interface name
* @param[out] segmentIdPtr        - (pointer to)index of segment
* @param[out] muxIdPtr            - (pointer to)index of mux select
* @param[out] instancePtr         - (pointer to)instance mxml node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
static GT_STATUS mxmlInstanceInfoSegmentIdGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *segmentIdPtr,
    OUT GT_U32                                          *muxIdPtr,
    OUT mxml_node_t                                     **instancePtr
);

/**
* @internal mxmlInstanceInfoSegmentIdGet function
* @endinternal
*
* @brief  Get segment index from interface name
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
*
* @param[in] devNum                - device number
* @param[in] instancePtr           - (pointer to)instance mxml node
* @param[in] segmentId             - index of segment
* @param[in] muxId                 - index of mux select
* @param[in] sizePtr               - (pointer to)size of segment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
static GT_STATUS mxmlInstanceInfoSegmentSizeGet
(
    IN  GT_U8                                           devNum,
    IN mxml_node_t                                     *instancePtr,
    IN GT_U32                                           segmentId,
    IN GT_U32                                           muxId,
    OUT GT_U32                                         *sizePtr
);



CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT idebugStageStrConv(const char* str)
{
    GT_U32 i;

    for(i=0; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        if(cpssOsStrCmp(str,idebugStageStrArr[i]) == 0)
        {
            return (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)i;
        }
    }

    return CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
}

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck
*           function
* @endinternal
*
* @brief   Check that sampling on interface is not enabled.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck
(
    IN  GT_U8                                            devNum,
    IN  GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN  GT_CHAR_PTR                                      interfaceName
)
{
    GT_STATUS rc;
    GT_BOOL enable;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,&enable);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (enable == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: Can't change configuration while sampling is enabled for interface [%s]",interfaceName);
    }

    return GT_OK;
}

static GT_STATUS prvCpssDxChIdebugBoundedBusGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          multiplicationIndex,
    OUT GT_CHAR                                         *busPtr,
    OUT GT_U32                                          *muxPtr
)
{
    GT_U32    i;
    GT_STATUS rc;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC  *boundedInterfacesArr = NULL ;
    GT_U32 size;


    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);

    boundedInterfacesArr = (PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC)*
                                                                                              (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1));
    if (boundedInterfacesArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate boundedInterfacesArr");
    }

    rc =prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet(devNum,boundedInterfacesArr,&size);
    if(rc!=GT_OK)
    {
        if (boundedInterfacesArr != NULL)
            cpssOsFree(boundedInterfacesArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    for(i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E;i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E;i++)
    {
        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,i) == GT_FALSE)
            continue;

        if (cpssOsStrCmp(boundedInterfacesArr[i].info.interfaceId,interfaceName) == 0)
        {
            if(multiplicationIndex==0)
            {
                cpssOsStrCpy(busPtr,boundedInterfacesArr[i].info.instanceId);
                *muxPtr = boundedInterfacesArr[i].info.interfaceIndex;
            }
            else
            {
                if (boundedInterfacesArr != NULL)
                    cpssOsFree(boundedInterfacesArr);
                return prvCpssDxChIdebugMultipleInterfaceByIndexGet(devNum,interfaceName,multiplicationIndex,busPtr,muxPtr);
            }
            if (boundedInterfacesArr != NULL)
                cpssOsFree(boundedInterfacesArr);
            return GT_OK;
        }
    }

    if (boundedInterfacesArr != NULL)
        cpssOsFree(boundedInterfacesArr);
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

}

/**
* @internal mxmlFindInterfaceFirstFieldNode function
* @endinternal
*
* @brief   Get pointer to interface's first field.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval mxml_node_t                 - (pointer to) interface's first field.
*/
mxml_node_t * mxmlFindInterfaceFirstFieldNode
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName
)
{
    mxml_node_t *node,*head;
    const GT_CHAR *value;
    GT_STATUS rc;

    /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
        return NULL;

    value = interfaceName;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    node = prvCpssMxmlFindElement(head, head, "InterfacesList", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, NULL, "internalName", value, MXML_DESCEND_FIRST);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, "subElements", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;

    /* find first child */
    node = node->child; /* Field */
    if (!node)
        return NULL;

    return node;
}

/**
* @internal mxmlFindInstanceFirstInterfaceNode function
* @endinternal
*
* @brief   Get pointer to instance's first interface (bounded interface).
*
* @param[in] devNum                   - device number
* @param[in] instanceId               - instance identification
* @param[in] noInterfacePtr           - if instance have no interface
*
* @retval mxml_node_t                 - (pointer to) instance's first interface.
*/
mxml_node_t * mxmlFindInstanceFirstInterfaceNode
(
    IN  GT_U8                                     devNum,
    IN  GT_CHAR_PTR                               instanceId,
    OUT GT_BOOL                                   *noInterfacePtr
)
{
    mxml_node_t *node, *head;
    const GT_CHAR *value;
    GT_STATUS rc;
    *noInterfacePtr = GT_FALSE;
    /* Check if the instance is applicable for the device */
    rc = prvCpssDxChIdebugInstanceDeviceCheck(devNum, instanceId);
    if (rc != GT_OK)
        return NULL;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    value = instanceId;

    node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, NULL, "name", value, MXML_DESCEND_FIRST);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, "Interfaces", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    /* find first interface */
    node = node->child;
    if (!node)
    {
        *noInterfacePtr = GT_TRUE;
        return NULL;
    }

    return node;
}

/**
* @internal mxmlInterfaceFieldSizeGet function
* @endinternal
*
* @brief   Get pointer to field in interface and return it's start/end bit.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] fieldName                - field name
* @param[out] startBitPtr             - (pointer to) field's start bit
* @param[out] endBitPtr               - (pointer to) field's end bit
*
* @retval mxml_node_t                 - (pointer to) field in interface.
*/
mxml_node_t * mxmlInterfaceFieldSizeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *startBitPtr,
    OUT GT_U32                                  *endBitPtr
)
{
    mxml_node_t *node;
    const GT_CHAR *value, *attr;
    GT_CHAR *str1;
    GT_STATUS rc;

    /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
        return NULL;

    /* Check if the field is applicable for the device */
    rc = prvCpssDxChIdebugFieldDeviceCheck(devNum, fieldName);
    if (rc != GT_OK)
        return NULL;

    value = fieldName;

    node = mxmlFindInterfaceFirstFieldNode(devNum,interfaceName);
    if (!node)
        return NULL;

    while (node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        attr = prvCpssMxmlElementGetAttr(node,"internalName");

        /* skip fields that their name starts with 'reserved_' */
        str1 = cpssOsStrStr(attr, "reserved_");
        if((str1 == NULL) && (cpssOsStrStr(attr,value)) && (cpssOsStrlen(attr) == cpssOsStrlen(value)))
        {
            attr = prvCpssMxmlElementGetAttr(node,"bitStop");
            *endBitPtr = cpssOsStrTo32(attr);
            attr = prvCpssMxmlElementGetAttr(node,"bitStart");
            *startBitPtr = cpssOsStrTo32(attr);
            return node;
        }
        node = node->next;
    }

    return NULL;

}

/**
* @internal mxmlFieldEnumsGet function
* @endinternal
*
* @brief   Get pointer to field and return it's enumerations.
*
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
* @param[out] numOfEnumsPtr           - (pointer to) field's
*                                       number of enums
* @param[out] enumsArr                - (pointer to) field's
*                                       enums
*
* @retval mxml_node_t                 - (pointer to) field in interface.
*/
mxml_node_t * mxmlFieldEnumsGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *numOfEnumsPtr,
    OUT PRV_CPSS_DXCH_IDEBUG_ENUM_STC           enumsArr[]
)
{
    mxml_node_t *node, *field, *head, *interface;
    const GT_CHAR *attr;
    GT_CHAR attr1[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_STATUS rc;
    GT_U32 numOfEnums = 0;

    /* Check if the field is applicable for the device */
    rc = prvCpssDxChIdebugFieldDeviceCheck(devNum, fieldName);
    if (rc != GT_OK)
        return NULL;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    interface = prvCpssMxmlFindElement(head, head, "InterfacesList", NULL, NULL, MXML_DESCEND);
    if (!interface)
        return NULL;

    /* find first interface */
    interface = interface->child; /* InterfaceDesc */
    if (!interface)
        return NULL;

    while(interface != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(interface)

        attr = prvCpssMxmlElementGetAttr(interface,"internalName");
        cpssOsStrCpy(attr1,attr);
        node = mxmlFindInterfaceFirstFieldNode(devNum,attr1);
        if (!node)
            return NULL;

        /* find specific field */
        while (node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            field = node;

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrCmp(attr,fieldName)==0)
            {

                node = prvCpssMxmlFindElement(node, node, "Enums", NULL, NULL, MXML_DESCEND);
                if (!node)
                    return NULL;

                node = prvCpssMxmlFindElement(node, node, "Value", NULL, NULL, MXML_DESCEND);
                /* this is not enum */
                if (!node)
                {
                    *numOfEnumsPtr = 0;
                    return field;
                }
                else
                {
                    /* go over all enums */
                    while (node != NULL)
                    {
                        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

                        attr = prvCpssMxmlElementGetAttr(node,"name");
                        cpssOsStrCpy(enumsArr[numOfEnums].name,attr);
                        attr = prvCpssMxmlElementGetAttr(node,"value");
                        if (cpssOsStrCmp(attr,"a")==0)
                            enumsArr[numOfEnums].value = 10;
                        else if (cpssOsStrCmp(attr,"b")==0)
                            enumsArr[numOfEnums].value = 11;
                        else if (cpssOsStrCmp(attr,"c")==0)
                            enumsArr[numOfEnums].value = 12;
                        else if (cpssOsStrCmp(attr,"d")==0)
                            enumsArr[numOfEnums].value = 13;
                        else if (cpssOsStrCmp(attr,"e")==0)
                            enumsArr[numOfEnums].value = 14;
                        else if (cpssOsStrCmp(attr,"f")==0)
                            enumsArr[numOfEnums].value = 15;
                        else
                            enumsArr[numOfEnums].value = cpssOsStrToU32(attr, NULL, 0);

                        numOfEnums++;
                        node = node->next;
                    }

                    *numOfEnumsPtr = numOfEnums;
                    return field;
                }
            }
            /* continue to next field*/
            node = node->next;
        }
        /* continue to next interface*/
        interface = interface->next;
    }

    return NULL;
}
#if 0
if(muxId == (GT_U32)cpssOsStrToU32(attr, NULL, 0))
        {
            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            cpssOsStrCpy(segmentsArrPtr[numOfSeg],attr);

            numOfSeg++;
        }
        node = node->next;
GT_STATUS prvCpssDxChIdebugFieldValidityLogicEnumParse
(
    IN   GT_U8                                          devNum,
    IN   const GT_CHAR_PTR                              interfaceName,
    IN   const GT_CHAR_PTR                              fieldName,
    IN  const GT_CHAR_PTR                               fieldValueInStr,
    OUT  GT_U32                                         *fieldValuePtr
)
{
    const GT_CHAR  *attr;
    mxml_node_t    *node;
    node = mxmlFindInterfaceFirstFieldNode(devNum,interfaceName);

    if(node)
    {

        while (node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrCmp(attr,fieldName)==0)
            {
                  node = prvCpssMxmlFindElement(node, node, PRV_IDEBUG_ENUM_INFO_START, NULL, NULL, MXML_DESCEND);
                  if (!node)
                  {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
                  }
                  node = prvCpssMxmlFindElement(node, node, PRV_IDEBUG_ENUM_INFO_VAL,
                        PRV_IDEBUG_ENUM_INFO_NAME, fieldValueInStr, MXML_DESCEND);
                  if (!node)
                  {
                    /*This is a number not an enum*/
                    *fieldValuePtr = cpssOsStrToU32(fieldValueInStr, NULL, 0);
                  }
                  else

                  {
                    attr = prvCpssMxmlElementGetAttr(node,PRV_IDEBUG_ENUM_ATTR_VAL);
                    if(!attr)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
                    }
                    *fieldValuePtr = cpssOsStrToU32(attr, NULL, 0);
                  }
                  return GT_OK;
            }
            node=node->next;
         }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}
#endif
/**
* @internal mxmlInterfaceFieldSizeGet function
* @endinternal
*
* @brief   Get pointer to interface's stage identification.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval mxml_node_t                 - (pointer to) interface's stage id.
*/
mxml_node_t * mxmlInterfaceStageGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *stagePtr
)
{
    mxml_node_t *node,*head;
    const GT_CHAR *value;
    GT_CHAR *attr, attr1[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32 size;
    GT_STATUS rc;

     /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
        return NULL;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    value = interfaceName;

    node = prvCpssMxmlFindElement(head, head, "InterfacesList", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, NULL, "internalName", value, MXML_DESCEND_FIRST);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, "tags", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, NULL, "category", "SW", MXML_DESCEND_FIRST);
    if (!node)
        return NULL;
    node = node->child; /* tagName */

    while(node->type!=MXML_ELEMENT)
    {
        node=node->next;
    }

    node=node->child;

    while(node->type!=MXML_ELEMENT)
    {
        node=node->next;
    }

    attr = node->value.element.name; /* STAGE */

    if (cpssOsStrStr(attr,"STAGE"))
    {
        node = node->parent->next; /* tagValue */
        while(node->type!=MXML_ELEMENT)
        {
            node=node->next;
        }

         node=node->child;

        while(node->type!=MXML_ELEMENT)
        {
            node=node->next;
        }
        attr = node->value.element.name; /* stageId */
        /* <![CDATA[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]]>: remove ![CDATA[ from attr */
        attr = &(attr[8]);
        size = cpssOsStrlen(attr);
        /* remove ]] from attr */
        cpssOsMemSet(attr1, '\0', sizeof(attr1));
        cpssOsStrNCpy(attr1,attr,size-2);
        *stagePtr = idebugStageStrConv(attr1);

        return node; /* StageId */
    }

    return NULL;
}

/**
* @internal mxmlInterfaceFieldSizeGet function
* @endinternal
*
* @brief   Get pointer to field in interface and return it's start/end bit.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] fieldName                - field name
* @param[out] startBitPtr             - (pointer to) field's start bit
* @param[out] endBitPtr               - (pointer to) field's end bit
*
* @retval mxml_node_t                 - (pointer to) field in interface.
*/
mxml_node_t * mxmlInterfaceSizeGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    OUT GT_U32                                  *sizePtr,
    OUT GT_U32                                  *startBitPtr
)
{

    mxml_node_t *node,*head;
    const GT_CHAR *value,*attr;
    GT_STATUS rc;
    GT_U32    segmentId,muxId,prevSize=0;

     /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
        return NULL;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    value = interfaceName;

    node = prvCpssMxmlFindElement(head, head, "InterfacesList", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, NULL, "internalName", value, MXML_DESCEND_FIRST);
    if (!node)
        return NULL;
    attr = prvCpssMxmlElementGetAttr(node,"size");
    *sizePtr = cpssOsStrTo32(attr);

    /*Find place in mux*/
    if(GT_TRUE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
       rc = mxmlInstanceInfoSegmentIdGet(devNum,interfaceName,&segmentId,&muxId,&node);
       if (rc != GT_OK)
        return NULL;

       if(segmentId>0)
       {
           /*sum up all previous segments*/
           rc =mxmlInstanceInfoSegmentSizeGet(devNum,node,segmentId-1,muxId,&prevSize);
           if (rc != GT_OK)
            return NULL;
           *sizePtr+=prevSize;
           *startBitPtr=prevSize;
        }
    }

    return node;
}

/**
* @internal mxmlInterfacePolarityGet function
* @endinternal
*
* @brief   Get pointer to interface and return it's polarity.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] polarityPtr             - (pointer to) interface
*                                       polarity
*
* @retval mxml_node_t                 - (pointer to) interface.
*/
mxml_node_t * mxmlInterfacePolarityGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    OUT GT_CHAR                                 *polarityPtr
)
{

    mxml_node_t *node,*head;
    const GT_CHAR *value,*attr;
    GT_STATUS rc;

     /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
        return NULL;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    value = interfaceName;

    node = prvCpssMxmlFindElement(head, head, "InterfacesList", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    node = prvCpssMxmlFindElement(node, node, NULL, "internalName", value, MXML_DESCEND_FIRST);
    if (!node)
        return NULL;
    attr = prvCpssMxmlElementGetAttr(node,"ControlPolarity");
    cpssOsStrCpy(polarityPtr,attr);

    return node;
}

/**
* @internal mxmlNumOfInterfacesGet function
* @endinternal
*
* @brief   Get pointer to first interface and return number of interfaces for device.
*
* @param[in] devNum                 - device number
* @param[out] numOfInterfacesPtr    - (pointer to) num of
*                                     interfaces.
*
* @retval mxml_node_t               - (pointer to) first interface.
*/
mxml_node_t * mxmlNumOfInterfacesGet
(
    GT_U8           devNum,
    OUT GT_U32      *numOfInterfacesPtr
)
{
    mxml_node_t *node, *interface, *head;
    GT_U32 i=0;
    GT_STATUS rc;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    interface = node = prvCpssMxmlFindElement(head, head, "InterfacesList", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;

    /* find first interface */
    node = node->child; /* InterfaceDesc */
    if (!node)
        return NULL;

    while(node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)
        i++;
        node = node->next;
    }

    *numOfInterfacesPtr = i;

    return interface;
}

/**
* @internal mxmlNumOfInstancesGet function
* @endinternal
*
* @brief   Get pointer to first instance and return number of
*          instances for device.
*
* @param[in] devNum                 - device number
* @param[out] numOfInstancesPtr     - (pointer to) num of
*                                     instances.
*
* @retval mxml_node_t               - (pointer to) first
*                                     instance.
*/
static mxml_node_t * mxmlNumOfInstancesGet
(
    GT_U8           devNum,
    OUT GT_U32      *numOfInstancesPtr
)
{
    mxml_node_t *node, *instance, *head;
    GT_U32 i=0;
    GT_STATUS rc;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
    {
       return NULL;
    }

    instance = node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;

    /* find first instance */
    node = node->child;
    if (!node)
        return NULL;

    while(node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)
        i++;
        node = node->next;
    }

    *numOfInstancesPtr = i;

    return instance;
}

/**
* @internal mxmlInstanceInfoGet function
* @endinternal
*
* @brief   Get pointer to interface's instance and return instance information regurding interface.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] multiplicationIndex      - in case of multiplied interface this variable holds
*                                       the index of required multiplication.
* @param[out] instanceIdPtr           - (pointer to) instance
*                                       identification.
* @param[out] instanceDfxPipeIdPtr    - (pointer to) instance
*                                       dfx pipe.
* @param[out] instanceDfxBaseAddrPtr  - (pointer to) instance
*                                       dfx base address.
* @param[out] interfaceDfxIndexPtr     - (pointer to) interface
*                                       index in dfx.
*
* @retval mxml_node_t                 - (pointer to)
*                                       interface's instance.
*/
mxml_node_t * mxmlInstanceInfoGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          multiplicationIndex,
    OUT GT_CHAR                                         *instanceIdPtr,
    OUT GT_U32                                          *instanceDfxPipeIdPtr,
    OUT GT_U32                                          *instanceDfxBaseAddrPtr,
    OUT GT_U32                                          *interfaceDfxIndexPtr,
    OUT GT_U32                                          *metaDataPtr
)
{
    mxml_node_t *node, *instance, *head;
    const GT_CHAR *value, *attr,*currentBusName,*requiredBusName="";
    GT_STATUS rc;
    GT_CHAR                           bus[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_BOOL                           getFirst=GT_FALSE;
    GT_U32                            requiredMux=0;
    GT_CHAR_PTR                       muxTokeName;
    GT_U32                            count=0;
    GT_BOOL                           parseInterface;
    GT_U32                            dp=PRV_CPSS_DXCH_IDEBUG_META_INVALID_IDX_CNS;
    GT_U32                            pipe=PRV_CPSS_DXCH_IDEBUG_META_INVALID_IDX_CNS;
    GT_U32                            base;
    GT_BOOL                           dpInfoPresent = GT_FALSE;

     /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
        return NULL;

    if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChIdebugBoundedBusGet(devNum,interfaceName,multiplicationIndex,bus,&requiredMux);
        /*this is not bounded so get the first*/
        if(rc==GT_NOT_FOUND)
        {
            getFirst=GT_TRUE;
            rc = GT_OK;
        }

        if(rc==GT_OK)
        {
            if(GT_FALSE == getFirst)
            {
                requiredBusName = bus;
            }
        }
        else
        {
            return NULL;
        }
    }


    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    value = interfaceName;

    node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    /* find first instance */
    instance = node = node->child;
    if (!node)
        return NULL;

    while (instance != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(instance)

        currentBusName = prvCpssMxmlElementGetAttr(instance,"name");
        node = prvCpssMxmlFindElement(instance, instance, "Interfaces", NULL, NULL, MXML_DESCEND);
        if (!node)
            return NULL;
        /* find first interface */
        node = node->child;
        parseInterface = GT_FALSE;

        while(node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrCmp(attr,value) == 0)
            {
                if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    if((GT_TRUE==getFirst)||(cpssOsStrCmp(requiredBusName,currentBusName) == 0))
                    {
                        parseInterface = GT_TRUE;
                    }
                }
                else
                {
                    if(count==multiplicationIndex)
                    {
                        parseInterface = GT_TRUE;

                        if(metaDataPtr)
                        {
                            rc = prvCpssDxChIdebugPipeDpGet(currentBusName,&pipe,&dp,&dpInfoPresent);
                            if (rc != GT_OK)
                            {
                               return NULL;
                            }

                            *metaDataPtr = dp<<PRV_CPSS_DXCH_IDEBUG_META_DATA_DP_OFFSET_CNS;
                            *metaDataPtr|=(pipe<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PIPE_OFFSET_CNS);
                            *metaDataPtr|=(1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_VALID_OFFSET_CNS);

                            if(GT_TRUE==dpInfoPresent)
                            {
                                *metaDataPtr|=(1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PER_DP_OFFSET_CNS);
                            }
                        }
                    }
                    else
                    {
                        count++;
                    }
                }

                if(GT_TRUE==parseInterface)
                {
                    attr = prvCpssMxmlElementGetAttr(instance,"name");
                    cpssOsStrCpy(instanceIdPtr,attr);

                    rc = prvCpssDxChIdebugMuxTokenNameGet(devNum,&muxTokeName);
                    if (rc != GT_OK)
                        return NULL;

                     attr = prvCpssMxmlElementGetAttr(node,muxTokeName);
                    *interfaceDfxIndexPtr = cpssOsStrTo32(attr);

                    attr = prvCpssMxmlElementGetAttr(instance,"baseAddress");
                    *instanceDfxBaseAddrPtr = (GT_U32)cpssOsStrToU32(attr, NULL, 0);

                    if(metaDataPtr)
                    {
                      base =(*instanceDfxBaseAddrPtr)&(0xFFF<<8);
                      base>>=8;
                      *metaDataPtr|=(base<<PRV_CPSS_DXCH_IDEBUG_META_DATA_BASE_OFFSET_CNS);
                    }

                    attr = prvCpssMxmlElementGetAttr(instance,"dfxPipe");
                    *instanceDfxPipeIdPtr = cpssOsStrTo32(attr);

                    if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        /*check that we got required index*/
                        if((GT_TRUE==getFirst)||(requiredMux==*interfaceDfxIndexPtr))
                        {
                            return instance;
                        }
                    }
                    else
                    {
                      return instance;
                    }
                }

            }
            node = node->next;
        }

        instance = instance->next;
    }

    return NULL;
}

/**
* @internal mxmlBaseAddrInstanceGet function
* @endinternal
*
* @brief   Gets instance name by base address and dfx pipe.
*
* @param[in] devNum             - device number
* @param[in] baseAddr           - base address of searched instance .
* @param[out] dfxPipe           - dfx Pipe of searched instance
* @param[out] instanceIdPtr     - (pointer to) instance name.
*/
mxml_node_t * mxmlBaseAddrInstanceGet
(
    IN   GT_U8                                         devNum,
    IN   GT_U32                                        baseAddr,
    IN   GT_U32                                        dfxPipe,
    OUT  GT_CHAR                                       *instanceIdPtr
)
{
    mxml_node_t *node, *instance, *head;
    const GT_CHAR  *attr,*currentBusName;
    GT_STATUS rc;

    GT_U32           dfxPipeGet,baseAddGet;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        return NULL;

    node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
        return NULL;
    /* find first instance */
    instance = node = node->child;
    if (!node)
        return NULL;

    while (instance != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(instance)

        currentBusName = prvCpssMxmlElementGetAttr(instance,"name");
        attr = prvCpssMxmlElementGetAttr(instance,"baseAddress");
        baseAddGet = (GT_U32)cpssOsStrToU32(attr, NULL, 0);
        attr = prvCpssMxmlElementGetAttr(instance,"dfxPipe");
        dfxPipeGet = (GT_U32)cpssOsStrToU32(attr, NULL, 0);
        if ((baseAddr == baseAddGet) && (dfxPipe == dfxPipeGet))
        {
            cpssOsStrCpy(instanceIdPtr,currentBusName);
            break;
        }

        instance = instance->next;
    }

    return NULL;
}

/**
* @internal prvCpssDxChIdebugActionCheck function
* @endinternal
*
* @brief   Check that action parameters are valid.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] actionPtr                - (pointer to) action structure.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
GT_STATUS prvCpssDxChIdebugActionCheck
(
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC     *actionPtr
)
{
    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    switch(actionPtr->samplingMode) {
    case CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E:
    case CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E:
        /* do nothing */
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: sampling mode is not supported");
    }

    if (actionPtr->matchCounterIntThresh.lsb >= BIT_4 || actionPtr->matchCounterIntThresh.msb >= BIT_4 ||
        actionPtr->matchCounterIntThresh.middle >= BIT_1) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: match counter Thresh range lsb [0..%d] msb [0..%d] middle [0..%d]",BIT_4,BIT_4,BIT_1);
    }

    if (actionPtr->interruptMatchCounterEnable == GT_TRUE ||
        actionPtr->interruptSamplingEnable == GT_TRUE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Error: interrupts are not supported");

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInterfaceAttributesGet function
* @endinternal
*
* @brief   Get  interface DFX pipes , DFX mux indesx, DFX base adresses and number of multiplications.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] instanceDfxPipeIdPtr    - (pointer to) instance DFX pipe id
* @param[out] instanceDfxBaseAddrPtr  - (pointer to) instance DFX base address
* @param[out] interfaceDfxIndexPtr    - (pointer to) interface DFX index
* @param[out]numOfMultiplicationsPtr    (pointer to) number of multiplications
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChIdebugInterfaceAttributesGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          iDebugOpCode,
    OUT GT_U32                                          *instanceDfxPipeIdPtr,
    OUT GT_U32                                          *instanceDfxBaseAddrPtr,
    OUT GT_U32                                          *interfaceDfxIndexPtr,
    OUT GT_U32                                          *numOfMultiplicationsPtr,
    OUT GT_U32                                          *instanceMetaDataPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_CHAR instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32 i,multiplicationCounter = 0;
    mxml_node_t *node;
    GT_U32      numberOfTiles;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(instanceDfxPipeIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(instanceDfxBaseAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfaceDfxIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfMultiplicationsPtr)
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    *numOfMultiplicationsPtr=0;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_FALSE)
    {
        /*chek if multiple*/
        rc = prvCpssDxChIdebugIsMultipleInterface(devNum,interfaceName,numOfMultiplicationsPtr);
        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,\
                "prvCpssDxChIdebugIsMultipleInterface failed for interface %d",interfaceName);
        }

        for(i=0;i<*numOfMultiplicationsPtr;i++)
        {
            node = mxmlInstanceInfoGet(devNum,interfaceName,i,instanceId,instanceDfxPipeIdPtr,
                instanceDfxBaseAddrPtr,interfaceDfxIndexPtr,NULL);
            if(!node)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: instance info for interface [%s] was't found in XML",interfaceName);
            }

            instanceDfxPipeIdPtr++;
            instanceDfxBaseAddrPtr++;
            interfaceDfxIndexPtr++;
        }
     }
     else
     {
        do
        {
            node = mxmlInstanceInfoGet(devNum,interfaceName,multiplicationCounter++,instanceId,instanceDfxPipeIdPtr,
                instanceDfxBaseAddrPtr,interfaceDfxIndexPtr,instanceMetaDataPtr);
            if(!node)
            {
                continue;
            }

            /*In SIP6 write to DFX is automaticly copied to all tiles*/
            if(iDebugOpCode==PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS)
            {
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

                    if(instanceMetaDataPtr)
                    {
                        *instanceMetaDataPtr|=(0<<PRV_CPSS_DXCH_IDEBUG_META_DATA_TILE_OFFSET_CNS);
                    }

                    for(i=1;i<numberOfTiles;i++)
                    {
                        instanceDfxBaseAddrPtr[i]=instanceDfxBaseAddrPtr[0]
                            +PRV_DFX_XSB_TILE_OFFSET_MAC(devNum,i);
                        instanceDfxPipeIdPtr[i]=instanceDfxPipeIdPtr[0];
                        interfaceDfxIndexPtr[i]=interfaceDfxIndexPtr[0];

                        if(instanceMetaDataPtr)
                        {
                            instanceMetaDataPtr[i]= instanceMetaDataPtr[0];
                            instanceMetaDataPtr[i]|=(i<<PRV_CPSS_DXCH_IDEBUG_META_DATA_TILE_OFFSET_CNS);
                        }
                    }

                    (*numOfMultiplicationsPtr)+=numberOfTiles;
                    instanceDfxPipeIdPtr+=numberOfTiles;
                    instanceDfxBaseAddrPtr+=numberOfTiles;
                    interfaceDfxIndexPtr+=numberOfTiles;
                    if(instanceMetaDataPtr)
                    {
                        instanceMetaDataPtr+=numberOfTiles;
                    }
            }
            else
            {
                (*numOfMultiplicationsPtr)++;
                instanceDfxPipeIdPtr++;
                instanceDfxBaseAddrPtr++;
                interfaceDfxIndexPtr++;
            }
        }while(node);

     }

    return rc;
}

/**
* @internal prvCpssDxChIdebugInstanceDfxAccessPointerReset
*           function
* @endinternal
*
* @brief   Reset instance DFX access pointer.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] instanceDfxBaseAddr      - instance DFX base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
static GT_STATUS prvCpssDxChIdebugInstanceDfxAccessPointerReset
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          instanceDfxBaseAddr
)
{
    GT_U32 regAddr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    /* select DFX instance general configuration register address */
    regAddr = instanceDfxBaseAddr + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

    /* before each new key/mask configuration, reset access pointer */
    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,17,1,1);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,17,1,0);

    return rc;
}


GT_STATUS prvCpssDxChIdebugInstanceDfxPipeSet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          pipeInd
)
{
    GT_U32 regAddr;

    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

     /* select DFX pipe for Read and Write Transaction */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;

    return cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1 << pipeInd));

}

/**
* @internal prvCpssDxChIdebugInstanceDfxInterfaceSelect
*           function
* @endinternal
*
* @brief   Select instance DFX index
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] instanceDfxBaseAddr      - instance DFX base
* @param[in] interfaceDfxIndex        - interface DFX index
* @param[in] interfacePolarity        - interface Polarity
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChIdebugInstanceDfxInterfaceSelect
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          instanceDfxBaseAddr,
    IN  GT_U32                                          interfaceDfxIndex,
    IN  GT_CHAR_PTR                                     interfacePolarity
)
{
    GT_U32 regAddr;
    GT_STATUS rc;
    GT_U32 polarity;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    /* select DFX instance general configuration register address */
    regAddr = instanceDfxBaseAddr + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);


    /* select interface */
    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,11,5,interfaceDfxIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* select interface polarity */
    if(cpssOsStrCmp(interfacePolarity,"AL")==0)
    {
        polarity = 0; /* Active Low_Low */
    }
    else if (cpssOsStrCmp(interfacePolarity,"AH")==0)
    {
        polarity = 3; /* Active High_High */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unexpected polarity %s",interfacePolarity);
    }

    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,18,2,polarity);

    return rc;
}

/**
* @internal prvCpssDxChIdebugPacketMarkingSet function
* @endinternal
*
* @brief   Enable Packet Marking
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChIdebugPacketMarkingSet
(
    IN  GT_U8                                           devNum
)
{
    GT_U32 regAddr;
    GT_STATUS rc;
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 numOfMultiplications,multiplicationIterator;
    GT_CHAR interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS] = "tti2pcl_desc";

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E | CPSS_ALDRIN_E | CPSS_ALDRIN2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
            instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, interfaceName);
    if (rc != GT_OK)
    {
        return rc;
    }
    for(multiplicationIterator =0;multiplicationIterator<numOfMultiplications;multiplicationIterator++)
    {
         /* select DFX pipe for Read and Write Transaction */
        rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeIdArr[multiplicationIterator]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance general configuration register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);


        /* bit 23: Descriptor Setting Enable */
        /* bit 24-27: Descriptor Setting bit set '0' */
        /* bit 28: Descriptor Setting Polarity set '1' (high) */
        rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,23,6,0x21);
        if (rc != GT_OK)
        {
            return rc;
        }

    }

    return rc;
}

/**
* @internal prvCpssDxChIdebugInstanceDfxIndexGet function
* @endinternal
*
* @brief   get index of bounded interface
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceId              - interface Id
* @param[out] indexPtr                - (pointer to) interface DFX index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChIdebugInstanceDfxIndexGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceId,
    OUT GT_U32                                          *indexPtr
)
{
    GT_U32       regAddr;
    GT_U32       interfaceDfxIndex;
    GT_U32       instanceDfxPipeId;
    GT_U32       instanceDfxBaseAddr;
    mxml_node_t  *node;
    GT_CHAR instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_STATUS    rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceId);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();


    node = mxmlInstanceInfoGet(devNum,interfaceId,0,instanceId,&instanceDfxPipeId,&instanceDfxBaseAddr,&interfaceDfxIndex,NULL);
    if (!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: instance info for interface [%s] was't found in XML",interfaceId);

    /* select DFX pipe for Read and Write Transaction */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;
    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1 << instanceDfxPipeId));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance general configuration register address */
    regAddr = instanceDfxBaseAddr + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

    /* get interface */
    rc = cpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr,11,5,indexPtr);

    return rc;
}

/**
* @internal prvCpssDxChIdebugInterfaceSet function
* @endinternal
*
* @brief   Set interface fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] numOfFields              - (pointer to) interface
*                                       fields size
* @param[in] fieldsValueArr           - interface fields values
*                                       array
* @param[in] isData                   - update data/mask
* @param[in] regAddr                  - register address
* @param[in] interfaceNumOfWords      - interface num of words
* @param[in] interfaceStartBit        - interface start bit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*/
static GT_STATUS prvCpssDxChIdebugInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_U32                                  numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[],
    IN  GT_BOOL                                 isData,
    IN  GT_U32                                  regAddr,
    IN  GT_U32                                  interfaceNumOfWords,
    IN  GT_U32                                  interfaceStartBit
)
{
    GT_U32 size,endBit,startBit,i,j;
    GT_U32 offset,length;
    GT_STATUS rc = GT_OK;
    GT_U32 fieldNumOfWords;
    GT_U32 *arr = NULL;
    mxml_node_t *node;
    GT_U32 saveRegAddr;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* Check if the fields are applicable for the device */
    for (i=0; i<numOfFields; i++)
    {
        rc = prvCpssDxChIdebugFieldDeviceCheck(devNum, fieldsValueArr[i].fieldName);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (isData)
    {
        saveRegAddr = regAddr - PRV_DFX_XSB_MATCH_KEY_REG_OFFSET_MAC(devNum);
    }
    else
    {
        saveRegAddr = regAddr - PRV_DFX_XSB_MATCH_MASK_REG_OFFSET_MAC(devNum);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
         maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    }
    /* array allocation */
    arr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxInterfaceLength);
    if (arr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(arr,0,sizeof(GT_U32)*maxInterfaceLength);

    /* read all interface match key/mask according to register address */
    for (i = 0; i < interfaceNumOfWords; i++) {
        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &(arr[i]));
        if (rc != GT_OK) {
            goto exit_cleanly_lbl;
        }
    }

    /* update all fields data */
    for(i=0; i<numOfFields; i++)
    {
        node = mxmlInterfaceFieldSizeGet(devNum,interfaceName,fieldsValueArr[i].fieldName,&startBit,&endBit);
        if(!node)
        {
            rc = GT_BAD_STATE;
            CPSS_LOG_ERROR_MAC("Error: failed to get field [%s] size in XML",fieldsValueArr[i].fieldName);
            goto exit_cleanly_lbl;
        }

        fieldNumOfWords = LENGTH_TO_NUM_WORDS_MAC(endBit-startBit+ 1) ;
        if(fieldNumOfWords > CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS)
        {
            rc = GT_BAD_SIZE;
            CPSS_LOG_ERROR_MAC("Error: fields [%d] number of words",fieldsValueArr[i].fieldName,fieldNumOfWords);
            goto exit_cleanly_lbl;
        }

        size = endBit-startBit;
        length = LENGTH_TO_NUM_BITS_MAC(size);
        offset = startBit;

        for(j=0; j<fieldNumOfWords; j++)
        {
            if (isData)
            {
                U32_SET_FIELD_IN_ENTRY_MAC(arr,offset+interfaceStartBit,length,fieldsValueArr[i].data[j]);
            }
            else
            {
                U32_SET_FIELD_IN_ENTRY_MAC(arr,offset+interfaceStartBit,length,~(fieldsValueArr[i].mask[j])); /*mask has invert logic.*/
            }
            offset +=32;
            size -=32;
            length = LENGTH_TO_NUM_BITS_MAC(size);
        }
    }

    /* reset pointer */
    rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,saveRegAddr);
    if (rc != GT_OK) {
        return rc;
    }

    /* write all interface match key/mask according to register address */
    for(i=0; i<interfaceNumOfWords; i++)
    {
        rc = cpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,arr[i]);
        if(rc != GT_OK)
        {
           goto exit_cleanly_lbl;
        }
    }

exit_cleanly_lbl:

    cpssOsFree(arr);

    return rc;
}

/**
* @internal prvCpssDxChIdebugInterfaceGet function
* @endinternal
*
* @brief   Get interface fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] numOfFields              - (pointer to) interface
*                                       fields size
* @param[out] fieldsValueArr          - interface fields values
*                                       array
* @param[in] isData                   - update data/mask
* @param[in] regAddr                  - register address
* @param[in] interfaceNumOfWords      - interface num of words
* @param[in] interfaceStartBit        - interface start bit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*/
static GT_STATUS prvCpssDxChIdebugInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_U32                                  numOfFields,
    OUT  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[],
    IN  GT_BOOL                                 isData,
    IN  GT_U32                                  regAddr,
    IN  GT_U32                                  interfaceNumOfWords,
    IN  GT_U32                                  interfaceStartBit
)
{
    GT_U32 size,endBit,startBit,i,j;
    GT_U32 offset,length;
    GT_STATUS rc = GT_OK;
    GT_U32 fieldNumOfWords,temp;
    GT_U32 *arr = NULL;
    mxml_node_t *node;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC* cbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
         maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    }
    /* array allocation */
    arr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxInterfaceLength);
    if (arr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(arr,0,sizeof(GT_U32)*maxInterfaceLength);

    /* read all interface match key/mask according to register address */
    for (i = 0; i < interfaceNumOfWords; i++) {
        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &(arr[i]));
        if (rc != GT_OK) {
            goto exit_cleanly_lbl;
        }
    }

    /* return all fields data */
    for(i=0; i<numOfFields; i++)
    {
        node = mxmlInterfaceFieldSizeGet(devNum,interfaceName,fieldsValueArr[i].fieldName,&startBit,&endBit);
        if(!node)
        {
            rc = GT_BAD_STATE;
            CPSS_LOG_ERROR_MAC("Error: failed to get field [%s] size in XML",fieldsValueArr[i].fieldName);
            goto exit_cleanly_lbl;
        }

        fieldNumOfWords = LENGTH_TO_NUM_WORDS_MAC(endBit-startBit+ 1) ;
        if(fieldNumOfWords > CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS)
        {
            rc = GT_BAD_SIZE;
            CPSS_LOG_ERROR_MAC("Error: fields [%d] number of words",fieldsValueArr[i].fieldName,fieldNumOfWords);
            goto exit_cleanly_lbl;
        }

        size = endBit-startBit;
        length = LENGTH_TO_NUM_BITS_MAC(size);
        offset = startBit;

        for(j=0; j<fieldNumOfWords; j++)
        {
            if (isData)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arr,offset+interfaceStartBit,length,fieldsValueArr[i].data[j]);
            }
            else
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arr,offset+interfaceStartBit,length,temp); /*mask has invert logic.*/
                fieldsValueArr[i].mask[j] = ~temp;
            }
            offset +=32;
            size -=32;
            length = LENGTH_TO_NUM_BITS_MAC(size);
        }
    }

    /* return all fields validity */
    for(i=0; i<numOfFields; i++)
    {
        /* update validity for field */
        rc = prvCpssDxChIdebugDbCbGet(devNum, &cbPtr);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
        rc = cbPtr->fieldValidityLogicUpdateCbFunc(devNum,interfaceName,fieldsValueArr[i].fieldName,numOfFields,fieldsValueArr);
        if(rc != GT_OK)
        {
            if(GT_TRUE==prvCpssDxChIdebugFailOnUnfoundToken(devNum))
            {
                goto exit_cleanly_lbl;
            }
            rc = GT_OK;
        }
    }

exit_cleanly_lbl:

    cpssOsFree(arr);

    return rc;
}

/**
* @internal prvCpssDxChIdebugFieldDeviceCheck function
* @endinternal
*
* @brief   Check if the field is applicable for the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugFieldDeviceCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         fieldName
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    devNum = devNum;
    if (!idebugDB->idebugFieldsStrPtrArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    for (i=0; i<idebugDB->numOfFields; i++)
    {
        if(cpssOsStrCmp(idebugDB->idebugFieldsStrPtrArr[i],fieldName) == 0)
            break;
    }
    if (i >= idebugDB->numOfFields)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugFieldIsEnumCheck function
* @endinternal
*
* @brief   Check if the field is enum in the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
* @param[out] isEnumPtr               - whether field is enum
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugFieldIsEnumCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         fieldName,
    OUT GT_BOOL                             *isEnumPtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    devNum = devNum;
    if (!idebugDB->idebugFieldsStrPtrArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    for (i=0; i<idebugDB->numOfFields; i++)
    {
        if(cpssOsStrCmp(idebugDB->idebugFieldsStrPtrArr[i],fieldName) == 0)
        {
            break;
        }
    }
    if (i >= idebugDB->numOfFields)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if (!idebugDB->idebugFieldsIsEnumStrPtrArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    *isEnumPtr = idebugDB->idebugFieldsIsEnumStrPtrArr[i];

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInterfaceDeviceCheck function
* @endinternal
*
* @brief   Check if the interface is applicable for the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugInterfaceDeviceCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         interfaceName
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();
    devNum = devNum;

    if (!idebugDB->idebugInterfacesStrPtrArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    for (i=0; i<idebugDB->numOfInterfaces; i++)
    {
        if(cpssOsStrCmp(idebugDB->idebugInterfacesStrPtrArr[i],interfaceName) == 0)
            break;
    }
    if (i >= idebugDB->numOfInterfaces)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Error: interface [%s] is not applicable in devNum [%d]",interfaceName,devNum);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInstanceDeviceCheck function
* @endinternal
*
* @brief   Check if the instance is applicable for the device
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] instanceName             - instance name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugInstanceDeviceCheck
(
    IN  GT_U8                               devNum,
    IN  GT_CHAR_PTR                         instanceName
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();
    devNum = devNum;

    if (!idebugDB->idebugInstancesStrPtrArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    for (i=0; i<idebugDB->numOfInstances; i++)
    {
        if(cpssOsStrCmp(idebugDB->idebugInstancesStrPtrArr[i],instanceName) == 0)
            break;
    }
    if (i >= idebugDB->numOfInstances)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    return GT_OK;
}
/**
* @internal prvCpssDxChIdebugFileNameGet function
* @endinternal
*
* @brief   Get XML file name
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                  - device number
* @param[out] xmlFileName            -(pointer to)XML file name
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
*
*/
GT_STATUS prvCpssDxChIdebugFileNameGet
(
    IN  GT_U8                                      devNum,
    OUT GT_CHAR_PTR                                *xmlFileName
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            *xmlFileName= PRV_CPSS_ALDRIN_IDEBUG_XML_FILE;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            *xmlFileName= PRV_CPSS_ALDRIN2_IDEBUG_XML_FILE;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            *xmlFileName = PRV_CPSS_FALCON_IDEBUG_XML_FILE;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            *xmlFileName = PRV_CPSS_HAWK_IDEBUG_XML_FILE;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            *xmlFileName = PRV_CPSS_PHOENIX_IDEBUG_XML_FILE;
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            *xmlFileName = PRV_CPSS_HARRIER_IDEBUG_XML_FILE;
            break;
        default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: Device family is not supported  for iDebug");
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugXmlStringsInit function
* @endinternal
*
* @brief   Copy all XML strings to idebug DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChIdebugXmlStringsInit
(
    IN  GT_U8                                   devNum
)
{
    typedef struct
    {
        PRV_CPSS_DXCH_IDEBUG_FIELD_STC  field;
        GT_BOOL                         isEnum;
    }PRV_CPSS_DXCH_IDEBUG_FIELD_ENUM_STC;

    GT_U32 i,j,k,maxNumOfFields=0;
    GT_STATUS rc = GT_OK;
    mxml_node_t *node, *field;
    PRV_CPSS_DXCH_IDEBUG_FIELD_ENUM_STC *fieldsArr = NULL;
    GT_U32 numOfInterfaces, numOfInstances, numOfFields,fieldCounter=0;
    GT_BOOL found;
    const GT_CHAR *attr, *str1;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* memory allocations for strings */

    /* allocate interfaces */
    node = mxmlNumOfInterfacesGet(devNum,&numOfInterfaces);
    if (node == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get num of interfaces from XML");
    idebugDB->idebugInterfacesStrPtrArr  = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*numOfInterfaces);
    if (idebugDB->idebugInterfacesStrPtrArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"Error: failed to allocate memory for iDebug DB");
    cpssOsMemSet(idebugDB->idebugInterfacesStrPtrArr,0,sizeof(GT_CHAR_PTR)*numOfInterfaces);
    idebugDB->numOfInterfaces = numOfInterfaces;

    node = node->child;  /* node is first interface */
    if (!node)
    {
        rc = GT_BAD_PARAM;
        goto exit_cleanly_lbl;
    }

    i=0;
    while (node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        attr = prvCpssMxmlElementGetAttr(node,"internalName");

        if(cpssOsStrlen(attr) >= CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS)
        {
            CPSS_LOG_ERROR_MAC("Error: string [%s] length [%d] is bigger then [%d]",attr,cpssOsStrlen(attr),CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }
        idebugDB->idebugInterfacesStrPtrArr[i] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*(cpssOsStrlen(attr)+1));
        if (idebugDB->idebugInterfacesStrPtrArr[i] == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            goto exit_cleanly_lbl;
        }

        cpssOsStrCpy(idebugDB->idebugInterfacesStrPtrArr[i],attr);
        i++;
        node = node->next;
    }

    for (i=0; i<idebugDB->numOfInterfaces; i++)
    {
        rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,idebugDB->idebugInterfacesStrPtrArr[i],&numOfFields);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
        maxNumOfFields +=numOfFields;
    }

    /* allocate instances */
    node = mxmlNumOfInstancesGet(devNum,&numOfInstances);
    if (node == NULL)
    {
        rc = GT_BAD_STATE;
        CPSS_LOG_ERROR_MAC("Error: failed to get num of interfaces from XML");
        goto exit_cleanly_lbl;
    }
    idebugDB->idebugInstancesStrPtrArr = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*numOfInstances);
    if (idebugDB->idebugInstancesStrPtrArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(idebugDB->idebugInstancesStrPtrArr,0,sizeof(GT_CHAR_PTR)*numOfInstances);
    idebugDB->numOfInstances = numOfInstances;

    node = node->child;  /* node is first instance */
    if (!node)
    {
        rc = GT_BAD_PARAM;
        goto exit_cleanly_lbl;
    }

    i=0;
    while(node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        attr = prvCpssMxmlElementGetAttr(node,"name");
        if(cpssOsStrlen(attr) >= CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS)
        {
            CPSS_LOG_ERROR_MAC("Error: string [%s] length [%d] is bigger then [%d]",attr,cpssOsStrlen(attr),CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }
        idebugDB->idebugInstancesStrPtrArr[i] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*(cpssOsStrlen(attr)+1));
        cpssOsStrCpy(idebugDB->idebugInstancesStrPtrArr[i],attr);
        i++;
        node = node->next;
    }

    /* allocate fields */
    idebugDB->idebugFieldsStrPtrArr = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*maxNumOfFields);
    if (idebugDB->idebugFieldsStrPtrArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    idebugDB->idebugFieldsIsEnumStrPtrArr = (GT_BOOL*)cpssOsMalloc(sizeof(GT_BOOL)*maxNumOfFields);
    if (idebugDB->idebugFieldsIsEnumStrPtrArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(idebugDB->idebugFieldsIsEnumStrPtrArr,0,sizeof(GT_BOOL)*maxNumOfFields);

    for (i=0; i<numOfInterfaces; i++)
    {
        rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,idebugDB->idebugInterfacesStrPtrArr[i],&numOfFields);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        fieldsArr = (PRV_CPSS_DXCH_IDEBUG_FIELD_ENUM_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_ENUM_STC)*numOfFields);
        if (fieldsArr == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            goto exit_cleanly_lbl;
        }
        cpssOsMemSet(fieldsArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_ENUM_STC)*numOfFields);
        node = mxmlFindInterfaceFirstFieldNode(devNum,idebugDB->idebugInterfacesStrPtrArr[i]);
        if(!node)
        {
            rc = GT_BAD_PARAM;
            goto exit_cleanly_lbl;
        }

        j=0;
        while (node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");

            /* skip fields that their name starts with 'reserved_' */
            str1 = cpssOsStrStr(attr, "reserved_");
            if(str1 == NULL)
            {
                if(cpssOsStrlen(attr) >= CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS)
                {
                    CPSS_LOG_ERROR_MAC("Error: string [%s] length [%d] is bigger then [%d]",attr,cpssOsStrlen(attr),CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
                    rc = GT_FAIL;
                    goto exit_cleanly_lbl;
                }
                cpssOsStrCpy(fieldsArr[j].field.fieldName,attr);

                /* check if the field is enum */
                {
                    field = prvCpssMxmlFindElement(node, node, "Enums", NULL, NULL, MXML_DESCEND);
                    if (!field)
                    {
                        CPSS_LOG_ERROR_MAC("Error: no attribute Enums for field %s",attr);
                        rc = GT_FAIL;
                        goto exit_cleanly_lbl;
                    }

                    field = prvCpssMxmlFindElement(field, field, "Value", NULL, NULL, MXML_DESCEND);
                    /* this is not enum */
                    if (!field)
                        fieldsArr[j].isEnum = GT_FALSE;
                    else
                        fieldsArr[j].isEnum = GT_TRUE;
                }
                j++;
            }
            node = node->next;
        }

        for (j=0; j<numOfFields; j++)
        {
            found = GT_FALSE;
            for (k=0; k<fieldCounter; k++)
            {
                if (cpssOsStrCmp(idebugDB->idebugFieldsStrPtrArr[k],fieldsArr[j].field.fieldName) == 0)
                {
                    found = GT_TRUE;
                    continue;
                }
            }
            /* add new field to string array */
            if (found == GT_FALSE)
            {
                idebugDB->idebugFieldsStrPtrArr[fieldCounter] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*(cpssOsStrlen(fieldsArr[j].field.fieldName)+1));
                cpssOsStrCpy(idebugDB->idebugFieldsStrPtrArr[fieldCounter],fieldsArr[j].field.fieldName);
                idebugDB->idebugFieldsIsEnumStrPtrArr[fieldCounter] = fieldsArr[j].isEnum;
                fieldCounter++;
            }
        }

        cpssOsFree(fieldsArr);
    }
    idebugDB->numOfFields = fieldCounter;

    return GT_OK;

exit_cleanly_lbl:

    if (idebugDB->idebugInterfacesStrPtrArr)
    {
        for (i=0; i<idebugDB->numOfInterfaces; i++)
        {
            if (idebugDB->idebugInterfacesStrPtrArr[i])
            {
                cpssOsFree(idebugDB->idebugInterfacesStrPtrArr[i]);
                idebugDB->idebugInterfacesStrPtrArr[i] = NULL;
            }
        }
        cpssOsFree(idebugDB->idebugInterfacesStrPtrArr);
        idebugDB->idebugInterfacesStrPtrArr = NULL;
    }
    if (idebugDB->idebugInstancesStrPtrArr)
    {
        for (i=0; i<idebugDB->numOfInstances; i++)
        {
            if (idebugDB->idebugInstancesStrPtrArr[i])
            {
                cpssOsFree(idebugDB->idebugInstancesStrPtrArr[i]);
                idebugDB->idebugInstancesStrPtrArr[i] = NULL;
            }
        }
        cpssOsFree(idebugDB->idebugInstancesStrPtrArr);
        idebugDB->idebugInstancesStrPtrArr = NULL;
    }
    if (idebugDB->idebugFieldsStrPtrArr)
    {
        for (i=0; i<idebugDB->numOfFields; i++)
        {
            if (idebugDB->idebugFieldsStrPtrArr[i])
            {
                cpssOsFree(idebugDB->idebugFieldsStrPtrArr[i]);
                idebugDB->idebugFieldsStrPtrArr[i] = NULL;
            }
        }
        cpssOsFree(idebugDB->idebugFieldsStrPtrArr);
        idebugDB->idebugFieldsStrPtrArr = NULL;
    }

    if (idebugDB->idebugFieldsIsEnumStrPtrArr)
    {
        cpssOsFree(idebugDB->idebugFieldsIsEnumStrPtrArr);
    }

    if (fieldsArr)
    {
        cpssOsFree(fieldsArr);
    }

    return rc;
}

/**
* @internal prvCpssDxChIdebugFileOpen function
* @endinternal
*
* @brief   Open XML file according to device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChIdebugFileOpen
(
    IN  GT_U8                                   devNum
)
{
    GT_U32 i;
    GT_STATUS rc;
    GT_CHAR_PTR xmlFileName;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* memory allocation for counters */
    if (idebugDB->idebugInterfacePtrArr == NULL)
    {
        idebugDB->idebugInterfacePtrArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR)*PRV_CPSS_MAX_PP_DEVICES_CNS);
        if (idebugDB->idebugInterfacePtrArr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"Error: failed to allocate memory for iDebug DB");
        cpssOsMemSet(idebugDB->idebugInterfacePtrArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR)*PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        for (i=0; i<idebugDB->numOfFamilies; i++)
        {
            /* file is open; and also all other allocations were done */
            if (idebugDB->idebugFamilyPtrArr[i]->familyId == PRV_CPSS_PP_MAC(devNum)->devFamily)
                return GT_OK;
        }

        /* allocate the family */
        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies] = (PRV_CPSS_DXCH_IDEBUG_FAMILY_PTR)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FAMILY_STC));
        if (idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"Error: failed to allocate memory for iDebug DB");
        }
        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies]->familyId = PRV_CPSS_PP_MAC(devNum)->devFamily;
        idebugDB->numOfFamilies++;

        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
        if (idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            CPSS_LOG_ERROR_MAC("Error: failed to open XML file");
            goto exit_cleanly_lbl;
        }

        /* open CIDER XML */
        rc= prvCpssDxChIdebugFileNameGet(devNum,&xmlFileName);
        if(rc!=GT_OK)
        {
             CPSS_LOG_ERROR_MAC("Error: failed to get XML file name");
            goto exit_cleanly_lbl;
        }

        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file->type = CPSS_OS_FILE_REGULAR;
        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file->fd = cpssOsFopen(xmlFileName, "r",idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);

        if(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file->fd==CPSS_OS_FILE_INVALID)
        {
            rc = GT_FAIL;
            CPSS_LOG_ERROR_MAC("Error: open  XML file %s  failed",xmlFileName);
            goto exit_cleanly_lbl;
        }

        /*cpssOsPrintf("\n%s found and opened for read\n",PRV_CPSS_IDEBUG_XML_FILE);*/

        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head = prvCpssMxmlLoadFd(NULL, idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file, NULL, MXML_NO_CALLBACK);
        if (idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head == NULL)
        {
            cpssOsFclose(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
            cpssOsFree(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
            idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file = NULL;
            rc = GT_FAIL;
            CPSS_LOG_ERROR_MAC("Error: reading XML file instanses failed");
            goto exit_cleanly_lbl;
        }

       /* memory allocations for strings */
       rc = prvCpssDxChIdebugXmlStringsInit(devNum);
       if (rc != GT_OK)
       {
            cpssOsFclose(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
            prvCpssMxmlDelete(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head);
            cpssOsFree(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
            idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file = NULL;
            idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head = NULL;
            rc = GT_OUT_OF_CPU_MEM;
            CPSS_LOG_ERROR_MAC("Error: failed to allocate strings arrays for family");
            goto exit_cleanly_lbl;
       }

       /* memory allocation for counters */
       if (idebugDB->idebugInterfacePtrArr[devNum] == NULL)
       {
            idebugDB->idebugInterfacePtrArr[devNum] = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_STC)*idebugDB->numOfInterfaces);
            if (idebugDB->idebugInterfacePtrArr[devNum] == NULL)
            {
                cpssOsFclose(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
                prvCpssMxmlDelete(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head);
                cpssOsFree(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
                idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file = NULL;
                idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head = NULL;
                rc = GT_OUT_OF_CPU_MEM;
                CPSS_LOG_ERROR_MAC("Error: failed to allocate memory for iDebug DB");
                goto exit_cleanly_lbl;
            }
            /* clean idebug interface DB information to support counters 'ClearOnRead' operation for SIP5   */
            cpssOsMemSet(idebugDB->idebugInterfacePtrArr[devNum],0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_STC)*idebugDB->numOfInterfaces);
            idebugDB->numOfDevices++;
        }

        /* memory allocation for CB functions */
        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->cbFunc = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC));
        if(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->cbFunc == NULL) {

            cpssOsFclose(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
            prvCpssMxmlDelete(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head);
            cpssOsFree(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file);
            idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->file = NULL;
            idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->head = NULL;
            rc = GT_OUT_OF_CPU_MEM;
            CPSS_LOG_ERROR_MAC("Error: failed to allocate CB for family");
            goto exit_cleanly_lbl;
        }

        /* assign relevant cb functions */
       idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->cbFunc->fieldValidityLogicUpdateCbFunc = prvCpssDxChIdebugFieldValidityLogicUpdate;

       idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]->muxDb = NULL;

       break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Error: devNum [%d] is not applicable",devNum);
        break;
    }

    rc = prvCpssDxChIdebugMuxDbInit(devNum);
    if(rc!=GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    return GT_OK;

exit_cleanly_lbl:

    /* delete allocations */
    if(idebugDB->idebugInterfacePtrArr[devNum]) {
        cpssOsFree(idebugDB->idebugInterfacePtrArr[devNum]);
        idebugDB->idebugInterfacePtrArr[devNum] = NULL;
        idebugDB->numOfDevices--;
    }

     if(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]) {
        cpssOsFree(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]);
        idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1] = NULL;
        idebugDB->numOfFamilies--;
    }

     if (idebugDB->numOfDevices == 0)
     {
         cpssOsFree(idebugDB->idebugInterfacePtrArr);
         idebugDB->idebugInterfacePtrArr = NULL;
     }

    if (idebugDB->idebugInterfacesStrPtrArr)
    {
        for (i=0; i<idebugDB->numOfInterfaces; i++)
            if (idebugDB->idebugInterfacesStrPtrArr[i])
                cpssOsFree(idebugDB->idebugInterfacesStrPtrArr[i]);
        cpssOsFree(idebugDB->idebugInterfacesStrPtrArr);
        idebugDB->idebugInterfacesStrPtrArr = NULL;
    }
    if (idebugDB->idebugInstancesStrPtrArr)
    {
        for (i=0; i<idebugDB->numOfInstances; i++)
            if (idebugDB->idebugInstancesStrPtrArr[i])
                cpssOsFree(idebugDB->idebugInstancesStrPtrArr[i]);
        cpssOsFree(idebugDB->idebugInstancesStrPtrArr);
        idebugDB->idebugInstancesStrPtrArr = NULL;
    }
    if (idebugDB->idebugFieldsStrPtrArr)
    {
        for (i=0; i<idebugDB->numOfFields; i++)
            if (idebugDB->idebugFieldsStrPtrArr[i])
                cpssOsFree(idebugDB->idebugFieldsStrPtrArr[i]);
        cpssOsFree(idebugDB->idebugFieldsStrPtrArr);
        idebugDB->idebugFieldsStrPtrArr = NULL;
    }

    if (idebugDB->idebugFieldsIsEnumStrPtrArr)
    {
        cpssOsFree(idebugDB->idebugFieldsIsEnumStrPtrArr);
    }

    return rc;
}

/**
* @internal prvCpssDxChIdebugFileClose function
* @endinternal
*
* @brief   Close XML file according to device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChIdebugFileClose
(
    IN  GT_U8                                   devNum
)
{
    GT_U32 i,j;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(idebugDB->idebugInterfacePtrArr == NULL) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* release memory allocation for counters */
    if (idebugDB->idebugInterfacePtrArr[devNum] != NULL)
    {
        cpssOsFree(idebugDB->idebugInterfacePtrArr[devNum]);
        idebugDB->idebugInterfacePtrArr[devNum] = NULL;
        idebugDB->numOfDevices--;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    if (idebugDB->numOfDevices == 0)
    {
        if(idebugDB->idebugInterfacePtrArr != NULL) {
            cpssOsFree(idebugDB->idebugInterfacePtrArr);
            idebugDB->idebugInterfacePtrArr = NULL;
        }
        if (idebugDB->idebugInterfacesStrPtrArr != NULL )
        {
            for (i=0; i<idebugDB->numOfInterfaces; i++)
                if (idebugDB->idebugInterfacesStrPtrArr[i] != NULL)
                    cpssOsFree(idebugDB->idebugInterfacesStrPtrArr[i]);
            cpssOsFree(idebugDB->idebugInterfacesStrPtrArr);
            idebugDB->idebugInterfacesStrPtrArr = NULL;
        }
        if (idebugDB->idebugInstancesStrPtrArr != NULL)
        {
            for (i=0; i<idebugDB->numOfInstances; i++)
                if (idebugDB->idebugInstancesStrPtrArr[i] != NULL)
                    cpssOsFree(idebugDB->idebugInstancesStrPtrArr[i]);
            cpssOsFree(idebugDB->idebugInstancesStrPtrArr);
            idebugDB->idebugInstancesStrPtrArr = NULL;
        }
        if (idebugDB->idebugFieldsStrPtrArr != NULL)
        {
            for (i=0; i<idebugDB->numOfFields; i++)
                if (idebugDB->idebugFieldsStrPtrArr[i]!= NULL)
                    cpssOsFree(idebugDB->idebugFieldsStrPtrArr[i]);
            cpssOsFree(idebugDB->idebugFieldsStrPtrArr);
            idebugDB->idebugFieldsStrPtrArr = NULL;
        }
        if (idebugDB->idebugFieldsIsEnumStrPtrArr != NULL )
        {
            cpssOsFree(idebugDB->idebugFieldsIsEnumStrPtrArr);
        }
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        for (i=0; i<idebugDB->numOfFamilies; i++)
        {
            /* file is open; and also all other allocations were done */
            if (idebugDB->idebugFamilyPtrArr[i]->familyId == (PRV_CPSS_PP_MAC(devNum)->devFamily))
            {
                /* free allocations */
                cpssOsFclose(idebugDB->idebugFamilyPtrArr[i]->file);
                prvCpssMxmlDelete(idebugDB->idebugFamilyPtrArr[i]->head);
                cpssOsFree(idebugDB->idebugFamilyPtrArr[i]->cbFunc);
                idebugDB->idebugFamilyPtrArr[i]->cbFunc = NULL;
                cpssOsFree(idebugDB->idebugFamilyPtrArr[i]->file);
                idebugDB->idebugFamilyPtrArr[i]->file = NULL;
                idebugDB->idebugFamilyPtrArr[i]->head = NULL;

                for(j=0; j<idebugDB->numOfInstances; j++)
                {
                    cpssOsFree(idebugDB->idebugFamilyPtrArr[i]->muxDb[j]);
                }

                cpssOsFree(idebugDB->idebugFamilyPtrArr[i]->muxDb);

                /* copy all the remaining families */
                for (j=i; j<idebugDB->numOfFamilies-1; j++)
                {
                    if(idebugDB->idebugFamilyPtrArr[j+1])
                    {
                        idebugDB->idebugFamilyPtrArr[j] = idebugDB->idebugFamilyPtrArr[j+1];
                        idebugDB->idebugFamilyPtrArr[j+1] = NULL;
                    }
                }
                cpssOsFree(idebugDB->idebugFamilyPtrArr[idebugDB->numOfFamilies-1]);
                idebugDB->numOfFamilies--;

            }
        }

        /* file is not opened */
        return GT_OK;

        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Error: devNum [%d] is not applicable",devNum);
        break;
    }
}

/**
* @internal prvCpssDxChIdebugAllInterfacesSamplingDisable
*           function
* @endinternal
*
* @brief   Disable sampling on all interfaces in device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*/
GT_STATUS prvCpssDxChIdebugAllInterfacesSamplingDisable
(
    IN  GT_U8                                   devNum
)
{
    GT_U32                                      i,numOfInterfaces;
    GT_CHAR_PTR                                 *interfacesArrDbPtr = NULL;
    GT_STATUS                                   rc = GT_OK;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to interfaces array in iDebug DB */
    rc = prvCpssDxChIdebugDbInterfaceArrayGet(devNum,&numOfInterfaces,&interfacesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* disable sampling for all interfaces in devNum */
    for (i=0; i<numOfInterfaces; i++)
    {
        rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfacesArrDbPtr[i],GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugDbPowerSaveSet function
* @endinternal
*
* @brief   The function set power save in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum        - device number
* @param[in]  enable        - enable/disable power save mode
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbPowerSaveSet
(
    IN  GT_U8                                           devNum,
    IN  GT_BOOL                                         enable
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    idebugDB->powerSaveEnable = enable;

   return GT_OK;
}

/**
* @internal prvCpssDxChIdebugDbCbGet function
* @endinternal
*
* @brief   The function returns pointer to CB function in iDebug
*          DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum        - device number
* @param[out] idebugCbDbPtr - (pointer to) CB function in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbCbGet
(
    IN  GT_U8                                           devNum,
    OUT PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC           **idebugCbDbPtr
)
{
    GT_U32                                              i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC                         *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(idebugCbDbPtr);

    for (i=0; i<idebugDB->numOfFamilies; i++)
    {
        /* file is open; and also all other allocations were done */
        if (idebugDB->idebugFamilyPtrArr[i]->familyId == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            *idebugCbDbPtr = idebugDB->idebugFamilyPtrArr[i]->cbFunc;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChIdebugDbHeadGet function
* @endinternal
*
* @brief   The function returns pointer to head of MXML node
*          tree in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum          - device number
* @param[out] idebugHeadDbPtr - (pointer to) head of MXML node
*                               tree in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbHeadGet
(
    IN  GT_U8                                           devNum,
    OUT mxml_node_t                                     **idebugHeadDbPtr
)
{
    GT_U32                                              i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC                         *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(idebugHeadDbPtr);

    for (i=0; i <idebugDB->numOfFamilies; i++)
    {
        /* file is open; and also all other allocations were done */
        if (idebugDB->idebugFamilyPtrArr[i]->familyId == PRV_CPSS_PP_MAC(devNum)->devFamily)
        {
            *idebugHeadDbPtr = idebugDB->idebugFamilyPtrArr[i]->head;
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChIdebugDbInterfaceArrayGet function
* @endinternal
*
* @brief   The function returns pointer to interfaces string
*          array in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum                   - device number
* @param[out] numOfInterfacesPtr       - num of interfaces in DB
* @param[out] idebugInterfacesPtr      - (pointer to) interfaces
*                                        string array in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbInterfaceArrayGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInterfacesPtr,
    OUT GT_CHAR_PTR                                     **idebugInterfacesPtr
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(idebugInterfacesPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfInterfacesPtr);

    if (idebugDB->idebugInterfacesStrPtrArr)
    {
        *numOfInterfacesPtr = idebugDB->numOfInterfaces;
        *idebugInterfacesPtr = idebugDB->idebugInterfacesStrPtrArr;

        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChIdebugDbInstanceArrayGet function
* @endinternal
*
* @brief   The function returns pointer to instances string
*          array in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum                   - device number
* @param[out] numOfInstancesPtr       - num of interfaces in DB
* @param[out] idebugInstancesPtr      - (pointer to) instances
*                                        string array in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbInstanceArrayGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInstancesPtr,
    OUT GT_CHAR_PTR                                     **idebugInstancesPtr
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(idebugInstancesPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfInstancesPtr);

    if (idebugDB->idebugInstancesStrPtrArr)
    {
        *numOfInstancesPtr = idebugDB->numOfInstances;
        *idebugInstancesPtr = idebugDB->idebugInstancesStrPtrArr;

        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChIdebugDbFieldsArrayGet function
* @endinternal
*
* @brief   The function returns pointer to fields string
*          array in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum                   - device number
* @param[out] numOfFieldssPtr          - num of fields in DB
* @param[out] idebugFieldsPtr          - (pointer to) fields
*                                        string array in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbFieldsArrayGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfFieldsPtr,
    OUT GT_CHAR_PTR                                     **idebugFieldsPtr
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(idebugFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);

    if (idebugDB->idebugFieldsStrPtrArr)
    {
        *numOfFieldsPtr = idebugDB->numOfFields;
        *idebugFieldsPtr = idebugDB->idebugFieldsStrPtrArr;

        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChIdebugDbInterfaceGet function
* @endinternal
*
* @brief   The function returns pointer to interface structure
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] idebugInterfaceDbPtr - (pointer to) idebug
*                                   interface structure in DB
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChIdebugDbInterfaceGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR              *idebugInterfaceDbPtr
)
{

    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(idebugInterfaceDbPtr);

    if (idebugDB->idebugInterfacePtrArr[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    for (i=0; i<idebugDB->numOfInterfaces; i++)
    {
        if(cpssOsStrCmp(interfaceName,idebugDB->idebugInterfacesStrPtrArr[i])==0)
            break;
    }
    *idebugInterfaceDbPtr = &(idebugDB->idebugInterfacePtrArr[devNum][i]);

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugDbNumInterfacesGet function
* @endinternal
*
* @brief   The function returns number of interfaces
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] numOfInterfacesPtr   - (pointer to) number
*                                    of interfaces in idebug DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugDbNumInterfacesGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInterfacesPtr
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfInterfacesPtr);

    if (idebugDB->idebugInterfacesStrPtrArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    *numOfInterfacesPtr = idebugDB->numOfInterfaces;

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugDbNumInstancesGet function
* @endinternal
*
* @brief   The function returns number of instances
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] numOfInstancesPtr   - (pointer to) number
*                                    of instances in idebug DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugDbNumInstancesGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfInstancesPtr
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfInstancesPtr);

    if (idebugDB->idebugInstancesStrPtrArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    *numOfInstancesPtr = idebugDB->numOfInstances;

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugDbNumFieldsGet function
* @endinternal
*
* @brief   The function returns number of fields
*          in iDebug DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  devNum               - device number
* @param[out] numOfFieldsPtr       - (pointer to) number
*                                    of fields in idebug DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugDbNumFieldsGet
(
    IN  GT_U8                                           devNum,
    OUT GT_U32                                          *numOfFieldsPtr
)
{
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);

    if (idebugDB->idebugFieldsStrPtrArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    *numOfFieldsPtr = idebugDB->numOfFields;

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInterfaceNumFieldsGet function
* @endinternal
*
* @brief   Get interface number of fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] numOfFieldsPtr          - (pointer to) number of
*                                       fields for interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugInterfaceNumFieldsGet
(
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             interfaceName,
    OUT GT_U32                                  *numOfFieldsPtr
)
{
    GT_U32 i=0;
    mxml_node_t *node;
    const GT_CHAR *attr;
    GT_CHAR *str1;


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);

    node = mxmlFindInterfaceFirstFieldNode(devNum,interfaceName);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: failed to find interface [%s] in XML",interfaceName);
    }

    while (node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        attr = prvCpssMxmlElementGetAttr(node,"internalName");

        /* skip fields that their name starts with 'reserved_' */
        str1 = cpssOsStrStr(attr, "reserved_");
        if(str1 == NULL)
            i++;
        node = node->next;
    }

    *numOfFieldsPtr = i;

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInterfaceFieldsGet function
* @endinternal
*
* @brief   Get interface fields and sizes.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for interface
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       interface
* @param[out] fieldArr                - interface fields array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugInterfaceFieldsGet
(
    IN    GT_U8                                   devNum,
    IN    GT_CHAR_PTR                             interfaceName,
    INOUT GT_U32                                  *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_FIELD_STC          fieldsArr[]
)
{
    mxml_node_t *node;
    GT_U32 numOfFields, i=0;
    GT_STATUS rc;
    const GT_CHAR *attr;
    GT_CHAR *str1;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(*numOfFieldsPtr < numOfFields)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of fields is [%d] for interface [%s]",numOfFields,interfaceName);

    node = mxmlFindInterfaceFirstFieldNode(devNum,interfaceName);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: failed to find interface [%s] in XML",interfaceName);
    }

    while (node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        attr = prvCpssMxmlElementGetAttr(node,"internalName");

        /* skip fields that their name starts with 'reserved_' */
        str1 = cpssOsStrStr(attr, "reserved_");
        if(str1 == NULL)
        {
            cpssOsStrCpy(fieldsArr[i].fieldName,attr);
            attr = prvCpssMxmlElementGetAttr(node,"bitStop");
            fieldsArr[i].endBit = cpssOsStrTo32(attr);
            attr = prvCpssMxmlElementGetAttr(node,"bitStart");
            fieldsArr[i].startBit = cpssOsStrTo32(attr);
            i++;
        }
        node = node->next;
    }

    *numOfFieldsPtr = numOfFields;
    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSet function
* @endinternal
*
* @brief   Set interface fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] numOfFields              - (pointer to) interface
*                                       fields size
* @param[in] fieldsValueArr           - interface fields values
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_U32                                  numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[]
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 numberOfMultiplications,multiplicationIterator;

    GT_U32 regAddr,size,interfaceNumOfWords,i,tileInd=0;
    GT_STATUS rc;
    mxml_node_t *node;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    GT_U32 startBit =0 ;
    GT_CHAR polarity[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* Check if the fields are applicable for the device */
    for (i=0; i<numOfFields; i++)
    {
        rc = prvCpssDxChIdebugFieldDeviceCheck(devNum, fieldsValueArr[i].fieldName);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    node = mxmlInterfaceSizeGet(devNum,interfaceName,&size,&startBit);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] size in XML",interfaceName);
    }

    node = mxmlInterfacePolarityGet(devNum,interfaceName,polarity);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] polarity in XML",interfaceName);
    }

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
            instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }
    for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
    {
        /* select DFX pipe for Read and Write Transaction */
        rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeIdArr[multiplicationIterator]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* before each new key/mask configuration, reset access pointer */
        rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,instanceDfxBaseAddrArr[multiplicationIterator]);

        if(rc != GT_OK)
        {
            return rc;
        }

        /* select interface */
        rc = prvCpssDxChIdebugInstanceDfxInterfaceSelect(devNum,instanceDfxBaseAddrArr[multiplicationIterator],
        interfaceDfxIndexArr[multiplicationIterator],polarity);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance match key register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] + PRV_DFX_XSB_MATCH_KEY_REG_OFFSET_MAC(devNum);

        interfaceNumOfWords = LENGTH_TO_NUM_WORDS_MAC(size);
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
         maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
        }

        if(interfaceNumOfWords > maxInterfaceLength)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE,"Error: interface [%s] num of words is [%d]",interfaceName,interfaceNumOfWords);

        /* write all interface match key */
        rc = prvCpssDxChIdebugInterfaceSet(devNum,interfaceName,numOfFields,fieldsValueArr,GT_TRUE,regAddr,interfaceNumOfWords,startBit);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* before each new key/mask configuration, reset access pointer */
        rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,instanceDfxBaseAddrArr[multiplicationIterator]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance match mask register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] + PRV_DFX_XSB_MATCH_MASK_REG_OFFSET_MAC(devNum);

        /* write all interface match mask */
        rc = prvCpssDxChIdebugInterfaceSet(devNum,interfaceName,numOfFields,fieldsValueArr,GT_FALSE,regAddr,interfaceNumOfWords,startBit);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* update iDebug DB */
    if (idebugDB->idebugInterfacePtrArr[devNum] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupSingleInterfaceSet function
* @endinternal
*
* @brief   Set interface fields for single instance address.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] instanceDfxPipeId        - instance Dfx PipeId
* @param[in] instanceDfxBaseAddr      - instance Dfx BaseAddr
* @param[in] interfaceDfxIndex        - interface Dfx Index
* @param[in] numOfFields              - (pointer to) interface
*                                       fields size
* @param[in] fieldsValueArr           - interface fields values
*                                       array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupSingleInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_U32                                  instanceDfxPipeId,
    IN  GT_U32                                  instanceDfxBaseAddr,
    IN  GT_U32                                  interfaceDfxIndex,
    IN  GT_U32                                  numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC fieldsValueArr[]
)
{
    GT_U32              regAddr,size,interfaceNumOfWords,i,tileInd=0;
    GT_STATUS           rc;
    mxml_node_t         *node;
    GT_U32              maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    GT_U32              startBit =0 ;
    GT_CHAR             polarity[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_BOOL             value ;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* Check if the fields are applicable for the device */
    for (i=0; i<numOfFields; i++)
    {
        rc = prvCpssDxChIdebugFieldDeviceCheck(devNum, fieldsValueArr[i].fieldName);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    node = mxmlInterfaceSizeGet(devNum,interfaceName,&size,&startBit);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] size in XML",interfaceName);
    }

    node = mxmlInterfacePolarityGet(devNum,interfaceName,polarity);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] polarity in XML",interfaceName);
    }

    rc = prvCpssDxChIdebugPortGroupSingleInterfaceSamplingEnableGet(devNum ,interfaceName,instanceDfxPipeId,instanceDfxBaseAddr,&value);
    if (rc != GT_OK) {
        return rc;
    }
    if (value == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: Can't change configuration while sampling is enabled for interface [%s]",interfaceName);
    }

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* before each new key/mask configuration, reset access pointer */
    rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,instanceDfxBaseAddr);

    if(rc != GT_OK)
    {
        return rc;
    }

    /* select interface */
    rc = prvCpssDxChIdebugInstanceDfxInterfaceSelect(devNum,instanceDfxBaseAddr,
    interfaceDfxIndex,polarity);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance match key register address */
    regAddr = instanceDfxBaseAddr + PRV_DFX_XSB_MATCH_KEY_REG_OFFSET_MAC(devNum);

    interfaceNumOfWords = LENGTH_TO_NUM_WORDS_MAC(size);
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
     maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    }

    if(interfaceNumOfWords > maxInterfaceLength)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE,"Error: interface [%s] num of words is [%d]",interfaceName,interfaceNumOfWords);

    /* write all interface match key */
    rc = prvCpssDxChIdebugInterfaceSet(devNum,interfaceName,numOfFields,fieldsValueArr,GT_TRUE,regAddr,interfaceNumOfWords,startBit);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* before each new key/mask configuration, reset access pointer */
    rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,instanceDfxBaseAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance match mask register address */
    regAddr = instanceDfxBaseAddr + PRV_DFX_XSB_MATCH_MASK_REG_OFFSET_MAC(devNum);

    /* write all interface match mask */
    rc = prvCpssDxChIdebugInterfaceSet(devNum,interfaceName,numOfFields,fieldsValueArr,GT_FALSE,regAddr,interfaceNumOfWords,startBit);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* update iDebug DB */
    if (idebugDB->idebugInterfacePtrArr[devNum] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceGet function
* @endinternal
*
* @brief   Get interface fields.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for interface
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       interface
* @param[out] fieldsValueArr          - interface fields values
*                                       array (Data and Mask)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupsBmp,
    IN GT_CHAR_PTR                                  interfaceName,
    INOUT GT_U32                                    *numOfFieldsPtr,
    OUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC    fieldsValueArr[]
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 regAddr,size,interfaceNumOfWords,tileInd=0;
    GT_STATUS rc;
    mxml_node_t *node;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    GT_U32 numOfFields,startBit =0;
    GT_U32 i,numberOfMultiplications;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC *fieldsArr = NULL;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    node = mxmlInterfaceSizeGet(devNum,interfaceName,&size,&startBit);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] size in XML",interfaceName);
    }

    rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (numOfFields > *numOfFieldsPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: for interface [%s] num of fields [%d]",interfaceName,numOfFields);
    }

    *numOfFieldsPtr = numOfFields;

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
            instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,*instanceDfxPipeIdArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* before each new key/mask configuration, reset access pointer */
    rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,*instanceDfxBaseAddrArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance general configuration register address */
    regAddr = *instanceDfxBaseAddrArr + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);


    /* select interface */
    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,11,5,*interfaceDfxIndexArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance match key register address */
    regAddr = *instanceDfxBaseAddrArr + PRV_DFX_XSB_MATCH_KEY_REG_OFFSET_MAC(devNum);

    interfaceNumOfWords = LENGTH_TO_NUM_WORDS_MAC(size);
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
     maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    }

    if(interfaceNumOfWords > maxInterfaceLength)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE,"Error: interface [%s] num of words is [%d]",interfaceName,interfaceNumOfWords);

    fieldsArr = (PRV_CPSS_DXCH_IDEBUG_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (fieldsArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "Error: failed to allocate fieldsArr for interface [%s]",interfaceName);
    }

    rc = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
    if(rc != GT_OK)
    {
        cpssOsFree(fieldsArr);
        return rc;
    }

    for (i=0; i<numOfFields; i++)
    {
        cpssOsStrCpy(fieldsValueArr[i].fieldName,fieldsArr[i].fieldName);
    }

    /* read all interface match key */
    rc = prvCpssDxChIdebugInterfaceGet(devNum,interfaceName,*numOfFieldsPtr,fieldsValueArr,GT_TRUE,regAddr,interfaceNumOfWords,startBit);
    if(rc != GT_OK)
    {
        cpssOsFree(fieldsArr);
        return rc;
    }

    /* before each new key/mask configuration, reset access pointer */
    rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,*instanceDfxBaseAddrArr);
    if(rc != GT_OK)
    {
        cpssOsFree(fieldsArr);
        return rc;
    }

    /* select DFX instance match mask register address */
    regAddr = *instanceDfxBaseAddrArr + PRV_DFX_XSB_MATCH_MASK_REG_OFFSET_MAC(devNum);

    /* read all interface match mask */
    rc = prvCpssDxChIdebugInterfaceGet(devNum,interfaceName,*numOfFieldsPtr,fieldsValueArr,GT_FALSE,regAddr,interfaceNumOfWords,startBit);
    if(rc != GT_OK)
    {
        cpssOsFree(fieldsArr);
        return rc;
    }

    cpssOsFree(fieldsArr);

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceReset function
* @endinternal
*
* @brief   Reset interface in hardware.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceReset
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  GT_CHAR_PTR                             interfaceName
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 numberOfMultiplications,multiplicationIterator;

    GT_U32 regAddr,size,interfaceNumOfWords,i,interfaceStartBit=0;
    GT_STATUS rc;
    mxml_node_t *node;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    GT_CHAR polarity[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    node = mxmlInterfaceSizeGet(devNum,interfaceName,&size,&interfaceStartBit);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] size in XML",interfaceName);
    }

    node = mxmlInterfacePolarityGet(devNum,interfaceName,polarity);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] polarity in XML",interfaceName);
    }

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);

    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }
    for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
    {
        /* select DFX pipe for Read and Write Transaction */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;
        rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1 << instanceDfxPipeIdArr[multiplicationIterator]));
        if(rc != GT_OK)
        {
            return rc;
        }

        /* before each new key/mask configuration, reset access pointer */
        rc = prvCpssDxChIdebugInstanceDfxAccessPointerReset(devNum,instanceDfxBaseAddrArr[multiplicationIterator]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select interface */
        rc = prvCpssDxChIdebugInstanceDfxInterfaceSelect(devNum,instanceDfxBaseAddrArr[multiplicationIterator],
        interfaceDfxIndexArr[multiplicationIterator],polarity);
        if(rc != GT_OK)
        {
            return rc;
        }

       /* select DFX instance match mask register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] + PRV_DFX_XSB_MATCH_MASK_REG_OFFSET_MAC(devNum);

        /* write 'don't care' to all interface match mask */
        interfaceNumOfWords = LENGTH_TO_NUM_WORDS_MAC(size);
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
           maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
        }

        if(interfaceNumOfWords > maxInterfaceLength)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE,"Error: interface [%s] num of words is [%d]",interfaceName,interfaceNumOfWords);

        for(i=0; i<interfaceNumOfWords; i++)
        {
            rc = cpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,0xFFFFFFFF);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
     }

    return GT_OK;
}


/**
* @internal prvCpssDxChIdebugPortGroupInterfaceResetAll function
* @endinternal
*
* @brief   Reset all interface in hardware.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceResetAll
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp
)
{
    GT_STATUS rc;
    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    for(i=0; i<idebugDB->numOfInterfaces; i++)
    {
        rc =prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,idebugDB->idebugInterfacesStrPtrArr[i],GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,portGroupsBmp,idebugDB->idebugInterfacesStrPtrArr[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChIdebugPortGroupInterfaceActionSet
*           function
* @endinternal
*
* @brief   Set interface action configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] actionPtr                - (pointer to) interface
*                                       action configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceActionSet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC            *actionPtr
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 numberOfMultiplications,multiplicationIterator;
    GT_U32 regAddr,value,tileInd=0;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    rc = prvCpssDxChIdebugActionCheck(actionPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* select DFX pipe for Read and Write Transaction */
     rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,instanceDfxPipeIdArr,
        instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }
    for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
    {
        /* select DFX pipe for Read and Write Transaction */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;
        rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1 << instanceDfxPipeIdArr[multiplicationIterator]));
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance general configuration register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] +
                PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        U32_SET_FIELD_MASKED_MAC(value,0,1,BOOL2BIT_MAC(actionPtr->samplingEnable));
        U32_SET_FIELD_MASKED_MAC(value,1,4,actionPtr->matchCounterIntThresh.lsb);
        U32_SET_FIELD_MASKED_MAC(value,5,4,actionPtr->matchCounterIntThresh.msb);
        U32_SET_FIELD_MASKED_MAC(value,9,1,actionPtr->matchCounterIntThresh.middle);
        U32_SET_FIELD_MASKED_MAC(value,10,1,(actionPtr->samplingMode == CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ? 1 : 0));
        U32_SET_FIELD_MASKED_MAC(value,16,1,BOOL2BIT_MAC(actionPtr->inverseEnable));
        rc = cpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,value);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance interrupt mask register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] +
            PRV_DFX_XSB_INT_MASK_REG_OFFSET_MAC(devNum);

        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        U32_SET_FIELD_MASKED_MAC(value,1,1,BOOL2BIT_MAC(actionPtr->interruptMatchCounterEnable));
        U32_SET_FIELD_MASKED_MAC(value,2,1,BOOL2BIT_MAC(actionPtr->interruptSamplingEnable));

        rc = cpssDrvHwPpResetAndInitControllerWriteReg(devNum,regAddr,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceActionGet
*           function
* @endinternal
*
* @brief   Get interface action configuration.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[out] actionPtr               - (pointer to) interface
*                                       action configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceActionGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT CPSS_DXCH_PACKET_ANALYZER_ACTION_STC            *actionPtr
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];

    GT_U32 regAddr,value;
    GT_STATUS rc;
    GT_U32 numOfMultiplications;
    GT_U32 tileInd=0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS,
            instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX pipe for Read and Write Transaction */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;
    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1<< instanceDfxPipeIdArr[0]));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance general configuration register address */
    regAddr = instanceDfxBaseAddrArr[0] + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

    rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    actionPtr->samplingEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value,0,1));
    actionPtr->matchCounterIntThresh.lsb = U32_GET_FIELD_MAC(value,1,4);
    actionPtr->matchCounterIntThresh.msb = U32_GET_FIELD_MAC(value,5,4);
    actionPtr->matchCounterIntThresh.middle = U32_GET_FIELD_MAC(value,9,1);
    actionPtr->samplingMode = (U32_GET_FIELD_MAC(value,10,1) == 1 ? CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E : CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E);
    actionPtr->inverseEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value,16,1));

    /* select DFX instance interrupt mask register address */
    regAddr = instanceDfxBaseAddrArr[0] + PRV_DFX_XSB_INT_MASK_REG_OFFSET_MAC(devNum);

    rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,&value);
    if(rc != GT_OK)
    {
        return rc;
    }

    actionPtr->interruptMatchCounterEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value,1,1));
    actionPtr->interruptSamplingEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value,2,1));

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet
*           function
* @endinternal
*
* @brief   Get number of hits for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] metaData                 - search attribute data
*                                       (Applicable device:Falcon; AC5P)
* @param[in] interfaceName            - interface name
* @param[in] clearOnReadEnable        - enable/disable clear
*                                       on read for counter data
* @param[out] valuePtr                - (pointer to) number of
*                                       counter hits.
* @param[out] multndPtr               - (pointer to) in case of multiplied interface this variable holds
*                                       the index of multiplication that has a hit count greater then 0.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet
(
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          metaData,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_BOOL                                         clearOnReadEnable,
    OUT GT_U32                                          *valuePtr,
    OUT GT_U32                                          *multndPtr
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceMetaDataArr   [MAX_MULTIPLICATIONS_NUM];

    GT_STATUS rc;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR  idebugInterfaceDb;
    GT_U32       numberOfMultiplications,multiplicationIterator;
    GT_BOOL     hitDetected= GT_FALSE;
    GT_U32      hitPerMultiplication[MAX_MULTIPLICATIONS_NUM];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    /*CPSS_NULL_PTR_CHECK_MAC(multndPtr);*/
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    cpssOsMemSet(instanceMetaDataArr,0,sizeof(GT_U32)*MAX_MULTIPLICATIONS_NUM);
    cpssOsMemSet(hitPerMultiplication,0,sizeof(GT_U32)*MAX_MULTIPLICATIONS_NUM);

     /* select DFX pipe for Read and Write Transaction */
     rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,instanceMetaDataArr);
     if(rc != GT_OK)
     {
         return rc;
     }

    *valuePtr = 0;

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, interfaceName);
    if (rc != GT_OK) {
        return rc;
    }
    for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
    {
        /* select DFX pipe for Read and Write Transaction */
        rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeIdArr[multiplicationIterator]);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChIdebugPortGroupInterfaceReadMatchCounter(devNum,
          instanceDfxBaseAddrArr[multiplicationIterator],
          &(hitPerMultiplication[multiplicationIterator]));

        if (rc != GT_OK)
        {
            return rc;
        }

        if(hitPerMultiplication[multiplicationIterator]>0)
        {
            if(multndPtr)
            {
                *multndPtr = multiplicationIterator;
                hitDetected = GT_TRUE;
            }
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_FALSE)
        {
            *valuePtr+=(hitPerMultiplication[multiplicationIterator]);
        }

    }

    /* for SIP5 devices counter is reset when SW reads its value */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) && clearOnReadEnable == GT_FALSE)
    {
        /* get pointer to head of MXML node tree from iDebug DB */
        rc = prvCpssDxChIdebugDbInterfaceGet(devNum,interfaceName,&idebugInterfaceDb);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_FALSE)
        {
            idebugInterfaceDb->matchCounterValue[0] += *valuePtr;
             *valuePtr = idebugInterfaceDb->matchCounterValue[0];
        }
        else
        {

           for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
           {
             idebugInterfaceDb->matchCounterValue[multiplicationIterator]+=hitPerMultiplication[multiplicationIterator];
             idebugInterfaceDb->matchCounterMetaData[multiplicationIterator] = instanceMetaDataArr[multiplicationIterator];

             if(metaData!=META_DATA_ALL_CNS)
             {
                  if(prvCpssDxChIdebugCompareMetaData(idebugInterfaceDb->matchCounterMetaData[multiplicationIterator],metaData))
                  {
                    *valuePtr = idebugInterfaceDb->matchCounterValue[multiplicationIterator];
                    if (multndPtr)
                    {
                        *multndPtr = multiplicationIterator;
                        hitDetected = GT_TRUE;
                    }
                  }
             }
             else
             {
                        *valuePtr +=idebugInterfaceDb->matchCounterValue[multiplicationIterator];
                    }
             }

        }

        if( GT_TRUE == hitDetected)
        {
            idebugInterfaceDb->multInd = *multndPtr;
        }
    }
    else if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) && clearOnReadEnable == GT_TRUE)
    {
        /* get pointer to head of MXML node tree from iDebug DB */
        rc = prvCpssDxChIdebugDbInterfaceGet(devNum,interfaceName,&idebugInterfaceDb);
        if (rc != GT_OK)
        {
            return rc;
        }

        for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
        {
          idebugInterfaceDb->matchCounterValue[multiplicationIterator] = 0;
        }

    }

    return GT_OK;

}

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet
*           function
* @endinternal
*
* @brief   Get sampled data all fields for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*                                       (APPLICABLE DEVICES: SIP6)
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for interface
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       interface
* @param[out] fieldsValueArr          - interface fields values
*                                       array.
*                                       note:
*                                       fieldsValueArr[].mask is
*                                       not applicable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC      fieldsValueArr[]
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];

    GT_U32 regAddr,size,i,j,tileInd=0;
    GT_U32 offset,length;
    GT_STATUS rc = GT_OK;
    GT_U32 interfaceNumOfWords, fieldNumOfWords, numOfFields,interfaceStartBit=0;
    GT_U32 *sampledArr = NULL;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC *fieldsArr = NULL;
    mxml_node_t *node;
    PRV_CPSS_DXCH_IDEBUG_CB_FUNCTIONS_STC* cbPtr;
    GT_U32 numOfMultiplications,usedIndex;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR  idebugInterfaceDb;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* update iDebug DB */
    if (idebugDB->idebugInterfacePtrArr[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: iDebug DB for device [%d] and interface [%s] is not allocated",devNum,interfaceName);

    /* get number of fields for interface */
    rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(*numOfFieldsPtr < numOfFields)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE,"Error: num of fields is [%d] for interface [%s]",numOfFields,interfaceName);

    *numOfFieldsPtr = numOfFields;

    /* allocated array of fields */
    fieldsArr = (PRV_CPSS_DXCH_IDEBUG_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*idebugDB->numOfFields);
    if (fieldsArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocated arrays for device [%d]",devNum);
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(fieldsArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*idebugDB->numOfFields);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
         maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    }

    sampledArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxInterfaceLength);
    if (sampledArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        CPSS_LOG_ERROR_MAC("Error: failed to allocated arrays for device [%d]",devNum);
        goto exit_cleanly_lbl;
    }
    cpssOsMemSet(sampledArr,0,sizeof(GT_U32)*maxInterfaceLength);

    /* get interface fields */
    rc = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    node = mxmlInterfaceSizeGet(devNum,interfaceName,&size,&interfaceStartBit);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] size in XML",interfaceName);
    }

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    rc = prvCpssDxChIdebugDbInterfaceGet(devNum,interfaceName,&idebugInterfaceDb);
    if (rc != GT_OK)
    {
        return rc;
    }

    usedIndex = idebugInterfaceDb->multInd;

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeIdArr[usedIndex]);

    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance match sample register address */
    regAddr = instanceDfxBaseAddrArr[usedIndex] +
        PRV_DFX_XSB_MATCH_SAMPLE_REG_OFFSET_MAC(devNum);

    cpssOsMemSet(sampledArr,0,sizeof(GT_U32)*maxInterfaceLength);

    /* read all interface match sample */
    interfaceNumOfWords = LENGTH_TO_NUM_WORDS_MAC(size);
    if(interfaceNumOfWords > maxInterfaceLength)
    {
        rc = GT_BAD_SIZE;
        CPSS_LOG_ERROR_MAC("Error: interface [%s] num of words is [%d]",interfaceName,interfaceNumOfWords);
        goto exit_cleanly_lbl;
    }

    for(i=0; i<interfaceNumOfWords; i++)
    {
        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,(regAddr+(i%CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS)*4),&(sampledArr[i]));
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        if(i==CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS-1)
        {
            rc = prvCpssDxChIdebugPortGroupInterfaceUpperMatchEnableSet(devNum,instanceDfxBaseAddrArr[usedIndex],GT_TRUE);
            if(rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
    }

    if(i>=(CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS-1))
    {
        rc = prvCpssDxChIdebugPortGroupInterfaceUpperMatchEnableSet(devNum,instanceDfxBaseAddrArr[usedIndex],GT_FALSE);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
    }

    /* return all fields data */
    for(i=0; i<numOfFields; i++)
    {
        cpssOsStrCpy(fieldsValueArr[i].fieldName,fieldsArr[i].fieldName);

        fieldNumOfWords = LENGTH_TO_NUM_WORDS_MAC(fieldsArr[i].endBit-fieldsArr[i].startBit+ 1) ;
        if(fieldNumOfWords > CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS)
        {
            rc = GT_BAD_SIZE;
            CPSS_LOG_ERROR_MAC("Error: in interface [%s], num of words for field [%s] is [%d]",
                               interfaceName,fieldsArr[i].fieldName,fieldNumOfWords);
            goto exit_cleanly_lbl;
        }

        size = fieldsArr[i].endBit-fieldsArr[i].startBit;
        length = LENGTH_TO_NUM_BITS_MAC(size);
        offset = fieldsArr[i].startBit;

        for(j=0; j<fieldNumOfWords; j++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(sampledArr,offset+interfaceStartBit,length,fieldsValueArr[i].data[j]);
            offset +=32;
            size -=32;
            length = LENGTH_TO_NUM_BITS_MAC(size);
        }
    }

    /* return all fields validity */
    for(i=0; i<numOfFields; i++)
    {
        /* update validity for field */
        rc = prvCpssDxChIdebugDbCbGet(devNum, &cbPtr);
        if(rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
        rc = cbPtr->fieldValidityLogicUpdateCbFunc(devNum,interfaceName,fieldsArr[i].fieldName,numOfFields,fieldsValueArr);
        if(rc != GT_OK)
        {
            if(GT_TRUE==prvCpssDxChIdebugFailOnUnfoundToken(devNum))
            {
                goto exit_cleanly_lbl;
            }
            rc = GT_OK;
        }
    }


exit_cleanly_lbl:

    if (fieldsArr != NULL)
    {
        cpssOsFree(fieldsArr);
    }

    cpssOsFree(sampledArr);

    return rc;
}

/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet
*           function
* @endinternal
*
* @brief   Get sampled data for requested list of fields for
*          interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*                                       on read for sample data
*                                       (APPLICABLE DEVICES: SIP6)
* @param[in] numOfFields              - number of fields
*                                       requested for interface
* @param[in,out] fieldsValueArr       - in: list of requested
*                                       fields for interface
*                                       out: interface requested
*                                       fields values array
*                                       note:
*                                       fieldsValueArr[].mask is
*                                       not applicable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note clearOnRead will clear values on all interface fields.
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName,
    IN    GT_U32                                        numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC      fieldsValueArr[]
)
{
    GT_U32 interfaceNumOfFields,i,j,tileInd=0;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC *allFieldsValueArr = NULL;
    GT_BOOL found;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get number of fields for interface */
    rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&interfaceNumOfFields);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(interfaceNumOfFields < numOfFields)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "ERROR: interface [%s] interfaceNumOfFields [%d] is smaller then numOfFields [%d]",
                                      interfaceName,interfaceNumOfFields,numOfFields);
    }

    /* array allocation */
    allFieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*idebugDB->numOfFields);
    if (allFieldsValueArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"Error: failed to allocated array for device [%d]",devNum);

    cpssOsMemSet(allFieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*idebugDB->numOfFields);

    /* get sampled data all fields for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,&interfaceNumOfFields,allFieldsValueArr);
    if(rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for(i=0; i<numOfFields; i++)
    {
        found = GT_FALSE;
        /* copy fields values to output array */
        for(j=0; j<interfaceNumOfFields; j++)
        {
            if(cpssOsStrCmp(fieldsValueArr[i].fieldName,allFieldsValueArr[j].fieldName)==0)
            {
                cpssOsMemCpy(&(fieldsValueArr[i]), &(allFieldsValueArr[j]), sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC));
                found = GT_TRUE;
                break;
            }
        }
        if (found == GT_FALSE)
        {
            rc = GT_BAD_PARAM;
            CPSS_LOG_ERROR_MAC("Error: field [%s] wasn't found in interface [%s]", fieldsValueArr[i].fieldName, interfaceName);
            goto exit_cleanly_lbl;
        }

    }

exit_cleanly_lbl:

    if (allFieldsValueArr != NULL)
    {
        cpssOsFree(allFieldsValueArr);
    }

    return rc;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet
*           function
* @endinternal
*
* @brief   Enable/Disable sampling for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[in] enable                   - enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN GT_CHAR_PTR                                      interfaceName,
    IN GT_BOOL                                          enable
)
{
    GT_U32 regAddr,value,tileInd=0;
    GT_STATUS rc;
    GT_U32 numOfMultiplications,multiplicationIterator;
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    /* avoid warnings */
    portGroupsBmp = portGroupsBmp;

    value = BOOL2BIT_MAC(enable);

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }


    for(multiplicationIterator =0;multiplicationIterator<numOfMultiplications;multiplicationIterator++)
    {

        /* select DFX pipe for Read and Write Transaction */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;
        rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1 << instanceDfxPipeIdArr[multiplicationIterator]));
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance general configuration register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] +
            PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

        rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,1,value);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;

}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet
*           function
* @endinternal
*
* @brief   Get sampling status (enable/disable) for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
* @param[out] enablePtr               - (pointer to)
*                                       enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN  GT_CHAR_PTR                                      interfaceName,
    OUT GT_BOOL                                          *enablePtr
)
{
    GT_U32 numOfMultiplications,multiplicationIterator;
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_BOOL enableArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 regAddr,value,tileInd=0;
    GT_STATUS rc;

    cpssOsMemSet(enableArr, 0, sizeof(enableArr));

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,
        PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS,instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(multiplicationIterator=0;multiplicationIterator<numOfMultiplications;multiplicationIterator++)
    {
        /* select DFX pipe for Read and Write Transaction */
        rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeIdArr[multiplicationIterator]);

        if(rc != GT_OK)
        {
            return rc;
        }

        if(portGroupsBmp==CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        {
            tileInd = multiplicationIterator;
        }


        /* select DFX instance general configuration register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] +
            PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

        rc = cpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr,0,1,&value);
        if(rc != GT_OK)
        {
            return rc;
        }

        enableArr[multiplicationIterator] = BIT2BOOL_MAC(value);
    }


    /*chek that all is synced*/
    for(multiplicationIterator=1;multiplicationIterator<numOfMultiplications;multiplicationIterator++)
    {
        if(enableArr[0] !=enableArr[multiplicationIterator])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: not all multiplication are synced ");
        }
    }

    *enablePtr = enableArr[0];
    return GT_OK;

}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSampleDump
*           function
* @endinternal
*
* @brief   Dump sample result for interface
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum              -  device number
* @param[in] metaData            -  info that contain tile/cp/dp (Applicable device:Falcon)
* @param[in] interfaceName       -  interface name
* @param[in] instanceName        -  instance name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleDump
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      metaData,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_CHAR_PTR                             instanceName
)
{
    GT_U32 interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32 instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_CHAR instanceIdArr[MAX_MULTIPLICATIONS_NUM][CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32 regAddr,size,i,j,k=0,index=0,interfaceDfxIndex;
    GT_U32 startBit,endBit;
    GT_STATUS rc = GT_OK;
    GT_U32 numOfFields,fieldNumOfWords;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC *fieldsValueArr = NULL;
    GT_U32  numOfMultiplications;
    GT_BOOL found = GT_FALSE;
    mxml_node_t *node;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_PTR  idebugInterfaceDb;
    GT_U32 saveIndex;
    GT_U32 maxInterfaceLength = CPSS_DXCH_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    GT_U32 interfaceNumOfWords;
    GT_U32 *sampledArr = NULL;
    GT_U32  interfaceStartBit=0;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC action;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    PRV_CPSS_PACKET_ANALYZER_INSTANCE_ID_CHECK_MAC(devNum,instanceName);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();
    PRV_CPSS_PACKET_ANALYZER_DFX_POWER_SAVE_CHECK_MAC();

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }

    node = mxmlInterfaceSizeGet(devNum,interfaceName,&size,&interfaceStartBit);
    if(!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] size in XML",interfaceName);
    }

    /* check if interface is multiple */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,
           PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS,instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChIdebugDbInterfaceGet(devNum,interfaceName,&idebugInterfaceDb);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_FALSE)
    {

        for(i=0; i<numOfMultiplications; i++)
        {

            node = mxmlInstanceInfoGet(devNum,interfaceName,i,instanceIdArr[i],&(instanceDfxPipeIdArr[i]),&(instanceDfxBaseAddrArr[i]),&(interfaceDfxIndexArr[i]),NULL);
            if(!node)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: instance info for interface [%s] was't found in XML",interfaceName);
            }
        }

         /* check that instanceName is attached to instanceName */
         for(i=0; i<numOfMultiplications; i++)
         {
             if (cpssOsStrCmp(instanceIdArr[i],instanceName) == 0)
             {
                 found = GT_TRUE;
                 index = i;
                 break;
             }
         }

          if (found == GT_FALSE)
          {
              cpssOsPrintf("Error: interface [%s] isn't attached to instance [%s]",interfaceName,instanceName);
              return GT_OK;
          }
     }
     else
     {

        if(META_DATA_ALL_CNS==metaData)
        {
            index = idebugInterfaceDb->multInd;
        }
        else
        {
            for(i=0;i<numOfMultiplications;i++)
            {
                if((idebugInterfaceDb->matchCounterMetaData[i]&(1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PER_DP_OFFSET_CNS))==0)
                {
                   cpssOsPrintf("\nWarning: Not all counters read for %s.Found unread counter %d.\n",interfaceName,i);
                   cpssOsPrintf("Read counters first\n");
                   return GT_OK;
                }

                if(prvCpssDxChIdebugCompareMetaData(idebugInterfaceDb->matchCounterMetaData[i],metaData))
                {
                    index = i;
                    break;
                }
            }
        }
     }

    /* select DFX pipe for Read and Write Transaction */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnits.DFXServerRegs.pipeSelect;
    rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,0,8,(1 << instanceDfxPipeIdArr[index]));
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX instance general configuration register address */
    regAddr = instanceDfxBaseAddrArr[index] + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

    /* get interface */
    rc = cpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr,11,5,&interfaceDfxIndex);

    if (interfaceDfxIndex != interfaceDfxIndexArr[index])
    {
        cpssOsPrintf("Error: interface [%s] isn't valid in instance [%s]",interfaceName,instanceName);
        return GT_OK;
    }

    fieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*idebugDB->numOfFields);
    if (fieldsValueArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"Error: failed to allocated arrays for device [%d]",devNum);
    }
    cpssOsMemSet(fieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*idebugDB->numOfFields);

    numOfFields = idebugDB->numOfFields;
    rc = prvCpssDxChIdebugPortGroupInterfaceGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceName,&numOfFields,fieldsValueArr);
    if (rc != GT_OK)
    {
        cpssOsFree(fieldsValueArr);
        return rc;
    }

    cpssOsPrintf("\nSize of interface %s in instance %s is %d\n",interfaceName,instanceName,size);
    cpssOsPrintf("\nDFX address is 0x%x multiplication index %d \n",instanceDfxBaseAddrArr[index],index);
    cpssOsPrintf("\nCONFIGURATION:\n");
    cpssOsPrintf("\nField name              \t    key\t\t\t\t\t\t\tmask:\n");
    cpssOsPrintf("----------------------------------------------------------------------------------------------------------\n");
    for (i=0; i<numOfFields; i++)
    {
        if (fieldsValueArr[i].isValid)
        {
            node = mxmlInterfaceFieldSizeGet(devNum,interfaceName,fieldsValueArr[i].fieldName,&startBit,&endBit);
            if(!node)
            {
                cpssOsFree(fieldsValueArr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get field [%s] size in XML",fieldsValueArr[i].fieldName);
            }
            fieldNumOfWords = LENGTH_TO_NUM_WORDS_MAC(endBit-startBit+1);
            found = GT_FALSE;
            for(j=0; j<fieldNumOfWords; j++)
            {
                if (fieldsValueArr[i].data[j] != 0)
                {
                    found = GT_TRUE;
                    break;
                }
            }
            if (found)
            {
                cpssOsPrintf(".%-3d %-30s",k+1,fieldsValueArr[i].fieldName);
                k++;
                for(j=0; j<fieldNumOfWords; j++)
                    cpssOsPrintf(" 0x%-10x",fieldsValueArr[i].data[j]);
                switch (fieldNumOfWords)
                {
                case 1:
                    cpssOsPrintf("%-39s"," ");
                    break;
                case 2:
                    cpssOsPrintf("%-26s"," ");
                    break;
                case 3:
                    cpssOsPrintf("%-13s"," ");
                    break;
                default:
                    /* do nothing */
                    break;
                }
                for(j=0; j<fieldNumOfWords; j++)
                    cpssOsPrintf(" 0x%-10x",fieldsValueArr[i].mask[j]);
                cpssOsPrintf("\n");
            }
        }
    }

    rc = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceName,&action);
    if (rc != GT_OK)
    {
        cpssOsFree(fieldsValueArr);
        return rc;
    }

    cpssOsPrintf("\nsampling mode        inverse enable:\n");
    cpssOsPrintf("--------------------------------------------------------\n");
    cpssOsPrintf("%-25s",(action.samplingMode == CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ? "first-match" : "last-match"));
    cpssOsPrintf("%-10s",(action.inverseEnable == GT_TRUE ? "true" : "false"));
    cpssOsPrintf("\n");

    cpssOsMemSet(fieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*idebugDB->numOfFields);

    /* TBD: return last interface with hit for multi interfaces untill support for Falcon */
    numOfFields = idebugDB->numOfFields;
    saveIndex = idebugInterfaceDb->multInd;
    idebugInterfaceDb->multInd = index;
    rc = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceName,&numOfFields,fieldsValueArr);
    if (rc != GT_OK)
    {
        cpssOsFree(fieldsValueArr);
        return rc;
    }

    cpssOsPrintf("\nSAMPLING:\n");
    cpssOsPrintf("\nField name              \tstart-bit  end-bit    value:\n");
    cpssOsPrintf("-------------------------------------------------------------------------\n");
    for (i=0; i<numOfFields; i++)
    {
        if (fieldsValueArr[i].isValid)
        {
            node = mxmlInterfaceFieldSizeGet(devNum,interfaceName,fieldsValueArr[i].fieldName,&startBit,&endBit);
            if(!node)
            {
                cpssOsFree(fieldsValueArr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get field [%s] size in XML",fieldsValueArr[i].fieldName);
            }
            cpssOsPrintf(".%-3d %-30s %-8d %-8d",i+1,fieldsValueArr[i].fieldName,startBit,endBit);
            fieldNumOfWords = LENGTH_TO_NUM_WORDS_MAC(endBit-startBit+1);
            for(j=0; j<fieldNumOfWords; j++)
                cpssOsPrintf(" 0x%-10x",fieldsValueArr[i].data[j]);
            cpssOsPrintf("\n");
        }
    }

    idebugInterfaceDb->multInd = saveIndex;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
         maxInterfaceLength = CPSS_DXCH_SIP_6_MAX_INTERFACE_LENGTH_IN_WORDS_CNS;
    }
    sampledArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*maxInterfaceLength);
    if (sampledArr == NULL)
    {
        cpssOsFree(fieldsValueArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to allocated arrays for device [%d]",devNum);
    }
    cpssOsMemSet(sampledArr,0,sizeof(GT_U32)*maxInterfaceLength);

    /* select DFX instance match sample register address */
    regAddr = instanceDfxBaseAddrArr[index] +
        PRV_DFX_XSB_MATCH_SAMPLE_REG_OFFSET_MAC(devNum);

    /* read all interface match sample */
    interfaceNumOfWords = LENGTH_TO_NUM_WORDS_MAC(size);

    for(i=0; i<interfaceNumOfWords; i++)
    {
        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,(regAddr+(i%CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS)*4),&(sampledArr[i]));
        if(rc != GT_OK)
        {
            cpssOsFree(fieldsValueArr);
            cpssOsFree(sampledArr);
        }

        if(i==CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS-1)
        {
            rc = prvCpssDxChIdebugPortGroupInterfaceUpperMatchEnableSet(devNum,instanceDfxBaseAddrArr[index],GT_TRUE);
            if(rc != GT_OK)
            {
               cpssOsFree(fieldsValueArr);
               cpssOsFree(sampledArr);
            }
        }
    }

    if(i>=(CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS-1))
    {
        rc = prvCpssDxChIdebugPortGroupInterfaceUpperMatchEnableSet(devNum,instanceDfxBaseAddrArr[index],GT_FALSE);
        if(rc != GT_OK)
        {
            cpssOsFree(fieldsValueArr);
            cpssOsFree(sampledArr);
        }
    }

    if(interfaceStartBit)
    {
        cpssOsPrintf("\nConcatinated interface . Interface start bit :%d.\n",interfaceStartBit);
    }

    cpssOsPrintf("\nNative words:\n");
    cpssOsPrintf("------------------\n");
    for (i=0; i<interfaceNumOfWords; i++)
    {
        cpssOsPrintf(".%-3d 0x%x    ",i+1,(regAddr+(i%CPSS_DXCH_MAX_INTERFACE_WRAPAROUND_IN_WORDS_CNS)*4));
        cpssOsPrintf(" 0x%x\n",sampledArr[i]);
    }

    cpssOsFree(fieldsValueArr);
    cpssOsFree(sampledArr);

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceReadMatchCounter
*           function
* @endinternal
*
* @brief   Low level function to get number of hits for interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                  - device number
* @param[in] instanceDfxBaseAddr     - DFX XSB unit base address
* @param[out] valuePtr               - (pointer to) number of
*                                      counter hits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceReadMatchCounter
(
    IN    GT_U8       devNum,
    IN    GT_U32      instanceDfxBaseAddr,
    OUT   GT_U32      *valuePtr
)
{   GT_U32    regAddr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* select DFX instance match counter address */
    regAddr = instanceDfxBaseAddr +PRV_DFX_XSB_MATCH_COUNTER_REG_OFFSET_MAC(devNum);

    /* read match counter; in SIP5 devices counter is reset when sw reads it's value */
    rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,valuePtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



/**
* @internal prvCpssDxChIdebugIsMultipleInterface
*           function
* @endinternal
*
* @brief   The function return the number of multiplications
*          (for example if same descriptor exist per Data path) of descriptor.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] numOfMultiplicationsPtr   (pointer to) the number
*                                       of multiplications
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugIsMultipleInterface
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *numOfMultiplicationsPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(numOfMultiplicationsPtr);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
    {
        if (cpssOsStrCmp(interfaceName,"rxdma2ctrl_pipe_desc") == 0)
        {
            *numOfMultiplicationsPtr = 4;
        }
        else
        {
            *numOfMultiplicationsPtr = 1;
        }
    }
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
    {
        if ((cpssOsStrCmp(interfaceName,"RXDMA_2_CTRL_PIPE") == 0) ||
            (cpssOsStrCmp(interfaceName,"txdma2ha_desc") == 0) )
        {
            *numOfMultiplicationsPtr = 3;
        }
        else
        {
            *numOfMultiplicationsPtr = 1;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChIdebugMultipleInterfaceByIndexGet
*           function
* @endinternal
*
* @brief   The function return the bus and mux select of specific multiplication index.
*             (used only for indexes greater then 0).
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[out] numOfMultiplicationsPtr - (pointer to) the number
*                                       of multiplications
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugMultipleInterfaceByIndexGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_U32                                          multiplicationIndex,
    OUT GT_CHAR                                         *busPtr,
    OUT GT_U32                                          *muxPtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E |
                                          CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(busPtr);
    CPSS_NULL_PTR_CHECK_MAC(muxPtr);

    if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            if(cpssOsStrCmp(interfaceName,"rxdma2ctrl_pipe_desc") == 0)
            {
                switch(multiplicationIndex)
                {
                case 1:
                    cpssOsStrCpy(busPtr,"rxdma_ctrl_pipe_and_mppm_1_0_15");
                    *muxPtr =0;
                    break;
                case 2:
                    cpssOsStrCpy(busPtr,"rxdma_ctrl_pipe_and_mppm_2_1_9");
                    *muxPtr =0;
                    break;
                case 3:
                    cpssOsStrCpy(busPtr,"rxdma_ctrl_pipe_and_mppm_3_1_18");
                    *muxPtr =0;
                    break;
                }
            }
            else
            {
                CPSS_LOG_ERROR_MAC("Error: interfaceName [%s] is not multiple", interfaceName);
            }
        }
        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
        {
            if(cpssOsStrCmp(interfaceName,"RXDMA_2_CTRL_PIPE") == 0)
            {
                switch(multiplicationIndex)
                {
                case 1:
                    cpssOsStrCpy(busPtr,"rxdma1_ctrl_pipe_and_nextct");
                    *muxPtr =0;
                    break;
                case 2:
                    cpssOsStrCpy(busPtr,"rxdma2_ctrl_pipe_and_nextct");
                    *muxPtr =0;
                    break;
                }
            }
            if(cpssOsStrCmp(interfaceName,"txdma2ha_desc") == 0)
            {
                switch(multiplicationIndex)
                {
                case 1:
                    cpssOsStrCpy(busPtr,"txdma1_desc");
                    *muxPtr =0;
                    break;
                case 2:
                    cpssOsStrCpy(busPtr,"txdma2_desc");
                    *muxPtr =0;
                    break;
                }
            }
            else
            {
                CPSS_LOG_ERROR_MAC("Error: interfaceName [%s] is not multiple", interfaceName);
            }
        }

     }
     else
     {
           CPSS_LOG_ERROR_MAC("Error: prvCpssDxChIdebugMultipleInterfaceByIndexGet is not applicable for SIP6");
     }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugMuxDbInit function
* @endinternal
*
* @brief   The function set default configuration to mux
*          priority data base
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChIdebugMuxDbInit
(
    IN    GT_U8                                         devNum
)
{
    GT_U32 i,j,k;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            for (i=0; i<idebugDB->numOfFamilies; i++)
            {
                /* file is open; and also all other allocations were done */
                if (idebugDB->idebugFamilyPtrArr[i]&&
                        idebugDB->idebugFamilyPtrArr[i]->familyId == (PRV_CPSS_PP_MAC(devNum)->devFamily))
                    break;
            }
            if(i==idebugDB->numOfFamilies)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }
            if(idebugDB->idebugFamilyPtrArr[i]->muxDb==NULL)
            {
                idebugDB->idebugFamilyPtrArr[i]->muxDb = (GT_U32 **)cpssOsMalloc(sizeof(GT_U32 *)* idebugDB->numOfInstances);

                if (idebugDB->idebugFamilyPtrArr[i]->muxDb== NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                cpssOsMemSet(idebugDB->idebugFamilyPtrArr[i]->muxDb,0,sizeof(GT_U32 *)*idebugDB->numOfInstances);

                for(j=0; j<idebugDB->numOfInstances; j++)
                {
                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j] = (GT_U32 *)cpssOsMalloc(sizeof(GT_U32)* PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS);
                    if (idebugDB->idebugFamilyPtrArr[i]->muxDb[j]== NULL)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    for(k=0;k<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS;k++)
                    {
                        idebugDB->idebugFamilyPtrArr[i]->muxDb[j][k] = k;
                    }
                }

                /*custom settings*/
                if((PRV_CPSS_PP_MAC(devNum)->devFamily)==CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
                {
                    for (j=0; j<idebugDB->numOfInstances; j++)
                    {
                        if(cpssOsStrCmp("egress_desc_or_hdr_1_3",idebugDB->idebugInstancesStrPtrArr[j])==0)
                            break;
                    }
                    if(j==idebugDB->numOfInstances)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j][7] = 4;
                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j][4] = 7;

                    for (j=0; j<idebugDB->numOfInstances; j++)
                    {
                        if(cpssOsStrCmp("l2i_ipvx_desc_or_debug_buses_0_8",idebugDB->idebugInstancesStrPtrArr[j])==0)
                            break;
                    }
                    if(j==idebugDB->numOfInstances)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j][7] = 0;
                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j][0] = 7;

                    for (j=0; j<idebugDB->numOfInstances; j++)
                    {
                        if(cpssOsStrCmp("iplr_ioam_desc_0_9",idebugDB->idebugInstancesStrPtrArr[j])==0)
                            break;
                    }
                    if(j==idebugDB->numOfInstances)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j][7] = 1;
                    idebugDB->idebugFamilyPtrArr[i]->muxDb[j][1] = 7;
                 }
            }
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugSelectInerfaceByPriority function
* @endinternal
*
* @brief   Select specific mux based on defined priority
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] currentBus               - instance
* @param[in] priority                 - muxed interface priority
* @param[out] muxOutPtr               - (pointer to) muxed interface index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugSelectInerfaceByPriority
(
    GT_U8                                       devNum,
    GT_CHAR_PTR                                 currentBus,
    GT_U32                                      priority,
    GT_U32                                      *muxOutPtr
)
{
     GT_U32 i,j;
     PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

     for (i=0; i<idebugDB->numOfFamilies; i++)
     {
         if (idebugDB->idebugFamilyPtrArr[i]&&
                 idebugDB->idebugFamilyPtrArr[i]->familyId == (PRV_CPSS_PP_MAC(devNum)->devFamily))
             break;
     }

    if(i==idebugDB->numOfFamilies)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    for (j=0; j<idebugDB->numOfInstances; j++)
    {
        if(cpssOsStrCmp(currentBus,idebugDB->idebugInstancesStrPtrArr[j])==0)
            break;
    }
    *muxOutPtr = idebugDB->idebugFamilyPtrArr[i]->muxDb[j][priority];

   return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInerfacePrioritySet function
* @endinternal
*
* @brief The function set interface priority in case of mux
*        between interfaces.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] currentBus               - instance
* @param[in] priority                 - priority to set[0..7]
* @param[in] muxIn                    - mux index that should have highest priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChIdebugInerfacePrioritySet
(
    GT_U8                                       devNum,
    GT_CHAR_PTR                                 currentBus,
    GT_U32                                      priority,
    GT_U32                                      muxIn
)
{
    GT_U32 i,j,k,oldValue,oldIndex=0;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    for (i=0; i<idebugDB->numOfFamilies; i++)
    {
      if (idebugDB->idebugFamilyPtrArr[i]&&
              idebugDB->idebugFamilyPtrArr[i]->familyId == (PRV_CPSS_PP_MAC(devNum)->devFamily))
          break;
    }

    if(i==idebugDB->numOfFamilies)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    for (k=0; k<idebugDB->numOfInstances; k++)
    {
        if(cpssOsStrCmp(currentBus,idebugDB->idebugInstancesStrPtrArr[k])==0)
            break;
    }
    if(k==idebugDB->numOfInstances)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    oldValue = idebugDB->idebugFamilyPtrArr[i]->muxDb[k][priority];

    for(j=0;j<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS;j++)
    {
        if(idebugDB->idebugFamilyPtrArr[i]->muxDb[k][j]==muxIn)
        {
           oldIndex = j;
           break;
        }
    }

    if(j==PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /*do the swap*/
    idebugDB->idebugFamilyPtrArr[i]->muxDb[k][priority]=muxIn;
    idebugDB->idebugFamilyPtrArr[i]->muxDb[k][oldIndex]=oldValue;


    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugInerfaceMuxSet function
* @endinternal
*
* @brief   The function set   interface that will be chosen in case of mux between interfaces.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum             -  device number
* @param[in] currentBus         -  instance
* @param[in]muxIn               -  mux index that should have
*                                  highest priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugInerfaceMuxSet
(
    GT_U8                                       devNum,
    GT_CHAR_PTR                                 currentBus,
    GT_U32                                      muxIn
)
{
    return prvCpssDxChIdebugInerfacePrioritySet
            (devNum,currentBus,PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS-1,muxIn);
}

/**
* @internal prvCpssDxChIdebugFailOnUnfoundToken function
* @endinternal
*
* @brief   The function get failure action. The failue is considered not found token in XML.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                - device number
*
* @retval GT_TRUE                  - fail on unfound token
* @retval GT_FALSE                 - do notfail on unfound token
*
*/
GT_BOOL prvCpssDxChIdebugFailOnUnfoundToken
(
    IN  GT_U8                                       devNum
)
{
    /*this fuction is stub ,do not delete*/
    devNum=devNum;
    return GT_TRUE;
}


/**
* @internal prvCpssDxChIdebugMuxTokenNameGet function
* @endinternal
*
* @brief   The function return the right name token per device
*
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum            - device number
* @param[out]muxTokeName       - (pointer to) mux token Name.
*
* @retval GT_OK                - on success
* @retval GT_NOT_SUPPORTED     - on not applicable device
*
*/
GT_STATUS prvCpssDxChIdebugMuxTokenNameGet
(
    IN  GT_U8                                      devNum,
    OUT GT_CHAR_PTR                                *muxTokeName
)
{
    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            *muxTokeName= "index";
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            *muxTokeName= "muxIndex";
            break;
        default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: Device family is not supported  for iDebug");
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugEncodeMetaData function
* @endinternal
*
* @brief   The function encode search attribute structure into meta data
*
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2.
*
* @param[in] devNum             -  device number
* @param[in] searchAttributePtr -  (pointer to) search attributes.
* @param[out] metaDataPtr       -  (pointer to) encoded meta data.
*
* @retval GT_OK                 - on success
* @retval GT_OUT_OF_RANGE       - one of the parameters is out of range
*
*/
GT_STATUS prvCpssDxChIdebugEncodeMetaData
(
    IN    GT_U32                                          devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  *searchAttributePtr,
    OUT   GT_U32                                           *metaDataPtr
)
{
    if(searchAttributePtr->pipe>=MAX_PIPES_PER_TILE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"Pipe %d is out of range",searchAttributePtr->pipe);
    }


   if(searchAttributePtr->dp>=(MAX_DP_PER_TILE_CNS/MAX_PIPES_PER_TILE_CNS))
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"Pipe %d is out of range",searchAttributePtr->pipe);
   }
   PRV_IDEBUG_CHECK_TILE_NUM_MAC(devNum,searchAttributePtr->tile)

   *metaDataPtr = searchAttributePtr->dp<<PRV_CPSS_DXCH_IDEBUG_META_DATA_DP_OFFSET_CNS;
   *metaDataPtr|=(searchAttributePtr->pipe<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PIPE_OFFSET_CNS);
   *metaDataPtr|=(searchAttributePtr->tile<<PRV_CPSS_DXCH_IDEBUG_META_DATA_TILE_OFFSET_CNS);
   *metaDataPtr|=(1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_VALID_OFFSET_CNS);

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugCompareMetaData function
* @endinternal
*
* @brief   The function compare two meta data fields
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2.
*
* @param[in] devNum                   - device number
*
* @retval GT_TRUE                     - meta data equal
* @retval GT_FALSE                    - meta data not equal
*
*/
static GT_BOOL prvCpssDxChIdebugCompareMetaData
(
    IN   GT_U32                                           metaDataSource,
    IN   GT_U32                                           metaDataTarget
)
{
    GT_U32      baseMsk =(GT_U32)( ~(0xfff<<PRV_CPSS_DXCH_IDEBUG_META_DATA_BASE_OFFSET_CNS));
    GT_U32      dpMsk = (GT_U32)(~(0xf<<PRV_CPSS_DXCH_IDEBUG_META_DATA_DP_OFFSET_CNS));
    GT_U32      perDpMsk = (GT_U32)(~(0x1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PER_DP_OFFSET_CNS));

    GT_BOOL     result = GT_FALSE;


    /*Check if need to compare DP*/
    if((metaDataSource&(1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PER_DP_OFFSET_CNS))==0)
    {
        metaDataSource&=dpMsk;
        metaDataTarget&=dpMsk;
        metaDataTarget&=perDpMsk;
    }
    else
    {
        /*do not compare per DP bit*/
        metaDataSource&=perDpMsk;
        metaDataTarget&=perDpMsk;
    }

    /*do not compare adress*/
    metaDataSource&=baseMsk;
    metaDataTarget&=baseMsk;




    if(metaDataSource==metaDataTarget)
    {
        result = GT_TRUE;
    }

    return result;
}
static GT_STATUS prvCpssDxChIdebugPortGroupInterfaceUpperMatchEnableSet
(
    IN    GT_U8       devNum,
    IN    GT_U32      instanceDfxBaseAddr,
    IN    GT_BOOL     enable
)
{
    GT_U32    regAddr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);


    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* select DFX instance match counter address */
        regAddr = instanceDfxBaseAddr +PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);

        /* Select Upper/Lower part of Match Sample bus to be represented in <Match Sample %i> registers in case of Sample Bus is bigger than 1536 bits.*/
        rc = cpssDrvHwPpResetAndInitControllerSetRegField(devNum,regAddr,21,1,BOOL2BIT_MAC(enable));
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChIdebugPortGroupInterfaceSampleDumpByCore
*           function
* @endinternal
*
* @brief   The dump specific interface identified by tile/control pipe/data path
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;.
*
* @param[in] devNum                   - device number
* @param[in] tile                     - tile index[0..3]
* @param[in] pipe                     - control pipe index[0..1]
* @param[in] dataPath                 - data path index[0..3]
* @param[in] interfaceName            - interface name
* @param[in] instanceName             - instance name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleDumpByCore
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  tile,
    IN  GT_U32                                  pipe,
    IN  GT_U32                                  dataPath,
    IN  GT_CHAR_PTR                             interfaceName,
    IN  GT_CHAR_PTR                             instanceName
)
{
    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  searchAttribute;
    GT_STATUS                                       rc;
    GT_U32                                          metaData;

    if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    searchAttribute.tile = tile;
    searchAttribute.pipe = pipe;
    searchAttribute.dp = dataPath;

    rc = prvCpssDxChIdebugEncodeMetaData(devNum,&searchAttribute,&metaData);
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChIdebugPortGroupInterfaceSampleDump(devNum,metaData,interfaceName,instanceName);

}


/**
* @internal prvCpssDxChIdebugPipeDpGet function
* @endinternal
*
* @brief   The function parse control pipe/data path from
*          instance name.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2;.
*
* @param[in] currentBusName            - interface name
* @param[out]pipe                      - (pointer to) control pipe index[0..1]
* @param[out]dataPath                  - (pointer to) data path index[0..3]
* @param[in] dpInfoPresentPtr          - (pointer to) dp info present
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS prvCpssDxChIdebugPipeDpGet
(
    IN  const GT_CHAR                           *currentBusName,
    OUT GT_U32                                  *pipePtr,
    OUT GT_U32                                  *dpPtr,
    OUT GT_BOOL                                 *dpInfoPresentPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(currentBusName);
    CPSS_NULL_PTR_CHECK_MAC(pipePtr);
    CPSS_NULL_PTR_CHECK_MAC(dpPtr);
    CPSS_NULL_PTR_CHECK_MAC(dpInfoPresentPtr);

    /*Determine control path*/
    if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_0_TOKEN_CNS))
    {
        *pipePtr = 0;
    }
    else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_1_TOKEN_CNS))
    {
        *pipePtr = 1;
    }


    /*Determine data path*/
    if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_0_TOKEN_CNS))
    {
        *dpPtr = 0;
        *dpInfoPresentPtr = GT_TRUE;

        /*Check sub pipe presence*/
         if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_1_TOKEN_CNS))
         {
            *dpPtr = 1;
         }
         else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_1_TOKEN_CNS))
         {
            *dpPtr = 1;
         }
    }
    else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_1_TOKEN_CNS))
    {
        *dpPtr = 1;
        *dpInfoPresentPtr = GT_TRUE;

         /*Check sub pipe presence*/
         if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_0_TOKEN_CNS))
         {
            *dpPtr = 2;
         }
         else  if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_1_TOKEN_CNS))
         {
            *dpPtr = 3;
         }
         else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_0_TOKEN_CNS))
         {
            *dpPtr = 2;
         }
         else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_1_TOKEN_CNS))
         {
            *dpPtr = 3;
         }
    }
     else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_2_TOKEN_CNS))
    {
        *dpPtr = 2;
        *dpInfoPresentPtr = GT_TRUE;

        /*Check sub pipe presence*/
        if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_0_TOKEN_CNS))
        {
            *dpPtr = 0;
        }
        else  if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_1_TOKEN_CNS))
        {
           *dpPtr = 1;
        }
        else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_0_TOKEN_CNS))
         {
            *dpPtr = 0;
         }
         else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_1_TOKEN_CNS))
         {
            *dpPtr = 1;
         }
    }
    else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_3_TOKEN_CNS))
    {
       *dpPtr = 3;
        *dpInfoPresentPtr = GT_TRUE;

        /*Check sub pipe presence*/
        if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_0_TOKEN_CNS))
        {
            *dpPtr = 2;
        }
        else  if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_CP_SUB_PIPE_1_TOKEN_CNS))
        {
           *dpPtr = 3;
        }
        else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_0_TOKEN_CNS))
        {
           *dpPtr = 2;
        }
        else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_1_TOKEN_CNS))
        {
           *dpPtr = 3;
        }
    }
    /*try to find by agreggator*/
    else if((cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_IA_0_TOKEN_CNS))||
        (cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_IA_1_TOKEN_CNS)))
    {
        if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_0_TOKEN_CNS))
        {
            *dpPtr = 0;
            *dpInfoPresentPtr = GT_TRUE;
        }
        else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_1_TOKEN_CNS))
        {
            *dpPtr = 1;
            *dpInfoPresentPtr = GT_TRUE;
        }
         else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_2_TOKEN_CNS))
        {
            *dpPtr = 2;
            *dpInfoPresentPtr = GT_TRUE;
        }
        else if(cpssOsStrStr(currentBusName,PRV_CPSS_DXCH_IDEBUG_DP_IA_3_TOKEN_CNS))
        {
            *dpPtr = 3;
            *dpInfoPresentPtr = GT_TRUE;
        }
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChIdebugFileFpGet function
* @endinternal
*
* @brief  Get pointer to XML file
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
CPSS_OS_FILE_TYPE_STC * prvCpssDxChIdebugFileFpGet
(
    IN  GT_U8                                   devNum
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_IDEBUG_DB_STC *idebugDB = PRV_SHARED_IDEBUG_DIR_IDEBUG_SRC_DB_PTR();

    for (i=0; i<idebugDB->numOfFamilies; i++)
    {
        /* find the family*/
        if (idebugDB->idebugFamilyPtrArr[i]->familyId == PRV_CPSS_PP_MAC(devNum)->devFamily)
            break;
    }
    if(i==idebugDB->numOfFamilies)
    {
        return NULL;
    }
    return  idebugDB->idebugFamilyPtrArr[i]->file;
}

/**
* @internal mxmlInstanceInfoSegmentIdGet function
* @endinternal
*
* @brief  Get segment index from interface name
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
*
* @param[in] devNum               - device number
* @param[in] interfaceName        - interface name
* @param[out] segmentIdPtr        - (pointer to)index of segment
* @param[out] muxIdPtr            - (pointer to)index of mux select
* @param[out] instancePtr         - (pointer to)instance mxml node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
static GT_STATUS mxmlInstanceInfoSegmentIdGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *segmentIdPtr,
    OUT GT_U32                                          *muxIdPtr,
    OUT mxml_node_t                                     **instancePtr
)
{
    mxml_node_t         *node, *instance, *head;
    const GT_CHAR       *attr;
    GT_STATUS           rc;
    GT_CHAR_PTR         muxTokeName;

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    /* find first instance */
    instance = node = node->child;
     if (!node)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

      while (instance != NULL)
      {
          PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(instance)

          *instancePtr = instance;

          node = prvCpssMxmlFindElement(instance, instance, "Interfaces", NULL, NULL, MXML_DESCEND);
          if (!node)
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
          /* find first interface */
          node = node->child;

          while(node != NULL)
          {
              PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

              attr = prvCpssMxmlElementGetAttr(node,"internalName");
              if(cpssOsStrCmp(attr,interfaceName) == 0)
              {
                rc = prvCpssDxChIdebugMuxTokenNameGet(devNum,&muxTokeName);
                if (rc != GT_OK)
                {
                  return rc;
                }

                 attr = prvCpssMxmlElementGetAttr(node,muxTokeName);
                *muxIdPtr = cpssOsStrTo32(attr);

                attr = prvCpssMxmlElementGetAttr(node,"placeInMux");
                *segmentIdPtr = cpssOsStrTo32(attr);
                return GT_OK;
              }
              node = node->next;
          }

          instance = instance->next;
      }
     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
 }

/**
* @internal mxmlInstanceInfoSegmentIdGet function
* @endinternal
*
* @brief  Get segment index from interface name
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
*
* @param[in] devNum                - device number
* @param[in] instancePtr           - (pointer to)instance mxml node
* @param[in] segmentId             - index of segment
* @param[in] muxId                 - index of mux select
* @param[in] sizePtr               - (pointer to)size of segment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
static GT_STATUS mxmlInstanceInfoSegmentSizeGet
(
    IN  GT_U8                                           devNum,
    IN mxml_node_t                                     *instancePtr,
    IN GT_U32                                           segmentId,
    IN GT_U32                                           muxId,
    OUT GT_U32                                         *sizePtr
)
{
    mxml_node_t         *node;
    const GT_CHAR       *attr;
    GT_CHAR             *interfaceName;
    GT_STATUS           rc;
    GT_CHAR_PTR         muxTokeName;
    GT_U32              currentMuxId,currentSegment,startBit;

    node = prvCpssMxmlFindElement(instancePtr, instancePtr, "Interfaces", NULL, NULL, MXML_DESCEND);
    if (!node)
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
          /* find first interface */
     node = node->child;

      while(node != NULL)
      {
          PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

          interfaceName =(GT_CHAR*) prvCpssMxmlElementGetAttr(node,"internalName");
          rc = prvCpssDxChIdebugMuxTokenNameGet(devNum,&muxTokeName);
          if (rc != GT_OK)
          {
            return rc;
          }

          attr = prvCpssMxmlElementGetAttr(node,muxTokeName);
          currentMuxId = cpssOsStrTo32(attr);

          if(currentMuxId == muxId)
          {
             attr = prvCpssMxmlElementGetAttr(node,"placeInMux");
             currentSegment = cpssOsStrTo32(attr);
             if(currentSegment == segmentId)
             {
               node = mxmlInterfaceSizeGet(devNum,interfaceName,sizePtr,&startBit);
               if(NULL==node)
               {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
               }
               return GT_OK;
             }
          }

          node = node->next;
      }

     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChIdebugInterfaceMxmlSegmentGet function
* @endinternal
*
* @brief  Get segment index from interface name
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2 .
* @param[in] devNum                - device number
* @param[in] interfaceName         - interface name
* @param[out] segmentIdPtr         -(pointer to)index of segment
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
GT_STATUS   prvCpssDxChIdebugInterfaceMxmlSegmentGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *segmentIdPtr
)
{
    mxml_node_t         *node;
    GT_U32              muxId;
    GT_STATUS           rc= GT_OK;


    *segmentIdPtr = 0 ;
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc =mxmlInstanceInfoSegmentIdGet(devNum,interfaceName,segmentIdPtr,&muxId,&node);
    }

    return rc;

}

/**
* @internal mxmlInterfaceSegmentsArrGet function
* @endinternal
*
* @brief  Get array of interfaces with same mux index as
*         interfaceName in instanceId
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum               - device number
* @param[in] instanceId           - instance Id
* @param[in] interfaceName        - interface name
* @param[out] numOfSegmentsPtr    - (pointer to)num of segments
* @param[out] segmentsArrPtr      - (pointer to)segments array
*
* @retval GT_OK                     - on success
* @retval GT_OUT_OF_RANGE           - on out of range param
* @retval GT_NOT_APPLICABLE_DEVICE  - on none applicable device
* @retval GT_FAIL                   - on error
*
*/
GT_STATUS mxmlInterfaceSegmentsArrGet
(
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     instanceId,
    IN  GT_CHAR_PTR                                     interfaceName,
    OUT GT_U32                                          *numOfSegmentsPtr,
    OUT GT_CHAR                                         **segmentsArrPtr
)
{
    mxml_node_t         *node, *head;
    const GT_CHAR       *attr;
    GT_U32              segmentId,muxId,numOfSeg=0;
    GT_CHAR_PTR         muxTokenName;
    GT_STATUS           rc;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
    {
        *numOfSegmentsPtr = 1;
        cpssOsStrCpy(segmentsArrPtr[numOfSeg],interfaceName);

        return GT_OK;
    }

    /* Check if the instance is applicable for the device */
    rc = prvCpssDxChIdebugInstanceDeviceCheck(devNum, instanceId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Check if the interface is applicable for the device */
    rc = prvCpssDxChIdebugInterfaceDeviceCheck(devNum, interfaceName);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = mxmlInstanceInfoSegmentIdGet(devNum,interfaceName,&segmentId,&muxId,&node);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbHeadGet(devNum,&head);
    if (rc != GT_OK)
    {
        return rc;
    }

    attr = instanceId;

    node = prvCpssMxmlFindElement(head, head, "Instances", NULL, NULL, MXML_DESCEND);
    if (!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    node = prvCpssMxmlFindElement(node, node, NULL, "name", attr, MXML_DESCEND_FIRST);
    if (!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    node = prvCpssMxmlFindElement(node, node, "Interfaces", NULL, NULL, MXML_DESCEND);
    if (!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    /* find first interface */
    node = node->child;
    if (!node)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    while(node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        rc = prvCpssDxChIdebugMuxTokenNameGet(devNum,&muxTokenName);
        if (rc != GT_OK)
        {
            return rc;
        }

        attr = prvCpssMxmlElementGetAttr(node,muxTokenName);
        if(muxId == (GT_U32)cpssOsStrToU32(attr, NULL, 0))
        {
            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            cpssOsStrCpy(segmentsArrPtr[numOfSeg],attr);

            numOfSeg++;
        }
        node = node->next;
    }

    *numOfSegmentsPtr = numOfSeg;

    return GT_OK;
 }


/**
* @internal
*           prvCpssDxChIdebugPortGroupInterfaceResetSamplingTriger
*           function
* @endinternal
*
* @brief   clear the sampling triger by reading the first word of it .
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Aldrin2)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceResetSamplingTriger
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName
)
{
    GT_U32              interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32              instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32              instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];

    GT_U32              regAddr,tileInd=0;
    GT_STATUS           rc = GT_OK;
    GT_U32              sample;
    GT_U32              numberOfMultiplications,multiplicationIterator;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_IDEBUG_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp,tileInd);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* check that sampling is disabled for interface */
    rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableCheck(devNum,portGroupsBmp,interfaceName);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* select DFX pipe for Read and Write Transaction */
    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_READ_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(multiplicationIterator=0;multiplicationIterator<numberOfMultiplications;multiplicationIterator++)
    {

        /* select DFX pipe for Read and Write Transaction */
        rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeIdArr[multiplicationIterator]);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* select DFX instance match sample register address */
        regAddr = instanceDfxBaseAddrArr[multiplicationIterator] +
            PRV_DFX_XSB_MATCH_SAMPLE_REG_OFFSET_MAC(devNum);

        /*read the first word of the sampling*/
        rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum,regAddr,&sample);
        if(rc != GT_OK)
        {
            return rc ;
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChIdebugPortGroupSingleInterfaceSamplingEnableGet
*           function
* @endinternal
*
* @brief   Get sampling status (enable/disable) for single interface by instance info.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
* @param[in] instanceDfxPipeId        - instance Dfx Pipe Id
* @param[in] instanceDfxBaseAddr      - instance Dfx Base Addr
* @param[out] enablePtr               - (pointer to)
*                                       enable/disable sampling
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugPortGroupSingleInterfaceSamplingEnableGet
(
    IN  GT_U8                                            devNum,
    IN  GT_CHAR_PTR                                      interfaceName,
    IN  GT_U32                                           instanceDfxPipeId,
    IN  GT_U32                                           instanceDfxBaseAddr,
    OUT GT_BOOL                                          *enablePtr
)
{
    GT_U32 ii,regAddr,value,numberOfTiles;
    GT_U32 tempInstanceDfxBaseAddr;
    GT_BOOL enableArr[4] = {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE};
    GT_STATUS rc = GT_OK;


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

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

    for(ii=0;ii<numberOfTiles;ii++)
    {
        tempInstanceDfxBaseAddr=instanceDfxBaseAddr +PRV_DFX_XSB_TILE_OFFSET_MAC(devNum,ii);

        /* select DFX pipe for Read and Write Transaction */
        rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeId);

        if(rc != GT_OK)
        {
            return rc;
        }
        /* select DFX instance general configuration register address */
        regAddr = tempInstanceDfxBaseAddr + PRV_DFX_XSB_GEN_CONF_REG_OFFSET_MAC(devNum);
        rc = cpssDrvHwPpResetAndInitControllerGetRegField(devNum,regAddr,0,1,&value);
        if(rc != GT_OK)
        {
            return rc;
        }
        enableArr[ii] = BIT2BOOL_MAC(value);

    }

    /*chek that all is synced*/
    for(ii=0;ii<numberOfTiles;ii++)
    {
        if(enableArr[0] !=enableArr[ii])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: not all Tiles are synced ");
        }
    }
    *enablePtr = enableArr[0];
    return rc;
}

/**
* @internal prvCpssDxChIdebugClearLongestInterface function
* @endinternal
*
* @brief   set zeros in mask and data of the longest interface that
*          share same instance wite given interface.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager Id number
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface name
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChIdebugClearLongestInterface
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    IN  GT_CHAR_PTR                                 interfaceName
)
{
    GT_STATUS rc;

    GT_CHAR             interface[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_CHAR             instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32              numberOfMultiplications ,numOfInterfaces ;
    GT_U32              ii,mm ,jj ;

    GT_U32              instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32              instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32              interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];

    mxml_node_t                                     *node;
    GT_U32                                          size,startBit,maxSize=0,numOfSeg;
    GT_CHAR_PTR                                     *interfaceSegArr = NULL;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC                  *interfaceFieldsArr = NULL;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC        *idebugFieldsValueArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfacesArr = NULL;
    GT_U32                                          numOfFieldsDb,interfaceNumOfFields;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    rc = prvCpssDxChIdebugDbNumFieldsGet(devNum,&numOfFieldsDb);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* array allocation */
    idebugFieldsValueArr = (PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFieldsDb);
    interfaceFieldsArr = (PRV_CPSS_DXCH_IDEBUG_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*numOfFieldsDb);
    interfaceSegArr = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS);
    interfacesArr = (CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC)*100);
    if (idebugFieldsValueArr == NULL || interfaceFieldsArr == NULL || interfaceSegArr == NULL || interfacesArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    for (ii=0; ii<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS; ii++)
    {
        interfaceSegArr[ii] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
        if (interfaceSegArr[ii] == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            goto exit_cleanly_lbl;
        }
    }

    cpssOsMemSet(idebugFieldsValueArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC)*numOfFieldsDb);

    rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);
    if (rc !=GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for (mm=0;mm<numberOfMultiplications ;mm++)
    {
        numOfInterfaces=100;
        mxmlBaseAddrInstanceGet(devNum,instanceDfxBaseAddrArr[mm],instanceDfxPipeIdArr[mm],instanceId);

        rc = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,interfacesArr);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        maxSize = 0;
        for (jj=0; jj<numOfInterfaces; jj++)
        {
            node = mxmlInterfaceSizeGet(devNum,interfacesArr[jj].interfaceId,&size,&startBit);
            if (node == NULL)
            {
                rc = GT_BAD_STATE;
                goto exit_cleanly_lbl;
            }

            /* found longer interface on this instance; save params */
            if (size > maxSize)
            {
                cpssOsStrCpy(interface,interfacesArr[jj].interfaceId);
                maxSize = size;
            }
        }

        /* check if the interface is concatenated */
        rc = mxmlInterfaceSegmentsArrGet(devNum,instanceId,interface,&numOfSeg,interfaceSegArr);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }

        for (ii=0; ii<numOfSeg; ii++)
        {
            /* clean all the previous rules according to the longest interface attached to instanceId */
            interfaceNumOfFields = numOfFieldsDb;
            rc = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceSegArr[ii],&interfaceNumOfFields,interfaceFieldsArr);

            /* prepare array with rule's content (Data and Mask) */
            for (jj=0; jj<interfaceNumOfFields; jj++)
            {
                cpssOsStrCpy(idebugFieldsValueArr[jj].fieldName,interfaceFieldsArr[jj].fieldName);
            }

            /* delete interface from rule's content (Data and Mask) */
            rc = prvCpssDxChIdebugPortGroupSingleInterfaceSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfaceSegArr[ii],
                        instanceDfxPipeIdArr[mm],instanceDfxBaseAddrArr[mm],interfaceDfxIndexArr[mm],
                        interfaceNumOfFields,idebugFieldsValueArr);
            if(rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
    }

  exit_cleanly_lbl:

    if (idebugFieldsValueArr != NULL)
        cpssOsFree(idebugFieldsValueArr);
    if (interfaceFieldsArr != NULL)
        cpssOsFree(interfaceFieldsArr);
    if (interfaceSegArr != NULL)
    {
        for (ii=0; ii<PRV_CPSS_DXCH_IDEBUG_MAX_MUX_NUM_CNS; ii++)
        {
            if (interfaceSegArr[ii] != NULL)
            cpssOsFree(interfaceSegArr[ii]);
        }
        cpssOsFree(interfaceSegArr);
    }
    if (interfacesArr != NULL)
        cpssOsFree(interfacesArr);
    return rc;
}

/**
* @internal prvCpssDxChIdebugInterfaceInstancesGet function
* @endinternal
*
* @brief   Get interface instances list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface identification
* @param[in,out] numOfInstancesPtr    - in: (pointer to)
*                                       allocated number of
*                                       Instances for
*                                       instance
*                                       out: (pointer to) actual
*                                       number of interfaces for
*                                       interface
* @param[out] instancesArr            - (pointer to) interface
*                                       instances list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIdebugInterfaceInstancesGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   interfaceName,
    INOUT GT_U32                                        *numOfInstancesPtr,
    OUT   GT_CHAR_PTR                                   *instancesArr
)
{
    GT_U32              ii,numberOfMultiplications;
    GT_STATUS           rc;
    GT_U32              instanceDfxPipeIdArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32              instanceDfxBaseAddrArr[MAX_MULTIPLICATIONS_NUM];
    GT_U32              interfaceDfxIndexArr[MAX_MULTIPLICATIONS_NUM];

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(numOfInstancesPtr);
    CPSS_NULL_PTR_CHECK_MAC(instancesArr);

     rc = prvCpssDxChIdebugInterfaceAttributesGet(devNum,interfaceName,PRV_CPSS_DXCH_IDEBUG_OP_CODE_WRITE_CNS,
        instanceDfxPipeIdArr,instanceDfxBaseAddrArr,interfaceDfxIndexArr,&numberOfMultiplications,NULL);
    if (rc !=GT_OK)
    {
        return rc;
    }

    if (*numOfInstancesPtr<numberOfMultiplications)
    {
        *numOfInstancesPtr = numberOfMultiplications;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of Multiplications is [%d] for interface [%s]",
                                      numberOfMultiplications,interfaceName);
    }
    *numOfInstancesPtr = numberOfMultiplications;
    for (ii = 0; ii < numberOfMultiplications; ii++)
    {
        mxmlBaseAddrInstanceGet(devNum,instanceDfxBaseAddrArr[ii],instanceDfxPipeIdArr[ii],instancesArr[ii]);
    }
    return GT_OK;
}


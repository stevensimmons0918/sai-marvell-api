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
* @file prvCpssDxChExactMatchManager_hw.c
*
* @brief Exact Match manager support - manipulations of HW device(s) : read/write to device(s).
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_hw.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_debug.h>
#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTablesSip6.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>

/* the size of expanded action in bytes */
#define PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS              31

/* sip6_10 : the size of expanded action in bytes */
#define PRV_CPSS_SIP6_10_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS      32

/* the MAX size of expanded action in bytes */
#define PRV_CPSS_DXCH_MAX_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS      32

/* macro to set value to field of tti ACTION entry format in buffer */
#define SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)                                  \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                     \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E].fieldsInfoPtr, \
        _fieldName,                                                                                         \
        _value)


/* macro to Get value to field of tti ACTION entry format in buffer */
#define SIP6_TTI_HW_ENTRY_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)                                  \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                     \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E].fieldsInfoPtr, \
        _fieldName,                                                                                         \
        _value)

extern GT_STATUS ttiGetIndexForKeyType
(
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             isLegacyKeyTypeValid,
    OUT GT_U32                              *indexPtr
);

extern GT_STATUS sip5PclUDBEntryIndexGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    OUT GT_U32                               *entryIndexPtr
);

static GT_STATUS prvCpssDxChExactMatchManagerAacParamSet
(
    IN GT_U8                                            devNum
)
{
    GT_STATUS                                   rc;
    GT_U32                                      regAddr;
    GT_U32                                      regVal;
    PRV_CPSS_DXCH_AAC_CHANNELS_ENT              channel;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 0)
    {
        /* AAC not used for devices with single tile */
        return GT_OK;
    }

    /* AAC global control */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACControl;
    /* AAC Enable */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, 1);
    if(rc != GT_OK)
    {
        return rc;
    }

    for (channel = PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_PRIMARY_E;
         channel <= PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_SECONDARY_E; channel++)
    {
        /*  AAC Engine <<%n>> Control for channel 1,2 */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineControl[channel];
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regVal);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* AAC address mode - direct */
        U32_SET_FIELD_MAC(regVal, 0, 1, 0);
        /* AAC multicast enable */
        U32_SET_FIELD_MAC(regVal, 2, 1, 1);

        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regVal);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* AAC Engine <<%n>> MC External Offset for channel 1,2 */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCExternalOffset[channel];
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, FALCON_TILE_OFFSET_CNS);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*  AAC Engine <<%n>> MC Control for channel 1,2 */
        regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineMCControl[channel];
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 10, PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerHwLookupConfigSet function
* @endinternal
*
* @brief  This function set all lookup configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] portGroupsBmp   - port group bitmap
* @param[in] lookupPtr       - (pointer to) the Exact Match Manager lookup config.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwLookupConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupsBmp,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC     *lookupPtr
)
{
    GT_STATUS                                               rc;
    GT_BOOL                                                 lookupEnable;
    GT_U32                                                  lookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                        lookupClient;
    GT_U32                                                  lookupClientMappingsNum;
    GT_BOOL                                                 enableExactMatchLookup;
    GT_U32                                                  profileIdVal;
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT    *lookupClientMappingsArray;
    CPSS_PCL_DIRECTION_ENT                                  direction;
    GT_U32                                                  i;
    GT_PORT_GROUPS_BMP                                      newPortGroupsBmp;

    CPSS_DXCH_TCAM_CLIENT_ENT                               tcamClient;
    GT_U32                                                  tcamGroup;
    GT_BOOL                                                 enable;

    /* update portGroupsBmp
       both bits of a pair should have the same value - same configuration for pipe0 and pipe1*/
    newPortGroupsBmp = portGroupsBmp;
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_UPDATE_ODD_BITS_MAC(newPortGroupsBmp);

    for(lookupNum=0;lookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;lookupNum++)
    {
        lookupEnable                =   lookupPtr->lookupsArray[lookupNum].lookupEnable;
        lookupClient                =   lookupPtr->lookupsArray[lookupNum].lookupClient;
        lookupClientMappingsNum     =   lookupPtr->lookupsArray[lookupNum].lookupClientMappingsNum;
        lookupClientMappingsArray   =   lookupPtr->lookupsArray[lookupNum].lookupClientMappingsArray;

        if(lookupEnable==GT_TRUE)
        {
            switch(lookupClient)
            {
                case  CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E:
                    tcamClient = CPSS_DXCH_TCAM_TTI_E;
                    break;

                case  CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E:
                    tcamClient = CPSS_DXCH_TCAM_IPCL_0_E;
                    break;

                case  CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E:
                    tcamClient = CPSS_DXCH_TCAM_IPCL_1_E;
                    break;

                case  CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E:
                    tcamClient = CPSS_DXCH_TCAM_IPCL_2_E;
                    break;

                case  CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E:
                    tcamClient = CPSS_DXCH_TCAM_EPCL_E;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    break;
            }

            rc = cpssDxChTcamPortGroupClientGroupGet(devNum,
                                                     portGroupsBmp,
                                                     tcamClient,
                                                     &tcamGroup,
                                                     &enable);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(enable == GT_FALSE)
            {
                /* Check that TCAM group associated with the TCAM client is enabled */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "TCAM group associated with the TCAM client MUST be enabled \n");
            }

            /* set lookup client */
            rc = prvCpssDxChExactMatchPortGroupClientLookupSet(devNum,
                                                               portGroupsBmp,
                                                               (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                               lookupClient);
            if (rc != GT_OK)
            {
                return rc;
            }


            for(i=0;i<lookupClientMappingsNum;i++)
            {
                /* set mapping of key/packet type to profileId */
                if(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                {
                    rc = prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeSet(devNum,
                                                                                   newPortGroupsBmp,
                                                                                   lookupClientMappingsArray[i].ttiMappingElem.keyType,
                                                                                   (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                                                   lookupClientMappingsArray[i].ttiMappingElem.enableExactMatchLookup,
                                                                                   lookupClientMappingsArray[i].ttiMappingElem.profileId);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    enableExactMatchLookup = lookupClientMappingsArray[i].ttiMappingElem.enableExactMatchLookup;
                    profileIdVal = lookupClientMappingsArray[i].ttiMappingElem.profileId;
                }
                else
                {
                    if (lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
                        direction = CPSS_PCL_DIRECTION_EGRESS_E;
                    else
                    {
                        if ((!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))&&(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: IPCL_2 only supported for AC5P \n");
                        }

                        direction = CPSS_PCL_DIRECTION_INGRESS_E;
                    }
                    rc = prvCpssDxChExactMatchPortGroupPclProfileIdSet(devNum,
                                                                        newPortGroupsBmp,
                                                                        direction,
                                                                        lookupClientMappingsArray[i].pclMappingElem.packetType,
                                                                        lookupClientMappingsArray[i].pclMappingElem.subProfileId,
                                                                        (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                                        lookupClientMappingsArray[i].pclMappingElem.enableExactMatchLookup,
                                                                        lookupClientMappingsArray[i].pclMappingElem.profileId);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    enableExactMatchLookup = lookupClientMappingsArray[i].pclMappingElem.enableExactMatchLookup;
                    profileIdVal = lookupClientMappingsArray[i].pclMappingElem.profileId;
                }

                /* set profileId configuration */
                if (enableExactMatchLookup==GT_TRUE)
                {
                    /* set key params */
                    rc = prvCpssDxChExactMatchPortGroupProfileKeyParamsSet(devNum,
                                                                        portGroupsBmp,
                                                                        profileIdVal,
                                                                        &lookupPtr->profileEntryParamsArray[profileIdVal].keyParams);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    /* set default entry configuration */
                    rc = prvCpssDxChExactMatchPortGroupProfileDefaultActionSet(devNum,
                                                                            portGroupsBmp,
                                                                            profileIdVal,
                                                                            lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType,
                                                                            &lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction,
                                                                            lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionEn);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)==GT_TRUE)
            {
                /* set profile Id table regardless of the lookupClientMapping configuration
                   this was added to support configuration of packet_type+port / port
                   mode configures by low level API - cpssDxChExactMatchTtiPortProfileIdModeSet */

                for (profileIdVal=PRV_CPSS_DXCH_EXACT_MATCH_MIN_PROFILE_ID_VAL_CNS;
                     profileIdVal<=PRV_CPSS_DXCH_EXACT_MATCH_MAX_PROFILE_ID_VAL_CNS;
                     profileIdVal++)
                {
                    if (lookupPtr->profileEntryParamsArray[profileIdVal].isValidProfileId==GT_TRUE)
                    {
                        /* set key params */
                        rc = prvCpssDxChExactMatchPortGroupProfileKeyParamsSet(devNum,
                                                                               portGroupsBmp,
                                                                               profileIdVal,
                                                                               &lookupPtr->profileEntryParamsArray[profileIdVal].keyParams);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }

                        /* set default entry configuration */
                        rc = prvCpssDxChExactMatchPortGroupProfileDefaultActionSet(devNum,
                                                                                   portGroupsBmp,
                                                                                   profileIdVal,
                                                                                   lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType,
                                                                                   &lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction,
                                                                                   lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionEn);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
            }
        }
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwLookupConfigDisbale function
* @endinternal
*
* @brief   This function disable all lookup configuration from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] lookupPtr       - (pointer to) the Exact Match Manager lookup config.
* @param[in] devNum          - device number.
* @param[in] portGroupsBmp   - port group bitmap
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwLookupConfigDisbale
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC     *lookupPtr,
    IN GT_U8                                        devNum,
    IN GT_PORT_GROUPS_BMP                           portGroupsBmp
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  lookupNum;
    GT_BOOL                                                 lookupEnable;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                        lookupClient;
    GT_U32                                                  lookupClientMappingsNum;
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT    *lookupClientMappingsArray;
    GT_U32                                                  i;
    CPSS_PCL_DIRECTION_ENT                                  direction;
    GT_PORT_GROUPS_BMP                                      newPortGroupsBmp;

    /* update portGroupsBmp
       both bits of a pair should have the same value - same configuration for pipe1 nad pipe1*/
    newPortGroupsBmp = portGroupsBmp;
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_UPDATE_ODD_BITS_MAC(newPortGroupsBmp);

    for(lookupNum=0;lookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;lookupNum++)
    {
        lookupEnable                =   lookupPtr->lookupsArray[lookupNum].lookupEnable;
        lookupClient                =   lookupPtr->lookupsArray[lookupNum].lookupClient;
        lookupClientMappingsNum     =   lookupPtr->lookupsArray[lookupNum].lookupClientMappingsNum;
        lookupClientMappingsArray   =   lookupPtr->lookupsArray[lookupNum].lookupClientMappingsArray;

        if(lookupEnable==GT_TRUE)
        {
            for(i=0;i<lookupClientMappingsNum;i++)
            {
                /* set mapping of key/packet type to profileId */
                if(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                {
                    rc = prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeSet(devNum,
                                                                                   newPortGroupsBmp,
                                                                                   lookupClientMappingsArray[i].ttiMappingElem.keyType,
                                                                                   (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                                                   GT_FALSE,/*enableExactMatchLookup*/
                                                                                   lookupClientMappingsArray[i].ttiMappingElem.profileId);/* not used in case enableExactMatchLookup = GT_FALSE*/
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                else
                {
                    if (lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
                        direction = CPSS_PCL_DIRECTION_EGRESS_E;
                    else
                    {
                        if ((!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))&&(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: IPCL_2 only supported for AC5P \n");
                        }

                        direction = CPSS_PCL_DIRECTION_INGRESS_E;
                    }
                    rc = prvCpssDxChExactMatchPortGroupPclProfileIdSet(devNum,
                                                                        newPortGroupsBmp,
                                                                        direction,
                                                                        lookupClientMappingsArray[i].pclMappingElem.packetType,
                                                                        lookupClientMappingsArray[i].pclMappingElem.subProfileId,
                                                                        (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                                        GT_FALSE,/*enableExactMatchLookup*/
                                                                        lookupClientMappingsArray[i].pclMappingElem.profileId);/* not used in case enableExactMatchLookup = GT_FALSE*/
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwFlushLookup function
* @endinternal
*
* @brief   This function Flush HW of lookups from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwFlushLookupConfig
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr
)
{
    GT_STATUS           rc;
    GT_BOOL             isFirst = GT_TRUE;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);

    isFirst = GT_FALSE;
    while(rc == GT_OK)
    {
        /* per device+portGrpBmp disable profileId parameters */
        rc = prvCpssDxChExactMatchManagerHwLookupConfigDisbale(&exactMatchManagerPtr->lookupInfo,devNum,portGroupsBmp);

        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwExpandedTableConfigSet function
* @endinternal
*
* @brief   This function set expanded table configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                     - device number.
* @param[in] portGroupsBmp              - port group bitmap
* @param[in] exactMatchExpanderArray    - the Exact Match Manager expanded config array.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwExpandedTableConfigSet
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC    exactMatchExpanderArray[]
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 expandedActionIndex;
    GT_BOOL hwWriteBlock,managerHwWriteBlock;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* holds system recovery information */

    hwWriteBlock=GT_FALSE;
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
      return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || (managerHwWriteBlock == GT_TRUE) )
    {
        hwWriteBlock =GT_TRUE;
    }

    for (expandedActionIndex=0;expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;expandedActionIndex++)
    {
        if(exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)
        {
            rc = prvCpssDxChExactMatchPortGroupExpandedActionSet(devNum, portGroupsBmp, expandedActionIndex,
                                       exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedActionType,
                                       &exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedAction,
                                       &exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedActionOrigin,hwWriteBlock);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* keep reducedMaskArr in the DB to be used later when building the Exact Match Rule */
            cpssOsMemCpy(exactMatchExpanderArray[expandedActionIndex].reducedMaskArr,
                         &PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].reducedMaskArr,
                         sizeof(exactMatchExpanderArray[expandedActionIndex].reducedMaskArr));
        }
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerHwExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table HW for all devices defined on the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[in] paramsPtr             - the expanded action attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*         None
*/
GT_STATUS prvCpssDxChExactMatchManagerHwExpandedActionUpdate
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    GT_STATUS           rc;
    GT_BOOL             isFirst = GT_TRUE;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);

    isFirst = GT_FALSE;
    while(rc == GT_OK)
    {
        /* per device+portGrpBmp : configure Expanded Entry in HW */
        rc = prvCpssDxChExactMatchPortGroupExpandedActionSet(devNum,portGroupsBmp,expandedActionIndex,
                                                             paramsPtr->expandedActionType,
                                                             &paramsPtr->expandedAction,
                                                             &paramsPtr->expandedActionOrigin,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* save the reducedMask built in prvCpssDxChExactMatchPortGroupExpandedActionSet,
           this will be used when setting the Exact Match Rule */
        cpssOsMemCpy(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].reducedMaskArr,
                     (PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.ExpanderInfoArr[expandedActionIndex].reducedMaskArr),
                     sizeof(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].reducedMaskArr));

        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwFlushByHwIndex function
* @endinternal
*
* @brief   This function Flush HW of entry hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] hwIndex                - HW index.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwFlushByHwIndex
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U32                                                   hwIndex
)
{
    GT_STATUS           rc;
    GT_BOOL             isFirst = GT_TRUE;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);

    isFirst = GT_FALSE;
    while(rc == GT_OK)
    {
        /* per device+portGrpBmp : flush entry from the HW */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,portGroupsBmp,hwIndex);

        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);
    }

    return GT_OK;
}


/*
* @internal prvCpssDxChExactMatchManagerHwWriteByHwIndex function
* @endinternal
*
* @brief   This function write HW entry at hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwWriteByHwIndex
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
)
{
    GT_STATUS                                                rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                                   numOfPairs=0;
    GT_U8                                                    devNum;
    GT_PORT_GROUPS_BMP                                       portGroupBmp;

    /* reset pairList array */
    cpssOsMemSet(&pairListArr,0,sizeof(pairListArr));

    /* get the first pair from DB */
    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
    if(rc != GT_OK)
    {
        /* no devices to work with */
        return GT_OK;
    }
    else
    {
        pairListArr[0].devNum = devNum;
        pairListArr[0].portGroupsBmp = portGroupBmp;
        numOfPairs++;
    }

    /* build the pairListArr */
    while (GT_OK == prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp))
    {
        pairListArr[numOfPairs].devNum = devNum;
        pairListArr[numOfPairs].portGroupsBmp = portGroupBmp;
        numOfPairs++;
    }

    /* sanity check */
    if (numOfPairs!=exactMatchManagerPtr->numOfDevices)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "number of pairs counter registerd in the DB is not the same as number of pairs counted from the DB \n");
    }

    rc = prvCpssDxChExactMatchManagerHwWriteSingleEntryFromDbToHwOfNewDevices(exactMatchManagerPtr,
                                                                              calcInfoPtr->dbEntryPtr,
                                                                              pairListArr,
                                                                              numOfPairs);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail to add Exact Match Entry to HW.\n");
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwInitNewDevice function
* @endinternal
*
* @brief   This function do HW initialization from new device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwInitNewDevice(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp
)
{
    GT_STATUS   rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info; /* holds system recovery information */
    GT_BOOL                       managerHwWriteBlock;
    /*
       parameter              |   value                  |   details
     ===========================================================================
     EM size set              |  according to manager DB |  set numOfHwIndexes
                              |                          |
     ===========================================================================
     EM Lookup configuration  |  according to manager DB |
                              |                          |
     ===========================================================================
     EM Expanded configuration|  according to manager DB |
                              |                          |
     ===========================================================================
     EM GLobal Activity       |  according to manager DB | enable/disable
     ===========================================================================
    */

    /************************/
    /* set implicit values  */
    /************************/
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.initDone == GT_FALSE)
    {
        rc = prvCpssDxChExactMatchDbInit(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*******************************************************/
    /* set explicit values , given during 'manager create' */
    /*******************************************************/
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
      return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);

    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) && (managerHwWriteBlock == GT_FALSE))
    {
        /* set capacity info */
        rc = prvCpssDxChCfgEmSizeSet(devNum, exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set lookup info*/
        rc = prvCpssDxChExactMatchManagerHwLookupConfigSet(devNum, portGroupsBmp, &exactMatchManagerPtr->lookupInfo);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set aging info */
        rc = prvCpssDxChExactMatchPortGroupActivityBitEnableSet(devNum, portGroupsBmp, exactMatchManagerPtr->agingInfo.agingRefreshEnable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* set configuration for Exact Match expanded table */
    rc = prvCpssDxChExactMatchManagerHwExpandedTableConfigSet(devNum, portGroupsBmp, exactMatchManagerPtr->exactMatchExpanderArray);
    if(rc != GT_OK)
    {
        return rc;
    }

    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) && (managerHwWriteBlock == GT_FALSE))
    {
        rc = prvCpssDxChExactMatchManagerAacParamSet(devNum);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/*
* @internal buildHwEntry_prvExactMatchEntryTtiActionFormat function
* @endinternal
*
* @brief  function to build the HW format at :
*         exactMatchManagerPtr->tempInfo.exactMatchHwActionArray for
*         IN  format is 'DB format' the OUT format is 'HW format'
*
* @param[in] devNum                 - device number
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvExactMatchEntryTtiActionFormat
(
    IN GT_U8                                                        devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC    *dbEntryPtr
)
{
    GT_STATUS   rc;
    GT_BOOL     enable;
    GT_U32      hwValue;
    GT_U32      *hwDataArr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    hwDataArr =  exactMatchManagerPtr->tempInfo.exactMatchHwActionArray;

    /****************************************/
    /* convert SW to HW  to hardware format */
    /****************************************/

    /* logic taken from ttiActionSip5Logic2HwFormat */

    /****************************************/
    /* In this function we do some validity
       checks we could not do when we saved
       the created the DB format*/
    /****************************************/
    if ((dbEntryPtr->tunnelTerminate == GT_FALSE) &&
        (dbEntryPtr->tunnelHeaderLengthAnchorType == CPSS_DXCH_TUNNEL_HEADER_LENGTH_L4_ANCHOR_TYPE_E) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /****************************************/
    /* convert SW to HW  to hardware format */
    /****************************************/

    hwValue = dbEntryPtr->command;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->userDefinedCpuCode;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E,
        hwValue);

    if(dbEntryPtr->mirrorToIngressAnalyzerIndex!=0)
    {
        hwValue = dbEntryPtr->mirrorToIngressAnalyzerIndex + 1;
        if(0 == CHECK_BITS_DATA_RANGE_MAC(dbEntryPtr->mirrorToIngressAnalyzerIndex + 1, 3))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        hwValue = 0;
    }
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
        hwValue);

    hwValue = dbEntryPtr->redirectCommand;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E,
        hwValue);


    switch (dbEntryPtr->redirectCommand)
    {
    case 1:/*CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E*/
        switch (dbEntryPtr->egressInterface_type)
        {
        case 0:/* port */
            /* target is not trunk */
            hwValue = 0;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E,
                hwValue);


            hwValue = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_PORT_MAC(
                                dbEntryPtr->egressInterface.devPort.hwDevNum,
                                dbEntryPtr->egressInterface.devPort.portNum);
            if(hwValue > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
            {
                /* 6 bits for the port number */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E,
                hwValue);


            hwValue = PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_DEV_MAC(
                        dbEntryPtr->egressInterface.devPort.hwDevNum,
                        dbEntryPtr->egressInterface.devPort.portNum);
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E,
                hwValue);

            /* target not VIDX */
            hwValue = 0;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                hwValue);

            break;
        case 1: /*CPSS_INTERFACE_TRUNK_E*/
            /* target is trunk */
            hwValue = 1;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E,
                hwValue);

            hwValue = dbEntryPtr->egressInterface.trunkId;
            if(hwValue > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
            {
                /* 7 bits for the trunkId */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E,
                hwValue);

            /* target not VIDX */
            hwValue = 0;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                hwValue);
            break;
        case 2: /* CPSS_INTERFACE_VIDX_E */
        case 3: /* CPSS_INTERFACE_VID_E  */
            if(dbEntryPtr->egressInterface_type == 3)/* CPSS_INTERFACE_VID_E */
            {
                hwValue = 0xfff;
            }
            else
            {
                hwValue = dbEntryPtr->egressInterface.vidx;
                if (hwValue > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_VIDX_MAC(devNum))
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E,
                hwValue);

            /* target is VIDX */
            hwValue = 1;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E,
                hwValue);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        hwValue = dbEntryPtr->tunnelStart;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E,
            hwValue);

        if (dbEntryPtr->tunnelStart == 1)
        {
            hwValue = dbEntryPtr->redirectPtr; /* hold tunnelStartPtr */
            if (hwValue > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart )
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E,
                hwValue);

            hwValue = dbEntryPtr->tsPassengerPacketType;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E,
                hwValue);
        }
        else
        {
            hwValue = dbEntryPtr->redirectPtr; /* hold arpPtr */
            if (hwValue > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E,
                hwValue);
        }

        hwValue = dbEntryPtr->vntl2Echo;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E,
            hwValue);

        hwValue = dbEntryPtr->modifyMacDa;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E,
            hwValue);

        hwValue = dbEntryPtr->modifyMacSa;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E,
            hwValue);

        break;
    case 2: /* CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E */
        if(dbEntryPtr->redirectPtr >= PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
                dbEntryPtr->redirectPtr, PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
        }
        hwValue = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_SW_INDEX_TO_LEAF_HW_INDEX_MAC(dbEntryPtr->redirectPtr);

        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E,
            hwValue);
        break;
    case 4: /* CPSS_DXCH_TTI_VRF_ID_ASSIGN_E */
        hwValue = dbEntryPtr->redirectPtr; /* hold vrfId */
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E,
            hwValue);

        break;
    case 0: /* CPSS_DXCH_TTI_NO_REDIRECT_E */
        /*logicFormatPtr->multiPortGroupTtiEnable --> not applicable for Bobcat2; Caelum; Bobcat3; Aldrin2 ! */

        /* set flow id field*/
        hwValue = dbEntryPtr->redirectPtr; /* hold flowId */
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E,
                                hwValue);

        hwValue = dbEntryPtr->iPclUdbConfigTableIndex;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E,
                                hwValue);
        break;
    default:
        /* do nothing */
        break;
    }

    if(dbEntryPtr->redirectCommand != 1)/* !CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E */
    {
        if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            hwValue = dbEntryPtr->pcl1OverrideConfigIndex;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E,
                hwValue);

            hwValue = dbEntryPtr->pcl0_1OverrideConfigIndex;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                hwValue);

            hwValue = dbEntryPtr->pcl0OverrideConfigIndex;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E,
                hwValue);
        }
        else
        {
            /* IPCL-0 is bypassed */
            if (dbEntryPtr->pcl1OverrideConfigIndex)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            hwValue = dbEntryPtr->pcl0_1OverrideConfigIndex;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E,
                hwValue);

            hwValue = dbEntryPtr->pcl0OverrideConfigIndex;
            SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E,
                hwValue);
        }

        hwValue = dbEntryPtr->iPclConfigIndex;
        if (dbEntryPtr->iPclConfigIndex >= PRV_DXCH_IPCL_CONF_TBL_ROW_NUMBER_CNS(devNum))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E,
            hwValue);

        hwValue = BOOL2BIT_MAC(dbEntryPtr->copyReservedAssignmentEnable);
        if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            if (dbEntryPtr->triggerHashCncClient == GT_TRUE)
            {
                hwValue = 1;
            }
        }
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_ENABLE_E,
            hwValue);

        hwValue = dbEntryPtr->copyReserved;
        if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* Add bit#1 in copyreserved for Trigger Hash CNC Client */
            hwValue |= ((BOOL2BIT_MAC(dbEntryPtr->triggerHashCncClient)) << 1);
        }
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E,
            hwValue);
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        hwValue = BOOL2BIT_MAC(dbEntryPtr->triggerHashCncClient);
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP6_10_TTI_ACTION_TABLE_FIELDS_TRIGER_CNC_HASH_CLIENT_E,
            hwValue);
    }

    hwValue = dbEntryPtr->bindToCentralCounter;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        hwValue);


    hwValue = dbEntryPtr->centralCounterIndex;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        hwValue);


    hwValue = dbEntryPtr->bindToPolicerMeter;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
        hwValue);


    hwValue = dbEntryPtr->bindToPolicer;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
        hwValue);

    hwValue = dbEntryPtr->policerIndex;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
        hwValue);

    hwValue = dbEntryPtr->sourceIdSetEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
        hwValue);

    if (dbEntryPtr->sourceIdSetEnable == GT_TRUE)
    {
        hwValue = dbEntryPtr->sourceId;
        if (hwValue > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_SRC_ID_MAC(devNum))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E,
        hwValue);
    }

    hwValue = dbEntryPtr->actionStop;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E,
        hwValue);

    hwValue = dbEntryPtr->bridgeBypass;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E,
        hwValue);

    hwValue = dbEntryPtr->ingressPipeBypass;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
        hwValue);

    hwValue = dbEntryPtr->tag0VlanPrecedence;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
        hwValue);

    hwValue = dbEntryPtr->nestedVlanEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E,
        hwValue);

    hwValue = dbEntryPtr->tag0VlanCmd;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E,
        hwValue);

    hwValue = dbEntryPtr->tag0VlanId;
    PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC(devNum, hwValue);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E,
        hwValue);

    hwValue = dbEntryPtr->tag1VlanCmd;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E,
        hwValue);

    hwValue = dbEntryPtr->tag1VlanId;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E,
        hwValue);

    hwValue = dbEntryPtr->mplsTtl;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E,
        hwValue);

    hwValue = dbEntryPtr->enableDecrementTtl;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E,
        hwValue);

    hwValue = dbEntryPtr->qosPrecedence;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
        hwValue);

    hwValue = dbEntryPtr->qosProfile;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
        hwValue);

    hwValue = dbEntryPtr->modifyDscp;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
        hwValue);

    hwValue = dbEntryPtr->modifyTag0Up;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E,
        hwValue);

    hwValue = dbEntryPtr->keepPreviousQoS;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E,
        hwValue);

    hwValue = dbEntryPtr->trustUp;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E,
        hwValue);

    hwValue = dbEntryPtr->trustDscp;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E,
        hwValue);

    hwValue = dbEntryPtr->trustExp;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E,
        hwValue);

    hwValue = dbEntryPtr->remapDSCP;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E,
        hwValue);

    hwValue = dbEntryPtr->tag0Up;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E,
        hwValue);

    hwValue = dbEntryPtr->tag1UpCommand;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->tag1Up;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E,
        hwValue);

    hwValue = dbEntryPtr->ttPassengerPacketType;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E,
        hwValue);

    hwValue = dbEntryPtr->copyTtlExpFromTunnelHeader;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E,
        hwValue);

    hwValue = dbEntryPtr->tunnelTerminate;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E,
        hwValue);

    hwValue = dbEntryPtr->mplsCommand;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->hashMaskIndex;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E,
        hwValue);

    /* Source ePort Assignment Enable */
    hwValue = dbEntryPtr->sourceEPortAssignmentEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E,
        hwValue);

    /* Source ePort: assigned by TTI entry when
      <Source ePort Assignment Enable> = Enabled */
    hwValue = dbEntryPtr->sourceEPort;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E,
        hwValue);

    hwValue = dbEntryPtr->mplsLLspQoSProfileEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->ttHeaderLength;

    /* for AC5P, if tunnelHeaderLengthAnchor type is profile based, dbEntryPtr->ttHeaderLength
     * represents tti profile table index
     */
    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) || (dbEntryPtr->tunnelHeaderLengthAnchorType!= 2))/* !CPSS_DXCH_TUNNEL_HEADER_LENGTH_PROFILE_BASED_ANCHOR_TYPE_E */
    {
        /* this is the Tunnel header length in units of Bytes. Granularity is in 2 Bytes. */
        if ((hwValue % 2) != 0)/* length of field is checked by SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(...) */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        hwValue = hwValue / 2;
    }

    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E,
            hwValue);

    hwValue = dbEntryPtr->qosUseUpAsIndexEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E,
        hwValue);

    if (dbEntryPtr->qosUseUpAsIndexEnable == 1)/* GT_TRUE */
    {
        if (dbEntryPtr->qosMappingTableIndex >= QOS_MAPPING_TABLE_INDEX_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    hwValue = dbEntryPtr->qosMappingTableIndex;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E,
        hwValue);


    hwValue = dbEntryPtr->setMacToMe;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
        hwValue);

    hwValue = dbEntryPtr->isPtpPacket;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E,
        hwValue);

    if (dbEntryPtr->isPtpPacket == 1)/* GT_TRUE */
    {
        hwValue = dbEntryPtr->ptpTriggerType;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E,
            hwValue);

        hwValue = dbEntryPtr->ptpOffset;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E,
            hwValue);
    }
    else
    {
        hwValue = dbEntryPtr->oamTimeStampEnable;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E,
            hwValue);

        hwValue = dbEntryPtr->oamOffsetIndex;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E,
            hwValue);
    }

    hwValue = dbEntryPtr->oamProcessWhenGalOrOalExistsEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
    SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_OAM_PROCESSING_WHEN_GAL_OR_OAL_EXISTS_E,
    hwValue);

    hwValue = dbEntryPtr->oamProcessEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->oamProfile;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        hwValue);

    hwValue = dbEntryPtr->oamChannelTypeToOpcodeMappingEnable;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
        hwValue);

    /* The global enable bit is retained for BWC.  If enabled, it overrides the TTI Action */
    rc = cpssDxChPclL3L4ParsingOverMplsEnableGet(devNum,&enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (enable == GT_FALSE)
    {
        hwValue = dbEntryPtr->passengerParsingOfTransitMplsTunnelMode;
        SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E,
            hwValue);

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_BOBCAT2_MPLS_TRANSIT_TUNNEL_PARSING_WA_E))
        {
            /* FEr#4300730: MPLS Transit Tunnel Parsing requires setting the Tunnel Header Length in the TTI Action Entry */
            if ((hwValue != 0) && (dbEntryPtr->tunnelTerminate == GT_FALSE) && (dbEntryPtr->ttHeaderLength == 0))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    hwValue = dbEntryPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable;

    /* When setting <Passenger Parsing of Non-MPLS Transit tunnel Enable> == 1 and
       <TT Passenger Type> == MPLS --> unexpected behavior */
    if (hwValue && dbEntryPtr->ttPassengerPacketType == 1) /* translated from CPSS_DXCH_TTI_PASSENGER_MPLS_E in saveEntryFormatParamsToDb_exactMatchEntryTtiFormat */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->rxIsProtectionPath);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->rxProtectionSwitchEnable);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->continueToNextTtiLookup);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->cwBasedPw);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->ttlExpiryVccvEnable);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->pwe3FlowLabelExist);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->pwCwBasedETreeEnable);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->applyNonDataCwCommand);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->tunnelHeaderLengthAnchorType;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->skipFdbSaLookupEnable);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->ipv6SegmentRoutingEndNodeEnable);
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E,
        hwValue);

    hwValue = BOOL2BIT_MAC(dbEntryPtr->exactMatchOverTtiEn);
    hwValue ^= 1;
    SIP6_TTI_HW_ENTRY_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_TTI_ACTION_TABLE_FIELDS_TTI_OVER_EXACT_MATCH_ENABLE_E,
        hwValue);

    return GT_OK;
}

/*
* @internal buildHwEntry_prvExactMatchEntryPclActionFormat function
* @endinternal
*
* @brief  function to build the HW format at :
*         exactMatchManagerPtr->tempInfo.exactMatchHwActionArray for
*         IN  format is 'DB format' the OUT format is 'HW format'
*
* @param[in] devNum                 - device number
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvExactMatchEntryPclActionFormat
(
    IN GT_U8                                                        devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC    *dbEntryPtr
)
{
    GT_U32          hwValue;
    GT_U32          *hwDataArr;

    GT_ETHERADDR    macAddr;
    GT_U32          redirectCmd;
    GT_BOOL         useIpclProfileIndexAndEnablers = GT_FALSE;
    GT_BOOL         usePolicer = GT_TRUE;
    GT_U32          copyReservedMask;

    hwDataArr =  exactMatchManagerPtr->tempInfo.exactMatchHwActionArray;

    /* clear */
    cpssOsMemSet(hwDataArr, 0,(4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    /* convert packet command */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(hwValue, dbEntryPtr->pktCmd);
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->cpuCode;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
        hwValue);

    hwValue = dbEntryPtr->ingressMirrorToAnalyzerIndex;
    SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_20_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E,
            hwValue);

    switch(dbEntryPtr->redirectCmd)
    {
        case 0:/* CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E */
            redirectCmd = 0;
            useIpclProfileIndexAndEnablers = GT_TRUE;
            break;
        case 1:/* CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E*/
            redirectCmd = 1;

            switch (dbEntryPtr->outInterface_type)
            {
                case 0:/*CPSS_INTERFACE_PORT_E*/
                    hwValue = 0;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                        hwValue);

                    hwValue = 0;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E,
                        hwValue);

                    hwValue = dbEntryPtr->outInterface.devPort.portNum;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E,
                        hwValue);

                    hwValue = dbEntryPtr->outInterface.devPort.hwDevNum;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E,
                        hwValue);

                    break;

                case  1:/*CPSS_INTERFACE_TRUNK_E*/
                    hwValue = 0;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                        hwValue);

                    hwValue = 1;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E,
                        hwValue);

                    hwValue = dbEntryPtr->outInterface.trunkId;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E,
                        hwValue);
                    break;
                case 2:/*CPSS_INTERFACE_VIDX_E*/
                case 3:/*CPSS_INTERFACE_VID_E*/
                    hwValue = 1;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E,
                        hwValue);

                    hwValue = (dbEntryPtr->outInterface_type == 3) ? /* 3=CPSS_INTERFACE_VID_E*/
                        0xFFF :/* indication to flood the vlan */
                        dbEntryPtr->outInterface.vidx;
                    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                        SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E,
                        hwValue);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            hwValue = dbEntryPtr->vntL2Echo;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E,
                hwValue);

            hwValue = dbEntryPtr->tunnelStart;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E,
                hwValue);

            if (hwValue)/* tunnel start */
            {
                hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold tunnelPtr*/
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E,
                    hwValue);

                hwValue = dbEntryPtr->tunnelType;
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E,
                    hwValue);

            }
            else    /*ARP pointer*/
            {
                hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold arpPtr*/
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                    hwValue);
            }

            hwValue = dbEntryPtr->modifyMacSa;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E,
                hwValue);

            break;
        case 2:/*CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E*/
            redirectCmd = 2;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            if(dbEntryPtr->redirectPtr >=PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)/* redirectPtr hold routerLttIndex*/
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
                    dbEntryPtr->redirectPtr, PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
            }
            hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold routerLttIndex*/
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                hwValue);

            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_PBR_PACKETS_TREATED_AS_UNKNOWN_DA_WA_E))
            {
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
                    1);
            }

            if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                hwValue = dbEntryPtr->pbrMode;
                SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum, hwDataArr,
                     SIP5_20_IPCL_ACTION_TABLE_FIELDS_PBR_MODE_E,
                     hwValue);
            }
            break;
        case 3:/*CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E*/
            redirectCmd = 4;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold vrfId*/
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E,
                hwValue);
            break;
        case 4:/*CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E*/
            redirectCmd = 6;
            useIpclProfileIndexAndEnablers = GT_FALSE;
            usePolicer = GT_FALSE;

            hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold arpPtr*/
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E,
                hwValue);

            /* MAC address */
            macAddr.arEther[0] = (GT_U8)(dbEntryPtr->macSa_high_16 >>  8);
            macAddr.arEther[1] = (GT_U8)(dbEntryPtr->macSa_high_16 >>  0);
            macAddr.arEther[2] = (GT_U8)(dbEntryPtr->macSa_low_32  >> 24);
            macAddr.arEther[3] = (GT_U8)(dbEntryPtr->macSa_low_32  >> 16);
            macAddr.arEther[4] = (GT_U8)(dbEntryPtr->macSa_low_32  >>  8);
            macAddr.arEther[5] = (GT_U8)(dbEntryPtr->macSa_low_32  >>  0);

            /* MAC_SA_27_0 - 28 bits */
            hwValue = 0x0FFFFFFF &
                  (macAddr.arEther[5]
                | (macAddr.arEther[4] << 8)
                | (macAddr.arEther[3] << 16)
                | (macAddr.arEther[2] << 24));
            SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E,
                hwValue);

            /* MAC_SA_47_28 - 20 bits */
            hwValue = 0x000FFFFF &
                 ((macAddr.arEther[2]
                | (macAddr.arEther[1] << 8)
                | (macAddr.arEther[0] << 16)) >> 4);
            SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E,
                hwValue);

            break;

        case 6: /*CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E*/
            if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            redirectCmd = 7;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            if(dbEntryPtr->redirectPtr >=PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)/* redirectPtr hold routerLttIndex*/
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
                    dbEntryPtr->redirectPtr, PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
            }

            hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold routerLttIndex*/
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                hwValue);

            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_PBR_PACKETS_TREATED_AS_UNKNOWN_DA_WA_E))
            {
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
                    1);
            }

            if(dbEntryPtr->vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vrfId[%d] fineTuning.tableSize.vrfIdNum[%d]",
                    dbEntryPtr->vrfId, PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum);
            }
            hwValue = dbEntryPtr->vrfId;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E,
                hwValue);

            break;

      case 7: /*CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E*/
            if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }

            redirectCmd = 2;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold routerLttIndex*/

            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                hwValue);

            hwValue = dbEntryPtr->pbrMode;
            SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum, hwDataArr,
                 SIP5_20_IPCL_ACTION_TABLE_FIELDS_PBR_MODE_E,
                 hwValue);

            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_PBR_PACKETS_TREATED_AS_UNKNOWN_DA_WA_E))
            {
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
                    1);
            }
            break;

        case 8: /*CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E*/
            if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            redirectCmd = 7;
            useIpclProfileIndexAndEnablers = GT_TRUE;

            hwValue = dbEntryPtr->redirectPtr;/* redirectPtr hold routerLttIndex*/

            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E,
                hwValue);

            if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT2_PBR_PACKETS_TREATED_AS_UNKNOWN_DA_WA_E))
            {
                SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                    SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E,
                    1);
            }

            if(dbEntryPtr->vrfId >= PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vrfId[%d] fineTuning.tableSize.vrfIdNum[%d]",
                    dbEntryPtr->vrfId, PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.vrfIdNum);
            }
            hwValue = dbEntryPtr->vrfId;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E,
                hwValue);

            hwValue = dbEntryPtr->pbrMode;
            SIP_5_20_IPCL_ACTION_FIELD_SET_MAC(devNum, hwDataArr,
                 SIP5_20_IPCL_ACTION_TABLE_FIELDS_PBR_MODE_E,
                 hwValue);

            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E,
        redirectCmd);

    if(useIpclProfileIndexAndEnablers == GT_TRUE)
    {
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
        {
            hwValue = dbEntryPtr->pcl0_1OverrideConfigIndex;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E,
                hwValue);

            hwValue = dbEntryPtr->pcl1OverrideConfigIndex;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                hwValue);
        }
        else
        {
            hwValue = dbEntryPtr->pcl0_1OverrideConfigIndex;
            SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E,
                hwValue);

            hwValue = dbEntryPtr->pcl1OverrideConfigIndex;
            if (hwValue != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        hwValue = dbEntryPtr->ipclConfigIndex;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E,
            hwValue);
    }

    hwValue = dbEntryPtr->bindToCncCounter;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        hwValue);

    hwValue = dbEntryPtr->cncCounterIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        hwValue);

    if (usePolicer != GT_FALSE)
    {
        hwValue = dbEntryPtr->bindToPolicerMeter;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E,
            hwValue);

        hwValue = dbEntryPtr->bindToPolicerCounter;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
            hwValue);

        hwValue = dbEntryPtr->policerIndex;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
            SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E,
            hwValue);
    }

    hwValue = dbEntryPtr->assignSourceId;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->sourceIdValue;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E,
        hwValue);

    hwValue = dbEntryPtr->actionStop;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E,
        hwValue);

    hwValue = dbEntryPtr->bypassBridge;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E,
        hwValue);

    hwValue = dbEntryPtr->bypassIngressPipe;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E,
        hwValue);

    hwValue = dbEntryPtr->precedence;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E,
        hwValue);

    hwValue = dbEntryPtr->nestedVlan;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E,
        hwValue);

    hwValue = dbEntryPtr->modifyVlan;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->vlanId;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E,
        hwValue);

    hwValue =dbEntryPtr->profileAssignIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->profilePrecedence;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E,
        hwValue);

    hwValue = dbEntryPtr->profileIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E,
        hwValue);

    hwValue = dbEntryPtr->modifyDscp;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E,
        hwValue);

    /* enable modify UP */
    hwValue = dbEntryPtr->modifyUp;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E,
        hwValue);

    hwValue = dbEntryPtr->mirrorTcpRstAndFinPacketsToCpu;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E,
        hwValue);

    hwValue = dbEntryPtr->modifyMacDa;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E,
        hwValue);

    hwValue = dbEntryPtr->vlanId1Cmd;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E,
        hwValue);

    hwValue = dbEntryPtr->up1Cmd;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E,
        hwValue);

    hwValue = dbEntryPtr->vlanId1;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E,
        hwValue);

    hwValue = dbEntryPtr->up1;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E,
        hwValue);

    hwValue = dbEntryPtr->flowId;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
        hwValue);

    hwValue = dbEntryPtr->setMacToMe;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E,
        hwValue);

    hwValue = dbEntryPtr->oamTimeStampEnable;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E,
        hwValue);

    hwValue = dbEntryPtr->oamOffsetIndex;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E,
        hwValue);

    hwValue = dbEntryPtr->oamProcessEnable;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E,
        hwValue);

    hwValue = dbEntryPtr->oamProfile;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        hwValue);

    hwValue = dbEntryPtr->assignSourcePortEnable;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E,
        hwValue);

    hwValue = dbEntryPtr->sourcePortValue;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E,
        hwValue);

    hwValue = dbEntryPtr->skipFdbSaLookup;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_IPCL_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_EN_E,
        hwValue);

    hwValue = dbEntryPtr->triggerInterrupt;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_IPCL_ACTION_TABLE_FIELDS_TRIGGER_INTERRUPT_EN_E,
        hwValue);

    hwValue = dbEntryPtr->exactMatchOverPclEn;
    hwValue ^= 1;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_IPCL_ACTION_TABLE_FIELDS_PCL_OVER_EXACT_MATCH_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->monitoringEnable;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->latencyProfile;
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_IPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
        hwValue);


    hwValue = dbEntryPtr->copyReservedAssignEnable;
    if(!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        if (dbEntryPtr->triggerHashCncClient == GT_TRUE)
        {
            hwValue = 1;
        }
    }
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                   SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E,
                                   hwValue);

    copyReservedMask = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? BIT_19 : BIT_18;
    if(dbEntryPtr->copyReserved >= copyReservedMask)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* CPSS configures only bits [19:2] for SIP_5 and Falcon and bits [19:1]
     * for SIP_6_10 and above. Other bits are reserved */
    hwValue = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? dbEntryPtr->copyReserved << 1 :
                                                    dbEntryPtr->copyReserved << 2;

    if( (PRV_CPSS_SIP_6_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)) )
    {
        /* Add bit#1 in copyreserved for Trigger Hash CNC Client */
        hwValue |= ((BOOL2BIT_MAC(dbEntryPtr->triggerHashCncClient)) << 1);
    }
    SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                   SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E,
                                   hwValue);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        hwValue = BOOL2BIT_MAC(dbEntryPtr->triggerHashCncClient);
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                       SIP6_10_IPCL_ACTION_TABLE_FIELDS_FLOW_TRACK_ENABLE_E,
                                       hwValue);
    }

    return GT_OK;
}
/*
* @internal buildHwEntry_prvExactMatchEntryEpclActionFormat function
* @endinternal
*
* @brief  function to build the HW format at :
*         exactMatchManagerPtr->tempInfo.exactMatchHwActionArray for
*         IN  format is 'DB format' the OUT format is 'HW format'
*
* @param[in] devNum                 - device number
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS buildHwEntry_prvExactMatchEntryEpclActionFormat
(
    IN GT_U8                                                        devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC   *dbEntryPtr
)
{
    GT_U32      hwValue;
    GT_U32      *hwDataArr;
    GT_U32      copyReservedMask;

    hwDataArr =  exactMatchManagerPtr->tempInfo.exactMatchHwActionArray;

    /* clear */
    cpssOsMemSet(hwDataArr, 0,(4 * CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS));

    hwValue = dbEntryPtr->pktCmd;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E,
        hwValue);

    hwValue = dbEntryPtr->cpuCode;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E,
        hwValue);

    hwValue = dbEntryPtr->egressMirrorToAnalyzerIndex;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E,
        hwValue);

    hwValue = dbEntryPtr->egressMirrorToAnalyzerMode;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E,
        hwValue);
#if 0
    if (dbEntryPtr->phaThreadNumberAssignmentEnable!=0)
    {
        /******************************************/
        /* check that the PHA LIB was initialized */
        /******************************************/
        PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
        if (dbEntryPtr->phaThreadNumber!=0)
        {
             /* check the index to the table */
            PRV_CPSS_DXCH_PHA_THREAD_ID_CHECK_MAC(devNum,dbEntryPtr->phaThreadNumber);
        }
    }
#endif
    hwValue = dbEntryPtr->phaThreadNumberAssignmentEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->phaThreadNumber;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E,
        hwValue);

    switch(dbEntryPtr->phaThreadType)
    {
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E:
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E:
            /******************************************/
            /* check that the PHA LIB was initialized */
            /******************************************/
            PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_E:
            /******************************************/
            /* check that the PHA LIB was initialized */
            /******************************************/
            PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);
            PRV_CPSS_DXCH_PHA_ERSPAN_III_FT_CHECK_MAC(devNum,((dbEntryPtr->phaMetadata >> 26) & 0x1F));
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            /******************************************/
            /* check that the PHA LIB was initialized */
            /******************************************/
            PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(devNum);

            PRV_CPSS_DXCH_PHA_CLASSIFIER_NSH_OVER_VXLAN_GPE_TENANT_ID_CHECK_MAC(devNum,((dbEntryPtr->phaMetadata >> 16) & 0xFFFF));
            PRV_CPSS_DXCH_PHA_CLASSIFIER_NSH_OVER_VXLAN_GPE_SOURCE_CLASS_CHECK_MAC(devNum,((dbEntryPtr->phaMetadata) & 0xFFFF));
            break;
         default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->phaThreadType);
    }

    hwValue = dbEntryPtr->phaMetadataAssignEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->phaMetadata;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E,
        hwValue);

    hwValue = dbEntryPtr->dscp;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E,
        hwValue);

    hwValue = dbEntryPtr->up;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E,
        hwValue);

    hwValue = dbEntryPtr->modifyDscp;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E,
        hwValue);

    hwValue = dbEntryPtr->modifyUp;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E,
        hwValue);

    hwValue = dbEntryPtr->bindToCncCounter;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E,
        hwValue);

    hwValue = dbEntryPtr->cncCounterIndex;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E,
        hwValue);

    hwValue = dbEntryPtr->vlanId1ModifyEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E,
        hwValue);

    hwValue = dbEntryPtr->up1ModifyEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E,
        hwValue);

    hwValue = dbEntryPtr->vlanId1;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E,
        hwValue);

    hwValue = dbEntryPtr->up1;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E,
        hwValue);

    hwValue = dbEntryPtr->vlanCmd;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E,
        hwValue);

    hwValue = dbEntryPtr->vlanId;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E,
        hwValue);

    hwValue = dbEntryPtr->bindToPolicerCounter;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E,
        hwValue);

    hwValue = dbEntryPtr->bindToPolicerMeter;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E,
        hwValue);

    hwValue = dbEntryPtr->policerIndex;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E,
        hwValue);

    hwValue = dbEntryPtr->flowId;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E,
        hwValue);

    hwValue = dbEntryPtr->oamProcessEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->oamProfile;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E,
        hwValue);

    hwValue = dbEntryPtr->oamTimeStampEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->oamOffsetIndex;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E,
        hwValue);

    hwValue = dbEntryPtr->channelTypeToOpcodeMapEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E,
        hwValue);

    hwValue = dbEntryPtr->terminateCutThroughMode;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E,
        hwValue);

    hwValue = dbEntryPtr->exactMatchOverPclEn;
    hwValue ^= 1;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_PCL_OVER_EXACT_MATCH_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->monitoringEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E,
        hwValue);

    hwValue = dbEntryPtr->latencyProfile;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
        SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E,
        hwValue);


    hwValue = dbEntryPtr->copyReservedAssignEnable;
    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                   SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_ENABLE_E,
                                   hwValue);

    copyReservedMask = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? BIT_19 : BIT_18;
    if(dbEntryPtr->copyReserved >= copyReservedMask)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* CPSS configures only bits [19:2] for SIP_5 and Falcon and bits [19:1]
     * for SIP_6_10 and above. Other bits are reserved */
    hwValue = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? dbEntryPtr->copyReserved << 1 :
                                                    dbEntryPtr->copyReserved << 2;

    SIP5_EPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                   SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_E,
                                   hwValue);


    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        hwValue = BOOL2BIT_MAC(dbEntryPtr->enableEgressMaxSduSizeCheck);
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                       SIP6_30_EPCL_ACTION_TABLE_FIELDS_ENABLE_EGRESS_MAX_SDU_SIZE_CHECK_E,
                                       hwValue);

        hwValue = dbEntryPtr->egressMaxSduSizeProfile;
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                       SIP6_30_EPCL_ACTION_TABLE_FIELDS_EGRESS_MAX_SDU_SIZE_PROFILE_E,
                                       hwValue);

        switch (dbEntryPtr->egressCncIndexMode)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_HIT_COUNTER_E:
                hwValue = 0;
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_CNC_INDEX_MODE_MAX_SDU_PASS_FAIL_E:
                hwValue = 1;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        SIP5_IPCL_ACTION_FIELD_SET_MAC(devNum,hwDataArr,
                                       SIP6_30_EPCL_ACTION_TABLE_FIELDS_EGRESS_CNC_INDEX_MODE_E,
                                       hwValue);
    }

    return GT_OK;
}


/*
* @internal prvCpssDxChExactMatchManagerHwBuildActionFormatFromDbEntry function
* @endinternal
*
* @brief  function to build the HW Action format at : exactMatchManagerPtr->tempInfo.exactMatchHwActionArray
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] devNum                 - device number
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChExactMatchManagerHwBuildActionFormatFromDbEntry
(
    IN GT_U8                                                    devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    cpssOsMemSet(exactMatchManagerPtr->tempInfo.exactMatchHwActionArray, 0, sizeof(exactMatchManagerPtr->tempInfo.exactMatchHwActionArray));
    switch(dbEntryPtr->hwExactMatchEntryType)
    {
        case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E:
            return buildHwEntry_prvExactMatchEntryTtiActionFormat(devNum,exactMatchManagerPtr,&dbEntryPtr->specificFormat.prvTtiEntryFormat);
        case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_PCL_E:
            return buildHwEntry_prvExactMatchEntryPclActionFormat(devNum,exactMatchManagerPtr,&dbEntryPtr->specificFormat.prvPclEntryFormat);
        case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_EPCL_E:
            return buildHwEntry_prvExactMatchEntryEpclActionFormat(devNum,exactMatchManagerPtr,&dbEntryPtr->specificFormat.prvEpclEntryFormat);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwExactMatchEntryType);
    }
}

/*
* @internal prvCpssDxChExactMatchManagerHwBuildReducedEntryFormatFromDbEntry function
* @endinternal
*
* @brief  function to build the HW Action format at : exactMatchManagerPtr->tempInfo.exactMatchHwReducedActionDataArr
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] devNum                 - device number
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChExactMatchManagerHwBuildReducedEntryFormatFromDbEntry
(
    IN GT_U32                                                   devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    cpssOsMemSet(exactMatchManagerPtr->tempInfo.exactMatchHwReducedActionDataArr, 0,
                 (sizeof(GT_U8)*PRV_CPSS_DXCH_EXACT_MATCH_REDUCED_ACTION_SIZE_CNS));

    return prvCpssDxChExactMatchReducedToHwformat(devNum,
                      exactMatchManagerPtr->tempInfo.exactMatchHwActionArray, /* build in prvCpssDxChExactMatchManagerHwBuildActionFormatFromDbEntry*/
                      exactMatchManagerPtr->exactMatchExpanderArray[dbEntryPtr->hwExactMatchExpandedActionIndex].reducedMaskArr, /* taken from DB Manager*/
                      exactMatchManagerPtr->tempInfo.exactMatchHwReducedActionDataArr);
}
/*
* @internal prvCpssDxChExactMatchManagerHwBuildKeyEntryFormatFromDbEntry function
* @endinternal
*
* @brief  function to build the HW Action format at : exactMatchManagerPtr->tempInfo.exactMatchKeyEntry
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChExactMatchManagerHwBuildKeyEntryFormatFromDbEntry
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    exactMatchManagerPtr->tempInfo.exactMatchEntry.lookupNum = dbEntryPtr->hwExactMatchLookupNum;
    exactMatchManagerPtr->tempInfo.exactMatchEntry.key.keySize = dbEntryPtr->hwExactMatchKeySize;
    cpssOsMemCpy(&exactMatchManagerPtr->tempInfo.exactMatchEntry.key.pattern,&dbEntryPtr->pattern,sizeof(dbEntryPtr->pattern));
    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerHwBuildEntryFormatFromDbEntry function
* @endinternal
*
* @brief  function to build the HW Action format at : exactMatchManagerPtr->tempInfo.hwDataArr
*         the IN  format is 'DB format'
*         the OUT format is 'HW format'
*
* @param[in] devNum                 - device number
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChExactMatchManagerHwBuildEntryFormatFromDbEntry
(
    IN GT_U8                                                    devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    GT_STATUS rc = GT_OK;

    rc = prvCpssDxChExactMatchEntryToHwformat(devNum,
                                           dbEntryPtr->hwExactMatchExpandedActionIndex,
                                           &exactMatchManagerPtr->tempInfo.exactMatchEntry,                  /* build in prvCpssDxChExactMatchManagerHwBuildKeyEntryFormatFromDbEntry*/
                                           exactMatchManagerPtr->tempInfo.exactMatchHwReducedActionDataArr,/* build in prvCpssDxChExactMatchManagerHwBuildReducedEntryFormatFromDbEntry */
                                           exactMatchManagerPtr->tempInfo.hwDataArr);
    if (rc!=GT_OK)
    {
        return rc;
    }
    /* if we are in SER case meaning we are rewriting the entry -
       need to set the ageBit to 1 */
    if (dbEntryPtr->age == 1)
    {
        /*  bit 2
            The bit is set by the device when the entry is matched in the Exact Match Lookup.
            0x0 = Not Refreshed
            0x1 = Refreshed */
        SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,exactMatchManagerPtr->tempInfo.hwDataArr,
                                             SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E,
                                             1);/* set Activity bit - refreshed entry */
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerHwWriteSingleEntryFromDbToHwOfNewDevices function
* @endinternal
*
* @brief  This function start to write entries from manager to HW of the new devices
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  -(pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr            -(pointer to) Exact Match entry in DB format
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwWriteSingleEntryFromDbToHwOfNewDevices(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_STATUS           rc;
    GT_U32              ii;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              portGroupId;    /* port group Id        */

    GT_U32      singleHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */
    GT_U32      startWord, entryIndexPerBank;
    GT_U32      bankNum=0,numOfBanks=0;

    GT_U32                                  numActivePortGroups=0;
    CPSS_SYSTEM_RECOVERY_INFO_STC           oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_BOOL                                 tempSystemRecoveryUsed=GT_FALSE;
    GT_BOOL                                 managerHwWriteBlock,managerOldStatus;

    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        EM_LOCK_DEV_NUM(pairListArr[ii].devNum);

        /* write the same format to all devices */
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
        if (rc != GT_OK)
        {
            return rc;
        }
        newSystemRecoveryInfo = oldSystemRecoveryInfo;
        managerOldStatus = GT_FALSE;
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
        if (((oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
            (oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || managerHwWriteBlock ==GT_TRUE)
        {
                /* if we are in a case that the manager manage more then one tile then we need
                   to make sure all tiles have the same data - so we write data to HW.
                   if this is a single tie case then no need to write to HW */

                /* check if portGroupsBmp have more then one bit on */
                rc = prvCpssPpConfigNumActivePortGroupsInBmpGet(devNum,portGroupsBmp,&numActivePortGroups);
                if ((rc!=GT_OK)||(numActivePortGroups==0))
                {
                    EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in portsGroupsBmp");
                }

                if (numActivePortGroups>1)/* multi device portGroup */
                {
                    /* save current recovery state & managerStatus and make it complete - to enable HW write temporally

                     if the state is CATCH_UP then the automatic write mechanism  will not write to HW.
                     in HA case we need to set the value again to HW, to all dev/tiles members, to verify
                     the data is the same in all tiles */
                    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
                    newSystemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
                    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

                    managerOldStatus = PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted;
                    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = GT_FALSE;

                    /* set flag - need to reconstruct back to the original state after the HW write */
                    tempSystemRecoveryUsed = GT_TRUE;
                }
        }
        /* check that the hwIndex is suppoted in the currect device */
        /* check validity of Exact Match Entry index */
        rc = prvCpssDxChExactMatchIndexValidity(devNum,dbEntryPtr->hwIndex,(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT)(dbEntryPtr->hwExactMatchKeySize));
        if (rc  != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "illegal Exact Match Index \n");
        }

        cpssOsMemSet(singleHwDataArr, 0, sizeof(singleHwDataArr));

        /* build the HW format at : exactMatchManagerPtr->tempInfo.exactMatchHwActionArray */
        rc = prvCpssDxChExactMatchManagerHwBuildActionFormatFromDbEntry(devNum,exactMatchManagerPtr,dbEntryPtr);
        if(rc != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            return rc;
        }
        /* build the reduce entry to set into HW exactMatchManagerPtr->tempInfo.exactMatchHwReducedActionDataArr */
        rc = prvCpssDxChExactMatchManagerHwBuildReducedEntryFormatFromDbEntry(pairListArr[ii].devNum,
                                                                              exactMatchManagerPtr,dbEntryPtr);
        if (rc  != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            return rc;
        }

        /* build entry key format
           build the key entry to set into HW exactMatchManagerPtr->tempInfo.exactMatchEntry */
        prvCpssDxChExactMatchManagerHwBuildKeyEntryFormatFromDbEntry(exactMatchManagerPtr,dbEntryPtr);
        if(rc != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            return rc;
        }
        /* build entry hw format key + ReducedAction
           build the reduce entry to set into HW exactMatchManagerPtr->tempInfo.hwDataArr */
        rc = prvCpssDxChExactMatchManagerHwBuildEntryFormatFromDbEntry(devNum,exactMatchManagerPtr,dbEntryPtr);
        if(rc != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            return rc;
        }

        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
        rc = prvCpssPpConfigNumActivePortGroupsInBmpGet(devNum,portGroupsBmp,&numActivePortGroups);
        if(rc != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            return rc;
        }

        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, dbEntryPtr->hwExactMatchKeySize);

            for (bankNum=0; bankNum<numOfBanks; bankNum++)
            {
                startWord = bankNum*4;
                entryIndexPerBank = dbEntryPtr->hwIndex + bankNum;

                cpssOsMemCpy(singleHwDataArr, &exactMatchManagerPtr->tempInfo.hwDataArr[startWord], sizeof(singleHwDataArr));

                if(numActivePortGroups >= 4)
                {
                    /*Using AAC mechanism to accelerate entry insertion */
                    rc = prvCpssDxChExactMatchManagerAacHwWriteEntry(devNum,CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,entryIndexPerBank, &singleHwDataArr[0]);
                }
                else
                {
                    /* write entry to specific index format */
                    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,portGroupId,
                                                            CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                                            entryIndexPerBank,
                                                            &singleHwDataArr[0]);
                }
                if(rc != GT_OK)
                {
                    EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
                    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
                    return rc;
                }
            }
            if (numActivePortGroups >= 4)
            {   /*aac wrote the entry to all tiles so no need to continue PortGroups loop */
                break;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        if (tempSystemRecoveryUsed==GT_TRUE)
        {
            /* restore back the HA recovery state */
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            tempSystemRecoveryUsed = GT_FALSE;
        }
        EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwReadSingleEntryFromHw function
* @endinternal
*
* @brief  This function read a single entry from HW of the devices
*         NOTE: this function is used for SER purpose only to trigger the event in case of
*         corrupted entries. We need a HW read to trigger the event
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  -(pointer to) the Exact Match Manager.
* @param[in] dbEntryPtr            -(pointer to) Exact Match entry in DB format
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
static GT_STATUS    prvCpssDxChExactMatchManagerHwReadSingleEntryFromHw(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs,
    OUT GT_U32                                                  *hwDataArr
)
{
    GT_STATUS           rc;
    GT_U32              ii;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_U32              portGroupId;    /* port group Id        */

    GT_U32      singleHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */
    GT_U32      startWord, entryIndexPerBank;
    GT_U32      bankNum=0,numOfBanks=0;

    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        EM_LOCK_DEV_NUM(pairListArr[ii].devNum);

        /* read all devices */
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        /* check that the hwIndex is suppoted in the currect device */
        /* check validity of Exact Match Entry index */
        rc = prvCpssDxChExactMatchIndexValidity(devNum,
                                                dbEntryPtr->hwIndex,
                                                (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT)(dbEntryPtr->hwExactMatchKeySize));
        if (rc  != GT_OK)
        {
            EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "illegal Exact Match Index \n");
        }

        cpssOsMemSet(singleHwDataArr, 0, sizeof(singleHwDataArr));

        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, dbEntryPtr->hwExactMatchKeySize);

            for (bankNum=0; bankNum<numOfBanks; bankNum++)
            {
                startWord = bankNum*4;
                entryIndexPerBank = dbEntryPtr->hwIndex + bankNum;

                /* write entry to specific index format */
                rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                                        CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                                        entryIndexPerBank,
                                                        &singleHwDataArr[0]);
                if(rc != GT_OK)
                {
                    EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
                    return rc;
                }
               cpssOsMemCpy(&hwDataArr[startWord], singleHwDataArr, sizeof(singleHwDataArr));
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        EM_UNLOCK_DEV_NUM(pairListArr[ii].devNum);
    }
    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerHwReadByHwIndex function
* @endinternal
*
* @brief   This function read HW entry at hwIndex from all registered devices in the manager.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] calcInfoPtr            - using calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - convert of manager format to HW format failed.
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwReadByHwIndex
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
)
{
    GT_STATUS                                                rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                                   numOfPairs=0;
    GT_U8                                                    devNum;
    GT_PORT_GROUPS_BMP                                       portGroupBmp;

    GT_U32      hwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS];/* hw data to set entryType/Age/keySize/lookupNum/key/reduccedAction/ */

    /* reset pairList array */
    cpssOsMemSet(&pairListArr,0,sizeof(pairListArr));

    /* get the first pair from DB */
    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
    if(rc != GT_OK)
    {
        /* no devices to work with */
        return GT_OK;
    }
    else
    {
        pairListArr[0].devNum = devNum;
        pairListArr[0].portGroupsBmp = portGroupBmp;
        numOfPairs++;
    }

    /* build the pairListArr */
    while (GT_OK == prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp))
    {
        pairListArr[numOfPairs].devNum = devNum;
        pairListArr[numOfPairs].portGroupsBmp = portGroupBmp;
        numOfPairs++;
    }

    /* sanity check */
    if (numOfPairs!=exactMatchManagerPtr->numOfDevices)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "number of pairs counter registerd in the DB is not the same as number of pairs counted from the DB \n");
    }
    rc = prvCpssDxChExactMatchManagerHwReadSingleEntryFromHw(calcInfoPtr->dbEntryPtr,
                                                             pairListArr,
                                                             numOfPairs,
                                                             hwDataArr);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail to read Exact Match Entry to HW.\n");
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwWriteEntriesFromDbToHwOfNewDevices function
* @endinternal
*
* @brief  This function start to write entries from manager to HW of the new devices
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwWriteEntriesFromDbToHwOfNewDevices(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_STATUS   rc;
    GT_BOOL     getFirst = GT_TRUE;
    GT_U32      bankNum=0,numOfBanks=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    /* loop on all entries in the Exact Match manager */
    while(GT_OK ==
          prvCpssDxChExactMatchManagerDbEntryIteratorGetNext(exactMatchManagerPtr,getFirst,&dbEntryPtr))
    {
         /* in HA process when adding a device the exact match entries database should be empty
            the application must first add the device and only then to replay all entries */
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "fail to cpssSystemRecoveryStateGet \n");
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
        if (((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
             (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || (managerHwWriteBlock == GT_TRUE) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "fail in HA process, entries were replayed before device was added\n");
        }

        getFirst = GT_FALSE;/* get next from here on ... */

        rc = prvCpssDxChExactMatchManagerHwWriteSingleEntryFromDbToHwOfNewDevices(exactMatchManagerPtr, dbEntryPtr, pairListArr, numOfPairs);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "fail to write Exact Match entry to HW \n");
        }

        PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, dbEntryPtr->hwExactMatchKeySize);
        /* loop from 1 since the entry for bank 0 was already returned by the getNext in the while loop*/
        for (bankNum=1; bankNum<numOfBanks; bankNum++)
        {
            /* need to update the pointer of the next element in the DB,
               since the real data is only held in first element.
               the rest of the elements hold only data regarding the validity
               of the index - if it is used or not.
               for instance if the entry is with keySize=4 the dbEntryPtr->hwIndex
               will hold the data we should set into HW and
               dbEntryPtr->hwIndex+1, dbEntryPtr->hwIndex+2, dbEntryPtr->hwIndex+3
               will only be configured as taken but with no real data kept in the DB */
            rc =prvCpssDxChExactMatchManagerDbEntryIteratorGetNext(exactMatchManagerPtr,getFirst,&dbEntryPtr);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "can not jump to the next valid entry \n");
            }
        }
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwFlushAll function
* @endinternal
*
* @brief   This function Flush all entries from the HW from all registered devices in the manager.
*          NOTE this function not update the DB itself ,the Caller responsible for it.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwFlushAll
(
  IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr
)
{
    GT_STATUS                                               rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_BOOL                                                 getFirst = GT_TRUE;

    /* check if there are registered devices on the manager */
    if(!prvCpssDxChExactMatchManagerDbIsAnyDeviceRegistred(exactMatchManagerPtr))
    {
        /* no device to flush from */
        return GT_OK;
    }

    /* loop on all entries in the Exact Match manager */
    while(GT_OK ==
          prvCpssDxChExactMatchManagerDbEntryIteratorGetNext(exactMatchManagerPtr,getFirst,&dbEntryPtr))
    {
        getFirst = GT_FALSE;/* get next from here on ... */

        /********************************************************************/
        /* flush the entry from HW of all registered devices                */
        /********************************************************************/
        rc = prvCpssDxChExactMatchManagerHwFlushByHwIndex(exactMatchManagerPtr,dbEntryPtr->hwIndex);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* if this is the first bank index, then decrement the counter of expandedActionIndex
           update DB with the correct number of exact match bounded to Expander index */
        if(dbEntryPtr->isFirst==GT_TRUE)
        {
            if(exactMatchManagerPtr->exactMatchExpanderArray[dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)
            {
                if (exactMatchManagerPtr->exactMatchExpanderArray[dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter!=0)
                {
                    exactMatchManagerPtr->exactMatchExpanderArray[dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter--;
                }
                else
                {
                    /* should not happen */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR: exactMatchEntriesCounter[%d] can not be decremented.\n",
                                                  exactMatchManagerPtr->exactMatchExpanderArray[dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter);
                }
            }
            else
            {
                /* should never happen since if we delete an entry it must be pointed to a valid expander index */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR: exactMatchExpanderArray[%d] is not valid.\n",dbEntryPtr->hwExactMatchExpandedActionIndex);
            }
        }
    }

    /* disable exact match on all lookups */
    prvCpssDxChExactMatchManagerHwFlushLookupConfig(exactMatchManagerPtr);

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwFlushAllSpecificDevice function
* @endinternal
*
* @brief   This function Flush all entries from the HW from a specific devNum+portGroupsBmp.
*          NOTE this function not update the DB itself ,the Caller responsible for it.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
* @param[in] devNum                 - device number.
* @param[in] portGroupsBmp          - port group bitmap
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwFlushAllSpecificDevice
(
  IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC *exactMatchManagerPtr,
  IN GT_U8                                                  devNum,
  IN GT_PORT_GROUPS_BMP                                     portGroupsBmp
)
{
    GT_STATUS                                               rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_BOOL                                                 getFirst = GT_TRUE;

    /* loop on all entries in the Exact Match manager */
    while(GT_OK ==
          prvCpssDxChExactMatchManagerDbEntryIteratorGetNext(exactMatchManagerPtr,getFirst,&dbEntryPtr))
    {
        getFirst = GT_FALSE;/* get next from here on ... */

        /* per device+portGrpBmp : flush entry from the HW */
        rc = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,portGroupsBmp,dbEntryPtr->hwIndex);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* disable exact match on all lookups */
    prvCpssDxChExactMatchManagerHwLookupConfigDisbale(&exactMatchManagerPtr->lookupInfo,devNum,portGroupsBmp);

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerMatchHwEntryWithDbEntry function
* @endinternal
*
* @brief   The function build the HW entry format from sw entry(dbEntryPtr) and compares with input hwdata.
*
* @param[in] devNum                - device number.
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[in] hwData                - HW Entry
* @param[in] dbEntryPtr            - (pointer to) the DB entry
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note
*   NONE
*/
static GT_STATUS prvCpssDxChExactMatchManagerMatchHwEntryWithDbEntry
(
    IN GT_U8                                                    devNum,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U32                                                   hwData[],
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    GT_STATUS                                       rc;

    /* build the HW format at : exactMatchManagerPtr->tempInfo.hwDataArr (SW entry in HW format)
       hwData[] - contains HW entry

     there are steps to build the data
     1. build the exactMatchHwActionArray
     2. build exactMatchHwReducedActionDataArr
     3. build entry key format to set into exactMatchEntry
     4. build entry hw format key + ReducedAction to set into hwDataArr */

    /* build the HW format at : exactMatchManagerPtr->tempInfo.exactMatchHwActionArray */
    rc = prvCpssDxChExactMatchManagerHwBuildActionFormatFromDbEntry(devNum,exactMatchManagerPtr,dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* build the reduce entry to set into HW exactMatchManagerPtr->tempInfo.exactMatchHwReducedActionDataArr */
    rc = prvCpssDxChExactMatchManagerHwBuildReducedEntryFormatFromDbEntry(devNum,exactMatchManagerPtr,dbEntryPtr);
    if (rc  != GT_OK)
    {
        return rc;
    }

    /* build entry key format
       build the key entry to set into HW exactMatchManagerPtr->tempInfo.exactMatchEntry */
    prvCpssDxChExactMatchManagerHwBuildKeyEntryFormatFromDbEntry(exactMatchManagerPtr,dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* build entry hw format key + ReducedAction
       build the reduce entry to set into HW exactMatchManagerPtr->tempInfo.hwDataArr */
    rc = prvCpssDxChExactMatchManagerHwBuildEntryFormatFromDbEntry(devNum,exactMatchManagerPtr,dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* Ignore the Age Bit, before comparing the entry */
    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,exactMatchManagerPtr->tempInfo.hwDataArr,
                                     SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E,
                                     0);/* clear Activity bit */
    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,hwData,
                                     SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E,
                                     0);/* clear Activity bit */


    if(cpssOsMemCmp(exactMatchManagerPtr->tempInfo.hwDataArr, hwData, sizeof(exactMatchManagerPtr->tempInfo.hwDataArr)) != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwEntryMatchWithSwEntry_allDevice function
* @endinternal
*
* @brief   The function gets the HW entry from all device for dbEntryPtr and matches the content.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[in] hwIndex               - HW index.
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[out] resultPtr            - (pointer to) test result.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChExactMatchManagerHwEntryMatchWithSwEntry_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U32                                                   hwIndex,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
)
{
    GT_STATUS           rc;
    GT_U32              hwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_MAX_SIZE_CNS];/* hw data to set entryType/Age/keySize/lookupNum/key/reduccedAction/ */
    GT_U32              singleHwDataArr[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */
    GT_U32              startWord, entryIndexPerBank;
    GT_U32              bankNum=0,numOfBanks=0;

    GT_U8               devNum = 0;
    GT_PORT_GROUPS_BMP  portGroupBmp;
    GT_U32              portGroupId;

    CPSS_NULL_PTR_CHECK_MAC(exactMatchManagerPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(resultPtr);

    cpssOsMemSet(singleHwDataArr, 0, sizeof(singleHwDataArr));
    cpssOsMemSet(hwDataArr, 0, sizeof(hwDataArr));

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
    while(rc == GT_OK)
    {
        EM_LOCK_DEV_NUM(devNum);
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupBmp,PRV_CPSS_DXCH_UNIT_EM_E);

        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupBmp,portGroupId)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_CONVERT_KEY_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks, dbEntryPtr->hwExactMatchKeySize);

            for (bankNum=0; bankNum<numOfBanks; bankNum++)
            {
                startWord = bankNum*4;
                entryIndexPerBank = hwIndex + bankNum;

                /* read entry to specific index format */
                rc = prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,
                                                        CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                                        entryIndexPerBank,
                                                        &singleHwDataArr[0]);
                if(rc != GT_OK)
                {
                    EM_UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
                cpssOsMemCpy(&hwDataArr[startWord], singleHwDataArr, sizeof(singleHwDataArr));
            }

            if(rc != GT_OK)
            {
                EM_UNLOCK_DEV_NUM(devNum);
                return rc;
            }

            /* reset array used for comparisson*/
            cpssOsMemSet(&exactMatchManagerPtr->tempInfo.hwDataArr,0,sizeof(exactMatchManagerPtr->tempInfo.hwDataArr));
            rc = prvCpssDxChExactMatchManagerMatchHwEntryWithDbEntry(devNum, exactMatchManagerPtr, hwDataArr, dbEntryPtr);
            if(rc != GT_OK)
            {
                *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E;
                EM_UNLOCK_DEV_NUM(devNum);
                return GT_OK;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupBmp,portGroupId)

        EM_UNLOCK_DEV_NUM(devNum);
        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp);
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }
    return rc;
}


/*
* @internal compareAction function
* @endinternal
*
* @brief   The function compare HW Action with DB action and matches the content.
*
* @param[in] devNum                - device number.
* @param[in] actionType            - type of action to compare
* @param[in] hwDefaultAction       - HW Action.
* @param[in] dbDefaultAction       - DB Action
* @param[out]isEqualPtr            - pointer to comparing result
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note
*   NONE
*/
static GT_STATUS compareAction
(
    IN GT_U8                                 devNum,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT      *hwAction,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT      *dbAction,
    OUT GT_BOOL                              *isEqualPtr
)
{
    GT_STATUS rc;
    GT_U32    hwActionArray[PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS];
    GT_U32    dbActionArray[PRV_CPSS_DXCH_EXACT_MATCH_DEFAULT_ACTION_SIZE_CNS];
    if (actionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
    {
        rc = prvCpssDxChTtiActionType2Logic2HwFormat(devNum,
                                                     &(hwAction->ttiAction),
                                                     hwActionArray);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail to convert TTI hwAction format to HW format (hwActionArray).\n");
        }

        rc = prvCpssDxChTtiActionType2Logic2HwFormat(devNum,
                                                     &(dbAction->ttiAction),
                                                     dbActionArray);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail to convert TTI dbAction format to HW format (dbActionArray).\n");
        }
    }
    else
    {
        rc = prvCpssDxChPclTcamRuleActionSw2HwConvert(devNum,
                                                     &(hwAction->pclAction),
                                                     hwActionArray);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail to convert PCL hwAction format to HW format (hwActionArray).\n");
        }

        rc = prvCpssDxChPclTcamRuleActionSw2HwConvert(devNum,
                                                     &(dbAction->pclAction),
                                                     dbActionArray);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail to convert PCL dbAction format to HW format (dbActionArray).\n");
        }
    }

    /* compare HW and DB arrays */
    *isEqualPtr = (0 == cpssOsMemCmp((GT_VOID*) hwActionArray,
                                 (GT_VOID*) dbActionArray,
                                 sizeof(dbActionArray))) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup function
* @endinternal
*
* @brief  This function check HW lookup configuration match with SW lookup configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] portGroupsBmp   - port group bitmap
* @param[out]dbLookupPtr     - (pointer to) the Exact Match Manager lookup config saved in DB.
* @param[out]resultPtr       - (pointer to) test result.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC                 *dbLookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
)
{
    GT_STATUS                                               rc;
    GT_BOOL                                                 lookupEnable;
    GT_U32                                                  lookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                        lookupClient;
    GT_U32                                                  lookupClientMappingsNum;
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT    *lookupClientMappingsArray;
    CPSS_PCL_DIRECTION_ENT                                  direction;
    GT_U32                                                  i;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                        hwLookupClient;
    GT_BOOL                                                 hwEnableExactMatchLookup;
    GT_U32                                                  hwProfileId;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC            hwKeyParams;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                        hwDefaultAction;
    GT_BOOL                                                 hwDefaultActionEn;
    GT_BOOL                                                 isEqual;


    for(lookupNum=0;lookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;lookupNum++)
    {
        lookupEnable                =   dbLookupPtr->lookupsArray[lookupNum].lookupEnable;
        lookupClient                =   dbLookupPtr->lookupsArray[lookupNum].lookupClient;
        lookupClientMappingsNum     =   dbLookupPtr->lookupsArray[lookupNum].lookupClientMappingsNum;
        lookupClientMappingsArray   =   dbLookupPtr->lookupsArray[lookupNum].lookupClientMappingsArray;

        if(lookupEnable==GT_TRUE)
        {
            EM_LOCK_DEV_NUM(devNum)
            /* get lookup client */
            rc = prvCpssDxChExactMatchPortGroupClientLookupGet(devNum,
                                                               portGroupsBmp,
                                                               (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                               &hwLookupClient);
            if (rc != GT_OK)
            {
                EM_UNLOCK_DEV_NUM(devNum);
                return rc;
            }
            /* check sw equal hw values */
            if(hwLookupClient != dbLookupPtr->lookupsArray[lookupNum].lookupClient)
            {
                *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E;
                EM_UNLOCK_DEV_NUM(devNum);
                return GT_OK;
            }

            for(i=0;i<lookupClientMappingsNum;i++)
            {
                /* get mapping of key/packet type to profileId */
                if(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                {
                    rc = prvCpssDxChExactMatchPortGroupTtiProfileIdModePacketTypeGet(devNum,
                                                                                   portGroupsBmp,
                                                                                   lookupClientMappingsArray[i].ttiMappingElem.keyType,
                                                                                   (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                                                   &hwEnableExactMatchLookup,
                                                                                   &hwProfileId);
                    if (rc != GT_OK)
                    {
                        EM_UNLOCK_DEV_NUM(devNum);
                        return rc;
                    }
                    /* check sw equal hw values */
                    if((hwEnableExactMatchLookup != lookupClientMappingsArray[i].ttiMappingElem.enableExactMatchLookup)||
                       (hwProfileId != lookupClientMappingsArray[i].ttiMappingElem.profileId))
                    {
                        *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E;
                        EM_UNLOCK_DEV_NUM(devNum);
                        return GT_OK;
                    }
                }
                else
                {
                    if (lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
                        direction = CPSS_PCL_DIRECTION_EGRESS_E;
                    else
                    {
                        if ((!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))&&(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: IPCL_2 only supported for AC5P \n");
                        }

                        direction = CPSS_PCL_DIRECTION_INGRESS_E;
                    }
                    rc = prvCpssDxChExactMatchPortGroupPclProfileIdGet(devNum,
                                                                        portGroupsBmp,
                                                                        direction,
                                                                        lookupClientMappingsArray[i].pclMappingElem.packetType,
                                                                        lookupClientMappingsArray[i].pclMappingElem.subProfileId,
                                                                        (CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)lookupNum,
                                                                        &hwEnableExactMatchLookup,
                                                                        &hwProfileId);
                    if (rc != GT_OK)
                    {
                        EM_UNLOCK_DEV_NUM(devNum);
                        return rc;
                    }
                    /* check sw equal hw values */
                    if((hwEnableExactMatchLookup != lookupClientMappingsArray[i].pclMappingElem.enableExactMatchLookup)||
                       (hwProfileId != lookupClientMappingsArray[i].pclMappingElem.profileId))
                    {
                        *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E;
                        EM_UNLOCK_DEV_NUM(devNum);
                        return GT_OK;
                    }
                }

                /* get profileId configuration */
                if (hwEnableExactMatchLookup==GT_TRUE)
                {
                    /* get key params */
                    rc = prvCpssDxChExactMatchPortGroupProfileKeyParamsGet(devNum,
                                                                        portGroupsBmp,
                                                                        hwProfileId,
                                                                        &hwKeyParams);
                    if (rc != GT_OK)
                    {
                        EM_UNLOCK_DEV_NUM(devNum);
                        return rc;
                    }
                    /* check sw equal hw values */
                    if((hwKeyParams.keySize != dbLookupPtr->profileEntryParamsArray[hwProfileId].keyParams.keySize)||
                       (hwKeyParams.keyStart != dbLookupPtr->profileEntryParamsArray[hwProfileId].keyParams.keyStart)||
                       (cpssOsMemCmp(&hwKeyParams.mask,dbLookupPtr->profileEntryParamsArray[hwProfileId].keyParams.mask,
                                     sizeof(dbLookupPtr->profileEntryParamsArray[hwProfileId].keyParams.mask))!=0))
                    {
                        *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E;
                        EM_UNLOCK_DEV_NUM(devNum);
                        return GT_OK;
                    }

                    /* Get default entry configuration */
                    rc = prvCpssDxChExactMatchPortGroupProfileDefaultActionGet(devNum,
                                                                            portGroupsBmp,
                                                                            hwProfileId,
                                                                            dbLookupPtr->profileEntryParamsArray[hwProfileId].defaultActionType,
                                                                            &hwDefaultAction,
                                                                            &hwDefaultActionEn);
                    if (rc != GT_OK)
                    {
                        EM_UNLOCK_DEV_NUM(devNum);
                        return rc;
                    }
                    /* check sw equal hw values */
                    if (hwDefaultActionEn!=dbLookupPtr->profileEntryParamsArray[hwProfileId].defaultActionEn)
                    {
                        *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E;
                        EM_UNLOCK_DEV_NUM(devNum);
                        return GT_OK;
                    }
                    if (dbLookupPtr->profileEntryParamsArray[hwProfileId].defaultActionEn==GT_TRUE)
                    {
                        /* check sw equal hw values - default action */
                        rc = compareAction(devNum,
                                           dbLookupPtr->profileEntryParamsArray[hwProfileId].defaultActionType,
                                           &hwDefaultAction,
                                           &dbLookupPtr->profileEntryParamsArray[hwProfileId].defaultAction,
                                           &isEqual);
                        if (rc != GT_OK)
                        {
                            EM_UNLOCK_DEV_NUM(devNum);
                            return rc;
                        }
                        if (isEqual!=GT_TRUE)
                        {
                            *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E;
                            EM_UNLOCK_DEV_NUM(devNum);
                            return GT_OK;
                        }
                    }
                }
            }
            EM_UNLOCK_DEV_NUM(devNum);
        }
    }
    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerHwLookupConfigMatchWithSwLookupConfig_allDevice function
* @endinternal
*
* @brief   The function gets the HW configuration from all device for
*          exactMatchManagerPtr->lookupInfo and matches the content.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultPtr            - (pointer to) test result.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChExactMatchManagerHwLookupConfigMatchWithSwLookupConfig_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
)
{
    GT_STATUS           rc;
    GT_U8               devNum = 0;
    GT_PORT_GROUPS_BMP  portGroupBmp;

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
    while(rc == GT_OK)
    {
        rc = prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup(devNum,portGroupBmp,&exactMatchManagerPtr->lookupInfo,resultPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp);
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }
    return rc;
}
/*
* @internal prvCpssDxChExactMatchManagerExpandedActionHwToSwValidityMatch
*           function
* @endinternal
*
* @brief   the function check that the HW values corresponds
*          with the Shadow and return the SW values kept in the
*          shadow
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum;
*                                  Aldrin; AC3X; Lion2; Bobcat3;
*                                  Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:0..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] dbExpandedActionPtr-(pointer to)expanded action from shadow
* @param[in] hwValueArr[]      - the HW value
* @param[out] actionPtr        - (pointer to)Exact Match Action
* @param[out] expandedActionOriginPtr - (pointer to) Whether to
*                   use the action attributes from the Exact
*                   Match rule action or from the profile
* @param[out] isEqualPtr       - (pointer to)GT_TRUE: Shadow SW and HW are equal
*                                            GT_FALSE:Shadow SW and HW are NOT equal
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChExactMatchManagerExpandedActionHwToSwValidityMatch
(
    IN GT_U8                                                    devNum,
    IN GT_U32                                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                    actionType,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC    *dbExpandedActionPtr,
    IN GT_U32                                                   hwValueArr[],
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT        *expandedActionOriginPtr,
    OUT GT_BOOL                                                 *isEqualPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL isEqualAction = GT_FALSE;
    GT_BOOL isEqualOrigin = GT_FALSE;
    GT_U32  hwValuesDbArr[PRV_CPSS_DXCH_MAX_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS];
    GT_U32  reducedMaskArr[PRV_CPSS_DXCH_EXACT_MATCH_ACTION_SIZE_CNS];

    *isEqualPtr=GT_FALSE;

    /* get the data from DB */
    if(dbExpandedActionPtr->exactMatchExpandedEntryValid!=GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "The Expander Action Entry is not valid \n");
    }

    cpssOsMemSet(hwValuesDbArr, 0, sizeof(hwValuesDbArr));
    cpssOsMemSet(reducedMaskArr, 0, sizeof(reducedMaskArr));

    rc = prvCpssDxChExpandedActionToHwformat(devNum,
                                             expandedActionIndex,
                                             actionType,
                                             &dbExpandedActionPtr->exactMatchExpandedEntry.expandedAction,
                                             &dbExpandedActionPtr->exactMatchExpandedEntry.expandedActionOrigin,
                                             hwValuesDbArr,
                                             reducedMaskArr);
    if (rc!=GT_OK)
    {
        return rc;
    }
    isEqualAction = (0 == cpssOsMemCmp((GT_VOID*) hwValueArr,
                                 (GT_VOID*) hwValuesDbArr,
                                 sizeof(hwValuesDbArr))) ? GT_TRUE : GT_FALSE;

    isEqualOrigin = (0 == cpssOsMemCmp((GT_VOID*) dbExpandedActionPtr->reducedMaskArr,
                                 (GT_VOID*) reducedMaskArr,
                                 sizeof(reducedMaskArr))) ? GT_TRUE : GT_FALSE;

    if ((isEqualAction == GT_TRUE)&&(isEqualOrigin==GT_TRUE))
    {
        *isEqualPtr=GT_TRUE;

        switch (actionType)
        {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            /* return the Action and Reduce flags keept in the DB */
            cpssOsMemCpy(&actionPtr->ttiAction,
                         &dbExpandedActionPtr->exactMatchExpandedEntry.expandedAction.ttiAction,
                         sizeof(CPSS_DXCH_TTI_ACTION_STC));
            cpssOsMemCpy(expandedActionOriginPtr,
                         &dbExpandedActionPtr->exactMatchExpandedEntry.expandedActionOrigin.ttiExpandedActionOrigin,
                         sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC));
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            /* return the Action and Reduce flags keept in the DB */
            cpssOsMemCpy(&actionPtr->pclAction,
                         &dbExpandedActionPtr->exactMatchExpandedEntry.expandedAction.pclAction,
                         sizeof(CPSS_DXCH_PCL_ACTION_STC));
            cpssOsMemCpy(expandedActionOriginPtr,
                         &dbExpandedActionPtr->exactMatchExpandedEntry.expandedActionOrigin.pclExpandedActionOrigin,
                         sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC ));
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "prvCpssDxChExpandedActionToHwformat wrong actionType\n");
        }
    }
    else
    {
        *isEqualPtr=GT_FALSE;
    }
    return rc;
}
/**
* @internal prvCpssDxChExactMatchManagerPortGroupExpandedActionGet
*           function
* @endinternal
*
* @brief   Gets the action for Exact Match in case of a match in
*          Exact Match lookup The API also sets for each action
*          attribute whether to take it from the Exact Match
*          entry action or from the Exact Match profile
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum            - device number
* @param[in] portGroupsBmp    - bitmap of Port Groups.
*                               NOTEs:
*                               1. for non multi-port groups device this parameter is IGNORED.
*                               2. for multi-port groups device :
*                               (APPLICABLE DEVICES  Falcon)
*                               bitmap must be set with at least one bit representing
*                               valid port group(s). If a bit of non valid port group
*                               is set then function returns GT_BAD_PARAM.
*                               value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] expandedActionIndex -Exact Match Expander table
*                               index (APPLICABLE RANGES:0..15)
* @param[in] actionType        - Exact Match Action Type
* @param[in] dbExpandedActionPtr-(pointer to)expanded action from shadow
* @param[out] actionPtr         -(pointer to)Exact Match Action
* @param[out]expandedActionOriginPtr - (pointer to) Whether to
*                   use the action attributes from the Exact
*                   Match rule action or from the profile
* @param[out] isEqualPtr       - (pointer to)GT_TRUE: Shadow SW and HW are equal
*                                            GT_FALSE:Shadow SW and HW are NOT equal
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong param
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerPortGroupExpandedActionGet
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN GT_U32                                                   expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                    actionType,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC    *dbExpandedActionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT                        *actionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT        *expandedActionOriginPtr,
    OUT GT_BOOL                                                 *isEqualPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    regAddr;
    GT_U32    i=0;/* for loop */
    GT_U32    value[PRV_CPSS_DXCH_MAX_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS];
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32    expandedActioSize=0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
                                          CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);
    CPSS_NULL_PTR_CHECK_MAC(expandedActionOriginPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

    /* check validity of expandedActionIndex 0-15 */
    PRV_CPSS_DXCH_EXACT_MATCH_CHECK_EXPANDED_ACTION_INDEX_VALUE_MAC(expandedActionIndex);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, portGroupsBmp, portGroupId);

    cpssOsMemSet(value, 0, sizeof(value));

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* exactMatchActionAssignment[16][32] Packet Type/Byte*/
        expandedActioSize = PRV_CPSS_SIP6_10_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS;
    }
    else
    {
        /* exactMatchActionAssignment[16][31] Packet Type/Byte*/
        expandedActioSize = PRV_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS;
    }
    for (i=0; i<expandedActioSize; i++)
    {

        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->TCAM.exactMatchActionAssignment[expandedActionIndex][i];
         /* Exact Match Action data
            the action data for byte i
            0=origin, default data
            1=bits[0:2] mapped index from Exact Match

            Exact Match Action Type
            Determines whether this byte is assigned from
            the reduces action or from the default value*/
        rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,0,9,&value[i]);
        if (rc!=GT_OK)
        {
            return rc;
        }
    }

   /* the function check that the HW values corresponds with the Shadow kept in the Add operation
       and return the SW values kept in the shadow */
    rc = prvCpssDxChExactMatchManagerExpandedActionHwToSwValidityMatch(devNum,
                                                                       expandedActionIndex,
                                                                       actionType,
                                                                       dbExpandedActionPtr,
                                                                       value,
                                                                       actionPtr,
                                                                       expandedActionOriginPtr,
                                                                       isEqualPtr);
    if (rc  != GT_OK)
    {
       return rc;
    }


    return rc;
}


/*
* @internal prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray function
* @endinternal
*
* @brief  This function check HW expanded configuration match with SW expanded configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum          - device number.
* @param[in] portGroupsBmp   - port group bitmap
* @param[out]dbExpandedArray - the Exact Match Manager expanded config saved in DB.
* @param[out]resultPtr       - (pointer to) test result.
*
* @retval GT_OK              - on success
* @retval other              - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray
(
    IN GT_U8                                                    devNum,
    IN GT_PORT_GROUPS_BMP                                       portGroupsBmp,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_STC    dbExpandedArray[],
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
)
{
    GT_STATUS                                           rc;
    GT_U32                                              i=0;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    hwAction;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    hwExpandedActionOrigin;
    GT_BOOL                                             isEqual;

    EM_LOCK_DEV_NUM(devNum)

    /* go over the expanded array keept in the DB */
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
    {
        /* if the entry is valid in the DB then need to compare it to the HW entry */
        if (dbExpandedArray[i].exactMatchExpandedEntryValid==GT_TRUE)
        {
            /* get the entry for HW */
            rc = prvCpssDxChExactMatchManagerPortGroupExpandedActionGet(devNum,
                                                                        portGroupsBmp,
                                                                        i,
                                                                        dbExpandedArray[i].exactMatchExpandedEntry.expandedActionType,
                                                                        &dbExpandedArray[i],
                                                                        &hwAction,
                                                                        &hwExpandedActionOrigin,
                                                                        &isEqual);
            if (rc != GT_OK)
            {
                EM_UNLOCK_DEV_NUM(devNum);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in getting expanded entry from HW for expandedActionIndex[%d]",i);
            }

            if (isEqual!=GT_TRUE)
            {
                *resultPtr = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E;
                EM_UNLOCK_DEV_NUM(devNum);
                return GT_OK;
            }
        }
    }

    EM_UNLOCK_DEV_NUM(devNum);
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray_allDevice function
* @endinternal
*
* @brief   The function gets the HW configuration from all device for
*          exactMatchManagerPtr->exactMatchExpanderArray and matches the content.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultPtr            - (pointer to) test result.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      *resultPtr
)
{
    GT_STATUS           rc;
    GT_U8               devNum = 0;
    GT_PORT_GROUPS_BMP  portGroupBmp;

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
    while(rc == GT_OK)
    {
        rc = prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray(devNum,portGroupBmp,exactMatchManagerPtr->exactMatchExpanderArray,resultPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp);
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerHwAgedOutVerify function
* @endinternal
*
* @brief   The function verifies the DB entry is aged-out not not & update the status in agedOut Out param.
*             Not aged-out - Reset the age-bit of input entry in all the devices as part of aging process.
*             Aged-out     - No updates are required.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the ExactMatch manager
* @param[in] dbEntryPtr            - (pointer to) the DB entry
* @param[out] agedOutPtr           - (pointer to) the aged-out status.
*                                    GT_TRUE  - Entry is aged-out
*                                    GT_FALSE - Entry is not aged-out
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwAgedOutVerify
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr,
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                         *agedOutPtr
)
{
    GT_STATUS                                        rc;
    GT_BOOL                                          isFirst = GT_TRUE;
    GT_U32                                           hwValue;
    GT_U8                                            devNum = 0;
    GT_PORT_GROUPS_BMP                               portGroupsBmp;
    GT_U32                                           portGroupId; /* the port group Id */
    GT_U32                                           hwData[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* hold 115 bits of data */

    /* check if there are registered devices on the manager */
    *agedOutPtr = GT_TRUE;
    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);

    isFirst = GT_FALSE;
    while(rc == GT_OK)
    {
        EM_LOCK_DEV_NUM(devNum);
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

        /* loop on all active port groups in the bmp */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
        {
            /* Keep checking age-bit refresh in all device - Till the first device detected */
            if(*agedOutPtr == GT_TRUE)
            {
                /* per device : Check age bit */
                rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                                        portGroupId,
                                                        CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                                        dbEntryPtr->hwIndex,
                                                        &hwData[0]);
                if(rc != GT_OK)
                {
                    EM_UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
                SIP6_EXACT_MATCH_ENTRY_FIELD_GET_MAC(devNum, hwData, SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E, hwValue);
                if(hwValue == 1)
                {
                    *agedOutPtr = GT_FALSE;
                    SIP6_EXACT_MATCH_ENTRY_FIELD_SET_MAC(devNum,
                                                         hwData,
                                                         SIP6_EXACT_MATCH_TABLE_FIELDS_AGE_E,
                                                         0);/* hwValue - clear Activity bit */

                }
            }

            /* clear the age-bit for all devices - from the age-bit refresh detected device till end */
            if(*agedOutPtr == GT_FALSE)
            {
                rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                        portGroupId,
                        CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                        dbEntryPtr->hwIndex,
                        &hwData[0]);
                if(rc != GT_OK)
                {
                    EM_UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        EM_UNLOCK_DEV_NUM(devNum);
        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);
    }
    return GT_OK;
}

/*
* @internal validateManagerSwHwParams_allDevice function
* @endinternal
*
* @brief  The function validate 'API level' 'manager create' SW
*         against HW parameters.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS    validateManagerSwHwParams_allDevice
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr
)
{
    GT_STATUS                                           rc=GT_OK;
    GT_PORT_GROUPS_BMP                                  tempPortGroupsBmp;
    GT_U32                                              portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32                                              fieldLength,fieldOffset,hwValue,swValue;
    GT_U32                                              regAddr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  result;

    GT_U8               devNum=0;
    GT_PORT_GROUPS_BMP  portGroupBmp;

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
    while(rc == GT_OK)
    {
        EM_LOCK_DEV_NUM(devNum);
        if(0 == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm)
        {
            EM_UNLOCK_DEV_NUM(devNum);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "The device [%d] not supports the EM unit",
            devNum);
        }
        EM_UNLOCK_DEV_NUM(devNum);


        tempPortGroupsBmp = portGroupBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum, tempPortGroupsBmp, portGroupId);


        if(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes > (1 << HW_INDEX_NUM_BITS))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "numOfHwIndexes[%d] is more than the 'manager' supports[%d] (need to set new value to HW_INDEX_NUM_BITS)",
                exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes,
                (1 << HW_INDEX_NUM_BITS));
        }

        switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes)
        {
            case _4KB: swValue = 0; break;
            case _8KB: swValue = 1; break;
            case _16KB: swValue = 2; break;
            case _32KB: swValue = 3; break;
            case _64KB: swValue = 4; break;
            case _128KB: swValue = 5; break;
            case _256KB:
                if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    swValue = 6;
                    break;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
                }
            case _512KB: swValue = 7; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        }

        /* ************************************************************************************************* */
        /* ****** compare exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes with HW value ******* */
        /* ************************************************************************************************* */

        fieldLength = 3;
        /* get The size of the Exact Match Table */
        fieldOffset = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)? 0:11;
        EM_LOCK_DEV_NUM(devNum)
        rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,
               PRV_DXCH_REG1_UNIT_EM_MAC(devNum).EMGlobalConfiguration.EMGlobalConfiguration1,
               fieldOffset,
               fieldLength,
               &hwValue);
        EM_UNLOCK_DEV_NUM(devNum)
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in reading from HW Exact Match size");
        }
        if (swValue!=hwValue)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,
                "numOfHwIndexes in HW [%d] is not equal to the 'manager' supports numOfIndexes [%d] "
                "(fail in validateNewManagerSwHwParams)",hwValue,swValue);
        }

        /* ********************************************************************************************** */
        /* ****** compare exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes with HW value ******* */
        /* ********************************************************************************************** */

        /** number of hashes (banks) : 16,8,4 */
        switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)
        {
            case  4: swValue = 0; break;
            case  8: swValue = 1; break;
            case  CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS/*16*/: swValue = 2; break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes);
        }

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            regAddr = PRV_DXCH_REG1_UNIT_EM_MAC(devNum).EMGlobalConfiguration.EMGlobalConfiguration1;
            fieldOffset = 3;
        }
        else
        {
            regAddr = PRV_DXCH_REG1_UNIT_EM_MAC(devNum).EMGlobalConfiguration.EMGlobalConfiguration2;
            fieldOffset = 2;
        }
        EM_LOCK_DEV_NUM(devNum)
        rc = prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,fieldOffset,2,&hwValue);
        EM_UNLOCK_DEV_NUM(devNum)
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in reading from HW numOfHashes");
        }

        if (swValue!=hwValue)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,
                "numOfHashes in HW [%d] is not equal to the 'manager' supports numOfHashes [%d] "
                "(fail in validateNewManagerSwHwParams)",hwValue,swValue);
        }

        /* *************************************************************************** */
        /* ****** compare exactMatchManagerPtr->lookupInfo with HW value       ******* */
        /* *************************************************************************** */
        result=0;/* reset result */
        rc = prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup(devNum,portGroupBmp,&exactMatchManagerPtr->lookupInfo,&result);
        if ((rc != GT_OK)||
            (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E)||
            (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E)||
            (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E)||
            (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E)||
            (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E))

        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,"lookupInfo in HW is not equal to the 'manager' lookupInfo "
                "(fail in validateNewManagerSwHwParams - prvCpssDxChExactMatchManagerHwLookupMatchWithSwLookup)");
        }

        /* ********************************************************************************** */
        /* ****** compare exactMatchManagerPtr->exactMatchExpanderArray with HW value ******* */
        /* ********************************************************************************** */

        result=0;/* reset result */
        rc = prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray(devNum,portGroupBmp,exactMatchManagerPtr->exactMatchExpanderArray,&result);
        if ((rc != GT_OK)||
            (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in validation of SW/HW of exactMatchExpanderArray");
        }

        /* *************************************************************************** */
        /* ****** compare agingInfo.agingRefreshEnable with HW value            ******* */
        /* *************************************************************************** */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->EM.EMGlobalConfiguration.EMGlobalConfiguration1;
        fieldOffset = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum)? 5:20;
        EM_LOCK_DEV_NUM(devNum)
        rc =  prvCpssHwPpPortGroupGetRegField(devNum,portGroupId,regAddr,fieldOffset,1,&hwValue);
        EM_UNLOCK_DEV_NUM(devNum)
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in reading HW value of agingRefreshEnable for manager[%d]");
        }
        if (BIT2BOOL_MAC(hwValue)!=exactMatchManagerPtr->agingInfo.agingRefreshEnable)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,
                "agingRefreshEnable in HW [%d] is not equal to the 'manager' agingRefreshEnable [%d] "
                "(fail in validateNewManagerSwHwParams)",hwValue,BOOL2BIT_MAC(exactMatchManagerPtr->agingInfo.agingRefreshEnable));
        }

        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp);
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerHwDbCheck_global function
* @endinternal
*
* @brief This function verifies SW and HW Exact Match manager global parameters.
*
* @param[in]  exactMatchManagerPtr - (pointer to) the Exact Match manager
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerHwDbCheck_global
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr
)
{
    GT_STATUS                                               rc = GT_OK;

    rc = validateManagerSwHwParams_allDevice(exactMatchManagerPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerHwDbCheck_hwList function
* @endinternal
*
* @brief The function verifies the Exact Match entries in the manager are equal to HW data.
*
* @param[in]  exactMatchManagerPtr - (pointer to) the Exact Match manager
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - there is error in DB management.
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerHwDbCheck_hwList
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr
)
{
    GT_STATUS                                                   rc = GT_OK;
    GT_U32                                                      hwIndex,step;
    GT_U32                                                      dbIndex;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT          result;

    /* check all exact match entries defined in the DB */
    for(hwIndex = 0 ; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /*hwIndex updated according to entry keySize */)
    {
        result = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E;
        /* Validate used list content */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            if (exactMatchManagerPtr->entryPoolPtr[dbIndex].isFirst==GT_TRUE)
            {
                 rc = prvCpssDxChExactMatchManagerHwEntryMatchWithSwEntry_allDevice(exactMatchManagerPtr,
                                                                                   hwIndex,
                                                                                   &exactMatchManagerPtr->entryPoolPtr[dbIndex],
                                                                                   &result);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if(result == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E)/* we treat it as a fail */
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* jump to the next hwIndex to check */
                step = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize + 1;
                hwIndex = hwIndex+step;
            }
            else
            {
                hwIndex++;
            }
        }
        else
        {
            /* else - In case of invalid entry dbIndex will not be valid to check */
            hwIndex++;
        }
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }

    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerHwDbValidityCheck function
* @endinternal
*
* @brief The function verifies the Exact Match manager DB is sync with the HW.
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devNum                 - the device number (cpss devNum)
* @param[in] portGroupsBmp- bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES  Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in]  checksPtr            - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwDbValidityCheck
(
    IN  GT_U32                                              exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC         *checksPtr
)
{
    GT_STATUS                                               rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    if(checksPtr->globalCheckEnable)                /* globalCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerHwDbCheck_global(exactMatchManagerPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(checksPtr->hwUsedListCheckEnable) /* hwUsedListCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerHwDbCheck_hwList(exactMatchManagerPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerHwHaTempDbCreate function
* @endinternal
*
* @brief The function create a temporary database for in manager and read all
*       Exact Match entries from HW to be used later on when the entries are replayed
*       by the application in the HA process
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwHaTempDbCreate
(
    IN  GT_U32  exactMatchManagerId
)
{
    GT_STATUS                                               rc=GT_OK;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  numBytesToAlloc;
    GT_U32                                                  hwIndex=0,i=0;
    GT_BOOL                                                 isFirst = GT_TRUE;
    GT_U8                                                   devNum = 0;
    GT_PORT_GROUPS_BMP                                      portGroupsBmp;
    GT_BOOL                                                 isValid;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                        action;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC                         entry;
    GT_U32                                                  expandedActionIndex;
    CPSS_SYSTEM_RECOVERY_INFO_STC                           oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_BOOL                                                 managerOldStatus;
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* init the HA temporary database array */
    numBytesToAlloc = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_STC);
    EM_MEM_CALLOC_MAC(exactMatchManagerId,exactMatchManagerPtr->haTempDbArr,numBytesToAlloc);

    /* get first valid member in the manager: dev+portGroup */
    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr,isFirst,&devNum,&portGroupsBmp);
    if (rc!=GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail to get a valid devNum+portGroupBmp in function prvCpssDxChExactMatchManagerHaTempDbCreate \n");
    }

     /* save current recovery state & and make it complete - to enable HW read temporally
    if the state is INIT/CATCH_UP then the automatic read mechanism  will not read from HW.
    in HA case we need to read the value from HW, to build the haTempDb */
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    managerOldStatus = PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted;
    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = GT_FALSE;
    /* read all exact match entries from HW and store them in the manager
       in a temporary database*/
    for(hwIndex = 0; hwIndex<exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /* hwIndex change according to the keySize of the entry*/ )
    {
        /* Read Exact Match entry */
        EM_LOCK_DEV_NUM(devNum);
        rc = cpssDxChExactMatchPortGroupEntryGet(devNum,portGroupsBmp,hwIndex,&isValid,
                                                 &actionType,&action,&entry,&expandedActionIndex);
        EM_UNLOCK_DEV_NUM(devNum);
        if(rc != GT_OK)
        {
            /* set flag - need to recnstruct back to the original state after the HW read */
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Fail to get an entry -cpssDxChExactMatchPortGroupEntryGet- function prvCpssDxChExactMatchManagerHaTempDbCreate \n");
        }

        /*** add entry to the HA DB for each valid entry ***/
        if(isValid)
        {
            /* set all hw indexes related to this entry as valid
               5bytes entry has 1 index
               19bytes entry has 2 index
               33bytes entry has 3 index
               47bytes entry has 4 index */
            for (i=0; i <= (GT_U32)(entry.key.keySize); i++)
            {
                if (exactMatchManagerPtr->haTempDbArr[(hwIndex+i)].isValid == GT_TRUE)
                {
                    /* set flag - need to reconstruct back to the original state after the HW read */
                    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;

                    /* illegal case -->if isValid is already GT_TRUE in the HA database
                       it means another entry is already occupying this index */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Fail: isValid is already GT_TRUE in the HA database it means another entry is already"
                                                           " occupying this index- function prvCpssDxChExactMatchManagerHaTempDbCreate \n");
                }

                exactMatchManagerPtr->haTempDbArr[(hwIndex+i)].isValid = GT_TRUE;
                exactMatchManagerPtr->haTempDbArr[(hwIndex+i)].entryWasTaken = GT_FALSE;
            }

            numBytesToAlloc = sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_ENTRY_STC);
            EM_MEM_CALLOC_MAC(exactMatchManagerId,exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr,numBytesToAlloc);

            exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr->expandedActionIndex = expandedActionIndex;
            exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr->exactMatchActionType = actionType;
            if (actionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
            {
                cpssOsMemCpy(&exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr->exactMatchAction.ttiAction,
                             &action.ttiAction, sizeof(CPSS_DXCH_TTI_ACTION_STC));
            }
            else
            {
                cpssOsMemCpy(&exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr->exactMatchAction.pclAction,
                             &action.pclAction, sizeof(CPSS_DXCH_PCL_ACTION_STC));
            }

            cpssOsMemCpy(&exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr->exactMatchEntry,
                         &entry, sizeof(CPSS_DXCH_EXACT_MATCH_ENTRY_STC));

            hwIndex = hwIndex+i;
        }
        else
        {
            exactMatchManagerPtr->haTempDbArr[hwIndex].isValid = GT_FALSE;
            exactMatchManagerPtr->haTempDbArr[hwIndex].entryWasTaken = GT_FALSE;
            exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr=NULL;
            hwIndex++;
        }
    }
    /* set flag - need to recnstruct back to the original state after the HW read */
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E).recoveryStarted = managerOldStatus;
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerHwHaTempDbDelete function
* @endinternal
*
* @brief The function Delete the HA temporary database.
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerHwHaTempDbDelete
(
    IN  GT_U32  exactMatchManagerId
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  hwIndex;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* deallocate all 'malloc' that where done for HA in this manager */

    /* first free allocations for all exact match entries stored in the manager temporary database */
    for(hwIndex = 0; hwIndex<exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
       /*** delete entry from the HA DB for each valid entry ***/
        if((exactMatchManagerPtr->haTempDbArr[hwIndex].isValid==GT_TRUE)&&
           (exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr!=NULL))
        {
               EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr);
        }
    }

    /* second we deallocate the array */
    EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->haTempDbArr);

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerHwInvalidateNonTakenIndexFromHaTempDb function
* @endinternal
*
* @brief   This function go over the HA temporary database and
*          invalidate all non taken indexes in HW
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerHwInvalidateNonTakenIndexFromHaTempDb
(
        IN  GT_U32  exactMatchManagerId
)
{
    GT_STATUS                                               rc = GT_OK;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32                                                  hwIndex=0;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    for(hwIndex = 0; hwIndex<exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        /* if the entry is valid in the HW but in the end of the SW/HW sync it is still not taken,
           it means that the replay do not include this entry and we need to delete it from HW for
           all valid devices of the manager */
        if((exactMatchManagerPtr->haTempDbArr[hwIndex].isValid==GT_TRUE)&&
           (exactMatchManagerPtr->haTempDbArr[hwIndex].entryWasTaken==GT_FALSE)&&
           (exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr!=NULL))
        {
            /***************************************************************************/
            /* flush the entry from HW of all registered devices                       */
            /***************************************************************************/
            rc = prvCpssDxChExactMatchManagerHwFlushByHwIndex(exactMatchManagerPtr,hwIndex);
            if(rc != GT_OK)
            {
                return rc;
            }
            exactMatchManagerPtr->haTempDbArr[hwIndex].isValid=GT_FALSE;
            EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr);
        }
    }
    return GT_OK;;
}

/**
* @internal prvCpssDxChExactMatchManagerAacHwWriteEntry function
* @endinternal
*
* @brief   Write a whole EM entry to the HW using AAC method.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_HW_ERROR              - on hardware error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerAacHwWriteEntry
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT     tableType,
    IN GT_U32                  entryIndex,
    IN GT_U32                  *entryValuePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      numOfWords, currWord;
    GT_U32      regData,tmpPortGroupId;

    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr;


    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    /* Check that previous access done by busy-wait polling before trigger next access. */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEnginesStatus;
    rc = prvCpssPortGroupBusyWait(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr,
            PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_PRIMARY_E, GT_FALSE/*busyWait*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);

    /* AAC Engine <<%n>> Data for channel EM */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_PRIMARY_E];
    numOfWords = tableInfoPtr->entrySize;
    for (currWord = 0; currWord < numOfWords; currWord++)
    {
        regData = entryValuePtr[currWord];
        /* Write words of data */
        rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* AAC Engine <<%n>> Address for channel EM */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_PRIMARY_E];
    regData = PRV_DXCH_REG1_UNIT_EM_MAC(devNum).EMIndirectAccess.EMIndirectAccessData[0];

    /* Write address of EM indirect accsess data register */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Data for channel 2 */
    regAddr  = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineData[PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_SECONDARY_E];
    regData = entryIndex << 2 | 3;

    /* Write command, HW index and trigger bit */
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* AAC Engine <<%n>> Address for channel 2 */
    regAddr = PRV_DXCH_REG1_UNIT_SIP6_CNM_AAC_MAC(devNum).AACEngineAddress[PRV_CPSS_DXCH_AAC_CHANNEL_EM_MANAGER_SECONDARY_E];
    regData = PRV_DXCH_REG1_UNIT_EM_MAC(devNum).EMIndirectAccess.EMIndirectAccessControl;

    /* Write address of EM indirect accsess control register */
    return prvCpssDrvHwPpWriteRegister(devNum, regAddr, regData);
}

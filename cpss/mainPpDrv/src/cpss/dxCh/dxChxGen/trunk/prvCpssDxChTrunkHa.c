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
* @file prvCpssDxChTrunkHa.c
*
* @brief Trunk manager Ha support
*
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
/* get DxCh Trunk types */
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
/* get DxCh Trunk private types */
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunkHa.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvCpssDxChTrunkSyncSwHwForHa function
* @endinternal
*
* @brief  this function set trunkId into skip list for high availability
*         trunkId is added only for sip6 and if the trunk set from low level api
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,AC5P; AC5X; Harrier; Ironman;
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error.
*
*/
GT_STATUS prvCpssDxChTrunkHaSkipListCheckAndSet
(
    IN  GT_U8                   devNum,
    IN  GT_TRUNK_ID             trunkId
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;
    GT_BOOL   managerHwWriteBlock;    /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/

    if((!PRV_CPSS_TRUNK_HIGH_LEVEL_API_CALL_GET_MAC(devNum)))
    {
        rc = cpssSystemRecoveryStateGet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TRUNK_MANAGER_E);
        if (((system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)) || managerHwWriteBlock == GT_TRUE)
        {
            PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->skipListTrunkIdArry[trunkId] = GT_TRUE;
        }
    }
    return GT_OK;
}
/**
* @internal prvFixTrunkHwFromSwForHa function
* @endinternal
*
* @brief   the function overides the trunk group data given trunk type
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] trunkId                  - trunk id
* @param[in] numOfEnabledMembers      - number of enabled members in the array.
* @param[in] enabledMembersArray[]    - (array of) members to set in this trunk as enabled
*                                      members .
*                                      (this parameter ignored if numOfEnabledMembers = 0)
* @param[in] numOfDisabledMembers     - number of disabled members in the array.
* @param[in] disabledMembersArray[]   - (array of) members to set in this trunk as disabled
*                                      members .
*                                      (this parameter ignored if numOfDisabledMembers = 0)
* @param[in] CPSS_PORTS_BMP_STC       -(pointer to)cascade ports Members bitmap
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - when the sum of number of enabled members + number of
*                                    disabled members exceed the number of maximum number
*                                    of members in trunk (total of 0 - 8 members allowed)
* @retval GT_BAD_PARAM             - bad device number , or
*                                       bad trunkId number , or
*                                       bad members parameters :
*                                       (device & 0xE0) != 0 means that the HW can't support
*                                       this value , since HW has 5 bit
*                                       for device number
*                                       (port & 0xC0) != 0 means that the HW can't support
*                                       this value , since HW has 6 bit
*                                       for port number
* @retval GT_ALREADY_EXIST         - one of the members already exists in another trunk
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvFixTrunkHwFromSwForHa
(
    IN GT_U8                    devNum,
    IN GT_TRUNK_ID              trunkId,
    IN GT_U32                   numOfEnabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    enabledMembersArray[],
    IN GT_U32                   numOfDisabledMembers,
    IN CPSS_TRUNK_MEMBER_STC    disabledMembersArray[],
    IN CPSS_PORTS_BMP_STC       *portsMembersPtr,
    IN CPSS_TRUNK_TYPE_ENT      mode
)
{
    GT_STATUS rc ;

    CPSS_LOG_INFORMATION_MAC ("\nfix HW DB trunkId %d",trunkId);
    switch(mode)
    {
        case CPSS_TRUNK_TYPE_REGULAR_E:
            rc = cpssDxChTrunkMembersSet(devNum, trunkId, numOfEnabledMembers, enabledMembersArray, numOfDisabledMembers, disabledMembersArray);
            break;
        case CPSS_TRUNK_TYPE_CASCADE_E:
            rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, trunkId, portsMembersPtr);
            break;
        case CPSS_TRUNK_TYPE_FREE_E:
            rc = cpssDxChTrunkMembersSet(devNum, trunkId, 0, NULL, 0, NULL);
            if (rc != GT_OK)
            {
                return rc;
            }
            rc = cpssDxChTrunkCascadeTrunkPortsSet(devNum, trunkId, NULL);
            break;
        default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal internal_prvCpssDxChTrunkSyncSwHwForHa function
* @endinternal
*
* @brief  this function goes over all the devices and trunk groups defined in
*         the system, and for each one, check if SW DB matches HW DB,
*         in case SW doesnt match HW ,the trunk group gets overide with data from SW
*         in case the mismatch is only in inner order of group members the SW get overide
*         with data from HW
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,AC5P; AC5X; Harrier; Ironman;
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_BAD_BARAM             - wrong parameters.
* @retval GT_FAIL                  - on other error.
*
*/
static GT_STATUS internal_prvCpssDxChTrunkSyncSwHwForHa
(
    IN CPSS_TRUNK_MEMBER_STC           *tempGlobalMalloc
)
{
    GT_U32 maxTrunkGroups;  /*number of trunk groups allocated in device*/
    GT_U8 devNum ;
    GT_TRUNK_ID trunkId;
    GT_STATUS                       rc;
    GT_U32                          swNumOfEnableMembers,swNumOfDisableMembers,totalNumOfPorts,hwNumOfMembers=0;
    GT_U32                          ii,jj,swPortFound,hwPortFound;
    CPSS_TRUNK_MEMBER_STC           *hwMembersArray;
    CPSS_TRUNK_MEMBER_STC           *swEnableMembersArray;
    CPSS_TRUNK_MEMBER_STC           *swDisableMembersArray;
    CPSS_TRUNK_MEMBER_STC           tempPort;
    GT_BOOL                         isTrunkMember,swDbReorderNeeded = GT_FALSE,found;
    GT_TRUNK_ID                     trunkIdGet=0;
    CPSS_TRUNK_MEMBER_STC           *totalMemberArray = NULL;
    CPSS_PORTS_BMP_STC              nonTrunkPortsBmp,dbNonTrunkPortsBmp,designatedPortsBmpGet;
    CPSS_PORTS_BMP_STC              *designatedPortsBmpArray = NULL ;
    CPSS_PORTS_BMP_STC              cascadePorts;
    CPSS_TRUNK_TYPE_ENT             trunkType;
    GT_PHYSICAL_PORT_NUM            port;

    hwMembersArray          = &tempGlobalMalloc[0];
    swEnableMembersArray    = &tempGlobalMalloc[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
    swDisableMembersArray   = &tempGlobalMalloc[2*CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];

    /*check all devices*/
    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        if ( (PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] == NULL) ||
             (PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum) == 0) )
        {
            continue;
        }
        maxTrunkGroups = PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numberOfTrunks;
        /*check all trunk groups*/
        for (trunkId=1;trunkId<=maxTrunkGroups ;trunkId++)
        {
            /*******************init****************/
            totalNumOfPorts = 0;
            swNumOfEnableMembers = 0 ;
            swNumOfDisableMembers = 0;
            swPortFound=0;
            hwPortFound=0;
            swDbReorderNeeded = GT_FALSE;
            cpssOsMemSet(swEnableMembersArray,0,sizeof(swEnableMembersArray));
            cpssOsMemSet(swDisableMembersArray,0,sizeof(swDisableMembersArray));
            PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&cascadePorts);

            /*get SW trunk type*/
            rc = cpssDxChTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
            if (GT_OK != rc)
            {
                return rc;
            }

            /*geting SW group members */
            switch(trunkType)
            {
                case CPSS_TRUNK_TYPE_REGULAR_E:
                    /*get enable and disable ports from SW DB */
                    swNumOfEnableMembers = CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS;
                    rc = cpssDxChTrunkDbEnabledMembersGet(devNum,trunkId,&swNumOfEnableMembers,swEnableMembersArray);
                    if (GT_OK != rc)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"trunkId[%d]\n",trunkId);
                    }
                    swNumOfDisableMembers = CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS;
                    rc = cpssDxChTrunkDbDisabledMembersGet(devNum,trunkId,&swNumOfDisableMembers,swDisableMembersArray);
                    if (GT_OK != rc)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"trunkId[%d]\n",trunkId);
                    }

                    totalNumOfPorts = swNumOfEnableMembers + swNumOfDisableMembers;
                    break;

                case CPSS_TRUNK_TYPE_CASCADE_E:
                    for (port=0;port<PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);port++)
                    {
                        tempPort.port = port;
                        tempPort.hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
                        /*get cascade port member from sw DB of all the trunks*/
                        rc = cpssDxChTrunkDbIsMemberOfTrunk(devNum,&tempPort,&trunkIdGet);
                        if (GT_OK != rc)
                        {
                            if (rc == GT_NOT_FOUND)
                            {
                                continue;
                            }
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "trunkId[%d]\n", trunkId);
                        }
                        if (trunkIdGet == trunkId )
                        {
                            /*cascade ports are always enable*/
                            CPSS_PORTS_BMP_PORT_SET_MAC(&cascadePorts, port);
                            swEnableMembersArray[swNumOfEnableMembers].port = port;
                            swEnableMembersArray[swNumOfEnableMembers].hwDevice = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
                            swNumOfEnableMembers++;
                        }
                    }
                    totalNumOfPorts = swNumOfEnableMembers;
                    break;

                case CPSS_TRUNK_TYPE_FREE_E:
                    /*getting the number of group member that HW DB knows about*/
                    for (port=0;port<PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);port++)
                    {
                        rc = cpssDxChTrunkPortTrunkIdGet(devNum, port, &isTrunkMember, &trunkIdGet);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        if ((isTrunkMember == GT_TRUE) && (trunkIdGet == trunkId) )
                        {
                            hwPortFound++;
                            break;
                        }
                    }
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /*if trunk is not set skip to next trunk*/
            if (trunkType == CPSS_TRUNK_TYPE_FREE_E)
            {
                if (hwPortFound>0)
                {
                    CPSS_LOG_INFORMATION_MAC("hw have data on trunk group that not set in SW \n");
                    if(PRV_CPSS_TRUNK_HA_TRUNK_SKIP_LIST_GET_MAC(trunkIdGet))
                    {
                        CPSS_LOG_INFORMATION_MAC("trunk %d is in skip list ,nothing done \n",trunkIdGet);
                        continue;
                    }
                    else
                    {
                        CPSS_LOG_INFORMATION_MAC("clear HW data for trunk %d \n",trunkIdGet);
                        rc = prvFixTrunkHwFromSwForHa(devNum, trunkIdGet, 0, NULL, 0, NULL,NULL,trunkType);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                }
                continue;
            }

            totalMemberArray = cpssOsMalloc(sizeof(CPSS_TRUNK_MEMBER_STC) * (totalNumOfPorts));
            if (totalMemberArray == NULL )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"trunkId[%d]\n",trunkId);
            }
            cpssOsMemCpy(totalMemberArray, swEnableMembersArray, sizeof(CPSS_TRUNK_MEMBER_STC) * swNumOfEnableMembers);
            cpssOsMemCpy(&totalMemberArray[swNumOfEnableMembers], swDisableMembersArray, sizeof(CPSS_TRUNK_MEMBER_STC)*swNumOfDisableMembers);

/**********port reg*******************************************************************************************************************/
            swPortFound=0;
            hwPortFound=0;
            /*check SW port members in port register */
            for(ii=0;ii<totalNumOfPorts;ii++)
            {
                port =  totalMemberArray[ii].port;
                if (trunkType == CPSS_TRUNK_TYPE_CASCADE_E)
                {
                    swPortFound = totalNumOfPorts; /*all cascade ports belong to device*/
                }
                else /*REGULAR*/
                {
                    /*skip ports of other devices*/
                    if (totalMemberArray[ii].hwDevice != PRV_CPSS_HW_DEV_NUM_MAC(devNum))
                    {
                        continue;
                    }
                    swPortFound++;
                }

                /*get data from HW*/
                rc = cpssDxChTrunkPortTrunkIdGet(devNum, port, &isTrunkMember, &trunkIdGet);
                if (rc != GT_OK)
                {
                    FREE_PTR_MAC(totalMemberArray);
                    return rc;
                }

                if (isTrunkMember != GT_TRUE || trunkIdGet != trunkId)
                {
                    CPSS_LOG_INFORMATION_MAC("\nError port reg: SW DB have data that missing in HW DB . port[%d] trunkId %d ",port,trunkId);
                    goto fixTrunk_lbl;
                }
            }

            /*getting the number of group member that hw know about*/
            for (port=0;port<PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);port++)
            {
                rc = cpssDxChTrunkPortTrunkIdGet(devNum, port, &isTrunkMember, &trunkIdGet);
                if (rc != GT_OK)
                {
                    FREE_PTR_MAC(totalMemberArray);
                    return rc;
                }
                if ((isTrunkMember == GT_TRUE) && (trunkIdGet == trunkId) )
                {
                    hwPortFound++;
                }
            }
            if (hwPortFound != swPortFound)
            {
                CPSS_LOG_INFORMATION_MAC("\nError port reg: HW DB have data that missing in SW DB . HW[%d] SW[%d]  trunkId %d ",hwPortFound,swPortFound,trunkId);
                goto fixTrunk_lbl;
            }

/***********member table**************************************************************************************************************/
            if (trunkType == CPSS_TRUNK_TYPE_REGULAR_E)/*cascade ports skip this part*/
            {
                rc = cpssDxChTrunkTableEntryGet(devNum, trunkId, &hwNumOfMembers, hwMembersArray);
                if(rc != GT_OK)
                {
                    FREE_PTR_MAC(totalMemberArray);
                    return rc;
                }

                if (hwNumOfMembers != swNumOfEnableMembers)
                {
                    CPSS_LOG_INFORMATION_MAC("\nError member table : HW DB have data that missing in SW DB, diffrent number of port members HW[%d] SW[%d}.trunkId %d ",
                                             hwNumOfMembers,swNumOfEnableMembers,trunkId);
                    goto fixTrunk_lbl;
                }
                /*need to check only enable port members*/
                for (ii=0;ii<swNumOfEnableMembers;ii++)
                {
                    found = GT_FALSE;
                    for (jj=0;jj<hwNumOfMembers;jj++)
                    {
                        if ((swEnableMembersArray[ii].port ==  hwMembersArray[jj].port) &&
                            (swEnableMembersArray[ii].hwDevice ==  hwMembersArray[jj].hwDevice))
                        {
                            found=GT_TRUE;
                            if (ii!=jj)
                            {
                                /*set flag to reorder the trunk group members in case all tests pass*/
                                swDbReorderNeeded=GT_TRUE;
                            }
                            break;
                        }
                    }
                    if (found == GT_FALSE)
                    {
                        CPSS_LOG_INFORMATION_MAC("\nError SW DB have data that missing in HW DB .  trunkId %d ",trunkId);
                        goto fixTrunk_lbl;
                    }
                }
            }

/***************swDbReorderNeeded*****************************************************************************************************/
            if (swDbReorderNeeded == GT_TRUE)
            {
                CPSS_LOG_INFORMATION_MAC("swDbReorderNeeded == GT_TRUE trunkId %d \n ",trunkId);
                /*set SW DB with same member order as HW */
                rc = prvCpssGenericTrunkSwDbUpdateOnly(devNum, trunkId, hwNumOfMembers, hwMembersArray, swNumOfDisableMembers,swDisableMembersArray);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /*run again the loop*/
                trunkId--;
                continue;
            }

/***********nonTrunk******************************************************************************************************************/
            /*get SW nonTrunk port members*/
            rc = prvCpssGenericTrunkDbNonTrunkPortsCalcForHa(devNum,trunkId,&cascadePorts,&dbNonTrunkPortsBmp);
            if(rc != GT_OK)
            {
                FREE_PTR_MAC(totalMemberArray);
                return rc;
            }
            /*get HW nonTrunk port members*/
            rc = cpssDxChTrunkNonTrunkPortsEntryGet(devNum,trunkId,&nonTrunkPortsBmp);
            if(rc != GT_OK)
            {
                FREE_PTR_MAC(totalMemberArray);
                return rc;
            }

            if (!(PRV_CPSS_PORTS_BMP_ARE_EQUAL_MAC(&dbNonTrunkPortsBmp,&nonTrunkPortsBmp)))
            {
                CPSS_LOG_INFORMATION_MAC("\nError  NonTrunkPortsBmp SW and HW doesnt match,trunkId %d",trunkId);
                goto fixTrunk_lbl;
            }

/***********designatedPorts***********************************************************************************************************/
            designatedPortsBmpArray = cpssOsMalloc(sizeof(CPSS_PORTS_BMP_STC) * (PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw));
            if (designatedPortsBmpArray == NULL )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG) ;
            }
            /*get designated Ports for all indexes given sw nonTrunk members*/
            rc = prvCpssGenericTrunkDbDesignatedPortsCalcForHa(devNum, trunkId,&dbNonTrunkPortsBmp,designatedPortsBmpArray);
            if(rc != GT_OK)
            {
                cpssOsFree(designatedPortsBmpArray);
                return rc;
            }
            for (ii=0;ii<PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numOfDesignatedTrunkEntriesHw;ii++)
            {
                /*get designated Ports from HW */
                rc = cpssDxChTrunkDesignatedPortsEntryGet(devNum,ii,&designatedPortsBmpGet);
                if(rc != GT_OK)
                {
                    cpssOsFree(designatedPortsBmpArray);
                    return rc;
                }

                if (!(PRV_CPSS_PORTS_BMP_ARE_EQUAL_MAC(&designatedPortsBmpArray[ii], &designatedPortsBmpGet)))
                {
                    CPSS_LOG_INFORMATION_MAC("\nError, designatedPortsBmp calc from sw doesnt match HW designatedPortsBmpGet  trunkId %d index %d ",trunkId,ii);
                    goto fixTrunk_lbl;
                }
            }
            FREE_PTR_MAC(designatedPortsBmpArray);
            FREE_PTR_MAC(totalMemberArray);
            continue;

            fixTrunk_lbl:
                rc = prvFixTrunkHwFromSwForHa(devNum, trunkId, swNumOfEnableMembers, swEnableMembersArray, swNumOfDisableMembers, swDisableMembersArray, &cascadePorts, trunkType);
                if (rc != GT_OK)
                {
                    FREE_PTR_MAC(totalMemberArray);
                    FREE_PTR_MAC(designatedPortsBmpArray);
                    return rc;
                }

                FREE_PTR_MAC(totalMemberArray);
                FREE_PTR_MAC(designatedPortsBmpArray);
        }
        FREE_PTR_MAC(PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->skipListTrunkIdArry);

    }
    return GT_OK;
}
/**
* @internal prvCpssDxChTrunkSyncSwHwForHa function
* @endinternal
*
* @brief  this function goes over all the devices and trunk groups defined in
*         the system, and for each one, check if SW DB matches HW DB,
*         in case SW doesnt match HW ,the trunk group gets overide with data from SW
*         in case the mismatch is only in inner order of group members the SW get overide
*         with data from HW
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,AC5P; AC5X; Harrier; Ironman;
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on failed to allocate CPU memory
* @retval GT_BAD_BARAM             - wrong parameters.
* @retval GT_FAIL                  - on other error.
*
*/
GT_STATUS prvCpssDxChTrunkSyncSwHwForHa
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_TRUNK_MEMBER_STC           *tempGlobalMalloc;

    /* do single malloc , so able to do single free */
    tempGlobalMalloc = cpssOsMalloc(sizeof(CPSS_TRUNK_MEMBER_STC)* 3 * CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS);
    if(tempGlobalMalloc == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM,"tempGlobalMalloc\n");
    }

    rc = internal_prvCpssDxChTrunkSyncSwHwForHa(tempGlobalMalloc);

    cpssOsFree(tempGlobalMalloc);

    return rc;
}

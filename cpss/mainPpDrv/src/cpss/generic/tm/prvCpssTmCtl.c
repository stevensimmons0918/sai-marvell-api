/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssTmCtl.c
*
* DESCRIPTION:
*       Private TM Configuration Library Utilities
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*
*******************************************************************************/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <tm_ctl.h>
#include <tm_nodes_read.h>
#include <tm_drop.h>
#include <platform/tm_regs.h>
#include <platform/cpss_tm_rw_registers_proc.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>
#include <cpss/generic/tm/cpssTmDrop.h>
/* for definition of  DROP_MEMORY_BURST_SIZE */
#include <platform/tm_core_types_platform.h>
/*#include <platform/cpss/generic/tm/prvCpssTmDefs.h>*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal prvCpssTmGetInternalDevId function
* @endinternal
*
* @brief   Get TM Internal Device ID.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
*
* @retval PRV_CPSS_TM_DEVS_ENT != PRV_CPSS_TM_DEV_LAST_E - on success
*/
PRV_CPSS_TM_DEVS_ENT prvCpssTmGetInternalDevId
(
    IN  GT_U8   devNum
)
{
    PRV_CPSS_TM_DEVS_ENT prvCpssTmDevId = PRV_CPSS_TM_DEV_LAST_E;

    if (!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return prvCpssTmDevId;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        return prvCpssTmDevId;
    }

    switch (PRV_CPSS_PP_MAC(devNum)->devSubFamily)
    {
        case CPSS_PP_SUB_FAMILY_NONE_E:
            prvCpssTmDevId = PRV_CPSS_TM_DEV_BC2_E;
            break;
        case CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E:
            switch (PRV_CPSS_PP_MAC(devNum)->devType)
            {
                case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                    prvCpssTmDevId = PRV_CPSS_TM_DEV_BOBK_CETUS_E;
                    break;
                case CPSS_BOBK_CAELUM_DEVICES_CASES_MAC:
                    prvCpssTmDevId = PRV_CPSS_TM_DEV_BOBK_CAELUM_E;
                    break;
            }
            break;

        default:
            prvCpssTmDevId = PRV_CPSS_TM_DEV_LAST_E;
    }

    return prvCpssTmDevId;
}



/**
* @internal prvCpssTmDumpPortTreeFromHW function
* @endinternal
*
* @brief   print configured tree under a specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portIndex                - port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmDumpPortTreeFromHW
(
    IN GT_U8                   devNum,
    IN GT_U32                  portIndex
)
{
    GT_STATUS                  rc = GT_OK;

    GT_U32                     portFirstChild;
    GT_U32                     portLastChild;
    GT_U32                     cFirstChild;
    GT_U32                     cLastChild;
    GT_U32                     bFirstChild;
    GT_U32                     bLastChild;
    GT_U32                     aFirstChild;
    GT_U32                     aLastChild;
    GT_U64                     regData;
    GT_U64                     regAdd;
    GT_U32                     ic,ib,ia,iq;
    regData.l[0]=0;
    regData.l[1]=0;
    regAdd.l[0] =TM.Sched.PortRangeMap.l[0]+(portIndex*8);
    regAdd.l[1] =TM.Sched.PortRangeMap.l[1];

    rc = cpssTmCtlReadRegister( devNum, regAdd, &regData);
    if (rc)
    {
        return rc;
    }
    portFirstChild =( (regData.l[0])& 0x1FF);
    portLastChild = ((regData.l[0]>>16) & 0x1FF) ;

    cpssOsPrintf("  Port (index= %d)\n",portIndex);

    for (ic = portFirstChild ; ic<=portLastChild ; ic++)
    {
        cpssOsPrintf("  C node (index= %d)\n",ic);

        regData.l[0]=0;
        regData.l[1]=0;
        regAdd.l[0] =TM.Sched.ClvltoPortAndBlvlRangeMap.l[0]+(ic*8);
        regAdd.l[1] =TM.Sched.ClvltoPortAndBlvlRangeMap.l[1];

        rc = cpssTmCtlReadRegister( devNum, regAdd, &regData);
        if (rc)
        {
            return rc;
        }
        cFirstChild =( (regData.l[0])& 0x7FF);
        cLastChild = ((regData.l[0]>>16) & 0x7FF );
        if (portIndex != ((regData.l[1])& 0xFF))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        for (ib = cFirstChild ; ib <= cLastChild ; ib++)
        {
            cpssOsPrintf("  B node (index= %d)\n",ib);
            regData.l[0]=0;
            regData.l[1]=0;
            regAdd.l[0] =TM.Sched.BLvltoClvlAndAlvlRangeMap.l[0]+(ib*8);
            regAdd.l[1] =TM.Sched.BLvltoClvlAndAlvlRangeMap.l[1];
            rc = cpssTmCtlReadRegister( devNum, regAdd, &regData);
            if (rc)
            {
                return rc;
            }
            bFirstChild = ((regData.l[0])& 0xFFF);
            bLastChild = ((regData.l[0]>>16) & 0xFFF) ;
            if (ic != ((regData.l[1])& 0x1FF))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            for (ia = bFirstChild ; ia <= bLastChild ; ia++)
            {
                cpssOsPrintf("  A node (index= %d)\n",ia);
                regData.l[0]=0;
                regData.l[1]=0;
                regAdd.l[0] =TM.Sched.ALvltoBlvlAndQueueRangeMap.l[0]+(ia*8);
                regAdd.l[1] =TM.Sched.ALvltoBlvlAndQueueRangeMap.l[1];
                rc = cpssTmCtlReadRegister( devNum, regAdd, &regData);
                if (rc)
                {
                    return rc;
                }
                aFirstChild = ((regData.l[0])& 0x3FFF);
                aLastChild = ((regData.l[0]>>16) & 0x3FFF );
                if (ib != ((regData.l[1])& 0x7FF)) {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                cpssOsPrintf("  queue:");

                for (iq = aFirstChild ; iq <= aLastChild ; iq++)
                {
                    cpssOsPrintf(" (index= %d)",iq);
                }
                cpssOsPrintf(" \n");
            }
        }
    }
    return rc;
}

static int xml_dump_port_fun(int bBeginNodeEval,
                             void * vpEnv,
                             unsigned int level,
                             unsigned int node_index,
                             unsigned int node_status,
                             unsigned int node_parent_index ,
                             unsigned int elig_fun ,
                             void * vpUserData)
{
    GT_U8    devNum;
    devNum = *((GT_U8*)(vpEnv));

    (void)vpUserData;
    (void)node_parent_index;
    (void)devNum;
    (void)node_status;

    switch(level)
    {
        case CPSS_TM_LEVEL_P_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("port :  index = %d    elig_fun = %d\n",node_index,elig_fun);
            break;
        }
        case CPSS_TM_LEVEL_C_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("      C_node : index = %d    elig_fun = %d\n",node_index, elig_fun);
            break;
        }
        case CPSS_TM_LEVEL_B_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("            B_node :  index = %d    elig_fun = %d\n",node_index, elig_fun);
            break;
        }
        case CPSS_TM_LEVEL_A_E:
        {
            if (bBeginNodeEval) cpssOsPrintf("                  A_node :  index = %d    elig_fun=%d\n",node_index, elig_fun);
            break;
        }
        case CPSS_TM_LEVEL_Q_E:
        {
            /* queue callback is called once */
                                cpssOsPrintf("                        Queue :  index = %d    elig_fun = %d\n",node_index, elig_fun);
            break;
        }
        default :               cpssOsPrintf(" unknown level=%d index=%d\n",level,node_index); return 1;
    }

    return 0;
}


/**
* @internal internal_prvCpssTmDumpPort function
* @endinternal
*
* @brief   print the configure tree under a specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portIndex                - port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
static GT_STATUS internal_prvCpssTmDumpPort
(
    IN GT_U8                   devNum,
    IN GT_U32                  portIndex
)
{
    GT_STATUS                  rc = GT_OK;
        int                        ret;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    ret = tm_traverse_port_tree_sw(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    portIndex,
                                    xml_dump_port_fun,
                                    &devNum,
                                    NULL);
    if (ret) rc = XEL_TO_CPSS_ERR_CODE(ret);
    return rc;
}

/**
* @internal prvCpssTmDumpPort function
* @endinternal
*
* @brief   print configured tree under a specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portIndex                - port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmDumpPort
(
    IN GT_U8    devNum,
    IN GT_U32   portIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmDumpPort);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portIndex));

    rc = internal_prvCpssTmDumpPort(devNum, portIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

static GT_STATUS internal_prvCpssTmCtlDumpPortHW
(
    IN GT_U8    devNum,
    IN GT_U32   portIndex
)
{
    GT_STATUS                  rc = GT_OK;
    int                        ret;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    ret = tm_traverse_port_tree_hw(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    portIndex,
                                    xml_dump_port_fun,
                                    &devNum,
                                    NULL);
    if (ret) rc = XEL_TO_CPSS_ERR_CODE(ret);
    return rc;
}

/**
* @internal prvCpssTmCtlDumpPortHW function
* @endinternal
*
* @brief   print from HW configured tree under a specific port
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] portIndex                - port index.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmCtlDumpPortHW
(
    IN GT_U8    devNum,
    IN GT_U32   portIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmDumpPort);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portIndex));

    rc = internal_prvCpssTmCtlDumpPortHW(devNum, portIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal prvCpssTmCtlLadParamsGetHW function
* @endinternal
*
* @brief   Get TM LAD parameters from HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] numOfLadsPtr             - (out pointer of)LADs number.
* @param[in] ladParamsPtr             - (out pointer of) CPSS_DXCH_TM_GLUE_LAD_INF_PARAM_STC.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssTmCtlLadParamsGetHW
(
    IN  GT_U8                         devNum,
    OUT GT_U32                        *numOfLadsPtr,
    OUT CPSS_TM_CTL_LAD_INF_PARAM_STC *ladParamsPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U64      regData;
    GT_U64      readReg;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(numOfLadsPtr);
    CPSS_NULL_PTR_CHECK_MAC(ladParamsPtr);

    /* number of lads */
    rc = cpssTmCtlReadRegister(devNum, TM.QMgr.NumOfLADs, &regData);
    if (rc) {
        return rc;
    }
    *numOfLadsPtr = regData.l[0];

    /* minPkgSize */
    rc = cpssTmCtlReadRegister(devNum, TM.QMgr.MinPkgSize, &regData);
    if (rc) {
        return rc;
    }
    ladParamsPtr->minPkgSize = regData.l[0];

    /* numOfPagesPerBank */
    rc = cpssTmCtlReadRegister(devNum, TM.QMgr.NumOfPagesPerBank, &regData);
    if (rc) {
        return rc;
    }
    ladParamsPtr->pagesPerBank = regData.l[0];

    /* numOfPkgesPerBank */
    rc = cpssTmCtlReadRegister(devNum, TM.QMgr.NumOfPkgsPerBank, &regData);
    if (rc) {
        return rc;
    }
    ladParamsPtr->pkgesPerBank = regData.l[0];

    /* PortChunksEmitPerSel for port 191*/
    readReg.l[0] = TM.QMgr.PortChunksEmitPerSel.l[0] + (191*8);
    readReg.l[1] = TM.QMgr.PortChunksEmitPerSel.l[1];
    rc = cpssTmCtlReadRegister(devNum,  readReg, &regData);
    if (rc) {
        return rc;
    }

    ladParamsPtr->portChunksEmitPerSel = regData.l[0];

    return rc;
}


/* private function , for usage in command string */
/**
* @internal prvCpssTmDropQueueProfileIndexRead function
* @endinternal
*
* @brief   As a result of aging process the queue drop profile can be changed dynamically
*         internally by TM. This function allows to monitor ths changes.
*         this private function is for usage in command string
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] queueIndex               - The index of queue to read it,s profile
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/

GT_STATUS prvCpssTmDropQueueProfileIndexRead
(
    IN GT_U8                    devNum,
    IN GT_U32                   queueIndex
)
{
    GT_STATUS rc;
    GT_U32  dummy=(GT_U32)-1;

    rc = cpssTmNodeDropProfileIndexRead(devNum, CPSS_TM_LEVEL_Q_E, 0 , queueIndex, &dummy);
    if (GT_OK == rc)
    {
        cpssOsPrintf("   ReadQueueDropProfile  queue=%d, profile =%d\n",queueIndex , dummy);
    }

    return rc;
}


/* private function , for usage in command string */
/**
* @internal prvCpssTmPrintDropProfileParams function
* @endinternal
*
* @brief   The function reads drop profile data from H/W , reverse calculates drop profile parameters
*         and prints them.
*         this private function is for usage in command string
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] level                    - Level    : Queue / A- / B- / C- / Port
* @param[in] cos                      - CoS  - 0:7  for C-Level , 0:7 or  invalid for Port , invalid  for other levels
* @param[in] profileIndex             - The index of profile to read and print
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/

GT_STATUS prvCpssTmPrintDropProfileParams
(
    IN GT_U8                    devNum,
    IN  CPSS_TM_LEVEL_ENT       level,
    IN  GT_U32                  cos,
    IN  GT_U32                  profileIndex
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;
    struct      tm_drop_profile dp_hw_params;
    int         i;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);


    ret = tm_read_drop_profile_hw(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle,
                                    level,
                                    (uint8_t)cos,
                                    profileIndex,
                                    &dp_hw_params);

    if (ret) rc = XEL_TO_CPSS_ERR_CODE(ret);
    else
    {
/*      cpssOsPrintf("td_thresh_res:        %d\n", dp_hw_params.td_thresh_res);
        cpssOsPrintf("td_threshold:         %d\n", dp_hw_params.td_threshold_bursts * DROP_MEMORY_BURST_SIZE );
*/
        cpssOsPrintf("td_threshold:        %d\n",  TM_ENABLE == dp_hw_params.td_thresh_res ? dp_hw_params.td_threshold_bursts * DROP_MEMORY_BURST_SIZE * DROP_TD_RESOLUTION_MULTIPLYER : dp_hw_params.td_threshold_bursts * DROP_MEMORY_BURST_SIZE );
        cpssOsPrintf("                               estimated td_threshold:         %d\n", dp_hw_params.drop_profile_source.cbtdBytes );




        if (dp_hw_params.color_td_en)
        {
            cpssOsPrintf("color_td_en:       %d\n", dp_hw_params.color_td_en);

            for (i = 0; i < 3; i++)
            {
                cpssOsPrintf("min_threshold[%d]: %d\n", i, dp_hw_params.min_threshold[i] * DROP_MEMORY_BURST_SIZE * (1<<dp_hw_params.scale_exp[i]));
                cpssOsPrintf("                 estimated min_threshold[%d]: %d\n", i, dp_hw_params.drop_profile_source.caTdDp.catdBytes[i]);
            }

        }
        else
        {
            cpssOsPrintf("aql_exp: %d\n", dp_hw_params.aql_exp);
/*
            cpssOsPrintf("              estimated aql_exp: %d\n", dp_hw_params.drop_profile_source.wredDp.aqlExponent);
*/
           for (i = 0; i < 3; i++)
            {
                cpssOsPrintf("dp_ratio[%d]:      %d\n", i, dp_hw_params.dp_ratio[i]);
                cpssOsPrintf("curve_ind[%d]:     %d\n", i, dp_hw_params.scale_exp[i]);
                cpssOsPrintf("min_threshold[%d]: %d\n", i, dp_hw_params.min_threshold[i]);
                cpssOsPrintf("scale_exp[%d]:     %d\n", i, dp_hw_params.scale_exp[i]);
                cpssOsPrintf("scale_ratio[%d]:   %d\n", i, dp_hw_params.scale_ratio[i]);
            }
        }
    }
    return rc;
}


/*******************************************************************************
*              (c), Copyright 2014, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChDiag.c
*
* DESCRIPTION:
*       A lua wrapper for Diag functions
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/********************************************************************************
 *   Lua wrappers warnings                                                      *
 ********************************************************************************/
GT_CHAR wrongDataIntegrityMemoryTypeErrorMessage[] =
                                    "Wrong type of integrity memory";

/********************************************************************************
 *   Lua wrappers declarations                                                          *
 ********************************************************************************/

use_prv_struct(CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC)

add_mgm_enum(MPRV_CPSS_DXCH_DIAG_EVENT_ADDITION_INFO_ENT);

/*******************************************************************************
* pvrCpssDiagDataIntegrityMemoryLocationFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of data integrity memory location getting from lua stack.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                         - lua state
*       L_index                   - entry lua stack index
*       memType                   - data integrity memory type
*
* OUTPUTS:
*       locationPtr               - data integrity memory location
*       errorMessagePtr           - error message
*
* RETURNS:
*       operation succeeded or error code if occur
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pvrCpssDiagDataIntegrityMemoryLocationFromLuaGet
(
    IN  lua_State                                       *L,
    IN  GT_32                                           L_index,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    *locationPtr,
    OUT GT_CHAR_PTR                                     *errorMessagePtr
)
{
    GT_STATUS                   status          = GT_OK;

    *errorMessagePtr = NULL;

    lua_pushvalue(L, L_index);


    if(memType >= CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_LION2_LAST_E)
    {
        status              = GT_BAD_PARAM;
        *errorMessagePtr    = wrongDataIntegrityMemoryTypeErrorMessage;
        lua_pop(L, 1);
        return status;
    }

    switch (memType)
    {
    /*  tcamMemLocation  - special memory location indexes for TCAM memory    */
    case  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ROUTER_TCAM_E:
    case  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_POLICY_TCAM_E:
    case  CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_EPCL_TCAM_DESC_FIFO_E:
        PARAM_ENUM  (status,  locationPtr->tcamMemLocation.arrayType,  L_index, CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_ENT)
        PARAM_NUMBER(status, locationPtr->tcamMemLocation.ruleIndex, L_index+1, GT_U32);
        break;

    /*  mppmMemLocation  - special memory location indexes for MPPM memory    */
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E:
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E:
    case CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_DATA_FROM_MPPM_E:
        PARAM_NUMBER(status, locationPtr->mppmMemLocation.bankId, L_index, GT_U32);
        PARAM_NUMBER(status, locationPtr->mppmMemLocation.mppmId, L_index+1, GT_U32);
        PARAM_NUMBER(status, locationPtr->mppmMemLocation.portGroupId, L_index+2, GT_U32);
        break;

    /*  memLocation      - regular memory location indexes                    */
    default:
        PARAM_NUMBER(status, locationPtr->memLocation.dfxClientId, L_index, GT_U32);
        PARAM_NUMBER(status, locationPtr->memLocation.dfxMemoryId, L_index+1, GT_U32);
        PARAM_NUMBER(status, locationPtr->memLocation.dfxPipeId,   L_index+2,   GT_U32);
        break;
    }

    lua_pop(L, 1);

    return status;
}


/*******************************************************************************
* wrlCpssDxChDiagDataIntegrityEventsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       returns array of data integrity events.
*
* APPLICABLE DEVICES:
*        Lion2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*
* RETURNS:
*       status, CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC[]
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChDiagDataIntegrityEventsGet
(
    IN lua_State *L
)
{
    GT_U8       devNum;
    GT_U32      portGroupId;
    GT_U32      evType = 0;
    GT_STATUS   status = GT_OK;
    GT_U32      evExtData;
    GT_U32      eventsNum = 20;     /* initial number of ECC/parity events */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC eventsArr[20];
    GT_BOOL     isScanFinished = GT_FALSE;     /* events scan finish status */
    GT_U32      i, idx;

    PARAM_NUMBER(status, devNum,      1, GT_U8);
    PARAM_NUMBER(status, portGroupId, 2, GT_U32);
    /* PARAM_ENUM  (status,  evType,  3, MPRV_CPSS_DXCH_DIAG_EVENT_ADDITION_INFO_ENT) */
    if (lua_type(L, 3) == LUA_TNUMBER || lua_type(L, 3) == LUA_TSTRING)
    {
        lua_pushvalue(L, 3);
        evType = (GT_U32)prv_lua_to_c_enum(L, "MPRV_CPSS_DXCH_DIAG_EVENT_ADDITION_INFO_ENT");
        lua_pop(L, 1);
    } else {
        status = GT_BAD_PARAM;
    }

    evExtData = (portGroupId << 16) | evType;

    lua_pushinteger(L, (lua_Integer)status);
    if (status != GT_OK)
    {
        return 1;
    }

    lua_newtable(L);
    idx = 0;

    /* read data */
    while(isScanFinished != GT_TRUE)
    {
        status = cpssDxChDiagDataIntegrityEventsGet(devNum, evExtData, &eventsNum, eventsArr, &isScanFinished);
        if(GT_OK != status)
        {
            break;
        }

        for(i = 0; i < eventsNum; i++)
        {
            prv_c_to_lua_CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC(L, &(eventsArr[i]));
            lua_rawseti(L, -2, ++idx);
        }
    }

    lua_pushinteger(L, status);
    lua_replace(L, -3);

    return 2;
}


/*******************************************************************************
* wrlCpssDxChDiagDataIntegrityErrorInfoGet
*
* DESCRIPTION:
*       Function gets ECC/Parity error info.
*
* APPLICABLE DEVICES:
*       Lion2.
*
* NOT APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; xCat2; Lion; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       L                            - lua state:
*       L[1]       devNum            - PP device number
*       L[2]       memType           - type of memory(table)
*       L[3,4,5]   locationPtr       - (pointer to) memory location indexes
*
* OUTPUTS:
*       pushed to lua stack:
*           errorCounterPtr    - (pointer to) error counter
*           failedRowPtr       - (pointer to) failed raw
*           failedSyndromePtr  - (pointer to) failed syndrome
*
* RETURNS:
*       status
*
* COMMENTS:
*       Error status or counter that isn't supported returns 0xFFFFFFFF
*
*******************************************************************************/

int wrlCpssDxChDiagDataIntegrityErrorInfoGet
(
    IN lua_State *L
)
{
    GT_U8                                           devNum;
    CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT    location;
    GT_U32                                          errorCounter;
    GT_U32                                          failedRow;
    GT_U32                                          failedSegment;
    GT_U32                                          failedSyndrome;
    GT_STATUS                                       status             = GT_OK;
    GT_CHAR_PTR                                     error_message      = NULL;

    PARAM_NUMBER(status, devNum,  1, GT_U8);
    PARAM_ENUM  (status,  memType,  2, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT)
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    status = pvrCpssDiagDataIntegrityMemoryLocationFromLuaGet(L, 3, memType, &location, &error_message);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }
    /* call cpss api function */
    {
        CPSS_TBD_BOOKMARK
       /* TODO: temporary WA to avoid compilation error.
        * need to update this wrapper according to changed API prototype:
        * "IN CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC *memEntryPtr"
        * instead of
        * "IN CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType" */
        CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC memEntry;
        CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmPtr = NULL;
        cpssOsMemSet(&memEntry, 0, sizeof(memEntry));
        if((memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_E ||
            memType == CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_DATA_BUFFER_MEMORY_ECC_E)
           && PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            mppmPtr = &location.mppmMemLocation;
        }

        memEntry.type = CPSS_DXCH_LOCATION_RAM_INDEX_INFO_TYPE;
        memEntry.info.ramEntryInfo.memType = memType;
        memEntry.info.ramEntryInfo.memLocation.dfxClientId = location.memLocation.dfxPipeId;
        memEntry.info.ramEntryInfo.memLocation.dfxClientId = location.memLocation.dfxClientId;
        memEntry.info.ramEntryInfo.memLocation.dfxClientId = location.memLocation.dfxMemoryId;

        status = cpssDxChDiagDataIntegrityErrorInfoGet(devNum, &memEntry, mppmPtr,
                                                       &errorCounter, &failedRow,
                                                       &failedSegment, &failedSyndrome);
    }



    lua_pushinteger(L, (lua_Integer)status);
    lua_pushinteger(L, (lua_Integer)errorCounter);
    lua_pushinteger(L, (lua_Integer)failedRow);
    lua_pushinteger(L, (lua_Integer)failedSyndrome);
    return 4;
}

int wrlCpssDxChDiagPrbsTimeInfoGet
(
    IN lua_State *L
)
{
    GT_U8      devNum;
    GT_U32     laneNum;
    GT_U32     startTimeSec;
    GT_U32     startTimeNsec;
    GT_U32     readTimeSec;
    GT_U32     readTimeNsec;
    GT_STATUS  status  = GT_OK;

    PARAM_NUMBER(status, devNum,  1, GT_U8);
    PARAM_NUMBER(status, laneNum,  2, GT_U32);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    startTimeSec  = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].startTime.seconds;
    startTimeNsec = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].startTime.nanoSeconds;
    readTimeSec   = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].readTime.seconds;
    readTimeNsec  = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].readTime.nanoSeconds;

    lua_pushinteger(L, (lua_Integer)status);
    lua_pushinteger(L, (lua_Integer)startTimeSec);
    lua_pushinteger(L, (lua_Integer)startTimeNsec);
    lua_pushinteger(L, (lua_Integer)readTimeSec);
    lua_pushinteger(L, (lua_Integer)readTimeNsec);
    return 5;
}

int wrlCpssDxChDiagPrbsErrorCntrGet
(
    IN lua_State *L
)
{
    GT_U8      devNum;
    GT_U32     laneNum;
    GT_U32     errorCntr;
    GT_STATUS  status  = GT_OK;

    PARAM_NUMBER(status, devNum,  1, GT_U8);
    PARAM_NUMBER(status, laneNum, 2, GT_U32);
    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    errorCntr     = PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsErrorCntr[laneNum];

    lua_pushinteger(L, (lua_Integer)status);
    lua_pushinteger(L, (lua_Integer)errorCntr);
    return 2;
}

int wrlCpssDxChDiagPrbsClearDb
(
    IN lua_State *L
)
{
    GT_U8      devNum;
    GT_U32     laneNum;
    GT_U32     seconds;
    GT_U32     nanoSeconds;
    GT_STATUS  status  = GT_OK;

    cpssOsTimeRT(&seconds, &nanoSeconds);

    PARAM_NUMBER(status, devNum,  1, GT_U8);
    PARAM_NUMBER(status, laneNum, 2, GT_U32);

    if (status != GT_OK)
    {
        lua_pushinteger(L, (lua_Integer)status);
        return 1;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].startTime.seconds = seconds;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].startTime.nanoSeconds = nanoSeconds;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].readTime.seconds = seconds;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsTimeStamp[laneNum].readTime.nanoSeconds = nanoSeconds;
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.prbsErrorCntr[laneNum] = 0;

    lua_pushinteger(L, (lua_Integer)status);
    return 1;
}

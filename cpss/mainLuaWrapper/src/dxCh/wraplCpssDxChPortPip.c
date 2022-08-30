 /*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChPortPip.c
*
* DESCRIPTION:
*         CPSS DxCh Port's Pre-Ingress Prioritization (PIP).
*
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <generic/private/prvWraplGeneral.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpssCommon/wrapCpssDebugInfo.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>

/*******************************************************************************
* wrlCpssDxChPortPipGlobalDropCounterGet
*
* DESCRIPTION:
    wrapper for:
    GT_STATUS cpssDxChPortPipGlobalDropCounterGet
    (
        IN GT_U8        devNum,
        OUT GT_U64      countersArr[4]
    );
*
* APPLICABLE DEVICES:
*        Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; Lion2; xCat2.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       sip5 :7 values  , sip5_20: 9 values
*       1. rc
*       2. priority[0].low_32bits
*       3. priority[0].high_32bits
*       4. priority[1].low_32bits
*       5. priority[1].high_32bits
*       6. priority[2].low_32bits
*       7. priority[2].high_32bits
*       8. priority[3].low_32bits  (sip5_20)
*       9. priority[3].high_32bits (sip5_20)
*
*   for sip6 : single counter , so return 3 values:
*       1. rc
*       2. counter.low_32bits
*       3. counter.high_32bits
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPortPipGlobalDropCounterGet
(
    IN lua_State                            *L
)
{
    GT_STATUS                                   rc;
    GT_U32                                      ii;
    GT_U8                                       devNum;
    GT_U64      countersArr[4];
    P_CALLING_CHECKING;

    devNum            = (GT_U8)                          lua_tonumber(L, 1);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc  = cpssDxChPortRxDmaGlobalDropCounterGet(devNum, CPSS_DATA_PATH_UNAWARE_MODE_CNS, &countersArr[0]);

        P_CALLING_API(
            cpssDxChPortRxDmaGlobalDropCounterGet,
            PC_NUMBER(devNum,  devNum,  GT_U8,
            PC_LAST_PARAM),
            rc);
        lua_pushinteger(L, rc);
        lua_pushinteger(L, countersArr[0].l[0]);
        lua_pushinteger(L, countersArr[0].l[1]);
        return 3;
    }


    rc = cpssDxChPortPipGlobalDropCounterGet(devNum, countersArr);

    P_CALLING_API(
        cpssDxChPortPipGlobalDropCounterGet,
        PC_NUMBER(devNum,                       devNum,                  GT_U8,
        PC_LAST_PARAM),
        rc);

    lua_pushinteger(L, rc);

    ii=0;
    lua_pushinteger(L, countersArr[ii].l[0]);
    lua_pushinteger(L, countersArr[ii].l[1]);
    ii++;

    lua_pushinteger(L, countersArr[ii].l[0]);
    lua_pushinteger(L, countersArr[ii].l[1]);
    ii++;

    lua_pushinteger(L, countersArr[ii].l[0]);
    lua_pushinteger(L, countersArr[ii].l[1]);
    ii++;

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        lua_pushinteger(L, countersArr[ii].l[0]);
        lua_pushinteger(L, countersArr[ii].l[1]);
        ii++;
        return 9;
    }
    else
    {
        return 7;
    }
}

/*******************************************************************************
* wrlCpssDxChPortPipDropCounterGet
*
* DESCRIPTION:
*    wrapper for:
*    GT_STATUS cpssDxChPortPipDropCounterGet
*    (
*        IN  GT_U8                    devNum,
*        IN  GT_PHYSICAL_PORT_NUM     portNum,
*        OUT GT_U64                  *counterPtr
*    );
*
* APPLICABLE DEVICES:
*        Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* NOT APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* INPUTS:
*       L                   - lua state
*
* OUTPUTS:
*
* RETURNS:
*       3 values:
*       1. rc
*       2. dropCounter.low_32bits
*       3. dropCounter.high_32bits
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChPortPipDropCounterGet
(
    IN lua_State                            *L
)
{
    GT_STATUS                               rc;
    GT_U8                                   devNum;
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_U64                                  dropCounter;

    P_CALLING_CHECKING;

    devNum      = (GT_U8)                          lua_tonumber(L, 1);
    portNum     = (GT_PHYSICAL_PORT_NUM)           lua_tonumber(L, 2);

    rc = cpssDxChPortPipDropCounterGet(devNum, portNum, &dropCounter);

    P_CALLING_API(
        cpssDxChPortPipDropCounterGet,
        PC_NUMBER(devNum,               devNum,   GT_U8,
        PC_NUMBER(portNum,              portNum,  GT_PHYSICAL_PORT_NUM,
        PC_LAST_PARAM)),
        rc);

    lua_pushinteger(L, rc);
    lua_pushinteger(L, dropCounter.l[0]);
    lua_pushinteger(L, dropCounter.l[1]);
    return 3;
}

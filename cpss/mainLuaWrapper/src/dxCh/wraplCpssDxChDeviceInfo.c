/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChDeviceInfo.c
*
* DESCRIPTION:
*       device info wrapper
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 14 $
*******************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/common/config/cpssGenCfg.h>
/*
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
*/
#include <cpss/generic/version/cpssGenStream.h>
/*
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
*/
#include <generic/private/prvWraplGeneral.h>
#include <dxCh/wraplCpssDxChTrunk.h>
#include <dxCh/wraplCpssDxChBrgMcEntry.h>
#include <dxCh/wraplCpssDxChVlan.h>
#include <cpssCommon/wraplCpssDeviceInfo.h>
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#include <dxCh/wraplCpssDxChDeviceInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*******************************************************************************
 * External variables
 ******************************************************************************/


/*******************************************************************************
*   Mirror wrapper errors                                                      *
*******************************************************************************/

GT_CHAR deviceProcessingGetErrorMessage[] = "Error in device processing";


/*******************************************************************************
*   Shared lib static                                                          *
*******************************************************************************/

#ifdef SHARED_MEMORY
    static GT_BOOL shared_lib_mode = 1;
#else
    static GT_BOOL shared_lib_mode = 0;
#endif


/*******************************************************************************
*   No kernel module static                                                    *
*******************************************************************************/

#ifdef LINUX_NOKM
    static GT_BOOL linux_no_kernel_module = 1;
#else
    static GT_BOOL linux_no_kernel_module = 0;
#endif


/*******************************************************************************
*   Is 64bit OS                                                                *
*******************************************************************************/

#ifdef IS_64BIT_OS
    static GT_BOOL is_64bit_os = 1;
#else
    static GT_BOOL is_64bit_os = 0;
#endif


/*******************************************************************************
* prvCpssDxChIsValidInterface
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Checks that CPSS_INTERFACE_INFO_STC interface is valid.
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devNum                - device number
*       interfacePtr          - checked interface
*       P_CALLING_FORMAL_DATA - general debug information (environment variables
*                               etc); could be empty
*
* OUTPUTS:
*       isValidInterfacePtr   - interface validness
*       errorMessagePtrPtr    - error message
*
* RETURNS:
*       GT_TRUE, if exists, otherwise GT_FALSE
*
* COMMENTS:
*
*******************************************************************************/
GT_BOOL prvCpssDxChIsValidInterface
(
    IN  GT_U8                   devNum,
    IN  CPSS_INTERFACE_INFO_STC *interfacePtr,
    OUT GT_BOOL                 *isValidInterfacePtr,
    OUT GT_CHAR_PTR             *errorMessagePtrPtr
    P_CALLING_FORMAL_DATA
)
{
    GT_STATUS                   status          = GT_OK;

    *isValidInterfacePtr    = GT_TRUE;
    *errorMessagePtrPtr     = NULL;

    switch (interfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            status  = prvCpssIsHwDevicePortExist(interfacePtr->devPort.hwDevNum,
                                                 interfacePtr->devPort.portNum,
                                                 isValidInterfacePtr,
                                                 errorMessagePtrPtr);
            break;

        case CPSS_INTERFACE_TRUNK_E:
            status  = prvCpssDxChDoesTrunkExist(devNum, interfacePtr->trunkId,
                                                isValidInterfacePtr,
                                                errorMessagePtrPtr
                                                P_CALLING_ACTUAL_DATA);
            break;

        case CPSS_INTERFACE_VIDX_E:
            status  = prvCpssDxChDoesMulticastGroupIndexExist(
                          devNum, interfacePtr->vidx, isValidInterfacePtr,
                          errorMessagePtrPtr);
            break;

        case CPSS_INTERFACE_VID_E:
            status  = prvCpssDxChDoesVlanExist(devNum, interfacePtr->vlanId,
                                               isValidInterfacePtr,
                                               errorMessagePtrPtr
                                               P_CALLING_ACTUAL_DATA);
            break;

        case CPSS_INTERFACE_DEVICE_E:
            *isValidInterfacePtr    =   GT_FALSE;
            break;

        case CPSS_INTERFACE_FABRIC_VIDX_E:
            *isValidInterfacePtr    =   GT_FALSE;
            break;

        case CPSS_INTERFACE_INDEX_E:
            *isValidInterfacePtr    =   GT_FALSE;
            break;

        default:
            *isValidInterfacePtr    =   GT_FALSE;
            break;
    }

    return status;
}


/*******************************************************************************
* wrlMvDxchVersionGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of cpss dxch version name.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*     HWS version
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/

int wrlMvDxchVersionGet
(
    IN lua_State                *L
)
{
    GT_STATUS rc;
    CPSS_VERSION_INFO_STC   versionInfo;

        rc = cpssDxChVersionGet(&versionInfo);
        if (rc != GT_OK)
        {
                return 0; /* nil */
        }
    lua_pushstring(L, (char*)(versionInfo.version));
    lua_pushboolean(L, shared_lib_mode);
    lua_pushboolean(L, linux_no_kernel_module);
    lua_pushboolean(L, is_64bit_os);
    return 4;
}

/*******************************************************************************
* wrlIsVplsModeEnabled
*
* DESCRIPTION:
*       Check VPLS mode enabled
*
* APPLICABLE DEVICES:
*       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       none
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*       true or false pushed to lua stack.
*
* RETURNS:
*       1;
*
* COMMENTS:
*
*******************************************************************************/

int wrlIsVplsModeEnabled
(
    IN lua_State *L
)
{
    GT_STATUS  rc      = GT_OK;
    GT_BOOL    ret;
    GT_U8      devNum;

    PARAM_NUMBER(rc, devNum, 1, GT_U8);

    if (rc != GT_OK)
    {
        /* illegal input parameter */
        return 0; /* return nil */
    }

    ret = WRL_PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.legacyVplsMode.enabled;
    lua_pushboolean(L,ret);
    return 1;
}

/*******************************************************************************
* wrlIsCpuMii
*
* DESCRIPTION:
*       Check if CPU using SGMII or SDMA
*
* APPLICABLE DEVICES:
*       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       none
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*       true or false pushed to lua stack.
*
*       true  - the CPU uses SGMII as packets interface.
*       false - the CPU uses SDMA as packets interface.
* RETURNS:
*       1;
*
* COMMENTS:
*
*******************************************************************************/
int wrlIsCpuMii
(
    IN lua_State *L
)
{
    GT_STATUS  rc      = GT_OK;
    GT_BOOL    useMii;
    GT_U8      devNum;

    PARAM_NUMBER(rc, devNum, 1, GT_U8);

    if (rc != GT_OK)
    {
        /* illegal input parameter */
        return 0; /* return nil */
    }

    useMii =((PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E) ? GT_TRUE:
             (PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_NONE_E) ? GT_FALSE:
             PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum) ? GT_TRUE : GT_FALSE);
    lua_pushboolean(L,useMii);
    return 1;
}

/*******************************************************************************
* wrlIsTmEnabled
*
* DESCRIPTION:
*       Check Traffic Manager is enabled
*
* APPLICABLE DEVICES:
*       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       none
*
* INPUTS:
*       L                     - lua state
*
* OUTPUTS:
*       true or false pushed to lua stack.
*
* RETURNS:
*       1;
*
* COMMENTS:
*
*******************************************************************************/
int wrlIsTmEnabled
(
    IN lua_State *L
)
{
    GT_STATUS  rc      = GT_OK;
    GT_BOOL    ret;
    GT_U8      devNum;

    PARAM_NUMBER(rc, devNum, 1, GT_U8);

    if (rc != GT_OK)
    {
        /* illegal input parameter */
        return 0; /* return nil */
    }

    ret = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmLibIsInitilized;
    lua_pushboolean(L,ret);
    return 1;
}

/*******************************************************************************
* wrlCpssNumOfCpuSdmaPortsGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Get's device's number of CPU SDMA ports
*
* APPLICABLE DEVICES:
*       DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       L                    - lua state (devNum at the Top)
*
* OUTPUTS:
*
* RETURNS:
*       1 if failed. A GT_BAD_PARAM is placed in the Lua stack
*       2 if success , next are placed in the stack:
*           a. GT_STATUS
*           b . number of CPU SDMA ports
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssNumOfCpuSdmaPortsGet
(
    IN lua_State *L
)
{
    GT_STATUS   rc;
    GT_U32      numOfNetIfs;
    GT_U8       devNum = (GT_U8) lua_tonumber(L, 1);

    if (GT_FALSE == prvCpssIsDevExists(devNum))
    {
        lua_pushinteger(L, GT_BAD_PARAM);
        return 1;
    }
    else
    {
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum,&numOfNetIfs);
        /* 1'st param */
        lua_pushinteger(L, rc);
        /* 2'nd param */
        lua_pushinteger(L, numOfNetIfs);

        return 2;
    }
}

/*******************************************************************************
* wrlCpssDxChDeviceMg0BaseAddrGet
*
* DESCRIPTION:
*
*       Getting of base address of the MG0 unit in the device.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                     - lua state
*       devNum                - the device number
* OUTPUTS:
*     GT_STATUS             - GT_OK or error type
*     mg0base               - the MG0 base address
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int wrlCpssDxChDeviceMg0BaseAddrGet
(
    IN lua_State                *L
)
{
    GT_U8       devNum = (GT_U8) lua_tonumber(L, 1);
    GT_U32      mg0base;
    GT_BOOL isError;

    mg0base = 0;
    if (GT_FALSE == prvCpssIsDevExists(devNum))
    {
        /* 1'st param */
        lua_pushinteger(L, GT_NOT_INITIALIZED);
        /* 2'nd param */
        lua_pushinteger(L, mg0base);  /* give base also on 'error' */

        return 2;
    }
    else
    {
        /* 1'st param */
        lua_pushinteger(L, GT_OK);

        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            isError = GT_FALSE;
            mg0base = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E, &isError);
            if(GT_TRUE == isError)
            {
                mg0base = 0;/* give base also on 'error' */
            }
        }
        /* 2'nd param */
        lua_pushinteger(L, mg0base);

        return 2;
    }
}


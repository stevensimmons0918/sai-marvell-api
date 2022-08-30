// xpsMtuProfile.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsMtuProfile.h"
#include "xpsLock.h"
#include "xpsScope.h"
#include "xpsLag.h"
#include "openXpsLag.h"
#include "cpssHalUtil.h"
#include "cpssDxChBrgGen.h"
#include "cpssDxChIpCtrl.h"
#include "cpssDxChIp.h"
#include "xpsGlobalSwitchControl.h"
#include "cpssHalDevice.h"
//#include "cpssGenNetIfTypes.h"


#ifdef __cplusplus
extern "C" {
#endif

static xpsDbHandle_t mtuDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t mtuL3DbHndl = XPS_STATE_INVALID_DB_HANDLE;

extern XP_STATUS xpsInterfaceSetL3MtuProfile(xpsInterfaceId_t intfId,
                                             uint32_t l3MtuProfile);

/* file: xpsMtuProfile.c
* @Brief: MTU is Maximum Transmission Unit for L2 and L3.
*  MTU can be configured according to Prestera Design as the following:
*  1. per ePort for L2 or L3 using maximum of 4 profiles.
*  2. per Next Hop Interface MTU for IPv4/v6 L3 using maximum of 8 profiles (independent of per ePort's profiles).
*
*/




/**
 * \brief Statically defined key compare function for Mtu
 *        database
 *
 * This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the mtu database
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */

static int32_t mtuProfileIdxKeyCompare(void *key1, void *key2)
{
    return ((((xpsMtuMap_t *) key1)->profileIdx) - (((xpsMtuMap_t *)
                                                     key2)->profileIdx));
}

XP_STATUS xpsMtuInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsMtuInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMtuInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    // Create PER_DEVICE Mtu Db
    mtuDbHndl = XPS_MTU_DB_HNDL;
    status = xpsStateRegisterDb(scopeId, "Mtu Db", XPS_PER_DEVICE,
                                &mtuProfileIdxKeyCompare, mtuDbHndl);
    if (status != XP_NO_ERR)
    {
        mtuDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU state DB register failed");
        return status;
    }

    // Create PER_DEVICE Mtu Db
    mtuL3DbHndl = XPS_MTU_L3_DB_HNDL;
    status = xpsStateRegisterDb(scopeId, "Mtu L3 Db", XPS_PER_DEVICE,
                                &mtuProfileIdxKeyCompare, mtuL3DbHndl);
    if (status != XP_NO_ERR)
    {
        mtuDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU L3 state DB register failed");
        return status;
    }
    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsMtuDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    status = xpsMtuDeInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsMtuDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsStateDeRegisterDb(scopeId, &mtuDbHndl);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU state DB deRegister failed");
    }

    status = xpsStateDeRegisterDb(scopeId, &mtuL3DbHndl);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU L3 state DB deRegister failed");
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsMtuAddDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInitType_t initType;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsStateInsertDbForDevice(devId, mtuDbHndl, mtuProfileIdxKeyCompare);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    status = xpsStateInsertDbForDevice(devId, mtuL3DbHndl, mtuProfileIdxKeyCompare);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    initType =  xpsGetInitType();
    if (initType == INIT_COLD)
    {
        status = xpsSetDefaultMtuFrmSize(devId);
    }
    return status;

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsSetDefaultMtuFrmSize(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    int portId = 0;
    XP_STATUS retVal = XP_NO_ERR;
    uint8_t maxTotalPorts;
    xpsMtuMap_t* mtuInfo = NULL;
    GT_STATUS st = GT_OK;
    GT_U8 devNum;
    GT_U32 cpssPortNum = 0;
    GT_U8  cpssDevId = 0;
    xpsPort_t locPortId = 0;
    xpsDevice_t locDevId = 0;

    /* Enable MTU Command and */
    if ((retVal = xpsMtuEnableMtuCheck(devId, TRUE)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error occurs while enabling MTU \n");
        return retVal;
    }

    if ((retVal = xpsGlobalSwitchControlGetMaxPorts(devId,
                                                    &maxTotalPorts)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return retVal;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        st = cpssDxChBrgGenMtuProfileSet(devNum, MTU_PROFILE_INDEX0,
                                         CPSS_DXCH_PORT_MAX_MRU_CNS);
        if (st != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add MTU profile for dev %d\n", devNum);
            return xpsConvertCpssStatusToXPStatus(st);;
        }
    }

    if ((retVal = xpsStateHeapMalloc(sizeof(xpsMtuMap_t),
                                     (void **)&mtuInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for port info failed");
        return retVal;
    }

    if (!mtuInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    mtuInfo->profileIdx = MTU_PROFILE_INDEX0;
    //mtuInfo->mtuSize    = UMAC_MTU_FRAME_SIZE;
    mtuInfo->mtuSize    = CPSS_DXCH_PORT_MAX_MRU_CNS;

    XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
    {
        retVal = xpsPortGetDevAndPortNumFromIntf(portId, &locDevId, &locPortId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port and dev Id from port intf : %d \n", portId);
            xpsStateHeapFree((void *)mtuInfo);
            return retVal;
        }

        cpssDevId = xpsGlobalIdToDevId(locDevId, locPortId);
        cpssPortNum = xpsGlobalPortToPortnum(locDevId, locPortId);

        st = cpssDxChBrgGenMtuPortProfileIdxSet(cpssDevId, cpssPortNum,
                                                MTU_PROFILE_INDEX0);
        if (st != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Port MTU : port %d dev %d\n", cpssPortNum, cpssDevId);
            xpsStateHeapFree((void *)mtuInfo);
            return xpsConvertCpssStatusToXPStatus(st);;
        }

        mtuInfo->refCount += 1;
    }

    if ((retVal = xpsStateInsertDataForDevice(devId, mtuDbHndl,
                                              (void *)mtuInfo)) != XP_NO_ERR)
    {
        (void)xpsStateHeapFree((void *)mtuInfo);
        return retVal;
    }

    mtuInfo = NULL;

    // Create a default L3 MTU profile
    retVal = xpsStateHeapMalloc(sizeof(xpsMtuMap_t), (void **)&mtuInfo);
    if (retVal  != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for interfce mtu info failed");
        return retVal;
    }

    if (!mtuInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(mtuInfo, 0, sizeof(xpsMtuMap_t));
    mtuInfo->profileIdx = MTU_L3_PROFILE_INDEX7;
    mtuInfo->mtuSize    = UMAC_MTU_FRAME_SIZE;

    //TODO: move this call to HAL
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        st = cpssDxChIpMtuProfileSet(devNum, MTU_L3_PROFILE_INDEX7,
                                     UMAC_MTU_FRAME_SIZE);
        if ((retVal = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "default MTU set for Ip interface failed\n", devNum);
            xpsStateHeapFree((void *)mtuInfo);
            return retVal;
        }
    }

    //add mtuInfo state
    if ((retVal = xpsStateInsertDataForDevice(devId, mtuL3DbHndl,
                                              (void *)mtuInfo)) != XP_NO_ERR)
    {
        // Free Allocated memory
        xpsStateHeapFree((void *)mtuInfo);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to insert MTU info with %d\n", retVal);
        return retVal;
    }

    mtuInfo = NULL;

    //set jumbo frame size
    if ((retVal = xpsStateHeapMalloc(sizeof(xpsMtuMap_t),
                                     (void **)&mtuInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Memory allocation for interfce mtu info failed");
        return retVal;
    }

    if (!mtuInfo)
    {
        retVal = XP_ERR_NULL_POINTER;
        return retVal;
    }

    //TODO: check if it is required to add jumbo profile during init time
    mtuInfo->profileIdx = MTU_L3_PROFILE_INDEX0;
    mtuInfo->mtuSize = CPSS_DXCH_PORT_MAX_MRU_CNS;

    //TODO: move this call to HAL
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        st = cpssDxChIpMtuProfileSet(devNum, MTU_L3_PROFILE_INDEX0,
                                     CPSS_DXCH_PORT_MAX_MRU_CNS);
        if ((retVal = xpsConvertCpssStatusToXPStatus(st))!=XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "default MTU set for Ip interface failed\n", devNum);
            xpsStateHeapFree((void *)mtuInfo);
            return retVal;
        }
    }

    //add mtuInfo state
    if ((retVal = xpsStateInsertDataForDevice(devId, mtuL3DbHndl,
                                              (void *)mtuInfo)) != XP_NO_ERR)
    {
        // Free Allocated memory
        xpsStateHeapFree((void *)mtuInfo);
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to insert MTU info with %d\n", retVal);
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMtuRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsStateDeleteDbForDevice(devId, mtuDbHndl);


    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsMtuUpdateInterfaceMtuSize(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId, uint32_t mtuSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS  cpssStatus = GT_OK;
    xpsMtuMap_t info;
    xpsMtuMap_t* mtuInfo = NULL;
    uint32_t mtuProfileIdx = 0;
    xpsInterfaceType_e intfType;
    xpsScope_t scopeId;
    GT_U32 cpssPortNum;

    //get scope from Device ID
    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    if ((status = xpsInterfaceGetTypeScope(scopeId, intfId,
                                           &intfType)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfId, status);
        return status;
    }

    if (intfType == XPS_BRIDGE_PORT)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU configuration is not supported on bridge-SubPort interface(%d)", intfId);
        return XP_ERR_INVALID_INPUT;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);

    if (intfType == XPS_LAG)
    {
        cpssPortNum = (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(intfId, intfType);
    }

    cpssStatus = cpssDxChBrgGenMtuPortProfileIdxGet(devId, cpssPortNum,
                                                    &mtuProfileIdx);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Retrieving MTU Profile information Faild on interface %d", intfId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    if (mtuProfileIdx < MTU_PROFILE_INDEX_MAX_VAL)
    {
        info.profileIdx = mtuProfileIdx;

        if ((status = xpsStateSearchDataForDevice(devId, mtuDbHndl, (xpsDbKey_t) &info,
                                                  (void **)&mtuInfo)) != XP_NO_ERR)
        {
            return status;
        }
        if (mtuInfo != NULL)
        {
            mtuInfo->mtuSize = mtuSize;
            cpssStatus = cpssDxChBrgGenMtuProfileSet(devId, mtuProfileIdx, mtuSize);
            status = xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        else
        {
            status = XP_ERR_INVALID_DATA;
        }
    }


    XPS_FUNC_EXIT_LOG();
    return status;
}

/**
* @function xpMtuDecrementL3MtuProfileRefCount
*
* @brief   Decrements the refCount for a given l3 mtu profile and deletes
*          the same if refCount becomes 0
*
* @param[in] devId       - device Id
* @param[in] mtuProfile  - MTU profile id
*/
static XP_STATUS xpMtuDecrementL3MtuProfileRefCount(xpsDevice_t devId,
                                                    uint32_t mtuProfile)
{
    uint8_t     devNum     = 0;
    XP_STATUS    status     = XP_NO_ERR;
    GT_STATUS    cpssStatus = GT_OK;
    xpsMtuMap_t *pMtuInfo   = NULL;
    xpsMtuMap_t  mtuInfoKey;

    // Populate key
    memset(&mtuInfoKey, 0, sizeof(xpsMtuMap_t));
    mtuInfoKey.profileIdx = mtuProfile;

    status = xpsStateSearchDataForDevice(devId, mtuL3DbHndl,
                                         (xpsDbKey_t)&mtuInfoKey, (void **)&pMtuInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to retrieve mtu profile %d state data, error: %d\n", mtuProfile,
              status);
        return status;
    }

    if (pMtuInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU profile %d not found in state data, error: %d\n", mtuProfile, status);
        return XP_ERR_KEY_NOT_FOUND;
    }

    pMtuInfo->refCount = (pMtuInfo->refCount - 1);

    // Delete the mtu profile if refCount is 0
    if ((pMtuInfo->refCount == 0) && (mtuProfile != MTU_L3_PROFILE_INDEX0))
    {
        XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
        {
            cpssStatus = cpssDxChIpMtuProfileSet(devNum, mtuProfile, 1500);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to add IP MTU profile for dev %d\n", devId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }

        status = xpsStateDeleteDataForDevice(devId, mtuL3DbHndl,
                                             (xpsDbKey_t)&mtuInfoKey, (void **)&pMtuInfo);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to delete mtu profile %d state data, error: %d\n", mtuProfile, status);
            return status;
        }

        // Free the memory
        status = xpsStateHeapFree(pMtuInfo);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to free the mtu profile %d heap memory, error: %d\n", mtuProfile,
                  status);
            return status;
        }
    }

    return XP_NO_ERR;
}

/**
* @Function  xpsMtuSetDefaultL3InterfaceMtuSize
*
* @brief     Sets the default L3 Mtu Size to L3Interface.
*
* @param[in] devId     device Id
* @param[in] intfL3Id  L3 Interface id
* @param[in] mtuSize   MTU Size
*/
XP_STATUS xpsMtuSetDefaultL3InterfaceMtuSize(xpsDevice_t devId,
                                             xpsInterfaceId_t intfL3Id)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS    status     = XP_NO_ERR;
    xpsMtuMap_t *pMtuInfo   = NULL;
    xpsMtuMap_t  mtuInfoKey;

    // Populate key
    memset(&mtuInfoKey, 0, sizeof(xpsMtuMap_t));
    mtuInfoKey.profileIdx = MTU_L3_PROFILE_INDEX0;

    status = xpsStateSearchDataForDevice(devId, mtuL3DbHndl,
                                         (xpsDbKey_t)&mtuInfoKey, (void **)&pMtuInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to retrieve mtu profile %d state data, error: %d\n",
              mtuInfoKey.profileIdx, status);
        return status;
    }

    if (pMtuInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU profile %d not found in state data, error: %d\n", mtuInfoKey.profileIdx,
              status);
        return XP_ERR_KEY_NOT_FOUND;
    }

    //Assign the newly added profile to L3Interface Info
    status = xpsInterfaceSetL3MtuProfile(intfL3Id, mtuInfoKey.profileIdx);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set L3 mtu profile %d on interface: %d, error: %d",
              mtuInfoKey.profileIdx, intfL3Id, status);
        return status;
    }

    pMtuInfo->refCount += 1;

    XPS_FUNC_EXIT_LOG();

    return status;
}

/**
* @  xpsMtuSetIL3nterfaceMtuSize function
*
* @brief   Set L3 Mtu Size to L3Interface and creates a profile
*          if does not exist.
*
* @param[in] devId          - device Id
*
* @param[in] xpsInterfaceId_t      - L3 Interface which will be
*       converted to Port Num or Lag based on type.
* @param[in] mtuSize              - MTU Size
*
*/
XP_STATUS xpsMtuSetIL3nterfaceMtuSize(xpsDevice_t devId,
                                      xpsInterfaceId_t intfL3Id, uint32_t mtuSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS  cpssStatus = GT_OK;
    xpsInterfaceType_e l3IntfType;
    xpsMtuMap_t info;
    xpsMtuMap_t* mtuInfo = NULL;
    uint8_t availableProfileIndex = MTU_L3_PROFILE_INDEX_MAX_VAL;
    GT_U8 cpssDevId;
    GT_BOOL foundMtuProfile = GT_FALSE;
    xpsInterfaceInfo_t *intfInfo;

    if ((status = xpsInterfaceGetInfo(intfL3Id, &intfInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfL3Id, status);
        return status;
    }
    if (intfInfo == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    /* Validate the type of the egress interface */
    l3IntfType = intfInfo->type;

    if ((l3IntfType != XPS_PORT_ROUTER) &&
        (l3IntfType != XPS_SUBINTERFACE_ROUTER) && (l3IntfType != XPS_VLAN_ROUTER) &&
        (l3IntfType != XPS_TUNNEL_ROUTER))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU configuration is not supported on interface(%d)", intfL3Id);
        return XP_ERR_INVALID_PARAMS;
    }

    for (uint8_t i = 0; i < MTU_L3_PROFILE_INDEX_MAX_VAL; i ++)
    {
        info.profileIdx = i;
        if ((status = xpsStateSearchDataForDevice(devId, mtuL3DbHndl,
                                                  (xpsDbKey_t) &info, (void **)&mtuInfo)) != XP_NO_ERR)
        {
            return status;
        }

        if (mtuInfo != NULL)
        {
            if (mtuInfo->mtuSize == mtuSize)
            {
                foundMtuProfile = GT_TRUE;
                availableProfileIndex = i;
                break;
            }
        }
        else
        {
            //Use first available Profile Index
            if (availableProfileIndex == MTU_L3_PROFILE_INDEX_MAX_VAL)
            {
                availableProfileIndex = i;
            }
        }
    }

    uint32_t currentMtuProfile = MTU_L3_PROFILE_INDEX_MAX_VAL;

    status = xpsInterfaceGetIpMtuProfileIndex(intfL3Id, &currentMtuProfile);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get the interface mtu profile, interface: %d, error: %d", intfL3Id,
              status);
        return status;
    }

    if (currentMtuProfile == availableProfileIndex)
    {
        // In case of reassigning the same mtu, don't increment the refCount, just return success
        return XP_NO_ERR;
    }

    //Add MTU Info into MTU State and create MTU Profile if there is available profile
    if (availableProfileIndex != MTU_L3_PROFILE_INDEX_MAX_VAL)
    {
        if (foundMtuProfile != GT_TRUE)
        {
            XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
            {
                cpssStatus = cpssDxChIpMtuProfileSet(cpssDevId, availableProfileIndex, mtuSize);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to add IP MTU profile for dev %d\n", devId);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }

            if ((status = xpsStateHeapMalloc(sizeof(xpsMtuMap_t),
                                             (void **)&mtuInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Memory allocation for port info failed");
                return status;
            }
            if (!mtuInfo)
            {
                status = XP_ERR_NULL_POINTER;
                return status;
            }

            memset(mtuInfo, 0, sizeof(xpsMtuMap_t));
            mtuInfo->profileIdx = availableProfileIndex;
            mtuInfo->mtuSize = mtuSize;

            //add mtuInfo state
            if ((status = xpsStateInsertDataForDevice(devId, mtuL3DbHndl,
                                                      (void *)mtuInfo)) != XP_NO_ERR)
            {
                // Free Allocated memory
                xpsStateHeapFree((void *)mtuInfo);
                return status;
            }
        }

        //Assign the newly added profile to L3Interface Info
        status = xpsInterfaceSetL3MtuProfile(intfL3Id, availableProfileIndex);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set L3 mtu profile %d on interface: %d, error: %d",
                  mtuInfo->profileIdx, intfL3Id, status);
            return status;
        }

        // mtuInfo points to a new mtu info db or existing mtu info db.
        // In both the cases refCount needs to be incremented.
        mtuInfo->refCount += 1;

        // Decrement the refCount for current mtu profile after successfully binding a new mtu profile to port
        status = xpMtuDecrementL3MtuProfileRefCount(devId, currentMtuProfile);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to decrement the mtu profile %d refCount, devId: %d, status: %d\n",
                  currentMtuProfile, devId, status);
            return status;
        }
    }
    else
    {
        status = XP_ERR_RESOURCE_NOT_AVAILABLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set IP MTU for interface %d on device  %d: NO available MTU Profile \n",
              intfL3Id, devId);

    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

/**
* @function xpMtuDecrementMtuProfileRefCount
*
* @brief   Decrements the refCount for a given l2 mtu profile and deletes
*          the same if refCount becomes 0
*
* @param[in] devId       - device Id
* @param[in] mtuProfile  - MTU profile id
*/
static XP_STATUS xpMtuDecrementMtuProfileRefCount(xpsDevice_t devId,
                                                  uint32_t mtuProfile)
{
    uint8_t     devNum     = 0;
    XP_STATUS    status     = XP_NO_ERR;
    GT_STATUS    cpssStatus = GT_OK;
    xpsMtuMap_t *pMtuInfo   = NULL;
    xpsMtuMap_t  mtuInfoKey;

    // Populate key
    memset(&mtuInfoKey, 0, sizeof(xpsMtuMap_t));
    mtuInfoKey.profileIdx = mtuProfile;

    status = xpsStateSearchDataForDevice(devId, mtuDbHndl, (xpsDbKey_t)&mtuInfoKey,
                                         (void **)&pMtuInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to retrieve mtu profile %d state data, error: %d\n", mtuProfile,
              status);
        return status;
    }

    if (pMtuInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU profile %d not found in state data, error: %d\n", mtuProfile, status);
        return XP_ERR_KEY_NOT_FOUND;
    }

    (pMtuInfo->refCount)--;

    // Delete the mtu profile if refCount is 0
    if (pMtuInfo->refCount == 0)
    {
        XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
        {
            cpssStatus = cpssDxChBrgGenMtuProfileSet(devNum, mtuProfile, 0);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set mtu for given mtu profile %d, device: %d, status: %d\n",
                      mtuProfile, devNum, cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }

        status = xpsStateDeleteDataForDevice(devId, mtuDbHndl, (xpsDbKey_t)&mtuInfoKey,
                                             (void **)&pMtuInfo);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to delete mtu profile %d state data, error: %d\n", mtuProfile, status);
            return status;
        }

        // Free the memory
        status = xpsStateHeapFree(pMtuInfo);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to free the mtu profile %d heap memory, error: %d\n", mtuProfile,
                  status);
            return status;
        }
    }

    return XP_NO_ERR;
}


/*
 *  Set MTU for physcial port which is used for L2-based MTU
 *
 */
XP_STATUS xpsMtuSetInterfaceMtuSize(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t mtuSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS  cpssStatus = GT_OK;
    xpsMtuMap_t info;
    xpsMtuMap_t* mtuInfo = NULL;
    uint8_t availableProfileIndex = MTU_PROFILE_INDEX_MAX_VAL;
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t lagPortList;
    GT_U32 cpssPortNum;
    GT_U8  cpssDevId;
    xpsInterfaceId_t portIntf;
    int devNum;
    xpsPort_t portId ;
    xpsDevice_t  portDevId;
    GT_BOOL foundMtuProfile = GT_FALSE;
    memset(&lagPortList, 0, sizeof(lagPortList));

    /* Get the type of the egress interface */
    status = xpsInterfaceGetType(intfId, &intfType);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Port MTU on interface %d due to failure in retrieving interface type\n",
              intfId);
        return status;
    }

    if (intfType == XPS_BRIDGE_PORT)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU Size is not supported on bridge-SubPort interface(%d)", intfId);
        return XP_ERR_INVALID_INPUT;
    }

    if (intfType == XPS_PORT)
    {
        lagPortList.portIntf[0] = intfId;
        lagPortList.size = 1;
    }

    else if (intfType == XPS_LAG)
    {
        if ((status = xpsLagGetPortIntfList(intfId, &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", intfId);
            return status;
        }

    }

    for (uint8_t i = 0; i < MTU_PROFILE_INDEX_MAX_VAL; i++)
    {
        info.profileIdx = i;
        if ((status = xpsStateSearchDataForDevice(devId, mtuDbHndl, (xpsDbKey_t) &info,
                                                  (void **)&mtuInfo)) != XP_NO_ERR)
        {
            return status;
        }

        if (mtuInfo != NULL)
        {
            if (mtuInfo->mtuSize == mtuSize)
            {
                foundMtuProfile = GT_TRUE;
                availableProfileIndex = i;
                break;
            }
        }
        else
        {
            //Use first available Profile Index
            if (availableProfileIndex == MTU_PROFILE_INDEX_MAX_VAL)
            {
                availableProfileIndex = i;
            }
        }
    }

    uint32_t currentMtuProfile = MTU_PROFILE_INDEX_MAX_VAL;

    status = xpsMtuGetInterfaceMtuProfileIdx(devId, intfId, &currentMtuProfile);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get the interface mtu profile, interface: %d, error: %d", intfId,
              status);
        return status;
    }

    if (currentMtuProfile == availableProfileIndex)
    {
        // In case of reassigning the same mtu, don't increment the refCount, just return success
        return XP_NO_ERR;
    }

    if (availableProfileIndex != MTU_PROFILE_INDEX_MAX_VAL)
    {
        if (foundMtuProfile != GT_TRUE)
        {
            // No profile found for a given MTU, so create a new profile
            XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
            {
                cpssStatus = cpssDxChBrgGenMtuProfileSet(devNum, availableProfileIndex,
                                                         mtuSize);
                if (cpssStatus != GT_OK)
                {
                    status =  xpsConvertCpssStatusToXPStatus(cpssStatus);
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to add MTU profile for dev %d\n", devNum);
                    return status;
                }

            }

            if ((status = xpsStateHeapMalloc(sizeof(xpsMtuMap_t),
                                             (void **)&mtuInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Memory allocation for port info failed");
                return status;
            }

            if (!mtuInfo)
            {
                status = XP_ERR_NULL_POINTER;
                return status;
            }

            memset(mtuInfo, 0, sizeof(xpsMtuMap_t));
            mtuInfo->profileIdx = availableProfileIndex;
            mtuInfo->mtuSize = mtuSize;

            // Insert the portNum,deviceId info structure into the database, using the portNum as a key
            if ((status = xpsStateInsertDataForDevice(devId, mtuDbHndl,
                                                      (void *)mtuInfo)) != XP_NO_ERR)
            {
                // Free Allocated memory
                xpsStateHeapFree((void *)mtuInfo);
                return status;
            }
        }

        // mtuInfo points to a new mtu info db or existing mtu info db.
        // In both the cases refCount needs to be incremented.
        mtuInfo->refCount += 1;

        for (uint32_t j = 0; j < lagPortList.size; j++)
        {
            portIntf = lagPortList.portIntf[j];
            status = xpsPortGetDevAndPortNumFromIntf(portIntf, &portDevId, &portId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get port and dev Id from port intf : %d \n", portIntf);
                return status;
            }

            cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

            cpssStatus = cpssDxChBrgGenMtuPortProfileIdxSet(cpssDevId, cpssPortNum,
                                                            availableProfileIndex);
            if (cpssStatus != GT_OK)
            {
                status =  xpsConvertCpssStatusToXPStatus(cpssStatus);
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set Port MTU : port %d dev %d\n", cpssPortNum, cpssDevId);
                return status;
            }
        }

        // Decrement the refCount for current mtu profile after successfully binding a new mtu profile to port
        status = xpMtuDecrementMtuProfileRefCount(devId, currentMtuProfile);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to decrement the mtu profile %d refCount, devId: %d, status: %d\n",
                  currentMtuProfile, devId, status);
            return status;
        }
    }
    else
    {
        status = XP_ERR_RESOURCE_NOT_AVAILABLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Port MTU for interface %d on device %d as NO available MTU Profile\n",
              intfId, devId);
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsMtuGetInterfaceMtuSize(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t* mtuSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    GT_STATUS  cpssStatus = GT_OK;
    uint32_t mtuProfileIdx = 0;
    xpsMtuMap_t info;
    xpsMtuMap_t* mtuInfo = NULL;
    xpsInterfaceType_e intfType;
    GT_U32 cpssPortNum;
    GT_U8  cpssDevId;


    /* Get the type of the egress interface */
    status = xpsInterfaceGetType(intfId, &intfType);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Port MTU on interface %d due to failure in retrieving interface type\n",
              intfId);
        return status;
    }

    if (intfType == XPS_BRIDGE_PORT)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU Size is not supported on bridge-SubPort interface(%d)", intfId);
        return XP_ERR_INVALID_INPUT;
    }

    cpssDevId = xpsGlobalIdToDevId(devId, intfId);

    cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);

    if (intfType == XPS_LAG)
    {
        cpssPortNum = (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(intfId, intfType);
    }

    cpssStatus = cpssDxChBrgGenMtuPortProfileIdxGet(cpssDevId, cpssPortNum,
                                                    &mtuProfileIdx);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU port profileId Get failed for cpssPort:%d \n", cpssPortNum);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    info.profileIdx = mtuProfileIdx;

    if ((status = xpsStateSearchDataForDevice(devId, mtuDbHndl, (xpsDbKey_t) &info,
                                              (void **)&mtuInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Data search failed for profileIdx:%d \n", mtuProfileIdx);
        return status;
    }

    if (mtuInfo != NULL)
    {
        *mtuSize = mtuInfo->mtuSize;
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface mtu size is not configured");
        status = XP_ERR_INVALID_ARG;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsMtuGetMtuProfile(xpsDevice_t devId, uint32_t profileIdx,
                              uint32_t* mtuSize)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS  cpssStatus = GT_OK;

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    //We may also get it from SW mtuDb
    cpssStatus = cpssDxChBrgGenMtuProfileGet(devId, profileIdx, mtuSize);

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

XP_STATUS xpsMtuGetInterfaceMtuProfileIdx(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, uint32_t* mtuProfileIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceType_e intfType;
    GT_STATUS  cpssStatus = GT_OK;
    GT_U32 cpssPortNum;
    GT_U8 cpssDevId;


    /* Get the type of the egress interface */
    status = xpsInterfaceGetType(intfId, &intfType);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Port MTU on interface %d due to failure in retrieving interface type\n",
              intfId);
        return status;
    }

    if (intfType == XPS_BRIDGE_PORT)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "MTU configuration is not supported on bridge-SubPort interface(%d)", intfId);
        return XP_ERR_INVALID_INPUT;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, intfId);

    cpssDevId = xpsGlobalIdToDevId(devId, intfId);

    if (intfType == XPS_LAG)
    {
        cpssPortNum = (GT_TRUNK_ID)xpsUtilXpstoCpssInterfaceConvert(intfId, intfType);
    }

    cpssStatus = cpssDxChBrgGenMtuPortProfileIdxGet(cpssDevId, cpssPortNum,
                                                    mtuProfileIdx);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read MTU profile index from ASIC on port %d\n", cpssPortNum);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

/*
 *  Enable MTU and set command to hardDrop for L2
 */
XP_STATUS xpsMtuEnableMtuCheck(xpsDevice_t devId, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT  byteCountMode;
    CPSS_PACKET_CMD_ENT  exceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT exceptionCpuCode;
    GT_U8  cpssDevNum;
    GT_STATUS  cpssStatus = GT_OK;


    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    /* L2 bytecount is used for MTU check. */
    byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
    exceptionCpuCode = CPSS_NET_CONTROL_E;
    exceptionCommand = CPSS_PACKET_CMD_DROP_HARD_E;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* exceptionCommand by default is assigned to Forwarding. Second time MTU is enabled/disabled, exceptionCommand is restored.*/
        cpssStatus = cpssDxChBrgGenMtuConfigSet(cpssDevNum, (GT_BOOL) enable,
                                                byteCountMode, exceptionCommand,
                                                exceptionCpuCode);

        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Set MTU error Packet Command Failded on device %d \n", cpssDevNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

    }

    XPS_FUNC_EXIT_LOG();
    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

XP_STATUS xpsMtuIsMtuCheckEnabled(xpsDevice_t devId, uint32_t *enable)
{
    XPS_FUNC_ENTRY_LOG();

    GT_BOOL mtuCheckEnable;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT  byteCountMode;
    CPSS_PACKET_CMD_ENT  exceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT exceptionCpuCode;
    GT_STATUS  cpssStatus = GT_OK;

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    cpssStatus = cpssDxChBrgGenMtuConfigGet(devId, &mtuCheckEnable,
                                            &byteCountMode, &exceptionCommand,
                                            &exceptionCpuCode);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Read MTU Configuration Failed on device %d \n", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    *enable = (uint32_t) mtuCheckEnable;


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMtuSetMtuErrorPktCmd(xpsDevice_t devId, uint32_t pktCmd)
{
    XPS_FUNC_ENTRY_LOG();
    GT_BOOL mtuCheckEnable = GT_TRUE;
    int  cpssDevNum;
    CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT  byteCountMode;
    CPSS_PACKET_CMD_ENT  exceptionCommand;
    CPSS_NET_RX_CPU_CODE_ENT exceptionCpuCode;
    GT_STATUS  cpssStatus = GT_OK;
    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    if (!((pktCmd == XP_PKTCMD_TRAP) || (pktCmd == XP_PKTCMD_DROP)))
    {
        LOGFN(xpLogModeXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "pktCmd can only be set as DROP/TRAP");
        return XP_ERR_INVALID_ARG;
    }

    /* L2 bytecount is used for MTU check. */
    byteCountMode  = CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_L2_E;
    exceptionCpuCode = CPSS_NET_CONTROL_E;
    exceptionCommand = xpsConvertPktCmdToCpssPktCmd((xpsPktCmd_e) pktCmd);

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        cpssStatus = cpssDxChBrgGenMtuConfigSet(cpssDevNum, mtuCheckEnable,
                                                byteCountMode, exceptionCommand,
                                                exceptionCpuCode);

        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Set MTU error Packet Command Failed on device %d \n", cpssDevNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return xpsConvertCpssStatusToXPStatus(cpssStatus);
}

/**
* @function xpsMtuGetIpMtuProfile
*
* @brief   This is a debug API used to fetch the mtu size for a given L3 mtu profile
*
* @param[in] devId       - device Id
* @param[in] mtuProfile  - MTU profile id
* @param[out] mtuSize    - MTU size
*/
XP_STATUS xpsMtuGetIpMtuProfile(xpsDevice_t devId, uint32_t mtuProfile,
                                uint32_t *mtuSize)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpssStatus = GT_OK;

    /* Check if the device is valid */
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    cpssStatus = cpssDxChIpMtuProfileGet(devId, mtuProfile, mtuSize);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get the mtu size for profile %d, status: %d \n", mtuProfile,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
* @function xpsMtuGetIpMtuProfile
*
* @brief   This is a debug API used to fetch the nhid, egress port and mtu profile information
*
* @param[in] devId       - Device Id
* @param[in] egrPortNum  - Egress port number
*/
XP_STATUS xpsMtuDisplayNhTable(xpsDevice_t devId, uint32_t egrPortNum)
{
    GT_STATUS cpssStatus   = GT_OK;
    uint32_t  numNhEntries = 0;
    uint32_t  mtuSize      = 0;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *pNhEntries = NULL;

    cpssStatus = cpssDxChCfgTableNumEntriesGet(devId,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &numNhEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Router Next Hop count \n");
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    pNhEntries = (CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC*) malloc(numNhEntries * sizeof(
                                                               CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC));
    if (pNhEntries == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate heap memory for NH Entries\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    memset(pNhEntries, 0, sizeof(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC) * numNhEntries);
    cpssStatus = cpssDxChIpUcRouteEntriesRead(devId, 0, pNhEntries, numNhEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Router Next Hop count \n");
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    printf("------------------------------------------------------------------\n");
    printf("-------------------- Route Next Hop Table ------------------------\n");
    printf("------------------------------------------------------------------\n");
    printf("Total entries: %d\n\n", numNhEntries);
    printf("%-4s %-10s %-14s %-8s\n", "Idx", "Port Num", "Mtu Profile", "Mtu Size");
    printf("%-4s %-10s %-14s %-8s\n", "---", "--------", "-----------", "--------");
    for (uint32_t i = 0; i < numNhEntries; i++)
    {
        cpssStatus = cpssDxChIpMtuProfileGet(devId,
                                             pNhEntries[i].entry.regularEntry.mtuProfileIndex, &mtuSize);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get the Ip Mtu profile, status: %d \n", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        if ((egrPortNum == 255) &&
            (pNhEntries[i].entry.regularEntry.nextHopInterface.devPort.portNum !=
             0)) // Display all entries except egress port 0
        {
            printf("%-4d %-10d %-14d %-8d\n", i,
                   pNhEntries[i].entry.regularEntry.nextHopInterface.devPort.portNum,
                   pNhEntries[i].entry.regularEntry.mtuProfileIndex, mtuSize);
        }
        else
        {
            if (pNhEntries[i].entry.regularEntry.nextHopInterface.devPort.portNum ==
                egrPortNum)
            {
                printf("%-4d %-10d %-14d %-8d\n", i, egrPortNum,
                       pNhEntries[i].entry.regularEntry.mtuProfileIndex, mtuSize);
            }
        }
    }
    printf("------------------------------------------------------------------\n");

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif


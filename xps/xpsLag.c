// xpsLag.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsLag.h"
#include "xpsInterface.h"
#include "xpsPort.h"
#include "xpsInternal.h"
#include "xpsInit.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsGlobalSwitchControl.h"
#include "xpTypes.h"
#include "xpsStp.h"
#include "xpsMulticast.h"
#include "xpsMirror.h"
#include "xpsAllocator.h"

#include "cpssHalUtil.h"
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>

#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include "cpssHalLag.h"
#include "cpssHalMulticast.h"
#include "cpssHalAcl.h"

#ifdef __cplusplus
extern "C" {
#endif


/*TODO 4k lag is allowed as per aldrian spec. But lag creation fails at 64. Need to debug further.
 XP interface state needs unique value for lag(4k onward range),port(<4k). But cpss needs id from 1-4096.
 To handle both we have another allocator for cpss - XPS_ALLOCATOR_TRUNKID_TBL . LagVif is used as
 an id only in XPS layer(defined as 1k). We need to revisit the xpsInterface allocation module to handle id collision. */
#define XP_TRUNKID_MAX_IDS 4095
#define XP_TRUNKID_RANGE_START 1

// TODO 4096 ports are supported per lag from aldrain spec. This includes eports etc
// Now we are considering only front panel ports of B2B connection(26(24-1g+2-10g) front panel per device).
// As of now XPS_LAG_MAX_PORTS is defined as 8 in openXPS( opensourced header file, so comments are recorded here).

/**
 * \file
 * \brief This file contains APIs that allow for creating,
 *        deleting and managing Link Aggregation Groups (LAGs)
 *
 * This file contains APIs to manage LAGs in an XPA based
 * system. This manager will maintain a relationship between LAG
 * interfaces and Port Interfaces and use this to configure a
 * LAG into a device
 */

/*
 * Local Enum and Structure defintions
 */


/**
 * This state contains the relationship between a port interface
 * (which is part of lag) and the lag interface.
 *
 * This state is internal to XPS and is not exposed to the user
 */
typedef struct xpsIntfLagMap_t
{
    xpsInterfaceId_t      portIntf;
    xpsInterfaceId_t      lagIntf;
} xpsIntfLagMap_t;

/**
 * \brief Lag Manager State Database Handle
 *
 */



xpsDbHandle_t lagDbHandle = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t intfToLagDbHandle = XPS_STATE_INVALID_DB_HANDLE;

static XP_STATUS xpsLagEnableHashFieldsInArray(xpHashField* fields, size_t size,
                                               bool* lagHashFieldArray);

static XP_STATUS xpsLagResetHashFields(xpsDevice_t deviceId);

extern int32_t xpSaiSwitchHashAlgorithmGet(int32_t* hashAlgotiryhm, bool isLag);

/**
 * \brief Lag State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */

static int32_t lagKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpLagDbEntry_t*)key1)->lagIntf) - (int32_t)(((
                                                                         xpLagDbEntry_t*)key2)->lagIntf));
}


/**
 * \brief Interface State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] key1
 * \param [in] key2
 *
 * \return int32_t
 */

static int32_t intfKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpsIntfLagMap_t*)key1)->portIntf) - (int32_t)(((
                                                                           xpsIntfLagMap_t*)key2)->portIntf));
}


/*
 * Static Function Prototypes
 */



static XP_STATUS xpsLagInfoInit(xpLagDbEntry_t *info);

static bool randomEnable = false;
bool xpsLagRandomEnableGet(void)
{
    return randomEnable;
}

void xpsLagRandomEnableSet(bool enable)
{
    randomEnable = enable;
}
/*
 * API Implementation
 */

XP_STATUS xpsLagInit(void)
{

    return xpsLagInitScope(XP_SCOPE_DEFAULT);

}

XP_STATUS xpsLagInitScope(xpsScope_t scopeId)
{

    XP_STATUS status = XP_NO_ERR;

    lagDbHandle = XPS_LAG_DB_HNDL;
    // Create Global Lag Db
    if ((status = xpsStateRegisterDb(scopeId, "Lag Db", XPS_GLOBAL, &lagKeyCompare,
                                     lagDbHandle)) != XP_NO_ERR)
    {
        lagDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        return status;
    }

    // Create a Global Port Interface to Lag Db
    intfToLagDbHandle = XPS_INTF_TO_LAG_DB_HNDL;
    if ((status = xpsStateRegisterDb(scopeId, "Intf to Lag Db", XPS_GLOBAL,
                                     &intfKeyCompare, intfToLagDbHandle)) != XP_NO_ERR)
    {
        intfToLagDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        return status;
    }

    return status;
}

XP_STATUS xpsLagDeInit(void)
{
    return xpsLagDeInitScope(XP_SCOPE_DEFAULT);

}

XP_STATUS xpsLagDeInitScope(xpsScope_t scopeId)
{

    XP_STATUS status = XP_NO_ERR;


    // Purge Global Lag Db
    if ((status = xpsStateDeRegisterDb(scopeId, &lagDbHandle)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "De-Register Lag Db  handler failed.\n");
        return status;
    }

    if ((status = xpsStateDeRegisterDb(scopeId, &intfToLagDbHandle)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "De-Register Intf to Lag Db handler failed.\n");
        return status;
    }

    return status;
}

XP_STATUS xpsLagAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    /*Set default HASH Params for LAG*/
    status = xpsLagSetHashDefaults(devId);

    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set default Hash params for Lag on Device (%d)", devId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsLagRemoveDevice(xpsDevice_t devId)
{

    XP_STATUS status = XP_NO_ERR;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Disabling Lag Successful on  Device Id = %d!!!\n", devId);

    return status;
}

/*API to set LAG HASH Defaults*/
XP_STATUS xpsLagSetHashDefaults(xpsDevice_t devId)
{
    GT_STATUS cpssStatus = GT_OK;
    GT_U32 hashSeed =0;

    /*Set default trunk HASH mode to CRC*/
    cpssStatus = cpssHalSetTrunkHashGlobalMode(devId,
                                               CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to set Trunk Hash Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    /*set CRC mode type and default seed=0*/
    cpssStatus = cpssHalSetTrunkHashCrcParameters(devId,
                                                  CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E, hashSeed);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to set Trunk Hash CRC parameters on DevId (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    /*set Default fields to be enabled for LAG hash selction as SMAC and DMAC*/
    cpssStatus = cpssHalSetTrunkHashIpAddMacMode(devId, GT_TRUE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to set Trunk Hash CRC parameters on DevId (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}

/*API to set LAG HASH Seed*/
XP_STATUS xpsLagSetHashSeed(xpsDevice_t devId, uint32_t hashSeed)
{
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    xpsLagHashAlgoType_e xpsHashAlgo;

    status = xpsLagGetHashAlgo(devId, &xpsHashAlgo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to get Trunk Hash Mode on device (%d)", devId);
        return status;
    }

    if (xpsHashAlgo != XPS_LAG_HASH_ALGO_CRC_E)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't set hash seed as Trunk Hash Mode on device (%d) is not CRC", devId);
        return XP_ERR_FAILED;
    }

    cpssStatus = cpssHalSetTrunkHashCrcParameters(devId,
                                                  CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E, hashSeed);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Trunk Hash CRC parameters on DevId (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}

/*API to set LAG HASH Seed*/
XP_STATUS xpsLagSetRandomHashSeed(xpsDevice_t devId, uint32_t hashSeed)
{
    GT_STATUS cpssStatus = GT_OK;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "xpsLagSetRandomHashSeed setting hashseed to (%d) for DevId (%d)", hashSeed,
          devId);

    cpssStatus = cpssHalSetRandomHashSeedParameters(devId,
                                                    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E, hashSeed);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Random Hash parameters on DevId (%d), client trunk", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}

/*API to get LAG HASH Seed*/
XP_STATUS xpsLagGetHashSeed(xpsDevice_t devId, uint32_t *hashSeed)
{
    GT_U8 devNum = (GT_U8)devId;
    GT_STATUS cpssStatus = GT_OK;
    XP_STATUS status = XP_NO_ERR;
    uint32_t  seedHash1 = 0;
    xpsLagHashAlgoType_e xpsHashAlgo;
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT crcMode;

    /*get Lag Hash Algo*/
    status = xpsLagGetHashAlgo(devId, &xpsHashAlgo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to get Trunk Hash Mode on device (%d)", devId);
        return status;
    }

    if (xpsHashAlgo != XPS_LAG_HASH_ALGO_CRC_E)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get hash seed as Trunk Hash Mode on device (%d) is not CRC", devId);
        return XP_ERR_FAILED;
    }

    /*as both the devices in CPSS will be symetrically set the same algo, we will retrieve from devId=0 and return*/
    cpssStatus = cpssDxChTrunkHashCrcParametersGet(devNum, &crcMode, hashSeed,
                                                   &seedHash1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to get Trunk Hash CRC parameters on DevId (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}

/*API to set LAG HASH Algorithm Global*/
XP_STATUS xpsLagSetHashAlgo(xpsDevice_t devId, xpsLagHashAlgoType_e xpsHashAlgo,
                            GT_U32 hashSeed)
{
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT cpssLagHashAlgo;
    GT_STATUS cpssStatus = GT_OK;

    if (xpsHashAlgo == XPS_LAG_HASH_ALGO_RANDOM_E)
    {
        /*random take effect in cpssDxChTrunkEcmpLttTableSet, the default value of randomEnable is false.
        ever time when add or remove a port to lag, we need to set randomEnable to true in ecmpLttTable*/
        return XP_NO_ERR;
    }
    else
    {
        cpssLagHashAlgo = ((xpsHashAlgo == XPS_LAG_HASH_ALGO_XOR_E) ?
                           CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E : CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E);
        cpssStatus = cpssHalSetTrunkHashGlobalMode(devId, cpssLagHashAlgo);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Falied to set Trunk Hash Mode on device (%d)", devId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /*if hash mode is CRC, set other CRC related parameters*/
        if (xpsHashAlgo == XPS_LAG_HASH_ALGO_CRC_E)
        {
            cpssStatus = cpssHalSetTrunkHashCrcParameters(devId,
                                                          CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E, hashSeed);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Falied to set Trunk Hash CRC parameters on DevId (%d)", devId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }

            /*cpssHalSetTrunkHashGlobalMode and cpssHalSetTrunkHashCrcParameters not set the hashBits for L3_ECMP(CRC)*/
            cpssStatus = cpssDxChIpEcmpHashNumBitsSet(devId, XP_LAG_START_BIT_FOR_ECMP_CRC,
                                                      XP_LAG_NUM_OF_BITS_FOR_CRC);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set the hash bit config: Device: %d, status: %d\n", devId,
                      cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else if (xpsHashAlgo == XPS_LAG_HASH_ALGO_XOR_E)
        {
            /*cpssHalSetTrunkHashGlobalMode not set the hashBits for L3_ECMP(XOR)*/
            cpssStatus = cpssDxChIpEcmpHashNumBitsSet(devId, XP_LAG_START_BIT_FOR_ECMP_XOR,
                                                      XP_LAG_NUM_OF_BITS_FOR_XOR);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set the hash bit config: Device: %d, status: %d\n", devId,
                      cpssStatus);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
    }

    return XP_NO_ERR;
}

/*API to get LAG HASH Algorithm Global*/
XP_STATUS xpsLagGetHashAlgo(xpsDevice_t devId,
                            xpsLagHashAlgoType_e *xpsHashAlgo)
{
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT cpssLagHashAlgo;
    GT_U8 devNum = (GT_U8)devId;
    GT_STATUS cpssStatus = GT_OK;

    /*as both the devices in CPSS will be symetrically set the same algo, we will retrieve from devId=0 and return*/
    cpssStatus = cpssDxChTrunkHashGlobalModeGet(devNum, &cpssLagHashAlgo);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Falied to retrieve Trunk Hash Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    *xpsHashAlgo = (xpsLagHashAlgoType_e)cpssLagHashAlgo;
    return XP_NO_ERR;
}

/*API to get LAG HASH BIT for lag or ecmp client*/
XP_STATUS xpsLagSetHashBit(xpsDevice_t devId, xpsLagHashAlgoType_e xpsHashAlgo,
                           bool isLag)
{
    GT_STATUS cpssStatus = GT_OK;
    GT_U32 startBit = 0;
    GT_U32 numOfBits = 0;
    switch (xpsHashAlgo)
    {
        case XPS_LAG_HASH_ALGO_CRC_32HI_E:
            startBit = XP_LAG_START_BIT_FOR_CRC_32HI;
            numOfBits = XP_LAG_NUM_OF_BITS_FOR_CRC;
            break;

        case XPS_LAG_HASH_ALGO_CRC_32LO_E:
            startBit = XP_LAG_START_BIT_FOR_CRC_32LO;
            numOfBits = XP_LAG_NUM_OF_BITS_FOR_CRC;
            break;

        default:
            return XP_ERR_INVALID_PARAMS;
            break;
    }
    if (isLag)
    {
        cpssStatus = cpssHalSetHashNumBits(devId, startBit, numOfBits,
                                           CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Falied to set Hash CRC BIT on DevId (%d) for CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E",
                  devId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        cpssStatus = cpssHalSetHashNumBits(devId, startBit, numOfBits,
                                           CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Falied to set Hash CRC BIT on DevId (%d) for CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E",
                  devId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    return XP_NO_ERR;
}

XP_STATUS xpsLagSetL3IntfToDb(xpsInterfaceId_t lagIntf,
                              xpsInterfaceId_t l3IntfId)
{

    return xpsLagSetL3IntfToDbScope(XP_SCOPE_DEFAULT, lagIntf, l3IntfId);

}

XP_STATUS xpsLagSetL3IntfToDbScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                   xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS               status  = XP_NO_ERR;
    static xpLagDbEntry_t   lagInfoKey;
    xpLagDbEntry_t          *lagInfo = NULL;

    lagInfoKey.lagIntf = lagIntf;
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", lagIntf);
        return status;
    }
    if (!lagInfo)
    {
        return XP_ERR_NOT_FOUND;
    }

    lagInfo->l3IntfId = l3IntfId;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetL3IntfFromDb(xpsInterfaceId_t lagIntf,
                                xpsInterfaceId_t *l3IntfId)
{

    return xpsLagGetL3IntfFromDbScope(XP_SCOPE_DEFAULT, lagIntf, l3IntfId);

}

XP_STATUS xpsLagGetL3IntfFromDbScope(xpsScope_t scopeId,
                                     xpsInterfaceId_t lagIntf, xpsInterfaceId_t *l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS               status  = XP_NO_ERR;
    static xpLagDbEntry_t   lagInfoKey;
    xpLagDbEntry_t          *lagInfo = NULL;

    lagInfoKey.lagIntf = lagIntf;
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", lagIntf);
        return status;
    }
    if (!lagInfo)
    {
        return XP_ERR_NOT_FOUND;
    }

    *l3IntfId = lagInfo->l3IntfId;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagCreate(xpsInterfaceId_t *lagIntf)
{

    return xpsLagCreateScope(XP_SCOPE_DEFAULT, lagIntf);

}

XP_STATUS xpsLagCreateScope(xpsScope_t scopeId, xpsInterfaceId_t *lagIntf)
{

    XPS_FUNC_ENTRY_LOG();

    XP_STATUS     status  = XP_NO_ERR;
    xpLagDbEntry_t *lagInfo = NULL;


    // We are allocating the interface Id for the LAG here. And putting in an Interface Database.
    // Allocate an interface id for this new lag
    if ((status = xpsInterfaceCreateScope(scopeId, XPS_LAG, lagIntf)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Allocation of interface id for LAG failed");
        return status;
    }
    // Create a new Lag Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpLagDbEntry_t),
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Heap Allocation for Lag info failed.");
        return status;
    }

    // Initialize the allocated state
    if ((status = xpsLagInfoInit(lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Initialization if Lag info failed");
        xpsStateHeapFree(lagInfo);
        return status;
    }

    lagInfo->lagIntf = *lagIntf;
    lagInfo->l3IntfId = XPS_INTF_INVALID_ID;

    // Insert the state into the database
    if ((status = xpsStateInsertData(scopeId, lagDbHandle,
                                     (void*)lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Insert data failed");
        xpsStateHeapFree(lagInfo);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Lag Created with Intf Id = %d ", *lagIntf);

    XPS_FUNC_EXIT_LOG();
    return status;


}

XP_STATUS xpsLagDestroy(xpsInterfaceId_t lagIntf)
{

    return xpsLagDestroyScope(XP_SCOPE_DEFAULT, lagIntf);

}

XP_STATUS xpsLagDestroyScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf)
{

    XP_STATUS     status  = XP_NO_ERR;
    static xpLagDbEntry_t  lagInfoKey;
    xpLagDbEntry_t *lagInfo = NULL;

    lagInfoKey.lagIntf = lagIntf;

    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", lagIntf);
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    if (lagInfo->numPorts)
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Destroy of lag interface(%d) failed. Please remove all the ports in the lag before destroying it",
              lagIntf);
        return XP_ERR_INVALID_INPUT;
    }

    // Remove the corresponding state
    if ((status = xpsStateDeleteData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Deletion of data failed, lag interface(%d)", lagIntf);
        return status;
    }

    // Dellocate the lag Id and Dsetroy the interface DB entry
    // Destroy the interface
    if ((status = xpsInterfaceDestroyScope(scopeId, lagIntf)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Desttroy Interface failed, lag interface(%d)", lagIntf);
        return status;
    }

    // Free the memory allocated for the corresponding state
    if ((status = xpsStateHeapFree(lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Freeing allocated memory failed.");
        return status;
    }

    return status;
}

XP_STATUS xpsLagAddPort(xpsInterfaceId_t lagIntf, xpsInterfaceId_t portIntf)
{

    return xpsLagAddPortScope(XP_SCOPE_DEFAULT, lagIntf, portIntf);

}

XP_STATUS xpsLagAddPortScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                             xpsInterfaceId_t portIntf)
{
    XP_STATUS          status  = XP_NO_ERR;
    static xpLagDbEntry_t     lagInfoKey;
    xpLagDbEntry_t    *lagInfo = NULL;
    xpsIntfLagMap_t    portInfoKey;
    xpsIntfLagMap_t   *portIntfInfo = NULL;
    xpsInterfaceInfo_t *intfInfo = NULL;
    uint32_t lagLocalId =0;
    GT_STATUS cpssStatus = GT_OK;
    CPSS_TRUNK_MEMBER_STC member;
    xpsDevice_t devId = 0;
    int devNum;
    int portNum;

    lagInfoKey.lagIntf = lagIntf;
    // Retrieve the lag information for this id if it exists
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    status = xpsInterfaceGetInfo(lagIntf, &intfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface info failed,"
              " devId(%d)  interface(%d)", devId, lagIntf);
        return status;
    }
    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface info failed recieved Null Pointer,"
              " devId(%d)  interface(%d)", devId, lagIntf);
        return XP_ERR_NULL_POINTER;
    }


    // Ensure before attaching port to Lag we have enough space in the lag intf port list
    if (lagInfo->numPorts >= XPS_LAG_MAX_PORTS)
    {
        status = XP_ERR_MAX_LIMIT;
        return status;
    }
    portInfoKey.portIntf = portIntf;

    // Retrieve the Port interface to Lag Map information for this portIntf if it exists
    if ((status = xpsStateSearchData(scopeId, intfToLagDbHandle,
                                     (xpsDbKey_t)&portInfoKey, (void**)&portIntfInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if ((portIntfInfo) && (portIntfInfo->lagIntf != lagIntf))
    {
        // Port interface is already member of some other Lag
        status = XP_ERR_INVALID_PORTNUM;
        return status;
    }

    for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
    {
        // Pick the first invalid location to populate the new port interface
        if (lagInfo->portIntf[i] == XPS_INTF_INVALID_ID)
        {
            lagInfo->portIntf[i] = portIntf;

            // Increment number of ports in the lag
            lagInfo->numPorts += 1;
            devNum = xpsGlobalIdToDevId(devId, portIntf);
            portNum = xpsGlobalPortToPortnum(devId, portIntf);
            member.hwDevice = devNum;
            member.port = portNum;
            lagLocalId = xpsUtilXpstoCpssInterfaceConvert(lagIntf, XPS_LAG);
            cpssStatus = cpssHalAddTrunkMember(devId, lagLocalId, &member);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Adding Port Id  = %d to Trunk Id = %d  Failed on Device Id = %d!!!\n",
                      member.port, lagIntf, devId);
                lagInfo->portIntf[i] = XPS_INTF_INVALID_ID;
                lagInfo->numPorts -= 1;
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }

            if (xpsLagRandomEnableGet() == true)
            {
                cpssStatus = cpssHalTrunkEcmpLttTableRandomEnableSet(devId, lagLocalId,
                                                                     GT_TRUE);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "failed to set TrunkEcmpLttTable for Trunk Id = %d on Device Id = %d!!!\n",
                          lagIntf, devId);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }

            cpssStatus = cpssHalEnableTrunkMember(devId, lagLocalId, &member);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Enabling Port Id  = %d to Trunk Id = %d  Failed on Device Id = %d!!!\n",
                      member.port, lagIntf, devId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
            // Create a new Port interface to Lag map structure
            if ((status = xpsStateHeapMalloc(sizeof(xpsIntfLagMap_t),
                                             (void**)&portIntfInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Heap Allocation for Intf Lag Map failed.");
                return status;
            }
            portIntfInfo->portIntf = portIntf;
            portIntfInfo->lagIntf = lagIntf;

            // Insert the state into the database
            if ((status = xpsStateInsertData(scopeId, intfToLagDbHandle,
                                             (void*)portIntfInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Insert data failed");
                return status;
            }
            /* Add this port interface to all vlans this lag belongs to */
            if ((status = xpsVlanAddPortToLag(devId, lagInfo->lagIntf,
                                              portIntf)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Add port to lag failed, portIntf(%d)", lagInfo->portIntf[i]);
                return status;
            }

            /* Add this port interface to all L2MC this lag belongs to */
            if ((status = xpsMulticastAddRemoveLagPortToL2Mc(devId, lagInfo->lagIntf,
                                                             portIntf, false)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Add port to lag failed, portIntf(%d)", lagInfo->portIntf[i]);
                return status;
            }

            if (intfInfo->stpId != XPS_INVALID_STP)
            {
                if ((status = xpsStpUpdLagMember(devId, lagInfo->lagIntf, portIntf,
                                                 false)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update STP state on added LAG member failed, port interface(%d)", portIntf);
                    return status;
                }
            }

            /* Below API is valid for L3 over LAG.*/
            if ((status = xpsL3UpdateLagMember(devId, lagInfo->lagIntf, portIntf,
                                               true)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Update L3 info on added LAG member failed, port interface(%d)", portIntf);
                return status;
            }

            /* Below API is valid for Mirror monitor intf is LAG.*/
            if (lagInfo->isMirrorIntf)
            {
                if ((status = xpsMirrorUpdLagMember(devId, lagInfo->lagIntf, portIntf,
                                                    false, lagInfo->mirrorAnalyzerId)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update Mirror on added LAG member failed, port interface(%d)", portIntf);
                    return status;
                }
            }
            /*Add LagInfo to Port list*/
            xpsInterfaceInfo_t *portInfo = NULL;
            status = xpsInterfaceGetInfoScope(scopeId, portIntf, &portInfo);
            if (status != XP_NO_ERR || portInfo == NULL)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to get portInfo %d\n", portIntf);
                return status;
            }
            status = xpsIntfChildAdd(portInfo, lagIntf, XPS_LAG,
                                     xpsUtilXpstoCpssInterfaceConvert(lagIntf, XPS_LAG));
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to add LAG intf %d to Port %d\n", lagIntf,
                      portIntf);
                return status;
            }

            break;
        }
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Adding Port Id  = %d to Trunk Id = %d  successful on Device Id = %d!!!\n",
          portIntf, lagIntf, devId);

    return status;
}

XP_STATUS xpsLagRemovePort(xpsInterfaceId_t lagIntf, xpsInterfaceId_t portIntf)
{
    return xpsLagRemovePortScope(XP_SCOPE_DEFAULT, lagIntf, portIntf);
}

XP_STATUS xpsLagRemovePortScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                xpsInterfaceId_t portIntf)
{
    XP_STATUS          status  = XP_NO_ERR;
    xpLagDbEntry_t     lagInfoKey;
    xpLagDbEntry_t    *lagInfo = NULL;
    xpsIntfLagMap_t    portInfoKey;
    uint32_t lagLocalId =0;
    xpsIntfLagMap_t   *portIntfInfo = NULL;
    xpsInterfaceType_e type;
    CPSS_TRUNK_MEMBER_STC member;
    xpsDevice_t devId = 0;
    GT_STATUS cpssStatus = GT_OK;
    int cpssPortNum ;
    int cpssDevId ;

    xpsInterfaceInfo_t *intfInfo = NULL;
    xpsInterfaceInfo_t *portInfo = NULL;

    status = xpsInterfaceGetInfoScope(scopeId, portIntf, &portInfo);
    if (status != XP_NO_ERR || portInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get portInfo %d\n", portIntf);
        return status;
    }

    type = portInfo->type;
    if (type != XPS_PORT)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid port type");
        return XP_ERR_INVALID_PARAMS;
    }

    lagInfoKey.lagIntf = lagIntf;
    // Retrieve the lag information for this id if it exists
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", lagIntf);
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    portInfoKey.portIntf = portIntf;
    // Retrieve the Port interface to Lag Map information for this portIntf if it exists
    if ((status = xpsStateSearchData(scopeId, intfToLagDbHandle,
                                     (xpsDbKey_t)&portInfoKey, (void**)&portIntfInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if ((!portIntfInfo) || (portIntfInfo->lagIntf != lagIntf))
    {
        // Port interface is not member of this Lag
        status = XP_ERR_KEY_NOT_FOUND;
        return status;
    }

    status = xpsInterfaceGetInfo(lagIntf, &intfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface info failed,"
              " devId(%d)  interface(%d)", devId, lagIntf);
        return status;
    }
    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface info failed recieved Null Pointer,"
              " devId(%d)  interface(%d)", devId, lagIntf);
        return XP_ERR_NULL_POINTER;
    }

    // Remove the port to the port list by search
    for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
    {
        if ((lagInfo->portIntf[i] == portIntf))
        {
            cpssDevId = xpsGlobalIdToDevId(devId, portIntf);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portIntf);
            member.hwDevice = cpssDevId;
            member.port = cpssPortNum;
            lagLocalId = xpsUtilXpstoCpssInterfaceConvert(lagIntf, XPS_LAG);
            cpssStatus = cpssHalRemoveTrunkMember(devId, lagLocalId, &member);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Removing Port Id  = %d to Trunk Id = %d  Failed on Device Id = %d!!!\n",
                      member.port, lagIntf, devId);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            if (xpsLagRandomEnableGet() == true)
            {
                cpssStatus = cpssHalTrunkEcmpLttTableRandomEnableSet(devId, lagLocalId,
                                                                     GT_TRUE);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "failed to set TrunkEcmpLttTable for Trunk Id = %d on Device Id = %d!!!\n",
                          lagIntf, devId);
                    return xpsConvertCpssStatusToXPStatus(cpssStatus);
                }
            }

            lagInfo->portIntf[i] = XPS_INTF_INVALID_ID;

            // Decrement number of ports in the lag.
            lagInfo->numPorts -= 1;

            // Remove the corresponding state port interface to Lag map entry
            if ((status = xpsStateDeleteData(scopeId, intfToLagDbHandle,
                                             (xpsDbKey_t)&portInfoKey, (void**)&portIntfInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " Deletion of data failed, portIntf (%d)", portIntf);
                return status;
            }
            // Free the memory allocated for the corresponding state
            if ((status = xpsStateHeapFree((void*)portIntfInfo)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Freeing allocated memory failed.");
                return status;
            }
            if ((status = xpsVlanRemovePortFromLag(devId, lagInfo->lagIntf,
                                                   portIntf)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Remove port from lag failed, port interface(%d)",
                      lagInfo->portIntf[i]);
                return status;
            }

            /* Remove this port interface from all L2MC this lag belongs to */
            if ((status = xpsMulticastAddRemoveLagPortToL2Mc(devId, lagInfo->lagIntf,
                                                             portIntf, true)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Remove from lag failed, portIntf(%d)", lagInfo->portIntf[i]);
                return status;
            }
            if (intfInfo->stpId != XPS_INVALID_STP)
            {
                if ((status = xpsStpUpdLagMember(devId, lagInfo->lagIntf, portIntf,
                                                 true)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update STP state on added LAG member failed, port interface(%d)", portIntf);
                    return status;
                }
            }
            /* Below API is valid for L3 over LAG.*/
            if ((status = xpsL3UpdateLagMember(devId, lagInfo->lagIntf, portIntf,
                                               false)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Update l3 info on added LAG member failed, port interface(%d)", portIntf);
                return status;
            }

            if (lagInfo->isMirrorIntf)
            {
                /* Below API is valid for Mirror monitor intf is LAG.*/
                if ((status = xpsMirrorUpdLagMember(devId, lagInfo->lagIntf, portIntf,
                                                    true, lagInfo->mirrorAnalyzerId)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Update Mirror on removed LAG member failed, port interface(%d)", portIntf);
                    return status;
                }
            }
            status = xpsIntfChildRemove(portInfo, lagIntf);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove Lag intf %d from Port %d\n", lagIntf,
                      portIntf);
                return status;
            }

            break;
        }
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Removing Port Id  = %d to Trunk Id = %d  Successful on Device Id = %d!!!\n",
          portIntf, lagIntf, devId);
    return status;
}

XP_STATUS xpsLagGetIngressPortIntfList(xpsScope_t scopeId,
                                       xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetEgressPortIntfList(xpsScope_t scopeId,
                                      xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetPortIntfList(xpsInterfaceId_t lagIntf,
                                xpsLagPortIntfList_t *portIntfList)
{
    return xpsLagGetPortIntfListScope(XP_SCOPE_DEFAULT, lagIntf, portIntfList);
}

XP_STATUS xpsLagGetPortIntfListScope(xpsScope_t scopeId,
                                     xpsInterfaceId_t lagIntf, xpsLagPortIntfList_t *portIntfList)
{

    XPS_LOCK(xpsLagGetPortIntfListScope);
    XP_STATUS status  = XP_NO_ERR;
    static xpLagDbEntry_t  lagInfoKey;
    xpLagDbEntry_t *lagInfo = NULL;

    /* Validate input parameters */
    if (portIntfList == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    lagInfoKey.lagIntf = lagIntf;
    /* Search for lag interface */
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", lagIntf);
        return status;
    }

    if (lagInfo == NULL)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    /* Iterate through the list of available port interfaces
       Note that caller must initialize size to 0 and in Vxlan case,
       size will be non-zero, so continue the numbering. */
    for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
    {
        // Pick the valid port interface items
        if (lagInfo->portIntf[i] != XPS_INTF_INVALID_ID)
        {
            portIntfList->portIntf[portIntfList->size] = lagInfo->portIntf[i];
            portIntfList->size++;
        }
    }

    return status;
}

XP_STATUS xpsLagMemberIngressEnablePort(xpsDevice_t devId,
                                        xpsInterfaceId_t lagIntf,
                                        xpsInterfaceId_t portIntf, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;
    xpsStpState_e stpState;
    xpsStp_t stpId;
    xpsInterfaceInfo_t *intfInfo = NULL;

    status = xpsInterfaceGetInfo(lagIntf, &intfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get interface info failed,"
              " devId(%d)  interface(%d)", devId, lagIntf);
        return status;
    }
    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface info failed recieved Null Pointer,"
              " devId(%d)  interface(%d)", devId, lagIntf);
        return XP_ERR_NULL_POINTER;
    }
    stpId = intfInfo->stpId;

    if (enable)
    {
        if ((status = xpsStpGetState(devId, stpId, lagIntf, &stpState)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "STP get failed for (%d)", lagIntf);
            stpState = SPAN_STATE_DISABLE;
        }
        if ((status = xpsStpSetPortStpState(devId, portIntf,
                                            stpState)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Update STP state on added LAG member failed, port interface(%d)", portIntf);
            return status;
        }
    }
    else
    {
        /* Lag member Ingress is disabled using STP_BLOCK_STATE */
        if ((status = xpsStpSetPortStpState(devId, portIntf,
                                            SPAN_STATE_BLOCK)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Update STP state on added LAG member failed, port interface(%d)", portIntf);
            return status;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagMemberEgressEnablePort(xpsDevice_t devId,
                                       xpsInterfaceId_t lagIntf,
                                       xpsInterfaceId_t portIntf, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpssStatus = GT_OK;
    CPSS_TRUNK_MEMBER_STC member;
    uint32_t lagLocalId =0;
    int devNum;
    int portNum;

    devNum = xpsGlobalIdToDevId(devId, portIntf);
    portNum = xpsGlobalPortToPortnum(devId, portIntf);
    member.hwDevice = devNum;
    member.port = portNum;
    lagLocalId = xpsUtilXpstoCpssInterfaceConvert(lagIntf, XPS_LAG);

    if (enable)
    {
        cpssStatus = cpssHalEnableTrunkMember(devId, lagLocalId, &member);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Enabling Port Id  = %d to Trunk Id = %d  Failed on Device Id = %d!!!\n",
                  member.port, lagIntf, devId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    else
    {
        cpssStatus = cpssHalDisableTrunkMember(devId, lagLocalId, &member);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Disabling Port Id  = %d to Trunk Id = %d  Failed on Device Id = %d!!!\n",
                  member.port, lagIntf, devId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagDeploy(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                       xpsLagAutoDistributeControl_e autoEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetIngressOperationalState(xpsDevice_t devId,
                                           xpsInterfaceId_t lagIntf, xpsInterfaceId_t portIntf, uint32_t *state)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagIngressEnablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                  xpsInterfaceId_t portIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagIngressDisablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                   xpsInterfaceId_t portIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetEgressOperationalState(xpsDevice_t devId,
                                          xpsInterfaceId_t lagIntf, xpsInterfaceId_t portIntf, uint32_t *state)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagEgressEnablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                 xpsInterfaceId_t portIntf, xpsLagAutoDistributeControl_e autoEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagEgressDisablePort(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                  xpsInterfaceId_t portIntf, xpsLagAutoDistributeControl_e autoEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagDeployIngress(xpsDevice_t devId, xpsInterfaceId_t lagIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagDeployEgress(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                             xpsLagAutoDistributeControl_e autoEnable, bool isRedirected)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagReadDistributionTable(xpsDevice_t devId, uint8_t *tableBuffer)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagWriteDistributionTable(xpsDevice_t devId, uint8_t *tableBuffer)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagAddPortToDistributionEntry(xpsDevice_t devId, uint32_t idx,
                                           xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagRemovePortFromDistributionEntry(xpsDevice_t devId, uint32_t idx,
                                                xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagReadDistributionTableEntry(xpsDevice_t devId, uint32_t idx,
                                           xpLagDistributionEntry_t *entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagWriteDistributionTableEntry(xpsDevice_t devId, uint32_t idx,
                                            xpLagDistributionEntry_t *entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetDistributionTableEntryWidth(xpDevice_t devId,
                                               uint32_t *entryWidthBits)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetDistributionTableDepth(xpDevice_t devId,
                                          uint32_t *tableDepth)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagIsPortMember(xpsDevice_t devId, xpPort_t port,
                             xpsInterfaceId_t lagIntf, uint32_t *isMember)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS        status = XP_NO_ERR;
    xpsInterfaceId_t portIntf;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    if ((status = xpsPortGetPortIntfId(devId, port, &portIntf)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get port interface id  failed");
        return status;
    }

    if ((status = xpsLagIsPortIntfMemberScope(scopeId, portIntf, lagIntf,
                                              isMember)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Post is not an Interface member, "
              "port interface(%d) lag interface(%d)", portIntf, lagIntf);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagIsPortIntfMember(xpsInterfaceId_t portIntf,
                                 xpsInterfaceId_t lagIntf, uint32_t *isMember)
{
    return xpsLagIsPortIntfMemberScope(XP_SCOPE_DEFAULT, portIntf, lagIntf,
                                       isMember);
}

XP_STATUS xpsLagIsPortIntfMemberScope(xpsScope_t scopeId,
                                      xpsInterfaceId_t portIntf, xpsInterfaceId_t lagIntf, uint32_t *isMember)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS     status  = XP_NO_ERR;
    static xpLagDbEntry_t  lagInfoKey;
    xpLagDbEntry_t *lagInfo = NULL;

    lagInfoKey.lagIntf = lagIntf;

    // Search for the lag info structure for this lag interface
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search for Lag interface failed, lag interface(%d)", lagIntf);
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    // Search for this portIntf (membership is determined by dirty value and portIntf value
    *isMember = 0;
    for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
    {
        if (lagInfo->portIntf[i] == portIntf)
        {
            *isMember = 1;
            break;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsLagEnableMirroring(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsLagSetHashFields(xpsDevice_t deviceId, xpHashField* fields,
                              size_t size)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsLagSetHashFields);
    XP_STATUS status = XP_NO_ERR;
    GT_U8 devNum = deviceId;
    GT_STATUS cpssStatus = GT_OK;
    bool lagHashFieldArray[XP_NUM_HASH_FIELDS] = {GT_FALSE};
    uint8_t validFlag = 0;
    status = xpsLagResetHashFields(deviceId);
    if (status !=XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Disabling Hash Fields failed");
        return status;
    }

    status = xpsLagEnableHashFieldsInArray(fields, size, lagHashFieldArray);
    if (status !=XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Boolean array of Hash Fields is all false");
        return status;
    }

    /* XP_ETHERNET_MAC_DA, XP_ETHERNET_MAC_SA */
    if ((lagHashFieldArray[XP_ETHERNET_MAC_DA]==true)&&
        (lagHashFieldArray[XP_ETHERNET_MAC_SA]==true))
    {
        validFlag++;
        /*set fields to be enabled for LAG hash selection as SMAC and DMAC*/
        cpssStatus = cpssHalSetTrunkHashIpAddMacMode(devNum, GT_TRUE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Trunk Hash Mac Mode on device (%d)", deviceId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    /*XP_IPV4_SOURCE_IP_ADDRESS , XP_IPV4_DESTINATION_IP_ADDRESS */
    if ((lagHashFieldArray[XP_IPV4_SOURCE_IP_ADDRESS]==true)&&
        (lagHashFieldArray[XP_IPV4_DESTINATION_IP_ADDRESS]==true))
    {
        validFlag++;
        /* XP_UDP_SOURCE_PORT ,XP_UDP_DESTINATION_PORT or XP_TCP_SOURCE_PORT ,XP_TCP_DESTINATION_PORT */
        if ((lagHashFieldArray[XP_UDP_SOURCE_PORT]==true)&&
            (lagHashFieldArray[XP_UDP_DESTINATION_PORT]==true)&&
            (lagHashFieldArray[XP_TCP_SOURCE_PORT]==true)&&
            (lagHashFieldArray[XP_TCP_DESTINATION_PORT]==true))
        {
            /*Enable IPV4 Mode before enabling L4 Mode*/
            cpssStatus = cpssHalSetTrunkHashIpMode(devNum, GT_TRUE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set Trunk Hash Ipv4 Mode on device (%d)", deviceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }

            /*set fields to be enabled for LAG hash selection L4 source port and destination port*/
            /*Use long L4 Hash Mode. L4 Trunk hash function is based on bits [5:0]
              and [13:8] of the UDP/TCP destination and source ports.*/
            cpssStatus = cpssHalSetTrunkHashL4Mode(devNum, CPSS_DXCH_TRUNK_L4_LBH_LONG_E);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set Trunk Hash L4 Mode on device (%d)", deviceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }
        else
        {
            /*set fields to be enabled for LAG hash selection as Ipv4 SIP and DIP*/
            cpssStatus = cpssHalSetTrunkHashIpMode(devNum, GT_TRUE);
            if (cpssStatus != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set Trunk Hash Ipv4 Mode on device (%d)", deviceId);
                return xpsConvertCpssStatusToXPStatus(cpssStatus);
            }
        }

    }

    /* XP_IPV6_FLOW_LABEL, XP_IPV6_SPLIT_SOURCE_IP_ADDRESS, XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS */
    if ((lagHashFieldArray[XP_IPV6_FLOW_LABEL]==true)&&
        (lagHashFieldArray[XP_IPV6_SPLIT_SOURCE_IP_ADDRESS]==true)&&
        (lagHashFieldArray[XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS]==true))
    {
        validFlag++;
        /*set fields to be enabled for LAG hash selection as IPv6 SIP, DIP and flow*/
        /*set hashMode to use MSB and LSB of SIP, DIP and Flow Label*/
        cpssStatus = cpssHalSetTrunkHashIpv6Mode(devNum,
                                                 CPSS_DXCH_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Trunk Hash Ipv6 Mode on device (%d)", deviceId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }

    /* XP_MPLS_LABEL0,XP_MPLS_LABEL1,XP_MPLS_LABEL2 */
    if ((lagHashFieldArray[XP_MPLS_LABEL0]==true)&&
        (lagHashFieldArray[XP_MPLS_LABEL1]==true)&&
        (lagHashFieldArray[XP_MPLS_LABEL2]==true))
    {
        validFlag++;
        /*set fields to be enabled for LAG hash selection as MPLS labels*/
        /*set hashMode to use bits [5:0] in MPLS Label 0,1 and 2*/
        cpssStatus = cpssHalSetTrunkHashMplsModeEnable(devNum, GT_TRUE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set Trunk Hash MPLS Mode on device (%d)", deviceId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

    }

    if (!validFlag)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid Hash Fields/ Hash fields not supported");
        return XP_ERR_INVALID_PARAMS;
    }


    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsLagGetHashFields(xpsDevice_t devId, xpHashField* fields,
                              uint32_t* size)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsLagGetHashFields);
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    GT_BOOL isMac, isIpv4, isMpls = GT_FALSE;
    CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT l4HashMode;
    CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT ipv6HashMode;
    uint32_t arrIndex = 0;

    /* Since we program both the devices symmetrically, device iterator is not used
       value is fetched from device zero and returned in fields*/
    cpssStatus = cpssDxChTrunkHashIpAddMacModeGet(devId, &isMac);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Trunk Hash Mac Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    if (isMac)
    {
        fields[arrIndex++] = XP_ETHERNET_MAC_DA;
        fields[arrIndex++] = XP_ETHERNET_MAC_SA;
    }

    cpssStatus = cpssDxChTrunkHashIpModeGet(devId, &isIpv4);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Trunk Hash Ipv4 Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    if (isIpv4)
    {
        cpssStatus = cpssDxChTrunkHashL4ModeGet(devId, &l4HashMode);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get Trunk Hash L4 Mode on device (%d)", devId);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        if (l4HashMode == CPSS_DXCH_TRUNK_L4_LBH_LONG_E)
        {
            fields[arrIndex++] = XP_IPV4_SOURCE_IP_ADDRESS;
            fields[arrIndex++] = XP_IPV4_DESTINATION_IP_ADDRESS;
            fields[arrIndex++] = XP_UDP_SOURCE_PORT;
            fields[arrIndex++] = XP_UDP_DESTINATION_PORT;
            fields[arrIndex++] = XP_TCP_SOURCE_PORT;
            fields[arrIndex++] = XP_TCP_DESTINATION_PORT;
        }
        else
        {
            fields[arrIndex++] = XP_IPV4_SOURCE_IP_ADDRESS;
            fields[arrIndex++] = XP_IPV4_DESTINATION_IP_ADDRESS;
        }
    }

    cpssStatus = cpssDxChTrunkHashIpv6ModeGet(devId, &ipv6HashMode);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Trunk Hash Ipv6  Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    if (ipv6HashMode == CPSS_DXCH_TRUNK_IPV6_HASH_MSB_LSB_SIP_DIP_FLOW_E)
    {
        fields[arrIndex++] = XP_IPV6_SPLIT_SOURCE_IP_ADDRESS;
        fields[arrIndex++] = XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS;
        fields[arrIndex++] = XP_IPV6_FLOW_LABEL;
    }

    cpssStatus = cpssDxChTrunkHashMplsModeEnableGet(devId, &isMpls);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Trunk Hash Mpls Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    if (isMpls)
    {
        fields[arrIndex++] = XP_MPLS_LABEL0;
        fields[arrIndex++] = XP_MPLS_LABEL1;
        fields[arrIndex++] = XP_MPLS_LABEL2;
    }

    *size = arrIndex;

    XPS_FUNC_EXIT_LOG();

    return status;
}

//TODO Hack for clearing HashFieldList before SetFields each time
static XP_STATUS xpsLagResetHashFields(xpsDevice_t devId)
{
    GT_STATUS cpssStatus = GT_OK;

    cpssStatus = cpssHalSetTrunkHashIpAddMacMode(devId, GT_FALSE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to reset Trunk Hash Mac Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    cpssStatus = cpssHalSetTrunkHashIpMode(devId, GT_FALSE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to reset Trunk Hash Ipv4 Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalSetTrunkHashL4Mode(devId,
                                           CPSS_DXCH_TRUNK_L4_LBH_DISABLED_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to reset Trunk Hash L4 Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    cpssStatus = cpssHalSetTrunkHashIpv6Mode(devId,
                                             CPSS_DXCH_TRUNK_IPV6_HASH_LSB_SIP_DIP_FLOW_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to reset Trunk Hash Ipv6 Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalSetTrunkHashMplsModeEnable(devId, GT_FALSE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to reset Trunk Hash MPLS Mode on device (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;

}

XP_STATUS xpsLagSetHashFieldsWithHashId(xpsDevice_t deviceId,
                                        xpHashField* fields,
                                        size_t size, uint8_t hashId, bool symmetricHashEnable)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsLagSetHashFieldsWithHashId);
    XP_STATUS status = XP_NO_ERR;
    GT_U8 devNum = deviceId;
    GT_STATUS cpssStatus = GT_OK;
    bool lagHashFieldArray[XP_NUM_HASH_FIELDS] = {GT_FALSE};
    CPSS_DXCH_TRUNK_LBH_CRC_MASK_ENTRY_STC dxChEntry;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT packetType = CPSS_DXCH_PCL_PACKET_TYPE_LAST_E;

    memset(&dxChEntry, 0, sizeof(dxChEntry));

    /*support to set all field to false, when size == 0, lagHashFieldArray is all false*/
    for (uint8_t i = 0; i<(uint8_t)size; i++)
    {
        lagHashFieldArray[fields[i]] = GT_TRUE;
    }

    switch (hashId)
    {
        case XP_HASH_MASK_TABLE_IPV4_TCP:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
            break;
        case XP_HASH_MASK_TABLE_IPV4_UDP:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
            break;
        case XP_HASH_MASK_TABLE_IPV4_FRAGMENT:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E;
            break;
        case XP_HASH_MASK_TABLE_IPV4_OTHER:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E;
            break;
        case XP_HASH_MASK_TABLE_ETHERNET:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E;
            break;
        case XP_HASH_MASK_TABLE_IPV6:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E;
            break;
        case XP_HASH_MASK_TABLE_IPV6_TCP:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E;
            break;
        case XP_HASH_MASK_TABLE_IPV6_UDP:
            packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid hash mask table");
            return XP_ERR_INVALID_TABLE_TYPE;
    }

    if (lagHashFieldArray[XP_ETHERNET_MAC_SA] == true)
    {
        dxChEntry.macSaMaskBmp = 0x3f;
    }

    if (lagHashFieldArray[XP_ETHERNET_MAC_DA] == true)
    {
        dxChEntry.macDaMaskBmp = 0x3f;
    }

    if (lagHashFieldArray[XP_IPV4_SOURCE_IP_ADDRESS] == true)
    {
        dxChEntry.ipSipMaskBmp = 0xf000;
    }

    if (lagHashFieldArray[XP_IPV4_DESTINATION_IP_ADDRESS] == true)
    {
        dxChEntry.ipDipMaskBmp = 0xf000;
    }

    if (lagHashFieldArray[XP_IPV6_SPLIT_SOURCE_IP_ADDRESS] == true)
    {
        dxChEntry.ipSipMaskBmp = 0xffff;
    }

    if (lagHashFieldArray[XP_IPV6_SPLIT_DESTINATION_IP_ADDRESS] == true)
    {
        dxChEntry.ipDipMaskBmp = 0xffff;
    }

    if (lagHashFieldArray[XP_ETHERNET_S_TAG_VID] == true)
    {
        dxChEntry.mplsLabel0MaskBmp = 0x7;
    }

    if (lagHashFieldArray[XP_IN_PORT] == true)
    {
        dxChEntry.localSrcPortMaskBmp = 0x1;
    }

    if ((lagHashFieldArray[XP_UDP_SOURCE_PORT] == true) ||
        (lagHashFieldArray[XP_TCP_SOURCE_PORT] == true))
    {
        dxChEntry.l4SrcPortMaskBmp = 0x3;
    }

    if ((lagHashFieldArray[XP_UDP_DESTINATION_PORT] == true) ||
        (lagHashFieldArray[XP_TCP_DESTINATION_PORT] == true))
    {
        dxChEntry.l4DstPortMaskBmp = 0x3;
    }

    /* udb14 udb15 for EtherType, non-ip acl not use udb14 and udb15*/
    if (lagHashFieldArray[XP_ETHERNET_ETHERTYPE] == true)
    {
        cpssStatus = cpssHalPclUserDefinedByteSet(deviceId,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                                  14,/* ethernet*/
                                                  CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                                  0);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte, error:%d\n ", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        cpssStatus = cpssHalPclUserDefinedByteSet(deviceId,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                                  15,/* ethernet*/
                                                  CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                                  1);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte,error:%d\n ", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        dxChEntry.udbsMaskBmp |= 0xC000;
    }
    /* udb16 for v4 protocol, ip acl alse use udb16 as protocol, they do the same action*/
    if (lagHashFieldArray[XP_IPV4_PROTOCOL] == true)
    {
        cpssStatus = cpssHalPclUserDefinedByteSet(deviceId,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  packetType,
                                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                                  16,/* ipProtocol*/
                                                  CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                                  11);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte, error:%d\n ", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        dxChEntry.udbsMaskBmp |= 0x10000;
    }
    /* udb16 for v6 protocol, ip acl alse use udb16 as protocol, they do the same action*/
    if (lagHashFieldArray[XP_IPV6_NEXT_HEADER] == true)
    {
        cpssStatus = cpssHalPclUserDefinedByteSet(deviceId,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                                  packetType,
                                                  CPSS_PCL_DIRECTION_INGRESS_E,
                                                  16,/* ipProtocol*/
                                                  CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                                  8);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set user defined byte, error:%d\n ", cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        dxChEntry.udbsMaskBmp |= 0x10000;
    }

    dxChEntry.symmetricMacAddrEnable = (symmetricHashEnable? GT_TRUE: GT_FALSE);
    dxChEntry.symmetricIpv4AddrEnable = (symmetricHashEnable? GT_TRUE: GT_FALSE);
    dxChEntry.symmetricIpv6AddrEnable = (symmetricHashEnable? GT_TRUE: GT_FALSE);
    dxChEntry.symmetricL4PortEnable = (symmetricHashEnable? GT_TRUE: GT_FALSE);

    cpssStatus = cpssDxChTrunkHashMaskCrcEntrySet(devNum, 0, hashId,
                                                  &dxChEntry);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the Hash mask entry: Index: %d Status: %d\n", hashId,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    return status;
}

XP_STATUS xpsLagSetLagHashPolynomial(xpsDevice_t deviceId, uint32_t instance,
                                     uint32_t polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetLagHashPolynomial(xpsDevice_t deviceId, uint32_t instance,
                                     uint32_t* polynomialId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagDisableMirroring(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                                 uint32_t analyzerId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagSetLagHashPolynomialSeed(xpsDevice_t deviceId,
                                         uint32_t instance, uint32_t seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetLagHashPolynomialSeed(xpsDevice_t deviceId,
                                         uint32_t instance, uint32_t* seed)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


/*
 * Static APIs for internal use
 */

/**
 * \brief Function that provides Lag Port distribution
 *        functionality
 *
 * A Lag in the system is a group of ports treated as a single
 * logical interface. It is represented in the system as Lag
 * Interface.
 *
 * XPS Lag will interact with the XPS Interface Manager and
 * will return a lag interface which will be passed back to the
 * caller.
 *
 * Traffic is distributed across a lag. This distribution is
 * represented in hardware via the programming of the Trunk
 * Resolution Table. This table contains a bit map of all ports
 * on a device. The Lag packet hash will be used to select one
 * of these bit maps and from its entry a port in a lag will be
 * chosen for egress.
 *
 * For a lag of 2 ports (0, 1) the table is programmed as
 * follows:
 *      - 1 0 1 1 1 1 1 ... 1
 *      - 0 1 1 1 1 1 1 ... 1
 *      - 1 0 1 1 1 1 1 ... 1
 *      - 0 1 1 1 1 1 1 ... 1
 *      - etc...
 *
 * Currently, auto distribution will only distribute ports
 * evenly across a Lag. Cost is not taken into account.
 *
 * \param [in] devId
 * \param [in] lagIntf
 *
 * \return int
 */


/**
 * \brief API to initialize the Lag info structure
 *
 * \param [in] info
 *
 * \return int
 */

static XP_STATUS xpsLagInfoInit(xpLagDbEntry_t *info)
{
    int count;
    count = XP_MAX_DEVICES/SIZEOF_BYTE;

    // Invalidate the lag interface id
    info->lagIntf = XPS_INTF_INVALID_ID;

    for (uint32_t i = 0; i < XPS_LAG_MAX_PORTS; i++)
    {
        // Invalidate all of the port interfaces associated with this lag
        info->portIntf[i] = XPS_INTF_INVALID_ID;

        // Memset the ingress operational state
        info->ingrOper[i] = PORT_OPER_ENABLED;

        // Memset the egress operational state
        info->egrOper[i] = PORT_OPER_ENABLED;
    }

    // Memset the device Map
    memset(info->deviceMap.bits, 0, sizeof(count));

    // Zero out the number of ports
    info->numPorts = 0;

    info->isMirrorIntf = false;
    info->mirrorAnalyzerId = 0;

    return XP_NO_ERR;
}

static XP_STATUS xpsLagEnableHashFieldsInArray(xpHashField* fields, size_t size,
                                               bool* lagHashFieldArray)
{
    uint8_t flag = 0;

    for (uint8_t j = 0; j<(uint8_t)size; j++)
    {
        lagHashFieldArray[fields[j]] = GT_TRUE;
        flag++;
    }

    if (!flag)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Boolean array is all False");
        return XP_ERR_INVALID_DATA;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetFirst(xpsInterfaceId_t *lagIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetFirstScope(xpsScope_t scopeId, xpsInterfaceId_t *lagIntf)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetNext(xpsInterfaceId_t lagIntf, xpsInterfaceId_t *lagIntfNext)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetNextScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                             xpsInterfaceId_t *lagIntfNext)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsLagGetLagIdCount(uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS  retVal  = XP_NO_ERR;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;

    retVal = xpsStateGetCount(scopeId, lagDbHandle, count);
    XPS_FUNC_EXIT_LOG();
    return retVal;
}

XP_STATUS xpsLagGetLagIdList(xpsScope_t scopeId, xpsInterfaceId_t* lagId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS                 result                  = XP_NO_ERR;
    xpLagDbEntry_t *curLagDbEntry  = NULL;
    xpLagDbEntry_t *prevLagDbEntry = NULL;
    uint32_t                  count                   = 0;

    result = xpsStateGetNextData(scopeId, lagDbHandle, NULL,
                                 (void **)&curLagDbEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Failed to get Data | retVal : %d\n", result);
        return result;
    }

    while (curLagDbEntry)
    {
        //Store the node information in previous and get-next
        prevLagDbEntry = curLagDbEntry;
        lagId[count] = xpsUtilXpstoCpssInterfaceConvert(prevLagDbEntry->lagIntf,
                                                        XPS_LAG);

        result = xpsStateGetNextData(scopeId, lagDbHandle,
                                     (xpsDbKey_t)prevLagDbEntry, (void **)&curLagDbEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "Failed to get Data | retVal : %d\n", result);
            return result;
        }
        count++;
    }
    XPS_FUNC_EXIT_LOG();
    return result;;
}

void xpscustSaiUpdateEgressFilter(xpsInterfaceId_t xpsLag)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();


}

XP_STATUS xpsLagIpV4RouteEnable(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                bool isMcast, uint32_t enable)
{
    XP_STATUS          status  = XP_NO_ERR;
    xpLagDbEntry_t    *lagInfo = NULL;
    xpLagDbEntry_t    lagInfoKey;

    lagInfoKey.lagIntf = lagIntf;
    // Retrieve the lag information for this id if it exists
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    if (isMcast)
    {
        lagInfo->ipv4McRoutingEn = enable;
    }
    else
    {
        lagInfo->ipv4RoutingEn = enable;
    }
    return status;
}

XP_STATUS xpsLagIpV4RouteEnableGet(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                   bool isMcast, uint32_t *enable)
{
    XP_STATUS          status  = XP_NO_ERR;
    xpLagDbEntry_t    *lagInfo = NULL;
    xpLagDbEntry_t    lagInfoKey;

    lagInfoKey.lagIntf = lagIntf;
    // Retrieve the lag information for this id if it exists
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    if (isMcast)
    {
        *enable = lagInfo->ipv4McRoutingEn;
    }
    else
    {
        *enable = lagInfo->ipv4RoutingEn;
    }

    return status;
}

XP_STATUS xpsLagIpV6RouteEnable(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                bool isMcast, uint32_t enable)
{
    XP_STATUS          status  = XP_NO_ERR;
    xpLagDbEntry_t    *lagInfo = NULL;
    xpLagDbEntry_t    lagInfoKey;

    lagInfoKey.lagIntf = lagIntf;
    // Retrieve the lag information for this id if it exists
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    if (isMcast)
    {
        lagInfo->ipv6McRoutingEn = enable;
    }
    else
    {
        lagInfo->ipv6RoutingEn = enable;
    }

    return status;
}

XP_STATUS xpsLagIpV6RouteEnableGet(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                   bool isMcast, uint32_t *enable)
{
    XP_STATUS          status  = XP_NO_ERR;
    xpLagDbEntry_t    *lagInfo = NULL;
    xpLagDbEntry_t    lagInfoKey;

    lagInfoKey.lagIntf = lagIntf;
    // Retrieve the lag information for this id if it exists
    if ((status = xpsStateSearchData(scopeId, lagDbHandle, (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    if (isMcast)
    {
        *enable = lagInfo->ipv6McRoutingEn;
    }
    else
    {
        *enable = lagInfo->ipv6RoutingEn;
    }

    return status;
}

XP_STATUS xpsLagSetMirrorEnable(xpsInterfaceId_t lagIntf,
                                bool enable, uint32_t analyzerId)
{
    XP_STATUS      status  = XP_NO_ERR;
    xpLagDbEntry_t *lagInfo = NULL;
    xpLagDbEntry_t lagInfoKey;

    lagInfoKey.lagIntf = lagIntf;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, lagDbHandle,
                                     (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    lagInfo->isMirrorIntf = enable;
    lagInfo->mirrorAnalyzerId = analyzerId;

    return status;
}

XP_STATUS xpsLagGetMirrorEnable(xpsInterfaceId_t lagIntf,
                                bool *enable, uint32_t *analyzerId)
{
    XP_STATUS      status  = XP_NO_ERR;
    xpLagDbEntry_t *lagInfo = NULL;
    xpLagDbEntry_t lagInfoKey;

    lagInfoKey.lagIntf = lagIntf;
    if ((status = xpsStateSearchData(XP_SCOPE_DEFAULT, lagDbHandle,
                                     (xpsDbKey_t)&lagInfoKey,
                                     (void**)&lagInfo)) != XP_NO_ERR)
    {
        return status;
    }
    if (!lagInfo)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *enable = lagInfo->isMirrorIntf;
    *analyzerId = lagInfo->mirrorAnalyzerId;

    return status;
}


XP_STATUS xpsLagRandomEnableRewrite(xpsDevice_t devId, bool enable)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    uint32_t *trunkId = NULL;
    uint32_t count;

    status = xpsLagGetLagIdCount(&count);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    trunkId = (uint32_t *)xpMalloc(sizeof(uint32_t) * (count));
    if (trunkId == NULL)
    {
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    status = xpsLagGetLagIdList(XP_SCOPE_DEFAULT, trunkId);
    if (status != XP_NO_ERR)
    {
        xpFree(trunkId);
        return status;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        cpssStatus = cpssHalTrunkEcmpLttTableRandomEnableSet(devId, trunkId[i],
                                                             (enable ? GT_TRUE : GT_FALSE));
        if (cpssStatus != GT_OK)
        {
            xpFree(trunkId);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set IpEcmpEntry for devId %d trunkId:%d . Status %d\n", devId,
                  trunkId[i], cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    xpFree(trunkId);
    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif


/* adapter_init.c
 *
 * Adapter module responsible for adapter initialization tasks.
 *
 */

/*****************************************************************************
* Copyright (c) 2015 INSIDE Secure B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from INSIDE Secure.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* INSIDE Secure.
*
* For more information or support, please go to our online support system at
* https://customersupport.insidesecure.com.
* In case you do not have an account for this system, please send an e-mail
* to ESSEmbeddedHW-Support@insidesecure.com.
*****************************************************************************/

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include <Integration/Adapter_EIP164/incl/adapter_init.h>


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Driver Framework Device API */
#include <Kit/DriverFramework/incl/device_mgmt.h>
/* Driver Framework Basic Definitions API */
#include <Kit/DriverFramework/incl/basic_defs.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegsVer1.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecGlobalDb.h>

/*global variables macros*/
#define PRV_SHARED_MACSEC_ADAPTER_INIT_SRC_GLOBAL_VAR_SET(_var,_value)\
    ((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->adapterInitSrc._var = _value

#define PRV_SHARED_MACSEC_ADAPTER_INIT_SRC_GLOBAL_VAR_GET(_var)\
    (((PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr))->adapterInitSrc._var)


/*----------------------------------------------------------------------------
 * Adapter_Init
 *
 * Return Value
 *     true   Success
 *     false  Failure (fatal!)
 */
GT_BOOL
Adapter_Init(GT_U8 devNum, GT_U32 unitBmp)
{
    PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC * globalDbPtr;

    if (PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr) == NULL)
    {
        /* Allocate Global DB */
        globalDbPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC));
        if (globalDbPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(globalDbPtr, 0, sizeof(PRV_CPSS_DXCH_MACSEC_GLOBAL_DB_STC));
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr) = globalDbPtr;
    }

    /* trigger first-time initialization of the adapter */
    if (Device_Initialize(devNum, unitBmp) < 0)
        return false;

    PRV_SHARED_MACSEC_ADAPTER_INIT_SRC_GLOBAL_VAR_SET(Adapter_IsInitialized,true);

    return true;    /* success */
}


/*----------------------------------------------------------------------------
 * Adapter_UnInit
 */
void
Adapter_UnInit(GT_U8 devNum, GT_U32 unitBmp, GT_BOOL lastDevice)
{
    if (PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr) == NULL)
    {
        /* DB already destroyed */
        return;
    }

    if (!PRV_SHARED_MACSEC_ADAPTER_INIT_SRC_GLOBAL_VAR_GET(Adapter_IsInitialized))
    {
        return;
    }

    /* Set to false only if this is the last device working with MACSec */
    if (lastDevice)
    {
        PRV_SHARED_MACSEC_ADAPTER_INIT_SRC_GLOBAL_VAR_SET(Adapter_IsInitialized,false);
    }

    Device_UnInitialize(devNum, unitBmp, lastDevice);

    if (lastDevice)
    {
        /* Free global DB */
        cpssOsFree(PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr));
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr) = NULL;
    }
}


/*----------------------------------------------------------------------------
 * Adapter_Report_Build_Params
 */
void
Adapter_Report_Build_Params(void)
{
#ifdef LOG_INFO_ENABLED
    GT_32 dummy = 0;

    /* This function is dependent on config file cs_adapter.h. */
    /* Please update this when Config file for Adapter is changed. */
    LOG_INFO("Adapter build configuration:\n");

#define REPORT_SET(_X) \
    LOG_INFO("\t" #_X "\n")

#define REPORT_STR(_X) \
    LOG_INFO("\t" #_X ": %s\n", _X)

#define REPORT_INT(_X) \
    dummy = _X; LOG_INFO("\t" #_X ": %d\n", _X)

#define REPORT_HEX32(_X) \
    dummy = _X; LOG_INFO("\t" #_X ": 0x%08X\n", _X)

#define REPORT_EQ(_X, _Y) \
    dummy = (_X + _Y); LOG_INFO("\t" #_X " == " #_Y "\n")

#define REPORT_EXPL(_X, _Y) \
    LOG_INFO("\t" #_X _Y "\n")

    /* Adapter EIP-164 */
    LOG_INFO("Adapter EIP-164:\n");
    REPORT_STR(ADAPTER_EIP164_DRIVER_NAME);
    REPORT_STR(ADAPTER_EIP164_LICENSE);

#ifdef ADAPTER_EIP164_DBG
    REPORT_SET(ADAPTER_EIP164_DBG);
#endif

#ifdef ADAPTER_EIP164_STRICT_ARGS
    REPORT_SET(ADAPTER_EIP164_STRICT_ARGS);
#endif

    REPORT_INT(ADAPTER_EIP164_MAX_NOF_INIT_RETRY_COUNT);
    REPORT_INT(ADAPTER_EIP164_INIT_RETRY_TIMEOUT_MS);
    REPORT_INT(ADAPTER_EIP164_MAX_NOF_SYNC_RETRY_COUNT);
    REPORT_INT(ADAPTER_EIP164_SYNC_RETRY_TIMEOUT_MS);

    REPORT_INT(ADAPTER_EIP164_FIXED_LATENCY);
    REPORT_INT(ADAPTER_EIP164_STATIC_BYPASS);
    REPORT_INT(ADAPTER_EIP164_CLEAR_ON_READ);
    REPORT_INT(ADAPTER_EIP164_DEVICE_CLOCK_CONTROL);

    REPORT_INT(ADAPTER_EIP164_MAX_NOF_DEVICES);
    REPORT_INT(ADAPTER_EIP164_MAX_IC_COUNT);

#ifdef ADAPTER_EIP164_INTERRUPTS_ENABLE
    REPORT_EXPL(ADAPTER_EIP164_INTERRUPTS_ENABLE,
                " is SET => Interrupts ENABLED");
    REPORT_HEX32(ADAPTER_EIP164_INTERRUPTS_TRACEFILTER);
#else
    REPORT_EXPL(ADAPTER_EIP164_INTERRUPTS_ENABLE,
                " is NOT set => Interrupts DISABLED");
#endif

#ifdef ADAPTER_EIP164_64BIT_HOST
    REPORT_EXPL(ADAPTER_EIP164_64BIT_HOST,
                " is SET => addresses are 64-bit");
#else
    REPORT_EXPL(ADAPTER_EIP164_64BIT_HOST,
                " is NOT set => addresses are 32-bit");
#endif

#ifdef ADAPTER_EIP164_MODE_EGRESS
    REPORT_SET(ADAPTER_EIP164_MODE_EGRESS);
    /*REPORT_STR(ADAPTER_EIP164_DEVICE_NAME_EGRESS); */
#endif

#ifdef ADAPTER_EIP164_MODE_INGRESS
    REPORT_SET(ADAPTER_EIP164_MODE_INGRESS);
    /*REPORT_STR(ADAPTER_EIP164_DEVICE_NAME_INGRESS); */
#endif

#ifdef ADAPTER_EIP163_DRIVER_NAME
    /* Adapter EIP-163 */
    LOG_INFO("Adapter EIP-163:\n");
    REPORT_STR(ADAPTER_EIP163_DRIVER_NAME);

#ifdef ADAPTER_EIP163_DBG
    REPORT_SET(ADAPTER_EIP163_DBG);
#endif

#ifdef ADAPTER_EIP163_STRICT_ARGS
    REPORT_SET(ADAPTER_EIP163_STRICT_ARGS);
#endif

#ifdef ADAPTER_EIP163_MAX_NOF_INIT_RETRY_COUNT
    REPORT_INT(ADAPTER_EIP163_MAX_NOF_INIT_RETRY_COUNT);
#endif

#ifdef ADAPTER_EIP163_INIT_RETRY_TIMEOUT_MS
    REPORT_INT(ADAPTER_EIP163_INIT_RETRY_TIMEOUT_MS);
#endif

#ifdef ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT
    REPORT_INT(ADAPTER_EIP163_MAX_NOF_SYNC_RETRY_COUNT);
#endif

#ifdef ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS
    REPORT_INT(ADAPTER_EIP163_SYNC_RETRY_TIMEOUT_MS);
#endif

#ifdef ADAPTER_EIP163_CLOCK_CONTROL
    REPORT_INT(ADAPTER_EIP163_CLOCK_CONTROL);
#endif

#ifdef ADAPTER_EIP163_LOW_LATENCY_BYPASS
    REPORT_INT(ADAPTER_EIP163_LOW_LATENCY_BYPASS);
#endif

#ifdef ADAPTER_EIP163_CLEAR_ON_READ
    REPORT_INT(ADAPTER_EIP163_CLEAR_ON_READ);
#endif

#ifdef ADAPTER_EIP163_MAX_NOF_DEVICES
    REPORT_INT(ADAPTER_EIP163_MAX_NOF_DEVICES);
#endif

#ifdef ADAPTER_EIP163_MAX_IC_COUNT
    REPORT_INT(ADAPTER_EIP163_MAX_IC_COUNT);
#endif

#ifdef ADAPTER_EIP163_INTERRUPTS_ENABLE
    REPORT_EXPL(ADAPTER_EIP163_INTERRUPTS_ENABLE,
                " is SET => Interrupts ENABLED");
#else
    REPORT_EXPL(ADAPTER_EIP163_INTERRUPTS_ENABLE,
                " is NOT set => Interrupts DISABLED");
#endif

#ifdef ADAPTER_EIP163_INTERRUPTS_TRACEFILTER
    REPORT_HEX32(ADAPTER_EIP163_INTERRUPTS_TRACEFILTER);
#endif

#ifdef ADAPTER_EIP163_MODE_EGRESS
    REPORT_SET(ADAPTER_EIP163_MODE_EGRESS);
    /*REPORT_STR(ADAPTER_EIP163_DEVICE_NAME_EGRESS); */
#endif

#ifdef ADAPTER_EIP163_MODE_INGRESS
    REPORT_SET(ADAPTER_EIP163_MODE_INGRESS);
    /*REPORT_STR(ADAPTER_EIP163_DEVICE_NAME_INGRESS); */
#endif
#endif /* ADAPTER_EIP163_DRIVER_NAME */

    /* Log */
    LOG_INFO("Logging:\n");

#if (LOG_SEVERITY_MAX == LOG_SEVERITY_INFO)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_INFO);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_WARNING)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_WARNING);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_CRITICAL)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_CRITICAL);
#else
    REPORT_EXPL(LOG_SEVERITY_MAX, " - Unknown (not info/warn/crit)");
#endif

    IDENTIFIER_NOT_USED(dummy);

#endif /* LOG_INFO_ENABLED */
}


/* end of file adapter_init.c */

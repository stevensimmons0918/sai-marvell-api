/* adapter_cfye_stats.c
 *
 * CfyE API statistics  implementation
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 2.5                                                      */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2020-Feb-27                                              */
/*                                                                            */
/* Copyright (c) 2008-2020 INSIDE Secure B.V. All Rights Reserved             */
/*                                                                            */
/* This confidential and proprietary software may be used only as authorized  */
/* by a licensing agreement from INSIDE Secure.                               */
/*                                                                            */
/* The entire notice above must be reproduced on all authorized copies that   */
/* may only be made to the extent permitted by a licensing agreement from     */
/* INSIDE Secure.                                                             */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://customersupport.insidesecure.com.                                  */
/* In case you do not have an account for this system, please send an e-mail  */
/* to ESSEmbeddedHW-Support@insidesecure.com.                                 */
/* -------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

/* Internal adapter API */
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Logging API */
#include <Kit/Log/incl/log.h>                    /* LOG_* */
/* Driver Framework Run-Time C Library Abstraction API */
#include <Kit/DriverFramework/incl/clib.h>       /* ZEROINIT() */

/*----------------------------------------------------------------------------
 * CfyE_Statistics_TCAM_Get
 */
CfyE_Status_t
CfyE_Statistics_TCAM_Get(
        const unsigned int DeviceId,
        const unsigned int StatIndex,
        CfyE_Statistics_TCAM_t * const Stat_p,
        const bool fSync)
{
    EIP163_Error_t EIP163_Rc;
    CfyE_Status_t Rc;
    EIP163_Statistics_TCAM_t DevStat;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Stat_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        Rc = CfyELib_Device_Sync(DeviceId);
        if (Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, Rc);
            CfyELib_Device_Unlock(DeviceId);
            return Rc;
        }
    }

    ZEROINIT(DevStat);

    LOG_INFO("\n\t EIP163_Statistics_Read \n");

    EIP163_Rc = EIP163_Statistics_TCAM_Get(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                     StatIndex,
                                     &DevStat,
                                     ADAPTER_EIP163_WRITE_TO_DECREMENT);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read EIP-163 device %d (%s) statistics, "
                 "error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                     EIP163_Rc);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(EIP163_Rc);
    }

    Stat_p->Counter.Lo = DevStat.Counter.low;
    Stat_p->Counter.Hi = DevStat.Counter.hi;

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*-----------------------------------------------------------------------------
 * CfyE_Statistics_Channel_Get
 */
CfyE_Status_t
CfyE_Statistics_Channel_Get(
        const unsigned int DeviceId,
        const unsigned int StatIndex,
        CfyE_Statistics_Channel_t * const Stat_p,
        const bool fSync)
{
    EIP163_Error_t EIP163_Rc;
    CfyE_Status_t Rc;
    EIP163_Statistics_Channel_t DevStat;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Stat_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    /* Synchronize with the device if required */
    if (fSync)
    {
        Rc = CfyELib_Device_Sync(DeviceId);
        if (Rc != CFYE_STATUS_OK)
        {
            LOG_CRIT("%s: synchronization failed for device %d, error %d\n",
                     __func__, DeviceId, Rc);
            CfyELib_Device_Unlock(DeviceId);
            return Rc;
        }
    }

    ZEROINIT(DevStat);

    LOG_INFO("\n\t EIP163_Statistics_Channel_Read \n");

    EIP163_Rc = EIP163_Statistics_Channel_Get(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                        StatIndex,
                                        &DevStat,
                                        ADAPTER_EIP163_WRITE_TO_DECREMENT);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read EIP-163 device %d (%s) channel statistics, "
                 "error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 EIP163_Rc);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(EIP163_Rc);
    }

    Stat_p->TCAMHitMultiple.Lo = DevStat.TCAMHitMultiple.low;
    Stat_p->TCAMHitMultiple.Hi = DevStat.TCAMHitMultiple.hi;
    Stat_p->HeaderParserDroppedPkts.Lo = DevStat.HeaderParserDroppedPkts.low;
    Stat_p->HeaderParserDroppedPkts.Hi = DevStat.HeaderParserDroppedPkts.hi;
    Stat_p->TCAMMiss.Lo = DevStat.TCAMMiss.low;
    Stat_p->TCAMMiss.Hi = DevStat.TCAMMiss.hi;
    Stat_p->PktsCtrl.Lo = DevStat.PktsCtrl.low;
    Stat_p->PktsCtrl.Hi = DevStat.PktsCtrl.hi;
    Stat_p->PktsData.Lo = DevStat.PktsData.low;
    Stat_p->PktsData.Hi = DevStat.PktsData.hi;
    Stat_p->PktsDropped.Lo = DevStat.PktsDropped.low;
    Stat_p->PktsDropped.Hi = DevStat.PktsDropped.hi;
    Stat_p->PktsErrIn.Lo = DevStat.PktsErrIn.low;
    Stat_p->PktsErrIn.Hi = DevStat.PktsErrIn.hi;

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/*----------------------------------------------------------------------------
 * CfyE_Statistics_Summary_TCAM_Read
 */
CfyE_Status_t
CfyE_Statistics_Summary_TCAM_Read(
        const unsigned int DeviceId,
        const unsigned int StartOffset,
        uint32_t * const Summary_p,
        const unsigned int Count)
{
    EIP163_Error_t EIP163_Rc;
    unsigned int i;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(Summary_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP163_Statistics_Summary_Read \n");

    EIP163_Rc = EIP163_Statistics_Summary_TCAM_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                               StartOffset,
                                               Summary_p,
                                               Count);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read EIP-163 device %d (%s) statistics summary, "
                 "error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 EIP163_Rc);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(EIP163_Rc);
    }

    /* Clear statistics summary if not 0 */
    for (i = 0; i < Count; i++)
        if (Summary_p[i] != 0)
            EIP163_Statistics_Summary_TCAM_Write(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                 StartOffset + i,
                                                 &Summary_p[i],
                                                 1);

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;

}


/*----------------------------------------------------------------------------
 * CfyE_Statistics_Summary_Channel_Read
 */
CfyE_Status_t
CfyE_Statistics_Summary_Channel_Read(
        const unsigned int DeviceId,
        CfyE_Ch_Mask_t * const ChSummary_p)
{
    EIP163_Error_t EIP163_Rc;

    LOG_INFO("\n %s \n", __func__);

    ADAPTER_EIP163_CHECK_INT_ATMOST(DeviceId, ADAPTER_EIP163_MAX_NOF_DEVICES - 1);
    ADAPTER_EIP163_CHECK_POINTER(ChSummary_p);

    CfyELib_Device_Lock(DeviceId);

    if (!CfyELib_Initialized_Check(DeviceId, __func__, true))
    {
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_ERROR_INTERNAL;
    }

    LOG_INFO("\n\t EIP163_Statistics_Summary_Channel_Read \n");

    EIP163_Rc = EIP163_Statistics_Summary_Channel_Read(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                       (EIP163_Ch_Mask_t * const) ChSummary_p);
    if (EIP163_Rc != EIP163_NO_ERROR)
    {
        LOG_CRIT("%s: Failed to read EIP-163 device %d (%s) statistics summary, "
                 "error %d\n",
                 __func__,
                 DeviceId,
                 ADAPTER_EIP163_DEVICE_NAME(DeviceId),
                 EIP163_Rc);
        CfyELib_Device_Unlock(DeviceId);
        return CFYE_DLIB_ERROR(EIP163_Rc);
    }

    /* Clear the summary registers in the device. */
    if (ChSummary_p->ch_bitmask[0] != 0)
    {
        EIP163_Statistics_Summary_Channel_Write(&PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Devices)[DeviceId]->IOArea,
                                                (const EIP163_Ch_Mask_t * const) ChSummary_p);
    }

    CfyELib_Device_Unlock(DeviceId);

    LOG_INFO("\n %s done \n", __func__);

    return CFYE_STATUS_OK;
}


/* end of file adapter_cfye_stats.c */

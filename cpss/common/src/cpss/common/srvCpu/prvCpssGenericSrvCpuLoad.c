/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
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
* @file prvCpssGenericSrvCpuLoad.c
*
* @brief Service CPU firmware load/start APIs
*
* @version   1
********************************************************************************
*/
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>
#include <cpss/generic/ipc/mvShmIpc.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuIpcDevCfg.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifndef ASIC_SIMULATION
PRV_SRVCPU_IPC_CHANNEL_STC* prvIpcDevCfg[PRV_CPSS_MAX_PP_DEVICES_CNS];

struct PRV_CPSS_SRVCPU_TYPES_STCT {
    GT_U16      devId;
    GT_U16      devIdMask;
    PRV_SRVCPU_OPS_FUNC ops;
} prvSrvCpuDescr[] = {
    { 0xfc00, 0xff00, &prvSrvCpuOpsMsys }, /* bobcat2 */
    { 0xbe00, 0xff00, &prvSrvCpuOpsBobK }, /* cetus */
    { 0xbc00, 0xff00, &prvSrvCpuOpsBobK }, /* caelum */
    { 0xf400, 0xfe00, &prvSrvCpuOpsMsys }, /* alleycat3\s */
    { 0xc800, 0xff00, &prvSrvCpuOpsCM3  }, /* aldrin */
    { 0xd400, 0xff00, &prvSrvCpuOpsCM3  }, /* bc3 */
    { 0xc400, 0xff00, &prvSrvCpuOpsCM3  }, /* pipe */
    { 0xcc00, 0xff00, &prvSrvCpuOpsCM3  }, /* aldrin2 */
    { 0x8600, 0xfc00, &prvSrvCpuOpsCM3  }, /* Falcon - 12.8/6.4 : 0x86xx or 0x84xx (note the mask 0xfc00)  */
    { 0xb400, 0xff00, &prvSrvCpuOpsCM3_AC5  }, /* AC5 */
    { 0xffff, 0xffff, NULL }
};

PRV_SRVCPU_OPS_FUNC prvSrvCpuGetOps(
    IN  GT_U8   devNum
)
{
    GT_U32 i;
    GT_U16 deviceId; /* PCI device Id */

    i = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devType;
    deviceId = (i >> 16) & 0xffff;

    for (i = 0; prvSrvCpuDescr[i].ops; i++)
    {
        if ((deviceId & prvSrvCpuDescr[i].devIdMask) == (prvSrvCpuDescr[i].devId & prvSrvCpuDescr[i].devIdMask))
            break;
    }

    if(prvSrvCpuDescr[i].ops == NULL)
    {
        CPSS_LOG_ERROR_MAC("The deviceId[0x%4.4x] not defined as 'supporting' Service CPU (see prvSrvCpuDescr[])",
            deviceId);
    }
    return prvSrvCpuDescr[i].ops;
}
#else
/* next 2 functions needed for 'ASIC_SIMULATION_ENV_FORBIDDEN' that those functions needed by linker by 'EXTHWG_POE_CM3_Ipc_driverInit' */
PRV_SRVCPU_OPS_FUNC prvSrvCpuGetOps(
    IN  GT_U8   devNum
)
{
    (void)devNum;
    CPSS_LOG_ERROR_MAC("prvSrvCpuGetOps : not implemented for WM");
    return NULL;
}
GT_VOID prvIpcAccessInit(PRV_SRVCPU_IPC_CHANNEL_STC *s, GT_BOOL init)
{
    (void)s;
    (void)init;
    CPSS_LOG_ERROR_MAC("prvIpcAccessInit : not implemented for WM");
}
#endif

#ifndef ASIC_SIMULATION

#define REMOTE_UPGRADE_HEADER_SIZE 0x0C /* 12  */
#define REMOTE_UPGRADE_MSG_MIN_LEN 0x4  /* 4  */
#define REMOTE_UPGRADE_MSG_MAX_LEN 0xF8 /* 248 */

/* defined in boot_channel.h */
typedef enum {  MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_NOR_FLASH_NOT_EXIST_E   = 0x012,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_PRIMARY_FW_HEADER_TYPE_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_CURR_PRIMARY_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_CURR_SECONDARY_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_NEW_FW_INVALID_CRC_E,
                MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_INVALID_CHECKSUM_E
} MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_ENT;

typedef struct { /* remote upgrade message header */
/*  type        name              byte order */
    GT_U16      opCode;       /*   0  - 1    */
    GT_U16      bmp;          /*   2  - 3    */
    GT_U32      bufNum;       /*   4  - 7    */
    GT_U16      bufSize;      /*   8  - 9    */
    GT_U8       checkSum;     /*   10        */
    GT_U8       reseved;      /*   11        */
} upgradeMsgHdr;

/* 8 bit checksum */
static GT_U8 prvChecksum8(GT_U8* start, GT_U32 len, GT_U8 csum)
{
    GT_U8  sum = csum;
    GT_U8* startp = start;

    do {
        sum += *startp;
        startp++;

    } while(--len);

    return (sum);
}
#endif

/**
* @internal prvCpssGenericSrvCpuInitCust function
* @endinternal
*
* @brief   Upload firmware to service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - Device Number
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] fwId                  - firmware identifier string
*                                    "AP_BobK", "AP_Bobcat3", "DBA_xxxx"
* @param[in] ipcConfig             - IPC configuration, optional
*                                    should end with channel==0xff
* @param[in] preStartCb            - A custom routine executed before unreset
*                                    service CPU. (optional can be NULL)
* @param[in] preStartCookie        - parameter for preStartCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssGenericSrvCpuInitCust
(
    IN  GT_U8       devNum,
    IN  GT_U32      scpuId,
    IN  const char *fwId,
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)fwId;
    (void)ipcConfig;
    (void)preStartCb;
    (void)preStartCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    PRV_SRVCPU_OPS_FUNC opsFunc;
    PRV_CPSS_FIRMWARE_IMAGE_FILE_STC *f;
    PRV_SRVCPU_IPC_CHANNEL_STC *s;
    GT_U32    p;
    GT_STATUS rc;
    GT_U32 data = 0x1;
    GT_BOOL tamFalcon = GT_FALSE;
    GT_BOOL flowManagerFalcon = GT_FALSE;
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;
    if (cpssOsStrCmp(fwId,"TAM_Falcon" ) == 0)
    {
        tamFalcon = GT_TRUE;
    }
    if (cpssOsStrCmp(fwId,"IPFIX_MANAGER_FALCON" ) == 0)
    {
        /* eagle is the target*/
        if ((scpuId >= 16) && (scpuId <= 19))
        {
            flowManagerFalcon = GT_TRUE;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
        }
    }
    hwInfo = &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    if (drv == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    opsFunc = prvSrvCpuGetOps(devNum);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_EXISTS_E, NULL);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    f = prvCpssSrvCpuFirmareOpen(fwId);
    if (f == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_RESET_E, NULL);
    if (rc != GT_OK)
    {
        prvCpssSrvCpuFirmareClose(f);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_PRE_LOAD_E, &s);
    if (rc != GT_OK)
    {
        prvCpssSrvCpuFirmareClose(f);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if ((tamFalcon == GT_TRUE) || (flowManagerFalcon == GT_TRUE))
    {
        drv->writeMask(drv, CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E, 0x1d000124, &data, 0x1, 0xffffffff);
    }
    for (p = 0; p < f->fwSize;)
    {
        GT_U32 buf[128];
        GT_U32 k;
        k = prvCpssSrvCpuFirmareRead(f, buf, sizeof(buf));
        if (k == 0xffffffff)
            break;
        s->drv->writeMask(s->drv, 0, f->fwLoadAddr + p, buf, k / 4, 0xffffffff);
        p += k;
    }

    if (p < f->fwSize)
    {
        /* failure reading firmware */
        prvCpssSrvCpuFirmareClose(f);
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    s->shmAddr = f->ipcShmAddr;
    s->shmSize = f->ipcShmSize;
    prvCpssSrvCpuFirmareClose(f);

    if (ipcConfig != NULL)
    {
        /* setup ipc... */
        prvIpcAccessInit(s, GT_TRUE);
        for (; ipcConfig->channel != 0xff; ipcConfig++)
        {
            shmIpcConfigChannel(&(s->shm), ipcConfig->channel,
                    ipcConfig->maxrx, ipcConfig->rxsize,
                    ipcConfig->maxtx, ipcConfig->txsize);
        }
        /* create fwChannel object */

        /* Write magick to reserved area */
        shmResvdAreaRegWrite(&(s->shm), s->shmResvdRegs, 0, IPC_RESVD_MAGIC);
        /* Write pointer to SHM registers to reserved area */
        shmResvdAreaRegWrite(&(s->shm), s->shmResvdRegs, 1, s->shmAddr + 0x100);
    }
    if (preStartCb != NULL)
        preStartCb((GT_UINTPTR)s, preStartCookie);
    if (fwChannelPtr != NULL)
    {
        *fwChannelPtr = (GT_UINTPTR)s;
    }
    else
    {
        prvIpcAccessDestroy(s);
    }

    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_UNRESET_E, NULL);
    return rc;
#endif
}

/**
* @internal prvCpssGenericSrvCpuInit function
* @endinternal
*
* @brief   Upload firmware to service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - device number.
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] fwId                  - firmware identifier string
*                                    "AP_BobK", "AP_Bobcat3", "DBA_xxxx"
* @param[in] ipcConfig             - IPC configuration, optional
*                                    should end with channel==0xff
* @param[in] preStartCb            - A custom routine executed before unreset
*                                    service CPU. (optional can be NULL)
* @param[in] preStartCookie        - parameter for preStartCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retval GT_ALREADY_EXIST         - Already initialized for given devNum
*/
GT_STATUS prvCpssGenericSrvCpuInit
(
    IN  GT_U8       devNum,
    IN  GT_U8       scpuId,
    IN  const char *fwId,
    IN  PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC *ipcConfig,
    IN  GT_STATUS (*preStartCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR  preStartCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
#ifndef ASIC_SIMULATION
    GT_U32 index;
    GT_STATUS rc;

    index = SRVCPU_IDX_GET_MAC(scpuId);
    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) || (index >= SRVCPU_MAX_IPC_CHANNEL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

   /* if (prvIpcDevCfg[devNum] != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);*/

    rc = prvCpssGenericSrvCpuInitCust(devNum, scpuId, fwId, ipcConfig, preStartCb, preStartCookie, fwChannelPtr);

    /* store fwChannel */
    if (fwChannelPtr != NULL && (index < SRVCPU_MAX_IPC_CHANNEL))
    {
       /* if (HWS_DEV_SILICON_TYPE(devNum) == Falcon)
            prvIpcDevCfg[scpuId] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
        else*/
         /*   prvIpcDevCfg[devNum] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);*/
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[index] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
    }

    return rc;
#else
    (void)devNum;
    (void)scpuId;
    (void)fwId;
    (void)ipcConfig;
    (void)preStartCb;
    (void)preStartCookie;
    (void)fwChannelPtr;
    return GT_OK;
#endif
}

/**
* @internal prvCpssGenericSrvCpuProbeCust function
* @endinternal
*
* @brief   Probe Service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - Device Number
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] hasIPC                - GT_TRUE if IPC should be probed on this
*                                    serviceCPU
* @param[in] checkIpcCb            - A custom routine executed when IPC structure
*                                    detected. Could be used for check transaction
*                                    Returns GT_TRUE if success
* @param[in] checkIpcCookie        - parameter for checkIpcCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retvel GT_NOT_INITIALIZED       - not initialized. FW chould be reloaded
*/
GT_STATUS prvCpssGenericSrvCpuProbeCust
(
    IN  GT_U8      devNum,
    IN  GT_U32      scpuId,
    IN  GT_BOOL     hasIPC,
    IN  GT_BOOL (*checkIpcCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR checkIpcCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{
#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)hasIPC;
    (void)checkIpcCb;
    (void)checkIpcCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    PRV_SRVCPU_OPS_FUNC opsFunc;
    GT_BOOL isOn;
    GT_STATUS rc;
    PRV_SRVCPU_IPC_CHANNEL_STC *s;
    CPSS_HW_INFO_STC *hwInfo;
    CPSS_HW_DRIVER_STC *drv;
    CPSS_SYSTEM_RECOVERY_INFO_STC currentSystemRecoveryInfo;
    CPSS_SYSTEM_RECOVERY_INFO_STC newSystemRecoveryInfo;

    if (fwChannelPtr != NULL)
        *fwChannelPtr = 0;

    hwInfo = &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0]);
    drv = prvCpssDrvHwPpPortGroupGetDrv(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS);
    opsFunc = prvSrvCpuGetOps(devNum);
    if (opsFunc == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_EXISTS_E, NULL);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_IS_ON_E, &isOn);
    if (rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    if (isOn != GT_TRUE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    rc = opsFunc(hwInfo, drv, scpuId, PRV_SRVCPU_OPS_PRE_CHECK_IPC_E, &s);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    if (hasIPC == GT_FALSE)
    {
        /* No IPC check, return OK */
        if (fwChannelPtr != NULL)
        {
            *fwChannelPtr = (GT_UINTPTR)s;
        }
        else
        {
            prvIpcAccessDestroy(s);
        }
        return GT_OK;
    }
    s->shmSize = 0x800; /* handled later */
    prvIpcAccessInit(s, GT_FALSE);
    /* Check magick in reserved area */
    if (shmResvdAreaRegRead(&(s->shm), s->shmResvdRegs, 0) != IPC_RESVD_MAGIC)
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    s->shmAddr = shmResvdAreaRegRead(&(s->shm), s->shmResvdRegs, 1) - 0x100;
    if (s->shmAddr < s->targetBase || s->shmAddr >= s->targetBase+(s->size-4))
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    s->shm.shm = s->shmAddr;
    s->shm.shmLen = s->shmSize;
    /* check if ipc link was initialized */
    if ( (shmIpcMagicGet(&(s->shm)) != IPC_SHM_MAGIC)  &&
         (shmIpcMagicGet(&(s->shm)) != IPC_SHM_MAGIC+1) )
    {
        prvIpcAccessDestroy(s);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }
    /* fix mirror buffer */
    s->shm.shmLen = shmIpcSizeGet(&(s->shm));
    if (s->shmSize < s->shm.shmLen && s->base == 0)
    {
        s->shmSize = s->shm.shmLen;
    }

    /* check ipc transaction */
    if (checkIpcCb != NULL)
    {
        rc = cpssSystemRecoveryStateGet(&currentSystemRecoveryInfo);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        newSystemRecoveryInfo = currentSystemRecoveryInfo;
        newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
        if (checkIpcCb((GT_UINTPTR)s, checkIpcCookie) != GT_TRUE)
        {
            prvIpcAccessDestroy(s);
            prvCpssSystemRecoveryStateUpdate(&currentSystemRecoveryInfo);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
        prvCpssSystemRecoveryStateUpdate(&currentSystemRecoveryInfo);
    }
    if (fwChannelPtr != NULL)
    {
        *fwChannelPtr = (GT_UINTPTR)s;
    }
    else
    {
        prvIpcAccessDestroy(s);
    }

    return GT_OK;
#endif
}


/**
* @internal prvCpssGenericSrvCpuProbe function
* @endinternal
*
* @brief   Probe Service CPU
*          Start it, initialize IPC
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; PIPE;
* @note   NOT APPLICABLE DEVICES:  Lion2;
*
* @param[in] devNum                - device number.
* @param[in] scpuId                - service CPU id:
*                                    0 - CM3_0
*                                    1 - CM3_1
*                                    ...
*                                    0xffffffff - MSYS
* @param[in] hasIPC                - GT_TRUE if IPC should be probed on this
*                                    serviceCPU
* @param[in] checkIpcCb            - A custom routine executed when IPC structure
*                                    detected. Could be used for check transaction
*                                    Returns GT_TRUE if success
* @param[in] checkIpcCookie        - parameter for checkIpcCb
* @param[out] fwChannelPtr         - object ID for IPC operations with this
*                                    service CPU. If this pointer is NULL, then
*                                    access object destroyed before return
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
* @retvel GT_NOT_INITIALIZED       - not initialized. FW chould be reloaded
* @retval GT_ALREADY_EXIST         - Already initialized for given devNum
*/
GT_STATUS prvCpssGenericSrvCpuProbe
(
    IN  GT_U8       devNum,
    IN  GT_U8       scpuId,
    IN  GT_BOOL     hasIPC,
    IN  GT_BOOL (*checkIpcCb)(GT_UINTPTR  fwChannel, GT_UINTPTR  cookie),
    IN  GT_UINTPTR checkIpcCookie,
    OUT GT_UINTPTR *fwChannelPtr
)
{

#ifdef ASIC_SIMULATION
    (void)devNum;
    (void)scpuId;
    (void)hasIPC;
    (void)checkIpcCb;
    (void)checkIpcCookie;
    (void)fwChannelPtr;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#else
    GT_STATUS rc;
    GT_U32 index;

    index = SRVCPU_IDX_GET_MAC(scpuId);
    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) || (index >= SRVCPU_MAX_IPC_CHANNEL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

   /* if (prvIpcDevCfg[devNum] != NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);*/

    rc = prvCpssGenericSrvCpuProbeCust(devNum, scpuId,
            hasIPC, checkIpcCb, checkIpcCookie, fwChannelPtr);

    /* store fwChannel */
    if (rc == GT_OK)
    {
        if (fwChannelPtr != NULL  && (index < SRVCPU_MAX_IPC_CHANNEL))
        {
           /* prvIpcDevCfg[devNum] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);*/
             PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[index] = (PRV_SRVCPU_IPC_CHANNEL_STC*)(*fwChannelPtr);
        }
    }
    return rc;

#endif
}





/**
* @internal prvCpssDbaToSrvCpuLoad function
* @endinternal
*
* @brief   Upload DBA FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssDbaToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    return prvCpssGenericSrvCpuInit(devNum, 1, "DBA_Bobcat3", NULL, NULL, 0, NULL);
#else
    (void)devNum;
    return GT_OK;
#endif
}

/**
* @internal prvCpssTamToSrvCpuLoad function
* @endinternal
*
* @brief   Upload TAM FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssTamToSrvCpuLoad
(
    IN GT_U8    devNum
)
{
#ifndef ASIC_SIMULATION
    GT_U8 scpuId;
    const char *fwId;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        scpuId = 16;
        fwId = "TAM_Falcon";
    }
    else
    {
        fwId = "TAM_Bobcat3";
        scpuId =1;
    }
    return prvCpssGenericSrvCpuInit(devNum,scpuId , fwId, NULL, NULL, 0, NULL);
#else
    (void)devNum;
    return GT_OK;
#endif
}

/**
* @internal prvCpssFlowManagerToSrvCpuLoad function
* @endinternal
*
* @brief   Upload Flow Manager FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Aldrin2;
* @note   NOT APPLICABLE DEVICES:  AC3; Bobcat2; Caelum; Cetus; Aldrin;
*
* @param[in] devNum                - device number.
* @param[in] sCpuId                - service CPU ID.
* @param[out] fwChannel            - IPC channel.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssFlowManagerToSrvCpuLoad
(
    IN  GT_U8      devNum,
    IN  GT_U32     sCpuId,
    OUT GT_UINTPTR *fwChannel
)
{
#ifndef ASIC_SIMULATION
    PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
            { 1,   2, 200, 2, 200 },
            { 0xff,0,  0, 0,  0 }
    };
    return prvCpssGenericSrvCpuInit(devNum, sCpuId, "FLOWMNG_Aldrin2", ipcCfg, NULL, 0, fwChannel);
#else
    (void)devNum;
    (void)sCpuId;
    (void)fwChannel;
    return GT_OK;
#endif
}

/**
* @internal prvCpssIpfixManagerToSrvCpuLoad function
* @endinternal
*
* @brief   Upload Flow Manager FW to second service CPU
*
* @note   APPLICABLE DEVICES:      Falcon;
*
* @param[in] devNum                - device number.
* @param[in] sCpuId                - service CPU ID.
* @param[out] fwChannel            - IPC channel.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_SUPPORTED         - no service CPU on this PP,
* @retval GT_NO_SUCH               - scpuId not applicable for this PP
*                                    or firmare not found
* @retval GT_FAIL                  - on error.
*/
GT_STATUS prvCpssIpfixManagerToSrvCpuLoad
(
    IN  GT_U8      devNum,
    IN  GT_U32     sCpuId,
    OUT GT_UINTPTR *fwChannel
)
{
#ifndef ASIC_SIMULATION
    PRV_CPSS_GENERIC_SRV_CPU_IPC_CHANNEL_CONFIG_STC ipcCfg[] = {
            { 1,    32, 44,256, 44 },
            { 0xff,  0,  0,  0,  0 }
    };
    /* Currently UART console is not working for other Falcon Eagle CM3
     * instances #17-19 so for now going ahead with only CM3#16 TBD.
     */
    if( sCpuId > 19 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
    }
    return prvCpssGenericSrvCpuInit(devNum, sCpuId, "IPFIX_MANAGER_FALCON", ipcCfg, NULL, 0, fwChannel);
#else
    (void)devNum;
    (void)sCpuId;
    (void)fwChannel;
    return GT_OK;
#endif
}

/*******************************************************************************
* prvCpssGenericSrvCpuRemove
*
* DESCRIPTION:
*       Cleanup service CPU resources while cpssDxChCfgDevRemove
*
* APPLICABLE DEVICES:
*       AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Pipe.
*
* NOT APPLICABLE DEVICES:
*       Lion2.
*
* INPUTS:
*       devNum                   - device number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success.
*       GT_FAIL                  - on error.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssGenericSrvCpuRemove
(
    IN GT_U8    devNum,
    IN GT_U8    scpuId
)
{
#ifndef ASIC_SIMULATION
    GT_U8 cpuIdx;
    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) ||
        (scpuId >= SRVCPU_MAX_IPC_CHANNEL && scpuId != SRVCPU_IPC_CHANNEL_ALL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    if (scpuId == SRVCPU_IPC_CHANNEL_ALL)
    {
        for (cpuIdx = 0; cpuIdx < SRVCPU_MAX_IPC_CHANNEL; cpuIdx++) {
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]!=NULL&&PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[cpuIdx] != NULL)
            {
                prvIpcAccessDestroy(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[cpuIdx]);
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[cpuIdx] = NULL;
            }
        }
    }
    else if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]!=NULL&&PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[scpuId] != NULL)
    {
        prvIpcAccessDestroy(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[SRVCPU_IDX_GET_MAC(scpuId)]);
       /* prvIpcAccessDestroy(prvIpcDevCfg[devNum]);
        prvIpcDevCfg[devNum] = NULL;*/
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[SRVCPU_IDX_GET_MAC(scpuId)] = NULL;

    }
#else /* defined(ASIC_SIMULATION) */
    (void)devNum;
    (void)scpuId;
#endif
    return GT_OK;
}

/**
* @internal CpssGenericSrvCpuFWUpgrade function
* @endinternal
*
* @brief   Send buffer with FW upgrade to service CPU,
*         to be programmed on flash, using boot channel.
*         buffer size is limited to 240 bytes.
*
* @note   APPLICABLE DEVICES:      AC3; Bobcat2; Caelum; Cetus; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number,
* @param[in] bufBmp                   - bitmap of the FW:
*                                      0x01 - primary SuperImage
*                                      0x02 - secondary SuperImage,
*                                      0x03 - both.
* @param[in] bufNum                   - block number,
* @param[in] bufSize                  - buffer size:
*                                      must be: 32B < fwBufSize < 240B,
*                                      last bufSize=bufSize|0x100)
* @param[in] bufPtr                   - FW data ptr,
*
* @param[out] status                   - microInit boot channel retCode.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_BAD_PARAM             - wrong devNum or flowId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS CpssGenericSrvCpuFWUpgrade
(
    IN  GT_U8   devNum,
    IN  GT_U32  bufBmp,
    IN  GT_U32  bufNum,
    IN  GT_U32  bufSize,
    IN  GT_U8  *bufPtr,
    IN  GT_U8   scpuId,
    OUT GT_U32 *status
)
{

#ifndef ASIC_SIMULATION
    PRV_SRVCPU_IPC_CHANNEL_STC      *s;
    GT_32                           rc = 0, bc_rc = 0;
    GT_U32                          i, j, msgSize;
    GT_U32                          maxDelay;
    GT_U32                          bufWithHeader[256];
    GT_U32                          dummyBuf;
    upgradeMsgHdr                   *msgHeader = NULL;

    cpssOsPrintf("bufNum=%d bufSize=0x%X\n",bufNum, bufSize );

    msgSize = bufSize & 0xFF;
    if( (msgSize<REMOTE_UPGRADE_MSG_MIN_LEN) || (msgSize>REMOTE_UPGRADE_MSG_MAX_LEN) ) {
        cpssOsPrintf("buffer size must be %d < X < %d \n", REMOTE_UPGRADE_MSG_MIN_LEN, REMOTE_UPGRADE_MSG_MAX_LEN);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) || (scpuId >= SRVCPU_MAX_IPC_CHANNEL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    /*s = prvIpcDevCfg[devNum];*/
    s = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->ipcDevCfg[scpuId];
    if (s == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);

    /* prepare command parameters */
    msgHeader = (upgradeMsgHdr *)&bufWithHeader[0];
    msgHeader->opCode  = 0x10;          /* remote upgrade opCode */
    msgHeader->bmp     = bufBmp & 0xFF;
    msgHeader->bufNum  = bufNum;
    msgHeader->bufSize = (GT_U16)bufSize;
    msgHeader->checkSum = 0;
    msgHeader->reseved  = 0;
    cpssOsMemCpy( (msgHeader + 1), bufPtr, msgSize );

    msgSize = msgSize + REMOTE_UPGRADE_HEADER_SIZE;
    /* calculate checksum and update packet header */
    msgHeader->checkSum = prvChecksum8((GT_U8*)msgHeader,msgSize,0);

    i = 0;
    /* wait reply status about 2 sec for regular packets
         and 30 sec for finalize procedure of bmp #3 (crc validation and copy image to other offset) */
    if ((bufSize | 0x100) && (bufBmp ==3)) maxDelay = 30000;
    else maxDelay = 2000;

    for ( j=0 ; j < 3; j++ ) { /* 3 retries for packet with wrong checkSum */
        if (GT_OK != (rc = shmIpcBootChnSend(&(s->shm), 0, msgHeader, msgSize) ) ){
            if (j >= 3){
                cpssOsPrintf("shmIpcBootChnSend failed %d\n", rc);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            continue;
        }

        do
        {
            cpssOsTimerWkAfter(1);

            i++;
            /* for each boot channel send operation, feedback receive is necessary.
               if not expected any valuable feedback data - dummy buffer must be used */
            rc=shmIpcBootChnRecv(&s->shm, &bc_rc, &dummyBuf, 4);
            if (rc > 0)
                break;
            if (i > maxDelay)
            {
                cpssOsPrintf("\n\nTimeOut error\n\n");
                *status = bc_rc;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        } while(i <= maxDelay);

        /* only in case of invalid checksum - send message again */
        if( bc_rc != MICRO_INIT_BOOT_CH_FW_REMOTE_UPGRADE_ERR_INVALID_CHECKSUM_E )
            break;
    }
    *status = bc_rc;

    return (rc>0) ? bc_rc : GT_FAIL;


#else /* defined(ASIC_SIMULATION) */

    if (bufPtr) {
        devNum  = devNum;
        bufBmp  = bufBmp;
        bufNum  = bufNum;
        bufSize = bufSize;
        scpuId  = scpuId;
    };
    *status = 4;
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
#endif /* defined(ASIC_SIMULATION) */
}




















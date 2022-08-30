/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssHwDriverSip5SlaveSMI.c
*
* @brief Slave SMI driver for SIP5 devices
*
* @version   1
*
* IGNORE_CPSS_LOG_RETURN_SCRIPT
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef struct CPSS_HW_DRIVER_SIP5_SSMI_STCT {
    CPSS_HW_DRIVER_STC  common;
    CPSS_OS_MUTEX       mtx;
    GT_U32              phy;
    GT_U32              compl[16];
    int                *asMap;
} CPSS_HW_DRIVER_SIP5_SSMI_STC;


#define SMI_WRITE_ADDRESS_MSB_REGISTER    (0x00)
#define SMI_WRITE_ADDRESS_LSB_REGISTER    (0x01)
#define SMI_WRITE_DATA_MSB_REGISTER       (0x08)
#define SMI_WRITE_DATA_LSB_REGISTER       (0x09)

#define SMI_READ_ADDRESS_MSB_REGISTER     (0x04)
#define SMI_READ_ADDRESS_LSB_REGISTER     (0x05)
#define SMI_READ_DATA_MSB_REGISTER        (0x06)
#define SMI_READ_DATA_LSB_REGISTER        (0x07)

#define SMI_STATUS_REGISTER               (0x1f)
#define SMI_STATUS_WRITE_DONE             (0x02)
#define SMI_STATUS_READ_READY             (0x01)


#define SMI_WAIT_FOR_STATUS_DONE
#define SMI_TIMEOUT_COUNTER  10000
#define SMI_INVALID_DATA     0xffff

#define SSMI_XBAR_PORT_REMAP(_port) (0x13100+(_port)*4)


#define SMI drv->common.parent

/* In Pipe, the following AS is the iUnit */
#define PIPE_IUINIT_AS              CPSS_HW_DRIVER_AS_MG2_E
/* The 'user mask' reg addr in iUint,which is also used to trigger COPY operation */
#define PIPE_IUINIT_USR_MASK_REG    0x7D0
/* SPR is a register used by iUnit to store the value that was read from an I2C-1 reg */
#define SCRATCH_PAD_REG             0x1034C404

/* Addresses of I2C-1 regs, that cannot be read by SSMI, thus require the WA, by using iUnit */
#define I2C1_BASE                   0x11100
#define I2C1_SOFT_RST               (I2C1_BASE+0x1C)
#define I2C1_SOFT_RST_ENTRY_OFF     4

#undef MV_DEBUG

/* 6 Iunit entries for reading 6 I2C regs */
#define READ_ENTRY_ID       0

#ifdef MV_DEBUG
static CPSS_HW_DRIVER_SIP5_SSMI_STC *dbgSsmiDrv;
#endif


static int prvPipeUnitMapArr[16] = {
    0,  /* 0=CNM */
    -1, /* 1 */
    -1, /* 2 */
    3,  /* 3=MG0 */
    -1, /* 4 */
    5,  /* 5=MG1 */
    6, /* 6=iUnit(DMA) */
    -1, /* 7=MG3 */
    8,  /* 8=DFX */
    -1, -1, -1, -1, -1, -1, -1
};


#define SMI_ACCESS_TRACE
#ifdef SMI_ACCESS_TRACE
GT_BOOL smi_access_trace_enable = GT_FALSE;
#define SMI_ACCESS_TRACE_PRINTF(...) \
    if( GT_TRUE == smi_access_trace_enable) \
    {                                       \
        cpssOsPrintf(__VA_ARGS__);          \
    }
void smiAccessTraceEnable(GT_BOOL enable)
{
    smi_access_trace_enable = enable;
}
#else /*!SMI_ACCESS_TRACE*/
#define SMI_ACCESS_TRACE_PRINTF(...)    /*empty*/
#endif/*!SMI_ACCESS_TRACE*/
/**
* @internal prvSlaveSmiWaitForStatus function
* @endinternal
*
* @brief   Slave SMI wait
*/
static GT_STATUS prvSlaveSmiWaitForStatus(CPSS_HW_DRIVER_SIP5_SSMI_STC *drv, GT_U32 waitFor)
{
#ifdef SMI_WAIT_FOR_STATUS_DONE
    GT_U32 stat;
    unsigned int timeOut;
    int rc;

    /* wait for write done */
    timeOut = SMI_TIMEOUT_COUNTER;
    do
    {
        rc = SMI->read(SMI, drv->phy, SMI_STATUS_REGISTER, &stat, 1);
        if (rc != GT_OK)
            return rc;
        if (--timeOut < 1)
        {
            /* printk("bspSmiWaitForStatus timeout !\n"); */
            return GT_TIMEOUT;
        }
    } while ((stat & waitFor) == 0);

    /* 0xFFFF is returned when SMI device is not present */
    if(stat == SMI_INVALID_DATA)
        return GT_FAIL;
#endif
    return GT_OK;
}

#define CHK_RC(cmd)      \
        rc = cmd;        \
        if (rc != GT_OK) \
            return rc

/**
* @internal prvSlaveSmiRead32 function
* @endinternal
*
* @brief   Slave SMI read 32bit word (low level)
*
* @param[in] regAddr                  - 32-bit register address
*
* @param[out] dataPtr                  - pointer to store data read
*                                       GT_STATUS
*/
static GT_STATUS prvSlaveSmiRead32(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr
)
{
    GT_STATUS rc;
    GT_U32  msb, lsb;

    msb = regAddr >> 16;
    lsb = regAddr & 0xffff;
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_READ_ADDRESS_MSB_REGISTER, &msb, 1, 0xffffffff));
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_READ_ADDRESS_LSB_REGISTER, &lsb, 1, 0xffffffff));

    CHK_RC(prvSlaveSmiWaitForStatus(drv, SMI_STATUS_READ_READY));

    CHK_RC(SMI->read(SMI, drv->phy, SMI_READ_DATA_MSB_REGISTER, &msb, 1));
    CHK_RC(SMI->read(SMI, drv->phy, SMI_READ_DATA_LSB_REGISTER, &lsb, 1));

    *dataPtr = (msb << 16) | lsb;

    return GT_OK;
}


/**
* @internal prvSlaveSmiWrite32 function
* @endinternal
*
* @brief   Slave SMI write 32bit word (low level)
*
* @param[in] regAddr                  - 32-bit register address
* @param[in] data                     - 32-bit data
*                                       GT_STATUS
*/
static GT_STATUS prvSlaveSmiWrite32(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  regAddr,
    IN  GT_U32  data
)
{
    GT_STATUS rc;
    GT_U32  msb, lsb;

    msb = regAddr >> 16;
    lsb = regAddr & 0xffff;

    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_ADDRESS_MSB_REGISTER, &msb, 1, 0xffffffff));
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_ADDRESS_LSB_REGISTER, &lsb, 1, 0xffffffff));

    msb = data >> 16;
    lsb = data & 0xffff;

    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_DATA_MSB_REGISTER, &msb, 1, 0xffffffff));
    CHK_RC(SMI->writeMask(SMI, drv->phy, SMI_WRITE_DATA_LSB_REGISTER, &lsb, 1, 0xffffffff));

    rc = prvSlaveSmiWaitForStatus(drv, SMI_STATUS_WRITE_DONE);

    return rc;
}

static GT_STATUS prvSlaveSmiCompl(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    OUT GT_U32  *reg
)
{
    int mbusUnitId = -1;
    GT_U32 r;
    GT_STATUS rc;

    if (as == 0)
    {
        /* cnm */
        *reg = regAddr /* & 0xfffffffc */;
        return GT_OK;
    }
    else if (as < 16 && drv->asMap)
    {
        mbusUnitId = drv->asMap[as];
    }
    else if (as == SSMI_FALCON_ADDRESS_SPACE) {
        mbusUnitId = (regAddr>>20) & 0x0f;
        regAddr &=(~0xF00000);
    }
    if (mbusUnitId < 0)
            return GT_BAD_PARAM;

    r = (regAddr>>28) & 0x0f;
    if (r != drv->compl[mbusUnitId])
    {
        rc = prvSlaveSmiWrite32(drv, SSMI_XBAR_PORT_REMAP(mbusUnitId), r|0x00010000);
        if (rc != GT_OK)
            return rc;
        drv->compl[mbusUnitId] = r;
    }
    if (as == SSMI_FALCON_ADDRESS_SPACE && mbusUnitId == 1)
    {
        *reg = (regAddr&0x0ffffffc) | ((mbusUnitId<<28));
    } else{
        *reg = (regAddr&0x0ffffffc) | ((mbusUnitId<<28)|2);
    }

    return GT_OK;
}

static GT_STATUS prvSlaveSmiRead(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    reg;
    cpssOsMutexLock(drv->mtx);

    for (; count; count--,regAddr+=4, dataPtr++)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI (before)Read: regAddr 0x%8.8x \n", regAddr);

        rc = prvSlaveSmiCompl(drv, as, regAddr, &reg);
        if (rc != GT_OK)
            break;
        rc = prvSlaveSmiRead32(drv, reg, dataPtr);
        if (rc != GT_OK)
            break;

        SMI_ACCESS_TRACE_PRINTF("SMI (after) Read: rc %x, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr, *dataPtr);
    }

    cpssOsMutexUnlock(drv->mtx);
    return rc;
}

/* In Pipe, SSMI cannot access I2C-1 regs, so iUnit is used to access it instead.
iUnit is able to perform actions of COPY(SRC, DST).
So In order to use SMI on I2C1, the following is used:
-   SMI read: iUnit copies I2C1_reg to SPR, then SMI reads from scratch-pad-reg (SPR)
Bottom line: each read from SMI->I2C1 now requires twice (x2) SMI transactions compared to other SMI access.

The iUnit is comprised of 24 entries, each has 'SRC' part (called read), and 'DST' part called write.
Each of these parts contains an address field to act upon, and attribute field ? which contains the unit id to act upon (for example, I2C regs are located in unitId 1, while SPR in unitId 3).

In order to run (activate) the n-th entry, the user-mask reg (0x7D0) is written with the n-th bit set to '1'.

In order to perform as few SMI transactions as possible, 5 iUint entries are pre-configured (API prvIunitCfg).
Each of the 5, is configured with one of the following I2C mngt reg: 11100, 11104, 11108, 1110C and 1111C.
When need to access an I2C1 reg, only need to activate a single entry (in addition to SPR)
*/
static GT_STATUS prvPipeSlaveSmiRead(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    OUT GT_U32 *dataPtr,
    IN  GT_U32  count
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    reg;
    GT_U32    entry_bit;
    cpssOsMutexLock(drv->mtx);

    for (; count; count--,regAddr+=4, dataPtr++)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI (before)Read: regAddr 0x%8.8x \n", regAddr);

        if (as==0 &&
            ((regAddr>=I2C1_BASE && regAddr<=(I2C1_BASE+0xC)) || regAddr==I2C1_SOFT_RST )
           )
        {
#ifdef MV_DEBUG
            cpssOsPrintf(">> regAddrR 0x%x\n", regAddr);
#endif
            /* run entry - reads I2C reg */
            rc = prvSlaveSmiCompl(drv, PIPE_IUINIT_AS, PIPE_IUINIT_USR_MASK_REG, &reg);
            if (rc != GT_OK)
               break;
            if (regAddr==I2C1_SOFT_RST)
                entry_bit = 1 << (READ_ENTRY_ID + I2C1_SOFT_RST_ENTRY_OFF);
            else
                entry_bit = 1 << (READ_ENTRY_ID + ((regAddr-I2C1_BASE)/4));
            /* even if we set the USR_MASK_REG to the same value that it currently holds - action will be triggered */
            rc = prvSlaveSmiWrite32(drv, reg, entry_bit);
            if (rc != GT_OK)
                break;

            /* read from scratch-pad */
            rc = prvSlaveSmiCompl(drv, CPSS_HW_DRIVER_AS_SWITCHING_E, SCRATCH_PAD_REG, &reg);
            if (rc != GT_OK)
                break;
            rc = prvSlaveSmiRead32(drv, reg, dataPtr);
            if (rc != GT_OK)
                break;
        }
        else
        {
            rc = prvSlaveSmiCompl(drv, as, regAddr, &reg);
            if (rc != GT_OK)
                break;
            rc = prvSlaveSmiRead32(drv, reg, dataPtr);
            if (rc != GT_OK)
                break;
        }

        SMI_ACCESS_TRACE_PRINTF("SMI (after) Read: rc %x, regAddr 0x%8.8x, data 0x%8.8x\n", rc, regAddr, *dataPtr);
    }

    cpssOsMutexUnlock(drv->mtx);
    return rc;
}


static GT_STATUS prvSlaveSmiWriteMask(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv,
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    IN  GT_U32 *dataPtr,
    IN  GT_U32  count,
    IN  GT_U32  mask
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32 dataWr, reg;

    cpssOsMutexLock(drv->mtx);

    for (; count; count--,regAddr+=4, dataPtr++)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI Write: regAddr 0x%8.8x, data 0x%8.8x, mask 0x%8.8x \n", regAddr, *dataPtr, mask);

        rc = prvSlaveSmiCompl(drv, as, regAddr, &reg);
        if (rc != GT_OK)
            break;
        if (mask != 0xffffffff)
        {
            rc = prvSlaveSmiRead32(drv, reg, &dataWr);
            if (rc != GT_OK)
                break;
            dataWr &= ~mask;
            dataWr |= (*dataPtr & mask);
        }
        else
        {
            dataWr = *dataPtr;
        }
        rc = prvSlaveSmiWrite32(drv, reg, dataWr);
        if (rc != GT_OK)
            break;
    }

    if(rc != GT_OK)
    {
        SMI_ACCESS_TRACE_PRINTF("SMI Write: FAILED rc=[%d]\n",rc);
    }

    cpssOsMutexUnlock(drv->mtx);
    return rc;
}

static GT_STATUS prvSlaveSmiDesroy(
    IN  CPSS_HW_DRIVER_SIP5_SSMI_STC *drv
)
{
    cpssOsMutexDelete(drv->mtx);
    cpssOsFree(drv);
    return GT_OK;
}

/* 2 debug functions, that make use of the SSMI-driver that has been set.
   Are used to make sure all I2C 0 & 1 regs are accesible, for read/write ops */
#ifdef MV_DEBUG
int prvCpssSip5SSMIDbgRead(
    IN  GT_U32  as,
    IN  GT_U32  regAddr
)
{
    GT_STATUS rc;
    GT_U32 data;

    if (!dbgSsmiDrv)
        return -1;
    rc = dbgSsmiDrv->common.read((struct CPSS_HW_DRIVER_STCT*)dbgSsmiDrv, as, regAddr, &data, 1);
    cpssOsPrintf("data 0x%x\n", data);
    return rc;
}

int prvCpssSip5SSMIDbgWrite(
    IN  GT_U32  as,
    IN  GT_U32  regAddr,
    IN  GT_U32  val
)
{
    GT_STATUS rc;

    if (!dbgSsmiDrv)
        return -1;
    rc = dbgSsmiDrv->common.writeMask((struct CPSS_HW_DRIVER_STCT*)dbgSsmiDrv, as, regAddr, &val, 1, 0xFFFFFFFF);
    return rc;
}
#endif


#define READ_ADDR_HIGH(n)   (n*0x20)
#define READ_ADDR_LOW(n)    (4 + n*0x20)
#define READ_ADDR_ATTR(n)   (8 + n*0x20)
#define WRITE_ADDR_HIGH(n)  (0xC +n*0x20)
#define WRITE_ADDR_LOW(n)   (0x10 + n*0x20)
#define WRITE_ADDR_ATTR(n)  (0x14 + n*0x20)

/* Configure the iUnit with 5 predefined entries for read op */
static void prvIunitCfg(CPSS_HW_DRIVER_SIP5_SSMI_STC *hwInfoPtr)
{
    GT_U32 val, i, entryId;

    /* I2Creg to scratch-pad copy */
    for (i=0; i<5; i++) {
        entryId = READ_ENTRY_ID + i;

        val = 0;
        prvSlaveSmiWriteMask(hwInfoPtr, PIPE_IUINIT_AS, READ_ADDR_HIGH(entryId), &val, 1, 0xFFFFFFFF);
        if (i==4)
            val = I2C1_SOFT_RST;
        else
            val = I2C1_BASE + i*4;
        prvSlaveSmiWriteMask(hwInfoPtr, PIPE_IUINIT_AS, READ_ADDR_LOW(entryId), &val, 1, 0xFFFFFFFF);
        val = 0x104000;
        prvSlaveSmiWriteMask(hwInfoPtr, PIPE_IUINIT_AS, READ_ADDR_ATTR(entryId), &val, 1, 0xFFFFFFFF);
        val = 0;
        prvSlaveSmiWriteMask(hwInfoPtr, PIPE_IUINIT_AS, WRITE_ADDR_HIGH(entryId), &val, 1, 0xFFFFFFFF);
        val = SCRATCH_PAD_REG;
        prvSlaveSmiWriteMask(hwInfoPtr, PIPE_IUINIT_AS, WRITE_ADDR_LOW(entryId), &val, 1, 0xFFFFFFFF);
        val = 0x13001;
        prvSlaveSmiWriteMask(hwInfoPtr, PIPE_IUINIT_AS, WRITE_ADDR_ATTR(entryId), &val, 1, 0xFFFFFFFF);
    }
}


/**
* @internal cpssHwDriverSip5SlaveSMICreateDrv function
* @endinternal
*
* @brief Create driver instance for Sip5 Slave SMI
*
* @param[in] smi        - pointer to SMI driver
* @param[in] phy        - PP phy address
* @param[in] haSkipInit - skip Init if true
*
* @retval CPSS_HW_DRIVER_STC* or NULL
*/
CPSS_HW_DRIVER_STC *cpssHwDriverSip5SlaveSMICreateDrv(
    IN  CPSS_HW_DRIVER_STC *smi,
    IN  GT_U32              phy,
    IN  GT_BOOL             haSkipInit
)
{
    CPSS_HW_DRIVER_SIP5_SSMI_STC *drv;
    char buf[64];
    GT_STATUS rc = GT_OK;
    GT_U32 i;
    GT_BOOL asicSimulationIsRunning = GT_FALSE;

    /* check params validity */
    if ((smi == NULL) || (phy >= 32))
        return NULL;

#ifdef ASIC_SIMULATION
    asicSimulationIsRunning = GT_TRUE;
#endif

    drv = cpssOsMalloc(sizeof(*drv));
    if (drv == NULL)
        return NULL;


    cpssOsMemSet(drv,0,sizeof(*drv));
    drv->common.read = (CPSS_HW_DRIVER_METHOD_READ)prvSlaveSmiRead;
    drv->common.writeMask = (CPSS_HW_DRIVER_METHOD_WRITE_MASK)prvSlaveSmiWriteMask;
    drv->common.destroy = (CPSS_HW_DRIVER_METHOD_DESTROY)prvSlaveSmiDesroy;
    drv->common.parent = smi;
    drv->phy = phy;
    drv->common.type = CPSS_HW_DRIVER_TYPE_GEN_SLAVE_SMI_E;

    /*
      enters in case haSkipInit == GT_FALSE or simulation ,
      in case of simlation code should be enter and rc returns GT_FAIL as expected
    */
    if ((haSkipInit == GT_FALSE) ||
        (haSkipInit == GT_TRUE && asicSimulationIsRunning == GT_TRUE))
    {
        /* init SSMI XBAR */
        for (i = 1; i < 16; i++)
        {
            rc = prvSlaveSmiWrite32(drv, SSMI_XBAR_PORT_REMAP(i), 0x00010000);
            if (rc != GT_OK)
                break;
        }
        if (rc != GT_OK)
        {
            cpssOsFree(drv);
            return NULL;
        }
    }

    /* TODO: detect device ID, apply correct unit map */
    /* PIPE only */
    drv->asMap = prvPipeUnitMapArr;

    cpssOsSprintf(buf, "ssmi@0x%x", phy);
    cpssOsMutexCreate(buf, &(drv->mtx));
#ifdef MV_DEBUG
    dbgSsmiDrv = drv;
#endif
    return (CPSS_HW_DRIVER_STC*)drv;
}


GT_STATUS prvCpssHwDriverPipeSlaveSMISetDrv(CPSS_HW_DRIVER_STC* drv)
{
    if (!drv)
        return GT_NOT_INITIALIZED;

    prvIunitCfg((CPSS_HW_DRIVER_SIP5_SSMI_STC *)drv);
    drv->read = (CPSS_HW_DRIVER_METHOD_READ)prvPipeSlaveSmiRead;
    return GT_OK;
}


/**
* @internal cpssHwDriverSlaveSmiConfigure function
* @endinternal
*
* @brief   Configure Slave SMI device driver
*
* @param[in] smiMasterPath            - driver pointer
* @param[in] slaveSmiPhyId            - SMI Id of slave device
*
* @param[out] hwInfoPtr               - The pointer to HW info, will be used
*                                       for cpssDxChHwPpPhase1Init()
*                                       NULL to not fill HW info
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PARAM                - on wrong smiMasterPath
* @retval GT_NOT_INITIALIZED          - CPSS was not initialized yet by cpssPpInit
* @retval GT_OUT_OF_CPU_MEM           - on failed malloc
*/
GT_STATUS cpssHwDriverSlaveSmiConfigure(
    IN  const char         *smiMasterPath,
    IN  GT_U32              slaveSmiPhyId,
    OUT CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    CPSS_HW_DRIVER_STC *smi, *ssmi;
    char drvName[16];
    if (smiMasterPath == NULL)
        return GT_BAD_PARAM;

    if (NULL == cpssSharedGlobalVarsPtr)
    {
        return GT_NOT_INITIALIZED;
    }

    smi = cpssHwDriverLookup(smiMasterPath);
    if (smi == NULL)
        return GT_NOT_FOUND;
    ssmi = cpssHwDriverSip5SlaveSMICreateDrv(smi, slaveSmiPhyId,GT_FALSE);
    if (ssmi == NULL)
        return GT_OUT_OF_CPU_MEM;
    cpssOsSprintf(drvName, "SSMI@%d", slaveSmiPhyId);
    cpssHwDriverRegister(ssmi, drvName);
    if (hwInfoPtr != NULL)
    {
        CPSS_HW_INFO_STC hwInfo = CPSS_HW_INFO_STC_DEF;
        hwInfo.busType = CPSS_HW_INFO_BUS_TYPE_SMI_E;
        hwInfo.hwAddr.busNo = 0;
        hwInfo.hwAddr.devSel = slaveSmiPhyId;
        hwInfo.driver = ssmi;
        hwInfo.irq.switching = CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS;
        *hwInfoPtr = hwInfo;
    }
    return GT_OK;
}



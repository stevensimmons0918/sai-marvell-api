/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalPhyConfig.c
*
* DESCRIPTION:
*       initialize PHY system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include "xpsCommon.h"
#include <cpssHalProfile.h>

#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if defined(INCLUDE_MPD)
#undef MIN
#include <mpdPrv.h>

#define PHY_ID_REG_DEV_NUM_CNS                1
#define PHY_ID_REG_ADDR_CNS                   3

#define MRVL_PHY_1780_MODEL_NUM_CNS 0x2D
#define MRVL_PHY_2540_MODEL_NUM_CNS 0x3C1 /* 4.2003=0xBC1, bit[9:4]=Model_Num */
#define MRVL_PHY_2580_MODEL_NUM_CNS 0x3C3

typedef struct GT_CPSSHAL_XPHY_INFO_STCT
{
    GT_U32                              phyIndex;
    GT_U32                              portGroupId;
    CPSS_PHY_XSMI_INTERFACE_ENT         xsmiInterface;
    GT_U16                              phyAddr;
    GT_U32                              phyType;
    GT_U32                              hostDevNum;
    GT_VOID_PTR                         driverObj;
} GT_CPSSHAL_XPHY_INFO_STC;

static struct
{
    GT_U8       devNum;
    GT_U32      macNum;
} ifIndexInfoArr[MPD_MAX_PORT_NUMBER_CNS+1];

static GT_U32 nextFreeGlobalIfIndex = 1;

static GT_BOOL cpssHalPhyMpdInitDone = GT_FALSE;


GT_STATUS cpssHalPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  smiInterface,
    IN  GT_U32                      smiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;

    /* read PHY ID */
    rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface,
                                  smiAddr, PHY_ID_REG_ADDR_CNS, &data);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssSmiRegisterReadShort rc=%d\n", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {
        *phyIdPtr = (data >> 4) & 0x3F;
    }
    else
    {
        *phyIdPtr = 0;
    }

    return GT_OK;
}


GT_STATUS cpssHalXPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;

    /* read PHY ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                       xsmiAddr, PHY_ID_REG_ADDR_CNS,
                                       PHY_ID_REG_DEV_NUM_CNS, &data);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssXsmiPortGroupRegisterRead rc=%d\n", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {
        switch (data)
        {
            case 0xBC1: /* E2540, 0x3C1 */
            case 0xBC3: /* E2580, 0x3C3 */
                *phyIdPtr = (data & 0x3FF);
                break;
            default:
                *phyIdPtr = (data >> 4) & 0x3F;
                break;
        }
    }
    else
    {
        *phyIdPtr = 0;
    }

    return GT_OK;
}


GT_VOID cpssHalXSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    XP_DEV_TYPE_T                   xpDevType,
    OUT GT_CPSSHAL_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;  /* SMI i/f iterator */
    GT_U32      xsmiAddr;               /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U8       maxXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;
    GT_U8       minXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;

    *phyInfoArrayLenPtr = 0;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for (xsmiInterface = minXSMIinterface; xsmiInterface <= maxXSMIinterface;
             xsmiInterface++)
        {
            for (xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = cpssHalXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("cpssHalPhyIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }
                if (data == 0)
                {
                    continue;
                }

                phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
                phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)xsmiAddr;
                phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = xsmiInterface;
                phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
                cpssOsPrintf("%d)portGroupId=%d,phyAddr=0x%x,xsmiInterface=%d,phyType=0x%x\n",
                             *phyInfoArrayLenPtr,
                             phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                             phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                             phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                             phyInfoArray[*phyInfoArrayLenPtr].phyType);
                (*phyInfoArrayLenPtr)++;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

    return;
}


GT_VOID cpssHalSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    XP_DEV_TYPE_T                   xpDevType,
    OUT GT_CPSSHAL_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    GT_U32      smiAddr;                /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    CPSS_PHY_SMI_INTERFACE_ENT smiInterfaceEnum = 0;
    *phyInfoArrayLenPtr = 0;

    if (IS_DEVICE_AC5X(xpDevType))
    {
        /*RD-AC5X use SMI[1] + XSMI[0] */
        smiInterfaceEnum = CPSS_PHY_SMI_INTERFACE_1_E;
    }


    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for (smiAddr = 0; smiAddr < BIT_5; smiAddr++)
        {
            rc = cpssHalPhyIdGet(devNum, portGroupsBmp, smiInterfaceEnum, smiAddr, &data);
            if (rc != GT_OK)
            {
                cpssOsPrintf("cpssHalPhyIdGet FAIL:devNum=%d,portGroupId=%d,smiInterface=%d,smiAddr=%d\n",
                             devNum, portGroupId, smiInterfaceEnum, smiAddr);
                continue;
            }
            if (data == 0)
            {
                continue;
            }

            phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
            phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)smiAddr;
            phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = smiInterfaceEnum;
            phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
            cpssOsPrintf("%d)portGroupId=%d,phyAddr=0x%x,smiInterface=%d,phyType=0x%x\n",
                         *phyInfoArrayLenPtr,
                         phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                         phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                         phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                         phyInfoArray[*phyInfoArrayLenPtr].phyType);
            (*phyInfoArrayLenPtr)++;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    return;
}

static GT_STATUS cpssHalPhyPortNumGet
(
    XP_DEV_TYPE_T                   xpDevType,
    PROFILE_STC                     profile[],
    IN  GT_CPSSHAL_XPHY_INFO_STC    phyInfo,
    OUT GT_U32                      *portNum
)
{
    int i = 0;
    GT_U32  portStart = 0;
    GT_U32  portEnd = 0;

    if (IS_DEVICE_AC5X(xpDevType))
    {
        if (phyInfo.phyType == MRVL_PHY_1780_MODEL_NUM_CNS)
        {
            portStart = 0;
            portEnd   = 31;
        }
        else if (phyInfo.phyType == MRVL_PHY_2580_MODEL_NUM_CNS)
        {
            portStart = 32;
            portEnd   = 39;
        }
        else if (phyInfo.phyType == MRVL_PHY_2540_MODEL_NUM_CNS)
        {
            portStart = 40;
            portEnd   = 47;
        }
        else
        {
            cpssOsPrintf("Invalid phyType 0x%x\n", phyInfo.phyType);
            return GT_FAIL;
        }
    }
    else
    {
        cpssOsPrintf("Unsupported device\n");
        return GT_FAIL;
    }


    while (profile[i].profileType != PROFILE_TYPE_LAST_E)
    {
        if ((profile[i].profileValue.portMap.portNum < portStart) ||
            (profile[i].profileValue.portMap.portNum > portEnd))
        {
            i++;
            continue;
        }
        if ((profile[i].profileType == PROFILE_TYPE_PORT_MAP_E) ||
            (profile[i].profileType == PROFILE_TYPE_CASCADE_PORT_MAP_E))
        {
            if (profile[i].profileValue.portMap.phyInfo.existsPhy &&
                (phyInfo.phyAddr == profile[i].profileValue.portMap.phyInfo.phyAddr))
            {
                *portNum = profile[i].profileValue.portMap.portNum;
                return GT_OK;
            }
        }
        i++;
    }
    return GT_NO_MORE;
}

MPD_TYPE_ENT cpssHalMpdPhyTypeGet
(
    GT_U32 phyType
)
{
    switch (phyType)
    {
        case MRVL_PHY_2540_MODEL_NUM_CNS:
            return MPD_TYPE_88E2540_E;
        case MRVL_PHY_2580_MODEL_NUM_CNS:
            return MPD_TYPE_88E2580_E;
        case MRVL_PHY_1780_MODEL_NUM_CNS:
            return MPD_TYPE_88E1780_E;
        default:
            return MPD_TYPE_INVALID_E;
    }
}

GT_U32 cpssHalMpdPhyIndexGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum
)
{
    GT_U32 index;

    for (index = 1; index < nextFreeGlobalIfIndex; index++)
    {
        if (ifIndexInfoArr[index].devNum == devNum &&
            ifIndexInfoArr[index].macNum == portNum)
        {
            return index;
        }
    }
    ifIndexInfoArr[nextFreeGlobalIfIndex].devNum  = devNum;
    ifIndexInfoArr[nextFreeGlobalIfIndex].macNum = portNum;
    if (nextFreeGlobalIfIndex < MPD_MAX_PORT_NUMBER_CNS)
    {
        return nextFreeGlobalIfIndex++;
    }
    else
    {
        //Array overflow
        cpssOsPrintf("MPD Global ifindex overflow\n");
        return nextFreeGlobalIfIndex;
    }
}



BOOLEAN cpssHalMpdPhyOsDelay
(
    IN UINT_32 delay
)
{
    struct timespec sleep, remain;
    if (1000000000 < delay)
    {
        /* sleep time to big */
        return 0;
    }

    sleep.tv_nsec = delay*1000000;
    sleep.tv_sec = 0;
    if (nanosleep(&sleep, &remain))
    {
        return TRUE;
    }

    return FALSE;
}

void * cpssHalMpdPhyOsMalloc
(
    UINT_32  size
)
{
    return cpssOsMalloc(size);
}

void cpssHalMpdPhyOsFree
(
    void* data
)
{
    return cpssOsFree(data);
}

BOOLEAN cpssHalMpdPhyDebugBind(
    IN const char   * comp_PTR,
    IN const char   * pkg_PTR,
    IN const char   * flag_PTR,
    IN const char   * help_PTR,
    OUT UINT_32         * flagId_PTR
)
{
    /* no more debug flags */
    return FALSE;
}

BOOLEAN cpssHalMpdPhyDebugIsActive(
    IN UINT_32  flagId
)
{
    return FALSE;
}


void cpssHalMpdPhyDebugLog(
    IN const char    * funcName_PTR,
    IN const char    * format_PTR,
    IN ...
)
{
    va_list      argptr;

    printf("%s:", funcName_PTR);
    va_start(argptr, format_PTR);
    vprintf(format_PTR, argptr);
    va_end(argptr);
    printf("\n");
}

MPD_RESULT_ENT cpssHalMpdPhyHandleFailure(
    IN UINT_32                              rel_ifIndex,
    IN MPD_ERROR_SEVERITY_ENT                  severity,
    IN UINT_32                                     line,
    IN const char                           *calling_func_PTR,
    IN const char                           *error_text_PTR
)
{
    char * severityText[MPD_ERROR_SEVERITY_MINOR_E+1] = {"Fatal", "Error", "Minor"};
    cpssOsPrintf("Failure Level [%s] on port [%d] line [%d] called by [%s].  %s\n",
                 (severity<=MPD_ERROR_SEVERITY_MINOR_E)?severityText[severity]:"Unknown",
                 rel_ifIndex,
                 line,
                 calling_func_PTR,
                 error_text_PTR);

    if (severity == MPD_ERROR_SEVERITY_FATAL_E)
    {
        cpssOsPrintf("Fatal\n");
    }

    return MPD_OK_E;
}

MPD_RESULT_ENT cpssHalMpdDisableSmiAutoNeg(
    IN  UINT_32      rel_ifIndex,
    IN  BOOLEAN      disable,
    OUT BOOLEAN     *prev_status_PTR
)
{
    GT_BOOL             prev_state, new_status;
    GT_STATUS           gt_status;
    UINT_8              dev, port;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL)
    {
        return MPD_OP_FAILED_E;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E)
    {
        return MPD_OP_FAILED_E;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;

    gt_status = cpssDxChPhyAutonegSmiGet(dev, port, &prev_state);
    if (gt_status != GT_OK)
    {
        return MPD_OP_FAILED_E;
    }

    (* prev_status_PTR) = (prev_state == GT_TRUE)?FALSE:TRUE;
    new_status = (disable == TRUE) ? GT_FALSE : GT_TRUE;
    if (prev_state != new_status)
    {
        gt_status = cpssDxChPhyAutonegSmiSet(dev, port, new_status);
        if (gt_status != GT_OK)
        {
            return MPD_OP_FAILED_E;
        }
    }

    return MPD_OK_E;
}



BOOLEAN cpssHalMpdPhyGetFwFiles
(
    IN  MPD_TYPE_ENT          phyType,
    OUT MPD_FW_FILE_STC   * mainFile_PTR
)
{
    char * fwFileName_PTR = NULL;
    char * fwFileName_PTR_relative = NULL;
    FILE * fp;
    int ret;
    UINT_32 fileSize;

    switch (phyType)
    {
        case MPD_TYPE_88E2540_E:
            fwFileName_PTR = "/fwFiles/phy/v0A0A0000_11508_11488_e2540.hdr";
            break;
        case MPD_TYPE_88E2580_E:
            fwFileName_PTR = "/fwFiles/phy/v0A0A0000_11508_11488_e2580.hdr";
            break;
        default:
            cpssOsPrintf("not supported PHY type\n");
            break;
    }
    if (fwFileName_PTR == NULL)
    {
        return FALSE;
    }

    fp = fopen(fwFileName_PTR, "r");
    if (fp == NULL)
    {
        // Search if file exists in relative path
        // xpSaiApp execution will look for file in this path
        fwFileName_PTR_relative = (char *)malloc(sizeof(char)*(strlen(
                                                                   fwFileName_PTR)+1));
        fwFileName_PTR_relative[0] = '.';
        fwFileName_PTR_relative[1] = '\0';
        strcat(fwFileName_PTR_relative, fwFileName_PTR);
        cpssOsPrintf("Reading firmware file from %s\n", fwFileName_PTR_relative);
        fp = fopen(fwFileName_PTR_relative, "r");
    }

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        mainFile_PTR->dataSize = fileSize;
        mainFile_PTR->data_PTR = cpssHalMpdPhyOsMalloc(fileSize);

        if (mainFile_PTR->data_PTR == NULL)
        {
            fclose(fp);
            if (fwFileName_PTR_relative != NULL)
            {
                free(fwFileName_PTR_relative);
            }
            return FALSE;
        }
        ret = fread(mainFile_PTR->data_PTR, sizeof(char), fileSize, fp);
        fclose(fp);
        if (fwFileName_PTR_relative != NULL)
        {
            free(fwFileName_PTR_relative);
        }
        if (ret <= 0)
        {
            cpssOsPrintf("Read from file fail\n");
            return GT_ERROR;
        }
    }
    return TRUE;
}

BOOLEAN cpssHalMpdPhyXsmiMdioRead(
    IN  UINT_32      rel_ifIndex,
    IN  UINT_8       mdioAddress,
    IN  UINT_16      deviceOrPage,
    IN  UINT_16      address,
    OUT UINT_16    * value_PTR
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface, smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL)
    {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E)
    {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    if (port_entry_PTR->initData_PTR->phyType != MPD_TYPE_88E1780_E)
    {
        xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssXsmiPortGroupRegisterRead(dev,
                                               portGroupsBmp,
                                               xsmiInterface,
                                               mdioAddress,
                                               address,
                                               deviceOrPage,
                                               value_PTR);
    }
    else
    {
        smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssSmiRegisterReadShort(dev,
                                          portGroupsBmp,
                                          smiInterface,
                                          mdioAddress,
                                          address,
                                          value_PTR);

        if (status != GT_OK)
        {
            return FALSE;
        }
    }
    if (status != GT_OK)
    {
        return FALSE;
    }
    return TRUE;
}

BOOLEAN cpssHalMpdPhyXsmiMdioWrite(
    IN UINT_32      rel_ifIndex,
    IN UINT_8       mdioAddress,
    IN UINT_16      deviceOrPage,   /* PRV_MPD_IGNORE_PAGE_CNS */
    IN UINT_16      address,
    IN UINT_16      value
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface, smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL)
    {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E)
    {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    if (port_entry_PTR->initData_PTR->phyType != MPD_TYPE_88E1780_E)
    {
        xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssXsmiPortGroupRegisterWrite(dev,
                                                portGroupsBmp,
                                                xsmiInterface,
                                                mdioAddress,
                                                address,
                                                deviceOrPage,
                                                value);
    }
    else
    {
        smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssSmiRegisterWriteShort(dev,
                                           portGroupsBmp,
                                           smiInterface,
                                           mdioAddress,
                                           address,
                                           value);
    }
    if (status != GT_OK)
    {
        return FALSE;
    }
    return TRUE;
}


MPD_RESULT_ENT cpssHalMpdPhyPrintLog(
    const char              * log_text_PTR
)
{
    cpssOsPrintf(log_text_PTR);
    return MPD_OK_E;
}




GT_STATUS cpssHalPhyMpdInit
(
    GT_U8         devNum,
    XP_DEV_TYPE_T xpDevType,
    PROFILE_STC   profile[]
)
{
    UINT_32                                     rel_ifIndex, i;
    MPD_RESULT_ENT                              rc = MPD_OK_E;
    MPD_CALLBACKS_STC                           phy_callbacks;
    MPD_PORT_INIT_DB_STC                        phy_entry;
    GT_U32                                      portNum = 0;
    GT_CPSSHAL_XPHY_INFO_STC                    xsmiPhyInfoArray[256];
    GT_CPSSHAL_XPHY_INFO_STC                    smiPhyInfoArray[256];
    GT_U32                                      xsmiPhyInfoArrayLen = 0,
                                                smiPhyInfoArrayLen = 0;
    MPD_TYPE_ENT                                phyType;
    GT_U32                                      phyNumber = 0;
    MPD_OPERATIONS_PARAMS_UNT                   opParams;

    if (cpssHalPhyMpdInitDone)
    {
        return GT_OK;
    }

    memset(xsmiPhyInfoArray, 0, sizeof(xsmiPhyInfoArray));
    memset(smiPhyInfoArray, 0, sizeof(smiPhyInfoArray));

    cpssHalXSmiPhyAddrArrayBuild(devNum, xpDevType, xsmiPhyInfoArray,
                                 &xsmiPhyInfoArrayLen);
    cpssHalSmiPhyAddrArrayBuild(devNum, xpDevType, smiPhyInfoArray,
                                &smiPhyInfoArrayLen);

    /* bind basic callbacks needed from host application */
    memset(&phy_callbacks, 0, sizeof(phy_callbacks));

    phy_callbacks.sleep_PTR               = cpssHalMpdPhyOsDelay;
    phy_callbacks.alloc_PTR               = cpssHalMpdPhyOsMalloc;
    phy_callbacks.free_PTR                = cpssHalMpdPhyOsFree;
    phy_callbacks.debug_bind_PTR          = cpssHalMpdPhyDebugBind;
    phy_callbacks.is_active_PTR           = cpssHalMpdPhyDebugIsActive;
    phy_callbacks.debug_log_PTR           = cpssHalMpdPhyDebugLog;
    phy_callbacks.txEnable_PTR            = NULL;
    phy_callbacks.handle_failure_PTR      = cpssHalMpdPhyHandleFailure;
    phy_callbacks.smiAn_disable_PTR       = cpssHalMpdDisableSmiAutoNeg;
    phy_callbacks.getFwFiles_PTR          = cpssHalMpdPhyGetFwFiles;
    phy_callbacks.mdioRead_PTR            = cpssHalMpdPhyXsmiMdioRead;
    phy_callbacks.mdioWrite_PTR           = cpssHalMpdPhyXsmiMdioWrite;
    phy_callbacks.logging_PTR             = cpssHalMpdPhyPrintLog;


    if (IS_DEVICE_AC5X(xpDevType))
    {
        rc = genRegisterSet(0, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("genRegisterSet failed: rc = %d\n", rc);
            return rc;
        }
    }

    rc = mpdDriverInitDb(&phy_callbacks);
    if (rc != MPD_OK_E)
    {
        cpssOsPrintf("cpssHalPhyMpdInit failed in mpdDriverInitDb \n");
        return GT_FAIL;
    }

    /* xsmi update */
    for (i = 0; i < xsmiPhyInfoArrayLen ; i++)
    {
        rc = cpssHalPhyPortNumGet(xpDevType, profile, xsmiPhyInfoArray[i], &portNum);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssHalPhyMpdInit failed \n");
            return GT_FAIL;
        }
        phyType = cpssHalMpdPhyTypeGet(xsmiPhyInfoArray[i].phyType);

        memset(&phy_entry, 0, sizeof(phy_entry));
        phy_entry.port                          = portNum;
        phy_entry.phyNumber                     = phyNumber;
        phy_entry.phyType                       = phyType;

        phy_entry.transceiverType               = MPD_TRANSCEIVER_COPPER_E;

        phy_entry.mdioInfo.mdioAddress          = xsmiPhyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = xsmiPhyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = xsmiPhyInfoArray[i].xsmiInterface;
        phy_entry.disableOnInit                 = GT_FALSE;

        switch (phy_entry.phyType)
        {
            case MPD_TYPE_88E2540_E:
                phy_entry.phySlice                  = (xsmiPhyInfoArray[i].phyAddr % 4);
                phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_OXGMII;
                break;
            case MPD_TYPE_88E2580_E:
                phy_entry.phySlice                  = (xsmiPhyInfoArray[i].phyAddr % 8);
                if (IS_DEVICE_AC5X(xpDevType))
                {
                    phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_OXGMII;
                }
                else
                {
                    phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_20G_QXGMII;
                }
                break;
            default:
                phy_entry.phySlice                  = portNum;
        }

        rel_ifIndex = cpssHalMpdPhyIndexGet(devNum, portNum);
        if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E)
        {
            cpssOsPrintf("cpssHalMpdInit failed in mpdPortDbUpdate\n");
            return GT_FAIL;
        }
    }

    /* smi update */
    for (i = 0; i < smiPhyInfoArrayLen; i++)
    {
        rc = cpssHalPhyPortNumGet(xpDevType, profile, smiPhyInfoArray[i], &portNum);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssHalPhyMpdInit failed \n");
            return GT_FAIL;
        }
        phyType = cpssHalMpdPhyTypeGet(smiPhyInfoArray[i].phyType);
        memset(&phy_entry, 0, sizeof(phy_entry));
        phy_entry.port                          = portNum;
        phy_entry.phyNumber                     = phyNumber /*portNum/8*/;
        phy_entry.phyType                       = phyType;

        phy_entry.transceiverType               = MPD_TRANSCEIVER_COPPER_E;

        phy_entry.mdioInfo.mdioAddress          = smiPhyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = smiPhyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = smiPhyInfoArray[i].xsmiInterface;
        phy_entry.phySlice                      = (smiPhyInfoArray[i].phyAddr % 4);
        phy_entry.usxInfo.usxType               = MPD_PHY_USX_TYPE_OXGMII;
        phy_entry.disableOnInit                 = GT_FALSE;

        rel_ifIndex = cpssHalMpdPhyIndexGet(devNum, portNum);
        if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E)
        {
            cpssOsPrintf("cpssHalPhyMpdInit failed in mpdPortDbUpdate\n");
            return GT_FAIL;
        }
    }

    for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++)
    {
        prvMpdGlobalDb_PTR->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_RAM_E;
    }

    cpssOsPrintf("MPD PHY initialize........ \n");

    if (mpdDriverInitHw() != MPD_OK_E)
    {
        cpssOsPrintf("cpssHalPhyMpdInit failed in mpdDriverInitHw");
        return GT_FAIL;
    }

    /* Advertise all supported speeds */
    opParams.phyAutoneg.enable       = MPD_AUTO_NEGOTIATION_ENABLE_E;
    opParams.phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS;
    opParams.phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_UNKNOWN_E;

    for (i = 0; i < xsmiPhyInfoArrayLen ; i++)
    {
        rc = cpssHalPhyPortNumGet(xpDevType, profile, xsmiPhyInfoArray[i], &portNum);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssHalPhyMpdInit failed \n");
            return GT_FAIL;
        }

        rel_ifIndex = cpssHalMpdPhyIndexGet(devNum, portNum);
        rc = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_AUTONEG_E, &opParams);
        if (MPD_OK_E != rc)
        {
            cpssOsPrintf("Error : devNum[%d]portNum[%d]rel_ifIndex[%d] : failed on [%s] status[%d]\n",
                         devNum, portNum, rel_ifIndex, "MPD_OP_CODE_SET_AUTONEG_E", rc);
            return GT_FAIL;
        }
    }

    cpssOsPrintf("MPD PHY initialize is finished \n");

    cpssHalPhyMpdInitDone = GT_TRUE;
    return GT_OK;

}

int cpssHalMpdPortDisable
(
    GT_SW_DEV_NUM            devNum,
    GT_U32                   portNum,
    GT_BOOL                  disable
)
{
    UINT_32 rel_ifIndex;
    MPD_OPERATIONS_PARAMS_UNT params;
    MPD_RESULT_ENT rc = MPD_OK_E;

    rel_ifIndex = cpssHalMpdPhyIndexGet(devNum, portNum);

    cpssOsMemSet(&params, 0, sizeof(params));
    params.phyDisable.forceLinkDown = disable;

    rc = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                                &params);
    if (rc != MPD_OK_E)
    {
        cpssOsPrintf("Error : devNum[%d]portNum[%d]rel_ifIndex[%d] : failed on [%s] enable[%d] status[%d]\n",
                     devNum, portNum, rel_ifIndex, "MPD_OP_CODE_SET_PHY_DISABLE_OPER_E", disable,
                     rc);
        return GT_FAIL;
    }

    return GT_OK;

}
#endif

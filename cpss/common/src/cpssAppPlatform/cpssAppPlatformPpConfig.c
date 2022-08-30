/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatfomPpConfig.c
*
* @brief This file contains APIs for adding and removing PPs.
*
* @version   1
********************************************************************************
*/
#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformBoardConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformPciConfig.h>
#include <cpssAppPlatformPpUtils.h>
#include <cpssAppPlatformPortInit.h>
#include <extUtils/common/cpssEnablerUtils.h>
#include <cpssAppUtilsEvents.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    #include <gtOs/gtOsExc.h>
    #include <gtExtDrv/drivers/gtDmaDrv.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/generic/cpssHwInfo.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#ifdef INCLUDE_MPD
#undef MAX_UINT_8
#undef MIN
#include <mpdPrv.h>
#define __USE_POSIX199309
#include <sched.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#endif

#define PHY_ID_REG_DEV_NUM_CNS                1
#define PHY_VENDOR_ID_REG_ADDR_CNS            2
#define PHY_ID_REG_ADDR_CNS                   3
#define APP_DEMO_XCAT5X_A0_BOARD_RD_CNS       0xB1

/* Marvell PHYs unique vendor ID - register 2 */
#define MRVL_PHY_REV_A_UID_CNS 0x2B
#define MRVL_PHY_UID_CNS 0x141
#define MRVL_PHY_1780_MODEL_NUM_CNS 0x2D
#define MRVL_PHY_2540_MODEL_NUM_CNS 0x3C /* 4.2003=0xBC1, bit[9:4]=Model_Num */
#define MRVL_PHY_3240_MODEL_NUM_CNS 0x18
#define MRVL_PHY_3340_MODEL_NUM_CNS 0x1A
#define MRVL_PHY_2180_MODEL_NUM_CNS 0x1B
#define MRVL_PHY_7120_MODEL_NUM_CNS 0x3B

#define MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS       (10)
GT_U16 portToPhyArrayAc5xRd[][2] = {
    {32,0x0},
    {33,0x1},
    {34,0x2},
    {35,0x3},
    {36,0x4},
    {37,0x5},
    {38,0x6},
    {39,0x7},
    {40,0x1c},
    {41,0x1d},
    {42,0x1e},
    {43,0x1f},
    {44,0xc},
    {45,0xd},
    {46,0xe},
    {47,0xf},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5xRd1780[][2] = {
    {0, 0x0},
    {1, 0x1},
    {2, 0x2},
    {3, 0x3},
    {4, 0x4},
    {5, 0x5},
    {6, 0x6},
    {7, 0x7},
    {8, 0x8},
    {9, 0x9},
    {10,0xa},
    {11,0xb},
    {12,0xc},
    {13,0xd},
    {14,0xe},
    {15,0xf},
    {16,0x10},
    {17,0x11},
    {18,0x12},
    {19,0x13},
    {20,0x14},
    {21,0x15},
    {22,0x16},
    {23,0x17},
    {24,0x18},
    {25,0x19},
    {26,0x1a},
    {27,0x1b},
    {28,0x1c},
    {29,0x1d},
    {30,0x1e},
    {31,0x1f},
    {0xff, 0xff}
};

static struct{
    GT_U8       devNum;
    GT_U32      macNum;
}ifIndexInfoArr[MPD_MAX_PORT_NUMBER_CNS];
static GT_U32 nextFreeGlobalIfIndex = 1;
GT_U16 vendorId;
GT_BOOL checkEnable = GT_TRUE;
GT_U16 phyModelNum;
typedef struct {
        BOOLEAN   isFree;
        char    * flagName_PTR;
        char    * flagHelp_PTR;
        BOOLEAN   flagStatus; /* TRUE - On */
}MPD_SAMPLE_DEBUG_FLAG_INFO_STC;
static MPD_SAMPLE_DEBUG_FLAG_INFO_STC mpdSampleDebugFlags[MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS];
static UINT_32 MPD_SAMPLE_DEBUG_FREE_INDEX = 0;

#define GT_DUMMY_REG_VAL_INFO_LIST                                          \
{                                                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},                                \
    {0x00000001, 0x00000000, 0x00000000, 0},                                \
    {0x00000002, 0x00000000, 0x00000000, 0},                                \
    {0x00000003, 0x00000000, 0x00000000, 0},                                \
    {0x00000004, 0x00000000, 0x00000000, 0},                                \
    {0x00000005, 0x00000000, 0x00000000, 0},                                \
    {0x00000006, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}     /* Delimiter        */      \
}

#define TIME_DIFF_MAC(sec1, nsec1, sec2, nsec2, sec, nsec) \
    if(nsec2 < nsec1)\
    {\
        nsec2 += 1000000000;\
        sec2  -= 1;\
    }\
    sec  = sec2 - sec1;\
    nsec = nsec2 - nsec1;\

#ifdef CHX_FAMILY
static CPSS_REG_VALUE_INFO_STC defaultRegValInfoList[] = GT_DUMMY_REG_VAL_INFO_LIST;
#else /*CHX_FAMILY*/
static CPSS_REG_VALUE_INFO_STC defaultRegValInfoList[] = {{0}};
#endif /*CHX_FAMILY*/

CPSS_DXCH_PORT_MAP_STC  *cpssAppPlatformPortMapPtr = NULL;
GT_U32 cpssAppPlatformPortMapSize = 0;

/* next info not saved in CPSS, Saving locally, for fast retrieval */
GT_U32 cpssCapMaxPortNum[CPSS_APP_PLATFORM_MAX_PP_CNS];

GT_STATUS prvEventMaskSet
(
    IN GT_U8 devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
);

extern GT_VOID cpssAppPlatformBelly2BellyEnable(IN GT_BOOL enable);

extern GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);

#ifdef LINUX_NOKM
extern GT_STATUS prvNoKmDrv_configure_dma_per_devNum
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

extern GT_STATUS prvNoKmDrv_configure_dma_over_the_pci(
    IN GT_U8    devNum
);

extern GT_STATUS prvNoKmDrv_configure_dma_internal_cpu(
    IN GT_U8    devNum
);

#endif

#ifdef ASIC_SIMULATION
extern GT_STATUS cpssSimSoftResetDoneWait(void);
#endif

/*********************************** Phy APIs ************************************/

BOOLEAN cpssAppPlatformPhyOsDelay
(
     IN UINT_32 delay
)
{
    struct timespec sleep, remain;
    if (1000000000 < delay) {
    /* sleep time to big */
        return 0;
    }

    sleep.tv_nsec = delay*1000000;
    sleep.tv_sec = 0;
    if (nanosleep(  &sleep, &remain)) {
        return TRUE;
    }

    return FALSE;
}

void * cpssAppPlatformPhyOsMalloc
(
     UINT_32  size
)
{
        return cpssOsMalloc(size);
}
void cpssAppPlatformPhyOsFree
(
     void* data
)
{
    return cpssOsFree(data);
}

BOOLEAN cpssAppPlatformPhyXsmiMdioWrite (
    IN UINT_32      rel_ifIndex,
    IN UINT_8       mdioAddress,
    IN UINT_16      deviceOrPage,   /* PRV_MPD_IGNORE_PAGE_CNS */
    IN UINT_16      address,
    IN UINT_16      value
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    status = cpssXsmiPortGroupRegisterWrite( dev,
                                             portGroupsBmp,
                                             xsmiInterface,
                                             mdioAddress,
                                             address,
                                             deviceOrPage,
                                             value);
    if (status != GT_OK){
        return FALSE;
    }
    return TRUE;
}

BOOLEAN cpssAppPlatformPhyXsmiMdioRead (
        IN  UINT_32      rel_ifIndex,
        IN  UINT_8       mdioAddress,
        IN  UINT_16      deviceOrPage,
        IN  UINT_16      address,
        OUT UINT_16    * value_PTR
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    status = cpssXsmiPortGroupRegisterRead( dev,
                                            portGroupsBmp,
                                            xsmiInterface,
                                            mdioAddress,
                                            address,
                                            deviceOrPage,
                                            value_PTR);
    if (status != GT_OK){
        return FALSE;
    }
    return TRUE;
}

/* mpdSampleGetFwFiles */
BOOLEAN cpssAppPlatformPhyGetFwFiles
(
    IN  MPD_TYPE_ENT          phyType,
    OUT MPD_FW_FILE_STC       * mainFile_PTR
)
{
    char * fwFileName_PTR = NULL;
    FILE * fp;
    int ret;
    UINT_32 fileSize;

    switch(phyType)
    {
        case MPD_TYPE_88E2540_E:
/*            slaveFileName_PTR = "x3240flashdlslave_0_6_6_0_10673.hdr"; */
            fwFileName_PTR = "v0A080000_11386_11374_e2540.hdr";
            break;
        case MPD_TYPE_88X3540_E:
            fwFileName_PTR = "samples/resources/v0A030000_10979_10954_x3540.hdr";
            break;
        case MPD_TYPE_88X32x0_E:
            fwFileName_PTR    = "88X32xx-FW.hdr";
            break;
        case MPD_TYPE_88X33x0_E:
             if(vendorId == MRVL_PHY_UID_CNS)
             {
                fwFileName_PTR    = "88X33xx-Z2-FW.hdr";
             }
             else if(vendorId == MRVL_PHY_REV_A_UID_CNS)
             {
                 fwFileName_PTR = "x3310fw_0_2_8_0_8850.hdr";
             }
             break;
        case MPD_TYPE_88X2180_E:
            if(vendorId == MRVL_PHY_REV_A_UID_CNS)
            {
                fwFileName_PTR = "e21x0fw_8_0_0_36_8923.hdr";
            }
            break;
        default:
            cpssOsPrintf("not supported PHY type\n");
            break;
    }
    if (fwFileName_PTR == NULL)
    {
            return FALSE;
    }

    fp = fopen(fwFileName_PTR,"r");
    if (fp)
    {
        fseek(fp, 0 , SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);
        mainFile_PTR->dataSize = fileSize;
        mainFile_PTR->data_PTR = cpssAppPlatformPhyOsMalloc(fileSize);

        if (mainFile_PTR->data_PTR == NULL)
        {
            fclose(fp);
            return FALSE;
        }
        ret = fread(mainFile_PTR->data_PTR, sizeof(char), fileSize, fp);
        fclose(fp);
        if (ret <= 0)
        {
            cpssOsPrintf("Read from file fail\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    return TRUE;
}

MPD_RESULT_ENT cpssAppPlatformPhyHandleFailure(
        IN UINT_32                              rel_ifIndex,
        IN MPD_ERROR_SEVERITY_ENT       severity,
        IN UINT_32                                      line,
        IN const char                           * calling_func_PTR,
        IN const char                           * error_text_PTR
)
{
     char * severityText[MPD_ERROR_SEVERITY_MINOR_E+1] = {"Fatal", "Error", "Minor"};
     cpssOsPrintf("Failure Level [%s] on port [%d] line [%d] called by [%s].  %s\n",
                (severity<=MPD_ERROR_SEVERITY_MINOR_E)?severityText[severity]:"Unknown",
                 rel_ifIndex,
                 line,
                 calling_func_PTR,
                 error_text_PTR);

        if (severity == MPD_ERROR_SEVERITY_FATAL_E){
            cpssOsPrintf("Fatal\n");
        }

        return MPD_OK_E;
}

BOOLEAN cpssAppPlatformPhyDebugBind (
    IN const char   * comp_PTR,
        IN const char   * pkg_PTR,
        IN const char   * flag_PTR,
        IN const char   * help_PTR,
    OUT UINT_32         * flagId_PTR
)
{
    UINT_32 len = 0,index;

    if (MPD_SAMPLE_DEBUG_FREE_INDEX >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS) {
            /* no more debug flags */
            return FALSE;
    }
    index = MPD_SAMPLE_DEBUG_FREE_INDEX++;
    mpdSampleDebugFlags[index].isFree = FALSE;
    mpdSampleDebugFlags[index].flagStatus = FALSE;
#if 0  /* Enable for logs */
    if(!strcmp(flag_PTR, "write") || !strcmp(flag_PTR, "read"))
    {
        mpdSampleDebugFlags[index].flagStatus = TRUE;
    }
#endif
    if(strcmp(flag_PTR,"fw") == 0)
    {
        /*cpssOsPrintf("Firmware logs enabled\n");*/
        mpdSampleDebugFlags[index].flagStatus = TRUE;
    }
    len += (comp_PTR != NULL)?strlen(comp_PTR):0;
    len += (pkg_PTR != NULL)?strlen(pkg_PTR):0;
    len += (flag_PTR != NULL)?strlen(flag_PTR):0;

    if (len)
    {
        len += 6;
        mpdSampleDebugFlags[index].flagName_PTR = cpssAppPlatformPhyOsMalloc(len+1/*room for '\0'*/);
        mpdSampleDebugFlags[index].flagName_PTR[0] = '\0';
        strcat(mpdSampleDebugFlags[index].flagName_PTR,comp_PTR);
        strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
        strcat(mpdSampleDebugFlags[index].flagName_PTR,pkg_PTR);
        strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
        strcat(mpdSampleDebugFlags[index].flagName_PTR,flag_PTR);

    }
    len = (help_PTR != NULL)?strlen(help_PTR):0;
    if (len)
    {
        mpdSampleDebugFlags[index].flagHelp_PTR = cpssAppPlatformPhyOsMalloc(len);
        mpdSampleDebugFlags[index].flagHelp_PTR[0] = '\0';
        strncpy(mpdSampleDebugFlags[index].flagHelp_PTR,help_PTR,len);
    }
    *flagId_PTR = index;
    return TRUE;
}

/**
 * @brief   example implementation of debug check is Flag active
 *
 * @return Active / InActive
 */
BOOLEAN cpssAppPlatformPhyDebugIsActive (
    IN UINT_32  flagId
)
{
    if (flagId >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS)
    {
        return FALSE;
    }
    if (mpdSampleDebugFlags[flagId].isFree == TRUE)
    {
        return FALSE;
    }
    return mpdSampleDebugFlags[flagId].flagStatus;
}

/**
 * @brief   example implementation of debug log (print)
 *
 */

void cpssAppPlatformPhyDebugLog (
    IN const char    * funcName_PTR,
    IN const char    * format_PTR,
    IN ...
)
{

        va_list      argptr;

        printf("%s:",funcName_PTR);
        va_start(argptr, format_PTR);
        vprintf( format_PTR, argptr);
        va_end(argptr);
        printf("\n");
}

GT_STATUS cpssAppPlatformPhyPortPowerdown
(
   GT_SW_DEV_NUM            devNum,
   GT_U32                   portNum,
   GT_BOOL                  enable
)
{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));
    devNum = devNum;

    params.phyDisable.forceLinkDown = enable;

    ret = mpdPerformPhyOperation(portNum,
                                 MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                                 &params);

    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

/**
* @internal cpssAppPlatformPhyPortOperGet function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] mpdIndex         - relIfindex
* @param [out] enable          - CPSS device Number,
* @param [out] speed           - CPSS device Number,
* @param [out] duplex          - CPSS device Number,
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPhyPortOperGet
(
   IN  GT_SW_DEV_NUM            devNum,
   IN  GT_U32                   mpdIndex,
   OUT GT_BOOL                  *enable,
   OUT GT_U32                   *speed,
   OUT GT_BOOL                  *duplex
)
{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));
    devNum = devNum;
    enable = enable;
    speed = speed;
    duplex = duplex;

    ret = mpdPerformPhyOperation(mpdIndex,
                                 MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                                 &params);

    *enable = params.phyInternalOperStatus.isOperStatusUp;
    *speed = params.phyInternalOperStatus.phySpeed;
    *duplex = params.phyInternalOperStatus.duplexMode;

    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}



GT_U32 cpssAppPlatformPhyMpdIndexGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum
)
{
   GT_U32 index;

   for(index = 1; index < nextFreeGlobalIfIndex; index++)
   {
       if(ifIndexInfoArr[index].devNum == devNum &&
          ifIndexInfoArr[index].macNum == portNum)
       {
           return index;
       }
   }
   ifIndexInfoArr[nextFreeGlobalIfIndex].devNum  = devNum;
   ifIndexInfoArr[nextFreeGlobalIfIndex].macNum = portNum;
   return nextFreeGlobalIfIndex++;
}

MPD_TYPE_ENT cpssAppPlatformMpdPhyTypeGet
(
    GT_U32 phyType
)
{
    switch(phyType)
    {
        case MRVL_PHY_2540_MODEL_NUM_CNS:
            return MPD_TYPE_88E2540_E;
        case MRVL_PHY_3240_MODEL_NUM_CNS:
            return MPD_TYPE_88X32x0_E;
        case MRVL_PHY_1780_MODEL_NUM_CNS:
            return MPD_TYPE_88E1780_E;
        case MRVL_PHY_3340_MODEL_NUM_CNS:
            return MPD_TYPE_88X33x0_E;
        case MRVL_PHY_2180_MODEL_NUM_CNS:
            return MPD_TYPE_88X2180_E;
        case MRVL_PHY_7120_MODEL_NUM_CNS:
            return MPD_TYPE_88X7120_E;
        default:
            return MPD_TYPE_INVALID_E;
    }
}


static GT_STATUS cpssAppPlatformPhyPortNumGet
(
    GT_U32  currBoardType,
    IN  CPSS_APP_PLATFORM_XPHY_INFO_STC    phyInfo,
    OUT GT_U32                      *portNum
)
{
    GT_U32  i;
    GT_U32  portStart = 0;
    GT_U32  portEnd = 0;
    GT_U16  *portToPhyArray = NULL;

    switch(currBoardType)
    {
       case APP_DEMO_XCAT5X_A0_BOARD_RD_CNS:
           if(phyInfo.phyType == MRVL_PHY_2540_MODEL_NUM_CNS)
             {
                 portToPhyArray = &portToPhyArrayAc5xRd[0][0];
                 portStart = 32;
                 portEnd   = 47;
             }
             if(phyInfo.phyType == MRVL_PHY_1780_MODEL_NUM_CNS)
             {
                 portToPhyArray = &portToPhyArrayAc5xRd1780[0][0];
                 portStart = 0;
                 portEnd   = 31;
             }
            break;
        default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    for(i = 0; portToPhyArray[i] != 0xff ; i=i+2)
    {
        if(portToPhyArray[i] < portStart || portToPhyArray[i]  > portEnd)
        {
            continue;
        }
        if(portToPhyArray[i+1] == phyInfo.phyAddr)
        {
            *portNum = portToPhyArray[i];
        }
     }
    return GT_OK;
}

GT_STATUS cpssAppPlatformXPhyIdGet
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
    /*GT_U16    phyRev = 0;  future use for 2180 phy*/

    /* read PHY ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS, &data);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssSmiRegisterReadShort %d", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {

        /*phyRev = data & 0xF;*/

        *phyIdPtr = (data >> 4) & 0x3F;
        phyModelNum = *phyIdPtr;
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}


GT_STATUS cpssAppPlatformXPhyVendorIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT  GT_U16                    *vendorIdPtr
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
    saved_system_recovery = system_recovery;

    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc =  cpssSystemRecoveryStateSet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* read vendor ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_VENDOR_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS,
                                        /*OUT*/vendorIdPtr);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssXsmiPortGroupRegisterRead %d", rc);
    }
    rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    return rc;
}

GT_VOID cpssAppPlatformXSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT CPSS_APP_PLATFORM_XPHY_INFO_STC    *phyInfoArray,
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
    maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_1_E;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(xsmiInterface = minXSMIinterface; xsmiInterface <= maxXSMIinterface; xsmiInterface++)
        {
            for(xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = cpssAppPlatformXPhyVendorIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssAppPlatformXPhyVendorIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,\n",
                                devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }

                if ((data != MRVL_PHY_UID_CNS) && (data != MRVL_PHY_REV_A_UID_CNS))
                {
                    continue;
                }
                else if ((data == MRVL_PHY_UID_CNS || data == MRVL_PHY_REV_A_UID_CNS) &&
                         checkEnable == GT_TRUE)
                {
                    vendorId = data;
                    checkEnable = GT_FALSE;
                }
                rc = cpssAppPlatformXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssAppPlatformXPhyIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
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
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}

GT_STATUS cpssAppPlatformPhyIdGet
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
        cpssOsPrintf("cpssSmiRegisterReadShort : %d", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {
        *phyIdPtr = (data >> 4) & 0x3F;
        phyModelNum = *phyIdPtr;
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}

GT_VOID cpssAppPlatformSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT CPSS_APP_PLATFORM_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    GT_U32      smiAddr;                /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    *phyInfoArrayLenPtr = 0;

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(smiAddr = 0; smiAddr < BIT_5; smiAddr++)
        {
            rc = cpssAppPlatformPhyIdGet(devNum, portGroupsBmp, CPSS_PHY_SMI_INTERFACE_1_E, smiAddr, &data);
            if(rc != GT_OK)
            {
                cpssOsPrintf("cpssAppPlatformPhyIdGet FAIL:devNum=%d,portGroupId=%d,smiInterface=%d,smiAddr=%d\n",
                        devNum, portGroupId, CPSS_PHY_SMI_INTERFACE_1_E, smiAddr);
                continue;
            }

            phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
            phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)smiAddr;
            phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = CPSS_PHY_SMI_INTERFACE_1_E;
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
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        return;
}



GT_STATUS cpssAppPlatformPhyMpdInit
(
    GT_U8 devNum
)
{
    UINT_32                                     rel_ifIndex, i;
    MPD_RESULT_ENT                              rc = MPD_OK_E;
    MPD_CALLBACKS_STC                           phy_callbacks;
    MPD_PORT_INIT_DB_STC                        phy_entry;
    GT_U32                                      portNum = 0;
    CPSS_APP_PLATFORM_XPHY_INFO_STC             phyInfoArray[256];
    CPSS_APP_PLATFORM_XPHY_INFO_STC             smiPhyInfoArray[256];
    GT_U32                                      phyInfoArrayLen = 0;
    GT_U32                                      smiPhyInfoArrayLen = 0;
    GT_U32                                      currBoardType = APP_DEMO_XCAT5X_A0_BOARD_RD_CNS;
    MPD_TYPE_ENT                                phyType;
    GT_U32                                      phyNumber = 0;

    memset(phyInfoArray, 0, sizeof(phyInfoArray));
    memset(smiPhyInfoArray, 0, sizeof(smiPhyInfoArray));

    cpssAppPlatformXSmiPhyAddrArrayBuild(devNum, phyInfoArray, &phyInfoArrayLen);
    cpssAppPlatformSmiPhyAddrArrayBuild(devNum, smiPhyInfoArray, &smiPhyInfoArrayLen);

    /* bind basic callbacks needed from host application */
    memset(&phy_callbacks,0,sizeof(phy_callbacks));

    phy_callbacks.sleep_PTR               = cpssAppPlatformPhyOsDelay;
    phy_callbacks.alloc_PTR               = cpssAppPlatformPhyOsMalloc;
    phy_callbacks.free_PTR                = cpssAppPlatformPhyOsFree;
    phy_callbacks.debug_bind_PTR          = cpssAppPlatformPhyDebugBind;
    phy_callbacks.is_active_PTR           = cpssAppPlatformPhyDebugIsActive;
    phy_callbacks.debug_log_PTR           = cpssAppPlatformPhyDebugLog;
    phy_callbacks.txEnable_PTR            = NULL;
    phy_callbacks.handle_failure_PTR      = cpssAppPlatformPhyHandleFailure;
    phy_callbacks.getFwFiles_PTR          = cpssAppPlatformPhyGetFwFiles;
    phy_callbacks.mdioRead_PTR            = cpssAppPlatformPhyXsmiMdioRead;
    phy_callbacks.mdioWrite_PTR           = cpssAppPlatformPhyXsmiMdioWrite;

    rc = genRegisterSet(0, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF);
    if (rc != GT_OK)
    {
        cpssOsPrintf("genRegisterSet(0, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF): rc = %d\n", rc);
        return rc;
    }

    rc = mpdDriverInitDb(&phy_callbacks);
    if (rc != MPD_OK_E) {
        cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in mpdDriverInitDb \n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < phyInfoArrayLen ; i++)
    {
        rc = cpssAppPlatformPhyPortNumGet(currBoardType, phyInfoArray[i], &portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssAppPlatformPhyMpdInit failed \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        phyType = cpssAppPlatformMpdPhyTypeGet(phyInfoArray[i].phyType);

        memset(&phy_entry,0,sizeof(phy_entry));
        phy_entry.port                          = portNum;
        phy_entry.phyNumber                     = phyNumber;
        phy_entry.phyType                       = phyType;

        phy_entry.transceiverType               = MPD_TRANSCEIVER_COPPER_E;

        phy_entry.mdioInfo.mdioAddress          = phyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = phyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = phyInfoArray[i].xsmiInterface;

        if(phy_entry.phyType == MPD_TYPE_88E2540_E)
        {
            phy_entry.phySlice                  = (phyInfoArray[i].phyAddr % 4);
            phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_OXGMII;
        }
        else
        {
            phy_entry.phySlice                  = portNum;
        }

        rel_ifIndex = cpssAppPlatformPhyMpdIndexGet(devNum, portNum);
        if (rel_ifIndex < 12)
        {

            if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
                cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in mpdPortDbUpdate\n");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    for(i = 0; i < smiPhyInfoArrayLen; i++)
    {
        rc = cpssAppPlatformPhyPortNumGet(currBoardType, smiPhyInfoArray[i], &portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        phyType = cpssAppPlatformMpdPhyTypeGet(smiPhyInfoArray[i].phyType);
        memset(&phy_entry,0,sizeof(phy_entry));
        phy_entry.port                          = portNum;
        phy_entry.phyNumber                     = phyNumber /*portNum/8*/;
        phy_entry.phyType                       = phyType;

        phy_entry.transceiverType               = MPD_TRANSCEIVER_COPPER_E;

        phy_entry.mdioInfo.mdioAddress          = smiPhyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = smiPhyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = smiPhyInfoArray[i].xsmiInterface;
        phy_entry.phySlice                      = (smiPhyInfoArray[i].phyAddr % 4);
        phy_entry.usxInfo.usxType               = MPD_PHY_USX_TYPE_OXGMII;

        rel_ifIndex = cpssAppPlatformPhyMpdIndexGet(devNum, portNum);
        if (rel_ifIndex < 12)
        {
            if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
                cpssOsPrintf("gtAppDemoPhyMpdInit failed in mpdPortDbUpdate\n");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    }

    for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++)
    {
        prvMpdGlobalDb_PTR->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_RAM_E;
    }
    cpssOsPrintf("MPD PHY initialize........ \n");

    if (mpdDriverInitHw() != MPD_OK_E)
    {
        cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in mpdDriverInitHw");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    cpssOsPrintf("MPD PHY initialized........ \n");

    for(i = 0; i < phyInfoArrayLen ; i++)
    {
        rc = cpssAppPlatformPhyPortNumGet(currBoardType, phyInfoArray[i], &portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssAppPlatformPhyMpdInit failed \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        phyType = cpssAppPlatformMpdPhyTypeGet(phyInfoArray[i].phyType);

        if (phyType == MPD_TYPE_88E2540_E)
        {
            rel_ifIndex = cpssAppPlatformPhyMpdIndexGet(devNum, portNum);

            if (rel_ifIndex < 12)
            {
                rc = cpssAppPlatformPhyPortPowerdown(devNum, rel_ifIndex, GT_FALSE);
                if (rc != MPD_OK_E)
                {
                    cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in cpssAppPlatformPhyPortPowerdown\n");
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
        }
    }

    return GT_OK;
}

/*****************reInit Handling for unexpected app exit***************/

static GT_STATUS    doDeviceResetAndRemove(IN GT_U8   devNum,GT_BOOL includePex, IN GT_U32 pciBusNum, IN GT_U32 pciIdSel, IN GT_U32 funcNo)
{
    GT_STATUS   rc;

#ifndef SUPPORT_PCIe_RESCAN
    /* the environment just not supports PCIe rescan , so do not allow to reset the device with the PCIe */
    includePex = GT_FALSE;
#endif /*SUPPORT_PCIe_RESCAN*/

    /* Disable All Skip Reset options ,exclude PEX */
    /* this Enable Skip Reset for PEX */
    if( GT_FALSE == includePex )
    {
        osPrintf("doDeviceResetAndRemove : NO PEX reset !!! \n");

        rc = cpssDxChHwPpSoftResetSkipParamSet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    cpssOsPrintf("PCIe info :pciBusNum[%d],pciIdSel[%d],funcNo[%d] \n",
        pciBusNum,pciIdSel,funcNo);

    /* provide time to finish printings */
    osTimerWkAfter(100);

    rc = cpssDxChHwPpSoftResetTrigger(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef ASIC_SIMULATION
    rc = cpssSimSoftResetDoneWait();
#endif

#ifndef ASIC_SIMULATION
#ifdef SUPPORT_PCIe_RESCAN
/*SUPPORT_PCIe_RESCAN*/
    if(GT_TRUE == includePex)
    {
        CPSS_HW_INFO_STC hwInfo;
        osMemSet(&hwInfo, 0, sizeof(CPSS_HW_INFO_STC));

        rc = extDrvPexRemove(
                    pciBusNum,
                    pciIdSel,
                    funcNo);

        rc = extDrvPexRescan(
                   pciBusNum,
                   pciIdSel,
                   funcNo,
                   &hwInfo);
    }
/*SUPPORT_PCIe_RESCAN*/
#endif
#endif

    rc = cpssDxChCfgDevRemove(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS    doAtomicDeviceResetAndRemove(IN GT_U8 devNum, IN GT_U32 pciBusNum, IN GT_U32 pciIdSel, IN GT_U32 funcNo)
{
    GT_STATUS   rc;
    GT_BOOL     pexReset = GT_FALSE;

    /* make 'atomic' protection on the CPSS APIs that access this device that will do reset + remove */
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(GT_TRUE == PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /*Reset PEX for SIP6 only */
        pexReset = GT_TRUE;
#ifdef ASIC_SIMULATION_ENV_FORBIDDEN
        /* WM-ASIM environment , the ASIM not supports reset with the PCIe */
        pexReset = GT_FALSE;
#endif /*ASIC_SIMULATION_ENV_FORBIDDEN*/
    }


    rc = doDeviceResetAndRemove(devNum,pexReset, pciBusNum, pciIdSel, funcNo);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*****************reInit Handling for unexpected app exit***************/

/**
* @internal cpssAppPlatformPhase1Init function
* @endinternal
*
* @brief   Phase1 PP configurations
*
* @param [in] *ppMapPtr          - PCI/SMI info of the device,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPhase1Init
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS                         rc = GT_OK;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC cpssPpPhase1 = {0};
    CPSS_PP_DEVICE_TYPE               ppDevType;
    CPSS_REG_VALUE_INFO_STC          *regCfgList;
    GT_U32                            regCfgListSize;
    GT_U32                            pciConfigFlag;
    PCI_INFO_STC                      pciInfo;
    CPSS_PP_FAMILY_TYPE_ENT           ppDevFamily;
    GT_U32                            phase1InitCounter = 0;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!ppMapPtr || !ppProfilePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    if (ppMapPtr->mapType != CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("mapType=%d not supported.\n", ppMapPtr->mapType);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(!ppProfilePtr->internalCpu)
    {
        /* Get the PCI info specific device on bus,dev with devNum*/
        rc = prvCpssAppPlatformSysGetPciInfo(ppMapPtr->devNum, ppMapPtr->mngInterfaceAddr.pciAddr.busId, ppMapPtr->mngInterfaceAddr.pciAddr.deviceId, ppMapPtr->mngInterfaceAddr.pciAddr.functionId, &ppDevFamily, &pciInfo);
        if(rc != GT_OK)
        {
            if(rc == GT_NO_MORE)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("Prestera Device on pciBusNum:%d Not Present\n", ppMapPtr->mngInterfaceAddr.pciAddr.busId);
            } else {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("prvCpssAppPlatformSysGetPciInfo rc=%d\n", rc);
            }
            return rc;
        }

        switch (ppDevFamily)
        {
           case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
           case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
           case CPSS_PP_FAMILY_DXCH_AC3X_E:
              pciConfigFlag = MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E;
              break;

           case CPSS_PP_FAMILY_DXCH_FALCON_E:
           case CPSS_PP_FAMILY_DXCH_AC5X_E:
              pciConfigFlag = MV_EXT_DRV_CFG_FLAG_EAGLE_E;
              break;

           default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* WA For AC5X */
        if (ppDevFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
            prvCpssDrvHwPpPrePhase1NextDevFamilySet(ppDevFamily);

        rc = extDrvPexConfigure(pciInfo.pciBus, pciInfo.pciDev, pciInfo.pciFunc,
                                pciConfigFlag, &(cpssPpPhase1.hwInfo[0]));
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPexConfigure);

    }
    else
    {
        /* Internal CPU WA for AC5x */
        ppDevFamily = CPSS_PP_FAMILY_DXCH_AC5X_E;
        pciConfigFlag = MV_EXT_DRV_CFG_FLAG_EAGLE_E;
        pciInfo.pciDev = 0xFF;
        pciInfo.pciFunc = 0xFF;

        /* WA For AC5X */
        if (ppDevFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
            prvCpssDrvHwPpPrePhase1NextDevFamilySet(ppDevFamily);

        rc = extDrvPexConfigure(0xFFFF, pciInfo.pciDev, pciInfo.pciFunc,/*pciBus value to be supplied as 0xFFFF */
                                pciConfigFlag | (MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5X_ID_CNS << MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS) , &(cpssPpPhase1.hwInfo[0]));
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPexConfigure);

    }

    cpssPpPhase1.devNum                      = ppMapPtr->devNum;
    cpssPpPhase1.mngInterfaceType            = ppMapPtr->mngInterfaceType;

    cpssPpPhase1.coreClock                   = ppProfilePtr->coreClock;
    cpssPpPhase1.serdesRefClock              = ppProfilePtr->serdesRefClock;
    cpssPpPhase1.isrAddrCompletionRegionsBmp = ppProfilePtr->isrAddrCompletionRegionsBmp;
    cpssPpPhase1.appAddrCompletionRegionsBmp = ppProfilePtr->appAddrCompletionRegionsBmp;
    cpssPpPhase1.numOfPortGroups             = ppProfilePtr->numOfPortGroups;
    cpssPpPhase1.tcamParityCalcEnable        = ppProfilePtr->tcamParityCalcEnable;
    cpssPpPhase1.numOfDataIntegrityElements  = ppProfilePtr->numOfDataIntegrityElements;
    cpssPpPhase1.maxNumOfPhyPortsToUse       = ppProfilePtr->maxNumOfPhyPortsToUse;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("ApEnabled %d\n",  ppProfilePtr->apEnable);
    rc = cpssDxChPortApEnableSet(ppMapPtr->devNum, 0x1, ppProfilePtr->apEnable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortApEnableSet);

phase_1_init_lbl:
    phase1InitCounter++;
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1, &ppDevType);
    if(rc == GT_HW_ERROR_NEED_RESET )
    {
        doAtomicDeviceResetAndRemove(CAST_SW_DEVNUM(ppMapPtr->devNum), pciInfo.pciBus, pciInfo.pciDev, pciInfo.pciFunc);
        osTimerWkAfter(100);

        if(phase1InitCounter < 5)
        {
            cpssOsPrintf("loop iteration [%d] : call again to 'phase-1' init \n",
                phase1InitCounter);
            goto  phase_1_init_lbl;
        }
        else
        {
            rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1, &ppDevType);
        }
    }
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpPhase1Init);

    /* Change miscellaneous MPP configurations */
    if (CPSS_PP_FAMILY_DXCH_AC5X_E == ppDevFamily)
    {
       /* MPP 19 is used as OUT for DEV_INIT_DONE signal.
        This signal is connected to RESET input of external CPU cards on DB board.
        AC5X Device toggles DEV_INIT_DONE during Soft Reset and this resets external CPU.
        Following configuration changes MPP settings to connecte constant value '1' to MPP 19..
        And external CPU will not get reset. */

        /* set output for GPIO 19 to be 1 */
        rc = prvCpssDrvHwPpSetRegField(ppMapPtr->devNum, 0x7F018130, 19, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable output for GPIO 19 */
        rc = prvCpssDrvHwPpSetRegField(ppMapPtr->devNum, 0x7F01812C, 19, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* set MPP 19 to be GPIO 19 */
        rc = prvCpssDrvHwPpSetRegField(ppMapPtr->devNum, 0x80020108, 12, 4, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

#ifdef LINUX_NOKM
    /* How to profile? Or should it be profiled */
    if (CPSS_PP_FAMILY_DXCH_FALCON_E == ppDevFamily)
    {
       rc = prvNoKmDrv_configure_dma_per_devNum(CAST_SW_DEVNUM(ppMapPtr->devNum),0/*portGroup*/);
       CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_configure_dma_per_devNum);
    }

    if (CPSS_PP_FAMILY_DXCH_AC5X_E == ppDevFamily)
    {
        if(ppProfilePtr->internalCpu)
        {
            rc = prvNoKmDrv_configure_dma_internal_cpu(CAST_SW_DEVNUM(ppMapPtr->devNum));
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_configure_dma_internal_cpu);
        }
        else
        {
            rc = prvNoKmDrv_configure_dma_over_the_pci(CAST_SW_DEVNUM(ppMapPtr->devNum));
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_configure_dma_over_the_pci);
        }
    }

#endif

    regCfgList     = defaultRegValInfoList;
    regCfgListSize = sizeof(defaultRegValInfoList) / sizeof(CPSS_REG_VALUE_INFO_STC);

    rc = cpssDxChHwPpStartInit(ppMapPtr->devNum, regCfgList, regCfgListSize);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpStartInit);

    return rc;
}

/************** 100/400G WA ****************/
/* by default the WA is not disabled */
static GT_U32   falcon_initPortDelete_WA_disabled = 0;

extern GT_STATUS prvFalconPortDeleteWa_enableSet(IN GT_U32 waNeeded);

/* allow to init with/without the WA for the 'port delete' */
GT_STATUS   falcon_initPortDelete_WA_set(IN GT_U32  waNeeded)
{
    falcon_initPortDelete_WA_disabled = !waNeeded;

    /* notify the CPSS that not need the WA */
    (void) prvFalconPortDeleteWa_enableSet(waNeeded);

    return GT_OK;
}

typedef struct{
    GT_U32  sdmaPort;
    GT_U32  macPort;
}MUX_DMA_STC;
/* MUXING info */
static MUX_DMA_STC falcon_3_2_muxed_SDMAs[]  = {{ 68, 66},{ 70, 67},{GT_NA,GT_NA}};
static MUX_DMA_STC falcon_6_4_muxed_SDMAs[]  = {{136,130},{138,131},{140,132},{142,133},{GT_NA,GT_NA}};
static MUX_DMA_STC falcon_12_8_muxed_SDMAs[] = {{272,258},{274,259},{276,260},{278,261},{280,266},{282,267},{284,268},{286,269},{GT_NA,GT_NA}};

typedef struct{
    GT_U32   firstDma;
    GT_U32   numPorts;
}SDMA_RANGE_INFO_STC;

static GT_U32   reservedCpuSdmaGlobalQueue[2] = {0};

/**
* @internal falcon_initPortDelete_WA function
* @endinternal
*
*   purpose : state CPSS what reserved resources it should use for the 'port delete' WA.
*   actually the WA is activated during 'cpssDxChPortModeSpeedSet(...,powerUp = GT_FALSE)'
*   NOTE: the function gets the same parameters as the cpssDxChPortPhysicalPortMapSet(...)
*       because we need to find non-used 'physical port number' and to 'steal'
*       one SDMA queue number.
*/
static GT_STATUS falcon_initPortDelete_WA
(
    IN GT_SW_DEV_NUM       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC      portMapArrayPtr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    CPSS_DXCH_PORT_MAP_STC      *currPtr = &portMapArrayPtr[0];
    CPSS_PORTS_BMP_STC          usedPhyPorts;
    MUX_DMA_STC                 *muxed_SDMAs_Ptr;
    SDMA_RANGE_INFO_STC         falcon_3_2_SDMAs_available  = { 64 +  4 ,  4};
    SDMA_RANGE_INFO_STC         falcon_6_4_SDMAs_available  = {128 +  8 ,  8};
    SDMA_RANGE_INFO_STC         falcon_12_8_SDMAs_available = {256 + 16 , 16};
    SDMA_RANGE_INFO_STC         *SDMAs_available_Ptr;
    CPSS_PORTS_BMP_STC          availableSDMAPorts;/* note : this is BMP of MAC/DMA ... not of physical ports */
    GT_U32                      maxPhyPorts;
    CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC    waInfo;
    GT_U32                      numOfMappedSdma;
    GT_U32                      numCpuSdmas = 1;

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&usedPhyPorts);
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&availableSDMAPorts);

    muxed_SDMAs_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? falcon_12_8_muxed_SDMAs :
                      PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? falcon_6_4_muxed_SDMAs  :
                                                   /* single tile*/        falcon_3_2_muxed_SDMAs  ;

    SDMAs_available_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? &falcon_12_8_SDMAs_available :
                          PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? &falcon_6_4_SDMAs_available  :
                                                       /* single tile*/        &falcon_3_2_SDMAs_available  ;

    /**********************************************************************/
    /********** start by search for CPU SDMA to be used by the WA *********/
    /**********************************************************************/

    /* build BMP of available SDMA ports that can be used */
    for(ii = SDMAs_available_Ptr->firstDma ;
        ii < SDMAs_available_Ptr->firstDma + SDMAs_available_Ptr->numPorts ;
        ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&availableSDMAPorts, ii);
    }

    for(ii = 0 ; ii < portMapArraySize; ii++,currPtr++)
    {
        /* state that this physical port is used */
        CPSS_PORTS_BMP_PORT_SET_MAC(&usedPhyPorts, currPtr->physicalPortNumber);

        /* for CPU SDMA , find the highest 'physical port number' and still it's 'queue 7' */
        if(currPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* state that this DMA port is used as CPU SDMA */
            /* note : this is BMP of DMAs ... not of physical ports */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, currPtr->interfaceNum);
        }
        else
        {
            for(jj = 0 ; muxed_SDMAs_Ptr[jj].sdmaPort != GT_NA ; jj++)
            {
                if(currPtr->interfaceNum == muxed_SDMAs_Ptr[jj].macPort)
                {
                    /* since the application uses this MAC that is muxed with the SDMA ...
                       it is not free for the WA */
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, muxed_SDMAs_Ptr[jj].sdmaPort);
                    break;
                }
            }
        }
    }

    if(PRV_CPSS_PORTS_BMP_IS_ZERO_MAC(&availableSDMAPorts))
    {
        /* the application uses ALL the SDMAs that the device can offer */
        /* so we will steal the 'queue 7,6' of the 'last one' */
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numCpuSdmas * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numCpuSdmas * 8) - 2;
    }
    else
    {
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfMappedSdma);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* the highest used SDMAs, not used SDMAs not supporte by WA implementation */
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numOfMappedSdma * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numOfMappedSdma * 8) - 2;
        /*  next not supported (yet) by the CPSS
            waInfo.reservedCpuSdmaGlobalQueue[0]         = 0xFFFFFFFF;
            waInfo.reservedCpuSdmaGlobalQueue[1]         = 0xFFFFFFFF;
        */
    }

    if(waInfo.reservedCpuSdmaGlobalQueue[0] == 7)/* single CPU port */
    {
        waInfo.reservedCpuSdmaGlobalQueue[0] = 6;/* the '7' is reserved for all 'from cpu' tests (LUA/enh-UT) */
        waInfo.reservedCpuSdmaGlobalQueue[1] = 5;
    }

    /* saved for later use */
    reservedCpuSdmaGlobalQueue[0] = waInfo.reservedCpuSdmaGlobalQueue[0];
    reservedCpuSdmaGlobalQueue[1] = waInfo.reservedCpuSdmaGlobalQueue[1];

    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts , 61))
    {
        /* check to use 61 as 'default' because it stated as 'special port' that
           should not be used by the application in the FS */
        /* but it is for the L2 traffic that is used by the WA */
        waInfo.reservedPortNum = 61;/* use 61 regardless to the port mode (64/128/512/1024) */
    }
    else
    {
        /**********************************************************************/
        /********** start the search for physical port to be used by the WA ***/
        /**********************************************************************/
        for(ii = maxPhyPorts-1 ; ii ; ii--)
        {
            if(ii == CPSS_CPU_PORT_NUM_CNS ||
               ii == CPSS_NULL_PORT_NUM_CNS)
            {
                /* not valid numbers to use ! */
                continue;
            }

            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts , ii))
            {
                /* not used */
                break;
            }
        }

        if(ii == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        waInfo.reservedPortNum = ii;
    }

    /*************************************************/
    /********** State the tail drop profile to use ***/
    /*************************************************/
    waInfo.reservedTailDropProfile = 15;


    /**********************************/
    /* call the CPSS to have the info */
    /**********************************/
    rc = cpssDxChHwPpImplementWaInit_FalconPortDelete(CAST_SW_DEVNUM(devNum),&waInfo);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}
/************** 100/400G WA ****************/

/*
* @internal cpssAppPlatformPpPortsInit function
* @endinternal
*
* @brief  Set port mapping. (after Phase1 Init)
*
* @param[in] devNum              - CPSS device number.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpPortsInit
(
   IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr

)
{
    GT_STATUS               rc;
    GT_U32                  mapArrLen = 0;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr   = NULL;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if(ppMapPtr->portMap != NULL)
    {
        mapArrLen = ppMapPtr->portMapSize;
        mapArrPtr = ppMapPtr->portMap;
    }
    else
    {
        rc = cpssAppPlatformPpPortMapGet(ppMapPtr->devNum, &mapArrPtr, &mapArrLen);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpPortMapGet);
    }

    if (mapArrPtr && mapArrLen)
    {
        GT_U32                  i;
        GT_U32                  maxPortNum  = 0;
       /* Find Max Port Number in this mapArray and store in our per Device DB for future use */
       for (i=0; i < mapArrLen; i++)
          if (mapArrPtr[i].physicalPortNumber > maxPortNum)
              maxPortNum = mapArrPtr[i].physicalPortNumber;

       rc = cpssDxChPortPhysicalPortMapSet(ppMapPtr->devNum, mapArrLen, mapArrPtr);
       CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapSet);

       cpssCapMaxPortNum[ppMapPtr->devNum] = maxPortNum;
       CPSS_APP_PLATFORM_LOG_INFO_MAC("Max Port Number [%d] = %d.\n", ppMapPtr->devNum, maxPortNum);

       if(!falcon_initPortDelete_WA_disabled && PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)/* if the WA initialization is not disabled */
       {
           /****************************************************************************/
           /* do init for the 'port delete' WA.                                        */
           /* with the same parameters as the cpssDxChPortPhysicalPortMapSet(...) used */
           /****************************************************************************/
           rc = falcon_initPortDelete_WA(ppMapPtr->devNum, mapArrLen, mapArrPtr);
           CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, falcon_initPortDelete_WA);
       }
    }


#if 0
    /* TODO */
    /* restore OOB port configuration after systemReset */
    if ((bc2BoardResetDone == GT_TRUE) && appDemoBc2IsInternalCpuEnabled(devNum) &&  (!isBobkBoard))
    {
        MV_HWS_SERDES_CONFIG_STR    serdesConfig;

        /* configure SERDES TX interface */
        rc = mvHwsSerdesTxIfSelect(devNum, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, COM_PHY_28NM, 1);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, mvHwsSerdesTxIfSelect);

        serdesConfig.baudRate = _1_25G;
        serdesConfig.media = XAUI_MEDIA;
        serdesConfig.busWidth = _10BIT_ON;
        serdesConfig.refClock = _156dot25Mhz;
        serdesConfig.refClockSource = PRIMARY;
        serdesConfig.rxEncoding = SERDES_ENCODING_NA;
        serdesConfig.serdesType = COM_PHY_28NM;
        serdesConfig.txEncoding = SERDES_ENCODING_NA;

        /* power UP and configure SERDES */
        rc = mvHwsSerdesPowerCtrl(devNum, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, GT_TRUE, &serdesConfig);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, mvHwsSerdesPowerCtrl);
    }
#endif

    /* set the EGF to filter traffic to ports that are 'link down'.
       state that all ports are currently 'link down' (except for 'CPU PORT') */
    rc = cpssDxChPortManagerInit(ppMapPtr->devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortManagerInit);
    return rc;
}

/**
* @internal cpssAppPlatformAfterPhase2Init function
* @endinternal
*
* @brief   After phase2 PP configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS cpssAppPlatformAfterPhase2Init
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_PHYSICAL_PORT_NUM  port;   /* port number */
    GT_STATUS             rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

#ifndef ASIC_SIMULATION
    /******************************************************/
                    CPSS_TBD_BOOKMARK_BOBCAT3
    /* BC3 board got stuck - code should be checked later */
    /******************************************************/
#if 0
/* TODO LATER */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssSystemRecoveryStateGet);

    if(system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            rc = gtApplicationPlatformXPhyFwDownload(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, gtApplicationPlatformXPhyFwDownload);
        }
    }
#endif
#endif

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
    {
        for(port = 0; port < CPSS_MAX_PORTS_NUM_CNS;port++)
        {
            if( !(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PP_MAC(devNum)->existingPorts, port)))
            {
                continue;
            }

            /* split ports between MC FIFOs for Multicast arbiter */
            rc = cpssDxChPortTxMcFifoSet(devNum, port, port%2);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
        }
    }
    /* Init LED interfaces */
    rc = appPlatformLedInterfacesInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformLedInterfacesInit);
    CPSS_APP_PLATFORM_LOG_INFO_MAC("After phase2 done...\n");
    return rc;
}

/*
* @internal cpssAppPlatformPpHwInit function
* @endinternal
*
* @brief   initialize Pp phase1 init and WAs.
*
* @param [in] *ppMapPtr          - PCI/SMI info of the device,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpHwInit
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr
)
{
    GT_STATUS                                       rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformPhase1Init(ppMapPtr, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPhase1Init);

    /* Wa Init */
    if(ppProfilePtr->cpssWaNum)
    {
        rc = cpssDxChHwPpImplementWaInit(ppMapPtr->devNum, ppProfilePtr->cpssWaNum, &ppProfilePtr->cpssWaList[0], NULL);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpImplementWaInit);
    }

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        rc = cpssAppPlatformEzbDevInfoSerdesGet(ppMapPtr->devNum, ppMapPtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbDevInfoSerdesGet);
        rc = cpssAppPlatformEzbSerdesPolarityGet(ppMapPtr->devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbSerdesPolarityGet);
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Pp HwInit Done...\n");
    return rc;
}

/**
* @internal internal_cpssAppPlatformPpInsert function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] *ppMapPtr        - PP_MAP board profile,
* @param [in] *ppProfileInfo   - Pp Profile,
* @param [in] *systemRecovery  - System Recovery mode
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
static GT_STATUS internal_cpssAppPlatformPpInsert
(
    IN GT_U8                                     devNum,
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC            *systemRecovery
)
{
    GT_STATUS rc         = GT_OK;
    GT_U32    start_sec  = 0;
    GT_U32    start_nsec = 0;
    GT_U32    end_sec    = 0;
    GT_U32    end_nsec   = 0;
    GT_U32    diff_sec   = 0;
    GT_U32    diff_nsec  = 0;
    (void)systemRecovery;
    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    if (!ppMapPtr || !ppProfilePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    if (ppMapPtr->mapType != CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("mapType=%d not supported.\n", ppMapPtr->mapType);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Device %d Already Exists\n", ppMapPtr->devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* devNum should be 0-31 so this number will be used both as hw device number and cpss device number */
    if (devNum >= CPSS_APP_PLATFORM_MAX_PP_CNS)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Cpss Dev Num is %d - But supported range is 0-%d\n", devNum, CPSS_APP_PLATFORM_MAX_PP_CNS);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssAppPlatformBelly2BellyEnable(ppProfilePtr->belly2belly);

    rc = cpssAppPlatformPpHwInit(ppMapPtr, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpHwInit);

    rc = cpssAppPlatformPpPortsInit(ppMapPtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpPortsInit);

    rc = cpssAppPlatformPhase2Init(ppMapPtr->devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPhase2Init);

    rc = cpssAppPlatformAfterPhase2Init(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformAfterPhase2Init);

    if (ppProfilePtr->initMpd)
    {
        rc = cpssAppPlatformPhyMpdInit(ppMapPtr->devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPhyMpdInit);
    }

    rc = cpssAppPlatformPpLogicalInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpLogicalInit);

    rc = cpssAppPlatformPpLibInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpLibInit);

    rc = cpssAppPlatformPpGeneralInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpGeneralInit);

    rc = cpssAppPlatformAfterInitConfig(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformAfterInitConfig);

    rc = cpssAppUtilsEventHandlerPreInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppUtilsEventHandlerPreInit);

    /* unmask user events for the device */
    rc = prvEventMaskSet(devNum, CPSS_EVENT_UNMASK_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvEventMaskSet);

    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    TIME_DIFF_MAC(start_sec, start_nsec, end_sec, end_nsec, diff_sec, diff_nsec);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppPlatformPpInsert Time is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}

/**
* @internal cpssAppPlatformPpInsert function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] *ppMapPtr        - PP_MAP board profile,
* @param [in] *ppProfileInfo   - Pp Profile,
* @param [in] *systemRecovery  - System Recovery mode
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPpInsert
(
    IN GT_U8                                     devNum,
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC            *systemRecovery
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssAppPlatformPpInsert);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssAppPlatformPpInsert(devNum, ppMapPtr, ppProfilePtr, systemRecovery);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssAppPlatformPpRemove function
* @endinternal
*
* @brief   Remove packet processor.
*
* @param[in] devNum       - Device number.
* @param[in] removalType  - Removal type: 0 - Managed Removal,
*                                         1 - Unmanaged Removal,
*                                         2 - Managed Reset
*
* @retval GT_OK           - on success,
* @retval GT_FAIL         - otherwise.
*/
static GT_STATUS internal_cpssAppPlatformPpRemove
(
    IN GT_SW_DEV_NUM                         devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT removalType
)
{
    GT_STATUS                           rc = GT_FAIL;
    GT_PHYSICAL_PORT_NUM                portNum;
    GT_U32                              maxPortNumber = 0;
#ifdef CHX_FAMILY
    GT_U32                              regAddr;
    GT_U32                              vTcamMngItr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC *vTcamMng;
    GT_U8                               vTcamDev;
    GT_UINTPTR                          slItr;
    GT_U32                              lpmDb = 0;
    GT_U32                              trunkId;
    CPSS_TRUNK_MEMBER_STC               trunkMembersArray[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                              numOfMembers;
    GT_U32                              trunkMemberItr;
#endif
    GT_BOOL                             isLast     = GT_FALSE;
    GT_BOOL                             isPci      = GT_FALSE;
    GT_BOOL                             isSmi      = GT_FALSE;
    GT_BOOL                             isPipe     = GT_FALSE;
    GT_U32                              start_sec  = 0;
    GT_U32                              start_nsec = 0;
    GT_U32                              end_sec    = 0;
    GT_U32                              end_nsec   = 0;
    GT_U32                              diff_sec   = 0;
    GT_U32                              diff_nsec  = 0;
    CPSS_TRUNK_TYPE_ENT                 trunkType;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("PP [%d] Not Initialized\n",devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    maxPortNumber = cpssCapMaxPortNum[devNum]; /*PRV_CPSS_PP_MAC(devNum)->numOfPorts;*/

    PRV_IS_LAST_DEVICE_MAC(devNum, isLast);

    switch (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType)
    {
        case CPSS_CHANNEL_PEX_MBUS_E:
        case CPSS_CHANNEL_PEX_EAGLE_E:
        case CPSS_CHANNEL_PEX_FALCON_Z_E:
            isPci = GT_TRUE;
            break;
        case CPSS_CHANNEL_SMI_E:
            isSmi = GT_TRUE;
            break;
        default:
            break;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
        isPipe = GT_TRUE;
    }

    if (isPci)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("%s Removal of PP on PCI Bus %d  Dev %d device \n",
                (removalType == CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)? "Unmanaged" : "Managed",
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].hwAddr.busNo,
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].hwAddr.devSel
                );
    }
    else if (isSmi)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("%s Removal of PP of SMI slave Dev %d device \n",
                (removalType == CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)? "Unmanaged" : "Managed",
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].hwAddr.devSel
                );
    }

    /*Kill PM task*/
    rc = cpssAppPlatformPmTaskDelete();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPmTaskDelete);

    /* TBD : TODO If system recovery porcess is fastBoot then skip HW disable steps
       if(appDemoPpConfigList[devNum].systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
       {
       removalType = CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E;
       }
     */

    /* HW disable steps - Applicable for Managed Removal/Restart*/
    if (removalType != CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)
    {
        /* Disable interrupts : interrupt bits masked int cause registers*/
        /* this call causing crash in AC5x with internal cpu */
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            rc = cpssPpInterruptsDisable(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,cpssPpInterruptsDisable);
        }

        if (isPipe != GT_TRUE)
        {
#ifdef CHX_FAMILY
            /* Link down the ports */
            for (portNum = 0; portNum < maxPortNumber; portNum++)
            {
                CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                /* Link down ports */
                rc = cpssDxChPortForceLinkDownEnableSet(devNum, portNum, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortForceLinkDownEnableSet);

#ifndef ASIC_SIMULATION
                /*Make Link partner down*/
                rc= cpssDxChPortSerdesTxEnableSet(devNum, portNum, GT_FALSE);
                if (rc != GT_OK && rc != GT_NOT_SUPPORTED)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesTxEnableSet ret=%d", rc);
                    return rc;
                }
#endif
            }
            /* Disable CPU traffic */
            for (portNum = 0; portNum < maxPortNumber; portNum++)
            {
                CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                /* Disable PP-CPU Traffic-egress */
                rc = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortEgressFrwFilterSet);

                /* Disable PP-CPU Traffic-ingress */
                rc = cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortIngressFrwFilterSet);
            }

            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 1);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,prvCpssHwPpSetRegField);
            }

            for (portNum = 0 ; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
            {
                rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaToCpuPerPortSet);
            }
#endif
        }

        /* Masks unified events specific to device*/
        rc = prvEventMaskSet(devNum, CPSS_EVENT_MASK_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvEventMaskSet);

    }

    /* SW cleanup - Applicable for all removal Types*/
    if (isPipe != GT_TRUE)
    {
#ifdef CHX_FAMILY
        GT_U8    devListArr[1];
        /* Remove device from LPM DB*/
        devListArr[0] = CAST_SW_DEVNUM(devNum);
        slItr = 0;
        while ((rc = prvCpssDxChIpLpmDbIdGetNext(&lpmDb, &slItr)) == GT_OK)
        {
            rc = cpssDxChIpLpmDBDevsListRemove(lpmDb, devListArr, 1);
            if (rc != GT_OK && rc != GT_NOT_FOUND)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpLpmDBDevsListRemove ret=%d", rc);
                return rc;
            }
        }

        /* Delete LPM DB while removing last device*/
        if (isLast == GT_TRUE)
        {
            rc = prvCpssAppPlatformIpLpmLibReset();
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformIpLpmLibReset);
        }

        /* Remove Trunks from device */
        for (trunkId = 0; trunkId < (PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numberOfTrunks); trunkId++)
        {
            if (prvCpssGenericTrunkDbIsValid(devNum, trunkId) == GT_OK)
            {

                rc = cpssDxChTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,cpssDxChTrunkDbTrunkTypeGet);

                if(CPSS_TRUNK_TYPE_CASCADE_E==trunkType)
                {
                   continue;
                }
                else
                {
                    numOfMembers = CPSS_MAX_PORTS_NUM_CNS;
                    rc = cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, &numOfMembers, trunkMembersArray);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkDbEnabledMembersGet);
                }

                /* Remove enabled trunk members */

                for (trunkMemberItr = 0; trunkMemberItr < numOfMembers; trunkMemberItr++)
                {
                    rc = cpssDxChTrunkMemberRemove(devNum, trunkId, &trunkMembersArray[trunkMemberItr]);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,cpssDxChTrunkMemberRemove);
                }
                /* Remove disabled trunk members */
                numOfMembers = CPSS_MAX_PORTS_NUM_CNS;
                rc = cpssDxChTrunkDbDisabledMembersGet(devNum, trunkId, &numOfMembers, trunkMembersArray);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkDbDisabledMembersGet);

                for (trunkMemberItr = 0; trunkMemberItr < numOfMembers; trunkMemberItr++)
                {
                    rc = cpssDxChTrunkMemberRemove(devNum, trunkId, &trunkMembersArray[trunkMemberItr]);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkMemberRemove);
                }
            }
        }

        for (vTcamMngItr = 0; vTcamMngItr < CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS; vTcamMngItr++)
        {
            vTcamMng = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngItr);
            if (vTcamMng != NULL)
            {
                rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMng, &vTcamDev);
                if (rc == GT_NO_MORE)
                {
                    continue;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                }
                if (devNum == vTcamDev)
                {
                    /* Remove device from default vTcam manager */
                    rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngItr, &vTcamDev, 1);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChVirtualTcamManagerDevListRemove);
                }
            }
        }
#endif /*CHX_FAMILY*/
    }

    /* Delete User Event handlers */
    rc =  cpssAppPlatformEventHandlerReset(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEventHandlerReset);

    /* Deallocate FUQ, AUQ, SDMA Tx/Rx Bescriptors & Buffers */
    if (isPci)
    {
#ifdef CHX_FAMILY
#ifndef ASIC_SIMULATION
        rc = prvCpssAppPlatformDeAllocateDmaMem(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformDeAllocateDmaMem);
#else
        rc = osCacheDmaFreeAll();
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFreeAll);
#endif
#endif
    }

    if(removalType != CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)
    {
        if (isPipe != GT_TRUE)
        {
#ifdef CHX_FAMILY
            /* Disable All Skip Reset options ,exclude PEX */
            /* this Enable Skip Reset for PEX */
            if (PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum))
            {
                rc = cpssDxChHwPpSoftResetSkipParamSet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpSoftResetSkipParamSet);
            }

            rc = cpssDxChHwPpSoftResetTrigger(devNum);
            if (GT_OK != rc)
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChHwPpSoftResetTrigger ret=%d", rc);
#endif
        }
    }

    rc = prvCpssAppPlatformPpPhase1ConfigClear(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformPpPhase1ConfigClear);

    if (isPipe != GT_TRUE)
    {
#ifdef CHX_FAMILY
        rc = cpssDxChCfgDevRemove(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevRemove);
#endif
    }

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        cpssAppPlatformEzbFree(devNum);
    }
    rc = cpssAppPlatformProfileDbDelete(devNum, CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileDbDelete);

    if (isLast)
    {
        rc = cpssAppPlatformProfileDbReset();
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileDbReset);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef ASIC_SIMULATION
    cpssSimSoftResetDoneWait();
#endif

    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    TIME_DIFF_MAC(start_sec, start_nsec, end_sec, end_nsec, diff_sec, diff_nsec);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppPlatformPpRemove Time is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}

/**
* @internal cpssAppPlatformPpRemove function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] ppRemovalType    - Removal type: 0 - Managed Removal,
*                                              1 - Unmanaged Removal,
*                                              2 - Managed Reset
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPpRemove
(
    IN GT_SW_DEV_NUM                         devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT ppRemovalType
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssAppPlatformPpRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = internal_cpssAppPlatformPpRemove(devNum, ppRemovalType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

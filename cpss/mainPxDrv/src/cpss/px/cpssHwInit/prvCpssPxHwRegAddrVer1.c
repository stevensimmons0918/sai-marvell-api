/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvCpssPxHwRegAddrVer1.c
*
* @brief This file includes definitions of all different 'eArch' registers
* addresses to be used by all functions accessing PP registers.
* a variable of type PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC should be allocated
* for each PP type.
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwRegAddrVer1_ppDb.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef ASIC_SIMULATION /* needed for GM*/
/* checks existence of unit according to address */
typedef GT_BOOL (* SMEM_GM_HW_IS_UNIT_USED_PROC_FUN ) (
                   IN GT_U32       cpssDevNum,
                   IN GT_U32       portGroupId,
                   IN GT_U32       regAddr
);

/* init unit memory use checking function */
extern void smemGmUnitUsedFuncInit
(
    IN GT_U32       simDeviceId,
    IN GT_U32       cpssDevNum,
    IN SMEM_GM_HW_IS_UNIT_USED_PROC_FUN unitUsedFuncPtr
);

/* convert CPSS devNum, coreId to simulation deviceId */
extern GT_STATUS prvCpssDrvDevNumToSimDevIdConvert
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32      portGroupId,
    OUT GT_U32      *simDeviceIdPtr
);
#endif  /*ASIC_SIMULATION*/

extern GT_BOOL prvCpssPxcheckIfRegisterExist
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 regAddr
);

static GT_BOOL pipeDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
);

static void txqDqInit
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32  index
);

static void txqDqUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts
);

#define  NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS         BIT_8/*256*/

/* filled in runtime */
static PRV_CPSS_DXCH_UNIT_ENT prvPxUnitIdPer8MSBitsBaseAddrArr[NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS] =
{
    0,
};

/* number of elements in unit */
#define NUM_ELEMENTS_IN_UNIT_MAC(unit,element) (sizeof(unit) / sizeof(element))

/* macro to set parameters for function regUnitBaseAddrSet(...) */
#define REG_UNIT_INFO_MAC(devNum, unitBase, unitName)                   \
        (void*)(&PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->unitName),    \
        NUM_ELEMENTS_IN_UNIT_MAC(PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->unitName , GT_U32) , \
        unitBase

/* macro to set parameters for function regUnitPerPortAddrUpdate(...) */
#define REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, unitName1,unitName2 , regName ,startIndexUnit1,startIndexUnit2,numOfPorts, \
        saveLastNumOfPorts,saveToIndex,localPortMapArr) \
        &PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->unitName1.regName[0],   \
        &PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->unitName2.regName[0],   \
        startIndexUnit1,                                                  \
        startIndexUnit2,                                                  \
        numOfPorts,                                                       \
        saveLastNumOfPorts,                                               \
        saveToIndex,                                                      \
        localPortMapArr

extern void prvCpssDefaultAddressUnusedSet
(
    INOUT void   *startPtr,
    IN GT_U32   numOfRegs
);

#define STC_AND_SIZE_MAC(stcName) \
    (GT_U32*)&(stcName) , (sizeof(stcName) / 4)

#define DESTROY_STC_MAC(stcName) \
    regDestroyArr(STC_AND_SIZE_MAC(stcName))

/*******************************************************************************
* regDestroy
*
* DESCRIPTION:
*       destroy register info --> the register not relevant to the device
*
* INPUTS:
*       regPtr  - pointer to the register info.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*
*******************************************************************************/
/*static void regDestroy(
    IN GT_U32 *regPtr
)
{
    *regPtr = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    return;
}*/

/**
* @internal regDestroyArr function
* @endinternal
*
* @brief   destroy array of registers info --> the registers not relevant to the device
*
* @param[in] regArrPtr                - pointer to the registers array info.
* @param[in] numOfElements            - number of elements
*                                       None.
*/
static void regDestroyArr(
    IN GT_U32 *regArrPtr,
    IN GT_U32 numOfElements
)
{
    prvCpssDefaultAddressUnusedSet(regArrPtr,numOfElements);
}
/**
* @internal regUnitPerPortAddrUpdate function
* @endinternal
*
* @brief   This function replaces registers from unit2 to unit1.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] unitStart1Ptr            - pointer to start of the unit1
* @param[in] unitStart2Ptr            - pointer to start of the unit2
* @param[in] startIndexUnit1          - the number of ports to skip in unitStart1Ptr
* @param[in] startIndexUnit2          - the number of ports to skip in unitStart2Ptr
* @param[in] numOfPorts               - number of ports to copy
* @param[in] saveOrMoveLastNumOfPorts - number of ports from the end of the unitStart1Ptr[]
*                                      that need to be either
*                                      - saved before overridden by unitStart2Ptr[] when localPortMapArr == NULL
*                                      OR
*                                      - moved when localPortMapArr != NULL
*                                      saveToOrMoveIndex - the index in unitStart1Ptr[] to either save or move the 'last ports'
*                                      valid when saveOrMoveLastNumOfPorts != 0
*                                      meaning : copy unitStart2Ptr from index = startIndexUnit2 to index = (startIndexUnit2 + numOfPorts - 1)
*                                      into unitStart2Ptr from index = startIndexUnit1 to index = (startIndexUnit1 + numOfPorts - 1)
* @param[in] localPortMapArr          - array with index is relative port in Unit2 and value is local port in Unit2
*                                      may be NULL in 1:1 map
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static void regUnitPerPortAddrUpdate
(
    IN GT_U32    *unitStart1Ptr,
    IN GT_U32    *unitStart2Ptr,
    IN GT_U32    startIndexUnit1,
    IN GT_U32    startIndexUnit2,
    IN GT_U32    numOfPorts,
    IN GT_U32    saveOrMoveLastNumOfPorts,
    IN GT_U32    saveOrMoveToIndex,
    IN const GT_U32 *localPortMapArr
)
{
    GT_U32  ii,jj,kk;
    GT_U32  mm;

    ii = startIndexUnit1;
    jj = startIndexUnit2;

    if(localPortMapArr)
    {
        numOfPorts -= saveOrMoveLastNumOfPorts;
        for(kk = 0 ; kk < numOfPorts ; kk++)
        {
            unitStart1Ptr[ii] = unitStart2Ptr[localPortMapArr[jj]];
            ii++;
            jj++;
        }

        if(saveOrMoveLastNumOfPorts > 0)
        {
            ii = saveOrMoveToIndex;
            /* jj - continue from previous loop */
            /* move last indexes to other place - used for aldrin CPU port moving to position 72 */
            for(mm = 0; mm < saveOrMoveLastNumOfPorts; mm++)
            {
                unitStart1Ptr[ii] = unitStart2Ptr[localPortMapArr[jj]];
                ii++;
                jj++;
            }
        }
    }
    else
    {
        if(saveOrMoveLastNumOfPorts > 0)
            /* save last indexes to other place to will not be totally deleted */
            for(mm = 0; mm < saveOrMoveLastNumOfPorts; mm++)
            {
                unitStart1Ptr[saveOrMoveToIndex + mm] = unitStart1Ptr[numOfPorts - saveOrMoveLastNumOfPorts + mm];
            }

        for(kk = 0 ; kk < numOfPorts ; kk++)
        {
            unitStart1Ptr[ii] = unitStart2Ptr[jj];
            ii++;
            jj++;
        }
    }
}

/**
* @internal prvCpssPxHwUnitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] unitId                   - the ID of the address space unit.
*
* @param[out] errorPtr                 - (pointer to) indication that function did error.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32 prvCpssPxHwUnitBaseAddrGet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN PRV_CPSS_DXCH_UNIT_ENT   unitId,
    OUT GT_BOOL                 *errorPtr
)
{
    GT_U32  baseAddr = 1;/* default to cause fatal error on simulation */

    if(errorPtr)
    {
        *errorPtr = GT_TRUE;
    }

    if(unitId >= PRV_CPSS_DXCH_UNIT_LAST_E)
    {
        /* error */
        return baseAddr;
    }

    /* need to use prvCpssDrvPpConfig[] because the 'DXCH DB' may not exists yet !
       (during phase1 init of the driver itself) */
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
       NULL == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum])
    {
        /* error */
        return baseAddr;
    }

    switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily)
    {
        case CPSS_PX_FAMILY_PIPE_E:
            baseAddr = prvPxUnitsBaseAddr[unitId];
            break;
        default:
            /* error */
            return baseAddr;
    }

    if(errorPtr && (baseAddr != NON_VALID_BASE_ADDR_CNS))
    {
        *errorPtr = GT_FALSE;
    }

    if(baseAddr == NON_VALID_BASE_ADDR_CNS)
    {
        baseAddr |= 1;/* indication of not valid unit */
    }

    return baseAddr;
}

/**
* @internal prvCpssPxHwSubunitBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of subunit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] subunitId                - the ID of the address space unit.
* @param[in] indexArr                 - (pointer to) array of indexes that identify the
*                                      subunit instance.
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*
* @note For multi-port-group devices expected local port.
*
*/
static GT_U32 prvCpssPxHwSubunitBaseAddrGet
(
    IN GT_SW_DEV_NUM          devNum,
    IN PRV_CPSS_SUBUNIT_ENT   subunitId,
    IN GT_U32                 *indexArr
)
{
    GT_U32 gopBase = 0;
    GT_U32 subUnitOffset;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  portNum;

    if ((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) ||
       (NULL == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]))
    {
        return 1;/* error */
    }

    switch (subunitId)
    {
        case PRV_CPSS_SUBUNIT_GOP_TAI_E:
            /* single TAI unit */
            unitId = PRV_CPSS_DXCH_UNIT_TAI_E;
            return prvCpssPxHwUnitBaseAddrGet(devNum,unitId,NULL);

        case PRV_CPSS_SUBUNIT_GOP_PTP_E:
        case PRV_CPSS_SUBUNIT_GOP_FCA_E:
            if(PRV_CPSS_SUBUNIT_GOP_FCA_E == subunitId)
            {
                subUnitOffset = 0x00180600;
            }
            else
            {
                subUnitOffset = 0x00180800;
            }

            portNum = indexArr[0];
            unitId = PRV_CPSS_DXCH_UNIT_GOP_E;
            gopBase = prvCpssPxHwUnitBaseAddrGet(devNum,unitId,NULL);

            return gopBase + subUnitOffset + (0x1000 * portNum);
        case PRV_CPSS_SUBUNIT_GOP_CG_E:
                portNum = indexArr[0];

                unitId = PRV_CPSS_DXCH_UNIT_GOP_E;
                gopBase = prvCpssPxHwUnitBaseAddrGet(devNum,unitId,NULL);

                /* Port offset: 36 ports (0-35) with CG units */
                return gopBase + (0x1000 * portNum);
        default:
            break;
    }

    return 1;/* error */
}


/**
* @internal dbReset function
* @endinternal
*
* @brief   reset the DB
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void dbReset
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;

    /*Fill all the words in the struct with initial value*/
    regsAddrPtr32 = (GT_U32*)regsAddrPtr;
    regsAddrPtr32Size = sizeof(PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC) / 4;

    prvCpssDefaultAddressUnusedSet(regsAddrPtr32,regsAddrPtr32Size);
}

/**
* @internal resetAndInitControllerDbReset function
* @endinternal
*
* @brief   reset the Reset and Init Controller DB
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void resetAndInitControllerDbReset
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *regsAddrPtr;
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;

    regsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    /*Fill all the words in the struct with initial value*/
    regsAddrPtr32 = (GT_U32*)regsAddrPtr;
    regsAddrPtr32Size =
                    sizeof(PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC)/4;

    prvCpssDefaultAddressUnusedSet(regsAddrPtr32,regsAddrPtr32Size);
}

/**
* @internal prvCpssPxHwRegAddrStcInfoGet function
* @endinternal
*
* @brief   This function returns the address of the registers struct of Pipe device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] dbKind                   - type of registers DB we wish get
*
* @param[out] regAddrPtrPtr            - (pointer to) (pointer to) address of the registers struct.
* @param[out] sizePtr                  - (pointer to) size of registers struct.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwRegAddrStcInfoGet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  PRV_CPSS_GEN_REG_DB_KIND_ENT dbKind,
    OUT GT_U32                       **regAddrPtrPtr,
    OUT GT_U32                       *sizePtr,
    OUT PRV_CPSS_DIAG_CHECK_REG_EXIST_FUNC *checkRegExistFuncPtr
)
{
    GT_U32 *dbPtr = NULL;
    GT_U32 dbSize = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (dbKind)
    {
        case PRV_CPSS_GEN_REG_DB_KIND_SIP5_E:
            dbPtr  = (GT_U32 *)PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
            dbSize = sizeof(*(PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)));
            *checkRegExistFuncPtr = &prvCpssPxcheckIfRegisterExist;
            break;

        case PRV_CPSS_GEN_REG_DB_KIND_DFX_E:
            dbPtr  = (GT_U32 *)PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
            dbSize = sizeof(*(PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)));
            checkRegExistFuncPtr = NULL;
            break;
         default:
            break;
    }
    *regAddrPtrPtr = dbPtr;
    *sizePtr       = dbSize;
    return GT_OK;
}

/**
* @internal regUnitBaseAddrSet function
* @endinternal
*
* @brief   This function replaces base address of all registers in this unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] unitStartPtr             - pointer to start of the unit
* @param[in] numOfElements            - number of elements in the unit (num of registers)
* @param[in] newBaseAddr              - new base address for the unit
*
* @param[out] unitStartPtr             - unit register are updated
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static void regUnitBaseAddrSet(
    IN GT_U32    *unitStartPtr,
    IN GT_U32    numOfElements,
    IN GT_U32    newBaseAddr
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numOfElements ; ii++)
    {
        if(unitStartPtr[ii] == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* skip registers that are not used in the 'legacy device' */
            continue;
        }

        if(newBaseAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            /* the whole unit is not valid ! --> set all addresses to 'not used' */
            unitStartPtr[ii] = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
        }
        else
        {
            /* addresses must be '0' based -- to support 'huge units' */
            unitStartPtr[ii] = unitStartPtr[ii] + newBaseAddr;
        }
    }
}

/* copy from index to index */
#define COPY_REG_ADDR_MAC(regArr,srcIndex,offset , trgIndex) \
    regArr[(int)trgIndex - (int)offset] = regArr[srcIndex]


/**
* @internal rxdmaInit function
* @endinternal
*
* @brief   init the DB - RXDMA unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      index   - unit index
*                                       None
*/
static void rxdmaInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  lastRxDmaIndex = PRV_CPSS_PX_DMA_PORTS_NUM_CNS - 1;

    {/*start of unit rxDMA */
        {/*start of unit singleChannelDMAConfigs */
            {/*304+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    regsAddrPtr->rxDMA.singleChannelDMAConfigs.SCDMAConfig0[n] =
                        0x304+n*0x4;
                }/* end of loop n */
            }/*304+n*0x4*/
            {/*450+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    regsAddrPtr->rxDMA.singleChannelDMAConfigs.SCDMAConfig1[n] =
                        0x450+n*0x4;
                }/* end of loop n */
            }/*450+n*0x4*/
            {/*600+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    regsAddrPtr->rxDMA.singleChannelDMAConfigs.SCDMACTPktIndentification[n] =
                        0x600+n*0x4;
                }/* end of loop n */
            }/*600+n*0x4*/

        }/*end of unit singleChannelDMAConfigs */


        {/*start of unit globalRxDMAConfigs */
            {/*start of unit rejectConfigs */
                regsAddrPtr->rxDMA.globalRxDMAConfigs.rejectConfigs.rejectPktCommand = 0x000000a0;
                regsAddrPtr->rxDMA.globalRxDMAConfigs.rejectConfigs.rejectCPUCode = 0x000000a4;

            }/*end of unit rejectConfigs */


            {/*start of unit globalConfigCutThrough */
                regsAddrPtr->rxDMA.globalRxDMAConfigs.globalConfigCutThrough.CTEtherTypeIdentification = 0x00000084;
                regsAddrPtr->rxDMA.globalRxDMAConfigs.globalConfigCutThrough.CTUPEnable = 0x00000088;

            }/*end of unit globalConfigCutThrough */


            {/*start of unit buffersLimitProfiles */
                GT_U32    n;
                regsAddrPtr->rxDMA.globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[0] = 0;
                regsAddrPtr->rxDMA.globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[0] = 0x00000004;
                for(n = 1 ; n <= 7 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[n] =
                        regsAddrPtr->rxDMA.globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileXONXOFFConfig[n - 1] + 8;
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[n] =
                        regsAddrPtr->rxDMA.globalRxDMAConfigs.buffersLimitProfiles.portsBuffersLimitProfileRxlimitConfig[n - 1] + 8;
                }/* end of loop n */
            }/*end of unit buffersLimitProfiles */

        }/*end of unit globalRxDMAConfigs */

        { /* start of unit SCDMA debug */
            GT_U32    n;
            for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                regsAddrPtr->rxDMA.scdmaDebug.portBuffAllocCnt[n] = 0x1900+n*0x4;
            }
        }
        {/*start of unit SIP_COMMON_MODULES */
            {/*start of unit pizzaArbiter */
                {/*2508+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x2508+n*0x4;
                    }/* end of loop n */
                }/*2508+n*0x4*/
                regsAddrPtr->rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg = 0x00002500;
                regsAddrPtr->rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterStatusReg = 0x00002504;

            }/*end of unit pizzaArbiter */


        }/*end of unit SIP_COMMON_MODULES */

        { /* start of unit debug */

            regsAddrPtr->rxDMA.debug.countersAndCountersStatuses.currentNumOfPacketsStoredInMppm = 0x0000123C;

        }/*end of unit debug */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            {/*start of unit SIP_COMMON_MODULES */
                {/*start of unit pizzaArbiter */
                    {/*2a08+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 84 ; n++) {
                            regsAddrPtr->rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterConfigReg[n] =
                                0x2a08+n*0x4;
                        }/* end of loop n */
                    }/*2a08+n*0x4*/
                    regsAddrPtr->rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterCtrlReg = 0x00002a00;
                    regsAddrPtr->rxDMA.SIP_COMMON_MODULES.pizzaArbiter.pizzaArbiterStatusReg = 0x00002a04;

                }/*end of unit pizzaArbiter */


            }/*end of unit SIP_COMMON_MODULES */

            {/*start of unit singleChannelDMAConfigs */
                {/*804+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        regsAddrPtr->rxDMA.singleChannelDMAConfigs.SCDMAConfig0[n] =
                            0x804+n*0x4;
                    }/* end of loop n */
                }/*804+n*0x4*/
                {/*950+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        regsAddrPtr->rxDMA.singleChannelDMAConfigs.SCDMAConfig1[n] =
                            0x950+n*0x4;
                    }/* end of loop n */
                }/*950+n*0x4*/
                {/*b00+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                        regsAddrPtr->rxDMA.singleChannelDMAConfigs.SCDMACTPktIndentification[n] =
                            0xb00+n*0x4;
                    }/* end of loop n */
                }/*b00+n*0x4*/

            }/*end of unit singleChannelDMAConfigs */
            { /* start of unit SCDMA debug */
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    regsAddrPtr->rxDMA.scdmaDebug.portBuffAllocCnt[n] = 0x1e00+n*0x4;
                }
            }
            { /* start of unit debug */

                regsAddrPtr->rxDMA.debug.countersAndCountersStatuses.currentNumOfPacketsStoredInMppm = 0x0000173C;

            }/*end of unit debug */
        }

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {/*start of unit preIngrPrioritizationConfStatus */
            {/*000000c4+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANUPPriorityArrayProfile[n] =
                        0x000000c4+n * 0x4;
                }/* end of loop n */
            }/*000000c4+n * 0x4*/
            {/*000003a8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEtherTypeConf[n] =
                        0x000003a8+n*0x4;
                }/* end of loop n */
            }/*000003a8+n*0x4*/
            {/*00000450+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.VIDConf[n] =
                        0x00000450+n*0x4;
                }/* end of loop n */
            }/*00000450+n*0x4*/
            {/*000003c8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.genericEtherType[n] =
                        0x000003c8+n*0x4;
                }/* end of loop n */
            }/*000003c8+n*0x4*/
            {/*000000e4+m*0x4 + n*0x20*/
                GT_U32    n,m;
                for(n = 0 ; n <= 3 ; n++) {
                    for(m = 0 ; m <= 7 ; m++) {
                        regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.qoSDSAPriorityArrayProfileSegment[n][m] =
                            0x000000e4+m*0x4 + n*0x20;
                    }/* end of loop m */
                }/* end of loop n */
            }/*000000e4+m*0x4 + n*0x20*/
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPPriorityDropGlobalCntrsClear = 0x00000740;
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrMsb = 0x00000724;
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPMedPriorityDropGlobalCntrLsb = 0x00000720;
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrMsb = 0x00000714;
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrLsb = 0x00000760;
                regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPVeryHighPriorityDropGlobalCntrMsb = 0x00000764;
            }
            {/*00000398+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.VLANEtherTypeConf[n] =
                        0x00000398+n*0x4;
                }/* end of loop n */
            }/*00000398+n*0x4*/
            {/*00000408+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA2MsbBytesConf[n] =
                        0x00000408+n*0x4;
                }/* end of loop n */
            }/*00000408+n*0x4*/
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrLsb = 0x00000730;
            {/*00000174+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MPLSEXPPriorityArrayProfile[n] =
                        0x00000174+n * 0x4;
                }/* end of loop n */
            }/*00000174+n * 0x4*/
            {/*00000630+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.UPDSATagArrayProfile[n] =
                        0x00000630+n * 0x4;
                }/* end of loop n */
            }/*00000630+n * 0x4*/
            {/*000002f0+n * 0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAPriorityArrayProfile[n] =
                        0x000002f0+n * 0x4;
                }/* end of loop n */
            }/*000002f0+n * 0x4*/
            {/*00000428+n *0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask4LsbConf[n] =
                        0x00000428+n *0x4;
                }/* end of loop n */
            }/*00000428+n *0x4*/
            {/*0000043c+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDAMask2MsbConf[n] =
                        0x0000043c+n*0x4;
                }/* end of loop n */
            }/*0000043c+n*0x4*/
            {/*000003e8+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.MACDA4LsbBytesConf[n] =
                        0x000003e8+n*0x4;
                }/* end of loop n */
            }/*000003e8+n*0x4*/
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPLowPriorityDropGlobalCntrLSb = 0x00000710;
            {/*000004d0+m*0x4 + n*0x40*/
                GT_U32    n,m;
                for(n = 0 ; n <= 3 ; n++) {
                    for(m = 0 ; m <= 15 ; m++) {
                        regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6TCPriorityArrayProfileSegment[n][m] =
                            0x000004d0+m*0x4 + n*0x40;
                    }/* end of loop m */
                }/* end of loop n */
            }/*000004d0+m*0x4 + n*0x40*/
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv6EtherTypeConf = 0x00000374;
            {/*00000198+m*0x4 + n*0x40*/
                GT_U32    n,m;
                for(n = 0 ; n <= 3 ; n++) {
                    for(m = 0 ; m <= 15 ; m++) {
                        regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4ToSPriorityArrayProfileSegment[n][m] =
                            0x00000198+m*0x4 + n*0x40;
                    }/* end of loop m */
                }/* end of loop n */
            }/*00000198+m*0x4 + n*0x40*/
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.IPv4EtherTypeConf = 0x00000350;
            regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPHighPriorityDropGlobalCntrMsb = 0x00000734;
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.PIPDropGlobalCounterTrigger  = 0x00000738;
            }
            {/*00000320+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrPtr->rxDMA.globalRxDMAConfigs.preIngrPrioritizationConfStatus.etherTypePriorityArrayProfile[n] =
                        0x00000320+n*0x4;
                }/* end of loop n */
            }/*00000320+n*0x4*/

        }/*end of unit preIngrPrioritizationConfStatus */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            /* manually added : rxDMA.singleChannelDMAPip */
            {/*3A00+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    regsAddrPtr->rxDMA.singleChannelDMAPip.SCDMA_n_PIP_Config[n] =
                        0x3A00+n*0x4;
                }/* end of loop n */
            }/*3A00+n*0x4*/

            {/*3500+n*0x4*/
                GT_U32    n;
                for(n = 0 ; n <= lastRxDmaIndex ; n++) {
                    regsAddrPtr->rxDMA.singleChannelDMAPip.SCDMA_n_Drop_Pkt_Counter[n] =
                        0x3500+n*0x4;
                }/* end of loop n */
            }/*3500+n*0x4*/
        }

    }/*end of unit rxDMA */
}

/**
* @internal txdmaInit function
* @endinternal
*
* @brief   init the DB - TXDMA unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      index   - unit index
*                                       None
*/
static void txdmaInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  lastTxDmaIndex = PRV_CPSS_PX_DMA_PORTS_NUM_CNS - 1;

    {/*start of unit txDMA */
        {/*start of unit txDMAThresholdsConfigs */
            {/*00001020+r*0x4*/
                GT_U32    r;
                for(r = 0 ; r <= 0 ; r++) {
                    regsAddrPtr->txDMA.txDMAThresholdsConfigs.RXDMAUpdatesFIFOsThresholds[r] =
                        0x00001020+r*0x4;
                }/* end of loop r */
            }/*00001020+r*0x4*/
            regsAddrPtr->txDMA.txDMAThresholdsConfigs.clearFIFOThreshold = 0x00001000;
            regsAddrPtr->txDMA.txDMAThresholdsConfigs.headerReorderFifoThreshold = 0x0000100C;
            regsAddrPtr->txDMA.txDMAThresholdsConfigs.payloadReorderFifoThreshold = 0x00001010;

        }/*end of unit txDMAThresholdsConfigs */


        {/*start of unit txDMAPerSCDMAConfigs */
            {/*0000300c+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.burstLimiterSCDMA[p] =
                        0x0000300c+0x20*p;
                }/* end of loop p */
            }/*0000300c+0x20*p*/
            {/*00003000+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.FIFOsThresholdsConfigsSCDMAReg1[p] =
                        0x00003000+p*0x20;
                }/* end of loop p */
            }/*00003000+p*0x20*/
            {/*00003008+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[p] =
                        0x00003008+0x20*p;
                }/* end of loop p */
            }/*00003008+0x20*p*/
            {/*00003010+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.SCDMAConfigs[p] =
                        0x00003010+p*0x20;
                }/* end of loop p */
            }/*00003010+p*0x20*/
            {/*00003014+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.SCDMAMetalFixReg[p] =
                        0x00003014+p*0x20;
                }/* end of loop p */
            }/*00003014+p*0x20*/
        }/*end of unit txDMAPerSCDMAConfigs */


        {/*start of unit txDMAGlobalConfigs */
            regsAddrPtr->txDMA.txDMAGlobalConfigs.txDMAGlobalConfig1      = 0x00000000;
            regsAddrPtr->txDMA.txDMAGlobalConfigs.txDMAGlobalConfig3      = 0x00000008;
            regsAddrPtr->txDMA.txDMAGlobalConfigs.ingrCoreIDToMPPMMap     = 0x0000000c;
            regsAddrPtr->txDMA.txDMAGlobalConfigs.TXDMAMetalFixReg        = 0x00000010;
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) /* BC3 and higher */
            {
                GT_U32 i;
                for (i = 0; i <= 2 ; i++)
                {
                    regsAddrPtr->txDMA.txDMAGlobalConfigs.srcSCDMASpeedTbl[i] = 0x000000220 + i*0x4;
                }
                for (i = 0 ; i < 8; i++)
                {
                    regsAddrPtr->txDMA.txDMAGlobalConfigs.localDevSrcPort2DMANumberMap[i] = 0x000000020 + i*0x4;
                }
            }
        }/*end of unit txDMAGlobalConfigs */


        {/*start of unit txDMADescCriticalFieldsECCConfigs */
            regsAddrPtr->txDMA.txDMADescCriticalFieldsECCConfigs.dataECCConfig = 0x00000500;
            regsAddrPtr->txDMA.txDMADescCriticalFieldsECCConfigs.dataECCStatus = 0x00000504;

        }/*end of unit txDMADescCriticalFieldsECCConfigs */


        {/*start of unit txDMADebug */
            {/*start of unit informativeDebug */
                {/*00005100+c*0x4*/
                    GT_U32    c;
                    for(c = 0 ; c <= 0 ; c++) {
                        regsAddrPtr->txDMA.txDMADebug.informativeDebug.nextBCFIFOFillLevel[c] =
                            0x00005100+c*0x4;
                    }/* end of loop c */
                }/*00005100+c*0x4*/
                {/*00005120+c*0x4*/
                    GT_U32    c;
                    for(c = 0 ; c <= 0 ; c++) {
                        regsAddrPtr->txDMA.txDMADebug.informativeDebug.nextBCFIFOMaxFillLevel[c] =
                            0x00005120+c*0x4;
                    }/* end of loop c */
                }/*00005120+c*0x4*/
                {/*00005200+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        regsAddrPtr->txDMA.txDMADebug.informativeDebug.SCDMAStatusReg1[p] =
                            0x00005200+p*0x4;
                    }/* end of loop p */
                }/*00005200+p*0x4*/
                {/*00005500+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        regsAddrPtr->txDMA.txDMADebug.informativeDebug.SCDMAStatusReg2[p] =
                            0x00005500+p*0x4;
                    }/* end of loop p */
                }/*00005500+p*0x4*/
                {/*00005700+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        regsAddrPtr->txDMA.txDMADebug.informativeDebug.SCDMAPrefFIFOFillLevel[p] =
                            0x00005700+p*0x4;
                    }/* end of loop p */
                }/*00005700+p*0x4*/
                {/*00005900+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                        regsAddrPtr->txDMA.txDMADebug.informativeDebug.SCDMAPrefFIFOMaxFillLevel[p] =
                            0x00005900+p*0x4;
                    }/* end of loop p */
                }/*00005900+p*0x4*/
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.descOverrunReg1 = 0x00005000;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.descOverrunReg2 = 0x00005004;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.descOverrunReg3 = 0x00005008;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.clearFIFOFillLevel = 0x0000500c;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.clearFIFOMaxFillLevel = 0x00005010;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.headerReorderFIFOFillLevel = 0x00005014;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.headerReorderFIFOMaxFillLevel = 0x0000501c;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.payloadReorderFIFOFillLevel = 0x00005020;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.payloadReorderFIFOMaxFillLevel = 0x00005024;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.descIDFIFOFillLevel = 0x00005028;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.descIDFIFOMaxFillLevel = 0x0000502c;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.debugBusCtrlReg = 0x00005030;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.txDMADebugBus = 0x00005034;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.incomingDescsCntrLsb = 0x00005038;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingDescsCntrLsb = 0x0000503c;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingClearRequestsCntrLsb = 0x00005040;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.MPPMReadRequestsCntr = 0x00005044;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.MPPMReadAnswersCntrLsb = 0x00005048;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingPayloadsCntrLsb = 0x0000504c;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingPayloadsEOPCntrLsb = 0x00005050;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingHeadersEOPCntrLsb = 0x00005054;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingCTBCUpdatesCntr = 0x00005058;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.HADescIDDBFreeIDFIFOFillLevel = 0x0000505c;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.internalDescIDDBFreeIDFIFOFillLevel = 0x00005060;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.headerReorderMemIDDBFreeIDFIFOFillLevel = 0x00005070;
                regsAddrPtr->txDMA.txDMADebug.informativeDebug.payloadReorderMemIDDBFreeIDFIFOFillLevel = 0x00005074;

            }/*end of unit informativeDebug */


        }/*end of unit txDMADebug */


        {/*start of unit txDMAPizzaArb */
            {/*start of unit pizzaArbiter */
                {/*00004008+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->txDMA.txDMAPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x00004008+n*0x4;
                    }/* end of loop n */
                }/*00004008+n*0x4*/
                regsAddrPtr->txDMA.txDMAPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg = 0x00004000;
                regsAddrPtr->txDMA.txDMAPizzaArb.pizzaArbiter.pizzaArbiterStatusReg = 0x00004004;

            }/*end of unit pizzaArbiter */


        }/*end of unit txDMAPizzaArb */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.incomingDescsCntrLsb = 0x00005038;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.incomingDescsCntrMsb = 0x0000503c;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingDescsCntrLsb = 0x00005040;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingDescsCntrMsb = 0x00005044;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingClearRequestsCntrLsb = 0x00005048;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingClearRequestsCntrMsb = 0x0000504c;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.MPPMReadRequestsCntr = 0x00005050;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.MPPMReadAnswersCntrLsb = 0x00005054;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.MPPMReadAnswersCntrMsb = 0x00005058;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingPayloadsCntrLsb = 0x0000505c;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingPayloadsCntrMsb = 0x00005060;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingPayloadsEOPCntrLsb = 0x00005064;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingPayloadsEOPCntrMsb = 0x00005068;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingHeadersEOPCntrLsb = 0x0000506c;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingHeadersEOPCntrMsb = 0x00005070;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.outgoingCTBCUpdatesCntr = 0x00005074;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.internalDescIDDBFreeIDFIFOFillLevel = 0x00005080;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.headerReorderMemIDDBFreeIDFIFOFillLevel = 0x00005084;
            regsAddrPtr->txDMA.txDMADebug.informativeDebug.payloadReorderMemIDDBFreeIDFIFOFillLevel = 0x00005088;
            {/*0000300c+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.txFIFOCntrsConfigsSCDMA[p] =
                        0x0000300c+0x20*p;
                }/* end of loop p */
            }/*0000300c+0x20*p*/

            {/*00003018+0x20*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.burstLimiterSCDMA[p] =
                        0x00003018+0x20*p;
                }/* end of loop p */
            }/*00003018+0x20*p*/

        }

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            {/*0000301C+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.SCDMAConfigs2[p] = 0x0000301C+p*0x20;
                }/* end of loop p */
            }/*0000301C+p*0x20*/
        }
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            {/*0000301C+p*0x20*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxDmaIndex ; p++) {
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.SCDMAConfigs1                 [p] = 0x00003010+p*0x20;
                    regsAddrPtr->txDMA.txDMAPerSCDMAConfigs.SCDMARateLimitResidueVectorThr[p] = 0x00003A00+p*0x4;
                }/* end of loop p */
            }/*0000301C+p*0x20*/
        }

    }/*end of unit txDMA */
}


/**
* @internal txfifoInit function
* @endinternal
*
* @brief   init the DB - TXFIFO unit (per unit index)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      index   - unit index
*                                       None
*/
static void txfifoInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  lastTxFifoIndex = PRV_CPSS_PX_DMA_PORTS_NUM_CNS - 1 ;

    {/*start of unit txFIFO */
        {/*start of unit txFIFOShiftersConfig */
            {/*00000600+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                    regsAddrPtr->txFIFO.txFIFOShiftersConfig.SCDMAShiftersConf[p] =
                        0x00000600+0x4*p;
                }/* end of loop p */
            }/*00000600+0x4*p*/

        }/*end of unit txFIFOShiftersConfig */


        {/*start of unit txFIFOLinkedLists */
            {/*start of unit payloadLinkedList */
                {/*start of unit payloadLinkListRAM */
                    regsAddrPtr->txFIFO.txFIFOLinkedLists.payloadLinkedList.payloadLinkListRAM.payloadLinkListAddr = 0x00024000;

                }/*end of unit payloadLinkListRAM */


            }/*end of unit payloadLinkedList */


            {/*start of unit headerLinkedList */
                {/*start of unit headerLinkListRAM */
                    regsAddrPtr->txFIFO.txFIFOLinkedLists.headerLinkedList.headerLinkListRAM.headerLinkListAddr = 0x00014000;

                }/*end of unit headerLinkListRAM */


            }/*end of unit headerLinkedList */


        }/*end of unit txFIFOLinkedLists */


        {/*start of unit txFIFOInterrupts */
            regsAddrPtr->txFIFO.txFIFOInterrupts.txFIFOGeneralCauseReg1 = 0x00000400;
            regsAddrPtr->txFIFO.txFIFOInterrupts.txFIFOGeneralMaskReg1 = 0x00000404;
            regsAddrPtr->txFIFO.txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg1 = 0x00000408;
            regsAddrPtr->txFIFO.txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg1 = 0x0000040c;
            regsAddrPtr->txFIFO.txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg2 = 0x00000410;
            regsAddrPtr->txFIFO.txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg2 = 0x00000414;
            regsAddrPtr->txFIFO.txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptCauseReg3 = 0x00000418;
            regsAddrPtr->txFIFO.txFIFOInterrupts.shifterSOPEOPFIFONotReadyInterruptMaskReg3 = 0x0000041c;
            regsAddrPtr->txFIFO.txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg1 = 0x00000420;
            regsAddrPtr->txFIFO.txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg1 = 0x00000424;
            regsAddrPtr->txFIFO.txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg2 = 0x00000428;
            regsAddrPtr->txFIFO.txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg2 = 0x0000042c;
            regsAddrPtr->txFIFO.txFIFOInterrupts.CTByteCountArrivedLateInterruptCauseReg3 = 0x00000430;
            regsAddrPtr->txFIFO.txFIFOInterrupts.CTByteCountArrivedLateInterruptMaskReg3 = 0x00000434;
            regsAddrPtr->txFIFO.txFIFOInterrupts.txFIFOSummaryCauseReg = 0x00000440;
            regsAddrPtr->txFIFO.txFIFOInterrupts.txFIFOSummaryMaskReg = 0x00000444;

        }/*end of unit txFIFOInterrupts */


        {/*start of unit txFIFOGlobalConfig */
            {/*00000008+p*0x4*/
                GT_U32    p;
                for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                    regsAddrPtr->txFIFO.txFIFOGlobalConfig.SCDMAPayloadThreshold[p] =
                        0x00000008+p*0x4;
                }/* end of loop p */
            }/*00000008+p*0x4*/
            regsAddrPtr->txFIFO.txFIFOGlobalConfig.txFIFOMetalFixReg = 0x00000004;

        }/*end of unit txFIFOGlobalConfig */


        {/*start of unit txFIFOEnd2EndECCConfigs */
            {/*00000508+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 1 ; i++) {
                    regsAddrPtr->txFIFO.txFIFOEnd2EndECCConfigs.dataECCConfig[i] =
                        0x00000508+0x4*i;
                }/* end of loop i */
            }/*00000508+0x4*i*/
            {/*00000518+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i <= 1 ; i++) {
                    regsAddrPtr->txFIFO.txFIFOEnd2EndECCConfigs.dataECCStatus[i] =
                        0x00000518+0x4*i;
                }/* end of loop i */
            }/*00000518+0x4*i*/

        }/*end of unit txFIFOEnd2EndECCConfigs */


        {/*start of unit txFIFODebug */
            {/*start of unit informativeDebug */
                {/*00001b00+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.SCDMAStatusReg6[p] =
                            0x00001b00+p*0x4;
                    }/* end of loop p */
                }/*00001b00+p*0x4*/
                {/*00001100+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.SCDMAStatusReg1[p] =
                            0x00001100+p*0x4;
                    }/* end of loop p */
                }/*00001100+p*0x4*/
                {/*00001300+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.SCDMAStatusReg2[p] =
                            0x00001300+p*0x4;
                    }/* end of loop p */
                }/*00001300+p*0x4*/
                {/*00001500+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.SCDMAStatusReg3[p] =
                            0x00001500+p*0x4;
                    }/* end of loop p */
                }/*00001500+p*0x4*/
                {/*00001700+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.SCDMAStatusReg4[p] =
                            0x00001700+p*0x4;
                    }/* end of loop p */
                }/*00001700+p*0x4*/
                {/*00001900+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= lastTxFifoIndex ; p++) {
                        regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.SCDMAStatusReg5[p] =
                            0x00001900+p*0x4;
                    }/* end of loop p */
                }/*00001900+p*0x4*/
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.debugBusCtrlReg = 0x00001000;
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.txFIFODebugBus = 0x00001004;
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.LLEOPCntrLsb = 0x0000100c;
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.incomingHeadersCntrLsb = 0x00001008;
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.headerIDDBFreeIDFIFOFillLevel = 0x00001010;
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.payloadIDDBFreeIDFIFOFillLevel = 0x00001014;
                regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.CTBCIDDBFreeIDFIFOFillLevel = 0x00001018;

            }/*end of unit informativeDebug */


        }/*end of unit txFIFODebug */


        {/*start of unit txFIFOPizzaArb */
            {/*start of unit pizzaArbiter */
                {/*00000808+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 84 ; n++) {
                        regsAddrPtr->txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterConfigReg[n] =
                            0x00000808+n*0x4;
                    }/* end of loop n */
                }/*00000808+n*0x4*/
                regsAddrPtr->txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterCtrlReg = 0x00000800;
                regsAddrPtr->txFIFO.txFIFOPizzaArb.pizzaArbiter.pizzaArbiterStatusReg = 0x00000804;

            }/*end of unit pizzaArbiter */


        }/*end of unit txFIFOPizzaArb */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.LLEOPCntrLsb = 0x00001010;
            regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.LLEOPCntrMsb = 0x00001014;
            regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.headerIDDBFreeIDFIFOFillLevel = 0x00001018;
            regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.incomingHeadersCntrMsb = 0x0000100c;
            regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.payloadIDDBFreeIDFIFOFillLevel = 0x0000101c;
            regsAddrPtr->txFIFO.txFIFODebug.informativeDebug.CTBCIDDBFreeIDFIFOFillLevel = 0x00001020;
        }

    }/*end of unit txFIFO */
}

static PRV_CPSS_DXCH_UNIT_ENT txqDqUnits[PRV_CPSS_PX_NUM_DQ_CNS] =
{
     PRV_CPSS_DXCH_UNIT_TXQ_DQ_E
    ,PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E
};


/**
* @internal dataPathInit function
* @endinternal
*
* @brief   init for DATA PATH . init for RxDma,txDma,txFifo,txq-DQ.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void dataPathInit
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32 * unitBasePtr
)
{
    GT_U32  ii;
    GT_U32  portIn_unit_0;/* current port in unit 0 (global port) */
    GT_U32  port_inUnit_ii;/* current port in unit ii (local port) */
    GT_U32  numOfPorts;/* number of ports in the unit ii*/

    /****************/
    /* init unit 0  */
    /****************/
    rxdmaInit(devNum);
    /* set the addresses of the rxdma unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_RXDMA_E], rxDMA));

    txdmaInit(devNum);
    /* set the addresses of the txdma unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXDMA_E], txDMA));

    txfifoInit(devNum);
    /* set the addresses of the txfifo unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TX_FIFO_E], txFIFO));


    for(ii = 0 ; ii < PRV_CPSS_PX_NUM_DQ_CNS ; ii++)
    {
        /* init the TXQ_DQ unit*/
        txqDqInit(devNum,ii);
        /* set the addresses of the TXQ_DQ unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[txqDqUnits[ii]], TXQ.dq[ii]));

        /* update the DB - txq-DQ unit - set 'global' ports 'per port' addresses. */
        numOfPorts    = PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS;
        portIn_unit_0 = ii*numOfPorts;
        port_inUnit_ii = 0;
        txqDqUpdateUnit0AsGlobalPortsAddr(devNum,ii,portIn_unit_0,port_inUnit_ii,numOfPorts);
    }

}


/**
* @internal bmInit function
* @endinternal
*
* @brief   init the DB - bm unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void bmInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  num_BMBufferTermination;
    GT_U32  num_rxDMANextUpdatesFIFOMaxFillLevel;
    GT_U32  num_rxDMANextUpdatesFIFOFillLevel;
    GT_U32  num_SCDMAInProgRegs;

    num_BMBufferTermination                 = 4     +1;
    num_rxDMANextUpdatesFIFOMaxFillLevel    = 0     +1;
    num_rxDMANextUpdatesFIFOFillLevel       = 0     +1;
    num_SCDMAInProgRegs                     = 147   +1;

    {/*start of unit BM */
        {/*start of unit BMInterrupts */
            regsAddrPtr->BM.BMInterrupts.BMGeneralCauseReg1 = 0x00000300;
            regsAddrPtr->BM.BMInterrupts.BMGeneralMaskReg1 = 0x00000304;
            regsAddrPtr->BM.BMInterrupts.BMGeneralCauseReg2 = 0x00000308;
            regsAddrPtr->BM.BMInterrupts.BMGeneralMaskReg2 = 0x0000030c;
            regsAddrPtr->BM.BMInterrupts.BMSummaryCauseReg = 0x00000310;
            regsAddrPtr->BM.BMInterrupts.BMSummaryMaskReg = 0x00000314;

        }/*end of unit BMInterrupts */


        {/*start of unit BMGlobalConfigs */
            {/*00000050+0x4*i*/
                GT_U32    i;
                for(i = 0 ; i < num_BMBufferTermination ; i++) {
                    regsAddrPtr->BM.BMGlobalConfigs.BMBufferTermination[i] =
                        0x00000050+0x4*i;
                }/* end of loop i */
            }/*00000050+0x4*i*/
            regsAddrPtr->BM.BMGlobalConfigs.BMGlobalConfig1 = 0x00000000;
            regsAddrPtr->BM.BMGlobalConfigs.BMGlobalConfig2 = 0x00000004;
            regsAddrPtr->BM.BMGlobalConfigs.BMPauseConfig = 0x00000010;
            regsAddrPtr->BM.BMGlobalConfigs.BMBufferLimitConfig1 = 0x00000014;
            regsAddrPtr->BM.BMGlobalConfigs.BMBufferLimitConfig2 = 0x00000018;
            regsAddrPtr->BM.BMGlobalConfigs.BMMetalFixReg = 0x00000020;
            /* Aldrin, AC3X, Bobcat3 */
            if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
            {
                regsAddrPtr->BM.BMGlobalConfigs.BMGlobalConfig3 = 0x00000008;
            }

        }/*end of unit BMGlobalConfigs */


        {/*start of unit BMDebug */
            {/*00000480+c*0x4*/
                GT_U32    c;
                for(c = 0 ; c < num_rxDMANextUpdatesFIFOMaxFillLevel ; c++) {
                    regsAddrPtr->BM.BMDebug.rxDMANextUpdatesFIFOMaxFillLevel[c] =
                        0x00000480+c*0x4;
                }/* end of loop c */
            }/*00000480+c*0x4*/
            {/*00000460+c*0x4*/
                GT_U32    c;
                for(c = 0 ; c < num_rxDMANextUpdatesFIFOFillLevel ; c++) {
                    regsAddrPtr->BM.BMDebug.rxDMANextUpdatesFIFOFillLevel[c] =
                        0x00000460+c*0x4;
                }/* end of loop c */
            }/*00000460+c*0x4*/
            {/*00000500+s*0x4*/
                GT_U32    s;
                for(s = 0 ; s < num_SCDMAInProgRegs ; s++) {
                    regsAddrPtr->BM.BMDebug.SCDMAInProgRegs[s] =
                        0x00000500+s*0x4;
                }/* end of loop s */
            }/*00000500+s*0x4*/

        }/*end of unit BMDebug */


        {/*start of unit BMCntrs */
            regsAddrPtr->BM.BMCntrs.BMGlobalBufferCntr = 0x00000200;
            regsAddrPtr->BM.BMCntrs.rxDMAsAllocationsCntr = 0x00000204;
            regsAddrPtr->BM.BMCntrs.BMFreeBuffersInsideBMCntr = 0x00000208;
            regsAddrPtr->BM.BMCntrs.BMRxDMAPendingAllocationCntr = 0x0000020c;
            regsAddrPtr->BM.BMCntrs.BMCTClearsInsideBMCntr = 0x00000210;
            regsAddrPtr->BM.BMCntrs.incomingCTClearsCntr = 0x00000214;
            regsAddrPtr->BM.BMCntrs.incomingRegularClearsCntr = 0x00000218;
            regsAddrPtr->BM.BMCntrs.nextTableAccessCntr = 0x0000021c;
            regsAddrPtr->BM.BMCntrs.nextTableWriteAccessCntr = 0x00000220;
            regsAddrPtr->BM.BMCntrs.BMTerminatedBuffersCntr = 0x00000224;
            regsAddrPtr->BM.BMCntrs.BMLostClearRequestsCntr = 0x00000228;

        }/*end of unit BMCntrs */


    }/*end of unit BM */

}

/**
* @internal bmaInit function
* @endinternal
*
* @brief   init the DB - bma unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void bmaInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    regsAddrPtr->BMA.BmaMCCNTParityErrorCounter1 =  0x05900C;

}

/**
* @internal gopTaiInit function
* @endinternal
*
* @brief   init the DB - TAI unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      gopIndex - index of GOP
*                                      taiIndex - index of TAI inside of GOP
*                                       None
*/
static void gopTaiInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    /*start of unit TAI */
    regsAddrPtr->GOP.TAI.TAIInterruptCause = 0x00000000;
    regsAddrPtr->GOP.TAI.TAIInterruptMask = 0x00000004;
    regsAddrPtr->GOP.TAI.TAICtrlReg0 = 0x00000008;
    regsAddrPtr->GOP.TAI.TAICtrlReg1 = 0x0000000c;
    regsAddrPtr->GOP.TAI.timeCntrFunctionConfig0 = 0x00000010;
    regsAddrPtr->GOP.TAI.timeCntrFunctionConfig1 = 0x00000014;
    regsAddrPtr->GOP.TAI.timeCntrFunctionConfig2 = 0x00000018;
    regsAddrPtr->GOP.TAI.frequencyAdjustTimeWindow = 0x0000001c;
    regsAddrPtr->GOP.TAI.TODStepNanoConfigLow = 0x00000020;
    regsAddrPtr->GOP.TAI.TODStepFracConfigHigh = 0x00000024;
    regsAddrPtr->GOP.TAI.TODStepFracConfigLow = 0x00000028;
    regsAddrPtr->GOP.TAI.timeAdjustmentPropagationDelayConfigHigh = 0x0000002c;
    regsAddrPtr->GOP.TAI.timeAdjustmentPropagationDelayConfigLow = 0x00000030;
    regsAddrPtr->GOP.TAI.triggerGenerationTODSecHigh = 0x00000034;
    regsAddrPtr->GOP.TAI.triggerGenerationTODSecMed = 0x00000038;
    regsAddrPtr->GOP.TAI.triggerGenerationTODSecLow = 0x0000003c;
    regsAddrPtr->GOP.TAI.triggerGenerationTODNanoHigh = 0x00000040;
    regsAddrPtr->GOP.TAI.triggerGenerationTODNanoLow = 0x00000044;
    regsAddrPtr->GOP.TAI.triggerGenerationTODFracHigh = 0x00000048;
    regsAddrPtr->GOP.TAI.triggerGenerationTODFracLow = 0x0000004c;
    regsAddrPtr->GOP.TAI.timeLoadValueSecHigh = 0x00000050;
    regsAddrPtr->GOP.TAI.timeLoadValueSecMed = 0x00000054;
    regsAddrPtr->GOP.TAI.timeLoadValueSecLow = 0x00000058;
    regsAddrPtr->GOP.TAI.timeLoadValueNanoHigh = 0x0000005c;
    regsAddrPtr->GOP.TAI.timeLoadValueNanoLow = 0x00000060;
    regsAddrPtr->GOP.TAI.timeLoadValueFracHigh = 0x00000064;
    regsAddrPtr->GOP.TAI.timeLoadValueFracLow = 0x00000068;
    regsAddrPtr->GOP.TAI.timeCaptureValue0SecHigh = 0x0000006c;
    regsAddrPtr->GOP.TAI.timeCaptureValue0SecMed = 0x00000070;
    regsAddrPtr->GOP.TAI.timeCaptureValue0SecLow = 0x00000074;
    regsAddrPtr->GOP.TAI.timeCaptureValue0NanoHigh = 0x00000078;
    regsAddrPtr->GOP.TAI.timeCaptureValue0NanoLow = 0x0000007c;
    regsAddrPtr->GOP.TAI.timeCaptureValue0FracHigh = 0x00000080;
    regsAddrPtr->GOP.TAI.timeCaptureValue0FracLow = 0x00000084;
    regsAddrPtr->GOP.TAI.timeCaptureValue1SecHigh = 0x00000088;
    regsAddrPtr->GOP.TAI.timeCaptureValue1SecMed = 0x0000008c;
    regsAddrPtr->GOP.TAI.timeCaptureValue1SecLow = 0x00000090;
    regsAddrPtr->GOP.TAI.timeCaptureValue1NanoHigh = 0x00000094;
    regsAddrPtr->GOP.TAI.timeCaptureValue1NanoLow = 0x00000098;
    regsAddrPtr->GOP.TAI.timeCaptureValue1FracHigh = 0x0000009c;
    regsAddrPtr->GOP.TAI.timeCaptureValue1FracLow = 0x000000a0;
    regsAddrPtr->GOP.TAI.timeCaptureStatus = 0x000000a4;
    regsAddrPtr->GOP.TAI.timeUpdateCntrLow = 0x000000a8;
    regsAddrPtr->GOP.TAI.timeUpdateCntrHigh = 0x000000fc;
    regsAddrPtr->GOP.TAI.PClkClockCycleConfigLow = 0x000000d8;
    regsAddrPtr->GOP.TAI.PClkClockCycleConfigHigh = 0x000000d4;
    regsAddrPtr->GOP.TAI.incomingClockInCountingConfigLow = 0x000000f8;
    regsAddrPtr->GOP.TAI.incomingClockInCountingEnable = 0x000000f4;
    regsAddrPtr->GOP.TAI.generateFunctionMaskSecMed = 0x000000b0;
    regsAddrPtr->GOP.TAI.generateFunctionMaskSecLow = 0x000000b4;
    regsAddrPtr->GOP.TAI.generateFunctionMaskSecHigh = 0x000000ac;
    regsAddrPtr->GOP.TAI.generateFunctionMaskNanoLow = 0x000000bc;
    regsAddrPtr->GOP.TAI.generateFunctionMaskNanoHigh = 0x000000b8;
    regsAddrPtr->GOP.TAI.generateFunctionMaskFracLow = 0x000000c4;
    regsAddrPtr->GOP.TAI.generateFunctionMaskFracHigh = 0x000000c0;
    regsAddrPtr->GOP.TAI.externalClockPropagationDelayConfigLow = 0x000000f0;
    regsAddrPtr->GOP.TAI.externalClockPropagationDelayConfigHigh = 0x000000ec;
    regsAddrPtr->GOP.TAI.driftThresholdConfigLow = 0x000000e0;
    regsAddrPtr->GOP.TAI.driftThresholdConfigHigh = 0x000000dc;
    regsAddrPtr->GOP.TAI.driftAdjustmentConfigLow = 0x000000cc;
    regsAddrPtr->GOP.TAI.driftAdjustmentConfigHigh = 0x000000c8;
    regsAddrPtr->GOP.TAI.clockCycleConfigLow = 0x000000e8;
    regsAddrPtr->GOP.TAI.clockCycleConfigHigh = 0x000000e4;
    regsAddrPtr->GOP.TAI.captureTriggerCntr = 0x000000d0;
}

/**
* @internal gopFcaInit function
* @endinternal
*
* @brief   init the DB - FCA unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] macNum                   - number of MAC unit
*                                       None
*/
static void gopFcaInit
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_U32 macNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    {/*0018060c+i*0x4*/
        GT_U32    i;
        for(i = 0 ; i <= 7 ; i++) {
            regsAddrPtr->GOP.FCA[macNum].LLFCDatabase[i] =
                0xc+i*0x4;
        }/* end of loop i */
    }/*0018060c+i*0x4*/
    {/*0018062c+p*0x4*/
        GT_U32    p;
        for(p = 0 ; p <= 7 ; p++) {
            regsAddrPtr->GOP.FCA[macNum].portSpeedTimer[p] =
                0x2c+p*0x4;
        }/* end of loop p */
    }/*0018062c+p*0x4*/
    {/*00180684+0x4*p*/
        GT_U32    p;
        for(p = 0 ; p <= 7 ; p++) {
            regsAddrPtr->GOP.FCA[macNum].lastSentTimer[p] =
                0x84+0x4*p;
        }/* end of loop p */
    }/*00180684+0x4*p*/
    regsAddrPtr->GOP.FCA[macNum].FCACtrl               = 0x0;
    regsAddrPtr->GOP.FCA[macNum].DBReg0                = 0x4;
    regsAddrPtr->GOP.FCA[macNum].DBReg1                = 0x8;
    regsAddrPtr->GOP.FCA[macNum].LLFCFlowCtrlWindow0   = 0x4c;
    regsAddrPtr->GOP.FCA[macNum].LLFCFlowCtrlWindow1   = 0x50;
    regsAddrPtr->GOP.FCA[macNum].MACDA0To15            = 0x54;
    regsAddrPtr->GOP.FCA[macNum].MACDA16To31           = 0x58;
    regsAddrPtr->GOP.FCA[macNum].MACDA32To47           = 0x5c;
    regsAddrPtr->GOP.FCA[macNum].MACSA0To15            = 0x60;
    regsAddrPtr->GOP.FCA[macNum].MACSA16To31           = 0x64;
    regsAddrPtr->GOP.FCA[macNum].MACSA32To47           = 0x68;
    regsAddrPtr->GOP.FCA[macNum].L2CtrlReg0            = 0x6c;
    regsAddrPtr->GOP.FCA[macNum].L2CtrlReg1            = 0x70;
    regsAddrPtr->GOP.FCA[macNum].DSATag0To15           = 0x74;
    regsAddrPtr->GOP.FCA[macNum].DSATag16To31          = 0x78;
    regsAddrPtr->GOP.FCA[macNum].DSATag32To47          = 0x7c;
    regsAddrPtr->GOP.FCA[macNum].DSATag48To63          = 0x80;
    regsAddrPtr->GOP.FCA[macNum].PFCPriorityMask       = 0x104;
    regsAddrPtr->GOP.FCA[macNum].sentPktsCntrLSb       = 0x108;
    regsAddrPtr->GOP.FCA[macNum].sentPktsCntrMSb       = 0x10c;
    regsAddrPtr->GOP.FCA[macNum].periodicCntrLSB       = 0x110;
    regsAddrPtr->GOP.FCA[macNum].periodicCntrMSB       = 0x114;

    return;
}

/**
* @internal gopPtpInit function
* @endinternal
*
* @brief   init the DB - GOP PTP unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - port number
*                                       None
*/
static void gopPtpInit
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_U32 portNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    /*start of unit PTP */
    regsAddrPtr->GOP.PTP[portNum].PTPInterruptCause = 0x00000000;
    regsAddrPtr->GOP.PTP[portNum].PTPInterruptMask = 0x00000004;
    regsAddrPtr->GOP.PTP[portNum].PTPGeneralCtrl = 0x00000008;
    regsAddrPtr->GOP.PTP[portNum].PTPTXTimestampQueue0Reg0 = 0x0000000c;
    regsAddrPtr->GOP.PTP[portNum].PTPTXTimestampQueue0Reg1 = 0x00000010;
    regsAddrPtr->GOP.PTP[portNum].PTPTXTimestampQueue0Reg2 = 0x00000014;
    regsAddrPtr->GOP.PTP[portNum].PTPTXTimestampQueue1Reg0 = 0x00000018;
    regsAddrPtr->GOP.PTP[portNum].PTPTXTimestampQueue1Reg1 = 0x0000001c;
    regsAddrPtr->GOP.PTP[portNum].PTPTXTimestampQueue1Reg2 = 0x00000020;
    regsAddrPtr->GOP.PTP[portNum].totalPTPPktsCntr = 0x00000024;
    regsAddrPtr->GOP.PTP[portNum].PTPv1PktCntr = 0x00000028;
    regsAddrPtr->GOP.PTP[portNum].PTPv2PktCntr = 0x0000002c;
    regsAddrPtr->GOP.PTP[portNum].Y1731PktCntr = 0x00000030;
    regsAddrPtr->GOP.PTP[portNum].NTPTsPktCntr = 0x00000034;
    regsAddrPtr->GOP.PTP[portNum].NTPReceivePktCntr = 0x00000038;
    regsAddrPtr->GOP.PTP[portNum].NTPTransmitPktCntr = 0x0000003c;
    regsAddrPtr->GOP.PTP[portNum].WAMPPktCntr = 0x00000040;
    regsAddrPtr->GOP.PTP[portNum].addCorrectedTimeActionPktCntr = 0x00000058;
    regsAddrPtr->GOP.PTP[portNum].NTPPTPOffsetHigh = 0x00000070;
    regsAddrPtr->GOP.PTP[portNum].noneActionPktCntr = 0x00000044;
    regsAddrPtr->GOP.PTP[portNum].forwardActionPktCntr = 0x00000048;
    regsAddrPtr->GOP.PTP[portNum].dropActionPktCntr = 0x0000004c;
    regsAddrPtr->GOP.PTP[portNum].captureIngrTimeActionPktCntr = 0x0000006c;
    regsAddrPtr->GOP.PTP[portNum].captureAddTimeActionPktCntr = 0x0000005c;
    regsAddrPtr->GOP.PTP[portNum].captureAddIngrTimeActionPktCntr = 0x00000068;
    regsAddrPtr->GOP.PTP[portNum].captureAddCorrectedTimeActionPktCntr = 0x00000060;
    regsAddrPtr->GOP.PTP[portNum].captureActionPktCntr = 0x00000050;
    regsAddrPtr->GOP.PTP[portNum].addTimeActionPktCntr = 0x00000054;
    regsAddrPtr->GOP.PTP[portNum].addIngrTimeActionPktCntr = 0x00000064;
    regsAddrPtr->GOP.PTP[portNum].NTPPTPOffsetLow = 0x00000074;
    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        regsAddrPtr->GOP.PTP[portNum].txPipeStatusDelay = 0x00000078;
    }
}


/**
* @internal gopCgInit function
* @endinternal
*
* @brief   init the DB - GOP CG unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - port number
*                                       None
*/
static void gopCgInit
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_U32 portNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    /*start of unit CG */
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAControl0 = 0x340000;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAControl1 = 0x340004;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersStatus = 0x340020;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersResets = 0x340010;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersIpStatus = 0x340028;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersIpStatus2 = 0x34002C;
    regsAddrPtr->GOP.CG[portNum].CG_CONVERTERS.CGMAConvertersFcControl0 = 0x340060;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACCommandConfig = 0x340408;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACaddress0 = 0x34040c;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACaddress1 = 0x340410;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACFrmLength = 0x340414;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACStatus = 0x340440;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACTxIpgLength = 0x340444;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACRxCrcOpt = 0x340448;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CGPORTMACStatNConfig = 0x340478;
    /* CG MIBS */
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_OCTETS_E] = 0x3404D0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_MC_PKTS_E] = 0x340510;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_ERRORS_E] = 0x3404F8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_UC_PKTS_E] = 0x340508;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_OUT_BC_PKTS_E] = 0x340518;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_OCTETS_E] = 0x3404D8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_ERRORS_E] = 0x340590;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_BC_PKTS_E] = 0x3404F0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_MC_PKTS_E] = 0x3404E8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_IF_IN_UC_PKTS_E] = 0x3404E0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAMES_TRANSMITTED_OK_E] = 0x340480;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAMES_RECIEVED_OK_E] = 0x340488;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAME_CHECK_SEQUENCE_ERRORS_E] = 0x340490;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_MAC_CONTROL_FRAMES_TRANSMITTED_E] = 0x340618;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_MAC_CONTROL_FRAMES_RECIEVED_E] = 0x340620;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E] = 0x3404A0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E] = 0x3404A8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E] = 0x340598;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E] = 0x3405A0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E] = 0x3405A8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E] = 0x3405B0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E] = 0x3405B8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E] = 0x3405C0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E] = 0x3405C8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E] = 0x3405D0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E] = 0x3405D8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E] = 0x3405E0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E] = 0x3405E8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E] = 0x3405F0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E] = 0x3405F8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E] = 0x340600;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E] = 0x340608;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E] = 0x340610;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_OCTETS_E] = 0x340528;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_UNDERSIZE_PKTS_E] = 0x340538;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_OVERSIZE_PKTS_E] = 0x340578;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_JABBERS_E] = 0x340580;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_DROP_EVENTS_E] = 0x340520;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_FRAGMENTS_E] = 0x340588;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PKTS_E] = 0x340530;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_64OCTETS_E] = 0x340540;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_65TO127_OCTETS_E] = 0x340548;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_128TO255_OCTETS_E] = 0x340550;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_256TO511_OCTETS_E] = 0x340558;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_512TO1023_OCTETS_E] = 0x340560;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_1024TO1518_OCTETS_E] = 0x340568;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E] = 0x340570;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_ALIGNMENT_ERRORS_E] = 0x340498;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_FRAME_TOO_LONG_ERRORS_E] = 0x3404B0;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_A_IN_RANGE_LENGTH_ERRORS_E] = 0x3404B8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_VLAN_RECIEVED_OK_E] = 0x3404C8;
    regsAddrPtr->GOP.CG[portNum].CG_PORT.CG_PORT_MAC.CG_PORT_MAC_MIBS_REG_ADDR_ARR[CPSS_CG_VLAN_TRANSMITED_OK_E] = 0x3404C0;

}

/**
* @internal gopMacPGInit function
* @endinternal
*
* @brief   Init the DB - GOP Mac PG unit register file
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PX's device number to init the struct for.
*                                       None
*/
static void gopMacPGInit
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_U32  i;
    GT_U32  portNum;
    GT_U32  patternRegId;
    GT_U32  offset;

    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *addrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    i = 0;
    for (portNum = 0; portNum < 15; portNum += 4, i++)
    {
        offset = 0x1000 * portNum;

        addrPtr->packGenConfig[i].macDa[0]             = 0x00180C00 + offset;
        addrPtr->packGenConfig[i].macDa[1]             = 0x00180C04 + offset;
        addrPtr->packGenConfig[i].macDa[2]             = 0x00180C08 + offset;
        addrPtr->packGenConfig[i].macSa[0]             = 0x00180C0C + offset;
        addrPtr->packGenConfig[i].macSa[1]             = 0x00180C10 + offset;
        addrPtr->packGenConfig[i].macSa[2]             = 0x00180C14 + offset;
        addrPtr->packGenConfig[i].etherType            = 0x00180C18 + offset;
        addrPtr->packGenConfig[i].vlanTag              = 0x00180C1C + offset;
        addrPtr->packGenConfig[i].packetLength         = 0x00180C20 + offset;
        addrPtr->packGenConfig[i].packetCount          = 0x00180C24 + offset;
        addrPtr->packGenConfig[i].ifg                  = 0x00180C28 + offset;
        addrPtr->packGenConfig[i].macDaIncrementLimit  = 0x00180C2C + offset;
        addrPtr->packGenConfig[i].controlReg0          = 0x00180C40 + offset;

        for (patternRegId = 0; patternRegId < 32; patternRegId++)
        {
            addrPtr->packGenConfig[i].dataPattern[patternRegId] = 0x00180C50 + offset + 4 * patternRegId;
        }
    }
}

/**
* @internal cncInit function
* @endinternal
*
* @brief   init the DB - CNC unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      instance    - CNC instance number
*                                       None
*/
static void cncInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    /* unit perBlockRegs */

    /* unit wraparound */
    {/*00001498+n*0x10+m*0x4*/
        GT_U32    n,m;

        for(n = 0; n < PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS; n++)
        {
            for(m = 0; m <= 3; m++)
            {
                regsAddrPtr->CNC.perBlockRegs.
                    wraparound.CNCBlockWraparoundStatusReg[n][m] =
                    0x00001498 + (n * 0x10) + (m * 0x4);
            }/* end of loop m */
        }/* end of loop n */

    }/*00001498+n*0x10+m*0x4*/

    /* unit rangeCount */
    {/*00001c98+0x10*n+j*0x4*/
        GT_U32    j,n;

        for(j = 0; j <= 1; j++)
        {
            for(n = 0; n < PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS; n++)
            {
                regsAddrPtr->CNC.perBlockRegs.
                    rangeCount.CNCBlockRangeCountEnable[j][n] =
                    0x00001c98 + (0x10 * n) + (j * 0x4);
            }/* end of loop n */
        }/* end of loop j */

    }/*00001c98+0x10*n+j*0x4*/

    /* unit clientEnable */
    {/*00001298+n*0x4*/
        GT_U32    n;

        for(n = 0; n < PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS; n++)
        {
            regsAddrPtr->CNC.perBlockRegs.
                clientEnable.CNCBlockConfigReg0[n] =
                0x00001298 + (n * 0x4);
        }/* end of loop n */

    }/*00001298+n*0x4*/

    /* unit entryMode */
    if(isSip5_20)
    {/*00001398+n*0x4*/
        GT_U32    n;

        for(n = 0; n <= 0; n++)
        {
            regsAddrPtr->CNC.perBlockRegs.
                entryMode.CNCBlocksCounterEntryModeRegister =
                0x00001398 + (n * 0x4);
        }/* end of loop n */

    }/*00001398+n*0x4*/


    /* unit memory */
    {/*0x00010000+n*0x2000*/
        GT_U32    n;

        for(n = 0; n < PRV_CPSS_PX_CNC_NUM_BLOCKS_CNS; n++)
        {
            regsAddrPtr->CNC.perBlockRegs.
                memory.baseAddress[n] =
                0x00010000 + (n * 0x2000);
        }/* end of loop n */

    }/*0x00010000+n*0x2000*/

    /* unit globalRegs */
    regsAddrPtr->CNC.globalRegs.CNCGlobalConfigReg                    = 0x00000000;
    regsAddrPtr->CNC.globalRegs.CNCFastDumpTriggerReg                 = 0x00000030;
    regsAddrPtr->CNC.globalRegs.CNCClearByReadValueRegWord0           = 0x00000040;
    regsAddrPtr->CNC.globalRegs.CNCClearByReadValueRegWord1           = 0x00000044;
    regsAddrPtr->CNC.globalRegs.CNCInterruptSummaryCauseReg           = 0x00000100;
    regsAddrPtr->CNC.globalRegs.CNCInterruptSummaryMaskReg            = 0x00000104;
    regsAddrPtr->CNC.globalRegs.wraparoundFunctionalInterruptCauseReg = 0x00000190;
    regsAddrPtr->CNC.globalRegs.wraparoundFunctionalInterruptMaskReg  = 0x000001a4;
    regsAddrPtr->CNC.globalRegs.rateLimitFunctionalInterruptCauseReg  = 0x000001b8;
    regsAddrPtr->CNC.globalRegs.rateLimitFunctionalInterruptMaskReg   = 0x000001cc;
    regsAddrPtr->CNC.globalRegs.miscFunctionalInterruptCauseReg       = 0x000001e0;
    regsAddrPtr->CNC.globalRegs.miscFunctionalInterruptMaskReg        = 0x000001e4;
}

/**
* @internal txqDqInit function
* @endinternal
*
* @brief   This function inits the DB - per TXQ_DQ unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] index                    - unit index
*                                       None.
*/
static void txqDqInit
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32  index
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numTxqDqPortsPerUnit_firstBlock = (PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS - 1);
    GT_U32  numTxqDqPortsPerUnit_secondBlock = 0;
    GT_U32  ii;

    { /*start of unit TXQ */

        { /*start of unit dq */

            { /*start of unit shaper */
                { /*00004100+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].shaper.portTokenBucketMode[n] =
                            0x00004100+n*0x4;

                    } /* end of loop n */
                    {
                        GT_U32  baseAddr = 0x00010000;
                        /* continue with n */
                        for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                        {
                            regsAddrPtr->TXQ.dq[index].shaper.portTokenBucketMode[n] =
                                baseAddr+n*0x4;
                        }
                    }
                } /*00004100+n*0x4*/
                { /*00004300+n * 0x4*/
                    GT_U32    n;
                    for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].shaper.portDequeueEnable[n] =
                        0x00004300+n * 0x4;
                    } /* end of loop n */
                    {
                        GT_U32  baseAddr = 0x00010100;
                        /* continue with n */
                        for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                        {
                            regsAddrPtr->TXQ.dq[index].shaper.portDequeueEnable[n] =
                                baseAddr+n*0x4;
                        }
                    }
                } /*00004300+n * 0x4*/
                regsAddrPtr->TXQ.dq[index].shaper.tokenBucketUpdateRate = 0x00004000;
                regsAddrPtr->TXQ.dq[index].shaper.tokenBucketBaseLine = 0x00004004;
                regsAddrPtr->TXQ.dq[index].shaper.CPUTokenBucketMTU = 0x00004008;
                regsAddrPtr->TXQ.dq[index].shaper.portsTokenBucketMTU = 0x0000400c;

            } /*end of unit shaper */

            { /*start of unit scheduler */
                { /*start of unit schedulerConfig */
                    { /*00001040+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.conditionalMaskForPort[n] =
                            0x00001040+n * 0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F000;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.conditionalMaskForPort[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00001040+n * 0x4*/
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.schedulerConfig = 0x00001000;
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.pktLengthForTailDropDequeue = 0x00001008;
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.highSpeedPorts[index] = 0x0000100c;

                } /*end of unit schedulerConfig */


                { /*start of unit priorityArbiterWeights */
                    { /*000012c0+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRRGroup[p] =
                            0x000012c0+p*0x4;
                        } /* end of loop p */
                    } /*000012c0+p*0x4*/
                    { /*00001240+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[p] =
                            0x00001240+p*0x4;
                        } /* end of loop p */
                    } /*00001240+p*0x4*/
                    { /*00001280+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg1[p] =
                            0x00001280+p*0x4;
                        } /* end of loop p */
                    } /*00001280+p*0x4*/
                    { /*00001300+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.profileSDWRREnable[p] =
                            0x00001300+p*0x4;
                        } /* end of loop p */
                    } /*00001300+p*0x4*/
                    { /*00001380+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.portSchedulerProfile[n] =
                            0x00001380+n * 0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F100;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.priorityArbiterWeights.portSchedulerProfile[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00001380+n * 0x4*/

                } /*end of unit priorityArbiterWeights */


                { /*start of unit portShaper */
                    { /*0000190c+0x4*n*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.portShaper.portRequestMaskSelector[n] =
                            0x0000190c+0x4*n;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F300;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.portShaper.portRequestMaskSelector[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*0000190c+0x4*n*/
                    regsAddrPtr->TXQ.dq[index].scheduler.portShaper.portRequestMask = 0x00001904;

                } /*end of unit portShaper */

                { /*start of unit portArbiterConfig */
                    { /*00001510+n*0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[n] =
                            0x00001510+n*0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000F200;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00001510+n*0x4*/
                    { /*00001650+n*4*/
                        GT_U32    n;
                        /*-----------------------------------------------*
                         * BC2/sip5_15/BC3 used 85 registers, Aldrin, AC3X -- 96  *
                         *-----------------------------------------------*/
                        GT_U32    size = sizeof(regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterMap)/
                                         sizeof(regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterMap[0]);
                        GT_U32    N;

                        #define PRV_CPSS_PX_PA_MAP_REGNUM_CNS                    85
                        N = PRV_CPSS_PX_PA_MAP_REGNUM_CNS;

                        if (N >= size)
                        {
                             CPSS_LOG_INFORMATION_MAC("TXQ pizza arbiter map register size greater than size");
                        }
                        for(n = 0; n < N; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterMap[n] =
                            0x00001650+n*4;
                        } /* end of loop n */
                    } /*00001650+n*4*/
                    regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterConfig = 0x00001500;
                    regsAddrPtr->TXQ.dq[index].scheduler.portArbiterConfig.portsArbiterStatus = 0x00001800;

                } /*end of unit portArbiterConfig */


            } /*end of unit scheduler */

            { /*start of unit global */
                { /*start of unit memoryParityError */
                    regsAddrPtr->TXQ.dq[index].global.memoryParityError.tokenBucketPriorityParityErrorCntr = 0x00000800;
                    regsAddrPtr->TXQ.dq[index].global.memoryParityError.parityErrorBadAddr = 0x00000808;

                } /*end of unit memoryParityError */


                { /*start of unit globalDQConfig */
                    { /*000002c0+n*0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portToDMAMapTable[n] =
                            0x000002c0+n*0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E100;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portToDMAMapTable[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*000002c0+n*0x4*/
                    { /*0000020c+p*0x4*/
                        GT_U32    p;
                        for(p = 0; p <= 15; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.profileByteCountModification[p] =
                            0x0000020c+p*0x4;
                        } /* end of loop p */
                    } /*0000020c+p*0x4*/
                    { /*00000004+n*0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portBCConstantValue[n] =
                            0x00000004+n*0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E000;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].global.globalDQConfig.portBCConstantValue[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00000004+n*0x4*/
                    regsAddrPtr->TXQ.dq[index].global.globalDQConfig.globalDequeueConfig = 0x00000000;
                    regsAddrPtr->TXQ.dq[index].global.globalDQConfig.BCForCutThrough = 0x00000208;

                    regsAddrPtr->TXQ.dq[index].global.globalDQConfig.creditCountersReset = 0x00000260;

                } /*end of unit globalDQConfig */

                { /*start of unit flushTrig */
                    { /*00000400+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n <= numTxqDqPortsPerUnit_firstBlock; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.flushTrig.portTxQFlushTrigger[n] =
                            0x00000400+n * 0x4;
                        } /* end of loop n */
                        {
                            GT_U32  baseAddr = 0x0000E400;
                            /* continue with n */
                            for(ii = 0 ; ii < numTxqDqPortsPerUnit_secondBlock ; ii++,n++)
                            {
                                regsAddrPtr->TXQ.dq[index].global.flushTrig.portTxQFlushTrigger[n] =
                                    baseAddr+n*0x4;
                            }
                        }
                    } /*00000400+n * 0x4*/

                } /*end of unit flushTrig */


                { /*start of unit ECCCntrs */
                    regsAddrPtr->TXQ.dq[index].global.ECCCntrs.DQIncECCErrorCntr = 0x00000920;
                    regsAddrPtr->TXQ.dq[index].global.ECCCntrs.DQClrECCErrorCntr = 0x00000924;

                } /*end of unit ECCCntrs */


                { /*start of unit dropCntrs */
                    regsAddrPtr->TXQ.dq[index].global.dropCntrs.egrMirroredDroppedCntr = 0x00000900;
                    regsAddrPtr->TXQ.dq[index].global.dropCntrs.STCDroppedCntr = 0x00000904;

                } /*end of unit dropCntrs */

                if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
                {
                    { /*00000B00+n * 0x4*/
                        GT_U32    n;
                        for(n = 0; n < PRV_CPSS_PX_DMA_PORTS_NUM_CNS; n++)
                        {
                            regsAddrPtr->TXQ.dq[index].global.creditCounters.txdmaPortCreditCounter[n] =
                                      0x00000b00+n * 0x4;
                        } /* end of loop n */
                    } /*00000b00+n * 0x4*/
                }
            } /*end of unit global */


            { /*start of unit flowCtrlResponse */
                { /*0000c100+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= 15; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[n] =
                        0x0000c100+n*0x4;
                    } /* end of loop n */
                } /*0000c100+n*0x4*/
                { /*0000c180+0x40*t+0x4*p*/
                    GT_U32    p,t;
                    for(p = 0; p <= 15; p++)
                    {
                        for(t = 0; t <= 7; t++)
                        {
                            regsAddrPtr->TXQ.dq[index].flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[p][t] =
                            0x0000c180+0x40*t+0x4*p;
                        } /* end of loop t */
                    } /* end of loop p */
                } /*0000c180+0x40*t+0x4*p*/
                { /*0000c008+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= 15; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].flowCtrlResponse.schedulerProfileLLFCXOFFValue[n] =
                        0x0000c008+n*0x4;
                    } /* end of loop n */
                } /*0000c008+n*0x4*/
                { /*0000c500+p*4*/
                    GT_U32    p;
                    /* in sip5.20 implemented as table, not registers */
                    if(0 == PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                    {
                        for(p = 0; p <= 255; p++)
                        {
                            regsAddrPtr->TXQ.dq[index].flowCtrlResponse.portToTxQPortMap[p] =
                                0x0000c500+p*4;
                        } /* end of loop p */
                    }
                } /*0000c500+p*4*/
                { /*0000c080+n*0x4*/
                    GT_U32    n;
                    for(n = 0; n <= 7; n++)
                    {
                        regsAddrPtr->TXQ.dq[index].flowCtrlResponse.PFCTimerToPriorityQueueMap[n] =
                        0x0000c080+n*0x4;
                    } /* end of loop n */
                } /*0000c080+n*0x4*/
                regsAddrPtr->TXQ.dq[index].flowCtrlResponse.flowCtrlResponseConfig = 0x0000c000;

            } /*end of unit flowCtrlResponse */

        } /*end of unit dq */

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            { /*0000100c+n*0x4*/
                GT_U32    n;
                for(n = 0; n <= 1; n++)
                {
                    regsAddrPtr->TXQ.dq[index].scheduler.schedulerConfig.highSpeedPorts[n] =
                        0x0000100c+n*0x4;
                } /* end of loop n */
            } /*0000100c+n*0x4*/

            regsAddrPtr->TXQ.dq[index].scheduler.portShaper.fastPortShaperThreshold =
                0x00001908;
        }

    } /*end of unit TXQ */

}


/**
* @internal txqDqUpdateUnit0AsGlobalPortsAddr function
* @endinternal
*
* @brief   update the DB - txq-DQ unit - after multi units set 'per port' addresses.
*         the DB of TXQ.dq[0] updated for the ports with 'global port index'
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] currentUnitIndex         - the index of current TXQ.dq[] unit
* @param[in] portIn_unit_0            - the 'global port' index (in unit 0)
* @param[in] portInCurrentUnit        - the 'local port'  index (in current unit)
* @param[in] numOfPorts               - number of ports to update.
*                                       None
*/
static void txqDqUpdateUnit0AsGlobalPortsAddr
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32  currentUnitIndex,
    IN  GT_U32  portIn_unit_0,
    IN  GT_U32  portInCurrentUnit,
    IN  GT_U32  numOfPorts
)
{
    GT_U32    saveLastNumOfPorts=0;/*number of ports from the end of the unitStart1Ptr[]
                                  that need to be saved before overridden by unitStart2Ptr[]*/
    GT_U32    saveToIndex=0;/*the index in unitStart1Ptr[] to save the 'last ports'
                                  valid when saveLastNumOfPorts != 0*/

    /************************************/
    /*   update the per port addresses  */
    /*   of the txq-DQ unit 0 with those */
    /*   of unit 1                      */
    /************************************/

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.portShaper.portRequestMaskSelector,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.priorityArbiterWeights.portSchedulerProfile,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , scheduler.schedulerConfig.conditionalMaskForPort,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , shaper.portDequeueEnable,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , shaper.portTokenBucketMode,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , global.globalDQConfig.portToDMAMapTable,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , global.globalDQConfig.portBCConstantValue,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );

    regUnitPerPortAddrUpdate(
        REG_NAME_AND_SIZE_FOR_2_UNITS_MAC(devNum, TXQ.dq[0] , TXQ.dq[currentUnitIndex] , global.flushTrig.portTxQFlushTrigger,
            portIn_unit_0 , portInCurrentUnit,numOfPorts,saveLastNumOfPorts,saveToIndex, NULL)
        );
}

/**
* @internal txqQcnInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_QCN unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqQcnInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    {/*start of unit qcn */
        {/*00000b40+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 71 ; n++) {
                regsAddrPtr->TXQ.qcn.CCFCSpeedIndex[n] =
                    0x00000d00+0x4*n;
            }/* end of loop n */
        }/*00000b40+0x4*n*/
        {/*00000090+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 7 ; n++) {
                regsAddrPtr->TXQ.qcn.CNSampleTbl[n] =
                    0x00000090+0x4*n;
            }/* end of loop n */
        }/*00000090+0x4*n*/
        {/*00000300+0x20*p + 0x4*t*/
            GT_U32    p,t, maxProfile;
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                maxProfile = 15;
            }
            else
            {
                maxProfile = 7;
            }

            for(p = 0 ; p <= maxProfile ; p++) {
                for(t = 0 ; t <= 7 ; t++) {
                    regsAddrPtr->TXQ.qcn.profilePriorityQueueCNThreshold[p][t] =
                        0x00000300+0x20*p + 0x4*t;
                }/* end of loop t */
            }/* end of loop p */
        }/*00000300+0x20*p + 0x4*t*/
        regsAddrPtr->TXQ.qcn.feedbackMIN = 0x00000050;
        regsAddrPtr->TXQ.qcn.feedbackMAX = 0x00000060;
        regsAddrPtr->TXQ.qcn.CNGlobalConfig = 0x00000000;
        {/*00000b20+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TXQ.qcn.enFCTriggerByCNFrameOnPort[n] =
                        0x00000b20+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->TXQ.qcn.enFCTriggerByCNFrameOnPort[n] =
                        0x00000c00+0x4*n;
                }/* end of loop n */
            }
        }/*00000b20+0x4*n*/
        {/*00000b00+0x4*n*/
            GT_U32    n;
            if(0 == isSip5_20)
            {
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrPtr->TXQ.qcn.enCNFrameTxOnPort[n] =
                        0x00000b00+0x4*n;
                }/* end of loop n */
            }
            else
            {
                for(n = 0 ; n <= 15 ; n++) {
                    regsAddrPtr->TXQ.qcn.enCNFrameTxOnPort[n] =
                        0x00000b00+0x4*n;
                }/* end of loop n */
            }
        }/*00000b00+0x4*n*/
        regsAddrPtr->TXQ.qcn.feedbackCalcConfigs = 0x00000040;
        regsAddrPtr->TXQ.qcn.txqECCConf = 0x00000130;
        regsAddrPtr->TXQ.qcn.descECCSingleErrorCntr = 0x00000120;
        regsAddrPtr->TXQ.qcn.descECCDoubleErrorCntr = 0x00000124;
        regsAddrPtr->TXQ.qcn.CNTargetAssignment = 0x00000004;
        regsAddrPtr->TXQ.qcn.CNDropCntr = 0x000000f0;
        regsAddrPtr->TXQ.qcn.QCNInterruptCause = 0x00000100;
        regsAddrPtr->TXQ.qcn.QCNInterruptMask = 0x00000110;
        regsAddrPtr->TXQ.qcn.CNDescAttributes = 0x00000008;
        regsAddrPtr->TXQ.qcn.CNBufferFIFOParityErrorsCntr = 0x000000e0;
        regsAddrPtr->TXQ.qcn.CNBufferFIFOOverrunsCntr = 0x000000d0;
        regsAddrPtr->TXQ.qcn.ECCStatus = 0x00000140;
        regsAddrPtr->TXQ.qcn.cnAvailableBuffers = 0x0000000C;

    }/*end of unit qcn */

}

/**
* @internal txqBmxInit function
* @endinternal
*
* @brief   This function inits the DB - BMX unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqBmxInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  isSip5_20 = PRV_CPSS_SIP_5_20_CHECK_MAC(devNum);

    {/*start of unit bmx */
        if(isSip5_20)
        {
            regsAddrPtr->TXQ.bmx.fillLevelDebugRegister = 0x00000004;
        }
    }/*end of unit bmx */

}

/**
* @internal txqLLInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_LL unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqLLInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit ll */
        {/*start of unit global */

            regsAddrPtr->TXQ.ll.global.globalLLConfig.Global_Configuration = 0x00110000;
        }/*end of unit global */
    }/*end of unit ll */

}

/**
* @internal txqPfcInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_PFC unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqPfcInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    {/*start of unit pfc */

        {/*00000170+0x4*n*/
            GT_U32    n;
            for(n = 0 ; n <= 4 ; n++) {
                regsAddrPtr->TXQ.pfc.LLFCTargetPortRemap[n] =
                    0x000002F0+0x4*n;
            }/* end of loop n */
        }/*00000170+0x4*n*/
        /*only oneportFCMode register in PIPE*/
        regsAddrPtr->TXQ.pfc.portFCMode[0] = 0x000006F0;
        {/*00000900+p*4*/
            GT_U32    p;
            for(p = 0 ; p <= 31 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCSourcePortToPFCIndexMap[p] =
                    0x00001C00+p*4;
            }/* end of loop p */
        }/*00000900+p*4*/
        {/*000000f0+0x4*r*/
            GT_U32    r;
            for(r = 0 ; r <= 2 ; r++) {
                regsAddrPtr->TXQ.pfc.PFCPortProfile[r] =
                    0x000000f0+0x4*r;
            }/* end of loop r */
        }/*000000f0+0x4*r*/
        {/*00000050+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry0[p] =
                    0x00000050+0x4*p;
            }/* end of loop p */
        }/*00000050+0x4*p*/
        {/*00000070+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[p] =
                    0x00000070+0x4*p;
            }/* end of loop p */
        }/*00000070+0x4*p*/
        {/*00000090+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[p] =
                    0x00000090+0x4*p;
            }/* end of loop p */
        }/*00000090+0x4*p*/
        {/*00000030+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupCntrsStatus[p] =
                    0x00000030+0x4*p;
            }/* end of loop p */
        }/*00000030+0x4*p*/
        {/*000000d0+0x4*p*/
            GT_U32    p;
            for(p = 0 ; p <= 0 ; p++) {
                regsAddrPtr->TXQ.pfc.PFCPortGroupCntrsParityErrorsCntr[p] =
                    0x000000d0+0x4*p;
            }/* end of loop p */
        }/*000000d0+0x4*p*/
        {/*00014200+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(t = 0 ; t <= 7 ; t++) {
                for(p = 0 ; p <= 0 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCXonThresholds[t][p] =
                        0x00014200+0x20*p + 0x4*t;
                }/* end of loop p */
            }/* end of loop t */
        }/*00014200+0x20*p + 0x4*t*/
        {/*00014000+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(t = 0 ; t <= 7 ; t++) {
                for(p = 0 ; p <= 0 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCXoffThresholds[t][p] =
                        0x00014000+0x20*p + 0x4*t;
                }/* end of loop p */
            }/* end of loop t */
        }/*00014000+0x20*p + 0x4*t*/
        {/*00014400+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(t = 0 ; t <= 7 ; t++) {
                for(p = 0 ; p <= 0 ; p++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCDropThresholds[t][p] =
                        0x00014400+0x20*p + 0x4*t;
                }/* end of loop p */
            }/* end of loop t */
        }/*00014400+0x20*p + 0x4*t*/
        {/*00000d00+0x20*p + 0x4*t*/
            GT_U32    p,t;
            for(p = 0 ; p <= 0 ; p++) {
                for(t = 0 ; t <= 7 ; t++) {
                    regsAddrPtr->TXQ.pfc.PFCGroupOfPortsTCCntr[p][t] =
                        0x00000d00+0x20*p + 0x4*t;
                }/* end of loop t */
            }/* end of loop p */
        }/*00000d00+0x20*p + 0x4*t*/
        {/*01800+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(p = 0 ; p <= 7 ; p++) {
                for(t = 0 ; t <= 7 ; t++) {
                    regsAddrPtr->TXQ.pfc.FCModeProfileTCXOffThresholds[p][t] =
                        0x001800+0x20*p + 0x4*t;
                }/* end of loop t */
            }/* end of loop p */
        }/*01800+0x20*p + 0x4*t*/
        {/*01800+0x20*p + 0x4*t*/
            GT_U32    t,p;
            for(p = 0 ; p <= 7 ; p++) {
                for(t = 0 ; t <= 7 ; t++) {
                    regsAddrPtr->TXQ.pfc.FCModeProfileTCXOnThresholds[p][t] =
                        0x001A00+0x20*p + 0x4*t;
                }/* end of loop t */
            }/* end of loop p */
        }/*01800+0x20*p + 0x4*t*/

        regsAddrPtr->TXQ.pfc.PFCTriggerGlobalConfig = 0x00000000;
        {/*dba*/
            {
                GT_U32    t,p;
                for(p = 0 ; p <= 7 ; p++)
                {
                    for(t = 0 ; t <= 7 ; t++)
                    {
                        regsAddrPtr->TXQ.pfc.dba.PFCProfileTCAlphaThresholds[p][t] = 0x003000 +0x20*p + 0x4*t;
                    }
                }
            }
            regsAddrPtr->TXQ.pfc.dba.PFCAvailableBuffers = 0x003100;
            regsAddrPtr->TXQ.pfc.dba.PFCDynamicBufferAllocationDisable = 0x003104;
            {
                GT_U32    p;
                for(p = 0 ; p <=16 ; p++)
                {
                    regsAddrPtr->TXQ.pfc.dba.PFCXON_XOFFstatus[p] = 0x03200 + 0x4*p;
                }
            }
        }/*end dba*/


    }/*end of unit pfc */

    {/*000000a0+0x4*p*/
        GT_U32    p;
        for(p = 0 ; p <= 0 ; p++) {
            regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunCntr[p] =
                0x000000a0+0x4*p;
        }/* end of loop p */
    }/*000000a0+0x4*p*/
    {/*00000060+0x4*p*/
        GT_U32    p;
        for(p = 0 ; p <= 0 ; p++) {
            regsAddrPtr->TXQ.pfc.PFCPortGroupFlowCtrlIndicationFIFOOverrunEntry1[p] =
                0x00000060+0x4*p;
        }/* end of loop p */
    }/*00000060+0x4*p*/
}


/**
* @internal txqQueueInit function
* @endinternal
*
* @brief   This function inits the DB - TXQ_Q unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static void txqQueueInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numOfPorts = PRV_CPSS_PX_PORTS_NUM_CNS;

    {/*start of unit queue */
        {/*start of unit tailDrop */
            {/*start of unit tailDropLimits */
                {/*000a0a20+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.poolBufferLimits[t] =
                            0x000a0a20+t*0x4;
                    }/* end of loop t */
                }/*000a0a20+t*0x4*/
                {/*000a0a00+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.poolDescLimits[t] =
                            0x000a0a00+t*0x4;
                    }/* end of loop t */
                }/*000a0a00+t*0x4*/
                {/*000a0820+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.profilePortDescLimits[n] =
                            0x000a0820+0x4*n;
                    }/* end of loop n */
                }/*000a0820+0x4*n*/
                {/*000a0900+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.profilePortBufferLimits[n] =
                            0x000a0900+0x4*n;
                    }/* end of loop n */
                }/*000a0900+0x4*n*/
                regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.globalDescsLimit = 0x000a0800;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropLimits.globalBufferLimit = 0x000a0810;

            }/*end of unit tailDropLimits */


            {/*start of unit tailDropCntrs */
                {/*000a4130+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= numOfPorts-1 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.portBuffersCntr[n] =
                            0x000a4130+n*0x4;
                    }/* end of loop n */
                }/*000a4130+n*0x4*/
                {/*000a4010+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= numOfPorts-1 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.portDescCntr[n] =
                            0x000a4010+n*0x4;
                    }/* end of loop n */
                }/*000a4010+n*0x4*/
                {/*000a4250+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.priorityDescCntr[t] =
                            0x000a4250+t*0x4;
                    }/* end of loop t */
                }/*000a4250+t*0x4*/
                {/*000a4290+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.priorityBuffersCntr[t] =
                            0x000a4290+t*0x4;
                    }/* end of loop t */
                }/*000a4290+t*0x4*/
                regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.totalDescCntr = 0x000a4000;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropCntrs.totalBuffersCntr = 0x000a4004;

            }/*end of unit tailDropCntrs */


            {/*start of unit tailDropConfig */
                {/*000a0200+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= numOfPorts-1 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portTailDropCNProfile[n] =
                            0x000a0200+n * 0x4;
                    }/* end of loop n */
                }/*000a0200+n * 0x4*/
                {/*000a0050+n * 0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= numOfPorts-1 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portEnqueueEnable[n] =
                            0x000a0050+n * 0x4;
                    }/* end of loop n */
                }/*000a0050+n * 0x4*/
                {/*000a0004+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileTailDropConfig[p] =
                            0x000a0004+p*0x4;
                    }/* end of loop p */
                }/*000a0004+p*0x4*/
                {/*000a03d0+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePriorityQueueToPoolAssociation[p] =
                            0x000a03d0+0x4*p;
                    }/* end of loop p */
                }/*000a03d0+0x4*p*/
                {/*000a03a0+t*0x4*/
                    GT_U32    t;
                    for(t = 0 ; t <= 7 ; t++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePriorityQueueEnablePoolUsage[t] =
                            0x000a03a0+t*0x4;
                    }/* end of loop t */
                }/*000a03a0+t*0x4*/
                {/*000a05c0+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePoolWRTDEn[p] =
                            0x000a05c0+0x4*p;
                    }/* end of loop p */
                }/*000a05c0+0x4*p*/
                {/*000a0580+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileQueueWRTDEn[p] =
                            0x000a0580+0x4*p;
                    }/* end of loop p */
                }/*000a0580+0x4*p*/
                {/*000a0540+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profilePortWRTDEn[p] =
                            0x000a0540+0x4*p;
                    }/* end of loop p */
                }/*000a0540+0x4*p*/
                {/*000a0500+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileMaxQueueWRTDEn[p] =
                            0x000a0500+0x4*p;
                    }/* end of loop p */
                }/*000a0500+0x4*p*/
                {/*000a0600+0x4*p*/
                    GT_U32    p;
                    for(p = 0 ; p <= 15 ; p++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileECNMarkEn[p] =
                            0x000a0600+0x4*p;
                    }/* end of loop p */
                }/*000a0600+0x4*p*/
                {/*000a0360+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.profileCNMTriggeringEnable[n] =
                            0x000a0360+0x4*n;
                    }/* end of loop n */
                }/*000a0360+0x4*n*/
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.globalTailDropConfig = 0x000a0000;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.pktLengthForTailDropEnqueue = 0x000a0044;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.maxQueueWRTDMasks = 0x000a0440;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.dynamicAvailableBuffers = 0x000a0450;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.portWRTDMasks = 0x000a0444;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.queueWRTDMasks = 0x000a0448;
                regsAddrPtr->TXQ.queue.tailDrop.tailDropConfig.poolWRTDMasks = 0x000a044c;

            }/*end of unit tailDropConfig */


            {/*start of unit resourceHistogram */
                {/*start of unit resourceHistogramLimits */
                    regsAddrPtr->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramLimits.resourceHistogramLimitReg1 = 0x000a9500;
                    regsAddrPtr->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramLimits.resourceHistogramLimitReg2 = 0x000a9504;

                }/*end of unit resourceHistogramLimits */


                {/*start of unit resourceHistogramCntrs */
                    {/*000a9510+n*0x4*/
                        GT_U32    n;
                        for(n = 0 ; n <= 3 ; n++) {
                            regsAddrPtr->TXQ.queue.tailDrop.resourceHistogram.resourceHistogramCntrs.resourceHistogramCntr[n] =
                                0x000a9510+n*0x4;
                        }/* end of loop n */
                    }/*000a9510+n*0x4*/

                }/*end of unit resourceHistogramCntrs */


            }/*end of unit resourceHistogram */


            {/*start of unit mcFilterLimits */
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mirroredPktsToAnalyzerPortDescsLimit = 0x000a9000;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.ingrMirroredPktsToAnalyzerPortBuffersLimit = 0x000a9004;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.egrMirroredPktsToAnalyzerPortBuffersLimit = 0x000a9010;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mcDescsLimit = 0x000a9020;
                regsAddrPtr->TXQ.queue.tailDrop.mcFilterLimits.mcBuffersLimit = 0x000a9030;

            }/*end of unit mcFilterLimits */


            {/*start of unit muliticastFilterCntrs */
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.snifferDescsCntr = 0x000a9200;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.ingrSnifferBuffersCntr = 0x000a9204;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.egrSnifferBuffersCntr = 0x000a9208;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.mcDescsCntr = 0x000a920c;
                regsAddrPtr->TXQ.queue.tailDrop.muliticastFilterCntrs.mcBuffersCntr = 0x000a9210;

            }/*end of unit muliticastFilterCntrs */


            {/*start of unit FCUAndQueueStatisticsLimits */
                {/*000aa1b0+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.XONLimit[n] =
                            0x000aa1b0+n*0x4;
                    }/* end of loop n */
                }/*000aa1b0+n*0x4*/
                {/*000aa110+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 7 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.XOFFLimit[n] =
                            0x000aa110+n*0x4;
                    }/* end of loop n */
                }/*000aa110+n*0x4*/
                {/*000aa0d0+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.queueXONLimitProfile[n] =
                            0x000aa0d0+n*0x4;
                    }/* end of loop n */
                }/*000aa0d0+n*0x4*/
                {/*000aa090+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.queueXOFFLimitProfile[n] =
                            0x000aa090+n*0x4;
                    }/* end of loop n */
                }/*000aa090+n*0x4*/
                {/*000aa050+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.portXONLimitProfile[n] =
                            0x000aa050+n*0x4;
                    }/* end of loop n */
                }/*000aa050+n*0x4*/
                {/*000aa004+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.portXOFFLimitProfile[n] =
                            0x000aa004+n*0x4;
                    }/* end of loop n */
                }/*000aa004+n*0x4*/
                regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.globalXONLimit = 0x000aa230;
                regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.globalXOFFLimit = 0x000aa210;
                regsAddrPtr->TXQ.queue.tailDrop.FCUAndQueueStatisticsLimits.FCUMode = 0x000aa000;

            }/*end of unit FCUAndQueueStatisticsLimits */


        }/*end of unit tailDrop */


        {/*start of unit global */

            {/*start of unit ECCCntrs */
                regsAddrPtr->TXQ.queue.global.ECCCntrs.TDClrECCErrorCntr = 0x00090a40;

            }/*end of unit ECCCntrs */


            {/*start of unit dropCntrs */
                {/*00090a00+p*0x4*/
                    GT_U32    p;
                    for(p = 0 ; p <= 3 ; p++) {
                        regsAddrPtr->TXQ.queue.global.dropCntrs.clearPktsDroppedCntrPipe[p] =
                            0x00090a00+p*0x4;
                    }/* end of loop p */
                }/*00090a00+p*0x4*/

            }/*end of unit dropCntrs */


        }/*end of unit global */


        {/*start of unit peripheralAccess */
            {/*start of unit peripheralAccessMisc */
                {/*00093004+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 3 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.peripheralAccessMisc.portGroupDescToEQCntr[n] =
                            0x00093004+0x4*n;
                    }/* end of loop n */
                }/*00093004+0x4*n*/
                regsAddrPtr->TXQ.queue.peripheralAccess.peripheralAccessMisc.peripheralAccessMisc = 0x00093000;
                regsAddrPtr->TXQ.queue.peripheralAccess.peripheralAccessMisc.QCNIncArbiterCtrl = 0x00093020;

            }/*end of unit peripheralAccessMisc */


            {/*start of unit egrMIBCntrs */
                {/*00093208+n*0x4*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.txQMIBCntrsPortSetConfig[n] =
                            0x00093208+n*0x4;
                    }/* end of loop n */
                }/*00093208+n*0x4*/
                {/*00093200+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.txQMIBCntrsSetConfig[n] =
                            0x00093200+0x4*n;
                    }/* end of loop n */
                }/*00093200+0x4*n*/
                {/*00093250+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setTailDroppedPktCntr[n] =
                            0x00093250+0x4*n;
                    }/* end of loop n */
                }/*00093250+0x4*n*/
                {/*00093210+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 1 ; n++) {
                        regsAddrPtr->TXQ.queue.peripheralAccess.egrMIBCntrs.setOutgoingPktCntr[n] =
                            0x00093210+0x4*n;
                    }/* end of loop n */
                }/*00093210+0x4*n*/

            }/*end of unit egrMIBCntrs */


            {/*start of unit CNCModes */
                regsAddrPtr->TXQ.queue.peripheralAccess.CNCModes.CNCModes = 0x000936a0;

            }/*end of unit CNCModes */


        }/*end of unit peripheralAccess */


    }/*end of unit queue */

}

/**
* @internal prv_smiInstanceInit function
* @endinternal
*
* @brief   init the DB - SMI unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      instanceCnt    - CNC instance number
*                                       None
*/
static void prv_smiInstanceInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_U32 phyRegCnt;
    GT_U32 phyRegN;

    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    regsAddrPtr->GOP.SMI.SMIManagement            = 0x00000000;
    regsAddrPtr->GOP.SMI.SMIMiscConfiguration     = 0x00000004;
    regsAddrPtr->GOP.SMI.PHYAutoNegotiationConfig = 0x00000008;

    phyRegN = sizeof(regsAddrPtr->GOP.SMI.PHYAddress)/sizeof(regsAddrPtr->GOP.SMI.PHYAddress[0]);
    for (phyRegCnt = 0 ; phyRegCnt < phyRegN; phyRegCnt++)
    {
        regsAddrPtr->GOP.SMI.PHYAddress[phyRegCnt] = 0x0000000C + 0x4*phyRegCnt;
    }
}

/**
* @internal prv_smiInit function
* @endinternal
*
* @brief   init the DB - SMI unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void prv_smiInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_U32  baseAddr;

    prv_smiInstanceInit(devNum);
    baseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_SMI_0_E,NULL);
    /* set the addresses of the SMI unit registers according to the needed base address */
    regUnitBaseAddrSet(
        REG_UNIT_INFO_MAC(devNum,baseAddr,GOP.SMI));
}



/**
* @internal prv_ledInstanceInit function
* @endinternal
*
* @brief   init the DB - LMS unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      instanceCnt    - CNC instance number
*                                       None
*/
static void prv_ledInstanceInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_U32  ii;
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  offset = 0;

    regsAddrPtr->GOP.LED.LEDControl                 = offset + 0x00000000;
    regsAddrPtr->GOP.LED.blinkGlobalControl         = offset + 0x00000004;
    regsAddrPtr->GOP.LED.classGlobalControl         = offset + 0x00000008;
    regsAddrPtr->GOP.LED.classesAndGroupConfig      = offset + 0x0000000C;
    for(ii = 0 ; ii < 6 ; ii++)
    {
        regsAddrPtr->GOP.LED.classForcedData[ii] =
            offset + 0x00000010 + 4*ii;
    }
    for(ii = 0 ; ii < 12 ; ii++)
    {
        regsAddrPtr->GOP.LED.portIndicationSelect[ii] =
            offset + 0x00000028 + 4*ii;
    }
    for(ii = 0 ; ii < 1 ; ii++)
    {
        regsAddrPtr->GOP.LED.portTypeConfig[ii] =
            offset + 0x00000120 + 4*ii;
    }
    for(ii = 0 ; ii < 3 ; ii++)
    {
        regsAddrPtr->GOP.LED.portIndicationPolarity[ii] =
            offset + 0x00000128 + 4*ii;
    }
    for(ii = 0 ; ii < 6 ; ii++)
    {
        regsAddrPtr->GOP.LED.classRawData[ii] =
            offset + 0x00000148 + 4*ii;
    }
}

/**
* @internal prv_ledInit function
* @endinternal
*
* @brief   init the DB - LED unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      instance    - CNC instance number
*                                       None
*/
static void prv_ledInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    GT_U32 baseAddr;

    prv_ledInstanceInit(devNum);
    /* set the addresses of the SMI unit registers according to the needed base address */
    baseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_LED_0_E,NULL);
    regUnitBaseAddrSet(
        REG_UNIT_INFO_MAC(devNum,baseAddr,GOP.LED));
}

/**
* @internal mgSdmaRegsInit function
* @endinternal
*
* @brief   This function initializes the SDMA registers
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                      addrPtr    - pointer to registers struct
*                                       None.
*/
static void mgSdmaRegsInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_U8   i;
    GT_U32  offset;
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *addrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    devNum = devNum;

    addrPtr->MG.sdmaRegs.sdmaCfgReg           = 0x00002800;
    addrPtr->MG.sdmaRegs.rxQCmdReg            = 0x00002680;
    addrPtr->MG.sdmaRegs.txQCmdReg            = 0x00002868;
    addrPtr->MG.sdmaRegs.rxDmaResErrCnt[0]    = 0x00002860;
    addrPtr->MG.sdmaRegs.rxDmaResErrCnt[1]    = 0x00002864;
    addrPtr->MG.sdmaRegs.txQFixedPrioConfig   = 0x00002870;
    addrPtr->MG.sdmaRegs.txSdmaWrrTokenParameters = 0x00002874;

    for (i = 0; i < 8; i++)
    {
        offset = i * 0x10;
        addrPtr->MG.sdmaRegs.rxDmaCdp[i]          = 0x260C + offset;
        addrPtr->MG.sdmaRegs.txQWrrPrioConfig[i]  = 0x2708 + offset;
        addrPtr->MG.sdmaRegs.txSdmaTokenBucketQueueCnt[i]    = 0x2700 + offset;
        addrPtr->MG.sdmaRegs.txSdmaTokenBucketQueueConfig[i] = 0x2704 + offset;

        offset = i * 4;
        addrPtr->MG.sdmaRegs.rxDmaPcktCnt[i]      = 0x2820 + offset;
        addrPtr->MG.sdmaRegs.rxDmaByteCnt[i]      = 0x2840 + offset;
        addrPtr->MG.sdmaRegs.txDmaCdp[i]          = 0x26C0 + offset;
    }

    for (i = 0; i < 8; i++)
    {
        offset = i * 4;
        addrPtr->MG.sdmaRegs.txSdmaPacketGeneratorConfigQueue[i] =
                                                            0x28B0 + offset;
        addrPtr->MG.sdmaRegs.txSdmaPacketCountConfigQueue[i]     =
                                                            0x28D0 + offset;
    }

    for (i = 0; i < 2; i++)
    {
        offset = i * 4;
        addrPtr->MG.sdmaRegs.rxSdmaResourceErrorCountAndMode[i] =
                                                            0x2860 + offset;
    }

    for (i = 2; i < 8; i++)
    {
        offset = i * 4;
        addrPtr->MG.sdmaRegs.rxSdmaResourceErrorCountAndMode[i] =
                                                            0x2870 + offset;
    }
}

/**
* @internal mgInit function
* @endinternal
*
* @brief   init the DB - MG unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None
*/
static void mgInit
(
    IN  GT_SW_DEV_NUM  devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  offset;
    GT_U32    n;

    offset = 0x00030000;

    n = 0;

    regsAddrPtr->MG.XSMI.XSMIManagement =
        offset + 0x00000000 + (n * 0x2000);
    regsAddrPtr->MG.XSMI.XSMIAddress =
        offset + 0x00000008 + (n * 0x2000);
    regsAddrPtr->MG.XSMI.XSMIConfiguration =
        offset + 0x0000000C + (n * 0x2000);

    regsAddrPtr->MG.globalRegs.addrCompletion              = 0x00000000;
    regsAddrPtr->MG.globalRegs.sampledAtResetReg           = 0x00000028;
    regsAddrPtr->MG.globalRegs.metalFix                    = 0x00000054;
    regsAddrPtr->MG.globalRegs.globalControl               = 0x00000058;
    regsAddrPtr->MG.globalRegs.fuQBaseAddr                 = 0x000000C8;
    regsAddrPtr->MG.globalRegs.fuQControl                  = 0x000000CC;
    regsAddrPtr->MG.globalRegs.interruptCoalescing         = 0x000000E0;
    regsAddrPtr->MG.globalRegs.lastReadTimeStampReg        = 0x00000040;
    regsAddrPtr->MG.globalRegs.extendedGlobalControl       = 0x0000005C;
    regsAddrPtr->MG.globalRegs.generalConfigurations       = 0x0000001C;
    regsAddrPtr->MG.globalRegs.genxsRateConfig             = 0x00000060;
    regsAddrPtr->MG.globalRegs.twsiReg.serInitCtrl         = 0x00000010;

    /* DFX interrupts summary registers */
    regsAddrPtr->MG.globalRegs.globalInterrupt.dfxInterruptCause  = 0x000000AC;
    regsAddrPtr->MG.globalRegs.globalInterrupt.dfxInterruptMask   = 0x000000B0;
    regsAddrPtr->MG.globalRegs.globalInterrupt.dfx1InterruptCause = 0x000000B8;
    regsAddrPtr->MG.globalRegs.globalInterrupt.dfx1InterruptMask  = 0x000000BC;

    mgSdmaRegsInit(devNum);
}

/**
* @internal macMibCountersInit function
* @endinternal
*
* @brief   Init MIB MAC counters memory address.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state of logic
*/
static GT_STATUS macMibCountersInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32                          i;          /* iterator */
    GT_U32                          mibBaseAddr, /* mib memory base address */
                                    mibOffset; /* offset per port */

    mibBaseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_MIB_E,NULL);

    for (i = 0; i < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; i++)
    {
        mibOffset = i * 0x400;

        regsAddrPtr->GOP.perPortRegs[i].macCounters = mibBaseAddr + mibOffset;
    }

    return GT_OK;
}

/**
* @internal gop_perPortRegs_Init function
* @endinternal
*
* @brief   Init the GOP.perPortRegs[] registers.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state of logic
*/
static GT_STATUS gop_perPortRegs_Init
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32                          ii,jj;          /* iterator */
    GT_U32                          baseSerdesAddr, baseAddr; /* GOP memory base address */
    GT_U32                          baseAddrToUse, lane;
    GT_U32                          xgOffset; /* XG offset from GOP */
    GT_U32                          gigOffset; /* XG offset from GOP */
    GT_U32                          currentPortOffset, xpcsOffset, xpcsLaneOffset, mpcsOffset;
    GT_U32                          perPortOffset = 0x1000;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    PRV_CPSS_PORT_TYPE_ENT          validPortTypes[] = {PRV_CPSS_PORT_GE_E,
                                                        PRV_CPSS_PORT_XLG_E,
                                                        /*delimiter*/
                                                        PRV_CPSS_PORT_NOT_APPLICABLE_E};
    GT_U32 macCtrl0Offset    , macCtrl1Offset    , macCtrl2Offset    , macCtrl3Offset    ,
           macCtrl4Offset    , macCtrl5Offset    , fcDsaTag2Offset   , fcDsaTag3Offset,
           macIntMaskOffset  ;

    xgOffset = 0x000C0000;
    gigOffset= 0x00000000;
    xpcsOffset = 0x00180400;
    mpcsOffset = 0x00180000;

    baseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_GOP_E,NULL);
    for (ii = 0; ii < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; ii++)
    {
        /* XG addresses */
        regsAddrPtr->GOP.perPortRegs[ii].xgMibCountersCtrl   = baseAddr + 0x30 + xgOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].xlgPortFIFOsThresholdsConfig   =
            baseAddr + 0x10 + xgOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].xlgDicPpmIpgReduce = baseAddr + 0x98 + xgOffset + perPortOffset*ii;

        /* GIGA addresses */
        regsAddrPtr->GOP.perPortRegs[ii].autoNegCtrl      = baseAddr + 0x0c + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].macStatus        = baseAddr + 0x10 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].serdesCnfg       = baseAddr + 0x28 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].serdesCnfg2      = baseAddr + 0x30 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].serdesCnfg3      = baseAddr + 0x34 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].serialParameters = baseAddr + 0x14 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].serialParameters1= baseAddr + 0x94 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].prbsCheckStatus  = baseAddr + 0x38 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].prbsErrorCounter = baseAddr + 0x3c + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].gePortFIFOConfigReg0 = baseAddr + 0x18 + gigOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].gePortFIFOConfigReg1 = baseAddr + 0x1C + gigOffset + perPortOffset*ii;

        /* XPCS registers - only for even ports */
        if (ii % 2)
        {
            continue;
        }

        regsAddrPtr->GOP.perPortRegs[ii].xgGlobalConfReg0 = baseAddr + 0 + xpcsOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].xgGlobalConfReg1 = baseAddr + 4 + xpcsOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].xgXpcsGlobalStatus = baseAddr + 0x10 + xpcsOffset + perPortOffset*ii;
        regsAddrPtr->GOP.perPortRegs[ii].xpcsGlobalMaxIdleCounter = baseAddr + 0xC + xpcsOffset + perPortOffset*ii;

        for(lane = 0; lane < PRV_CPSS_PX_NUM_LANES_CNS; lane++)
        {
            xpcsLaneOffset = lane * 0x44;
            regsAddrPtr->GOP.perPortRegs[ii].laneConfig0[lane]           = baseAddr + xpcsOffset + 0x50 + perPortOffset*ii + xpcsLaneOffset;
            regsAddrPtr->GOP.perPortRegs[ii].laneConfig1[lane]           = baseAddr + xpcsOffset + 0x54 + perPortOffset*ii + xpcsLaneOffset;
            regsAddrPtr->GOP.perPortRegs[ii].disparityErrorCounter[lane] = baseAddr + xpcsOffset + 0x6C + perPortOffset*ii + xpcsLaneOffset;
            regsAddrPtr->GOP.perPortRegs[ii].prbsErrorCounterLane[lane]  = baseAddr + xpcsOffset + 0x70 + perPortOffset*ii + xpcsLaneOffset;
            regsAddrPtr->GOP.perPortRegs[ii].laneStatus[lane]            = baseAddr + xpcsOffset + 0x5c + perPortOffset*ii + xpcsLaneOffset;
            regsAddrPtr->GOP.perPortRegs[ii].cyclicData[lane]            = baseAddr + xpcsOffset + 0x84 + perPortOffset*ii + xpcsLaneOffset;
        }
    }

    baseSerdesAddr = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_SERDES_E,NULL);

    /* SD addresses */
    for (ii = 0; ii < PRV_CPSS_PX_SERDES_NUM_CNS; ii++)
    {
        regsAddrPtr->serdesConfig[ii].serdesExternalReg1   = baseSerdesAddr + 0x0 + perPortOffset*ii;
        regsAddrPtr->serdesConfig[ii].serdesExternalReg2   = baseSerdesAddr + 0x4 + perPortOffset*ii;
    }


    /* Gig addresses */
    for (ii = 0; ii < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; ii++)
    {
        regsAddrPtr->GOP.perPortRegs[ii].mibCountersCtrl     = baseAddr + 0x44 + gigOffset + perPortOffset*ii;
    }

    for(jj = 0; validPortTypes[jj] != PRV_CPSS_PORT_NOT_APPLICABLE_E ; jj++)
    {
        portType = validPortTypes[jj];

        switch(portType)
        {
            case PRV_CPSS_PORT_GE_E:
                baseAddrToUse = baseAddr + gigOffset;

                macCtrl0Offset = 0;
                macCtrl1Offset = 4;
                macCtrl2Offset = 8;
                macCtrl3Offset = 0x48;
                macCtrl4Offset = 0x90;
                macCtrl5Offset = 0;/* no such register*/
                macIntMaskOffset = 0;
                fcDsaTag2Offset = 0x80;
                fcDsaTag3Offset = 0x84;
            break;

            case PRV_CPSS_PORT_XLG_E:
                baseAddrToUse = baseAddr + xgOffset;

                macCtrl0Offset = 0;
                macCtrl1Offset = 4;
                macCtrl2Offset = 8;
                macCtrl3Offset = 0x1C;
                macCtrl4Offset = 0x84;
                macCtrl5Offset = 0x88;
                macIntMaskOffset = 0x18;
                fcDsaTag2Offset = 0x70;
                fcDsaTag3Offset = 0x74;
            break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, "portType[%d] was not implemented",portType);
        }

        for (ii = 0; ii < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; ii++)
        {
            currentPortOffset = baseAddrToUse + perPortOffset*ii;

            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macCtrl =
                currentPortOffset + macCtrl0Offset;

            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macCtrl1 =
                currentPortOffset + macCtrl1Offset;

            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macCtrl2 =
                currentPortOffset + macCtrl2Offset;

            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macCtrl3 =
                currentPortOffset + macCtrl3Offset;

            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macCtrl4 =
                currentPortOffset + macCtrl4Offset;

            if (portType == PRV_CPSS_PORT_XLG_E)
            {
                regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macCtrl5 =
                    currentPortOffset + macCtrl5Offset;
                regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].macIntMask =
                    currentPortOffset + macIntMaskOffset;
                /* MPCS share same port offset as XLG  */
                regsAddrPtr->GOP.perPortRegs[ii].mpcs40GCommonStatus = baseAddr + mpcsOffset + 0x30 +  perPortOffset*ii;
            }


            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].fcDsaTag2 =
                currentPortOffset + fcDsaTag2Offset;

            regsAddrPtr->GOP.perPortRegs[ii].macRegsPerType[portType].fcDsaTag3 =
                currentPortOffset + fcDsaTag3Offset;
        }
    }


    return GT_OK;
}

/**
* @internal mppmInit function
* @endinternal
*
* @brief   Init MPPM and PIZZA register Init
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static GT_STATUS mppmInit
(
    GT_SW_DEV_NUM devNum
)
{
    GT_U32 i;
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    PRV_CPSS_PX_PP_MPPM_PIZZA_ARBITER_STC *pizzaArbiterPtr;
    GT_U32 regNum = 93;

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;
    pizzaArbiterPtr = &regsAddrVer1->MPPM.pizzaArbiter;

    if (regNum > sizeof(pizzaArbiterPtr->pizzaArbiterConfigReg)/sizeof(pizzaArbiterPtr->pizzaArbiterConfigReg[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regsAddrVer1->MPPM.dataIntegrity.lastFailingBuffer     = 0x00000510;
    regsAddrVer1->MPPM.dataIntegrity.lastFailingSegment    = 0x00000514;
    regsAddrVer1->MPPM.dataIntegrity.statusFailedSyndrome  = 0x00000518;

    pizzaArbiterPtr->pizzaArbiterCtrlReg   = 0x0000200;
    pizzaArbiterPtr->pizzaArbiterStatusReg = 0x0000204;
    for (i = 0; i < regNum; i++)
    {
        pizzaArbiterPtr->pizzaArbiterConfigReg[i] = 0x0000208 + i*4;
    }
    return GT_OK;
}

/**
* @internal MCFCInit function
* @endinternal
*
* @brief   Init MCFC register addresses
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static GT_STATUS MCFCInit
(
    IN GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;

    {/*start of unit MCFC */
        {/*start of unit interrupts */
            regsAddrVer1->MCFC.interrupts.MCFCInterruptsCause = 0x0000005c;
            regsAddrVer1->MCFC.interrupts.MCFCInterruptsMask = 0x00000060;

        }/*end of unit interrupts */


        {/*start of unit cntrs */
            regsAddrVer1->MCFC.cntrs.PFCReceivedCntr = 0x00000010;
            regsAddrVer1->MCFC.cntrs.MCReceivedCntr = 0x00000014;
            regsAddrVer1->MCFC.cntrs.UCReceivedCntr = 0x00000018;
            regsAddrVer1->MCFC.cntrs.QCNReceivedCntr = 0x0000001c;
            regsAddrVer1->MCFC.cntrs.MCQCNReceivedCntr = 0x00000020;
            regsAddrVer1->MCFC.cntrs.outgoingQCNPktsCntr = 0x00000040;
            regsAddrVer1->MCFC.cntrs.outgoingMcPktsCntr = 0x00000038;
            regsAddrVer1->MCFC.cntrs.outgoingQCN2PFCMsgsCntr = 0x0000003c;
            regsAddrVer1->MCFC.cntrs.outgoingMCQCNPktsCntr = 0x0000004c;
            regsAddrVer1->MCFC.cntrs.outgoingPFCPktsCntr = 0x00000044;
            regsAddrVer1->MCFC.cntrs.outgoingUcPktsCntr = 0x00000048;
            regsAddrVer1->MCFC.cntrs.ingrDropCntr = 0x00000024;

        }/*end of unit cntrs */


        {/*start of unit config */
            {/*1b000064+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    regsAddrVer1->MCFC.config.QCNMCPortmapForPort[n] =
                        0x00000064+0x4*n;
                }/* end of loop n */
            }/*1b000064+0x4*n*/
            regsAddrVer1->MCFC.config.PFCConfig = 0x00000000;
            regsAddrVer1->MCFC.config.QCNGlobalConfig = 0x00000004;
            regsAddrVer1->MCFC.config.portsQCN2PFCEnableConfig = 0x00000008;
            regsAddrVer1->MCFC.config.portsQCNEnableConfig = 0x0000000c;
            regsAddrVer1->MCFC.config.UCMCArbiterConfig = 0x00000050;
            regsAddrVer1->MCFC.config.ingrTrafficTrigQCNArbiterConfig = 0x00000054;

        }/*end of unit config */


    }/*end of unit MCFC */

    return GT_OK;
}


/**
* @internal PCP_A1_Init function
* @endinternal
*
* @brief   Init PCP A1 revision register addresses
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static GT_STATUS PCP_A1_Init
(
    IN GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;

    {/*start of unit PCP */
        {/*start of unit PRS */
            {/*e000d40+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPacketTypeKeyTableIP2MEMatchIndexEntry[n] =
                        0x00000d40+0x4*n;
                }/* end of loop n */
            }/*e000d40+0x4*n*/
            {/*e000dc0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPacketTypeKeyTableIP2MEMatchIndexEntryMask[n] =
                        0x00000dc0+0x4*n;
                }/* end of loop n */
            }/*e000dc0+0x4*n*/

        }/*end of unit PRS */
        {/*start of unit PORTMAP */
            {
                {/*e004314+0x4*n*/
                    GT_U32    n;
                    for(n = 0 ; n <= 255 ; n++) {
                        regsAddrVer1->PCP.PORTMAP.PCPLagDesignatedPortEntry[n] =
                            0x00004314+0x4*n;
                    }/* end of loop n */
                }/*e004314+0x4*n*/

                regsAddrVer1->PCP.PORTMAP.PCPPortsEnableConfig = 0x00004714;
                regsAddrVer1->PCP.PORTMAP.PCPPacketTypeEnableEgressPortFiltering = 0x00004718;
                regsAddrVer1->PCP.PORTMAP.PCPPacketTypeEnableIngressPortFiltering = 0x0000471c;
            }

        }/*end of unit PORTMAP */

        {/*start of unit HASH */
            regsAddrVer1->PCP.HASH.PCPPacketTypeLagTableNumber = 0x00001814;

        }/*end of unit HASH */

        {/*start of unit general */
            regsAddrVer1->PCP.general.PCPGlobalConfiguration = 0x000050f0;

        }/*end of unit general */

        {/*start of unit IP2ME */
            {/*e006000+0x4*p+0x16*n*/
                GT_U32    p,n;
                for(p = 0 ; p <= 3 ; p++) {
                    for(n = 0 ; n <= 6 ; n++) {
                        regsAddrVer1->PCP.IP2ME.PCPIP2MEIpPart[p][n] =
                            0x00006000+0x4*p+0x10*n;
                    }/* end of loop n */
                }/* end of loop p    */
            }/*e006000+0x4*p+0x16*n*/
            {/*e0060a0+0x4*p*/
                GT_U32    p;
                for(p = 0 ; p <= 6 ; p++) {
                    regsAddrVer1->PCP.IP2ME.PCPIP2MEControlBitsEntry[p] =
                        0x000060a0+0x4*p;
                }/* end of loop p    */
            }/*e0060a0+0x4*p*/
        }/*end of unit IP2ME*/
    }

    return GT_OK;
}

/**
* @internal PCPInit function
* @endinternal
*
* @brief   Init PCP register addresses
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static GT_STATUS PCPInit
(
    IN GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;
    GT_STATUS rc;

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;

    {/*start of unit PCP */
        {/*start of unit PTP */
            {/*e002620+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableUDPAndPort_profileEntry[n] =
                        0x00002620+0x4*n;
                }/* end of loop n */
            }/*e002620+0x4*n*/
            {/*e002000+0x4*m+ 0x8*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 1 ; m++) {
                    for(n = 0 ; n <= 16 ; n++) {
                        regsAddrVer1->PCP.PTP.PCPPortPTPTypeKeyUDBPConfigPort[m][n] =
                            0x00002000+0x4*m+ 0x8*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002000+0x4*m+ 0x8*n*/
            {/*e002088+0x4*(m-2) + 0x18*n*/
                GT_U32    m,n;
                for(m = 2 ; m <= 7 ; m++) {
                    for(n = 0 ; n <= 16 ; n++) {
                        regsAddrVer1->PCP.PTP.PCPPortPTPTypeKeyUDBConfigPort[m-2][n] =
                            0x00002088+0x4*(m-2) + 0x18*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002088+0x4*(m-2) + 0x18*n*/
            {/*e002aa0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableUDPAndPortProfileEntryMask[n] =
                        0x00002aa0+0x4*n;
                }/* end of loop n */
            }/*e002aa0+0x4*n*/
            {/*e0026a0+0x4*m +0x8*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 1 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableUDBPEntryMask[m][n] =
                            0x000026a0+0x4*m +0x8*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e0026a0+0x4*m +0x8*n*/
            {/*e002220+0x4*m + 0x8*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 1 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableUDBPEntry[m][n] =
                            0x00002220+0x4*m + 0x8*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002220+0x4*m + 0x8*n*/
            {/*e0027a0+0x4*(m-2) + 0x18*n*/
                GT_U32    m,n;
                for(m = 2 ; m <= 7 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableUDBEntryMask[m-2][n] =
                            0x000027a0+0x4*(m-2) + 0x18*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e0027a0+0x4*(m-2) + 0x18*n*/
            {/*e002320+0x4*(m-2) +0x18*n*/
                GT_U32    m,n;
                for(m = 2 ; m <= 7 ; m++) {
                    for(n = 0 ; n <= 31 ; n++) {
                        regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableUDBEntry[m-2][n] =
                            0x00002320+0x4*(m-2) +0x18*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e002320+0x4*(m-2) +0x18*n*/
            regsAddrVer1->PCP.PTP.PCPPTPTypeKeyTableEntriesEnable = 0x00002b20;

        }/*end of unit PTP */


        {/*start of unit PRS */
            {/*e000044+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPTPIDTable[n] =
                        0x00000044+0x4*n;
                }/* end of loop n */
            }/*e000044+0x4*n*/
            {/*e0000ac+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPortPktTypeKeyTableExt[n] =
                        0x000000ac+0x4*n;
                }/* end of loop n */
            }/*e0000ac+0x4*n*/
            {/*e000068+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPortPktTypeKeyTable[n] =
                        0x00000068+0x4*n;
                }/* end of loop n */
            }/*e000068+0x4*n*/
            {/*e000000+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPortDaOffsetConfig[n] =
                        0x00000000+0x4*n;
                }/* end of loop n */
            }/*e000000+0x4*n*/
            {/*e000d00+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 3 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyUDBPMissValue[n] =
                        0x00000d00+0x4*n;
                }/* end of loop n */
            }/*e000d00+0x4*n*/
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyTPIDEtherTypeMissValue = 0x00000d10;
            {/*e000680+0x4*m + 0x10*n*/
                GT_U32    n,m;
                for(n = 0 ; n <= 31 ; n++) {
                    for(m = 0 ; m <= 3 ; m++) {
                        regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableUDBPEntryMask[n][m] =
                            0x00000680+0x4*m + 0x10*n;
                    }/* end of loop m */
                }/* end of loop n */
            }/*e000680+0x4*m + 0x10*n*/
            {/*e000200+0x4*m + 0x10*n*/
                GT_U32    n,m;
                for(n = 0 ; n <= 31 ; n++) {
                    for(m = 0 ; m <= 3 ; m++) {
                        regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableUDBPEntry[n][m] =
                            0x00000200+0x4*m + 0x10*n;
                    }/* end of loop m */
                }/* end of loop n */
            }/*e000200+0x4*m + 0x10*n*/
            {/*e000900+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableTPIDEtherTypeEntryMask[n] =
                        0x00000900+0x4*n;
                }/* end of loop n */
            }/*e000900+0x4*n*/
            {/*e000400+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableTPIDEtherTypeEntry[n] =
                        0x00000400+0x4*n;
                }/* end of loop n */
            }/*e000400+0x4*n*/
            {/*e000580+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableSRCPortProfileEntry[n] =
                        0x00000580+0x4*n;
                }/* end of loop n */
            }/*e000580+0x4*n*/
            {/*e000980+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableMACDA4MSBEntryMask[n] =
                        0x00000980+0x4*n;
                }/* end of loop n */
            }/*e000980+0x4*n*/
            {/*e000480+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableMACDA4MSBEntry[n] =
                        0x00000480+0x4*n;
                }/* end of loop n */
            }/*e000480+0x4*n*/
            {/*e000a00+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableMACDA2LSBEntryMask[n] =
                        0x00000a00+0x4*n;
                }/* end of loop n */
            }/*e000a00+0x4*n*/
            {/*e000500+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableMACDA2LSBEntry[n] =
                        0x00000500+0x4*n;
                }/* end of loop n */
            }/*e000500+0x4*n*/
            {/*e000600+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableIsLLCNonSnapEntry[n] =
                        0x00000600+0x4*n;
                }/* end of loop n */
            }/*e000600+0x4*n*/
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableEntriesEnable = 0x00000b80;
            {/*e000a80+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableSRCPortProfileEntryMask[n] =
                        0x00000a80+0x4*n;
                }/* end of loop n */
            }/*e000a80+0x4*n*/
            {/*e000b00+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PRS.PCPPktTypeKeyTableIsLLCNonSnapEntryMask[n] =
                        0x00000b00+0x4*n;
                }/* end of loop n */
            }/*e000b00+0x4*n*/
            regsAddrVer1->PCP.PRS.PCPPktTypeKeySRCPortProfileMissValue = 0x00000d1c;
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyMACDA4MSBMissValue = 0x00000d14;
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyMACDA2LSBMissValue = 0x00000d18;
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyLookUpMissDetected = 0x00000d24;
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyLookUpMissCntr = 0x00000d28;
            regsAddrVer1->PCP.PRS.PCPPktTypeKeyIsLLCNonSnapMissValue = 0x00000d20;
            regsAddrVer1->PCP.PRS.PCPGlobalEtherTypeConfig4 = 0x00000064;
            regsAddrVer1->PCP.PRS.PCPGlobalEtherTypeConfig3 = 0x00000060;
            regsAddrVer1->PCP.PRS.PCPGlobalEtherTypeConfig2 = 0x0000005c;
            regsAddrVer1->PCP.PRS.PCPGlobalEtherTypeConfig1 = 0x00000058;
            regsAddrVer1->PCP.PRS.PCPGlobalEtherTypeConfig0 = 0x00000054;
            regsAddrVer1->PCP.PRS.PCPECNEnableConfig = 0x00000d2c;

        }/*end of unit PRS */


        {/*start of unit PORTMAP */
            {/*e004000+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPPktTypeDestinationFormat0Entry[n] =
                        0x00004000+0x4*n;
                }/* end of loop n */
            }/*e004000+0x4*n*/
            {/*e004080+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPPktTypeDestinationFormat1Entry[n] =
                        0x00004080+0x4*n;
                }/* end of loop n */
            }/*e004080+0x4*n*/
            regsAddrVer1->PCP.PORTMAP.PCPPortsEnableConfig = 0x00004614;
            {/*e004280+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPPktTypeSourceFormat2Entry[n] =
                        0x00004280+0x4*n;
                }/* end of loop n */
            }/*e004280+0x4*n*/
            {/*e004200+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPPktTypeSourceFormat1Entry[n] =
                        0x00004200+0x4*n;
                }/* end of loop n */
            }/*e004200+0x4*n*/
            {/*e004180+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPPktTypeSourceFormat0Entry[n] =
                        0x00004180+0x4*n;
                }/* end of loop n */
            }/*e004180+0x4*n*/
            {/*e004100+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPPktTypeDestinationFormat2Entry[n] =
                        0x00004100+0x4*n;
                }/* end of loop n */
            }/*e004100+0x4*n*/
            regsAddrVer1->PCP.PORTMAP.PCPDstIdxExceptionCntr = 0x00004300;
            regsAddrVer1->PCP.PORTMAP.PCPSrcIdxExceptionCntr = 0x00004304;
            {/*e004314+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrVer1->PCP.PORTMAP.PCPLagDesignatedPortEntry[n] =
                        0x00004314+0x4*n;
                }/* end of loop n */
            }/*e004314+0x4*n*/
            regsAddrVer1->PCP.PORTMAP.PCPSRCExceptionForwardingPortMap = 0x00004310;
            regsAddrVer1->PCP.PORTMAP.PCPDSTExceptionForwardingPortMap = 0x0000430c;

        }/*end of unit PORTMAP */


        {/*start of unit HASH */
            {/*e001050+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    regsAddrVer1->PCP.HASH.PCPPktTypeHashMode2Entry[n] =
                        0x00001050+0x4*n;
                }/* end of loop n */
            }/*e001050+0x4*n*/
            {/*e001804+4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 1 ; n++) {
                    regsAddrVer1->PCP.HASH.PCPLFSRSeed[n] =
                        0x00001804+4*n;
                }/* end of loop n */
            }/*e001804+4*n*/
            {/*e00102c+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    regsAddrVer1->PCP.HASH.PCPPktTypeHashMode1Entry[n] =
                        0x0000102c+0x4*n;
                }/* end of loop n */
            }/*e00102c+0x4*n*/
            {/*e001008+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    regsAddrVer1->PCP.HASH.PCPPktTypeHashMode0Entry[n] =
                        0x00001008+0x4*n;
                }/* end of loop n */
            }/*e001008+0x4*n*/
            {/*e001098+0x4*m + 0x54*n*/
                GT_U32    m,n;
                for(m = 0 ; m <= 20 ; m++) {
                    for(n = 0 ; n <= 8 ; n++) {
                        regsAddrVer1->PCP.HASH.PCPPktTypeHashConfigUDBPEntry[m][n] =
                            0x00001098+0x4*m + 0x54*n;
                    }/* end of loop n */
                }/* end of loop m */
            }/*e001098+0x4*m + 0x54*n*/
            {/*e001074+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 8 ; n++) {
                    regsAddrVer1->PCP.HASH.PCPPktTypeHashConfigIngrPort[n] =
                        0x00001074+0x4*n;
                }/* end of loop n */
            }/*e001074+0x4*n*/
            regsAddrVer1->PCP.HASH.PCPHashPktTypeUDE1EtherType = 0x00001000;
            regsAddrVer1->PCP.HASH.PCPHashPktTypeUDE2EtherType = 0x00001004;
            regsAddrVer1->PCP.HASH.PCPHashCRC32Seed = 0x00001800;
            regsAddrVer1->PCP.HASH.PCPLagTableIndexMode = 0x0000180c;
            regsAddrVer1->PCP.HASH.PCPPrngCtrl = 0x00001810;

        }/*end of unit HASH */


        {/*start of unit general */
            regsAddrVer1->PCP.general.PCPInterruptsCause = 0x00005000;
            regsAddrVer1->PCP.general.PCPInterruptsMask = 0x00005004;
            regsAddrVer1->PCP.general.PCPDebug = 0x00005008;

        }/*end of unit general */


        {/*start of unit COS */
            {/*e003430+0x40*p+0x4*n*/
                GT_U32    p,n;
                for(p = 0 ; p <= 16 ; p++) {
                    for(n = 0 ; n <= 15 ; n++) {
                        regsAddrVer1->PCP.COS.PCPPortL2CoSMapEntry[p][n] =
                            0x00003430+0x40*p+0x4*n;
                    }/* end of loop n */
                }/* end of loop p    */
            }/*e003430+0x40*p+0x4*n*/
            {/*e0033e0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 16 ; n++) {
                    regsAddrVer1->PCP.COS.PCPPortCoSAttributes[n] =
                        0x000033e0+0x4*n;
                }/* end of loop n */
            }/*e0033e0+0x4*n*/
            {/*e0033c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 7 ; n++) {
                    regsAddrVer1->PCP.COS.PCPMPLSCoSMapEntry[n] =
                        0x000033c0+0x4*n;
                }/* end of loop n */
            }/*e0033c0+0x4*n*/
            {/*e0032c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 63 ; n++) {
                    regsAddrVer1->PCP.COS.PCPL3CoSMapEntry[n] =
                        0x000032c0+0x4*n;
                }/* end of loop n */
            }/*e0032c0+0x4*n*/
            {/*e0030c0+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 127 ; n++) {
                    regsAddrVer1->PCP.COS.PCPDsaCoSMapEntry[n] =
                        0x000030c0+0x4*n;
                }/* end of loop n */
            }/*e0030c0+0x4*n*/
            {/*e003000+0x4*n*/
                GT_U32    n;
                for(n = 0 ; n <= 31 ; n++) {
                    regsAddrVer1->PCP.COS.PCPCoSFormatTableEntry[n] =
                        0x00003000+0x4*n;
                }/* end of loop n */
            }/*e003000+0x4*n*/

        }/*end of unit COS */

    }/*end of unit PCP  */

    if (PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        rc = PCP_A1_Init(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal PHAInit function
* @endinternal
*
* @brief   Init PHA register addresses
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*                                       None.
*/
static GT_STATUS PHAInit
(
    IN GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC  *regsAddrVer1;

    regsAddrVer1 = &PRV_CPSS_PX_PP_MAC(devNum)->regsAddrVer1;

    {/*start of unit units */
        {/*start of unit pha_regs */
            regsAddrVer1->PHA.pha_regs.PHACtrl = 0x007fff00;
            regsAddrVer1->PHA.pha_regs.PHAStatus = 0x007fff04;
            regsAddrVer1->PHA.pha_regs.portInvalidCRCMode = 0x007fff10;
            regsAddrVer1->PHA.pha_regs.egrTimestampConfig = 0x007fff14;
            regsAddrVer1->PHA.pha_regs.PHAGeneralConfig = 0x007fff18;
            regsAddrVer1->PHA.pha_regs.PHATablesSERCtrl = 0x007fff30;
            regsAddrVer1->PHA.pha_regs.PHAInterruptSumCause = 0x007fff60;
            regsAddrVer1->PHA.pha_regs.PHASERErrorCause = 0x007fff40;
            regsAddrVer1->PHA.pha_regs.PHASERErrorMask = 0x007fff44;
            regsAddrVer1->PHA.pha_regs.PHAInternalErrorCause = 0x007fff48;
            regsAddrVer1->PHA.pha_regs.PHAInternalErrorMask = 0x007fff4c;
            regsAddrVer1->PHA.pha_regs.headerSERCtrl = 0x007fff34;
            regsAddrVer1->PHA.pha_regs.PHAInterruptSumMask = 0x007fff64;

        }/*end of unit pha_regs */


    }/*end of unit units */

    {/*start of unit PPA */
        {/*start of unit ppa_regs */
            regsAddrVer1->PHA.PPA.ppa_regs.PPACtrl = 0x007eff00;
            regsAddrVer1->PHA.PPA.ppa_regs.PPAInterruptSumCause = 0x007eff10;
            regsAddrVer1->PHA.PPA.ppa_regs.PPAInterruptSumMask = 0x007eff14;
            regsAddrVer1->PHA.PPA.ppa_regs.PPAInternalErrorCause = 0x007eff20;
            regsAddrVer1->PHA.PPA.ppa_regs.PPAInternalErrorMask = 0x007eff24;

        }/*end of unit ppa_regs */


    }/*end of unit PPA */

    {/*start of unit PPG */
        {/*start of unit ppg_regs */
                regsAddrVer1->PHA.PPG[0].ppg_regs.PPGInterruptSumMask  = 0x0007FF04;
                regsAddrVer1->PHA.PPG[0].ppg_regs.PPGInterruptSumCause = 0x0007FF00;
                regsAddrVer1->PHA.PPG[0].ppg_regs.PPGInternalErrorMask = 0x0007FF14;
                regsAddrVer1->PHA.PPG[0].ppg_regs.PPGInternalErrorCause = 0x0007FF10;

        }/*end of unit ppg_regs */

        /* manually added */
        regsAddrVer1->PHA.PPG[0].PPG_IMEM_base_addr = 0x00040000;/* base address of this memory (4 words per entry) */


        {/*ffffffff+m*0x80000*/
            GT_U32    m;
            for(m = 0 ; m <= 3 ; m++) {
                GT_U32        unitArrayIndex;
                GT_U32        *unitArray_0_ptr,*currU32Ptr;
                currU32Ptr = (void*)&regsAddrVer1->PHA.PPG[m] ;
                unitArray_0_ptr = (void*)&(regsAddrVer1->PHA.PPG[0]);
                /* loop on all elements in the unit , and set/update the address of register/table */
                for(unitArrayIndex = 0 ;
                    unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(regsAddrVer1->PHA.PPG[0],GT_U32) ;
                    unitArrayIndex++ , currU32Ptr++){
                    (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0xffffffff+m*0x80000;
                } /*loop on unitArrayIndex */
            }/* end of loop m */
        }/*ffffffff+m*0x80000*/
    }/*end of unit PPG[4] */

    {/*start of unit PPN */
        {/*start of unit ppn_regs */
            regsAddrVer1->PHA.PPN[0][0].ppn_regs.PPNDoorbell = 0x00003010;
            regsAddrVer1->PHA.PPN[0][0].ppn_regs.pktHeaderAccessOffset = 0x00003024;
            regsAddrVer1->PHA.PPN[0][0].ppn_regs.PPNInternalErrorCause = 0x00003040;
            regsAddrVer1->PHA.PPN[0][0].ppn_regs.PPNInternalErrorMask = 0x00003044;

        }/*end of unit ppn_regs */


        {/*0+0x80000 * m + 0x4000 * p*/
            GT_U32    p,m;
            for(p = 0 ; p <= 7 ; p++) {
                for(m = 0 ; m <= 3 ; m++) {
                    GT_U32        unitArrayIndex;
                    GT_U32        *unitArray_0_ptr,*currU32Ptr;
                    currU32Ptr = (void*)&regsAddrVer1->PHA.PPN[p][m] ;
                    unitArray_0_ptr = (void*)&(regsAddrVer1->PHA.PPN[0][0]);
                    /* loop on all elements in the unit , and set/update the address of register/table */
                    for(unitArrayIndex = 0 ;
                        unitArrayIndex < NUM_ELEMENTS_IN_UNIT_MAC(regsAddrVer1->PHA.PPN[0][0],GT_U32) ;
                        unitArrayIndex++ , currU32Ptr++){
                        (*currU32Ptr) = unitArray_0_ptr[unitArrayIndex] + 0* 0x0+0x80000 * m + 0x4000 * p;
                    } /*loop on unitArrayIndex */
                }/* end of loop m */
            }/* end of loop p */
        }/*0+0x80000 * m + 0x4000 * p*/
    }/*end of unit PPN[8][4] */

    return GT_OK;
}



/**
* @internal prvCpssPxHwRegAddrToUnitIdConvert function
* @endinternal
*
* @brief   This function convert register address to unit id.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] regAddr                  - the register address to get it's base address unit Id.
*                                       the unitId for the given address
*/
PRV_CPSS_DXCH_UNIT_ENT prvCpssPxHwRegAddrToUnitIdConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_U32                   regAddr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  *unitIdPtr = NULL;

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
       NULL == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum])
    {
        /* error */
        return PRV_CPSS_DXCH_UNIT_LAST_E;
    }

    switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily)
    {
        case CPSS_PX_FAMILY_PIPE_E:
            unitIdPtr = prvPxUnitIdPer8MSBitsBaseAddrArr;
            break;
        default:
            /* error */
            return PRV_CPSS_DXCH_UNIT_LAST_E;
    }

    return unitIdPtr[(regAddr >> 24)];
}

/**
* @internal prvCpssPxHwIsUnitUsed function
* @endinternal
*
* @brief   This function checks existences of base addresses units in the Emulator/VERIFIER
*         of the GM for the given device.
*         since the Emulator/VERIFIER not supports all units , the CPSS wants to avoid
*         accessing the Emulator/VERIFIER.
*         the CPSS binds the Emulator/simulation with this functions so the simulation before
*         accessing the GM (and Emulator/VERIFIER) can know no to call them on such addresses.
*         NOTE: purpose is to reduce Emulator/VIRIFIER 'ERRORS' in LOG , and even 'crash' of
*         Emulator/VERIFIER on some unknown addresses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - thr PP's device number to init the struct for.
* @param[in] portGroupId              - port group ID
* @param[in] regAddr                  - register address
*
* @retval GT_TRUE                  - the unit is implemented in the Emulator/VERIFIER
* @retval GT_FALSE                 - the unit is NOT implemented in the Emulator/VERIFIER
*/
GT_BOOL prvCpssPxHwIsUnitUsed
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr
)
{
    PRV_CPSS_DXCH_UNIT_ENT unitId;  /* Unit ID */

    if(!PRV_CPSS_PP_MAC(devNum))
    {
        /* the device not defined yet at the CPSS
           (function called from the driver early stages !) */

        /* since we can't check the address ... assume it exists */
        return GT_TRUE;
    }

    portGroupId = portGroupId;

    /* code for emulator on PIPE
       (called only when cpssDeviceRunCheck_onEmulator())*/
    if((regAddr & 0xFF000000) == 0x06000000)
    {
        /*SERDES And SBC Controller*/
        /* NOT supports the unit */
        return GT_FALSE;
    }

    unitId = prvCpssPxHwRegAddrToUnitIdConvert(devNum, regAddr);

    switch(unitId)
    {   /* List of NON existing units */
        case PRV_CPSS_DXCH_UNIT_SERDES_E:
            /* allow limited access to : GOP/<SD_WRAPPER> SD_IP Units<<%s>>*/
            /* 0x13000000 + 0x1000*s: where s (0-32) represents serdes */
            switch(regAddr & 0x00000FFF) /*steps of 0x1000*/
            {
                case 0x000:/*SERDES External Configuration 0*/
                case 0x004:/*SERDES External Configuration 1*/
                    return GT_TRUE;
                default:
                    break;
            }
            return GT_FALSE;
        default:
            /* supports the unit */
            return GT_TRUE;
    }

}

/**
* @internal prvCpssPxHwRegAddrVer1Remove function
* @endinternal
*
* @brief   This function free allocated memories relate to the registers struct of the devices.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwRegAddrVer1Remove
(
    IN  GT_SW_DEV_NUM devNum
)
{
    /* no dynamic allocations */
    devNum = devNum;

    return GT_OK;
}

/**
* @internal prvCpssPxHwRegAddrVer1Init function
* @endinternal
*
* @brief   This function initializes the registers struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwRegAddrVer1Init
(
    IN  GT_SW_DEV_NUM devNum
)
{
    GT_U32 * unitBasePtr;
    PRV_CPSS_DXCH_UNIT_ENT * unitIdPtr;
    GT_U32  ii;
    GT_U32  subunitBase;
    GT_U32  indexArr[2];
    GT_U32  unit8MSBits;
    GT_STATUS rc;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* attach the generic code with the PX function */
    DUPLICATED_ADDR_GETFUNC(devNum) =
        pipeDuplicatedMultiPortGroupsGet_byDevNum;


    rc = prvCpssPxUnitBaseTableGet(devNum,/*OUT*/&unitBasePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* unitBasePtr = prvPxBobcat3UnitsBaseAddr;  */
    unitIdPtr = prvPxUnitIdPer8MSBitsBaseAddrArr;


    for(ii = 0 ; ii < NUM_ENTRIES_FOR_8_MSBITS_BASE_ADDR_CNS ; ii++)
    {
        /* init the table with 'not valid' */
        unitIdPtr[ii] = PRV_CPSS_DXCH_UNIT_LAST_E;
    }

    /* array of unitBasePtr[] is with different size per family/sub family
       iterations need to be until it's end and not according to 'constant'  */
    for(ii = 0 ; unitBasePtr[ii] != END_OF_TABLE_INDICATOR_CNS ; ii++)
    {
        /* according to unitId we get the 8 MSBits of the address*/
        unit8MSBits = unitBasePtr[ii] >> 24;
        if(unitIdPtr[unit8MSBits] != PRV_CPSS_DXCH_UNIT_LAST_E)
        {
            /* support BC3 units that the '2 pipes' are in the same '8 MSbits' */
            continue;
        }
        /* with those 8 MSBits we access the unitId table and save the unitId (ii)*/
        unitIdPtr[unit8MSBits] = ii;
    }


#ifdef ASIC_SIMULATION /* needed for GM only*/
    {
        GT_U32      simDeviceId;
        /* Convert devNum to simDeviceId */
        rc = prvCpssDrvDevNumToSimDevIdConvert(devNum, 0, &simDeviceId);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*
            bind the simulation with CallBack function to advice about addresses
            that belongs to 'non exists' units in the VERIFIER.

            the simulation will 'ignore' write to such register , and will
            'read' value of '0x00badadd'.
        */

        smemGmUnitUsedFuncInit(simDeviceId, devNum, prvCpssPxHwIsUnitUsed);
    }
#endif /*ASIC_SIMULATION*/

    /* reset the DB */
    dbReset(devNum);

    /* init for RxDma,txDma,txFifo */
    dataPathInit(devNum,unitBasePtr);

    /* init BM */
    bmInit(devNum);
    /* set the addresses of the BM unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_BM_E], BM));

    /* init BMA */
    bmaInit(devNum);
    /* set the addresses of the BMA unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_BMA_E], BMA));

    /* init mppm */
    mppmInit(devNum);
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MPPM_E], MPPM));


    /* init TAI (Time Application Interface) */
    /* single TAI device */
    gopTaiInit(devNum);
    subunitBase = unitBasePtr[PRV_CPSS_DXCH_UNIT_TAI_E];
    regUnitBaseAddrSet(
        REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.TAI));

    /* init GOP CG */
    {
        /* Setting index as the port number */
        ii = 12;/* only port 12 is CG */
        indexArr[0] = ii;

        /* Get CG subunit base address */
        subunitBase = prvCpssPxHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_CG_E, indexArr);

        /* Init CG subunit registers addresses */
        gopCgInit(devNum, ii);

        /* Add base address to the subunit registers */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[ii].CG_CONVERTERS));
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[ii].CG_PORT.CG_PORT_MAC));
    }

    /* init GOP PTP  */
    for (ii = 0; (ii < PRV_CPSS_PX_GOP_PORTS_NUM_CNS); ii++)
    {
        indexArr[0] = ii;
        subunitBase = prvCpssPxHwSubunitBaseAddrGet(
            devNum, PRV_CPSS_SUBUNIT_GOP_PTP_E, indexArr);

        gopPtpInit(devNum, ii /*macNum*/);

        /* set the addresses of the PTP unit registers according to the needed base address */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.PTP[ii]));

        gopFcaInit(devNum, ii /*macNum*/);
        subunitBase = prvCpssPxHwSubunitBaseAddrGet(
            devNum, PRV_CPSS_SUBUNIT_GOP_FCA_E, indexArr);

        /* set the addresses of the FCA unit registers according to the needed base address */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.FCA[ii]));
    }

    /* init the CNC unit (single instance) */
    cncInit(devNum);

    /* set the addresses of the CNC unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_CNC_0_E], CNC));

    /* TXQ unit init */
    {
        /* init the TXQ_QCN unit*/
        txqQcnInit(devNum);
        /* set the addresses of the TXQ_QCN unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_QCN_E], TXQ.qcn));

        /* init the TXQ_BMX unit*/
        txqBmxInit(devNum);
        /* set the addresses of the TXQ_QCN unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_BMX_E], TXQ.bmx));

        /* init the TXQ_LL unit*/
        txqLLInit(devNum);
        /* set the addresses of the TXQ_LL unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_LL_E], TXQ.ll));

        /* init the TXQ_PFC unit*/
        txqPfcInit(devNum);
        /* set the addresses of the TXQ_PFC unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_PFC_E], TXQ.pfc));

        /* init the TXQ_QUEUE unit*/
        txqQueueInit(devNum);
        /* set the addresses of the TXQ_QUEUE unit registers according to the needed base address */
        regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E], TXQ.queue));
    }

    /* the LMS unit was split into 2 separate units SMI , LED */
    prv_smiInit(devNum);
    prv_ledInit(devNum);

    /* init PG registers */
    gopMacPGInit(devNum);
    /* set the addresses of the PG unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_GOP_E], packGenConfig));

    /* init MG unit */
    mgInit(devNum);
    /* set the addresses of the MG unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_DXCH_UNIT_MG_E], MG));

    rc = macMibCountersInit(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = gop_perPortRegs_Init(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    PHAInit(devNum);
    /* set the addresses of the PHA unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_PX_UNIT_PHA_E], PHA));

    MCFCInit(devNum);
    /* set the addresses of the PHA unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_PX_UNIT_MCFC_E], MCFC));

    PCPInit(devNum);
    /* set the addresses of the PHA unit registers according to the needed base address */
    regUnitBaseAddrSet(REG_UNIT_INFO_MAC(devNum, unitBasePtr[PRV_CPSS_PX_UNIT_PCP_E], PCP));


    return GT_OK;
}

/**
* @internal prvCpssPxHwResetAndInitControllerRegAddrInit function
* @endinternal
*
* @brief   This function initializes the Reset and Init Controller registers
*         struct for eArch devices.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_CPU_MEM        - on malloc failed
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwResetAndInitControllerRegAddrInit
(
    IN  GT_SW_DEV_NUM devNum
)
{
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *regsAddrPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    resetAndInitControllerDbReset(devNum);

    regsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    {/*start of unit DFXServerUnitsDeviceSpecificRegs */
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl = 0x000f800c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.initializationStatusDone = 0x000f8014;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix = 0x000f8020;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix = 0x000f8030;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix = 0x000f8060;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix = 0x000f8064;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix = 0x000f8068;

        /* CPU Control Skip Initialization Matrix */
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix = 0x000f8040;
        /* DFX Registers Configuration Skip Initialization Matrix */
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXSkipInitializationMatrix = 0x000f8098;

        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR1 = 0x000f8200;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceSAR2 = 0x000f8204;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl3 = 0x000f825c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl0 = 0x000f8250;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl4 = 0x000f8260;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl5 = 0x000f8264;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl8 = 0x000f8270;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl10 = 0x000f8278;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl12 = 0x000f8280;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl21 = 0x000f82a4;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl20 = 0x000f82a0;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl19 = 0x000f829c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl18 = 0x000f8298;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl15 = 0x000f828c;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl14 = 0x000f8288;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceStatus0 = 0x000f8c80;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceStatus1 = 0x000f8c84;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl32 =  0x000f8D20;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceCtrl16 = 0x000f8290;

        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.pllPTPConfig = 0x000F82FC;
        regsAddrPtr->DFXServerUnitsDeviceSpecificRegs.pllPTPParams = 0x000F82F8;

    }/*end of unit DFXServerUnitsDeviceSpecificRegs */


    {/*start of unit DFXServerUnits */
        {/*start of unit DFXServerRegs */
            {/*f812c+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSDisabledCtrl2[a] =
                        0xf812c+28*a;
                }/* end of loop a */
            }/*f812c+28*a*/
            {/*f8128+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSDisabledCtrl1[a] =
                        0xf8128+28*a;
                }/* end of loop a */
            }/*f8128+28*a*/
            regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlLSB = 0x000f8070;
            {/*f8450+t*4*/
                GT_U32    t;
                for(t = 1 ; t <= 4 ; t++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.serverXBARTargetPortConfig[t-1] =
                        0xf8450+t*4;
                }/* end of loop t */
            }/*f8450+t*4*/
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverStatus = 0x000f8010;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptSummaryCause = 0x000f8100;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptSummaryMask = 0x000f8104;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptMask = 0x000f810c;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverInterruptCause = 0x000f8108;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.serverAddrSpace = 0x000f8018;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.pipeSelect = 0x000f8000;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensor28nmCtrlMSB = 0x000f8074;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.temperatureSensorStatus = 0x000f8078;
            {/*f8134+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSStatus[a] =
                        0xf8134+28*a;
                }/* end of loop a */
            }/*f8134+28*a*/
            {/*f8138+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSMinStatus[a] =
                        0xf8138+28*a;
                }/* end of loop a */
            }/*f8138+28*a*/
            {/*f813c+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSMaxStatus[a] =
                        0xf813c+28*a;
                }/* end of loop a */
            }/*f813c+28*a*/
            {/*f8130+28*a*/
                GT_U32    a;
                for(a = 0 ; a <= 0 ; a++) {
                    regsAddrPtr->DFXServerUnits.DFXServerRegs.AVSEnabledCtrl[a] =
                        0xf8130+28*a;
                }/* end of loop a */
            }/*f8130+28*a*/
            regsAddrPtr->DFXServerUnits.DFXServerRegs.snoopBusStatus = 0x000f8220;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.deviceIDStatus = 0x000f8240;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.JTAGDeviceIDStatus = 0x000f8244;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.PLLCoreConfig = 0x000f82e4;
            regsAddrPtr->DFXServerUnits.DFXServerRegs.PLLCoreParameters = 0x000f82e0;

        }/*end of unit DFXServerRegs */

    }/*end of unit DFXServerUnits */

    {/*start of unit DFXClientUnits*/
        /* skip DFXClientUnits during registers dump printing */
        regsAddrPtr->dfxClientBookmark     = PRV_CPSS_SW_PTR_BOOKMARK_CNS;
        regsAddrPtr->dfxClientBookmarkType = PRV_CPSS_SW_TYPE_WRONLY_CNS;
        regsAddrPtr->dfxClientBookmarkSize = sizeof(regsAddrPtr->DFXClientUnits);

        regsAddrPtr->DFXClientUnits.clientControl     = 0x00002000;
        regsAddrPtr->DFXClientUnits.clientStatus      = 0x00002010;
        regsAddrPtr->DFXClientUnits.clientDataControl = 0x00002008;
        {
            GT_U32    a;
            for(a = 0 ; a <= 3 ; a++) {
                regsAddrPtr->DFXClientUnits.clientRamBISTInfo[a] = 0x2040+4*a;
            }/* end of loop a */
        }
        regsAddrPtr->DFXClientUnits.BISTControl       = 0x00002070;
        regsAddrPtr->DFXClientUnits.BISTOpCode        = 0x00002074;
        regsAddrPtr->DFXClientUnits.BISTMaxAddress    = 0x00002080;
        regsAddrPtr->DFXClientUnits.dummyWrite        = 0x000020b0;
        {
            GT_U32    a;
            for(a = 0 ; a <= 3 ; a++) {
                regsAddrPtr->DFXClientUnits.clientRamMC_Group[a] = 0x20E0+4*a;
            }/* end of loop a */
        }
    }/*end of unit DFXClientUnits*/

    {/*start of unit DFXRam*/
        /* skip DFXRam during registers dump printing */
        regsAddrPtr->dfxRamBookmark     = PRV_CPSS_SW_PTR_BOOKMARK_CNS;
        regsAddrPtr->dfxRamBookmarkType = PRV_CPSS_SW_TYPE_WRONLY_CNS;
        regsAddrPtr->dfxRamBookmarkSize = sizeof(regsAddrPtr->DFXRam);

        regsAddrPtr->DFXRam.memoryControl    = 0x14;

    }/*end of unit DFXRam*/

    return GT_OK;
}

/* calculate the offset of registers array position in PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC from
   start of the structure */
/* NOTE: in VC10 when the macro hold " - (GT_U32*)NULL" --> error C2099: initializer is not a constant */
#define REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(regArrName)   \
    (((PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC*)NULL)->regArrName)

/* structure to hold the info about offset of registers array in the reg DB
   'PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC' */
typedef struct{
    GT_U32*  registerOffsetPtr;  /* pointer offset from start of PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC of register */
    GT_U32  numOfRegisters;  /* number of registers from the 'registerOffset' */
}OFFSET_IN_REG_DB1_INFO_STC;

/* use DB of non manipulated unit ... because unit 0 hold addresses of all units */
#define NON_FIRST_UNIT_INDEX_CNS    1
#define END_OF_TABLE_CNS   (GT_U32*)(GT_UINTPTR)0xFFFFFFFF
#define UNIT_OFFSET_MASK_CNS    0x00FFFFFF

/* structure to hold the info about offset of registers array in the reg DB
   'PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC' */
typedef struct{
    CPSS_PX_TABLE_ENT  tableType;  /* pointer offset from start of PRV_CPSS_PX_PP_MAC(_devNum)->accessTableInfoPtr []*/
    GT_U32  numOfEntries;/* number of entries from start of table
                            value 0 means use the info from tablesInfo : PRV_CPSS_PX_TABLES_INFO_STC*/
}OFFSET_IN_TABLE_INFO_STC;

/*check if register address is in the array of registers */
static GT_BOOL  isAddrInRegDb1Array(
    IN GT_SW_DEV_NUM    devNum,
    IN OFFSET_IN_REG_DB1_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    GT_U32  ii,jj;
    GT_U32  *dbRegAddrPtr;
    GT_U32  *base_dbRegAddrPtr = (GT_PTR)PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  numOfRegisters;

    regAddr &= UNIT_OFFSET_MASK_CNS;/* we compare only offset in the unit */

    for(ii = 0 ; infoArr[ii].registerOffsetPtr != END_OF_TABLE_CNS ; ii ++)
    {
        dbRegAddrPtr = base_dbRegAddrPtr + (infoArr[ii].registerOffsetPtr - (GT_U32*)NULL);
        numOfRegisters = infoArr[ii].numOfRegisters;

        if(numOfRegisters == 0)
        {
            /* place holder */
            continue;
        }

        /* assuming that the dbRegAddrPtr is in ascending order */
        if((dbRegAddrPtr[0] & UNIT_OFFSET_MASK_CNS               ) > regAddr ||
           (dbRegAddrPtr[numOfRegisters-1] & UNIT_OFFSET_MASK_CNS) < regAddr)
        {
            /* not in range of this group */
            continue;
        }

        for(jj = 0 ; jj < numOfRegisters; jj++)
        {
            if((dbRegAddrPtr[jj] & UNIT_OFFSET_MASK_CNS) == regAddr)
            {
                /* found */
                return GT_TRUE;
            }
        }

        /* do not break .... to support grouts that are not in steps of '4'
           between register addresses

           allow to check in other groups
        */
    }

    return GT_FALSE;
}

/*check if register address is in the array of registers */
static GT_BOOL  isAddrInTableArray(
    IN GT_SW_DEV_NUM    devNum,
    IN OFFSET_IN_TABLE_INFO_STC   *infoArr,
    IN GT_U32       regAddr
)
{
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *tablePtr;/* pointer to direct table info */
    GT_U32  ii,offset,numOfEntries,baseAddress;

    regAddr &= UNIT_OFFSET_MASK_CNS;/* we compare only offset in the unit */

    for(ii = 0 ; infoArr[ii].tableType != CPSS_PX_TABLE_LAST_E ; ii ++)
    {
        tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,infoArr[ii].tableType);

        tablePtr = &tableInfoPtr->directAccessInfo;

        baseAddress = tablePtr->baseAddress & UNIT_OFFSET_MASK_CNS;

        numOfEntries = infoArr[ii].numOfEntries ?
                       infoArr[ii].numOfEntries :
                       tablePtr->maxNumOfEntries;

        if( baseAddress                                                      > regAddr ||
           (baseAddress + (tablePtr->alignmentWidthInBytes * numOfEntries)) <= regAddr)
        {
            /* not in range of this table */
            continue;
        }

        offset = regAddr - (tablePtr->baseAddress & UNIT_OFFSET_MASK_CNS);

        if((offset % tablePtr->alignmentWidthInBytes) < (tablePtr->entryWidthInWords * 4))
        {
            /* we have a match. The address is with in this table range */
            return GT_TRUE;
        }

        /* do not break .... to support grouts that are not in steps of '4'
           between table entries

           allow to check in other tables
        */
    }

    return GT_FALSE;
}

/*check if register address is per port in Txq DQ unit */
static GT_BOOL  pipe_mustNotDuplicate_txqDqAddr(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           regAddr
)
{
    static OFFSET_IN_REG_DB1_INFO_STC perPortAddrArr[]=
    {
         {REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portArbiterConfig.portsArbiterPortWorkConservingEnable ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.portShaper.portRequestMaskSelector                     ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.priorityArbiterWeights.portSchedulerProfile            ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].scheduler.schedulerConfig.conditionalMaskForPort                 ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portDequeueEnable                                         ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].shaper.portTokenBucketMode                                       ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portToDMAMapTable                          ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.globalDQConfig.portBCConstantValue                        ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}
        ,{REG_ARR_OFFSET_FROM_REG_DB_PX_1_STC_MAC(TXQ.dq[NON_FIRST_UNIT_INDEX_CNS].global.flushTrig.portTxQFlushTrigger                             ),  PRV_CPSS_PX_MAX_DQ_NUM_PORTS_PER_DQ_CNS}

         /* must be last */
        ,{END_OF_TABLE_CNS,0}
    };

    static OFFSET_IN_TABLE_INFO_STC perPortTableArr[] =
    {         /* table type */                                                       /* number of entries */
        {CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,         0/*take from tabled DB*/},
        {CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,0/*take from tabled DB*/},

/*      removed from the list since DQ0 and DQ1 should have the identical data
        {CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E                   ,0},*/

        /* must be last */
        {CPSS_PX_TABLE_LAST_E , 0}
    };

    return isAddrInRegDb1Array(devNum,perPortAddrArr,regAddr) ||
           isAddrInTableArray(devNum,perPortTableArr,regAddr) ;
}
/**
* @internal pipeDuplicatedMultiPortGroupsGet_byDevNum function
* @endinternal
*
* @brief   Check if register address is duplicated in PIPE device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id. relevant only to 'multi-port-groups'
*                                      devices. Supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] regAddr                  - register address to access.
*
* @param[out] portGroupsBmpPtr         - (pointer to) the updated port groups
*                                      relevant only when function returns GT_TRUE
* @param[out] isAdditionalRegDupNeededPtr - (pointer to)
*                                      GT_TRUE - the caller need to duplicate the value to additional
*                                      register in all 'port groups bmp'.
*                                      GT_FALSE - the caller NOT need to duplicate the value to additional
*                                      register.
* @param[out] additionalRegDupPtr      - (pointer to)
*                                      register address of additional register duplication.
*                                      Only relevant when isAdditionalRegDupNeeded is GT_TRUE.
* @param[out] maskDrvPortGroupsPtr     = (pointer to)
*                                      GT_TRUE  - mask the cpssDrv active ports only
*                                      GT_FALSE - mask the CPSS    active ports only
*
* @retval GT_TRUE                  - the caller need to use the updated 'port groups bmp'
* @retval GT_FALSE                 - the caller NOT need to use the updated 'port groups bmp'
*/
static GT_BOOL pipeDuplicatedMultiPortGroupsGet_byDevNum
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   portGroupId,
    IN GT_U32                   regAddr,
    OUT  GT_PORT_GROUPS_BMP     *portGroupsBmpPtr,
    OUT  GT_BOOL                *isAdditionalRegDupNeededPtr,
    OUT  PRV_CPSS_DUPLICATED_ADDITIONAL_ADDRESSES_INFO_STC *additionalRegDupPtr,
    OUT  GT_BOOL                *maskDrvPortGroupsPtr
)
{
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_U32  *additionalBaseAddrPtr;
    GT_U32  dupIndex;
    GT_U32  ii;

    portGroupId = portGroupId;/* unused */

    *maskDrvPortGroupsPtr = GT_FALSE;/* no port groups in our device (not in CPSS and not in cpssDriver) */

    *portGroupsBmpPtr = BIT_0;/* initialization that indicated that unit is single instance
        needed by prv_cpss_multi_port_groups_bmp_check_specific_unit_func(...) */

    /* initial needed variables */
    dupIndex = 0;
    additionalBaseAddrPtr = &additionalRegDupPtr->additionalAddressesArr[0];

    unitId = prvCpssPxHwRegAddrToUnitIdConvert(devNum, regAddr);
    switch(unitId)
    {
        case PRV_CPSS_DXCH_UNIT_TXQ_DQ_E:
            if(GT_TRUE == pipe_mustNotDuplicate_txqDqAddr(devNum, regAddr))
            {
                /* the address is forbidden for duplication */
                return GT_FALSE;
            }

            additionalBaseAddrPtr[dupIndex++] = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E,NULL);
            break;
        default:
            return GT_FALSE;
    }

    additionalRegDupPtr->numOfAdditionalAddresses = dupIndex;

    *isAdditionalRegDupNeededPtr = GT_TRUE;

    for(ii = 0; ii < dupIndex ; ii++)
    {
        /* base address is 8 MSBits added with 24 LSBits of original address */
        additionalBaseAddrPtr[ii] |= (regAddr & 0x00FFFFFF);
    }

    return GT_TRUE;

}


/**
* @internal prvCpssPxCgPortDbInvalidate function
* @endinternal
*
* @brief   Invalidate or init CG port register database
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] portNum                  - CG MAC port number
* @param[in] cgUnitInvalidate         - invalidate status
*                                      GT_TRUE - invalidate DB for given MAC port
*                                      GT_FALSE - init DB for given MAC port
*                                       None
*/
GT_STATUS prvCpssPxCgPortDbInvalidate
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32 portNum,
    IN  GT_BOOL cgUnitInvalidate
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
    GT_U32  subunitBase;
    GT_U32  indexArr[2];

    if(cgUnitInvalidate == GT_TRUE)
    {
        DESTROY_STC_MAC(regsAddrPtr->GOP.CG[portNum]);
    }
    else
    {
        /* Setting index as the port number */
        indexArr[0] = portNum;

        /* Get CG subunit base address */
        subunitBase = prvCpssPxHwSubunitBaseAddrGet(
                devNum, PRV_CPSS_SUBUNIT_GOP_CG_E, indexArr);

        gopCgInit(devNum, portNum);

        /* Add base address to the subunit registers */
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[portNum].CG_CONVERTERS));
        regUnitBaseAddrSet(
            REG_UNIT_INFO_MAC(devNum, subunitBase, GOP.CG[portNum].CG_PORT.CG_PORT_MAC));

    }

    return GT_OK;
}


/**
* @internal prvCpssPxHwRegAddrPortMacUpdate function
* @endinternal
*
* @brief   This function updates mac registers addresses for given port accordingly
*         to currently used MAC Unit
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwRegAddrPortMacUpdate
(
    IN  GT_U32                  devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddrPtr;
    GT_U32    perPortOffset = 0x1000;
    GT_U32    currentPortOffset;
    GT_U32    baseAddr,xgOffset,gigOffset;
    GT_U32    portMacNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    regsAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    xgOffset = 0x000C0000;
    gigOffset= 0x00000000;

    baseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_GOP_E,NULL);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_XLG_E)
    {
        currentPortOffset = baseAddr + xgOffset + perPortOffset * portMacNum;
        regsAddrPtr->GOP.perPortRegs[portMacNum].macStatus = currentPortOffset + 0xC;
    }
    else if (PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_GE_E)
    {
        currentPortOffset = baseAddr + gigOffset + perPortOffset * portMacNum;
        regsAddrPtr->GOP.perPortRegs[portMacNum].macStatus = currentPortOffset + 0x10;
    }

    return GT_OK;
}

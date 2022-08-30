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
* @file snetCheetahDoInterrupts.c
*
* @brief simulation handing interrupts:
* 1. generation during traffic / triggered operation from CPU (FDB scan)
* 2. active memory for read/write cause/mask registers.
* 3. other generic interrupts utilities
*
* @version   1
********************************************************************************
*/
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

/*******************************************************************************
*   getInterruptRegAddr
*
* DESCRIPTION:
*       get the cause/mask register address from DB of interrupts.
*
* INPUTS:
*       devObjPtr           - pointer to device object.
*       currDbPtr           - the entry in the DB
*       getCause            - GT_TRUE  - get cause register address
*                             GT_FALSE - get mask register address
*
*   RETURNS:
*       regAddr
*
*******************************************************************************/
GT_U32 getInterruptRegAddr
(
    IN SKERNEL_DEVICE_OBJECT *         devObjPtr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC *currDbPtr,
    IN GT_BOOL                         getCause
)
{
    char    *dbBasePtr;
    GT_U32  regAddr;
    SKERNEL_INTERRUPT_BASIC_REG_INFO_STC  *regInfoPtr;

    if(currDbPtr == NULL)
    {
        return SMAIN_NOT_VALID_CNS;
    }

    switch(currDbPtr->causeReg.registersDbType)
    {
        case SKERNEL_REGISTERS_DB_TYPE__LAST___E:
            dbBasePtr = NULL;
            break;
        case SKERNEL_REGISTERS_DB_TYPE_SIP5_E:
            dbBasePtr = (char*)SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr);
            break;
        case SKERNEL_REGISTERS_DB_TYPE_LEGACY_E:
            dbBasePtr = (char*)SMEM_CHT_MAC_REG_DB_GET(devObjPtr);
            break;
        case SKERNEL_REGISTERS_DB_TYPE_DFX_E:
            dbBasePtr = (char*)SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr);
            break;
        case SKERNEL_REGISTERS_DB_TYPE_INTERNAL_PCI_E:
            dbBasePtr = (char*)SMEM_CHT_MAC_REG_DB_INTERNAL_PCI_GET(devObjPtr);
            break;
        case SKERNEL_REGISTERS_DB_TYPE_PIPE_E:
            dbBasePtr = (char*)SMEM_PIPE_MAC_REG_DB_GET(devObjPtr);
            break;

        case SKERNEL_REGISTERS_DB_TYPE_NOT_VALID_E:
            dbBasePtr = NULL;
            break;
        default:
            dbBasePtr = NULL;
            break;
    }

    if (dbBasePtr)
    {
        regInfoPtr = (getCause ? &currDbPtr->causeReg: &currDbPtr->maskReg);

        regAddr =
            *(GT_U32*)(dbBasePtr + regInfoPtr->registerOffsetInDb) +
            regInfoPtr->registerAddrOffset;

        if(regInfoPtr->mgUnitIndex & MG_UNIT_USED_CNS)
        {
            /* set the MG unit that relate to this register */
            smemSetCurrentMgUnitIndex(devObjPtr,regInfoPtr->mgUnitIndex - MG_UNIT_USED_CNS);

            /* support multi-MG units in Falcon (+ in multi-tile)*/
            smemConvertDevAndAddrToNewDevAndAddr(devObjPtr,regAddr,SKERNEL_MEMORY_READ_E,
                &devObjPtr,&regAddr);
        }
    }
    else
    {
        regAddr = SMAIN_NOT_VALID_CNS;
    }

    return regAddr;
}

/**
* @internal getInterruptCauseRegAddr function
* @endinternal
*
* @brief   get the cause register address from DB of interrupts.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currDbPtr                - the entry in the DB
*                                       regAddr
*/
static GT_U32 getInterruptCauseRegAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr
)
{
    return getInterruptRegAddr(devObjPtr, currDbPtr, GT_TRUE);
}

/**
* @internal getInterruptMaskRegAddr function
* @endinternal
*
* @brief   get the mask register address from DB of interrupts.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currDbPtr                - the entry in the DB
*                                       regAddr
*/
static GT_U32 getInterruptMaskRegAddr
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr
)
{
    return getInterruptRegAddr(devObjPtr, currDbPtr, GT_FALSE);
}

/**
* @internal lookForInterruptBasicInfoByCauseReg_isLogInfo function
* @endinternal
*
* @brief   look for match in the DB for cause register. (with without LOG info)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - Interrupt Cause Register
* @param[in] currDbPtr                - the place to check
* @param[in] isLogInfo                - indication to do LOG info
*
* @retval GT_TRUE                  - register was found
* @retval GT_FALSE                 - register was NOT found
*/
static GT_BOOL lookForInterruptBasicInfoByCauseReg_isLogInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr,
    IN GT_BOOL isLogInfo
)
{
    GT_U32  currCauseRegAddr;
    GT_U32  currMaskRegAddr;

    currCauseRegAddr = getInterruptCauseRegAddr(devObjPtr,currDbPtr);

    if(currCauseRegAddr != causeRegAddr)
    {
        return GT_FALSE;
    }

    currMaskRegAddr = getInterruptMaskRegAddr(devObjPtr,currDbPtr);

    if(isLogInfo != GT_FALSE)
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("causeRegAddr[0x%8.8x] [%s] \n"
                                                     "maskRegAddr [0x%8.8x] [%s] \n",
            causeRegAddr ,
            currDbPtr->causeReg.registerName ? currDbPtr->causeReg.registerName : "??",
            currMaskRegAddr ,
            currDbPtr->maskReg.registerName ? currDbPtr->maskReg.registerName : "??"));
    }

    return GT_TRUE;

}

/**
* @internal lookForInterruptBasicInfoByCauseReg function
* @endinternal
*
* @brief   look for match in the DB for cause register.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - Interrupt Cause Register
* @param[in] currDbPtr                - the place to check
*
* @retval GT_TRUE                  - register was found
* @retval GT_FALSE                 - register was NOT found
*/
static GT_BOOL lookForInterruptBasicInfoByCauseReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr
)
{
    return lookForInterruptBasicInfoByCauseReg_isLogInfo(devObjPtr,causeRegAddr,currDbPtr,GT_TRUE);
}


typedef void (*SMEM_REG_GET_FUNC)
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN GT_U32                  address,
    OUT GT_U32                  *dataPtr
);
typedef void (*SMEM_REG_SET_FUNC)
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj,
    IN GT_U32                  address,
    IN GT_U32                  data
);

static GT_U32   forceIntReadLogEnable = 0;
/* by default the read of interrupts (from the CPU) will not generate LOG info
    because otherwise the read of 'FDB learning' will cause LOG of 2 same runs
    to look different when compared to each other */
void simForceIntReadLogEnable(IN GT_U32 enable)
{
    forceIntReadLogEnable = enable;
}

/**
* @internal doInterruptsSubTree function
* @endinternal
*
* @brief   handle a sub tree in the interrupts DB.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] currDbPtr                - the place to check
* @param[in] causeBitBmp              - bmp of the bits that need to be set/clear in the cause register
* @param[in] isReadAction             - GT_TRUE  - read action by the CPU (the device will clear interrupts if needed)
*                                      GT_FALSE - the device sets interrupts if needed
*                                      OUTPUT:
*                                       None
*/
static void doInterruptsSubTree
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr,
    IN GT_U32 causeBitBmp,
    IN GT_BOOL  isReadAction
)
{
    DECLARE_FUNC_NAME(doInterruptsSubTree);

    GT_U32   causeRegAddr;
    GT_U32   maskRegAddr;
    GT_U32   causeRegVal;             /* Cause register value */
    GT_U32   causeRegMask;            /* Cause register mask */
    static GT_U32   globalCauseRegAddr = 0x00000030;/* global cause register address */
    static GT_U32   sip6_globalCauseRegAddr = SMAIN_NOT_VALID_CNS;/* sip6 : global cause register address (set at run time)*/
    static GT_U32   ac5_globalCauseRegAddr = SMAIN_NOT_VALID_CNS;/* AC5 : global cause register address (set at run time)*/
    GT_U32   causeBitBmpToMyFather;
    SKERNEL_INTERRUPT_REG_INFO_STC  *myFather_currDbPtr;
    SKERNEL_DEVICE_OBJECT * origDevObjPtr;/* orig device */
    GT_BIT  updateSummaryAndFather;
    GT_U32  ii,iiMax,kk;/* iterator and it's max value */
    SMEM_REG_GET_FUNC regGetFunc = NULL;
    SMEM_REG_SET_FUNC regSetFunc = NULL;
    GT_BIT  allowLogInfo = ((isReadAction == GT_FALSE || forceIntReadLogEnable) && simLogIsOpenFlag) ? 1 : 0;
    GT_CHAR* causeReg_registerName = currDbPtr->causeReg.registerName ? currDbPtr->causeReg.registerName : "??";
    GT_CHAR* maskReg_registerName = currDbPtr->maskReg.registerName ? currDbPtr->maskReg.registerName : "??";
    GT_U32  current_globalCauseRegAddr;
    GT_U32  origMgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);
    GT_U32  newMgUnitId;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        if(sip6_globalCauseRegAddr == SMAIN_NOT_VALID_CNS)
        {
            /* set it one time */
            sip6_globalCauseRegAddr = SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr);/*MG.globalInterrupt.globalInterruptCause;*/
        }
        current_globalCauseRegAddr = sip6_globalCauseRegAddr;
    }
    else
    {
        if (ac5_globalCauseRegAddr == SMAIN_NOT_VALID_CNS)
        {
            /* set it one time */
            ac5_globalCauseRegAddr = SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->MG.globalInterrupt.globalAc5CnmInterruptCause;
        }
        if (ac5_globalCauseRegAddr != SMAIN_NOT_VALID_CNS)
        {
            current_globalCauseRegAddr = ac5_globalCauseRegAddr;
        }
        else
        {
            current_globalCauseRegAddr = globalCauseRegAddr;
        }

    }

    causeRegAddr = getInterruptCauseRegAddr(devObjPtr,currDbPtr);
    maskRegAddr  = getInterruptMaskRegAddr(devObjPtr,currDbPtr);

    if(devObjPtr->devMemUnitMemoryGetPtr)
    {
        /* protect the interrupt tree between the cores that access to shared units */
        SCIB_SEM_TAKE;

        /* check if need to trigger the cause register interrupt on other device.
        if yes then also the ISR should triggered on the other device */
        origDevObjPtr = devObjPtr;
        devObjPtr =
            devObjPtr->devMemUnitMemoryGetPtr(origDevObjPtr,causeRegAddr);

        if(origDevObjPtr != devObjPtr)
        {
            /* the port group changed */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("due to 'shared unit'(reg addr[0x%8.8x]) the port group changed from [%d] to [%d]\n"
                          ,causeRegAddr
                          ,origDevObjPtr->portGroupId
                          ,devObjPtr->portGroupId
                          ));

        }
    }

    if(currDbPtr->useNonSwitchMemorySpaceAccessType &&
        currDbPtr->nonSwitchMemorySpaceAccessType != SCIB_MEM_ACCESS_CORE_E)
    {
        switch(currDbPtr->nonSwitchMemorySpaceAccessType)
        {
            case SCIB_MEM_ACCESS_PCI_E:
            regGetFunc = smemPciRegGet;
            regSetFunc = smemPciRegSet;
                break;
            case SCIB_MEM_ACCESS_DFX_E:
            regGetFunc = smemDfxRegGet;
            regSetFunc = smemDfxRegSet;
                break;
            default:
                skernelFatalError("doInterruptsSubTree: invalid nonSwitchMemorySpaceAccessType[%d]\n",
                    currDbPtr->nonSwitchMemorySpaceAccessType);
                return;
        }
    }
    else
    {
        regGetFunc = smemRegGet;
        regSetFunc = smemRegSet;
    }

    /* read interrupt cause data */
    regGetFunc(devObjPtr, causeRegAddr, &causeRegVal);

    if(maskRegAddr == SMAIN_NOT_VALID_CNS)
    {
        /*Allow the mask be not valid ... meaning that there is no mask and cause always valid */
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("No mask interrupt register for cause interrupt register [0x%x] \n",causeRegAddr));
        causeRegMask = 0xFFFFFFFF;
    }
    else
    {
        /* read interrupt mask data */
        regGetFunc(devObjPtr, maskRegAddr, &causeRegMask);
    }

    if(isReadAction == GT_TRUE)
    {
        if(causeBitBmp == 0xFFFFFFFF)
        {
            /* indication that called from the 'active memory' of 'read cause register' */
            if(currDbPtr->nonClearOnReadBmp)
            {
                /* remove the bits that should not be cleared by the 'read' that
                    triggered by the CPU */
                causeBitBmp &= ~currDbPtr->nonClearOnReadBmp;
            }
        }
        else
        {
            /* the device triggered 'clear' of specific bit even if considered
               not to be cleared when CPU access !!!! */
            /* see call from smemAldrinInterruptMpp(...) -->
                snetChetahReadCauseInterruptFromTree(...) */
        }

        if(allowLogInfo)
        {
            GT_U32  cleardedBits = causeRegVal & causeBitBmp;

            __LOG(("==== Start Read Cause interrupt register Addr [0x%8.8x] [%s]and value [0x%8.8x] ====\n",
                causeRegAddr, causeReg_registerName,causeRegVal));
            __LOG(("           Read Mask  interrupt register Addr [0x%8.8x] [%s]and value [0x%8.8x] \n",
                maskRegAddr, maskReg_registerName,causeRegMask));
            LOG_BMP_ARR("try to clear from interrupt cause register next bits",&causeBitBmp,32);
            LOG_BMP_ARR("actual clear from interrupt cause register next bits",&cleardedBits,32);
        }

        /* clear from the cause the needed bits */
        causeRegVal &= ~causeBitBmp;

        if(currDbPtr->isBit0NotSummary)
        {
            /* do not bypass bit 0 in considerations */
            updateSummaryAndFather = causeRegVal ? 0 : 1;
        }
        else
        {
            updateSummaryAndFather = (causeRegVal & 0xFFFFFFFE) ? 0 : 1;
        }

        if(!updateSummaryAndFather && allowLogInfo)
        {
            LOG_BMP_ARR("The interrupt cause register still hold next bits (so will not trigger the read on the 'father') ",&causeRegVal,32);
        }
    }
    else
    {
        if(allowLogInfo)
        {
            __LOG(("==== Start handling Cause interrupt register Addr [0x%8.8x] [%s]and value [0x%8.8x] ====\n",
                causeRegAddr, causeReg_registerName,causeRegVal));
            __LOG(("               Mask  interrupt register Addr [0x%8.8x] [%s]and value [0x%8.8x] \n",
                maskRegAddr, maskReg_registerName,causeRegMask));
            LOG_BMP_ARR("set interrupt cause register with bits",&causeBitBmp,32);
        }

        /* set cause bit in the data for interrupt cause */
        causeRegVal |= causeBitBmp;
        updateSummaryAndFather = (causeBitBmp & causeRegMask) ? 1 : 0;

    }

    /* if mask is set for bitmap, set summary bit in data for interrupt cause */
    if (updateSummaryAndFather)
    {
        if(isReadAction == GT_TRUE)
        {
            /* the register hold no more reasons to be with interrupts indications */
            causeRegVal = 0;
        }
        else
        {
            if(currDbPtr->isBit0NotSummary)
            {
                /* bit 0 is NOT summary so do not set it */
            }
            else
            {
                if(allowLogInfo)
                {
                    __LOG(("set bit0 at cause register (this is summary bit) \n"));
                }
                SMEM_U32_SET_FIELD(causeRegVal, 0, 1, 1);
            }
        }

        if(allowLogInfo)
        {
            __LOG(("Cause interrupt register Addr [0x%8.8x] [%s] updated with new value [0x%8.8x] \n",
                causeRegAddr, causeReg_registerName ,causeRegVal));
        }

        regSetFunc(devObjPtr, causeRegAddr, causeRegVal);

        iiMax = currDbPtr->isSecondFatherExists ? 2 : 1;

        if(iiMax > 1)
        {
            if(allowLogInfo)
            {
                __LOG(("There are [%d] multiple 'father' registers that are triggered by this summary \n"));
            }
        }


        for(ii = 0 ; ii < iiMax ; ii++)
        {
            if(ii == 0 )
            {
                /* my representative bit in my fathers register */
                causeBitBmpToMyFather = (1 << currDbPtr->myFatherInfo.myBitIndex);

                /* we are allowed to go to the 'father' and set our representative bit in it */
                myFather_currDbPtr = currDbPtr->myFatherInfo.interruptPtr;

                if(currDbPtr->splitInfoNumValid)
                {
                    for(kk = 0 ; kk < currDbPtr->splitInfoNumValid ; kk++)
                    {
                        if(causeBitBmp & currDbPtr->splitRegs[kk].bmpOfMyBitsToTriggerFather)
                        {
                            causeBitBmpToMyFather = 1 << currDbPtr->splitRegs[kk].bitInFather;
                            break;
                        }
                    }
                }

            }
            else /* support second father */
            {
                /* my representative bit in my second fathers register */
                causeBitBmpToMyFather = (1 << currDbPtr->myFatherInfo_2.myBitIndex);

                /* we are allowed to go to the 'second father' and set our representative bit in it */
                myFather_currDbPtr = currDbPtr->myFatherInfo_2.interruptPtr;
            }

            if(myFather_currDbPtr == NULL)
            {
                /* we should be at the 'global' register */
                if(current_globalCauseRegAddr == causeRegAddr)
                {
                    if(isReadAction == GT_TRUE)
                    {
                        /* notify if needed to the CPU that the interrupt is cleared ('LEVEL' mode behavior) */
                        if(simLogIsOpenFlag)/*we want to see this one also for 'read' that clear the Global summary */
                        {
                            GT_BOOL orig_simLogFilterTypeCpuIsr = simLogFilterTypeCpuIsr;
                            simLogFilterTypeCpuIsr = GT_FALSE;/* do not filter ISR */
                            __LOG(("Un-Trigger (de-assert) The interrupt line (that is connected to the CPU) \n"));
                            simLogFilterTypeCpuIsr = orig_simLogFilterTypeCpuIsr;/* restore */
                        }
                        snetChtPerformScibUnSetInterrupt(devObjPtr->deviceId);
                    }
                    else
                    {
                        /* set the global interrupt too */
                        if(allowLogInfo)
                        {
                            __LOG(("Trigger The interrupt line (that is connected to the CPU) \n"));
                        }
                        snetChtPerformScibSetInterrupt(devObjPtr->deviceId);
                    }
                }
            }
            else
            {
                if(allowLogInfo)
                {
                    if(ii == 0)
                    {
                        __LOG(("Ended handling Cause interrupt register Addr [0x%8.8x] [%s] \n",
                            causeRegAddr,causeReg_registerName));
                        __LOG(("               Mask  interrupt register Addr [0x%8.8x] [%s] \n",
                            maskRegAddr,maskReg_registerName));

                        __LOG(("======== Start handling 'my father' ========\n"));
                    }
                    else
                    {
                        __LOG(("============ Start handling another 'my father' for Cause interrupt register Addr [0x%8.8x] [%s] ============\n",
                            causeRegAddr,causeReg_registerName));
                    }

                }

                /* let my father handle it's predecessors */
                doInterruptsSubTree(devObjPtr,myFather_currDbPtr,causeBitBmpToMyFather,isReadAction);
            }
        }
    }
    else
    {
        if(isReadAction == GT_TRUE)
        {
            /* the register hold other reasons to be 'triggered' from other cause registers ... */
            /* so clear only the needed bits in it and do not clear it's summary and it's father */
        }

        if(allowLogInfo)
        {
            if(isReadAction == GT_TRUE)
            {
                __LOG(("Read of current cause register addr[0x%8.8x] (new)value[0x%8.8x] not continue to the 'father' cause register \n",
                    causeRegAddr,causeRegVal));
            }
            else
            {
                __LOG(("current mask register addr[0x%8.8x] value[0x%8.8x] not allow to trigger 'my' bit at the 'father' cause register \n",
                    maskRegAddr,causeRegMask));
            }
        }

        regSetFunc(devObjPtr, causeRegAddr, causeRegVal);
    }

    if(devObjPtr->devMemUnitMemoryGetPtr)
    {
        SCIB_SEM_SIGNAL;
    }

    newMgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);
    if(newMgUnitId != origMgUnitId)
    {
        /* restore the MG unit Index */
        __LOG(("restore the MG unit Index to [%d] from [%d]",origMgUnitId,newMgUnitId));
        smemSetCurrentMgUnitIndex(devObjPtr,origMgUnitId);
    }

    return;
}

/**
* @internal snetChetahDoInterruptFromTree function
* @endinternal
*
* @brief   Set cheetah interrupt according to the 'tree' of the interrupts
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - Interrupt Cause Register
*                                      causeBitNum         - Interrupt Cause Register Bit
*                                      setInterrupt        - set the interrupt (or not)
*
* @retval GT_TRUE                  - register was found and treated
* @retval GT_FALSE                 - register was NOT found and therefore not treated
*/
static GT_BOOL snetChetahDoInterruptFromTree
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN GT_U32 causeBitBmp
)
{
    DECLARE_FUNC_NAME(snetChetahDoInterruptFromTree);

    SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr;
    GT_BOOL    found;

    /* support multi-pipe in bc3 */
    smemConvertDevAndAddrToNewDevAndAddr(devObjPtr,causeRegAddr,SKERNEL_MEMORY_READ_E,
        &devObjPtr,&causeRegAddr);

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    while(currDbPtr &&
          currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        /* check the specific line */
        found = lookForInterruptBasicInfoByCauseReg_isLogInfo(devObjPtr,causeRegAddr,currDbPtr,GT_FALSE);

        if(found == GT_TRUE)
        {
            /* start handling the leaf with all it's predecessors */
            doInterruptsSubTree(devObjPtr,currDbPtr,causeBitBmp,GT_FALSE);

            return GT_TRUE;
        }

        currDbPtr++;
    }

    __LOG(("Cause interrupt register addr [0x%8.8x] was not found in the 'generic' interrupts tree of the simulation !!! \n",
        causeRegAddr));
    return GT_FALSE;
}

/**
* @internal snetChetahReadCauseInterruptFromTree function
* @endinternal
*
* @brief   Read may causing 'clear' of my father's cause register
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - Interrupt Cause Register
* @param[in] bmpOfBitsToClear         - bmp of bits to clear in the register
*
* @retval GT_TRUE                  - register was found and treated
* @retval GT_FALSE                 - register was NOT found and therefore not treated
*/
GT_BOOL snetChetahReadCauseInterruptFromTree
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN GT_U32 bmpOfBitsToClear
)
{
    SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr;
    GT_BOOL    found;

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    while(currDbPtr &&
          currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        /* check the specific line */
        found = lookForInterruptBasicInfoByCauseReg(devObjPtr,causeRegAddr,currDbPtr);

        if(found == GT_TRUE)
        {
            /* start handling the leaf with all it's predecessors */
            doInterruptsSubTree(devObjPtr,currDbPtr,bmpOfBitsToClear,GT_TRUE);

            return GT_TRUE;
        }

        currDbPtr++;
    }

    return GT_FALSE;
}


/**
* @internal interruptsTreeDump function
* @endinternal
*
* @brief   dump the interrupt tree of a device
*
* @retval GT_TRUE                  - register was found and treated
* @retval GT_FALSE                 - register was NOT found and therefore not treated
*/
static void interruptsTreeDump
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr;
    GT_U32   causeRegAddr;
    GT_U32   maskRegAddr;
    GT_U32   causeBitToMyFather;
    SKERNEL_INTERRUPT_REG_INFO_STC  *myFather_currDbPtr;
    GT_U32   myFather_causeRegAddr;
    GT_U32   myFather_maskRegAddr;
    GT_U32  ii,iiMax;/* iterator and it's max value */

    currDbPtr = devObjPtr->myInterruptsDbPtr;

    while(currDbPtr &&
          currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        causeRegAddr = getInterruptCauseRegAddr(devObjPtr,currDbPtr);
        maskRegAddr  = getInterruptMaskRegAddr(devObjPtr,currDbPtr);

        iiMax = currDbPtr->isSecondFatherExists ? 2 : 1;
        if(currDbPtr->isSecondFatherExists)
        {
            simGeneralPrintf("-- NOTE: This node with 2 fathers  -- \n");
        }

        for(ii = 0 ; ii < iiMax ; ii++)
        {
            if(ii == 0 )
            {
                /* my representative bit in my fathers register */
                causeBitToMyFather = currDbPtr->myFatherInfo.myBitIndex;

                /* we are allowed to go to the 'father' and set our representative bit in it */
                myFather_currDbPtr = currDbPtr->myFatherInfo.interruptPtr;
            }
            else /* support second father */
            {
                /* my representative bit in my second fathers register */
                causeBitToMyFather = currDbPtr->myFatherInfo_2.myBitIndex;

                /* we are allowed to go to the 'second father' and set our representative bit in it */
                myFather_currDbPtr = currDbPtr->myFatherInfo_2.interruptPtr;
            }

            myFather_causeRegAddr = getInterruptCauseRegAddr(devObjPtr,myFather_currDbPtr);
            myFather_maskRegAddr  = getInterruptMaskRegAddr(devObjPtr,myFather_currDbPtr);

            simGeneralPrintf("causeRegAddr[0x%8.8x] as [%s] \n"
                     "maskRegAddr[0x%8.8x] as [%s] \n"
                     "causeBitToMyFather[%d] \n"
                     "myFather_currDbPtr->causeRegAddr[0x%8.8x] as [%s] \n"
                     "myFather_currDbPtr->maskRegAddr[0x%8.8x] as [%s] \n"
                     ,
                /* info from me */
                causeRegAddr ,
                currDbPtr->causeReg.registerName ? currDbPtr->causeReg.registerName : "??",
                maskRegAddr ,
                currDbPtr->maskReg.registerName ? currDbPtr->maskReg.registerName : "??",
                /* info from my direct father */
                myFather_currDbPtr ? causeBitToMyFather : 0,
                myFather_currDbPtr ? myFather_causeRegAddr : 0,
                myFather_currDbPtr ? (myFather_currDbPtr->causeReg.registerName ? myFather_currDbPtr->causeReg.registerName : "??") : "no father",
                myFather_currDbPtr ? myFather_maskRegAddr : 0,
                myFather_currDbPtr ? (myFather_currDbPtr->maskReg.registerName  ? myFather_currDbPtr->maskReg.registerName  : "??") : "no father"

                );
        }

        currDbPtr++;
    }

    return;
}

/**
* @internal snetChetahInterruptsTreeDump function
* @endinternal
*
* @brief   dump the interrupt tree of a device
*
* @param[in] devNum                   - device number as stated in the INI file.
*
* @retval GT_TRUE                  - register was found and treated
* @retval GT_FALSE                 - register was NOT found and therefore not treated
*/
void snetChetahInterruptsTreeDump
(
    IN  GT_U32    devNum
)
{
    SKERNEL_DEVICE_OBJECT* deviceObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);
    SKERNEL_DEVICE_OBJECT* currDeviceObjPtr;
    GT_U32  dev;

    if(deviceObjPtr->shellDevice == GT_TRUE)
    {
        simGeneralPrintf(" multi-core device [%d] \n",devNum);
        for(dev = 0 ; dev < deviceObjPtr->numOfCoreDevs ; dev++)
        {
            currDeviceObjPtr = deviceObjPtr->coreDevInfoPtr[dev].devObjPtr;

            if(currDeviceObjPtr->myInterruptsDbPtr == NULL)
            {
                simGeneralPrintf("the device not supports interrupts tree \n");
                return;
            }

            simGeneralPrintf("\n\n ===interrupts tree : Start  ====\n\n");

            interruptsTreeDump(currDeviceObjPtr);

            simGeneralPrintf("\n\n ===interrupts tree : Done  ====\n\n");
        }
    }
    else
    {
        if(deviceObjPtr->myInterruptsDbPtr == NULL)
        {
            simGeneralPrintf("the device not supports interrupts tree \n");
            return;
        }

        simGeneralPrintf("\n\n ===interrupts tree : Start  ====\n\n");

        interruptsTreeDump(deviceObjPtr);

        simGeneralPrintf("\n\n ===interrupts tree : Done  ====\n\n");

    }


}

/**
* @internal snetChetahDoInterruptLimited function
* @endinternal
*
* @brief   Set cheetah interrupt
*/
GT_VOID snetChetahDoInterruptLimited
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN GT_U32 causeMaskRegAddr,
    IN GT_U32 causeBitBmp,
    IN GT_U32 globalBitBmp,
    IN GT_U32 setInterrupt
)
{
    DECLARE_FUNC_NAME(snetChetahDoInterruptLimited);

    GT_U32 causeRegVal;             /* Cause register value */
    GT_U32 causeRegMask;            /* Cause register mask */
    GT_BOOL setIntr = GT_FALSE;     /* Set interrupt */
    GT_BOOL setGlbIntr = GT_FALSE;  /* Set interrupt in global register */
    SKERNEL_DEVICE_OBJECT * origDevObjPtr;/* orig device */
    GT_BOOL found;

    if(devObjPtr->myInterruptsDbPtr)
    {
        GT_U32  origMgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);
        /* the device hold tree of interrupts , let it find the register in the tree */
        /* it may not be found due to 'intermediate' development time that not all
           tree supported fully */
        found = snetChetahDoInterruptFromTree(devObjPtr,causeRegAddr,causeBitBmp);

        /* restore the MG unit that handle the operation */
        smemSetCurrentMgUnitIndex(devObjPtr,origMgUnitId);
        if(found == GT_TRUE)
        {
            return;
        }
    }

    if(devObjPtr->devMemUnitMemoryGetPtr)
    {
        /* protect the interrupt tree between the cores that access to shared units */
        SCIB_SEM_TAKE;

        /* check if need to trigger the cause register interrupt on other device.
        if yes then also the ISR should triggered on the other device */
        origDevObjPtr = devObjPtr;
        devObjPtr =
            devObjPtr->devMemUnitMemoryGetPtr(origDevObjPtr,causeRegAddr);

        if(origDevObjPtr != devObjPtr)
        {
            /* the port group changed */
            __LOG(("due to 'shared unit'(reg addr[0x%8.8x]) the port group changed from [%d] to [%d]\n"
                          ,causeRegAddr
                          ,origDevObjPtr->portGroupId
                          ,devObjPtr->portGroupId
                          ));

        }
    }

    /* read interrupt cause data */
    smemRegGet(devObjPtr, causeRegAddr, &causeRegVal);

    /* read interrupt mask data */
    smemRegGet(devObjPtr, causeMaskRegAddr, &causeRegMask);

    /* set cause bit in the data for interrupt cause */
    causeRegVal = causeRegVal | causeBitBmp;

    /* if mask is set for bitmap, set summary bit in data for interrupt cause */
    if (causeBitBmp & causeRegMask)
    {
        SMEM_U32_SET_FIELD(causeRegVal, 0, 1, 1);
        setIntr = GT_TRUE;
    }

    smemRegSet(devObjPtr, causeRegAddr, causeRegVal);

    if (setIntr == GT_TRUE)
    {
        /* Global Interrupt Cause Register */
        smemRegGet(devObjPtr, SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr), &causeRegVal);

        /* Global Interrupt Summary Mask */
        smemRegGet(devObjPtr, SMEM_CHT_GLB_INT_MASK_REG(devObjPtr), &causeRegMask);

        /* set cause bit in the data for interrupt cause */
        causeRegVal = causeRegVal | globalBitBmp;

        /* if mask is set for bitmap, set summary bit in data for interrupt cause */
        if (globalBitBmp & causeRegMask)
        {
            /* IntSum */
            SMEM_U32_SET_FIELD(causeRegVal, 0, 1, 1);
            setGlbIntr = GT_TRUE;
        }

        smemRegSet(devObjPtr, SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr), causeRegVal);
    }

    if (setGlbIntr && setInterrupt)
    {
        snetChtPerformScibSetInterrupt(devObjPtr->deviceId);
    }

    __LOG(("Global Interrupt [%s generated] , causeRegAddr[0x%8.8x] causeMaskRegAddr[0x%8.8x] causeBitBmp[0x%8.8x]\n"
                  ,(setGlbIntr && setInterrupt) ? "" : "not"
                  ,causeRegAddr
                  ,causeMaskRegAddr
                  ,causeBitBmp
                  ));

    if(devObjPtr->devMemUnitMemoryGetPtr)
    {
        SCIB_SEM_SIGNAL;
    }

}

/**
* @internal snetChetahDoInterrupt function
* @endinternal
*
* @brief   Set cheetah interrupt
*/
GT_VOID snetChetahDoInterrupt
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 causeRegAddr,
    IN GT_U32 causeMaskRegAddr,
    IN GT_U32 causeBitBmp,
    IN GT_U32 globalBitBmp
)
{
    snetChetahDoInterruptLimited(devObjPtr,causeRegAddr,causeMaskRegAddr,
                causeBitBmp,globalBitBmp,1);
}
/**
* @internal snetCheetahInterruptsMaskChanged function
* @endinternal
*
* @brief   handle interrupt mask registers
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] causeRegAddr             - cause register address
* @param[in] maskRegAddr              - mask  register address
* @param[in] intRegBit                - interrupt bit in the global cause register
* @param[in] currentCauseRegVal       - current cause register values
* @param[in] lastMaskRegVal           - last mask register values
* @param[in] newMaskRegVal            - new mask register values
*/
void snetCheetahInterruptsMaskChanged(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  causeRegAddr,
    IN GT_U32  maskRegAddr,
    IN GT_U32  intRegBit,
    IN GT_U32  currentCauseRegVal,
    IN GT_U32  lastMaskRegVal,
    IN GT_U32  newMaskRegVal
)
{
    GT_U32  diffCause;

    diffCause = ((newMaskRegVal & 0xFFFFFFFE) & ~lastMaskRegVal) & currentCauseRegVal;

    /* check if there is a reason to do interrupt */
    if(diffCause)
    {
        snetChetahDoInterrupt(devObjPtr,causeRegAddr,
                              maskRegAddr,diffCause,intRegBit);
    }

    return;
}

/**
* @internal smemChtActiveGenericWriteInterruptsMaskReg function
* @endinternal
*
* @brief   The application changed the value of the interrupts mask register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveGenericWriteInterruptsMaskReg(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr;
    GT_BOOL                         found = GT_FALSE;
    GT_U32                          currMaskRegAddr = 0;
    GT_U32                          currCauseRegAddr = 0;
    GT_U32                          globalInterruptCause;
    GT_U32                          globalInterruptCauseBitIndex = 0;
    SKERNEL_INTERRUPT_REG_INFO_STC  *matchedCurrDbPtr = NULL;

    /* Find mask register in interrupt tree */
    currDbPtr = devObjPtr->myInterruptsDbPtr;
    if (currDbPtr == NULL)
        return;
    while(currDbPtr &&
                 currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        currMaskRegAddr = getInterruptMaskRegAddr(devObjPtr,currDbPtr);
        if (currMaskRegAddr == address)
        {
            found = GT_TRUE;
            currCauseRegAddr = getInterruptCauseRegAddr(devObjPtr,currDbPtr);
            matchedCurrDbPtr = currDbPtr;
            break;
        }
        currDbPtr++;
    }
    if (found == GT_TRUE && matchedCurrDbPtr)
    {
        /* Address of data path interrupt cause register */
        dataArray[0] = currCauseRegAddr;
        /* Address of data ptath interrupt mask register */
        dataArray[1] = currMaskRegAddr;
        /* Summary bit in global interrupt cause register */
        globalInterruptCause = SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr);
        currCauseRegAddr = getInterruptCauseRegAddr(devObjPtr, currDbPtr->myFatherInfo.interruptPtr);
        while (currCauseRegAddr != globalInterruptCause)
        {
            currDbPtr = currDbPtr->myFatherInfo.interruptPtr;
            currCauseRegAddr = getInterruptCauseRegAddr(devObjPtr, currDbPtr->myFatherInfo.interruptPtr);
        }
        if (currCauseRegAddr == globalInterruptCause)
        {
            globalInterruptCauseBitIndex = currDbPtr->myFatherInfo.myBitIndex;
            if (globalInterruptCauseBitIndex != 0 && globalInterruptCauseBitIndex <= 32)
            {
                dataArray[2] = globalInterruptCauseBitIndex;

                if(matchedCurrDbPtr->useNonSwitchMemorySpaceAccessType)
                {
                    /* indication to smemChtActiveWriteInterruptsMaskReg() how
                       to read register content */
                    memSize |= (1+(currDbPtr->nonSwitchMemorySpaceAccessType)) << 29;
                }

                /* Call for common interrupt mask active write function */
                smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                    (GT_UINTPTR)dataArray, inMemPtr);
            }
        }
    }
    return;
}

/**
* @internal smemChtActiveReadIntrCauseReg function
* @endinternal
*
* @brief   Read and reset interrupts cause registers and summary bits.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  -  for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - pointer to the register's memory in the simulation.
* @param[in] sumBit                   - global summary interrupt bit
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemChtActiveReadIntrCauseReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR sumBit,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32  found;
    GT_U32 * glbIntCauseRegPtr;     /* Pointer to global cause register */

    /* Copy registers content to the output memory */
    *outMemPtr = *memPtr;

    if(devObjPtr->myInterruptsDbPtr)
    {
        /* the device hold tree of interrupts , let it find the register in the tree */
        /* it may not be found due to 'intermediate' development time that not all
           tree supported fully */
        found = snetChetahReadCauseInterruptFromTree(devObjPtr,
            address/*causeRegAddr*/,
            0xFFFFFFFF/* clear all the bits in this cause register*/);

        if(found == GT_TRUE)
        {
            return;
        }
    }

    /* Clear interrupt cause register (there is an assumption to use this
       function only for interrupt cause registers when all fields type are ROC) */
    smemRegSet(devObjPtr, address, 0);

    /* Get memory of Global Cause Registers */
    glbIntCauseRegPtr =
        smemMemGet(devObjPtr, SMEM_CHT_GLB_INT_CAUSE_REG(devObjPtr));

    /* Update summary bit in the Global Interrupt Summary Cause */
    SMEM_U32_SET_FIELD(*glbIntCauseRegPtr, sumBit, 1, 0);

    /* Check another causes existence */
    if ((*glbIntCauseRegPtr & 0xfffffffe) == 0)
    {
        /* Clear if not any cause of interrupt */
        *glbIntCauseRegPtr = 0;
    }
}

/**
* @internal smemChtActiveWriteIntrCauseReg function
* @endinternal
*
* @brief   Write interrupts cause registers by read/write mask.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - memory size to be read.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] writeMask                - 32 bits mask of writable bits.
* @param[in] inMemPtr                 - Pointer to the memory to set register's content.
*/
void smemChtActiveWriteIntrCauseReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR writeMask,
    IN         GT_U32 * inMemPtr
)
{
    SKERNEL_INTERRUPT_REG_INFO_STC  *currDbPtr;
    GT_BOOL                         found = GT_FALSE;
    GT_U32                          causeBitBmp;
    GT_U32                          causeRegAddr;
    /* Write action is changing only writable bits */
    if (writeMask)
    {
        /* Clear writable bits by mask */
        (*memPtr) &= ~writeMask;
        /* Write values to writable bits by mask */
        (*memPtr) |= ((*inMemPtr) & writeMask);
    }

    currDbPtr = devObjPtr->myInterruptsDbPtr;
    if (currDbPtr == NULL)
        return;

    causeBitBmp = *inMemPtr;
    causeRegAddr = address;

    while(currDbPtr &&
          currDbPtr->causeReg.registersDbType != SKERNEL_REGISTERS_DB_TYPE__LAST___E)
    {
        found = lookForInterruptBasicInfoByCauseReg(devObjPtr, causeRegAddr, currDbPtr);
        /* Generate interrupt if cause register was found in the interrupt tree */
        if(found == GT_TRUE)
        {
            if (currDbPtr->isTriggeredByWrite == 1)
            {
                (*memPtr) = (*inMemPtr);
                doInterruptsSubTree(devObjPtr,currDbPtr,causeBitBmp,GT_FALSE);
            }
            break;
        }
        currDbPtr++;
    }

    if(found == GT_FALSE)
    {
        /* was not found in the tree */
        /* just write it's content   */
        (*memPtr) = (*inMemPtr);
    }

    return;
}

static GT_U32   waitTime = 10;
GT_STATUS   smemChtActiveWriteInterruptsMaskReg_waitTime(IN GT_U32 time)
{
    waitTime = time;
    return GT_OK;
}

/**
* @internal smemChtActiveWriteInterruptsMaskReg function
* @endinternal
*
* @brief   The application changes the value of the interrupts mask register.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemChtActiveWriteInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;               /* buffer */
    GT_U32 * dataPtr;                   /* pointer to the data in the buffer */
    GT_U32 dataSize;                    /* data size */
    GT_U32 currCauseReg;                /* interrupt cause register content */
    GT_U32 lastValue = *memPtr;         /* last value before the change */
    GT_U32 * prmArray = (GT_U32 *)param;/* parameters array */
    GT_U8  *dataU8Ptr;                   /* pointer to the data in the buffer */

    /* Read the cause register --- without clearing it */

    /*memSize is not in use .... use it to pass the memory type of the interrupt :
        PP/DFX/internal_pci */
    /*see logic in : smemChtActiveGenericWriteInterruptsMaskReg() */
    switch((memSize >> 29) & 0x7)/* 3 MSBits */
    {
        default:
            /* interrupt from the PP memory space */
            smemRegGet(devObjPtr, prmArray[0],&currCauseReg);
            break;
        case (SCIB_MEM_ACCESS_PCI_E + 1):
            /* interrupt from the internal PCI memory space */
            smemPciRegGet(devObjPtr, prmArray[0],&currCauseReg);
            break;
        case (SCIB_MEM_ACCESS_DFX_E + 1):
            /* interrupt from the DFX memory space */
            smemDfxRegGet(devObjPtr, prmArray[0],&currCauseReg);
            break;
    }

    if (lastValue == *inMemPtr)
    {
        /* unchanged, nothing to do */
        return;
    }
    /* Update the register value */
    *memPtr = *inMemPtr;

    /* Get buffer -- allocate size for max supported frame size */
    bufferId = sbufAlloc(devObjPtr->bufPool, SBUF_DATA_SIZE_CNS);   /*2000*/

    if (bufferId == NULL)
    {
        static GT_U32   counter = 0;
        /* limit the number of printings as when running huge events tests ,
           we get huge amount of printings that not impact the test functionality ,
           but make Huge LOG */
        GT_U32  lockCount = scibAccessMutexCountGet();

        /* need to allow the 'smainTask' to free buffers */
        /* but it can't free buffers when the scib mutex is taken */
        if(lockCount == 2)
        {   /* happen when 'usePexLogic' , that function wmMemPciWrite take
            the mutex and then calling to scibMemoryClientRegWrite that also
            take the mutex */
            SCIB_SEM_SIGNAL;
        }
        SCIB_SEM_SIGNAL;
        devObjPtr = skernelSleep(devObjPtr, waitTime);
        SCIB_SEM_TAKE;
        if(lockCount == 2)
        {
            SCIB_SEM_TAKE;
        }
        /* Get buffer -- allocate size for max supported frame size */
        bufferId = sbufAlloc(devObjPtr->bufPool, SBUF_DATA_SIZE_CNS);   /*2000*/

        if (bufferId == NULL)
        {
            if((counter < 256) || (0x3ff == (counter & 0x3ff)/*every 1024*/))
            {
                simWarningPrintf(" smemChtActiveWriteInterruptsMaskReg:device[%s] "\
                        "no buffers to update interrupt mask (count[%d])\n",
                        devObjPtr->deviceName,
                        counter);
            }
            counter++;

            return;
        }
    }
    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8**)&dataU8Ptr, &dataSize);
    dataPtr = (GT_U32*)dataU8Ptr;

    /* Interrupt cause register */
    dataPtr[0] = prmArray[0];
    /* Interrupt mask register */
    dataPtr[1] = prmArray[1];
    /* Summary bit in global interrupt cause register */
    dataPtr[2] = prmArray[2];
    /* Current interrupt cause register */
    dataPtr[3] = currCauseReg;
    /* Interrupt mask last value */
    dataPtr[4] = lastValue;
    /* Interrupt mask new value */
    dataPtr[5] = *memPtr;

    /* Set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* Set message type of buffer */
    bufferId->dataType = SMAIN_INTERRUPTS_MASK_REG_E;

    if(devObjPtr->numOfPipes > 2)
    {
        /* needed in Falcon to identify the 'TileId' */
        bufferId->pipeId = smemGetCurrentPipeId(devObjPtr);
    }

    /* needed in Falcon to identify the 'mgUnitId' */
    bufferId->mgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);

    /* Put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}



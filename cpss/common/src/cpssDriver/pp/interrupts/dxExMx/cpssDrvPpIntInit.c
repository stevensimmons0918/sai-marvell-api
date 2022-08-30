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
* @file cpssDrvPpIntInit.c
*
* @brief Includes driver level interrupts initialization code.
*
* @version   35
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/hardware/prvCpssDrvObj.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsFalcon.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#ifdef CHX_FAMILY
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#endif

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    extern GT_U32 simLogIsOpen(void);
    #include <os/simTypesBind.h>
    #define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #include <os/simOsBindOwn.h>
    #undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#endif /*ASIC_SIMULATION*/

#ifdef CHX_FAMILY
    extern GT_VOID prvCpssFalconRavenMemoryAddressSkipCheck
    (
        IN GT_U8                   devNum,
        IN GT_U32                  address,
        IN GT_BOOL                 *skipUnitPtr
    );
#endif

/*******************************************************************************
 * Forward functions declarations
 ******************************************************************************/
#define MAX_LEVELS_CNS  16
/**
* @internal drvInterruptsEnable function
* @endinternal
*
* @brief   Enable interrupts for the given device.
*
* @param[in] devNum                   - The device number to initialize the interrupts mechanism
*                                      for.
* @param[in] portGroupId              - the portGroupId
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
*/
static GT_STATUS drvInterruptsEnable
(
    IN  GT_U8   devNum,
    IN  GT_U32  portGroupId
)
{
    GT_STATUS   retVal;     /* Function's return value. */

    PRV_CPSS_DRV_CHECK_GEN_FUNC_OBJ_MAC(devNum);

    retVal = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_OBJ_CONFIG[devNum]->genPtr->drvHwPpInitInMasks(devNum,portGroupId,PRV_CPSS_DRV_INT_INIT_PHASE1_E);

    return retVal;
}


/**
* @internal drvInterruptsEnableFakeMaskSet function
* @endinternal
*
* @brief   Fake interrupts do not need unmasking, just return TRUE.
*
* @param[in] evReqNode                - The interrupt node representing the interrupt to be
*                                      unmasked.
*
* @retval GT_OK - on success,
*/
static GT_STATUS drvInterruptsEnableFakeMaskSet
(
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC *evReqNode,
    IN CPSS_EVENT_MASK_SET_ENT      operation
)
{
    /* fix warnings */
    evReqNode = evReqNode;
    operation = operation;

    return GT_OK;
}

/**
* @internal onEmulator_IntNonExistsUnitsSkip function
* @endinternal
*
* @brief   Set skip on registers that on non-existing units.
*         needed by 'EMULATOR' that not supports all the units
* @param[in] devNum                   - The device number
* @param[in,out] maskRegMapArr        - in : array of the default register addresses
*                                       out: updated array of the default register addresses ,
*                                            that 'removed' registers that are in not
*                                            supported units.
* @param[in] numMaskReg               - number of default register addresses (in maskRegMapArr[])
*
* @note 'removed' address is stated as 'CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS'
*
*/
static void onEmulator_IntNonExistsUnitsSkip
(
    IN  GT_U8  devNum,
    INOUT  GT_U32  *maskRegMapArr,
    IN  GT_U32  numMaskReg
)
{
    GT_U32 ii;
    GT_BOOL unitUsed = GT_TRUE;

    for (ii=0; ii < numMaskReg; ii++)
    {
        if(maskRegMapArr[ii] == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* The address is already with 'skip' value */
            /* avoid the CPSS 'ERROR LOG' about this address*/
            continue;

        }

        unitUsed = prvCpssOnEmulatorSupportedAddrCheck(devNum,0,maskRegMapArr[ii]);
        if(GT_FALSE == unitUsed)
        {
            maskRegMapArr[ii] = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;
        }
    }
    return;
}
#if defined GM_USED && defined CHX_FAMILY
/**
* @internal onGm_IntNonExistsUnitsSkip function
* @endinternal
*
* @brief   skip on registers that not supported in : ProjectFunctions::InitAddrRanges() in
*           file : GM_simulation\gm\verifier\HawkVrf\interunit\source\ProjectFunctions.cpp
*
* @param[in] devNum                   - The device number
* @param[in,out] maskRegMapArr        - in : array of the default register addresses
*                                       out: updated array of the default register addresses ,
*                                            that 'removed' registers that are in not
*                                            supported units.
* @param[in] numMaskReg               - number of default register addresses (in maskRegMapArr[])
*
* @note 'removed' address is stated as 'CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS'
*
*/
static void onGm_IntNonExistsUnitsSkip
(
    IN  GT_U8  devNum,
    INOUT  GT_U32  *maskRegMapArr,
    IN  GT_U32  numMaskReg
)
{
    GT_U32 ii;
    PRV_CPSS_DXCH_UNIT_ENT unitId;

    for (ii=0; ii < numMaskReg; ii++)
    {
        if(maskRegMapArr[ii] == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* The address is already with 'skip' value */
            /* avoid the CPSS 'ERROR LOG' about this address*/
            continue;

        }

        unitId = prvCpssDxChHwRegAddrToUnitIdConvert(devNum, maskRegMapArr[ii]);

        switch(unitId)
        {
            /* supported units */
            case PRV_CPSS_DXCH_UNIT_IPLR_E:
            case PRV_CPSS_DXCH_UNIT_EPLR_E:
            case PRV_CPSS_DXCH_UNIT_IOAM_E:
            case PRV_CPSS_DXCH_UNIT_EOAM_E:
            case PRV_CPSS_DXCH_UNIT_ERMRK_E:
            case PRV_CPSS_DXCH_UNIT_EM_E:
            case PRV_CPSS_DXCH_UNIT_FDB_E:
            case PRV_CPSS_DXCH_UNIT_CNC_0_E:
            case PRV_CPSS_DXCH_UNIT_CNC_1_E:
            case PRV_CPSS_DXCH_UNIT_CNC_2_E:
            case PRV_CPSS_DXCH_UNIT_CNC_3_E:
            case PRV_CPSS_DXCH_UNIT_L2I_E:
            case PRV_CPSS_DXCH_UNIT_MG_E:
            case PRV_CPSS_DXCH_UNIT_MG_0_1_E:
            case PRV_CPSS_DXCH_UNIT_MG_0_2_E:
            case PRV_CPSS_DXCH_UNIT_MG_0_3_E:
                break;
            default:/* the unit not supported in ProjectFunctions::InitAddrRanges() to send message like
                    TRK_MG_2_PLR_REG,       TRK_PLR_2_MG_REG: */
                /* the register need to be skipped */
                maskRegMapArr[ii] = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;
                break;
        }
    }
    return;
}
#endif /*defined GM_USED && defined CHX_FAMILY*/

#ifdef CHX_FAMILY
/**
* @brief   Set skip on registers that on non-existing units.
*         needed by Falcon 2T/4T/3.2T that not supports all the
*         Ravens
* @param[in] devNum                   - The device number
* @param[in,out] maskRegMapArr        - in : array of the default register addresses
*                                       out: updated array of the default register addresses ,
*                                            that 'removed' registers that are in not
*                                            supported units.
* @param[in] numMaskReg               - number of default register addresses (in maskRegMapArr[])
* @param[in,out] maskRegistersDefaultValuesArray        - in : array of the default values for mask registers
*                                                         out: array of the updated default values for mask registers
*
* @note 'removed' address is stated as 'CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS'
*
*/
static GT_VOID onFalcon_2T4T_IntNonExistsUnitsSkip
(
    IN      GT_U8  devNum,
    INOUT   GT_U32 *maskRegMapArr,
    IN      GT_U32 maskRegNums,
    INOUT   GT_U32 *maskRegistersDefaultValuesArray
)
{

    GT_U32  ii;
    GT_BOOL skipUnit = GT_FALSE;

    for (ii = 0; ii < maskRegNums; ii++)
    {
        if(maskRegMapArr[ii] == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* The address is already with 'skip' value */
            /* avoid the CPSS 'ERROR LOG' about this address*/
            continue;

        }

        prvCpssFalconRavenMemoryAddressSkipCheck(devNum, maskRegMapArr[ii], &skipUnit);

        if (skipUnit == GT_TRUE)
        {
            maskRegMapArr[ii] = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;
        }

        /* filter out not existing ravanes from tree by masking summary bits */
        if (maskRegMapArr[ii] == 0x1D0003FC)
        {
            maskRegistersDefaultValuesArray[ii] &= ((0xFFFFFF00 | PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->activeRavensBitmap) << 1);
        }
    }
}

/**
* @brief   Set skip on registers that on non-existing units.
*         needed by Falcon 8T that not supports all the
*         Ravens
* @param[in] devNum                   - The device number
* @param[in,out] maskRegMapArr        - in : array of the default register addresses
*                                       out: updated array of the default register addresses ,
*                                            that 'removed' registers that are in not
*                                            supported units.
* @param[in] numMaskReg               - number of default register addresses (in maskRegMapArr[])
* @param[in,out] maskRegistersDefaultValuesArray        - in : array of the default values for mask registers
*                                                         out: array of the updated default values for mask registers
*
* @note 'removed' address is stated as 'CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS'
*
*/
static GT_VOID onFalcon_8T_IntNonExistsUnitsSkip
(
    IN      GT_U8  devNum,
    INOUT   GT_U32 *maskRegMapArr,
    IN      GT_U32 maskRegNums,
    INOUT   GT_U32 *maskRegistersDefaultValuesArray
)
{

    GT_U32  ii;
    GT_BOOL skipUnit = GT_FALSE;

    for (ii = 0; ii < maskRegNums; ii++)
    {
        if(maskRegMapArr[ii] == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* The address is already with 'skip' value */
            /* avoid the CPSS 'ERROR LOG' about this address*/
            continue;

        }

        prvCpssFalconRavenMemoryAddressSkipCheck(devNum, maskRegMapArr[ii], &skipUnit);

        if (skipUnit == GT_TRUE)
        {
            maskRegMapArr[ii] = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;
        }

        /* filter out not existing ravens from tree by masking summary bits */
        if (maskRegMapArr[ii] == 0x1D0003FC)
        {
            maskRegistersDefaultValuesArray[ii] &= ((0xFFFFFF00 | PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->activeRavensBitmap) << 1);
        }

        if (maskRegMapArr[ii] == 0x5D000650)
        {
            maskRegistersDefaultValuesArray[ii] &= ((0xFFFFFF00 | (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->activeRavensBitmap >> 8) ) << 1);
        }


    }
}

#endif

/**
* @internal drvInterruptsInfoGet function
* @endinternal
*
* @brief   get pointer to the interrupt tree info according to the device type
*
* @param[in] devNum                   - The device number to get the interrupts tree info.
* @param[in] portGroupId              - The port group Id. relevant only to multi port group devices
*
* @param[out] devIntPtrPtr             - (pointer to)pointer to the interrupts tree of the
*                                      device
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_SUPPORTED         - device not supported
*/
static GT_STATUS drvInterruptsInfoGet
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    OUT PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC **devIntPtrPtr
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devIntPtr = NULL;

    switch (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            /* the xCat3 devices */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[
                                            PRV_CPSS_DRV_FAMILY_DXCH_XCAT3_E]);
            break;

        case CPSS_PP_FAMILY_DXCH_AC5_E:
            /* the AlleyCat5 devices */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[
                                            PRV_CPSS_DRV_FAMILY_DXCH_AC5_E]);
            break;

        case CPSS_PP_FAMILY_DXCH_LION2_E:
            if(portGroupId >= 8)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E + portGroupId]);

            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
#ifdef GM_USED  /* currently keep as was initially in cpss --> with lion2 tree */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E]);
#else
            if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                /* bobcat2-bobk info */
                switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devType)
                {
                    case CPSS_BOBK_CETUS_DEVICES_CASES_MAC:
                        devIntPtr =
                            &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_BOBK_CETUS_E]);
                        break;
                    default:
                        devIntPtr =
                            &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_BOBK_CAELUM_E]);
                        break;
                }
            }
            else
            {
                /* bobcat2 info */
                devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT2_E]);
            }
#endif
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
#ifdef GM_USED  /* currently keep as was initially in cpss --> with lion2 tree */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E]);
#else /*!GM_USED*/
            /* bobcat3 info */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_BOBCAT3_E]);
#endif /*!GM_USED*/
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
#ifdef GM_USED  /* currently keep as was initially in cpss --> with lion2 tree */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E]);
#else /*!GM_USED*/
            /* bobcat3 info */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN2_E]);
#endif /*!GM_USED*/
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
#ifdef GM_USED  /* currently keep as was initially in cpss --> with lion2 tree */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E]);
#else
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_ALDRIN_E]);
#endif
            break;
        case CPSS_PX_FAMILY_PIPE_E:
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_PX_PIPE_E]);
            break;

        case CPSS_PP_FAMILY_DXCH_FALCON_E:
#ifdef GM_USED  /* currently keep as was initially in cpss --> with lion2 tree */
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_LION2_PORT_GROUP_ID_0_E]);
#else /*!GM_USED*/
            /* Falcon info */
            switch(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->numOfTiles)
            {
                case 1:
                    devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_FALCON_1_TILE_E]);
                    break;
                case 2:
                    devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_FALCON_2_TILES_E]);
                    break;
                case 4:
                    devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_FALCON_4_TILES_E]);
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "Bad number of tiles[%d] (supported 1/2/4)",
                        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->numOfTiles);
            }

#endif /*!GM_USED*/
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5P_E]);
            break;

        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_AC5X_E]);
            break;

        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_HARRIER_E]);
            break;

        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            devIntPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_IRONMAN_L_E]);
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* sanity check on the info */
    if(devIntPtr                                  == NULL ||
       devIntPtr->drvIntCauseToUniEvConvertFunc   == NULL ||
       devIntPtr->maskRegistersAddressesArray     == NULL ||
       devIntPtr->maskRegistersDefaultValuesArray == NULL ||
       devIntPtr->interruptsScanArray             == NULL ||
       devIntPtr->numOfInterrupts                 == 0
       )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    *devIntPtrPtr = devIntPtr;

    return GT_OK;
}

#define EMULATE_INTERRUPTS
#ifdef EMULATE_INTERRUPTS

#define MAX_INTERRUPT 64

/* function based on : NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set() */
void DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime)
{
    if(new_milisec_pollingSleepTime == 0)
    {
        /* restore 'default' */
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(pollingSleepTime, 20);
    }
    else
    {
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(pollingSleepTime, new_milisec_pollingSleepTime);
    }

    return;
}
/*******************************************************************************
* intTaskPoll
*
* DESCRIPTION:
*       Interrupt handler task (polling mode).
*
* INPUTS:
*       paramsPtr - index in intArr[]
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
static unsigned __TASKCONV intTaskPoll(GT_VOID *paramsPtr)
{
    PRV_CPSS_DRV_INT_TASK_PARAMS_STC *p = &(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(intArr)[(GT_U32)((GT_UINTPTR)paramsPtr)]);

    /* code like done in interruptMainSr(...) */
#ifdef ASIC_SIMULATION
    {
        if(simLogIsOpen())
        {
            /* state that this task is the 'ISR' for the simulation logger */
            simOsTaskOwnTaskPurposeSet(SIM_OS_TASK_PURPOSE_TYPE_CPU_ISR_E,NULL);
        }
    }
#endif /*ASIC_SIMULATION*/

    while (1)
    {
        GT_U32              intSumValue;
        CPSS_EVENT_ISR_FUNC routine;
        void*               param;
        GT_STATUS           rc;

        routine = p->routine;
        param = p->param;
        if (p->enabled == GT_TRUE && p->drv)
        {
            /* check interrupt summary register */
            rc = p->drv->read(p->drv, p->intSumAs, p->intSumReg, &intSumValue, 1);
            if (rc == GT_OK && intSumValue != 0)
            {
                /* start protect the 'events DB' like done in interruptMainSr(...) */
                PRV_CPSS_INT_SCAN_LOCK();
                routine(param); /* calling to prvCpssDrvInterruptPpSR(...) */
                /* stop protect the 'events DB' like done in interruptMainSr(...) */
                PRV_CPSS_INT_SCAN_UNLOCK();
            }
            cpssOsTimerWkAfter(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(pollingSleepTime));
        } else {
            /* p->enabled != GT_TRUE */
            cpssOsTimerWkAfter(50);
        }
        /*TODO intLOCK/intUNLOCK ??? */
    }
    return 0;
}
#endif /* EMULATE_INTERRUPTS */

/**
* @internal prvCpssDrvPpMuliPortGroupIndicationCheckAndConvert function
* @endinternal
*
* @brief   for multi-port Group device
*         check if current uni-event need Convert due to multi-port group indication
* @param[in] portGroupId              - The port group Id.
* @param[in] uniEv                    - unified event
* @param[in,out] extDataPtr               - (pointer to)The event extended data.
* @param[in,out] extDataPtr               - (pointer to)The event extended data.
*                                       none
*/
void prvCpssDrvPpMuliPortGroupIndicationCheckAndConvert(
    IN      GT_U32                    portGroupId,
    IN      GT_U32                    uniEv,
    INOUT   GT_U32                    *extDataPtr
)
{
    switch(uniEv)
    {
        /* GPP */
        case CPSS_PP_GPP_E:
        /*CNC*/
        case CPSS_PP_CNC_WRAPAROUND_BLOCK_E:
        case CPSS_PP_CNC_DUMP_FINISHED_E:
        /*Policer*/
        case CPSS_PP_POLICER_DATA_ERR_E:
        case CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E:
        case CPSS_PP_POLICER_IPFIX_ALARM_E:
        case CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E:
        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
            break;
        default:
            return;
    }

    /* convert for those events */
    *extDataPtr += CPSS_PP_PORT_GROUP_ID_TO_EXT_DATA_CONVERT_MAC(portGroupId);

    return;
}

/**
* @internal prvCpssDrvPpPortGroupIntCauseToUniEvInit function
* @endinternal
*
* @brief   Init unified event type and data for interrupts.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The port group Id.
* @param[in] intNum                   - number of interrupts in DB
* @param[out] intReqNodeListPtr        - (pointer to) Interrupts DB.
*
* @retval GT_FAIL             - on error
* @retval GT_OK               - on success
*/
static GT_STATUS prvCpssDrvPpPortGroupIntCauseToUniEvInit
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portGroupId,
    IN  GT_U32                      intNum,
    OUT PRV_CPSS_DRV_EV_REQ_NODE_STC    *intReqNodeListPtr
)
{
    GT_U32  ii;               /* iterator                         */
    GT_U32  tmpExtData;       /* temporary value of extended data */
    GT_U32  numReservedPorts; /* number of reserved ports */
    GT_U32  intCauseIndex;    /* index in interrupts   DB */
    GT_U32  uniEvCause;       /* value of unified event   */
    GT_U32  extData;          /* value of extended data   */
    const GT_U32                         *tableWithExtDataPtr;    /* table with extended data */
    GT_U32                               tableWithExtDataSize;    /* size of table with extended data */
    const PRV_CPSS_DRV_EVENT_INFO_TYPE   *tableWithoutExtDataPtr; /* table without extended data */
    GT_U32                               tableWithoutExtDataSize; /* size of table without extended data */

    /* get the tables */
    tableWithExtDataPtr     = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataArr;
    tableWithExtDataSize    = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableWithExtDataSize;
    tableWithoutExtDataPtr  = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableArr;
    tableWithoutExtDataSize = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.uniEvMapTableSize;
    numReservedPorts        = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numReservedPorts;

    if(tableWithoutExtDataPtr == NULL ||
       tableWithExtDataPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* scan the map table for unified event without extended data */
    for (ii = 0; ii < tableWithoutExtDataSize; ii++)
    {
        intCauseIndex = tableWithoutExtDataPtr[ii][1];

        /* skip marked and out of range interrupts.
           some devices like Falcon 12.8 and 6.4 share same DB for conversion.
           Therefore need to skip out of range interrupts.*/
        if((intCauseIndex >= intNum) ||
           (intReqNodeListPtr[intCauseIndex].intMaskReg == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS))
        {
            continue;
        }

        extData = 0;

        uniEvCause = tableWithoutExtDataPtr[ii][0];
        prvCpssDrvPpMuliPortGroupIndicationCheckAndConvert(portGroupId,uniEvCause,&extData);
        if(CPSS_PP_PORT_802_3_AP_E == uniEvCause)
        {
            uniEvCause += portGroupId;
        }

        intReqNodeListPtr[intCauseIndex].uniEvCause = uniEvCause;
        intReqNodeListPtr[intCauseIndex].uniEvExt   = extData;
    }

    ii = 0;
    /* scan the map table for unified event with extended data */
    while (ii < tableWithExtDataSize)
    {
        /* remember the uni event */
        uniEvCause = tableWithExtDataPtr[ii++];
        while (tableWithExtDataPtr[ii] != MARK_END_OF_UNI_EV_CNS)
        {
            intCauseIndex = tableWithExtDataPtr[ii];

            /* skip marked and out of range interrupts */
            if((intCauseIndex < intNum) &&
               (intReqNodeListPtr[intCauseIndex].intMaskReg != CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS))
            {
                tmpExtData = tableWithExtDataPtr[ii+1];
                if ((uniEvCause == CPSS_PP_DATA_INTEGRITY_ERROR_E) ||
                    (uniEvCause == CPSS_PP_CRITICAL_HW_ERROR_E) ||
                    (uniEvCause == CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E))
                {
                    /* These unified events use interrupt enum as extData.
                       The CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E use own special codding including usage of MSBs.
                       There is no conversion is used. */
                    extData = tmpExtData;
                }
                else if(uniEvCause == CPSS_PP_PORT_EEE_E)/* patch because 'port indication' not supports '<< 8' */
                {
                    GT_U32  portNum = U32_GET_FIELD_MAC(tmpExtData,8,8);/* from bit 8 take 8 bits */
                    GT_U32  extVal  = U32_GET_FIELD_MAC(tmpExtData,0,8);/* from bit 0 take 8 bits */

                    /* convert the port to 'global port' */
                    /* each port group has it's reserved ports */
                    portNum += (numReservedPorts * portGroupId);

                    /* rebuild the '*extDataPtr' */
                    extData = extVal | (portNum << 8);
                }
                else
                {
                    extData = CLEAR_MARK_INT_MAC(tmpExtData);
                    if (IS_MARKED_PER_PORT_PER_LANE_INT_MAC(tmpExtData))
                    {
                        /* each port group has 16 reserved ports */
                        /* each port has 256 reserved lanes */
                        extData +=
                            (LANE_PORT_TO_EXT_DATA_CONVERT(numReservedPorts,0) * portGroupId);
                    }
                    else if (IS_MARKED_PER_PORT_INT_MAC(tmpExtData))
                    {
                       /* each port group has 16 reserved ports */
                        extData += (numReservedPorts * portGroupId);
                    }
                    else if (IS_MARKED_PER_HEM_INT_MAC(tmpExtData))
                    {
                       /* each hemisphere group has 64 reserved ports */
                        extData += (portGroupId >= 4) ? 64 : 0;
                    }
                }

                prvCpssDrvPpMuliPortGroupIndicationCheckAndConvert(portGroupId,uniEvCause,&extData);

                intReqNodeListPtr[intCauseIndex].uniEvCause = uniEvCause;
                intReqNodeListPtr[intCauseIndex].uniEvExt   = extData;
            }

            ii +=2;
        }
        ii++;
    }

    return GT_OK;
}

/**
* @internal drvInterruptsInit function
* @endinternal
*
* @brief   Initialize the interrupts mechanism for a given device.
*
* @param[in] devNum                   - The device number to initialize the interrupts
*                                      mechanism for.
* @param[in] portGroupId              - The port group Id.
* @param[in] intVecNum                - The interrupt vector number this device is connected
*                                      to.
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_SUPPORTED         - device not supported
*/
static GT_STATUS drvInterruptsInit
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId,
    IN  GT_U32          intVecNum,
    IN  GT_U32          intMask
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intReqNodeListPtr;/* event node array      */
    PRV_CPSS_DRV_EV_REQ_NODE_STC *intReqNodePtr = NULL;/* event node pointer */
    GT_U32                  scanArrIdx;
    GT_STATUS               retVal;             /* Function's return values.*/
    GT_U32                  i,jj;               /* Loops indexes.           */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devIntPtr = NULL;
        CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

        retVal = cpssSystemRecoveryStateGet(&system_recovery);
        if (retVal != GT_OK)
        {
          return retVal;
        }

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    /* Get the devices interrupts initialization parameters,    */
    /* according to the device's type.                          */
    retVal = drvInterruptsInfoGet(devNum,portGroupId,&devIntPtr);
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"drvInterruptsInfoGet failed");
    }

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* Set skip on registers that on non-existing units.
           needed by 'EMULATOR' that  not supports all the units */
        onEmulator_IntNonExistsUnitsSkip(
            devNum,
            devIntPtr->maskRegistersAddressesArray,
            devIntPtr->numOfInterrupts/32/*number of registers in maskRegistersAddressesArray*/);
    }
#if defined GM_USED && defined CHX_FAMILY
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily >= CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        /* Set skip on registers that not supported in : ProjectFunctions::InitAddrRanges() in
           file : GM_simulation\gm\verifier\HawkVrf\interunit\source\ProjectFunctions.cpp */
        onGm_IntNonExistsUnitsSkip(
            devNum,
            devIntPtr->maskRegistersAddressesArray,
            devIntPtr->numOfInterrupts/32/*number of registers in maskRegistersAddressesArray*/);
    }
#endif /*defined GM_USED && defined CHX_FAMILY*/

#ifdef CHX_FAMILY
    switch (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devType)
    {
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
        case CPSS_FALCON_REDUCED_2T_4T_3_2T_DEVICES_CASES_MAC:
            onFalcon_2T4T_IntNonExistsUnitsSkip(
                devNum,
                devIntPtr->maskRegistersAddressesArray,
                devIntPtr->numOfInterrupts/32/*number of registers in maskRegistersAddressesArray*/,
                devIntPtr->maskRegistersDefaultValuesArray);
            break;
        case CPSS_FALCON_8T_DEVICES_CASES_MAC:
            onFalcon_8T_IntNonExistsUnitsSkip(
                devNum,
                devIntPtr->maskRegistersAddressesArray,
                devIntPtr->numOfInterrupts/32/*number of registers in maskRegistersAddressesArray*/,
                devIntPtr->maskRegistersDefaultValuesArray);
            break;
    }
#endif /*defined CHX_FAMILY*/

    /* Allocate place for the interrupts nodes pool array.  */
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool =
        cpssOsMalloc(devIntPtr->numOfInterrupts * sizeof(PRV_CPSS_DRV_EV_REQ_NODE_STC));
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    intReqNodeListPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;

    /* bind in the PP Interrupt Service Routine */
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.isrFuncPtr = prvCpssDrvInterruptPpSR;

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numOfIntBits  = devIntPtr->numOfInterrupts;

    /* Initialize the Interrupts hierarchy tree.    */
    scanArrIdx = 0;
    retVal = prvCpssDrvInterruptScanInit(
                devIntPtr->numOfScanElements,
                devIntPtr->interruptsScanArray,
                &scanArrIdx,
                &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intScanRoot));
    if(retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"prvCpssDrvInterruptScanInit failed");
    }

    /* check numOfInterrupts%32 != 0 */
    if((devIntPtr->numOfInterrupts & 0x1f))
    {
        /* the enumeration of interrupts for this device has error */
        /* the numOfInterrupts number must be divided by 32 with no left over */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);
    }

    /* in polling mode, only summary interrupts are unmasked by default */
    /* according to the interrupt scan tree. */
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.needInterruptMaskDefaultValueInit == GT_TRUE)
    {
        retVal = prvCpssDrvInterruptsMaskDefSummaryInit(
                            devIntPtr->numOfInterrupts >> 5,
                            devIntPtr->maskRegistersAddressesArray,
                            devIntPtr->maskRegistersDefaultValuesArray,
                            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intScanRoot);

        if (GT_OK != retVal)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"prvCpssDrvInterruptsMaskDefSummaryInit failed");
        }
    }

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow = cpssOsMalloc(sizeof(GT_U32)* (devIntPtr->numOfInterrupts>> 5));
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase = cpssOsMalloc(sizeof(GT_BOOL)* (devIntPtr->numOfInterrupts>> 5));
    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow == NULL ||
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < devIntPtr->numOfInterrupts ; i++)
    {
        intReqNodePtr = &intReqNodeListPtr[i];

        intReqNodePtr->devNum           = devNum;
        intReqNodePtr->portGroupId           = portGroupId;
        intReqNodePtr->nextPtr          = NULL;
        intReqNodePtr->prevPtr          = NULL;
        intReqNodePtr->intStatus        = PRV_CPSS_DRV_EV_DRVN_INT_ACTIVE_E;
        intReqNodePtr->intRecStatus     = PRV_CPSS_DRV_EV_DRVN_INT_IDLE_E;
        if(PRV_INTERRUPT_CTRL_GET(devNum).intMaskSetFptr == NULL)
        {
            intReqNodePtr->intMaskSetFptr   = prvCpssDrvInterruptMaskSet;
        }
        else
        {
            intReqNodePtr->intMaskSetFptr   = PRV_INTERRUPT_CTRL_GET(devNum).intMaskSetFptr;
        }
        intReqNodePtr->intCbFuncPtr     = NULL;
        intReqNodePtr->intCause         = i;
        intReqNodePtr->intMaskReg       = devIntPtr->maskRegistersAddressesArray[i >> 5];
        intReqNodePtr->intBitMask       = 1 << (i & 0x1F);
#ifdef SHARED_MEMORY
        intReqNodePtr->fake             = GT_FALSE;
#endif
        /* assign default values to be "not used" for events notification.
           following prvCpssDrvPpPortGroupIntCauseToUniEvInit will assign
           value for used interrupts. */
        intReqNodePtr->uniEvCause = CPSS_UNI_RSRVD_EVENT_E;
        intReqNodePtr->uniEvExt   = 0;

        /* skip marked interrupts */
        if(intReqNodePtr->intMaskReg != CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* init fake interrupts */
            if(devIntPtr->hasFakeInterrupts == GT_TRUE &&
               i >= devIntPtr->firstFakeInterruptId)
            {
                if(i >= devIntPtr->firstFakeInterruptId)
                {
                    intReqNodePtr->intMaskSetFptr = drvInterruptsEnableFakeMaskSet;
#ifdef SHARED_MEMORY
                    intReqNodePtr->fake = GT_TRUE;
#endif
                }
            }
        }

        /* Set the interrupt mask registers to their default values */
        if((i % 32) == 0)
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intMaskShadow[i / 32] =
                devIntPtr->maskRegistersDefaultValuesArray[i / 32];
            /* Set the device's registers that may not be accessed  */
            /* before start init.                                   */

            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase[i / 32] = PRV_CPSS_DRV_INT_INIT_PHASE1_E;
            /* get the not accessible before 'Start Init' */
            for(jj = 0 ; jj < devIntPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit; jj++)
            {
                if(devIntPtr->maskRegistersAddressesArray[i / 32] == devIntPtr->notAccessibleBeforeStartInitPtr[jj])
                {
                    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase[i / 32] = PRV_CPSS_DRV_INT_INIT_PHASE2_E;
                    break;
                }
            }
        }
    }

    /* fill-in unified events and extended data in interrupts DB */
    retVal = prvCpssDrvPpPortGroupIntCauseToUniEvInit(devNum,portGroupId,devIntPtr->numOfInterrupts, intReqNodeListPtr);
    if (retVal != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"prvCpssDrvPpPortGroupIntCauseToUniEvInit failed");
    }

    /* get the not accessible before 'MACsec Init' register ids */
    if(devIntPtr->macSecRegIdArray != NULL)
    {
        jj = 0;
        while(devIntPtr->macSecRegIdArray[jj] != PRV_CPSS_MACSEC_REGID_LAST_CNS)
        {
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.initPhase[devIntPtr->macSecRegIdArray[jj]] = PRV_CPSS_DRV_INT_INIT_PHASE_MACSEC_E;
            jj++;
        }
    }

    /* bind the ISR callback routine done before signalling app. */
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.bindTheIsrCbRoutineToFdbTrigEnded == GT_TRUE)
    {
        /* set this specific call back at the end of the events loop */
        intReqNodeListPtr[devIntPtr->fdbTrigEndedId].intCbFuncPtr = devIntPtr->fdbTrigEndedCbPtr;
    }

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)
    {
        intVecNum = CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS;
    }
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intVecNum = intVecNum;

    /* Initialize the event queue */
    if (prvCpssDrvEvReqQInit(CPSS_UNI_EVENT_COUNT_E) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

#ifdef EMULATE_INTERRUPTS
    if (intVecNum == CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS)
    {
        GT_U32 i;
        if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].isrConnectionId == 0)
        {
            GT_CHAR taskName[40];
            CPSS_TASK tid;
            if (devNum >= MAX_INTERRUPT)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId] == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            /* Use devNum as Index instead of Incremental Integer as Index */
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].enabled, GT_FALSE);
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].routine, PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.isrFuncPtr);
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].param, &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].isrCookieInfo));
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].drv, PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->drivers[portGroupId]);
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].intSumAs, 3);/*TODO*/
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].intSumReg, 0x30); /* TODO */
#ifdef CHX_FAMILY
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E)
            {
                PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].intSumReg, (0x100 + prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E, NULL)));
                DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(1000);
            }
            else
            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_EAGLE_E)
            {
                PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(intArr[devNum].intSumReg) += prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_MG_E, NULL);
            }
#endif
            cpssOsSprintf(taskName, "intTask%s", PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(intArr[devNum].drv->name));
            i = devNum;
            retVal = cpssOsTaskCreate(taskName, 0/*prio*/, 0x4000/*stk size*/,
                intTaskPoll, (GT_VOID_PTR)((GT_UINTPTR)i), &tid);
            if (retVal != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"cpssOsTaskCreate failed");

            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].isrConnectionId = tid;
        }
        else
        {
            if (PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(intArr[devNum].param) != &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].isrCookieInfo))
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            /* enable interrupts in accordance with interrupt mask */
            PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].enabled, GT_TRUE);
        }
    }
    else
    {
#endif
    /* connect the interrupts    */
    retVal =
        prvCpssDrvInterruptConnect(intVecNum,intMask,
                           PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.isrFuncPtr,
                           &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].isrCookieInfo),
                           &(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].isrConnectionId));
    if (GT_OK != retVal)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"prvCpssDrvInterruptConnect failed");
    }

    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /* enable interrupts in accordance with interrupt mask */
        retVal =  prvCpssDrvInterruptEnable(intMask);
        if (GT_OK != retVal)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"prvCpssDrvInterruptEnable failed");
        }
    }
#ifdef EMULATE_INTERRUPTS
    }
#endif
    if (!((system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
             (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) )
    {
        /* Enable the interrupts in packet processor */
        retVal =  drvInterruptsEnable(devNum,portGroupId);
        if (GT_OK != retVal)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal,"drvInterruptsEnable failed");
        }
    }

    return retVal;
}


/**
* @internal prvCpssDrvDxExMxIntInitObject function
* @endinternal
*
* @brief   This function creates and initializes DxExMx device driver object
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - if the driver object have been created before
*/
GT_STATUS prvCpssDrvDxExMxIntInitObject
(
     void
)
{
    /* driver object initialization */
    prvCpssDrvGenExMxDxObjPtr->drvHwPpInterruptInit = drvInterruptsInit;

    return GT_OK;
}

GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);
GT_STATUS prvCpssGenericSrvCpuRegisterWriteCpuId
(
    IN GT_U8    devNum,
    IN GT_U8    cpuIdx,
    IN GT_U32   portGroupId GT_UNUSED,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

/**
* @internal prvCpssDrvExMxDxHwPpInterruptsMaskToDefault function
* @endinternal
*
* @brief   Initialize interrupts mask to default for a given device.
*         (all interrupts bits are masked except of summary)
* @param[in] devNum                   - The device number to set interrupts masks for.
* @param[in] portGroupId              - The port group Id.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_SUPPORTED         - device not supported
*/
GT_STATUS prvCpssDrvExMxDxHwPpInterruptsMaskToDefault
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId
)
{
    GT_U32 i;
    GT_STATUS rc = GT_OK;
    GT_U32  basRegAddr = 0x88c005c;
    GT_U32  regAddr = 0;
    GT_U32  causeBitIdx = 3;
    GT_U32  intMask = 0;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devIntPtr = NULL;
    /* Get the devices interrupts initialization parameters,    */
    /* according to the device's type.                          */
    rc = drvInterruptsInfoGet(devNum,portGroupId,&devIntPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    for(i = 0; i < (devIntPtr->numOfInterrupts >> 5); i++)
    {
        if(devIntPtr->maskRegistersAddressesArray[i] == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            continue;
        }

        if (((devIntPtr->maskRegistersAddressesArray[i] & 0x00FFFFFF) >= 0x00000010) &&
                ((devIntPtr->maskRegistersAddressesArray[i] & 0x00FFFFFF) <= 0x0000001f ))
        {
            prvCpssGenericSrvCpuRegisterWrite(devNum,portGroupId,
                    devIntPtr->maskRegistersAddressesArray[i],
                    devIntPtr->maskRegistersDefaultValuesArray[i]);
            /* ignore return status */
            rc = GT_OK;
        }
        else
        if(PRV_CPSS_IS_INT_MASK_REG_INTERNAL_PCI_MAC(devNum,devIntPtr->maskRegistersAddressesArray[i]))
        {
            rc = prvCpssDrvHwPpPortGroupWriteInternalPciReg(devNum,
                                                            portGroupId,
                                                            devIntPtr->maskRegistersAddressesArray[i],
                                                            devIntPtr->maskRegistersDefaultValuesArray[i]);
        }
        else if (PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(devIntPtr->maskRegistersAddressesArray[i]))
        {
            rc = prvCpssDrvHwPpMgWriteReg(devNum,
                                          PRV_CPSS_DRV_IS_MGS_INDICATION_IN_ADDR_MAC(devIntPtr->maskRegistersAddressesArray[i]),
                                          PRV_CPSS_MGS_REG_MASK_MAC(devIntPtr->maskRegistersAddressesArray[i]),
                                          devIntPtr->maskRegistersDefaultValuesArray[i]);
        }
        else
        {
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                                                      portGroupId,
                                                      devIntPtr->maskRegistersAddressesArray[i],
                                                      devIntPtr->maskRegistersDefaultValuesArray[i]);
        }
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        for (i = 0; i < 15; i++)
        {
            if(i == 13)
            {
                continue;
            }
            regAddr = basRegAddr + i*0x1000;
            /* read mask from register */
            rc = prvCpssDrvHwPpPortGroupReadRegister(devNum,portGroupId,regAddr,&intMask);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* mask bit3 HGL port interrupts*/
            intMask &= (~(1 << causeBitIdx));
            /* write updated mask */
            rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum,
                                                      portGroupId,
                                                      regAddr,
                                                      intMask);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}


/**
* @internal prvCpssDrvExMxDxHwPpInterruptsTreeGet function
* @endinternal
*
* @brief   function return :
*         1. the root to the interrupts tree info of the specific device
*         2. the interrupt registers that can't be accesses before 'Start Init'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group Id. relevant only to multi port group devices
*
* @param[out] numOfElementsPtr         - (pointer to) number of elements in the tree.
* @param[out] treeRootPtrPtr           - (pointer to) pointer to root of the interrupts tree info.
* @param[out] numOfInterruptRegistersNotAccessibleBeforeStartInitPtr - (pointer to)
*                                      number of interrupt registers that can't be accessed
*                                      before 'Start init'
* @param[out] notAccessibleBeforeStartInitPtrPtr (pointer to)pointer to the interrupt
*                                      registers that can't be accessed before 'Start init'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - the driver was not initialized for the device
*/
GT_STATUS   prvCpssDrvExMxDxHwPpInterruptsTreeGet
(
    IN GT_U8    devNum,
    IN  GT_U32          portGroupId,
    OUT GT_U32  *numOfElementsPtr,
    OUT const CPSS_INTERRUPT_SCAN_STC        **treeRootPtrPtr,
    OUT GT_U32  *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    OUT GT_U32  **notAccessibleBeforeStartInitPtrPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devIntPtr;/*pointer to interrupts info*/

    rc = drvInterruptsInfoGet(devNum,portGroupId,&devIntPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    *numOfElementsPtr = devIntPtr->numOfScanElements;/* number of elements in interruptsScanArray */
    *treeRootPtrPtr = devIntPtr->interruptsScanArray;/*attach to pointer of interrupts tree*/
    *numOfInterruptRegistersNotAccessibleBeforeStartInitPtr =devIntPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit;
    *notAccessibleBeforeStartInitPtrPtr = devIntPtr->notAccessibleBeforeStartInitPtr;

    return GT_OK;
}
/* macro to skip non valid entries that starts from specific index */
#define INDEX_AFTER_SKIPPED_ENTRIES_MAC(/*IN*/_startIndex,/*IN*/_rangeSize,/*OUT*/_indexAfterSkippedEntries)    \
{                                                                                                               \
    _indexAfterSkippedEntries = _startIndex;                                                                    \
    while(intrScanArr[_indexAfterSkippedEntries].maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)              \
    {                                                                                                           \
        /* the entry should be fully ignored !!! */                                                             \
        _indexAfterSkippedEntries++;                                                                            \
                                                                                                                \
        if((_indexAfterSkippedEntries+1) >= _rangeSize)                                                         \
        {                                                                                                       \
            /* the tree is over */                                                                              \
            break;                                                                                              \
        }                                                                                                       \
    }                                                                                                           \
}

/**
* @internal maskRegInfoGet function
* @endinternal
*
* @brief   function to build from the interrupts tree and the mask registers
*         addresses , the content of the summary bits in those mask registers.
*         1. fill the array of mask registers addresses
*         2. fill the array of default values for the mask registers
*         3. update the bits of nonSumBitMask in the scan tree
* @param[in] devFamily                - device family
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in,out] intrScanArr[]        - in : interrupts scan info array
*                                       out: non summary bits updated
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[out] maskRegDefaultSummaryArr[] - (pointer to) the registers values
* @param[out] maskRegMapArr[]          - (pointer to) the mask registers addresses
* @param[out] ptr1                     - pointer to allocated memory needed to be free by the caller
* @param[out] ptr2                     - pointer to allocated memory needed to be free by the caller
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
static GT_STATUS maskRegInfoGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT devFamily,
    IN  GT_U32  numScanElements,
    INOUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC intrScanArr[],
    IN  GT_U32   numMaskRegisters,
    OUT GT_U32   maskRegDefaultSummaryArr[],
    OUT GT_U32   maskRegMapArr[],
    OUT GT_U32   **ptr1,
    OUT GT_U32   **ptr2

)
{
    GT_STATUS rc = GT_OK;
    GT_U32  ii;
    GT_U32  jj;
    GT_U32  *tmpMaskRegMapArr;
    GT_U32  *tmpMaskRegDefaultSummaryArr;
    GT_U32  enumerationInterruptIndex;
    GT_U32  lastIndexChecked;
    GT_BOOL wasSubTree;
    GT_U32  currentLevel;
    GT_U32  treeNumLeafs[MAX_LEVELS_CNS]={0};
    GT_U32  dummyMaskRegAddr = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;/* register address to be used to fill all the
                                    'empty' indexes in the array of maskRegMapArr */
    GT_U32  pexCauseAddr ,pexMaskAddr;
    GT_U32  indexAfterSkippedEntries;

    rc = prvCpssDrvExMxDxHwPpPexAddrGet(devFamily,&pexCauseAddr,&pexMaskAddr);
    if(rc != GT_OK)
    {
        return rc;
    }

    tmpMaskRegMapArr = cpssOsMalloc(numScanElements * sizeof(GT_U32));
    tmpMaskRegDefaultSummaryArr = cpssOsMalloc(numScanElements * sizeof(GT_U32));

    *ptr1 = tmpMaskRegMapArr;
    *ptr2 = tmpMaskRegDefaultSummaryArr;

    if(tmpMaskRegMapArr == NULL || tmpMaskRegDefaultSummaryArr == NULL)
    {
        /* Error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* fill array tmpMaskRegMapArr[] */
    /* fill array tmpMaskRegDefaultSummaryArr[] */
    /* calculate the intrScanArr[].nonSumBitMask -->
       nonSumBitMask are the non-summary bits --> calculate it between bits of:
       startIdx , endIdx and by the subIntrListLen that hold sub tree of summary
       bits */
    for(ii = 0 ; ii < numScanElements; ii++)
    {
        GT_U32  nonSumBitMask = 0;
        GT_U32  numBitsStartToEnd;
        GT_U32  startBit;

        if(ii < numMaskRegisters)
        {
            maskRegDefaultSummaryArr[ii] = 0;/* init */
            maskRegMapArr[ii] = dummyMaskRegAddr;/* init */
        }

        if(intrScanArr[ii].maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            continue;
        }

        /* replace the place holder with real PEX interrupt mask address */
        if(intrScanArr[ii].maskRegAddr == PRV_CPSS_DRV_SCAN_PEX_MASK_REG_PLACE_HOLDER_CNS)
        {
            intrScanArr[ii].maskRegAddr = pexMaskAddr;
        }

        /* replace the place holder with real PEX interrupt cause address */
        if(intrScanArr[ii].causeRegAddr == PRV_CPSS_DRV_SCAN_PEX_CAUSE_REG_PLACE_HOLDER_CNS)
        {
            intrScanArr[ii].causeRegAddr = pexCauseAddr;
        }

        /*fill array tmpMaskRegMapArr[]*/
        tmpMaskRegMapArr[ii] = intrScanArr[ii].maskRegAddr;

        tmpMaskRegDefaultSummaryArr[ii] = 0;

        /* validation check */
        if((intrScanArr[ii].subIntrListLen + ii) > numScanElements)
        {
            /* Error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(intrScanArr[ii].endIdx < intrScanArr[ii].startIdx)
        {
            /* Error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        numBitsStartToEnd = intrScanArr[ii].endIdx - intrScanArr[ii].startIdx + 1;
        startBit = intrScanArr[ii].startIdx & 0x1F;/* % 32 */

        if(numBitsStartToEnd > 32)
        {
            /* Error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        else if(numBitsStartToEnd == 32)
        {
            nonSumBitMask = 0xFFFFFFFF;
        }
        else
        {
            /* the MACRO of U32_SET_FIELD_MASKED_MAC will cut the value of
               0xFFFFFFFF to set bits only in needed length */
            U32_SET_FIELD_MASKED_MAC(nonSumBitMask ,startBit,numBitsStartToEnd,0xFFFFFFFF);
        }

        lastIndexChecked = ii;
        wasSubTree = GT_FALSE;

        /* loop on the summary bits */
        for(jj = 0 ; jj < intrScanArr[ii].subIntrListLen; jj++)
        {
            GT_U32  bitmapOfBit;

            currentLevel = 0;

            INDEX_AFTER_SKIPPED_ENTRIES_MAC((lastIndexChecked+1) , numScanElements , indexAfterSkippedEntries);

            if((indexAfterSkippedEntries < numScanElements) &&
                wasSubTree == GT_FALSE &&
                intrScanArr[indexAfterSkippedEntries].subIntrListLen != 0)
            {
                /* the next index is going to have sub tree */
                /* so we first need to treat the root of this sub tree */
                lastIndexChecked = indexAfterSkippedEntries;
                wasSubTree = GT_TRUE;

            }
            else
            {
                wasSubTree = GT_FALSE;

                INDEX_AFTER_SKIPPED_ENTRIES_MAC(lastIndexChecked , numScanElements , indexAfterSkippedEntries);
                if(indexAfterSkippedEntries != lastIndexChecked)
                {
                    /* indication of skipped entries ... do the '-1'
                       because the 'do-while' will add '++' */
                    lastIndexChecked = indexAfterSkippedEntries - 1;
                }
                else
                if(lastIndexChecked != ii &&
                   intrScanArr[lastIndexChecked].subIntrListLen != 0)
                {
                    /* in the previous loop we treated the root of a sub tree */
                    /* so now we need to skip the sub tree , for that we must
                       start with the root of the sub tree again */
                    lastIndexChecked--;
                }

                do
                {
                    lastIndexChecked++;

                    if(lastIndexChecked >= numScanElements)
                    {
                        /* the tree is over */
                        break;
                    }

                    INDEX_AFTER_SKIPPED_ENTRIES_MAC(lastIndexChecked , numScanElements , indexAfterSkippedEntries);
                    lastIndexChecked = indexAfterSkippedEntries;

                    if(intrScanArr[lastIndexChecked].subIntrListLen != 0)
                    {
                        /* this one has sub tree */
                        if(treeNumLeafs[currentLevel] == 0)
                        {
                            treeNumLeafs[currentLevel] = intrScanArr[lastIndexChecked].subIntrListLen;
                            currentLevel++;
                            /* set this level sub tree num leafs */
                            if(currentLevel >= MAX_LEVELS_CNS)
                            {
                                /* error in tree or really this device needs more levels ?! */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }
                        }
                        else
                        {


                            /* set next level sub tree num leafs */
                            currentLevel++;
                            /* set this level sub tree num leafs */
                            if(currentLevel >= MAX_LEVELS_CNS)
                            {
                                /* error in tree or really this device needs more levels ?! */
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                            }
                            treeNumLeafs[currentLevel] = intrScanArr[lastIndexChecked].subIntrListLen;
                        }
                        wasSubTree = GT_TRUE;
                    }
                    else if(currentLevel == 0)
                    {
                        if(treeNumLeafs[currentLevel])
                        {
                            treeNumLeafs[currentLevel]--;
                        }
                    }
                    else if(treeNumLeafs[currentLevel] == 0)
                    {
                        currentLevel--;
                        lastIndexChecked--;/* make sure to check again this leaf to
                                              see if it ends more deep branch */
                    }
                    else
                    {
                        while(1)
                        {
                            treeNumLeafs[currentLevel]--;
                            if(treeNumLeafs[currentLevel] == 0)
                            {
                                /* this level cleared -->remove the sub tree from the 'father' */
                                if(currentLevel)
                                {
                                    currentLevel--;
                                    continue;
                                }
                            }

                            break;
                        }
                    }

                }while(currentLevel || treeNumLeafs[0]);

                if(wasSubTree == GT_TRUE)
                {
                    wasSubTree = GT_FALSE;
                    /* last checked index belongs to the sub tree */

                    /* so we need to start from the end of it */
                    jj--;/* we handled the sub-tree at this loop so we need to
                            be able to , makeup for it with extra loop */
                    continue;
                }

            }

            if(lastIndexChecked >= numScanElements)
            {
                /* not error */
                break;
            }

            if(intrScanArr[lastIndexChecked].bitNum > 31)
            {
                /* Error */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(intrScanArr[lastIndexChecked].maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
            {
                /* the entry should be fully ignored !!! */
                bitmapOfBit = 0;
            }
            else
            {
                bitmapOfBit = 1 << intrScanArr[lastIndexChecked].bitNum;
            }

            /*fill array tmpMaskRegDefaultSummaryArr[]*/
            tmpMaskRegDefaultSummaryArr[ii] |= bitmapOfBit;
        }

        /* remove the summary bits from nonSumBitMask */
        intrScanArr[ii].nonSumBitMask =
                        nonSumBitMask &~ tmpMaskRegDefaultSummaryArr[ii];
    }

    for(/*continue ii*/ ; ii < numMaskRegisters; ii++)
    {
        maskRegMapArr[ii] = dummyMaskRegAddr;/* init */
    }


    /* convert array tmpMaskRegMapArr[] --> maskRegMapArr[] */
    /* convert array tmpMaskRegDefaultSummaryArr[] --> maskRegDefaultSummaryArr[] */
    for(ii = 0 ; ii < numScanElements; ii++)
    {
        if(intrScanArr[ii].maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            continue;
        }
        /* the start index is in the range of interrupts for this register */
        enumerationInterruptIndex = intrScanArr[ii].startIdx >> 5;/* /32 */

        if(enumerationInterruptIndex >= numMaskRegisters)
        {
            /* Error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if ((intrScanArr[ii].maskRegAddr & 0xFF000001) == 0x10000001)
        {
            /*this is ptp on cg */
            /*allow duplication of ptp mask for cg ports*/
        }
        else
        if(maskRegMapArr[enumerationInterruptIndex] != dummyMaskRegAddr &&
                maskRegMapArr[enumerationInterruptIndex] != pexMaskAddr && /* there are 2 leaf point to the same register of PEX */
           /* "GPIO_0_31_Interrupt Cause(0x00018154)" is pointed by several leafs in Aldrin */
           !(PRV_CPSS_DRV_IS_PEX_INDICATION_IN_ADDR_MAC(maskRegMapArr[enumerationInterruptIndex]) &&
             (PRV_CPSS_REG_MASK_MAC(maskRegMapArr[enumerationInterruptIndex]) & (~0x00000004)) == 0x00018118))
        {
            /* already set !? */

            /* Error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The interrupt mask address[0x%8.8x] either was already set, or more than one leaf pointed to the same register of PEX\n",
                                          maskRegMapArr[enumerationInterruptIndex]);

        }

        maskRegMapArr[enumerationInterruptIndex] = tmpMaskRegMapArr[ii];

        /* NOTE : must be "|=" and not "=" to support the fact that there are
           2 summary bits to the same register --> see PexErrSum , PexIntSum */
        maskRegDefaultSummaryArr[enumerationInterruptIndex] |= tmpMaskRegDefaultSummaryArr[ii];

    }

    return GT_OK;
}

/**
* @internal prvCpssDrvExMxDxHwPpMaskRegInfoGet function
* @endinternal
*
* @brief   function to build from the interrupts tree and the mask registers
*         addresses , the content of the summary bits in those mask registers.
*         1. fill the array of mask registers addresses
*         2. fill the array of default values for the mask registers
*         3. update the bits of nonSumBitMask in the scan tree
* @param[in] devFamily                - device family
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in,out] intrScanArr[]        - in : interrupts scan info array
*                                       out: non summary bits updated
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[out] maskRegDefaultSummaryArr[] - (pointer to) the registers values
* @param[out] maskRegMapArr[]          - (pointer to) the mask registers addresses
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvExMxDxHwPpMaskRegInfoGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT devFamily,
    IN  GT_U32  numScanElements,
    INOUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC intrScanArr[],
    IN  GT_U32   numMaskRegisters,
    OUT GT_U32   maskRegDefaultSummaryArr[],
    OUT GT_U32   maskRegMapArr[]
)
{
    GT_STATUS   rc;
    GT_U32  *ptr1=NULL,*ptr2=NULL;

    rc = maskRegInfoGet(devFamily,numScanElements,intrScanArr,numMaskRegisters,maskRegDefaultSummaryArr,maskRegMapArr,
        &ptr1 , &ptr2);

    FREE_PTR_MAC(ptr1);
    FREE_PTR_MAC(ptr2);

    return rc;
}

/**
* @internal prvCpssDrvCgPtpIntTreeWa function
* @endinternal
*
* @brief   the fake node needed to skip 'duplication' in the tree checked by : prvCpssDrvExMxDxHwPpMaskRegInfoGet(...) --> maskRegInfoGet(...)
*         after calling prvCpssDrvExMxDxHwPpMaskRegInfoGet
*         1. replace the bobcat3MaskRegMapArr[] with CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS
*         2. replace the BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) with BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe)
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in,out] intrScanArr[]            - interrupts scan info array
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[in,out] maskRegMapArr[]          - (pointer to) the mask registers addresses
* @param[in,out] intrScanArr[]            - interrupts scan info array
* @param[in,out] maskRegMapArr[]          - (pointer to) the mask registers addresses after removing 'fake' values
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS  prvCpssDrvCgPtpIntTreeWa
(
    IN  GT_U32  numScanElements,
    INOUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC intrScanArr[],
    IN  GT_U32   numMaskRegisters,
    INOUT GT_U32   maskRegMapArr[]
)
{
    GT_U32  ii;

    /* peel ptp cg port indication from bobcat3MaskRegMapArr[] */

    /* replace the BOBCAT3_FAKE_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) with BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC(port, pipe) */
    for(ii = 0 ; ii < numScanElements; ii++)
    {
        if((intrScanArr[ii].maskRegAddr & 0xFF000001) == 0x10000001)/* indication of the GOP PTP WA for CG ports */
        {
            /* the entry should be replace by BOBCAT3_PTP_PORT_INTERRUPT_MASK_MAC */
            intrScanArr[ii].maskRegAddr &= ~(0x1) ;
        }
    }

    for(ii = 0 ; ii < numMaskRegisters; ii++)
    {
        if((maskRegMapArr[ii] & 0xFF000001) == 0x10000001)/* indication of the GOP PTP WA for CG ports */
        {
            maskRegMapArr[ii] &= ~(0x1);
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDrvExMxDxHwPpPexAddrGet function
* @endinternal
*
* @brief   function to get addresses of cause and mask interrupt registers of PEX/PCI.
*         (in address space of 'Config cycle')
* @param[in] devFamily                - device family
*
* @param[out] pexCauseRegAddrPtr       - (pointer to) PEX/PCI interrupt cause register address
* @param[out] pexMaskRegAddrPtr        - (pointer to) PEX/PCI interrupt mask register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad param
*/
GT_STATUS prvCpssDrvExMxDxHwPpPexAddrGet
(
    IN  CPSS_PP_FAMILY_TYPE_ENT devFamily,
    OUT GT_U32      *pexCauseRegAddrPtr,
    OUT GT_U32      *pexMaskRegAddrPtr
)
{
    enum{
         USE_PEX_E       /* pex */
        ,USE_PCI_E       /* pci */
        ,USE_NONE_E     /* none used */

    }usePexType;
    GT_U32  extraUnitOffset = 0;/* offset added to 'fixed' address */

    CPSS_NULL_PTR_CHECK_MAC(pexCauseRegAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(pexMaskRegAddrPtr);

    usePexType = USE_PEX_E;

    switch(devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            extraUnitOffset = 0x00070000;/*0x00071900*/
            break;

        case CPSS_DXCH_ALL_SIP5_FAMILY_CASES_MAC:
#ifdef GM_USED  /* currently keep as was initially in cpss --> with lion2 tree */
            extraUnitOffset = 0x00070000;/*0x00071900*/
#else
            extraUnitOffset = 0x00040000;/*0x00041900*/
#endif
            break;

        case PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_CASES_MAC:
            extraUnitOffset = 0x00040000;
            break;

        case CPSS_PP_FAMILY_START_DXCH_E:   /* needed for devices without external CPU to bypass 'PEX/PCI' address */
            usePexType = USE_NONE_E;
            break;
        case CPSS_PX_FAMILY_PIPE_E:
            extraUnitOffset = 0x00040000;/*0x00041900*/
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(usePexType)
    {
        case USE_PEX_E:
            *pexCauseRegAddrPtr = extraUnitOffset + 0x00001900;
            *pexMaskRegAddrPtr  = extraUnitOffset + 0x00001910;
            break;
        case USE_PCI_E:
            *pexCauseRegAddrPtr = 0x00000114;/*PCI*/
            *pexMaskRegAddrPtr  = 0x00000118;/*PCI*/
            break;
        default:
            *pexCauseRegAddrPtr = 0xFFFFFFFF; /* unused register address */
            *pexMaskRegAddrPtr  = 0xFFFFFFFF; /* unused register address */
            break;
    }


    return GT_OK;
}

#ifdef DUMP_DEFAULT_INFO
#define INDENT_STRING   "%s"
static void printElement(
    CPSS_INTERRUPT_SCAN_STC *currIntInfoPtr,
    GT_CHAR *indentStringOffsetPtr,
    GT_U32  *currentIndexPtr
)
{

    GT_U32   currentIndex = *currentIndexPtr;

    cpssOsPrintf("currentIndex = %d (0x%x) \n" ,
        currentIndex,currentIndex);
    currentIndex++;

    *currentIndexPtr = currentIndex;

    if(currIntInfoPtr->maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
    {
        /* the entry should be fully ignored !!! */
        cpssOsPrintf("the entry should be fully ignored !!! \n");

        /* generate the same amount of 'lines' as the 'valid' entry ...
           to allow simple 'compare' of trees */
        cpssOsPrintf("\n");
        cpssOsPrintf("\n");
        cpssOsPrintf("\n\n\n");
        return;
    }

    cpssOsSprintf(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(readRegFuncAddr) ,"0x%8.8x",currIntInfoPtr->pRegReadFunc );
    cpssOsSprintf(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(writeRegFuncAddr),"0x%8.8x",currIntInfoPtr->pRegWriteFunc);

    cpssOsPrintf(INDENT_STRING "%d %d %d 0x%8.8x \n"/*bitNum ..gppFuncPtr*/
                INDENT_STRING "0x%8.8x 0x%8.8x %s %s \n"/*causeRegAddr..pRegWriteFunc*/
                INDENT_STRING "0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x \n"/*startIdx..rwBitMask*/
                INDENT_STRING "%d %d \n\n\n"/*maskRcvIntrEn..rwBitMask*/
    ,indentStringOffsetPtr
            ,currIntInfoPtr->bitNum
            ,currIntInfoPtr->isGpp
            ,currIntInfoPtr->gppId
            ,currIntInfoPtr->gppFuncPtr
    ,indentStringOffsetPtr
            ,currIntInfoPtr->causeRegAddr
            ,currIntInfoPtr->maskRegAddr
            ,(currIntInfoPtr->pRegReadFunc  == prvCpssDrvHwPpPortGroupIsrRead  ? "prvCpssDrvHwPpPortGroupIsrRead"  : PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(readRegFuncAddr) )
            ,(currIntInfoPtr->pRegWriteFunc == prvCpssDrvHwPpPortGroupIsrWrite ? "prvCpssDrvHwPpPortGroupIsrWrite" : PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(writeRegFuncAddr))
    ,indentStringOffsetPtr
            ,currIntInfoPtr->startIdx
            ,currIntInfoPtr->endIdx
            ,currIntInfoPtr->nonSumBitMask
            ,currIntInfoPtr->rwBitMask
    ,indentStringOffsetPtr
            ,currIntInfoPtr->maskRcvIntrEn
            ,currIntInfoPtr->subIntrListLen
        );
}
/**
* @internal printSubTree function
* @endinternal
*
* @brief   print sub tree according to proper indent.
*
* @param[in] fullTreePtr              - pointer to the 'Full tree' of interrupts
* @param[in] numElementsInFullTree    - number of elements in the 'Full tree'
* @param[in] subTreeIndex             - index in the 'Full tree' that represent the 'Sub tree'
*                                      which need print.
* @param[in] indentId                 - the indent id.
*
* @param[out] lastIndexSubTreePtr      - (pointer to) Index in the 'Full tree' which belong
*                                      to the last Index in the 'Sub Tree'.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - algorithm / DB error
*/
static GT_STATUS printSubTree
(
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC    *fullTreePtr ,
    IN GT_U32    numElementsInFullTree,
    IN GT_U32    subTreeIndex,
    OUT GT_U32   *lastIndexSubTreePtr,
    IN GT_U32    indentId,
    INOUT GT_U32 *currentIndexPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii, iiNew;
    CPSS_INTERRUPT_SCAN_STC *currentElementPtr;
    GT_U32  count;
    GT_CHAR indentStringOffset[128];
    GT_U32 kk;
    GT_CHAR levelIdStr[20];
    GT_U32  stringLen;

    if(subTreeIndex >= numElementsInFullTree)
    {
        /* algorithm / DB error */
        cpssOsPrintf(" Invalid Tree ... got index[%d] >= range[%d] \n" ,
            subTreeIndex , numElementsInFullTree);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    currentElementPtr = &fullTreePtr[subTreeIndex];
    count = currentElementPtr->subIntrListLen;

    if(indentId == 0)
    {
        indentStringOffset[0] = ' ';
        kk = 1;
    }
    else
    {
        kk = 0;
        for(ii = 0 ; ii < indentId ; ii++)
        {
            kk = ii*4;
            indentStringOffset[kk]   = ' ';
            indentStringOffset[kk+1] = ' ';
            indentStringOffset[kk+2] = ' ';
            indentStringOffset[kk+3] = ' ';
        }
        kk += 4;
    }
    indentStringOffset[kk] = 0;/* end of string */

    /* set the level into the string */
    cpssOsSprintf(levelIdStr,"L[%d] ",indentId);
    stringLen = cpssOsStrlen(levelIdStr);
    if (kk >= stringLen)
    {
        cpssOsMemCpy(indentStringOffset,levelIdStr,stringLen);
    }


    /* print the base of the sub tree */
    printElement(currentElementPtr,indentStringOffset, currentIndexPtr);

    /* update indentStringOffset for the 'first level leafs' of this sub tree */
    {
        indentStringOffset[kk]   = ' ';
        indentStringOffset[kk+1] = ' ';
        indentStringOffset[kk+2] = ' ';
        indentStringOffset[kk+3] = ' ';
        kk += 4;
    }
    indentStringOffset[kk] = 0;/* end of string */

    indentId++;

    cpssOsSprintf(levelIdStr,"L[%d] ",indentId);
    stringLen = cpssOsStrlen(levelIdStr);
    if (kk >= stringLen)
    {
        cpssOsMemCpy(indentStringOffset,levelIdStr,stringLen);
    }


    for(ii = (subTreeIndex + 1);/* skip start of sub tree */
       (ii < numElementsInFullTree) && count ;
       ii++)
    {
        if(0 == (ii & 0x0f))
        {
            /* allow the terminal to observe all the 'printings' */
            cpssOsTimerWkAfter(50);
        }

        currentElementPtr = &fullTreePtr[ii];

        if(currentElementPtr->subIntrListLen) /* sub tree */
        {
            /* let recursive on the sub tree to print it */
            rc = printSubTree(fullTreePtr,numElementsInFullTree,ii,&iiNew,indentId,currentIndexPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            ii = iiNew;/* last index that is in the sub tree ...
                          with the '++' of the loop it will become 'first index out the tree' */
        }
        else /* leaf . not sub tree */
        {
            /* print the entry that is leaf in the sub tree */
            printElement(currentElementPtr,indentStringOffset, currentIndexPtr);
        }

        if(currentElementPtr->maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* not counting this invalid entry */
            continue;
        }


        count--;/* indicate that another child (leaf/sub tree) was done */
    }

    *lastIndexSubTreePtr = ii-1;/* we return value of 'last Index in the Sub Tree'*/

    return GT_OK;
}


/**
* @internal prvCpssDrvPpIntDefPrint function
* @endinternal
*
* @brief   print the interrupts arrays info, that build by
*         prvCpssDrvExMxDxHwPpMaskRegInfoGet
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in] intrScanArr[]            - interrupts scan info array
* @param[in] numMaskRegisters         - number of registers in maskRegMapArr,maskRegDefaultSummaryArr
* @param[in] maskRegMapArr[]          - (pointer to) place holder for the mask registers addresses
* @param[in] maskRegDefaultSummaryArr[] - (pointer to) place holder for the registers values
*/
void  prvCpssDrvPpIntDefPrint(
    IN GT_U32  numScanElements,
    IN CPSS_INTERRUPT_SCAN_STC intrScanArr[],
    IN GT_U32  numMaskRegisters,
    IN GT_U32  maskRegMapArr[] ,
    IN GT_U32  maskRegDefaultSummaryArr[]
)
{
    GT_U32  ii;

    GT_U32 currentIndex = 0;/* index for the lines */

    cpssOsPrintf("maskRegMapArr , maskRegDefaultSummaryArr \n");
    cpssOsPrintf("================================================ \n");
    for(ii = 0; ii < numMaskRegisters ; ii++)
    {
        if (maskRegMapArr[ii] == 0x00000000 || maskRegMapArr[ii] == 0xfffffff1)
        {
            continue;
        }
        if (0 == (ii & 0x1f))
        {
            /* allow the terminal to observe all the 'printings' */
            cpssOsTimerWkAfter(50);
        }
        cpssOsPrintf("0x%8.8x       , 0x%8.8x \n",maskRegMapArr[ii] , maskRegDefaultSummaryArr[ii]);
    }

    cpssOsPrintf("\n");

    cpssOsPrintf("intrScanArr \n");
    cpssOsPrintf("=============== \n");

    cpssOsPrintf("bitNum,isGpp,gppId,gppFuncPtr \n");
    cpssOsPrintf("causeRegAddr,maskRegAddr,pRegReadFunc,pRegWriteFunc \n");
    cpssOsPrintf("startIdx,endIdx,nonSumBitMask,rwBitMask \n");
    cpssOsPrintf("maskRcvIntrEn,subIntrListLen \n");
    cpssOsPrintf("=============== \n");

    ii = 0;
    printSubTree(intrScanArr,numScanElements,ii,&ii,1, &currentIndex);

    if((ii + 1) < numScanElements)
    {
        ii++;/* first index that was not printed yet */
        /* we got leftovers */
        cpssOsPrintf("NOTE: next entries are 'out of the tree' !!! \n");
        /* continue to dump 'left over' elements !!! */
        for(/*ii continue */;ii < numScanElements; ii++)
        {
            printElement(&intrScanArr[ii]," ----  ", &currentIndex);
        }
    }

    cpssOsPrintf("interrupt tree summary: numScanElements[%d] \n",
        numScanElements);
}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print the register info according to value in their 'huge' interrupts enum
*         for example : for bobcat2 : PRV_CPSS_BOBCAT2_INT_CAUSE_ENT
*         (assuming that this register is part of the interrupts tree)
* @param[in] numScanElements          - number of elements in intrScanArr[]
* @param[in] intrScanArr[]            - interrupts scan info array
* @param[in] interruptId              - the Id to look it the corresponding register.
* @param[in] deviceFlag               - to denote if function is called for
*                                       Falcon device or not.
*                                       GT_TRUE - for Falcon devices
*                                       GT_FALSE - for other devices
*/
void  prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(
    IN GT_U32  numScanElements,
    IN CPSS_INTERRUPT_SCAN_STC intrScanArr[],
    IN GT_U32   interruptId,
    IN GT_BOOL  deviceFlag
)
{
    GT_U32  ii;
    CPSS_INTERRUPT_SCAN_STC *currIntInfoPtr;
    GT_U32 registerId = (interruptId >> 5);/*ignore 5 first LSBits*/
    GT_U32 uniEvCause;
    static char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
    GT_U8   devNum = 0;
    GT_U32  portGroupId = 0;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC *devIntPtr;
    GT_STATUS   retVal;

    currIntInfoPtr = &intrScanArr[0];

    for(ii = 0; ii < numScanElements ; ii++, currIntInfoPtr++)
    {
        if(currIntInfoPtr->maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            continue;
        }

        if((currIntInfoPtr->startIdx >> 5) == registerId)/*ignore 5 first LSBits*/
        {
            /* found match */
            break;
        }
    }

    if(ii == numScanElements)
    {
        cpssOsPrintf("no match found for interruptId[%d] \n",
            interruptId);
        return;
    }

    cpssOsPrintf("\n");

    /* Falcon is a multi-tile architechture and hence interrupts can be
     * triggered from any tile so in order to get the correct interrupt index
     * as per the falcon interrupt enum definition in file
     * prvCpssDrvDxChEventsFalcon.h we need to use modulo on the given
     * interrupt index with max interrupts in tile 0.
     */
    if(deviceFlag)
    {
        registerId = (interruptId% PRV_CPSS_FALCON_TILE_0____LAST__E) >> 5;
    }
    cpssOsPrintf("interruptId[%d] is bit[%d] in register address [0x%8.8x]  (in the huge enum in H file in registerId[%d]) \n" ,
        interruptId,
        (interruptId & 0x1F),
        currIntInfoPtr->causeRegAddr,
        registerId);

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum] &&
       PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool)
    {
        retVal = drvInterruptsInfoGet(devNum,portGroupId,&devIntPtr);
        if(retVal == GT_OK && devIntPtr->numOfInterrupts > interruptId)
        {
            uniEvCause = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool[interruptId].uniEvCause;

            if(uniEvCause < CPSS_UNI_EVENT_COUNT_E)
            {
                cpssOsPrintf("Mapped to unified event [%s] [%d] \n",
                    uniEvName[uniEvCause],
                    uniEvCause);
            }
            else if (uniEvCause == CPSS_UNI_RSRVD_EVENT_E)
            {
                cpssOsPrintf("the interrupt is NOT Mapped to unified event \n");
            }
            else
            {
                cpssOsPrintf("the interrupt is ERROR unified event indication [%d] \n",
                    uniEvCause);
            }
        }
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("more info ... \n");
    cpssOsPrintf("position in intrScanArr[%d] \n" , ii);
    cpssOsPrintf("=============== \n");

    cpssOsPrintf("bitNum,isGpp,gppId,gppFuncPtr \n");
    cpssOsPrintf("causeRegAddr,maskRegAddr,pRegReadFunc,pRegWriteFunc \n");
    cpssOsPrintf("startIdx,endIdx,nonSumBitMask,rwBitMask \n");
    cpssOsPrintf("maskRcvIntrEn,subIntrListLen \n");
    cpssOsPrintf("=============== \n");

    cpssOsPrintf("%d %d %d 0x%8.8x \n"/*bitNum ..gppFuncPtr*/
                "0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x \n"/*causeRegAddr..pRegWriteFunc*/
                "0x%8.8x 0x%8.8x 0x%8.8x 0x%8.8x \n"/*startIdx..rwBitMask*/
                "%d %d \n\n\n"/*maskRcvIntrEn..rwBitMask*/
            ,currIntInfoPtr->bitNum
            ,currIntInfoPtr->isGpp
            ,currIntInfoPtr->gppId
            ,currIntInfoPtr->gppFuncPtr
            ,currIntInfoPtr->causeRegAddr
            ,currIntInfoPtr->maskRegAddr
            ,currIntInfoPtr->pRegReadFunc
            ,currIntInfoPtr->pRegWriteFunc
            ,currIntInfoPtr->startIdx
            ,currIntInfoPtr->endIdx
            ,currIntInfoPtr->nonSumBitMask
            ,currIntInfoPtr->rwBitMask
            ,currIntInfoPtr->maskRcvIntrEn
            ,currIntInfoPtr->subIntrListLen
        );

}

/**
* @internal prvCpssDrvPpIntEventInfoPrint function
* @endinternal
*
* @brief   print info about unified event for all interupts.
*
* @param[in] devNum          - device number
* @param[in] portGroupId     - port group ID
*
*/
GT_VOID prvCpssDrvPpIntEventInfoPrint
(
    IN  GT_U8           devNum,
    IN  GT_U32          portGroupId
)
{
    GT_U32                          intNum; /* number of interrupts */
    GT_U32                          ii;     /* iterator             */
    PRV_CPSS_DRV_EV_REQ_NODE_STC    *intReqNodeListPtr; /* interrupts DB */

    intNum = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.numOfIntBits;
    intReqNodeListPtr = PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->intCtrl.portGroupInfo[portGroupId].intNodesPool;
    if (intReqNodeListPtr == NULL)
    {
        cpssOsPrintf("\nEvent info empty for dev[%d] portGroup[%d]\n", devNum,portGroupId);
        return;
    }

    cpssOsPrintf("\nEvent info dev[%d] portGroup[%d] numOfInt %d\n", devNum,portGroupId,intNum);

    for(ii = 0; ii < intNum; ii++)
    {
        cpssOsPrintf("  %6d 0x%08X %6d 0x%08X\n", ii, intReqNodeListPtr[ii].intCause,
                     intReqNodeListPtr[ii].uniEvCause, intReqNodeListPtr[ii].uniEvExt);
    }
}

/**
* prvCpssDrvPPinitPollingEnableSet
*
* DESCRIPTION:
*       enable/disable Interrupt handler task (polling mode).
* INPUTS:
*       devNum - devNum
*       enable - enable/disable enter to rotine
*/
GT_STATUS prvCpssDrvPPinitPollingEnableSet
(
    GT_U8  devNum,
    GT_BOOL enable
)
{
#ifdef EMULATE_INTERRUPTS
    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(intArr[devNum].enabled, enable);
#endif
    return GT_OK ;
}

/**
* @internal prvCpssDrvDxExMxInterruptsMemoryInit function
* @endinternal
*
* @brief   Allocate and initialize memory for device family interrupts .
*
* @param[inout] infoArrayPtr        - (pointer to) device family interrupts info
* @param[in] numElements            - number of elements in array
*
* @retval GT_OK                     - on success,
* @retval GT_BAD_PTR                - one of the pointers is NULL
* @retval GT_BAD_PARAM              - the scan tree information has error
* @retval GT_OUT_OF_CPU_MEM         - on memory allocation fail
*/
GT_STATUS prvCpssDrvDxExMxInterruptsMemoryInit
(
    INOUT PRV_CPSS_DRV_INTERRUPTS_INFO_STC * infoArrayPtr,
    IN GT_U32   numElements
)
{
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(infoArrayPtr);

    for(ii = 0; ii < numElements; ii++)
    {
        if(infoArrayPtr[ii].intrScanArr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        infoArrayPtr[ii].intrScanOutArr = cpssOsMalloc(infoArrayPtr[ii].numScanElements * sizeof(PRV_CPSS_DRV_INTERRUPT_SCAN_STC));
        if (infoArrayPtr[ii].intrScanOutArr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemCpy(infoArrayPtr[ii].intrScanOutArr, infoArrayPtr[ii].intrScanArr, infoArrayPtr[ii].numScanElements * sizeof(PRV_CPSS_DRV_INTERRUPT_SCAN_STC));

        infoArrayPtr[ii].maskRegDefaultSummaryArr = cpssOsMalloc(infoArrayPtr[ii].numMaskRegisters * sizeof(GT_U32));
        if (infoArrayPtr[ii].maskRegDefaultSummaryArr == NULL)
        {
            /* Free previously alloctated memories */
            cpssOsFree(infoArrayPtr[ii].intrScanOutArr);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(infoArrayPtr[ii].maskRegDefaultSummaryArr, 0, infoArrayPtr[ii].numMaskRegisters * sizeof(GT_U32));

        infoArrayPtr[ii].maskRegMapArr = cpssOsMalloc(infoArrayPtr[ii].numMaskRegisters * sizeof(GT_U32));
        if (infoArrayPtr[ii].maskRegMapArr == NULL)
        {
            /* Free previously alloctated memories */
            cpssOsFree(infoArrayPtr[ii].intrScanOutArr);
            cpssOsFree(infoArrayPtr[ii].maskRegDefaultSummaryArr);

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(infoArrayPtr[ii].maskRegMapArr, 0, infoArrayPtr[ii].numMaskRegisters * sizeof(GT_U32));
    }

    return GT_OK;
}


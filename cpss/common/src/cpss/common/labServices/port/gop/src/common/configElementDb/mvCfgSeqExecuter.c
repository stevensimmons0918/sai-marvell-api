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
* @file mvCfgSeqExecuter.c
*
* @brief Run configuration sequence API
*
* @version   24
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/configElementDb/mvCfgElementDb.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsD2dIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

static MV_CFG_SEQ *hwsSeqDb = NULL;
static GT_U8 numberOfSequences = 0;

/******************************* pre-declaration  **************************************/

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)

static GT_STATUS mvCfgSeqParamManipulationMtiLowSpeedPcs
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 index,
    GT_U32 *regAddrPtr,
    GT_U32 *dataPtr,
    GT_U32 *maskPtr
)
{
    GT_U8   localPortIndex;
    GT_U32  mask, data;

    devNum = devNum;
    portGroup = portGroup;
    localPortIndex = index % 8;
    mask = *maskPtr;
    data = *dataPtr;

    switch(*regAddrPtr)
    {
        case MTI_LPCS_GMODE:
            if(mask & 0x1) /* Lpcs_enable {7:0} - Per channel 1G PCS enable */
            {
                *dataPtr = (data & 0x1) << localPortIndex;
                *maskPtr = (mask & 0x1) << localPortIndex;
            }
            break;

        case MTI_LPCS_IF_MODE:
        case MTI_LPCS_CONTROL:
            *regAddrPtr += localPortIndex * 0x80;
            break;

        default:
            return GT_BAD_VALUE;
    }

    return GT_OK;
}

static GT_STATUS mvCfgSeqParamManipulationMtiExt
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 index,
    GT_U32 *regAddrPtr,
    GT_U32 *dataPtr,
    GT_U32 *maskPtr
)
{
    GT_U8   localPortIndex;
    GT_U32  mask, data;

    devNum = devNum;
    portGroup = portGroup;
    localPortIndex = index % 8;
    mask = *maskPtr;
    data = *dataPtr;

    switch(*regAddrPtr)
    {
        case MTIP_EXT_GLOBAL_PMA_CONTROL:
            *dataPtr = (data & 0x0101) << localPortIndex;
            *maskPtr = (mask& 0x0101) << localPortIndex;
            break;

        case MTIP_EXT_GLOBAL_FEC_CONTROL:
            *dataPtr = (data & 0x0F0F0F0F) << localPortIndex;
            *maskPtr = (mask & 0x0F0F0F0F) << localPortIndex;
            break;

        case MTIP_EXT_GLOBAL_CHANNEL_CONTROL:
            if(mask & 4)
            {
                *maskPtr |= ~0x4;
                *maskPtr |= (mask & 0x4) << localPortIndex / 2;
                *dataPtr |= ~0x4;
                *dataPtr |= (data & 0x4) << localPortIndex / 2;
            }
            else if(mask & 0x400)
            {
                *maskPtr |= ~0x400;
                *maskPtr |= (mask & 0x400) << localPortIndex / 2;
                *dataPtr |= ~0x400;
                *dataPtr |= (data & 0x400) << localPortIndex / 2;
            }
            else if(mask & 0x4000)
            {
                *maskPtr |= ~0x4000;
                *maskPtr |= (mask & 0x4000) << localPortIndex / 2;
                *dataPtr |= ~0x4000;
                *dataPtr |= (data & 0x4000) << localPortIndex / 2;
            }
            break;


        case MTIP_EXT_GLOBAL_CLOCK_ENABLE:
            if(mask & 0x01)
            {
                *dataPtr = (data & 0x01) << localPortIndex;
                *maskPtr = mask << localPortIndex;
            }
            else if((mask & 0x0100))
            {
                *dataPtr = (data & 0x0100) << (localPortIndex / 4);
                *maskPtr = mask << (localPortIndex / 4);
            }
            break;

        /*0<port<8*/
        case MTIP_EXT_GLOBAL_RESET_CONTROL:
            *dataPtr = data & 0xff000000;
            *maskPtr = mask & 0xff000000;

            if (mask & 0x10101)
            {
                *dataPtr |= (data & 0x10101) << localPortIndex;
                *maskPtr |= (mask & 0x10101) << localPortIndex;
            }
            if (mask & (0x1<<24))
            {
                *dataPtr &= ~(3<<24);
                *dataPtr |= (data & (0x1<<24)) << (localPortIndex/4);
                *maskPtr &= ~(3<<24);
                *maskPtr |= (mask & (0x1<<24)) << (localPortIndex/4);
            }
            break;

        case MTIP_EXT_SEG_PORT_CONTROL:
            *regAddrPtr += (localPortIndex/4) * 0x14;
            break;

        case MTIP_EXT_PORT_CONTROL:
            *regAddrPtr += localPortIndex * 0x18;
            break;

        case MTIP_EXT_SEG_PORT_STATUS:
        case MTIP_EXT_GLOBAL_CLOCK_CONTROL:
        case MTIP_EXT_PORT_STATUS:
            /* no change needed */
            break;

        default:
            return GT_BAD_VALUE;
    }
    return GT_OK;
}

static GT_STATUS mvCfgSeqParamManipulationMsdb
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 index,
    GT_U32 *regAddrPtr,
    GT_U32 *dataPtr,
    GT_U32 *maskPtr
)
{
    /* NO need to support MULTI_SEGMENT*/
    GT_U8   localPortIndex;

    localPortIndex = index % 8;
    devNum = devNum;
    portGroup = portGroup;
    dataPtr = dataPtr;
    maskPtr = maskPtr;
    switch(*regAddrPtr)
    {
        case MSDB_CHANNEL_TX_CONTROL:
        case MSDB_CHANNEL_RX_CONTROL:
        case MSDB_FCU_CHANNEL_TX_CONTROL:
        case MSDB_FCU_CHANNEL_RX_CONTROL:
            *regAddrPtr += localPortIndex * HWS_MSDB_CHANNEL_OFFSET;
            break;

        case MSDB_FCU_CONTROL:
            /* no changes needed */
            break;

        case MSDB_SEGMENTED_CHANNEL_TX_CONTROL:
        case MSDB_SEGMENTED_CHANNEL_RX_CONTROL:
            *regAddrPtr += (localPortIndex/4) * HWS_MSDB_CHANNEL_OFFSET;
            break;

        default:
            return GT_BAD_VALUE;
    }
    return GT_OK;
}

static GT_STATUS mvCfgSeqParamManipulationMpfs
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 index,
    GT_U32 *regAddrPtr,
    GT_U32 *dataPtr,
    GT_U32 *maskPtr
)
{

    GT_U8   localPortIndex;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    localPortIndex = index % 8;

    devNum = devNum;
    portGroup = portGroup;
    regAddrPtr =regAddrPtr;
    dataPtr = dataPtr;
    maskPtr = maskPtr;

    switch(*regAddrPtr)
    {
        case MPFS_PAU_CONTROL:
        case MPFS_SAU_CONTROL:
        case MPFS_SAU_STATUS:

            /* no changes needed */
            break;

        default:
            return GT_BAD_VALUE;
    }
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, index, NON_SUP_MODE, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if(HWS_IS_PORT_MULTI_SEGMENT(curPortParams.portStandard))
    {
        /* ADD - segmented port support */
        if(localPortIndex == 0)
        {
            /* if multi speed(200/400) on port 0 - we use channel 8 */
            *regAddrPtr += 8*HWS_MPF_CHANNEL_OFFSET;
        }
        /* if multi speed(200) on port 4 - we use channel 9 - but we already set regAddr to channel 4 */
        else if(localPortIndex == 4)
        {
            *regAddrPtr += 5*HWS_MPF_CHANNEL_OFFSET;
        }
    }

    return GT_OK;
}

static GT_STATUS mvCfgSeqParamManipulationMtiRsFec
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 index,
    GT_U32 *regAddrPtr,
    GT_U32 *dataPtr,
    GT_U32 *maskPtr
)
{
    /* NO need to support MULTI_SEGMENT*/
    GT_U8   localPortIndex;
    localPortIndex = index % 8;

    devNum = devNum;
    portGroup = portGroup;
    index = index;
    regAddrPtr =regAddrPtr;
    dataPtr = dataPtr;
    maskPtr = maskPtr;

    switch(*regAddrPtr)
    {
        case MTI_RSFEC_CONTROL:
        case MTI_RSFEC_DEC_THRESH:
            *regAddrPtr += localPortIndex * 0x20;
            break;

        case  MTI_RSFEC_VENDOR_VL0_0 :
        case  MTI_RSFEC_VENDOR_VL0_1 :
        case  MTI_RSFEC_VENDOR_VL1_0 :
        case  MTI_RSFEC_VENDOR_VL1_1 :
        case  MTI_RSFEC_VENDOR_VL2_0 :
        case  MTI_RSFEC_VENDOR_VL2_1 :
        case  MTI_RSFEC_VENDOR_VL3_0 :
        case  MTI_RSFEC_VENDOR_VL3_1 :
        case  MTI_RSFEC_VENDOR_VL4_0 :
        case  MTI_RSFEC_VENDOR_VL4_1 :
        case  MTI_RSFEC_VENDOR_VL5_0 :
        case  MTI_RSFEC_VENDOR_VL5_1 :
        case  MTI_RSFEC_VENDOR_VL6_0 :
        case  MTI_RSFEC_VENDOR_VL6_1 :
        case  MTI_RSFEC_VENDOR_VL7_0 :
        case  MTI_RSFEC_VENDOR_VL7_1 :
        case  MTI_RSFEC_VENDOR_VL8_0 :
        case  MTI_RSFEC_VENDOR_VL8_1 :
        case  MTI_RSFEC_VENDOR_VL9_0 :
        case  MTI_RSFEC_VENDOR_VL9_1 :
        case  MTI_RSFEC_VENDOR_VL10_0:
        case  MTI_RSFEC_VENDOR_VL10_1:
        case  MTI_RSFEC_VENDOR_VL11_0:
        case  MTI_RSFEC_VENDOR_VL11_1:
        case  MTI_RSFEC_VENDOR_VL12_0:
        case  MTI_RSFEC_VENDOR_VL12_1:
        case  MTI_RSFEC_VENDOR_VL13_0:
        case  MTI_RSFEC_VENDOR_VL13_1:
        case  MTI_RSFEC_VENDOR_VL14_0:
        case  MTI_RSFEC_VENDOR_VL14_1:
        case  MTI_RSFEC_VENDOR_VL15_0:
        case  MTI_RSFEC_VENDOR_VL15_1:
            /* no changes needed */
            break;

        default:
            return GT_BAD_VALUE;
    }
    return GT_OK;
}

static GT_STATUS mvCfgSeqParamManipulationTsu
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 index,
    GT_U32 *regAddrPtr,
    GT_U32 *dataPtr,
    GT_U32 *maskPtr
)
{

    GT_U8   localPortIndex;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;

    localPortIndex = index % 8;

    devNum = devNum;
    portGroup = portGroup;
    regAddrPtr =regAddrPtr;
    dataPtr = dataPtr;
    maskPtr = maskPtr;

    switch(*regAddrPtr)
    {
        case TSU_CONTROL:
            /* no changes needed */
            break;

        default:
            return GT_BAD_VALUE;
    }
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, index, NON_SUP_MODE, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if(HWS_IS_PORT_MULTI_SEGMENT(curPortParams.portStandard))
    {
        /* ADD - segmented port support */
        if(localPortIndex == 0)
        {
            /* if multi speed(200/400) on port 0 - we use channel 8 */
            *regAddrPtr += 8*HWS_MPF_CHANNEL_OFFSET;
        }
        /* if multi speed(200) on port 4 - we use channel 9 - but we already set regAddr to channel 4 */
        else if(localPortIndex == 4)
        {
            *regAddrPtr += 5*HWS_MPF_CHANNEL_OFFSET;
        }
    }

    return GT_OK;
}

static GT_STATUS mvCfgSeqParamManipulation
(
    GT_U8           devNum,
    GT_U32          portGroup,
    MV_HWS_UNITS_ID unitId,
    GT_U32          index,
    GT_U32          *regAddrPtr,
    GT_U32          *dataPtr,
    GT_U32          *maskPtr
)
{
    GT_STATUS rc = GT_OK;

    if((hwsDeviceSpecInfo[devNum].devType != Falcon) && (hwsDeviceSpecInfo[devNum].devType != Raven))
    {
        return GT_OK;
    }

    switch (unitId)
    {
        case MTI_LOW_SP_PCS_UNIT:
            rc = mvCfgSeqParamManipulationMtiLowSpeedPcs(devNum, portGroup, index, regAddrPtr, dataPtr, maskPtr);
            break;

        case MTI_EXT_UNIT:
            rc = mvCfgSeqParamManipulationMtiExt(devNum, portGroup, index, regAddrPtr, dataPtr, maskPtr);
            break;

        case RAVEN_MSDB_UNIT:
            rc = mvCfgSeqParamManipulationMsdb(devNum, portGroup, index, regAddrPtr, dataPtr, maskPtr);
            break;

        case RAVEN_MPFS_UNIT:
            rc = mvCfgSeqParamManipulationMpfs(devNum, portGroup, index, regAddrPtr, dataPtr, maskPtr);
            break;

        case MTI_RSFEC_UNIT:
            rc = mvCfgSeqParamManipulationMtiRsFec(devNum, portGroup, index, regAddrPtr, dataPtr, maskPtr);
            break;

        case RAVEN_TSU_UNIT:
            rc = mvCfgSeqParamManipulationTsu(devNum, portGroup, index, regAddrPtr, dataPtr, maskPtr);
            break;

        default:
            break;
    }


    return rc;
}
#endif /* #if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) */
/***************************************************************************************/

/**
* @internal mvCfgSeqRunWriteOp function
* @endinternal
*
* @brief   Run write operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunWriteOp
(
       GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           index,
    const MV_OP_PARAMS    *params
)
{
    GT_U32              accessAddr;
    GT_U32              baseAddr;
    GT_U32              unitIndex;
    MV_HWS_UNITS_ID     unitId;
    GT_U32              data, mask, regOffset;

    unitId = params->unitId;

    if((unitId == CG_UNIT) || (unitId == CG_RS_FEC_UNIT))
    {
        index &= HWS_2_LSB_MASK_CNS;
    }

    data = params->operData;
    mask = params->mask;
    regOffset = params->regOffset;

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
    if(hwsDeviceSpecInfo[devNum].devType == Falcon)
    {
        if((unitId == D2D_EAGLE_UNIT) || (unitId == D2D_RAVEN_UNIT))
        {
            index = PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(devNum, index);
            regOffset = regOffset & 0xFFFF;
        }
        else
        {
            regOffset = regOffset & 0xFFF;
        }
    }

    /*
        Varialble 'index' set to '0' in function mvUnitExtInfoGet() for
        Falcon and Raven devices. Therefore sequence manipulation should
        be done before - with correct 'index' value!!
    */
    CHECK_STATUS(mvCfgSeqParamManipulation(devNum, portGroup, unitId, index, &regOffset, &data, &mask));
#endif

    mvUnitExtInfoGet(devNum, unitId, index, &baseAddr, &unitIndex , &index);

    accessAddr = baseAddr + index*unitIndex + regOffset;

    if ((unitId == INTLKN_RF_UNIT) || (unitId == ETI_ILKN_RF_UNIT))
    {
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        CHECK_STATUS(genInterlakenRegSet(devNum, portGroup, accessAddr, data, mask));
#endif
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, accessAddr, data, mask));
    }

    return GT_OK;
}

#if 0
/**
* @internal mvCfgSeqRunDelayOp function
* @endinternal
*
* @brief   Run delay operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunDelayOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           unitBaseAddr,
    MV_OP_PARAMS    *params
)
{
    MV_DELAY_OP_PARAMS      *wateLine;

    /* to avoid "unreferenced formal parameter" warning */
    devNum = devNum;
    portGroup = portGroup;
    unitBaseAddr = unitBaseAddr;

    wateLine = (MV_DELAY_OP_PARAMS *)params;
    hwsOsExactDelayPtr(devNum, portGroup, wateLine->delay);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run delay %d msec.\n",wateLine->delay);
    osFclose(fdHws);
#endif

    return GT_OK;
}

/**
* @internal mvCfgSeqRunPolOp function
* @endinternal
*
* @brief   Run polling operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunPolOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           index,
    MV_OP_PARAMS    *params
)
{
    GT_U32 polData;
    MV_POLLING_OP_PARAMS    *polLine;
    GT_U32 accessAddr;
    GT_U32              baseAddr;
    GT_U32              unitIndex;

    polLine = (MV_POLLING_OP_PARAMS *)params;
    mvUnitExtInfoGet(devNum, polLine->indexOffset, index, &baseAddr, &unitIndex , &index);

    accessAddr = baseAddr + index*unitIndex + polLine->regOffset;

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
#endif

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, accessAddr, &polData, 0));

    if ((polData & polLine->mask) != polLine->data)
    {
        return GT_TIMEOUT;
    }

    return GT_OK;
}
#endif
/**
* @internal mvCfgSeqExec function
* @endinternal
*
* @brief   Run configuration sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSeqExec
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           index,
    MV_OP_PARAMS    *params,
    GT_U32           paramSize
)
{
    GT_U32 i;

    if (params == NULL)
    {
        return GT_BAD_PARAM;
    }

    for (i = 0; i < paramSize; i++)
    {
        mvCfgSeqRunWriteOp(devNum,portGroup,index,&params[i]);
    }

    return GT_OK;
}

GT_STATUS mvCfgMacPcsSeqExec
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  index,
    const MV_OP_PARAMS      *params,
    GT_U32                  paramSize
)
{
    GT_U32 i;

    if (params == NULL)
    {
        return GT_BAD_PARAM;
    }

    for (i = 0; i < paramSize; i++)
    {
        mvCfgSeqRunWriteOp(devNum,portGroup,index,&params[i]);
    }

    return GT_OK;
}

#if 0
/**
* @internal mvCfgSeqRunParallelSerdesPolOp function
* @endinternal
*
* @brief   Run polling operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunParallelSerdesPolOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           numOfSer,
    GT_U32           *serdesArr,
    MV_OP_PARAMS     *params
)
{
    GT_U32                    i;
    GT_U32                    polData;
    GT_U32                    laneDoneCntr;
    GT_U32                    serdesNum;
    GT_U32                    curPortGroup;
    MV_POLLING_OP_PARAMS      *polLine;
    GT_U32                    accessAddr;
    GT_U32                    baseAddr;
    GT_U32                    unitIndex;

    polLine = (MV_POLLING_OP_PARAMS *)params;
    mvUnitInfoGet(devNum, polLine->indexOffset, &baseAddr, &unitIndex);

    laneDoneCntr = 0;

    for (i = 0; i < numOfSer; i++)
    {
        serdesNum = (serdesArr[i] & 0xFFFF);
        curPortGroup = portGroup + ((serdesArr[i] >> 16) & 0xFF);

        accessAddr = baseAddr + serdesNum*unitIndex + polLine->regOffset;

        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, curPortGroup, accessAddr, &polData, 0));
        if ((polData & polLine->mask) == polLine->data)
        {
            laneDoneCntr++;
        }
    }

    return GT_OK;
}
#endif
/**
* @internal mvCfgSeqRunParallelSerdesWriteOp function
* @endinternal
*
* @brief   Run write operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunParallelSerdesWriteOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           numOfSer,
    GT_U32           *serdesArr,
    MV_OP_PARAMS     *params
)
{
    MV_WRITE_OP_PARAMS  *writeLine;
    GT_U32              i;
    GT_U32              serdesNum;
    GT_U32              curPortGroup;
    GT_U32              accessAddr;
    GT_U32              baseAddr;
    GT_U32              unitIndex;

    writeLine = (MV_WRITE_OP_PARAMS *)params;

    mvUnitInfoGet(devNum, writeLine->indexOffset, &baseAddr, &unitIndex);

    for (i = 0; i < numOfSer; i++)
    {
        serdesNum = (serdesArr[i] & 0xFFFF);
        curPortGroup = portGroup + ((serdesArr[i] >> 16) & 0xFF);
        accessAddr = baseAddr + serdesNum*unitIndex + writeLine->regOffset;

        if ((writeLine->indexOffset == INTLKN_RF_UNIT) || (writeLine->indexOffset == ETI_ILKN_RF_UNIT))
        {
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
            CHECK_STATUS(genInterlakenRegSet(devNum, curPortGroup, accessAddr, writeLine->data, writeLine->mask));
#endif
        }
        else
        {
            CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup, accessAddr, writeLine->data, writeLine->mask));
        }
    }

    return GT_OK;
}
#if 0
/**
* @internal mvCfgSeqRunParallelDelayOp function
* @endinternal
*
* @brief   Run delay operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunParallelDelayOp
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           numOfSer,
    GT_U32           *serdesArr,
    MV_OP_PARAMS     *params
)
{
    MV_DELAY_OP_PARAMS      *wateLine;

    /* to avoid "unreferenced formal parameter" warning */
    devNum = devNum;
    portGroup = portGroup;
    numOfSer = numOfSer;
    serdesArr = serdesArr;

    wateLine = (MV_DELAY_OP_PARAMS *)params;
    hwsOsExactDelayPtr(devNum, portGroup, wateLine->delay);

#ifdef HWS_DEBUG
    if (fdHws == 0)
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "w");
    }
    else
    {
        fdHws = osFopen("C:\\Temp\\Lion2Log.txt", "a");
    }
    osFprintf(fdHws, "\n;Run delay %d msec.\n",wateLine->delay);
    osFclose(fdHws);
#endif

    return GT_OK;
}
#endif
/**
* @internal mvCfgSerdesSeqParallelExec function
* @endinternal
*
* @brief   Run Serdes parallel configuration sequence (in the manner of lanes)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSerdesSeqParallelExec
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           numOfSer,
    GT_U32           *serdesArr,
    MV_OP_PARAMS     *params,
    GT_U32           paramSize
)
{
    GT_U32 i;

    if (params == NULL)
    {
        return GT_BAD_PARAM;
    }

    for (i = 0; i < paramSize; i++)
    {
        mvCfgSeqRunParallelSerdesWriteOp(devNum,portGroup,numOfSer, serdesArr,&params[i]);
    }

    return GT_OK;
}

/**
* @internal mvHwsSerdesWaitForPllConfig function
* @endinternal
 *
*/
GT_STATUS mvHwsSerdesWaitForPllConfig
(
    GT_U8            devNum,
    GT_U32           portGroup,
    GT_U32           numOfSer,
    GT_U32           *serdesArr
)
{
    GT_U32 i;
    GT_U32 baseAddr;
    GT_U32 unitAddr;
    GT_U32 unitIndex;
    GT_U32 serdesNum, curPortGroup;
    GT_U32 data;
    GT_U32 sdDone;
    GT_U32 delayIter;

    if (serdesArr == NULL)
    {
        return GT_BAD_PARAM;
    }

    sdDone = 0;

    /* DB version V1  - Lion2 (Lion2 can also have V2 DB, in which case serdes is COM_PHY_H_REV2
       which is also the serdes in hooper. in AC3 and BC2 its COM_PHY_28NM)*/
    if (hwsDeviceSpecInfo[devNum].serdesType == COM_PHY_H)
    {
        delayIter = 0;

        /* parallel operation */
        while (delayIter<20 && sdDone < numOfSer)
        {
            sdDone = 0;
            for (i = 0; i < numOfSer; i++)
            {
                curPortGroup = portGroup + ((serdesArr[i] >> 16) & 0xFF);
                CHECK_STATUS(genUnitRegisterGet(devNum, curPortGroup, SERDES_UNIT, serdesArr[i], SERDES_EXTERNAL_STATUS_0, &data, (3 << 2)));
                if ( (data & (3 << 2)/*MASK*/) == (3 << 2)/*DATA*/)
                {
                    sdDone++;
                }
                if (sdDone == numOfSer)
                {
                    break;
                }
            }
            hwsOsExactDelayPtr(devNum, portGroup, 1);
            delayIter++;
        }

        if ( sdDone < numOfSer)
        {
            CHECK_STATUS_EXT(GT_TIMEOUT, LOG_ARG_STRING_MAC("timeout waiting for serdes"));
        }
    }
    else /* DB revisions V2 and 28nmDB */
    {
        GT_U32 serdesUnit;
        GT_U32 serdesSubUnitOffset;

        /* DB revision2, in Lion2 and hooper */
        if (hwsDeviceSpecInfo[devNum].serdesType == COM_PHY_H_REV2)
        {
            serdesUnit = SERDES_UNIT;
            serdesSubUnitOffset = SERDES_PHY_REGS;
        }
        else /* COM_PHY_28NM (BC2 and AC3)*/
        {
            serdesUnit = SERDES_PHY_UNIT;
            serdesSubUnitOffset = 0;
        }

        /* get unit base address and unit index for current device */
        mvUnitInfoGet(devNum, serdesUnit, &unitAddr, &unitIndex);

        for (i = 0; i < numOfSer; i++)
        {
                serdesNum = (serdesArr[i] & 0xFFFF);
                curPortGroup = portGroup + ((serdesArr[i] >> 16) & 0xFF);
                baseAddr = unitAddr + serdesSubUnitOffset + serdesNum * unitIndex;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup, baseAddr+0x08, 0x8000, 0x8000));
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup, baseAddr+0x08, 0x4000, 0x4000));
        }

        delayIter = 0;
        while (delayIter < 20 && sdDone < numOfSer)
        {
            sdDone = 0;
            for (i = 0; i < numOfSer; i++)
            {
                serdesNum = (serdesArr[i] & 0xFFFF);
                curPortGroup = portGroup + ((serdesArr[i] >> 16) & 0xFF);
                baseAddr = unitAddr + serdesSubUnitOffset + serdesNum * unitIndex;
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, curPortGroup, baseAddr+0x08, &data, 0x4000));
                if ( (data & 0x4000) == 0x4000 )
                {
                    sdDone++;
                }
            }
            hwsOsExactDelayPtr(devNum, portGroup, 1);
            delayIter++;
        }

        if (delayIter < 20)
        {
            for (i = 0; i < numOfSer; i++)
            {
                serdesNum = (serdesArr[i] & 0xFFFF);
                curPortGroup = portGroup + ((serdesArr[i] >> 16) & 0xFF);
                baseAddr = unitAddr + serdesSubUnitOffset + serdesNum * unitIndex;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup, baseAddr+0x08, 0x0, 0x8000));
            }
        }
        else
        {
            CHECK_STATUS_EXT(GT_TIMEOUT, LOG_ARG_STRING_MAC("timeout waiting for serdes"));
        }

        hwsOsExactDelayPtr(devNum, portGroup, 8);
    }

    return GT_OK;
}

/**
* @internal mvHwsSerdesRxIntUpConfig function
* @endinternal
 *
*/
GT_STATUS mvHwsSerdesRxIntUpConfig
(
    GT_U8 devNum,
    GT_U32 portGroup,
    GT_U32 numOfSer,
    GT_U32 *serdesArr
)
{
    GT_U32 unitAddr2;
    GT_U32 unitIndex2;
    GT_U32 curPortGroup2;
    GT_U32 baseAddr2;
    GT_U32 i=0;
    GT_U32 laneDoneCount=0;
    GT_U32 data2=0;
    GT_U32 polCnt=0;
    GT_U32 unitPerDbRev = 0;
    GT_U32 subUnitOffsetPrtDbRev = 0;

    if (hwsDeviceSpecInfo[devNum].serdesType == COM_PHY_28NM)
    {
        unitPerDbRev = SERDES_PHY_UNIT;
        subUnitOffsetPrtDbRev = 0;
    }
    else /* COM_PHY_H_REV2 */
    {
        unitPerDbRev = SERDES_UNIT;
        subUnitOffsetPrtDbRev = SERDES_PHY_REGS;
    }

    mvUnitInfoGet(devNum, unitPerDbRev, &unitAddr2, &unitIndex2);

    for (i = 0; i < numOfSer; i++)
    {
        curPortGroup2 = portGroup + ((serdesArr[i] >> 16) & 0xFF);
        baseAddr2 = unitAddr2 + subUnitOffsetPrtDbRev + serdesArr[i] * unitIndex2;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup2, baseAddr2+(0x98), 0x400, 0x400));
    }

    hwsOsExactDelayPtr(devNum, portGroup, 1);

    for (i = 0; i < numOfSer; i++)
    {
        curPortGroup2 = portGroup + ((serdesArr[i] >> 16) & 0xFF);
        baseAddr2 = unitAddr2 + subUnitOffsetPrtDbRev + serdesArr[i] * unitIndex2;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup2, baseAddr2+(0x8C), 0x40, 0x40));
    }

    while (polCnt < 100)
    {
        laneDoneCount = 0;
        for (i = 0; i < numOfSer; i++)
        {
            curPortGroup2 = portGroup + ((serdesArr[i] >> 16) & 0xFF);
            baseAddr2 = unitAddr2 + subUnitOffsetPrtDbRev + serdesArr[i] * unitIndex2;
            CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, curPortGroup2, baseAddr2+(0x8C), &data2, 0xFFFFFFFF));
            if ( (data2 & 0x40) == (0x40))
            {
                laneDoneCount++;
            }
        }

        if (laneDoneCount == numOfSer)
        {
            for (i = 0; i < numOfSer; i++)
            {
                curPortGroup2 = portGroup + ((serdesArr[i] >> 16) & 0xFF);
                baseAddr2 = unitAddr2 + subUnitOffsetPrtDbRev + serdesArr[i] * unitIndex2;
                CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, curPortGroup2, baseAddr2 + (0x98), 0x0, 0x400));
            }
            break;
        }

        hwsOsExactDelayPtr(devNum, portGroup, 1);
        polCnt++;
    }

    if (polCnt >= 100)
    {
        CHECK_STATUS_EXT(GT_TIMEOUT, LOG_ARG_STRING_MAC("timeout waiting for serdes"));
    }

    return GT_OK;
}

/**
* @internal mvCfgSeqExecInit function
* @endinternal
*
* @brief   Init configuration sequence executer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCfgSeqExecInit ()
{
    static GT_BOOL engineInitDone = GT_FALSE;

    if (!engineInitDone)
    {
        engineInitDone = GT_TRUE;
    }
}

/**
* @internal mvCfgSerdesSeqRunWriteOp function
* @endinternal
*
* @brief   Run write operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSerdesSeqRunWriteOp
(
    GT_U8         devNum,
    GT_UOPT       portGroup,
    GT_UOPT       serdesNum,
    MV_OP_PARAMS  *writeLine
)
{
    GT_U8           regType;
    GT_UREG_DATA    data;
    GT_UREG_DATA    mask;

    regType = (writeLine->unitId == SERDES_PHY_UNIT) ? INTERNAL_REG : EXTERNAL_REG;
    mask = (GT_U16) writeLine->mask;
    data = (GT_U16) writeLine->operData;

    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, (GT_U8)portGroup, regType, (GT_U8)serdesNum,
                                        writeLine->regOffset, data, mask));

    return GT_OK;
}
#if 0
/**
* @internal mvCfgSerdesSeqRunPolOp function
* @endinternal
*
* @brief   Run SERDES polling operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSerdesSeqRunPolOp
(
    GT_U8         devNum,
    GT_UOPT       portGroup,
    GT_UOPT       serdesNum,
    MV_OP_PARAMS  *polLine
)
{
    GT_U8        regType;
    GT_UREG_DATA polData;

    regType = (polLine->unitId == SERDES_PHY_UNIT) ? INTERNAL_REG : EXTERNAL_REG;

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, (GT_U8)portGroup, regType, (GT_U8)serdesNum,
                                        polLine->regOffset, &polData, 0));

    return GT_OK;
}
#endif
/**
* @internal mvCfgSerdesSeqExec function
* @endinternal
*
* @brief   Run configuration sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSerdesSeqExec
(
    GT_U8         devNum,
    GT_UOPT       portGroup,
    GT_UOPT       serdesNum,
    GT_U8         seqId
)
{
    MV_CFG_SEQ *seq;
    GT_U8 i;

    if (seqId >= numberOfSequences)
    {
        return GT_BAD_PARAM;
    }

    seq = &hwsSeqDb[seqId];
    if (seq->cfgSeqSize == 0)
    {
        return GT_NOT_INITIALIZED;
    }
    for (i = 0; i < seq->cfgSeqSize; i++)
    {
       mvCfgSerdesSeqRunWriteOp(devNum, portGroup, serdesNum, &seq->cfgSeq[i]);
    }

    return GT_OK;
}

/**
* @internal mvCfgSerdesPowerUpCtrl function
* @endinternal
 *
*/
GT_STATUS mvCfgSerdesPowerUpCtrl
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        numOfSer,
    GT_U32        *serdesArr
)
{
    GT_U32 data = 0;
    GT_U32 counter = 0;
    GT_U32 i = 0;

    /* COM_PHY_28NM devices - single lane only */
    if (hwsDeviceSpecInfo[devNum].serdesType == COM_PHY_28NM)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, serdesArr[i],      0,      0x1802, 0x1802));

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, serdesArr[i],      0x8,    0x10,   0x10));

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_PHY_UNIT, serdesArr[i],  0x08 ,  0x8000, 0x8000));

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_PHY_UNIT, serdesArr[i],  0x08 ,  0x4000, 0x4000));

        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_PHY_UNIT, serdesArr[i],  0x08 ,  &data,  0xFFFFFFFF));

        hwsOsExactDelayPtr(devNum, portGroup, 1);

        while ( (data & 0x4000) != 0x4000)
        {
            counter++;
            if (counter == 100)
            {
                CHECK_STATUS_EXT(GT_TIMEOUT, LOG_ARG_STRING_MAC("timeout waiting for serdes"));
            }
            hwsOsExactDelayPtr(devNum, portGroup, 1);
            CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, SERDES_PHY_UNIT, serdesArr[i],  0x08 , &data,   0xFFFFFFFF));
        }

        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_PHY_UNIT, serdesArr[i],  0x08 , 0x0,     0x8000));

        hwsOsExactDelayPtr(devNum, portGroup, 6);
    }
    else /*COM_PHY_H_REV2/COM_PHY_H */
    {
        hwsOsExactDelayPtr(devNum, portGroup, 1);
        for (i = 0; i < numOfSer; i++)
        {
            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_0, 0x1802, 0x1802));

            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_2, 0x10,   0x10));

            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, serdesArr[i], SERDES_EXTERNAL_CONFIGURATION_1, (1 << 3), (1 << 3)));

            CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, SERDES_UNIT, serdesArr[i], SERDES_PHY_REGS + 0x4,            0x7000, 0x7000));
        }
    }

    return GT_OK;
}

/**
* @internal mvCfgSeqRunNewParallelSerdesWriteOp function
* @endinternal
*
* @brief   Run write operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunNewParallelSerdesWriteOp
(
    GT_U8            devNum,
    GT_UOPT          portGroup,
    GT_UOPT          numOfSer,
    GT_UOPT          *serdesArr,
    MV_OP_PARAMS     *writeLine
)
{
    GT_U8           regType;
    GT_UREG_DATA    data;
    GT_UREG_DATA    mask;
    GT_U8  i;


    regType = (writeLine->unitId == SERDES_PHY_UNIT) ? INTERNAL_REG : EXTERNAL_REG;
    mask = (GT_U16) writeLine->mask;
    data = (GT_U16) writeLine->operData;

    for (i = 0; i < numOfSer; i++)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, (GT_U8)portGroup, regType, (GT_U8)serdesArr[i],
                                            writeLine->regOffset, data, mask));
    }

    return GT_OK;
}
#if 0
/**
* @internal mvCfgSeqRunNewParallelSerdesPolOp function
* @endinternal
*
* @brief   Run polling operation
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvCfgSeqRunNewParallelSerdesPolOp
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         numOfSer,
    GT_UOPT         *serdesArr,
    MV_OP_PARAMS    *polLine
)
{
    GT_U8           i;
    GT_U8           regType;
    GT_UREG_DATA    polData;
    GT_U8           laneDoneCntr;

    regType = (polLine->unitId == SERDES_PHY_UNIT) ? INTERNAL_REG : EXTERNAL_REG;

    laneDoneCntr = 0;

    for (i = 0; i < numOfSer; i++)
    {

        CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, (GT_U8)portGroup, regType, (GT_U8)serdesArr[i],
                                            polLine->regOffset, &polData, 0));

        if ((polData & polLine->mask) == polLine->operData)
        {
            laneDoneCntr++;
        }
    }

    return GT_OK;
}
#endif
/**
* @internal mvCfgSerdesNewSeqParallelExec function
* @endinternal
*
* @brief   Run configuration sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvCfgSerdesNewSeqParallelExec
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         numOfSer,
    GT_UOPT         *serdesArr,
    GT_UOPT         seqId
)
{
    MV_CFG_SEQ *seq;
    GT_U8 i;

    if (seqId >= numberOfSequences)
    {
        return GT_BAD_PARAM;
    }

    seq = &hwsSeqDb[seqId];

    for (i = 0; i < seq->cfgSeqSize; i++)
    {
        mvCfgSeqRunNewParallelSerdesWriteOp(devNum, portGroup, numOfSer, serdesArr, seq->cfgSeq);
    }

    return GT_OK;
}

/**
* @internal mvCfgSerdesSeqExecInit function
* @endinternal
*
* @brief   Init SERDES configuration sequence executer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvCfgSerdesSeqExecInit
(
    MV_CFG_SEQ *seqDb,
    GT_U8      numberOfSeq
)
{
    hwsSeqDb = seqDb;
    numberOfSequences = numberOfSeq;
}



/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChHwInitSoftRest.c
*       Internal function used for Soft Reset testing
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitSoftReset.h>
#include <cpss/common/config/private/prvCpssCommonRegs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* for SIP6 The prvCpssDrvHwPpApi.c functions         */
/* subsitute CPSS_HW_DRIVER_AS_SWITCHING_E instead of */
/* CPSS_HW_DRIVER_AS_RESET_AND_INIT_CONTROLLER_E and  */
/* CPSS_HW_DRIVER_AS_DFX_E that is the same value 8   */
/* So DFX registers can be accessed using function    */
/* prvCpssDrvHwPpPortGroupWriteRegister and other     */
/* Switching core oriented functions                  */
/* see                                                */
/* prvCpssDrvHwPpWriteRegBitMaskDrv and               */
/* prvCpssDrvHwPpDoReadOpDrv                          */

#ifdef ASIC_SIMULATION
extern GT_STATUS cpssSimSoftResetDoneWait(void);
#endif /*ASIC_SIMULATION*/
/*
 * @struct PRV_CPSS_MEM_CHUNK_STC
 *
 * @brief Describes the memory chunks, that represent the memories of the device
 *
 *
 */
typedef struct{
    /** @brief
     *  The first address the chunk represents
     */
    GT_U32  memFirstAddr;
    /** @brief
     *  Memory size (in words)
     */
    GT_U32  memSizeInWords;
}PRV_CPSS_MEM_CHUNK_STC;

/*
 * @struct PRV_CPSS_DXCH_MEM_CHUNK_INFO_STC
 *
 * @brief Memory chunks info, that represent the memories of the device
 *
 *
 */
typedef struct {
    /** @brief
     *  The first address this chunk represents
     */
    GT_U32  memFirstAddr;
    /** @brief
     * Pointer to dynamically allocated memory
     */
    GT_U32  *memPtr;
    /** @brief
     * Size of dynamically allocated memory in words
     */
    GT_U32  memSize;
}PRV_CPSS_DXCH_MEM_CHUNK_INFO_STC;

/*
 * @struct PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC
 *
 * @brief This struct defines TxQ HW table memory chunks (per unit) for backup/restore WA.
*/
typedef struct{
    PRV_CPSS_DXCH_MEM_CHUNK_INFO_STC *chunksArray;   /* array of chunks */
    GT_U32                           numOfChunks;    /* number of chunks in the array */
} PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC;

static GT_STATUS prvCpssDxChHwTxQMemWaInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   tile,
    IN PRV_CPSS_DXCH_UNIT_ENT   unit,
    INOUT PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
);

static GT_STATUS prvCpssDxChHwTxQMemWaFree
(
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
);

static GT_STATUS prvCpssDxChHwTxQMemWaBackUp
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
);

static GT_STATUS prvCpssDxChHwTxQMemWaRestore
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
);

static const int prvCpssDxChHwInitSoftResetTraceEnable = 0;
#ifdef SOFT_RESET_DEBUG
int prvCpssDxChHwInitSoftResetTraceEnableSet(int enable)
{
    prvCpssDxChHwInitSoftResetTraceEnable = enable;
    return 0;
}
#endif /* SOFT_RESET_DEBUG */

/**
* @internal prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet function
* @endinternal
*
* @brief   Get address of register from Skip Initialization Marix by type.
*          For SIP6 this address of the register in DFX of Tile0 Eagle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] skipType                 - the type of Skip Initialization Marix
*                                       see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*
* @param[out] regAddrPtr              - pointer to register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device - not supported DFX server or
*                                    specified type of Skip Initialization Marix
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    OUT GT_U32                                         *regAddrPtr
)
{
    GT_U32 regAddr;
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.configSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SRR_LOAD_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SRRSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_BIST_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.BISTSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SOFT_REPAIR_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SoftRepairSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RAMInitSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGS_GENX_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.RegxGenxSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_MULTI_ACTION_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.MultiActionSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.tableSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.SERDESSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.EEPROMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.PCIeSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DEVICE_EEPROM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DeviceEEPROMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_D2D_LINK_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.D2DLinkSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_REGISTERS_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXRegistersSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_DFX_PIPE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_TILE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_TileSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_MNG_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_MngSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_D2DSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.Core_RavenSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_D2DSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_MainSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_CNMSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.GOP_GWSkipInitializationMatrix;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_POE_E:
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.poeSkipInitializationMatrix;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    *regAddrPtr = regAddr;
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSkipTypeSw2HwConvert function
* @endinternal
*
* @brief   Convert SW type of Skip Initialization Domain to sequence of HW types.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] skipType                 - the type of Skip Initialization Marix
*                                       see comments in CPSS_HW_PP_RESET_SKIP_TYPE_ENT.
* @param[in] index                    - index in the type sequence.
*
* @param[out] hwSkipTypePtr           - pointer to HW Skip Type
*                                       see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*
* @retval GT_OK                    - on success,
* @retval GT_NO_MORE               - index is out of sequence length
* @retval GT_BAD_PARAM             - wrong devNum, or skipType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSkipTypeSw2HwConvert
(
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT                  skipType,
    IN  GT_U32                                          index,
    OUT  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  *hwSkipTypePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(hwSkipTypePtr);

    switch (skipType)
    {
        case CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_REGISTERS_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E:
            switch (index)
            {
                case 0:
                    *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_RAM_INIT_E;
                    break;
                case 1:
                    *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_TABLES_E;
                    break;
                default: return /* not error to LOG */ GT_NO_MORE;
            }
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_EEPROM_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_SERDES_E;
            break;
        case CPSS_HW_PP_RESET_SKIP_TYPE_POE_E:
            if (index > 0)
            {
                return /* not error to LOG */ GT_NO_MORE;
            }
            *hwSkipTypePtr = PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_POE_E;
            break;

        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert function
* @endinternal
*
* @brief   Get address of register instance in given SIP6 Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[out] portGroupPtr         - pointer to port group address
* @param[out] regAddrPtr           - pointer to register address
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert
(
    IN  GT_U8                                 devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT dfxType,
    IN  GT_U32                                tileIndex,
    IN  GT_U32                                gopIndex,
    IN  GT_U32                                regAddr,
    OUT GT_U32                                *portGroupPtr,
    OUT GT_U32                                *regAddrPtr
)
{
    GT_U32        dfxAddressMask   = 0x000FFFFF;
    GT_U32        dfxInGopLocaBase = 0x00700000;
    GT_U32        relativeDfxBase;
    GT_BOOL       error;

    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(portGroupPtr);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    if (PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (tileIndex > 0/* support AC5P */ && tileIndex >= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (gopIndex >= FALCON_RAVENS_PER_TILE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *portGroupPtr = 0;
    switch (dfxType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E:
            /* Processing Core DFX */
            dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);
            relativeDfxBase =
                (dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl
                 & (~ dfxAddressMask));
            *regAddrPtr =
                (relativeDfxBase | (regAddr & dfxAddressMask))
                + prvCpssSip6TileOffsetGet(devNum, tileIndex);
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E:
            /* GOP DFX */
            relativeDfxBase =
                (prvCpssDxChHwUnitBaseAddrGet(
                    devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + gopIndex), &error)
                 + dfxInGopLocaBase);
            *regAddrPtr =
                (relativeDfxBase | (regAddr & dfxAddressMask))
                + prvCpssSip6TileOffsetGet(devNum, tileIndex);
            if (error != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

GT_STATUS prvCpssDrvHwPpSmiChipletRegisterWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    IN GT_U32   value,
    IN GT_U32   mask
);
GT_STATUS prvCpssDrvHwPpSmiChipletRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   chipletId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);


/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask function
* @endinternal
*
* @brief   Write DFX Register bit mask to register instance in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         regAddr,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS    rc;
    GT_U32       portGroup;
    GT_U32       convertedRegAddr;

    rc = prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert(
        devNum, dfxType, tileIndex, gopIndex, regAddr,
        &portGroup, &convertedRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E == dfxType)
    {
        GT_U32 ravenId = tileIndex*FALCON_RAVENS_PER_TILE + gopIndex/*0..3*/;
        GT_U32 value;

        /* check that this Raven is accessable !*/
        rc = prvCpssDrvHwPpSmiChipletRegisterRead(devNum,ravenId,0x00300050,&value);
        if(rc != GT_OK)
        {
            /* no SMI to this chiplet */
            return GT_OK;
        }
        if(value != 0x11AB)
        {
            cpssOsPrintf("devNum[%d] ravenId[%d] is not accessable to MG 0x00300050 : read [0x%8.8x] instead of 0x11AB ... so skip it \n",
                devNum,ravenId,value);
            return GT_OK;
        }

        return prvCpssDrvHwPpSmiChipletRegisterWriteRegBitMask(devNum,ravenId,convertedRegAddr, data , mask);
    }

    return prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, portGroup, convertedRegAddr, mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6DfxRegRead function
* @endinternal
*
* @brief   Read DFX register instance in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] regAddr               - pattern register address used to take offset from DFX base.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6DfxRegRead
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         regAddr,
    OUT GT_U32                                         *dataPtr
)
{
    GT_STATUS    rc;
    GT_U32       portGroup;
    GT_U32       convertedRegAddr;

    rc = prvCpssDxChHwInitSoftResetSip6DfxRegAddrConvert(
        devNum, dfxType, tileIndex, gopIndex, regAddr,
        &portGroup, &convertedRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDrvHwPpPortGroupReadRegister(
        devNum, portGroup, convertedRegAddr, dataPtr);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS    rc;
    GT_U32       dbRegAddr;

    rc = prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet(
        devNum, skipType, &dbRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr,
        mask, data);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead function
* @endinternal
*
* @brief   Read given Skip Init Matrix register instance
*          in given Falcon Tile or in given Raven.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[out] dataPtr              - pointer to data read from register
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6SkipInitMatrixRead
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    OUT GT_U32                                         *dataPtr
)
{
    GT_STATUS    rc;
    GT_U32       dbRegAddr;

    rc = prvCpssDxChHwInitSoftResetSkipInitMatrixRegAddrGet(
        devNum, skipType, &dbRegAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChHwInitSoftResetSip6DfxRegRead(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr, dataPtr);
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to reset trigger register instance
*          in given Falcon Tile or in given Raven.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] dfxType               - dfx Type - core or GOP
* @param[in] tileIndex             - index of tile 0..3
* @param[in] gopIndex              - index of GOP 0..3 - for GOP connected to the given tile.
*                                    relevant only when dfx Type is GOP
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6ResetTriggerRegWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT          dfxType,
    IN  GT_U32                                         tileIndex,
    IN  GT_U32                                         gopIndex,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_U32       dbRegAddr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    dbRegAddr =
        PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
            DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;

    return prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
        devNum, dfxType, tileIndex, gopIndex, dbRegAddr,
        mask, data);
}

#define MAX_TILES_NUM           4
#define MAX_TXQ_UNITS_NUM       9       /* Maximal units (per tile) needed backup/restore tables in WA */

/**
* @internal prvCpssDxChHwPpSip6TxQMemoryWaRestore function
* @endinternal
*
* @brief   This function allocates memories for TxQ tables SW reset WA
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] beforeSwReset         - reset state
*                                       GT_TRUE     - allocate and init memory chunks and backup PP memory before SW reset
*                                       GT_FALSE    - restore PP memory and free memory chunks after SW reset
* @param[in] errorStatus           - error status before system restore
* @param[in/out] txqTableBackUpMemories - pointer to state DB for the WA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate memory
* @retval GT_TIMEOUT               - time out during system reset
*/
static GT_STATUS prvCpssDxChHwPpSip6TxQMemoryWaRestore
(
    IN GT_U8                devNum,
    IN GT_BOOL              beforeSwReset,
    IN GT_STATUS            errorStatus,
    INOUT PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC  txqTableBackUpMemories[MAX_TILES_NUM][MAX_TXQ_UNITS_NUM]
)
{
    GT_STATUS rc;
    GT_U32    tile;
    GT_U32    ii;
    PRV_CPSS_DXCH_UNIT_ENT unit;
    PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr;
    GT_U32    numOfUnits;

    static const PRV_CPSS_DXCH_UNIT_ENT txQUnitArr[] = {
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E                ,/*Falcon : Txq_SDQ - pipe 0*/
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E                ,
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E                ,
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E                ,
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ0_E                ,/*Falcon : Txq_SDQ - pipe 1*/
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ1_E                ,
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ2_E                ,
        PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ3_E                ,
        PRV_CPSS_DXCH_UNIT_TXQ_PSI_E                        /*Falcon : Txq_PSI */
    };
    numOfUnits = (sizeof(txQUnitArr) / sizeof(txQUnitArr[0]));

    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (beforeSwReset == GT_TRUE)
    {
        /* Zero memory DB before SW reset */
        cpssOsMemSet(txqTableBackUpMemories, 0, sizeof(PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC) * MAX_TILES_NUM * MAX_TXQ_UNITS_NUM);

        if (prvCpssDxChHwInitSoftResetTraceEnable)
        {
            cpssOsPrintf("TXQ registers backup\n");
        }
    }
    else
    {
        /* No error happend */
        if (errorStatus == GT_OK)
        {
#ifdef ASIC_SIMULATION
            rc = cpssSimSoftResetDoneWait();
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
            }
#else
            GT_U32 k = 4000;     /* Wait for a limited time */
            CPSS_HW_PP_INIT_STAGE_ENT  initStage = CPSS_HW_PP_INIT_STAGE_INIT_DURING_RESET_E;
            /* Wait initStage to be fully functional */
            while ((initStage != CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E) && (k > 0))
            {
                cpssOsTimerWkAfter(5);

                rc = cpssDxChHwPpInitStageGet(devNum, &initStage);
                if (rc != GT_OK)
                {
                    return rc;
                }
                k--;
            }
            if (k == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
            }
#endif
        }
        if (prvCpssDxChHwInitSoftResetTraceEnable)
        {
            cpssOsPrintf("TXQ registers restore\n");
        }
    }

    /* Allocate/free memory space for TxQ memories per tile */
    for (tile = 0;  tile == 0 /* support AC5P */ || tile < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tile++)
    {
        /* Iterate TxQ units and allocate memory chunks */
        for (ii = 0; ii < numOfUnits; ii++)
        {
            unit = txQUnitArr[ii];
            unitTableMemoryPtr = &txqTableBackUpMemories[tile][ii];
            if (beforeSwReset == GT_TRUE)
            {
                /* Initialize memory unit chunks */
                rc = prvCpssDxChHwTxQMemWaInit(devNum, tile, unit, unitTableMemoryPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                /* Backup table memory to internal buffer */
                rc = prvCpssDxChHwTxQMemWaBackUp(devNum, unitTableMemoryPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            else
            {
                if (unitTableMemoryPtr->chunksArray == NULL)
                {
                    /* Nothing to restore */
                    break;
                }
                /* No error happend before */
                if (errorStatus == GT_OK)
                {
                    /* Restore internal buffer to PP memory */
                    rc = prvCpssDxChHwTxQMemWaRestore(devNum, unitTableMemoryPtr);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                /* Free memory unit chunks */
                rc = prvCpssDxChHwTxQMemWaFree(unitTableMemoryPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa function
* @endinternal
*
* @brief   Run logic before triggering Falcon CNM System Soff Reset.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[out] doWaAfterResetPtr    - GT_TRUE - need execute WAs after soft reset
*                                    GT_FALSE - need not execute WAs after soft reset
* @param[out] txqTableBackUpMemories - pointer to state DB for the WA
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa
(
    IN  GT_U8                                          devNum,
    OUT GT_BOOL                                       *doWaAfterResetPtr,
    OUT PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC  txqTableBackUpMemories[MAX_TILES_NUM][MAX_TXQ_UNITS_NUM]
)
{
    GT_U32 singleHwDataWord = 0;
    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32  ii;
    GT_U32  rc;
    GT_U32  index;
    GT_U32  numOfEntries;
    GT_BOOL tablesSkipInit;
    GT_BOOL pexSkipInit;

    static const CPSS_DXCH_TABLE_ENT tablesToRead[] = {
         CPSS_DXCH_TABLE_FDB_E
        ,CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E
        ,CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E
        ,CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E
        ,CPSS_DXCH_TABLE_LAST_E/* must be last*/
    };

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
        PRV_CPSS_DXCH_FALCON_SBM_XOR_CLIENTS_READ_WRITE_BEFORE_SW_RESET_WA_E))
    {
        /*  the 2 JIRAs :
            ST-13:      SW reset is not supported for clients using XOR memorySW reset is not supported for clients using XOR memory
            HA-3719:    SW reset is not supported for eport1 target attributes table due to XOR memory implementation
        */
        if (prvCpssDxChHwInitSoftResetTraceEnable)
        {
            cpssOsPrintf("tables read/write ERRATA SBM_XOR_CLIENTS_READ_WRITE_BEFORE_SW_RESET\n");
        }

        /* need to read single entry from all the tables and write back to the same table */
        for(ii = 0; tablesToRead[ii] != CPSS_DXCH_TABLE_LAST_E; ii++)
        {
            tableType = tablesToRead[ii];

            if (tableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E &&
                PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum == 0)
            {
                continue;
            }

            if (tableType == CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E)
            {
                GT_U32 exactMatchNumOfBanks;
                PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(devNum,exactMatchNumOfBanks);
                if(exactMatchNumOfBanks == 0)
                {
                    /* the device to soft reset between 'phase 1' and 'pp logical init'
                      at this time such non-coharent happens , and we not want to fail in fucntion
                      prvCpssDxChReadTableEntryField(...) */
                    continue;
                }
            }


            numOfEntries = (tableType == CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E) ? 5 : 7;
            for (index = 0; index < numOfEntries; index++)
            {
                /* Read single bit of specific index into single word */
                rc = prvCpssDxChReadTableEntryField(devNum,
                                                    tableType,
                                                    index,
                                                    0, 0, 1,
                                                    &singleHwDataWord);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* Write single word to specific index */
                rc = prvCpssDxChWriteTableEntryField(devNum,
                                                     tableType,
                                                     index,
                                                     0, 0, 1,
                                                     singleHwDataWord);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                               PRV_CPSS_DXCH_FALCON_TXQ_SKIP_TABLES_BACKUP_WA_E))
    {
        rc = cpssDxChHwPpSoftResetSkipParamGet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E, &pexSkipInit);
        if(GT_OK != rc)
        {
            return rc;
        }
        rc = cpssDxChHwPpSoftResetSkipParamGet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E, &tablesSkipInit);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Don't skip tables on SW reset */
        if (tablesSkipInit == GT_FALSE || pexSkipInit == GT_FALSE)
        {
            /* No need WA */
            return GT_OK;
        }

        /* Allocate internal buffers and backup table memory */
        rc = prvCpssDxChHwPpSip6TxQMemoryWaRestore(devNum, GT_TRUE, GT_OK, txqTableBackUpMemories);
        if(rc != GT_OK)
        {
            return rc;
        }

        *doWaAfterResetPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6AfterSystemResetWa function
* @endinternal
*
* @brief   Run logic after triggering Falcon CNM System Soff Reset.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] errorStatus           - error status before system restore
* @param[in] txqTableBackUpMemories - pointer to state DB for the WA
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChHwInitSoftResetSip6AfterSystemResetWa
(
    IN  GT_U8                                          devNum,
    IN  GT_STATUS                                      errorStatus,
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC  txqTableBackUpMemories[MAX_TILES_NUM][MAX_TXQ_UNITS_NUM]
)
{
    GT_U32  rc;

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
                                               PRV_CPSS_DXCH_FALCON_TXQ_SKIP_TABLES_BACKUP_WA_E))
    {
        /* Restore and free table memory chunks */
        rc = prvCpssDxChHwPpSip6TxQMemoryWaRestore(devNum, GT_FALSE, errorStatus, txqTableBackUpMemories);
        if(rc != GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChChunkMalloc function
* @endinternal
*
* @brief   This function allocates chunks for TxQ WA memories
*
* @param[in, out] unitTableMemoryPtr
*                                  - (pointer to) unit table internal memory
* @param[in] numOfChunks           - number of chunks
* @param[in] chunksMem             - memory chunks
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
*/
static GT_STATUS prvCpssDxChChunkMalloc
(
    INOUT PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr,
    IN GT_U32  numOfChunks,
    IN PRV_CPSS_MEM_CHUNK_STC chunksMem[]
)
{
    GT_U32  ii;
    PRV_CPSS_DXCH_MEM_CHUNK_INFO_STC *chunkPtr;

    CPSS_NULL_PTR_CHECK_MAC(unitTableMemoryPtr);

    unitTableMemoryPtr->chunksArray = cpssOsMalloc(numOfChunks * sizeof(PRV_CPSS_DXCH_MEM_CHUNK_INFO_STC));
    if (unitTableMemoryPtr->chunksArray == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    unitTableMemoryPtr->numOfChunks = numOfChunks;
    for (ii = 0; ii < numOfChunks; ii++)
    {
        chunkPtr = &unitTableMemoryPtr->chunksArray[ii];
        chunkPtr->memPtr = cpssOsMalloc(chunksMem[ii].memSizeInWords * sizeof(GT_U32));
        if (chunkPtr->memPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        chunkPtr->memFirstAddr = chunksMem[ii].memFirstAddr;
        chunkPtr->memSize = chunksMem[ii].memSizeInWords;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwTxQSdqMemAllocWa function
* @endinternal
*
* @brief   This function allocates memories for TxQ SDQ tables
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] unitTableMemoryPtr    - (pointer to) unit table memory chunks
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate memory
*/
static GT_STATUS prvCpssDxChHwTxQSdqMemAllocWa
(
    IN GT_U8            devNum,
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
)
{
    #define NUM_OF_SDQ_TABLES   3
    GT_U32  numOfChunks = NUM_OF_SDQ_TABLES;

    PRV_CPSS_MEM_CHUNK_STC chunksMem[NUM_OF_SDQ_TABLES];

    chunksMem[0].memFirstAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[0].Port_Config[0];
    chunksMem[0].memSizeInWords = 9;

    chunksMem[1].memFirstAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[0].Port_Range_Low[0];
    chunksMem[1].memSizeInWords = 9;

    chunksMem[2].memFirstAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[0].Port_Range_High[0];
    chunksMem[2].memSizeInWords = 9;

    return prvCpssDxChChunkMalloc(unitTableMemoryPtr, numOfChunks, chunksMem);
}

/**
* @internal prvCpssDxChHwTxQPdqMemAllocWa function
* @endinternal
*
* @brief   This function allocates memory chunks for TxQ PDQ tables
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] ppPhase1ParamsPtr        - Packet processor hardware specific parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate memory
*/
static GT_STATUS prvCpssDxChHwTxQPdqMemAllocWa
(
    IN GT_U8            devNum,
    PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
)
{
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION *tilePtr;

    #define NUM_OF_PDQ_TABLES   57
    #define PDQ_MEM_BASE_ADDRESS_GET(name) addressSpacePtr->Sched.name.l[0]


    GT_U32  numOfChunks;
    GT_U32 ii;
    PRV_CPSS_MEM_CHUNK_STC chunksMem[NUM_OF_PDQ_TABLES] =
    {
        /* PDQ A Level tables */
        { 0,/*AlvlEligPrioFunc_Entry    */  256 },
        { 0,/*AlvlEligPrioFuncPtr       */  256 },
        { 0,/*AlvlTokenBucketTokenEnDiv */  256 },
        { 0,/*AlvlTokenBucketBurstSize  */  256 },
        { 0,/*AlvlDWRRPrioEn            */  256 },
        { 0,/*AlvlQuantum               */  256 },
        { 0,/*ALvltoBlvlAndQueueRangeMap*/  256 },
        { 0,/*AlvlShpBucketLvls         */  256 },
        { 0,/*ALevelShaperBucketNeg     */  4   },
        { 0,/*AlvlDef                   */  256 },
        /* PDQ B Level tables */
        { 0,/*BlvlEligPrioFunc_Entry    */  256 },
        { 0,/*BlvlEligPrioFuncPtr       */  256 },
        { 0,/*BlvlTokenBucketTokenEnDiv */  256 },
        { 0,/*BlvlTokenBucketBurstSize  */  256 },
        { 0,/*BlvlDWRRPrioEn            */  256 },
        { 0,/*BlvlQuantum               */  256 },
        { 0,/*BLvltoClvlAndAlvlRangeMap */  256 },
        { 0,/*BlvlShpBucketLvls         */  256 },
        { 0,/*BLevelShaperBucketNeg     */  4   },
        { 0,/*BlvlDef                   */  256 },
        /* PDQ C Level tables */
        { 0,/*ClvlEligPrioFunc_Entry    */  256 },
        { 0,/*ClvlEligPrioFuncPtr       */  256 },
        { 0,/*ClvlTokenBucketTokenEnDiv */  256 },
        { 0,/*ClvlTokenBucketBurstSize  */  256 },
        { 0,/*ClvlDWRRPrioEn            */  256 },
        { 0,/*ClvlQuantum               */  256 },
        { 0,/*ClvltoPortAndBlvlRangeMap */  256 },
        { 0,/*ClvlShpBucketLvls         */  256 },
        { 0,/*CLevelShaperBucketNeg     */  4   },
        { 0,/*CLvlDef                   */  256 },
        /* PDQ ports tables */
        { 0,/*PortEligPrioFunc_Entry    */  256 },
        { 0,/*PortEligPrioFuncPtr       */  256 },
        { 0,/*PortTokenBucketTokenEnDiv */  256 },
        { 0,/*PortTokenBucketBurstSize  */  256 },
        { 0,/*PortDWRRPrioEn            */  256 },
        { 0,/*PortQuantumsPriosLo       */  254 },
        { 0,/*PortQuantumsPriosHi       */  256 },
        { 0,/*PortRangeMap              */  256 },
        { 0,/*PortShpBucketLvls         */  256 },
        { 0,/*PortShaperBucketNeg       */  8   },
        { 0,/*PortDefPrio0              */  256 },
        { 0,/*PortDefPrio1              */  256 },
        { 0,/*PortDefPrio2              */  256 },
        { 0,/*PortDefPrio3              */  256 },
        { 0,/*PortDefPrio4              */  256 },
        { 0,/*PortDefPrio5              */  256 },
        { 0,/*PortDefPrio6              */  256 },
        { 0,/*PortDefPrio7              */  256 },
        /* PDQ queues tables */
        { 0,/*QueueEligPrioFunc         */  128 },
        { 0,/*QueueEligPrioFuncPtr      */  4096},
        { 0,/*QueueTokenBucketTokenEnDiv*/  4096},
        { 0,/*QueueTokenBucketBurstSize */  4096},
        { 0,/*QueueQuantum              */  4096},
        { 0,/*QueueAMap                 */  4096},
        { 0,/*QueueShpBucketLvls        */  4096},
        { 0,/*QueueShaperBucketNeg      */  128 },
        { 0,/*QueueDef                  */  4096},
    };

    tilePtr =PRV_CPSS_TILE_HANDLE_GET_MAC(devNum,0);
    addressSpacePtr = (struct prvCpssDxChTxqSchedAddressSpace *)(tilePtr->general.addressSpacePtr);

    ii = 0;
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlEligPrioFunc_Entry);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlEligPrioFuncPtr);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlTokenBucketTokenEnDiv);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlTokenBucketBurstSize);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlDWRRPrioEn);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlQuantum);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ALvltoBlvlAndQueueRangeMap);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlShpBucketLvls);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ALevelShaperBucketNeg);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(AlvlDef);

    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlEligPrioFunc_Entry);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlEligPrioFuncPtr);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlTokenBucketTokenEnDiv);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlTokenBucketBurstSize);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlDWRRPrioEn);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlQuantum);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BLvltoClvlAndAlvlRangeMap);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlShpBucketLvls);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BLevelShaperBucketNeg);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(BlvlDef);

    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlEligPrioFunc_Entry);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlEligPrioFuncPtr);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlTokenBucketTokenEnDiv);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlTokenBucketBurstSize);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlDWRRPrioEn);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlQuantum);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvltoPortAndBlvlRangeMap);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(ClvlShpBucketLvls);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(CLevelShaperBucketNeg);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(CLvlDef);

    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortEligPrioFunc_Entry);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortEligPrioFuncPtr);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortTokenBucketTokenEnDiv);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortTokenBucketBurstSize);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDWRRPrioEn);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortQuantumsPriosLo);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortQuantumsPriosHi);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortRangeMap);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortShpBucketLvls);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortShaperBucketNeg);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio0);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio1);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio2);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio3);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio4);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio5);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio6);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(PortDefPrio7);

    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueEligPrioFunc);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueEligPrioFuncPtr);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueTokenBucketTokenEnDiv);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueTokenBucketBurstSize);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueQuantum);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueAMap);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueShpBucketLvls);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueShaperBucketNeg);
    chunksMem[ii++].memFirstAddr = PDQ_MEM_BASE_ADDRESS_GET(QueueDef);

    if (ii != NUM_OF_PDQ_TABLES)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    numOfChunks = NUM_OF_PDQ_TABLES;
    return prvCpssDxChChunkMalloc(unitTableMemoryPtr, numOfChunks, chunksMem);
}

/**
* @internal prvCpssDxChHwTxQMemWaInit function
* @endinternal
*
* @brief   This function allocates and initializes backup/restore memories for TxQ tables
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tile                  - tile number
* @param[in] unit                  - unit ID
* @param[in,out]  unitTableMemoryPtr
*                                  - (pointer to) unit table internal memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate memory
*/
static GT_STATUS prvCpssDxChHwTxQMemWaInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   tile,
    IN PRV_CPSS_DXCH_UNIT_ENT   unit,
    INOUT PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
)
{
    GT_STATUS   rc;
    GT_U32      unitBaseAddr;
    GT_U32      tileOffset;     /* tile offset */
    GT_BOOL     didError;       /* error indication */
    GT_U32      ii;

    CPSS_NULL_PTR_CHECK_MAC(unitTableMemoryPtr);

    switch (unit)
    {
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ1_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ2_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ3_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ0_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ1_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ2_E:
        case PRV_CPSS_DXCH_UNIT_TXQ_PIPE1_SDQ3_E:
            rc = prvCpssDxChHwTxQSdqMemAllocWa(devNum, unitTableMemoryPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PRV_CPSS_DXCH_UNIT_TXQ_PSI_E:
            rc = prvCpssDxChHwTxQPdqMemAllocWa(devNum,unitTableMemoryPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Allign base address for current unit in tile */
    unitBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unit, &didError);
    if(didError == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    tileOffset = prvCpssSip6TileOffsetGet(devNum, tile);
    unitBaseAddr += tileOffset;

    for (ii = 0; ii < unitTableMemoryPtr->numOfChunks; ii++)
    {
        unitTableMemoryPtr->chunksArray[ii].memFirstAddr += unitBaseAddr;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwTxQMemWaBackUp function
* @endinternal
*
* @brief   Backup memories of TxQ tables to internal memory
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] unitTableMemoryPtr    - (pointer to) unit table internal memory
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChHwTxQMemWaBackUp
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      numOfWords;
    GT_U32      *memPtr;
    GT_U32      ii;

    CPSS_NULL_PTR_CHECK_MAC(unitTableMemoryPtr);

    for (ii = 0; ii < unitTableMemoryPtr->numOfChunks; ii++)
    {
        /* First address of memory */
        regAddr = unitTableMemoryPtr->chunksArray[ii].memFirstAddr;
        numOfWords = unitTableMemoryPtr->chunksArray[ii].memSize;
        memPtr = unitTableMemoryPtr->chunksArray[ii].memPtr;

        /* Read memory to internal buffer */
        rc = cpssDrvPpHwRamRead(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, numOfWords, memPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwTxQMemWaRestore function
* @endinternal
*
* @brief   Restore internal memories of TxQ tables to PP memory
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] unitTableMemoryPtr    - (pointer to) unit memory table
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChHwTxQMemWaRestore
(
    IN GT_U8                    devNum,
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      numOfWords;
    GT_U32      *memPtr;
    GT_U32      ii;

    CPSS_NULL_PTR_CHECK_MAC(unitTableMemoryPtr);

    for (ii = 0; ii < unitTableMemoryPtr->numOfChunks; ii++)
    {
        /* First address of memory */
        regAddr = unitTableMemoryPtr->chunksArray[ii].memFirstAddr;
        numOfWords = unitTableMemoryPtr->chunksArray[ii].memSize;
        memPtr = unitTableMemoryPtr->chunksArray[ii].memPtr;
        /* Write memory to PP memory */
        rc = cpssDrvPpHwRamWrite(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, numOfWords, memPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwTxQMemWaFree function
* @endinternal
*
* @brief   This function free internal memories for TxQ tables after SW reset WA
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] unitTableMemoryPtr    - (pointer to) unit table internal memory
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS prvCpssDxChHwTxQMemWaFree
(
    IN PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC *unitTableMemoryPtr
)
{
    GT_U32      ii;
    PRV_CPSS_DXCH_MEM_CHUNK_INFO_STC *chunkPtr;

    CPSS_NULL_PTR_CHECK_MAC(unitTableMemoryPtr);

    for (ii = 0; ii < unitTableMemoryPtr->numOfChunks; ii++)
    {
        chunkPtr = &unitTableMemoryPtr->chunksArray[ii];
        if (chunkPtr->memPtr)
        {
            /* Free table memory pointer */
            cpssOsFree(chunkPtr->memPtr);
        }
    }
    /* Free chunks array */
    cpssOsFree(unitTableMemoryPtr->chunksArray);

    return GT_OK;
}


/**
* @internal cm3CoprocessorsDisable function
* @endinternal
*
* @brief   Disable all CM3 CPUs in Eagle CNMs
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
*/
static GT_STATUS cm3CoprocessorsDisable
(
    IN GT_U8    devNum,
    IN GT_U32   regAddr/*address in MG0*/
)
{
    GT_U32      ii;
    GT_U32      diffFromMg0;
    GT_STATUS   rc;


    GT_U32 numMgs = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;
    if(numMgs == 0)
    {
        numMgs = 1;
    }

    for(ii = 0 ; ii < numMgs; ii=ii+4 /* CM3 is in each forth MG */)
    {
         diffFromMg0 = (ii == 0) ? 0 : prvCpssSip6OffsetFromFirstInstanceGet(devNum,ii,PRV_CPSS_DXCH_UNIT_MG_E);
         rc = prvCpssHwPpSetRegField(devNum, regAddr + diffFromMg0, 28, 2, 0);
         if (rc != GT_OK)
         {
             return rc;
         }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChWriteRegisterAllMGs function
* @endinternal
*
* @brief   duplicate addess from MG0 to all other MGs.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvCpssDxChWriteRegisterAllMGs
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr,/*address in MG0*/
    IN GT_U32 value
)
{
    GT_U32  ii;
    GT_U32  diffFromMg0;

    GT_U32 numMgs = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;
    if(numMgs == 0)
    {
        numMgs = 1;
    }

    for(ii = 0 ; ii < numMgs; ii++)
    {
         diffFromMg0 = (ii == 0) ? 0 : prvCpssSip6OffsetFromFirstInstanceGet(devNum,ii,PRV_CPSS_DXCH_UNIT_MG_E);
         prvCpssHwPpWriteRegister(devNum,regAddr + diffFromMg0,value);
    }

    return GT_OK;
}

/**
* @internal falconMgManualResetRegisters function
* @endinternal
*
* @brief   Falcon : manualy reset some MG registers that are important for logic
*           before the device start reset.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
*/
static GT_STATUS   falconMgManualResetRegisters
(
    IN GT_U8 devNum
)
{
    GT_U32  regAddr,mgBaseAddr;
    GT_U32  data;
    GT_U32  queue;

    mgBaseAddr =  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_generalControl & 0xFFFF0000;

    /* register that are not in the CPSS DB of registers */
    regAddr = mgBaseAddr + 0x00000070;/* user defined register 0 */
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0);

    regAddr = mgBaseAddr + 0x00000074;/* user defined register 1 */
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auqConfig_generalControl;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,0x000f0f0f);

    /* CM3 CPU in MG units get reset during Soft Reset incuding reset of RAM for all devices but not Falcon.
       Force CM3 to be disabled. */
    regAddr = mgBaseAddr + 0x00000500;/* Confi Processor Global Configuration register */
    cm3CoprocessorsDisable(devNum,regAddr);

    /* set resource error mode to HW default.
       This is needed to disable SDMA properly in case "Retry" mode was configured */
    for (queue = 0; queue < 8; queue++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxSdmaResourceErrorCountAndMode[queue];
        /* set register to default value that stops SDMA on resource error */
        data = 0x400;
        prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.rxQCmdReg;
    /* disable all 8 RX SDMA queues in all MGs */
    data = 0xFF00;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txQCmdReg;
    /* disable all 8 TX SDMA queues in all MGs.
       Use data same as for RX SDMA disable. */
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);

    /* wait to make sure that SDMA disabled */
    cpssOsTimerWkAfter(1);

#if 0     /* must not reset it so the prvCpssDxChCfgInitAuqEnableDisable will know to init it properly ! */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQBaseAddr;
    writeRegister_all_MGs(devNum,regAddr,0);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.auQControl;
    writeRegister_all_MGs(devNum,regAddr,1);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQBaseAddr;
    writeRegister_all_MGs(devNum,regAddr,0);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.fuQControl;
    writeRegister_all_MGs(devNum,regAddr,1);
#endif /*0*/


    return GT_OK;
}

/**
* @internal prvCpssDxChManualMgSdmaReset function
* @endinternal
*
* @brief   This function is doing sdma reset.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS  prvCpssDxChManualMgSdmaReset
(
    IN GT_U8 devNum
)
{
    GT_U32  regAddr;
    GT_U32  data;
    GT_STATUS rc;

    /* make sure that Soft Reset done */
    cpssOsTimerWkAfter(10);

    /* toggle SDMA SW Reset bit Extended Global Control registers of all MGs */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.extendedGlobalControl;
    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set bit 6 to Reset SDMAs */
    data |= BIT_6;
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);

    /* reset bit 6 to place SDMAs in normal state */
    data &= (~BIT_6);
    prvCpssDxChWriteRegisterAllMGs(devNum,regAddr,data);


    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger function
* @endinternal
*
* @brief   Trigger Falcon CNM System Soft Reset.
*          .
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6CnmSystemResetTrigger
(
    IN  GT_U8                                          devNum
)
{
    GT_U32    unitBase; /* base address of unit */
    GT_BOOL   error;    /* error indication     */
    GT_STATUS rc;       /* return code          */
    GT_U32    regAddr;  /* address of register  */
    GT_U32    dbRegAddr;    /* address of register from DB        */
    GT_U32    triggerValue; /* value of bit to trigger Soft Reset */
    GT_U32    tileIndex; /* index of tile           */
    GT_U32    gopIndex;  /* index of GOP device     */
    GT_U32    mask;      /* mask for register write */
    GT_U32    data;      /* data of register        */
    PRV_CPSS_RESET_AND_INIT_CTRL_REGS_ADDR_STC *dfxRegsAddrPtr; /* pointer to registers DB */
    GT_BOOL   doWaAfterReset = GT_FALSE; /* indication for WA after soft reset */
    GT_BOOL   isPexSkiped = GT_FALSE; /* indication for Skip PCI_e reset */
    GT_U32    ravenId;     /* index of raven device */
    GT_U32    resetSrcBit; /* bit related to source of soft reset */
    GT_U32    pci_e_dfxPipe; /* DFX Pipe of PCI_e unit */
    /* Internal memory DB. Holds data used during single call of cpssDxChHwPpSoftResetTrigger and protected by usual locking procedure.
       No extra protection and export/import required for this data */
    PRV_CPSS_DXCH_HW_TABLES_MEMORY_WA_STC  txqTableBackUpMemories[MAX_TILES_NUM][MAX_TXQ_UNITS_NUM];

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* use MNG source of Soft Reset - bit# 1 in "skip" registers */
    resetSrcBit = 1;
    dfxRegsAddrPtr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        rc = cpssDxChHwPpSoftResetSkipParamGet(devNum,CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,&isPexSkiped);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* when the PEX skipped then also the MG is skipped */
        /* there are some registers that we need to manually 'restore' them */
        /* and we can do it even before the reset it self */
        if(isPexSkiped)
        {
            rc = falconMgManualResetRegisters(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /* AC5X has internal CPUs and CPU subsystem (CNM). need to configure multiple skips to
       avoid reset of CNM and CPUs */
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        /* enable SKIP init for CPU */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.CPUSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* enable SKIP init for DFX */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* enable SKIP init for CNM */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.cnmSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Need to set skip CNM RAMs related pipe#4  */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 4, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Enable DFX Pipe skip for "MG" Reset source */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 23+resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Avoid toggle of device_enabled_and_init_done */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.metalFixRegister;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 26, 1, 1);
        if(GT_OK != rc)
        {
            return rc;
        }

        /* Skip MPP reset to avoid CPU subsystem disruption */
        regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.mppSkipInitializationMatrix;
        rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, resetSrcBit, 1, 0);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        /* AC5P, Harrier */
        rc = cpssDxChHwPpSoftResetSkipParamGet(devNum,CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,&isPexSkiped);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* need to avoid PCI_e unit RAMs initialization during Soft Reset. */
        if(isPexSkiped)
        {
            pci_e_dfxPipe = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ? 1 : 5;

            /* Need to set skip CNM RAMs related DFX pipe  */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, pci_e_dfxPipe, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* Enable DFX Pipe skip for "MG" Reset source */
            regAddr = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.DFXPipeSkipInitializationMatrix;
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 23+resetSrcBit, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    /* Run WA logic before system reset */
    rc = prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa(devNum,&doWaAfterReset,txqTableBackUpMemories);
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChHwInitSoftResetSip6BeforeSystemResetWa failed rc[%]", rc);
        goto cleanUp;
    }

    triggerValue   = 0;
    dbRegAddr      = dfxRegsAddrPtr->DFXServerUnitsDeviceSpecificRegs.deviceResetCtrl;
    for (tileIndex = 0;
          (tileIndex == 0/* support AC5P */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        /* Eagle DFX use External Soft Reset MNG signal for Soft Reset treatment.
           Need to enable this signal by the configuration in Server Reset Control register:
           bit 4  - External Soft Reset Mask mng - set to 0
           bit 12 - External Soft Reset Edge Select mng - set to triggerValue */
        mask           = ((1 << 12) | (1 << 4));
        data           = (triggerValue << 12);
        rc = prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
            devNum, PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E, tileIndex,
            0/*gopIndex*/, dbRegAddr, mask, data);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC(
                "prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask failed rc[%]", rc);
            goto cleanUp;
        }

        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)
        {
            /* no Raven */
            continue;
        }

        for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
        {
            if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
            {
                ravenId = 3 - gopIndex;
            }
            else
            {
                ravenId = gopIndex;
            }

            PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

            /* GOP (Raven) DFX use External Soft Reset SRN signal for Soft Reset treatment.
               Need to enable this signal by the configuration in Server Reset Control register:
               bit 3  - External Soft Reset Mask srn - set to 0
               bit 11 - External Soft Reset Edge Select srn - set to triggerValue */
            mask           = ((1 << 11) | (1 << 3));
            data           = (triggerValue << 11);
            rc = prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask(
                devNum, PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E, tileIndex,
                gopIndex, dbRegAddr, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC(
                    "prvCpssDxChHwInitSoftResetSip6DfxRegWriteBitMask failed rc[%]", rc);
                goto cleanUp;
            }
        }
    }

    error = GT_FALSE;
    unitBase = prvCpssDxChHwUnitBaseAddrGet(devNum, PRV_CPSS_DXCH_UNIT_CNM_RFU_E, &error);
    if (error != GT_FALSE)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDxChHwUnitBaseAddrGet failed rc[%]", rc);
        goto cleanUp;
    }

    /*regAddr - System Soft Reset*/
    regAddr = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ? 0x14 : 0x44;
    regAddr += unitBase;

    /* 0-th CNM unit connected to all tiles */
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0 /*portGroupId*/, regAddr, 1/*mask*/, ((~ triggerValue) & 1));
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDrvHwPpPortGroupWriteRegBitMask failed rc[%]", rc);
        goto cleanUp;
    }
    rc = prvCpssDrvHwPpPortGroupWriteRegBitMask(
        devNum, 0 /*portGroupId*/, regAddr, 1/*mask*/, (triggerValue & 1));
    if (rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("prvCpssDrvHwPpPortGroupWriteRegBitMask failed rc[%]", rc);
        goto cleanUp;
    }


    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&
       isPexSkiped)
    {
        /* do SDMA reset  */
        rc = prvCpssDxChManualMgSdmaReset(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* state to the special DB that the device did HW reset */
    prvCpssPpConfigDevDbHwResetSet(devNum,GT_TRUE);

cleanUp:

    if (doWaAfterReset)
    {
        /* Run WA logic after system reset */
        rc = prvCpssDxChHwInitSoftResetSip6AfterSystemResetWa(devNum, rc, txqTableBackUpMemories);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in all Falcon Tiles and Ravens relvant to given Skip Init Matrix.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6AllSubunitsSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS                               rc;
    GT_BOOL                                 perGop;
    GT_BOOL                                 perTile;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT   dfxType;
    GT_U32                                  tileIndex;
    GT_U32                                  gopIndex;
    GT_U32                                  ravenId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_TILE_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_MNG_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_PCIE_E:
            perGop   = GT_FALSE;
            perTile  = GT_TRUE;
            break;
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_MAIN_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_CNM_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_GOP_GW_E:
            perGop   = GT_TRUE;
            perTile  = GT_FALSE;
            break;
        default:
            perGop   = GT_TRUE;
            perTile  = GT_TRUE;
            break;
    }

    for (tileIndex = 0; (tileIndex == 0/* support AC5P */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        if (perTile != GT_FALSE)
        {
            dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_CORE_E;
            gopIndex = 0;
            rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        if(0 == PRV_CPSS_DXCH_PP_HW_INFO_RAVENS_MAC(devNum).numOfRavens)
        {
            /* no Raven */
            continue;
        }
        if (perGop != GT_FALSE)
        {
            for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
            {
                if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
                {
                    ravenId = 3 - gopIndex;
                }
                else
                {
                    ravenId = gopIndex;
                }

                PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

                dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E;
                rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                    devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask function
* @endinternal
*
* @brief   Write bit mask to given Skip Init Matrix register instance
*          in given Ravens Set.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] gopBitmap             - bitmap of GOP global indexes
*                                    CPSS_CHIPLETS_UNAWARE_MODE_CNS for all chiplets of the device
* @param[in] skipType              - the type of Skip Initialization Marix
*                                    see comments in PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT.
*                                    Skip types relevant to Main Die only ignored. GT_OK returned.
* @param[in] mask                  - Mask for selecting the written bits.
* @param[in] value                 - Data to be written to register.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong devNum, or tileIndex
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChHwInitSoftResetSip6GopSetSkipInitMatrixWriteBitMask
(
    IN  GT_U8                                          devNum,
    IN  GT_CHIPLETS_BMP                                gopBitmap,
    IN  PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_ENT  skipType,
    IN  GT_U32                                         mask,
    IN  GT_U32                                         data
)
{
    GT_STATUS                               rc;
    PRV_CPSS_DXCH_SOFT_RESET_DFX_TYPE_ENT   dfxType;
    GT_U32                                  tileIndex;
    GT_U32                                  gopIndex;
    GT_U32                                  gopGlobalIndex;
    GT_U32                                  ravenId;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }
    if (gopBitmap == CPSS_CHIPLETS_UNAWARE_MODE_CNS)
    {
        gopBitmap = 0xFFFFFFFF; /* rize all bits */
    }
    switch (skipType)
    {
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_TILE_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_MNG_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_D2D_E:
        case PRV_CPSS_DXCH_SOFT_RESET_SKIP_INIT_MATRIX_CORE_RAVEN_E:
            return GT_OK; /* ignored for GOP, relevant to Tile */
        default: break;
    }

    dfxType = PRV_CPSS_DXCH_SOFT_RESET_DFX_GOP_E;
    for (tileIndex = 0; (tileIndex == 0/* support AC5P */ || tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles); tileIndex++)
    {
        for (gopIndex = 0; (gopIndex < FALCON_RAVENS_PER_TILE); gopIndex++)
        {
            gopGlobalIndex =
                ((PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * tileIndex)
                 + gopIndex);
            if ((gopBitmap & (1 << gopGlobalIndex)) == 0) continue;

            if((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
            {
                ravenId = 3 - gopIndex;
            }
            else
            {
                ravenId = gopIndex;
            }

            PRV_CPSS_SKIP_NOT_EXIST_RAVEN_MAC(devNum, tileIndex, ravenId);

            rc = prvCpssDxChHwInitSoftResetSip6SkipInitMatrixWriteBitMask(
                devNum, dfxType, tileIndex, gopIndex, skipType, mask, data);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}



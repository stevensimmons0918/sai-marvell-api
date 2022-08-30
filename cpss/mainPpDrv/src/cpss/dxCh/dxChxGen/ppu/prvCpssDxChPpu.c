/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChPpu.c
*
* @brief Private CPSS CH PPU API implementation
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpuLog.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssCommon/cpssFormatConvert.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal prvCpssDxChPpuEnableSet function
* @endinternal
*
* @brief    This function enables or disables PPU block
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] enable                 - 0 = disable, 1 = enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPpuEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
    GT_STATUS rc;           /* return code */
    GT_U32    regAddr;      /* register address */

    /* get address for global config ext 2 register */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt2;

    /* write enable bit field in global config ext2 register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 27, 1, (GT_U32)enable);

    return rc;
}

/**
* @internal prvCpssDxChPpuEnableGet function
* @endinternal
*
* @brief    This function gets ppu enable status
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] enablePtr             - pointer to enable status. 0 = disable, 1 = enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChPpuEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data    */

    /* get address for global config ext 2 register */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt2;

    /* get enable bit field for global config register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 27, 1, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = (GT_BOOL) (regData & 0x1);

    return rc;
}

/**
* @internal prvCpssDxChPpuMaxLoopbackSet function
* @endinternal
*
* @brief    This function sets the maximum allowed loopbacks
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] maxLoopback                    - maximum number of loopbacks allowed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChPpuMaxLoopbackSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxLoopback
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */

    /* get address for global config register */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).generalRegs.ppuGlobalConfig;

    /* write loopback bit field in global config register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_SIZE, (GT_U32)maxLoopback);

    return rc;
}

/**
* @internal prvCpssDxChPpuMaxLoopbackGet function
* @endinternal
*
* @brief    This function gets the maximum allowed loopbacks
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] maxLoopbackPtr        - pointer to maximum number of loopbacks allowed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDxChPpuMaxLoopbackGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *maxLoopbackPtr
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data    */

    /* get address for global config register */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).generalRegs.ppuGlobalConfig;

    /* read loopback bit field from global config register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_SIZE, &regData);

    *maxLoopbackPtr = regData & 0x7; /* 3 bits */

    return rc;
}

/**
* @internal prvCpssDxChPpuTtiActionProfileIndexSet function
* @endinternal
*
* @brief    set ppu profile index in tti action
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum              - device number
* @param[in] ttiRuleIndex        - PPU profile index pointer by TTI action.
* @param[in] profileNum          - PPU profile table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuTtiActionProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    IN  GT_U32                              profileNum
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   entryNumber = 0;
    GT_U32                   dummy;
    GT_U32                   hwTtiActionArray[8];  /* TTI_ACTION_TYPE_2_SIZE_CNS = 8 */

    /* convert rule global index to TCAM entry number */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, ttiRuleIndex, &entryNumber, &dummy);
    if (rc != 0)
    {
        return rc;
    }

    /* Write ppu profile to TTI action entry */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                        (entryNumber/2), hwTtiActionArray);
    if (rc != 0)
    {
        return rc;
    }

    rc = prvCpssFieldValueSet(hwTtiActionArray, 250, 4, profileNum);
    if (rc != 0)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntry(devNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                        (entryNumber/2), hwTtiActionArray);
    if (rc != 0)
    {
        return rc;
    }

    return rc;
}


/**
* @internal prvCpssDxChPpuTtiActionProfileIndexGet function
* @endinternal
*
* @brief    Get ppu profile index in tti action
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ttiRuleIndex           - PPU profile index pointer by TTI action.
* @param[out] profileNumPtr         - pointer to PPU profile table index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuTtiActionProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    OUT GT_U32                              *profileNumPtr
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   entryNumber = 0;
    GT_U32                   dummy;
    GT_U32                   hwTtiActionArray[8];  /* TTI_ACTION_TYPE_2_SIZE_CNS = 8 */

    /* convert rule global index to TCAM entry number */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, ttiRuleIndex, &entryNumber, &dummy);
    if (rc != 0)
    {
        return rc;
    }

    /* Write ppu profile to TTI action entry */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                        (entryNumber/2), hwTtiActionArray);
    if (rc != 0)
    {
        return rc;
    }

    rc = prvCpssFieldValueGet(hwTtiActionArray, 250, 4, profileNumPtr);

    return rc;
}

/**
* @internal prvcpssDxChPpuSrcPortProfileIndexSet function
* @endinternal
*
* @brief  Set default PPU profile index for source port
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] srcPortNum             - source port number
* @param[in] profileNum             - ppu profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvcpssDxChPpuSrcPortProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    IN  GT_U32                              profileNum
)
{
    GT_STATUS rc = GT_OK;

    /* set default PPU Profile Idx in TTI physical port attribute table */
    rc = prvCpssDxChWriteTableEntryField(devNum,
             CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
             srcPortNum,
             PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
             SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PPU_PROFILE_E, /* field name */
             PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
             profileNum);

    return rc;
}

/**
* @internal prvCpssDxChPpuSrcPortProfileIndexGet  function
* @endinternal
*
* @brief  Get default PPU profile index for source port
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] srcPortNum             - source port number
* @param[out] profileNumPtr          - pointer to ppu profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuSrcPortProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    OUT GT_U32                              *profileNumPtr
)
{
    GT_STATUS rc = GT_OK;

    /* read default PPU Profile Idx in TTI physical port attribute table */
    rc = prvCpssDxChReadTableEntryField(devNum,
                 CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                 srcPortNum,
                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                 SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PPU_PROFILE_E, /* field name */
                 PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                 profileNumPtr);

    return rc;
}

/**
* @internal prvCpssDxChPpuGlobalConfigSet function
* @endinternal
*
* @brief    This function sets the Global configuration Register
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] ppuGlobalConfigPtr     - pointer to global configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuGlobalConfigSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PPU_GLOBAL_CONFIG_STC     *ppuGlobalConfigPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData = 0;      /* register data    */

    /* get address for global config register */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).generalRegs.ppuGlobalConfig;

    /* create 32 bit register data from global config structure*/
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_GLOBAL_REG_PPU_ENABLE_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_PPU_ENABLE_SIZE, 1 /*internal*/);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_SIZE,ppuGlobalConfigPtr->maxLoopBack);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_MAX_LOOPBACK_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_MAX_LOOPBACK_SIZE, ppuGlobalConfigPtr->errProfMaxLoopBack);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_SER_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_SER_SIZE, ppuGlobalConfigPtr->errProfSer);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_OFFSET_ORR_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_OFFSET_ORR_SIZE, ppuGlobalConfigPtr->errProfOffsetOor);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_GLOBAL_DEBUG_COUNTER_ENABLE_OFFSET,
            CPSS_DXCH_PPU_GLOBAL_DEBUG_COUNTER_ENABLE_SIZE, ppuGlobalConfigPtr->debugCounterEnable);

    /* write to global config register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal prvCpssDxChPpuGlobalConfigGet function
* @endinternal
*
* @brief    This function gets the Global configuration Register
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] ppuGlobalConfigPtr    - pointer to global configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuGlobalConfigGet
(
    IN  GT_U8                           devNum,
    OUT CPSS_DXCH_PPU_GLOBAL_CONFIG_STC *ppuGlobalConfigPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData = 0; /* register data    */

    /* get address for global config register */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).generalRegs.ppuGlobalConfig;

    /* read global config register value */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    ppuGlobalConfigPtr->maxLoopBack        = U32_GET_FIELD_MAC(regData,
                                                CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_OFFSET,
                                                CPSS_DXCH_PPU_GLOBAL_REG_MAX_LOOPBACK_SIZE);
    ppuGlobalConfigPtr->errProfMaxLoopBack = U32_GET_FIELD_MAC(regData,
                                                CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_MAX_LOOPBACK_OFFSET,
                                                CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_MAX_LOOPBACK_SIZE);
    ppuGlobalConfigPtr->errProfSer         = U32_GET_FIELD_MAC(regData,
                                                CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_SER_OFFSET,
                                                CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_SER_SIZE);
    ppuGlobalConfigPtr->errProfOffsetOor   = U32_GET_FIELD_MAC(regData,
                                                CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_OFFSET_ORR_OFFSET,
                                                CPSS_DXCH_PPU_GLOBAL_REG_ERR_PROFILE_OFFSET_ORR_SIZE);
    ppuGlobalConfigPtr->debugCounterEnable = U32_GET_FIELD_MAC(regData,
                                                CPSS_DXCH_PPU_GLOBAL_DEBUG_COUNTER_ENABLE_OFFSET,
                                                CPSS_DXCH_PPU_GLOBAL_DEBUG_COUNTER_ENABLE_SIZE);

    return rc;

}

/**
* @internal prvCpssDxChPpuProfileSet function
* @endinternal
*
* @brief    This function sets PPU profile table entry for a given profile index.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - PPU profile index
* @param[in] ppuProfilePtr          - pointer to PPU profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    IN  CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    GT_STATUS   rc;                 /* return code      */
    GT_U32      regAddr;            /* register address */
    GT_U32      regData = 0;        /* register data    */

    /* get address for tti ppu profile register */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).ppuProfile[profileNum];

    /* create 32 bit register data from tti ppu profile structure*/
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_ENABLE_OFFSET,
       CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_ENABLE_SIZE, ppuProfilePtr->ppuEnable);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_STATE_OFFSET,
       CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_STATE_SIZE, ppuProfilePtr->ppuState);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_OFFSET,
       CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_SIZE, ppuProfilePtr->anchorType);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_OFFSET_OFFSET,
       CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_OFFSET_SIZE, ppuProfilePtr->offset);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_PROFILE_ENTRY_SP_BUS_PROFILE_INDEX_OFFSET,
       CPSS_DXCH_PPU_PROFILE_ENTRY_SP_BUS_PROFILE_INDEX_SIZE, ppuProfilePtr->spBusDefaultProfile);

    /* write to tti ppu profile register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal prvCpssDxChPpuProfileGet function
* @endinternal
*
* @brief    This function gets PPU profile table entry for a given profile index.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - PPU profile index
* @param[out] ppuProfilePtr         - pointer to PPU profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data    */

    /* get address for tti ppu profile register */
    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).ppuProfile[profileNum];

    /* read tti ppu profile register value */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    ppuProfilePtr->ppuEnable           = U32_GET_FIELD_MAC(regData,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_ENABLE_OFFSET,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_ENABLE_SIZE);
    ppuProfilePtr->ppuState            = U32_GET_FIELD_MAC(regData,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_STATE_OFFSET,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_STATE_SIZE);
    ppuProfilePtr->anchorType          = U32_GET_FIELD_MAC(regData,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_OFFSET,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_SIZE);
    ppuProfilePtr->offset              = U32_GET_FIELD_MAC(regData,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_OFFSET_OFFSET,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_PPU_OFFSET_SIZE);
    ppuProfilePtr->spBusDefaultProfile = U32_GET_FIELD_MAC(regData,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_SP_BUS_PROFILE_INDEX_OFFSET,
                                            CPSS_DXCH_PPU_PROFILE_ENTRY_SP_BUS_PROFILE_INDEX_SIZE);

    return rc;
}

/**
* @internal prvCpssDxChPpuSpBusDefaultProfileSet function
* @endinternal
*
* @brief    This function sets SP_BUS default profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[in] spBusProfilePtr        - pointer to SP_BUS profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuSpBusDefaultProfileSet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    IN  CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      itr;         /* iterator         */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data    */

    for(itr = 0; itr < CPSS_DXCH_PPU_SP_BUS_WORDS_MAX_CNS; itr ++)
    {
        /* get address for SP_BUS profile word */
        regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).spBusDefaultProfile[profileNum].word[itr];

        /* create register data from SP_BUS profile data */
        regData  = spBusProfilePtr->data[itr*4 + 0] <<  0;
        regData |= spBusProfilePtr->data[itr*4 + 1] <<  8;
        regData |= spBusProfilePtr->data[itr*4 + 2] << 16;
        regData |= spBusProfilePtr->data[itr*4 + 3] << 24;

        /* write SP_BUS profile word*/
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPpuSpBusDefaultProfileGet function
* @endinternal
*
* @brief    This function gets SP_BUS default profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[out] spBusProfilePtr       - pointer to SP_BUS profile data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuSpBusDefaultProfileGet
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   profileNum,
    OUT CPSS_DXCH_PPU_SP_BUS_STC *spBusProfilePtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      itr;         /* iterator         */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data    */

    for(itr = 0; itr < CPSS_DXCH_PPU_SP_BUS_WORDS_MAX_CNS; itr ++)
    {
        /* get address for SP_BUS profile word */
        regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).spBusDefaultProfile[profileNum].word[itr];

        /* read register value for SP_BUS profile word */
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* parse register data and populate SP_BUS proifle structure */
        spBusProfilePtr->data[itr*4 + 0] = (GT_U8)((regData >>  0) & 0xFF);
        spBusProfilePtr->data[itr*4 + 1] = (GT_U8)((regData >>  8) & 0xFF);
        spBusProfilePtr->data[itr*4 + 2] = (GT_U8)((regData >> 16) & 0xFF);
        spBusProfilePtr->data[itr*4 + 3] = (GT_U8)((regData >> 24) & 0xFF);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPpuKstgKeyGenProfileSet function
* @endinternal
*
* @brief    This function sets K_STG key generation profile for a given profile index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] profileNum             - Key generation profile index
* @param[in] keygenProfilePtr       - pointer to key generation profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuKstgKeyGenProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileNum,
    IN  CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)
{
    GT_STATUS   rc = GT_OK;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data    */
    GT_U32      itr;         /* iterator         */

    for(itr = 0; itr < CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS; itr++)
    {
        /* get register address for key generation profile byte select offset*/
        regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].keyGenProfile[profileNum].byte[itr];

        regData = (GT_U32) keygenProfilePtr->byteSelOffset[itr];

        /* write key generation profile byte select offset */
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);
    }

    return rc;
}

/**
* @internal prvCpssDxChPpuKstgKeyGenProfileGet function
* @endinternal
*
* @brief    This function gets K_STG key generation profile for a given profile number.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] profileNum             - Key generation profile index
* @param[out] keygenProfilePtr      - pointer to key generation profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuKstgKeyGenProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  kstgNum,
    IN  GT_U32                                  profileNum,
    OUT CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)
{
    GT_STATUS   rc = GT_OK;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data    */
    GT_U32      itr;         /* iterator         */

    for(itr = 0; itr < CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS; itr++)
    {
        /* get register address for key generation profile byte select offset*/
        regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].keyGenProfile[profileNum].byte[itr];

        /* read key generation profile byte select offset */
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);

        keygenProfilePtr->byteSelOffset[itr] = (GT_U8)regData;
    }
    return rc;
}

/**
* @internal prvCpssDxChPpuKstgTcamEntrySet function
* @endinternal
*
* @brief    This function sets K_STG TCAM entry for a given index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - tcam entry index
* @param[in] tcamEntryPtr           - pointer to tcam entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuKstgTcamEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileNum,
    IN  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regAddr;     /* register address */

    /* Get address and write tcam entry key least significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].keyLsb;
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, tcamEntryPtr->keyLsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and write tcam entry key most significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].keyMsb;
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, tcamEntryPtr->keyMsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and write tcam entry mask least significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].maskLsb;
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, tcamEntryPtr->maskLsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and write tcam entry mask most significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].maskMsb;
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, tcamEntryPtr->maskMsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and write tcam entry key valid bit */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].validate;
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, (GT_U32) tcamEntryPtr->isValid);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal prvCpssDxChPpuKstgTcamEntryGet function
* @endinternal
*
* @brief    This function gets K_STG TCAM entry for a given index
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - tcam entry index
* @param[out] tcamEntryPtr          - pointer to tcam entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuKstgTcamEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              profileNum,
    OUT CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data    */

    /* Get address and read tcam entry key least significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].keyLsb;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &tcamEntryPtr->keyLsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and read tcam entry key most significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].keyMsb;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &tcamEntryPtr->keyMsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and read tcam entry mask least significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].maskLsb;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &tcamEntryPtr->maskLsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and read tcam entry mask most significant bytes */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].maskMsb;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &tcamEntryPtr->maskMsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get address and read tcam entry valid bit */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).kstg[kstgNum].tcamKey[profileNum].validate;
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    tcamEntryPtr->isValid =  (GT_BOOL) (regData & 0x1);

    return rc;
}

/**
* @internal prvCpssDxChPpuActionTableEntrySpBusOverlapCheck function
* @endinternal
*
* @brief    This function checks for the overlap in SP bus write
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] ppuActionEntryPtr      - pointer to PPU action profile entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
*
* @note NONE
*
*/
static GT_STATUS prvCpssDxChPpuActionTableEntrySpBusOverlapCheck
(
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    CPSS_DXCH_PPU_SP_BUS_STC spBusProfile;  /* SP bus profile   */
    GT_U32                       nRot;          /* iterator for ROTs */
    GT_U32                       nByte;         /* iterator for SP bus bytes */

    cpssOsMemSet(&spBusProfile, 0, sizeof(CPSS_DXCH_PPU_SP_BUS_STC));

    for(nRot = 0; nRot < CPSS_DXCH_PPU_KSTG_ROTS_MAX_CNS; nRot++)
    {
        if(ppuActionEntryPtr->rotActionEntry[nRot].target == (GT_BOOL)CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E)
        {
            if(ppuActionEntryPtr->rotActionEntry[nRot].ldSpBusOffset
                    + ppuActionEntryPtr->rotActionEntry[nRot].ldSpBusNumBytes > CPSS_DXCH_PPU_SP_BUS_BYTES_MAX_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            /* check for overlap */
            for(nByte = ppuActionEntryPtr->rotActionEntry[nRot].ldSpBusOffset;
                    nByte < (ppuActionEntryPtr->rotActionEntry[nRot].ldSpBusOffset
                            + ppuActionEntryPtr->rotActionEntry[nRot].ldSpBusNumBytes); nByte++)
            {
                if(spBusProfile.data[nByte] != 0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                spBusProfile.data[nByte] = 1;
            }
        }
    }


    return GT_OK;
}

/**
* @internal prvCpssDxChPpuActionTableEntrySet function
* @endinternal
*
* @brief    This function writes PPU action table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - entry index
* @param[in] ppuActionEntryPtr      - pointer to PPU action profile entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuActionTableEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      i;           /* iterator         */
    GT_U32      offset;      /* bit offset      */

    GT_U32      hwData[CPSS_DXCH_PPU_ACTION_TABLE_ENTRY_WIDTH_IN_WORDS_CNS]; /* table entry data */

    rc = prvCpssDxChPpuActionTableEntrySpBusOverlapCheck(ppuActionEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* create hardware action table entry data from action entry structure */
    offset = 0;
    for(i = 0; i < CPSS_DXCH_PPU_KSTG_ROTS_MAX_CNS; i++)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 0, 2,
                ppuActionEntryPtr->rotActionEntry[i].srcRegSel);
        if(ppuActionEntryPtr->rotActionEntry[i].srcRegSel
                == CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E)
        {
            U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 2, 32,
                    ppuActionEntryPtr->rotActionEntry[i].setCmd4Byte);
        }
        else
        {
            U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 2, 8,
                    ppuActionEntryPtr->rotActionEntry[i].srcOffset);
            U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 10, 5,
                    ppuActionEntryPtr->rotActionEntry[i].srcNumValBits);
            if(ppuActionEntryPtr->rotActionEntry[i].func == CPSS_DXCH_PPU_KSTG_ROT_FUNC_CMP_REG_E)
            {
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 15, 8,
                        ppuActionEntryPtr->rotActionEntry[i].cmpVal);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 23, 8,
                        ppuActionEntryPtr->rotActionEntry[i].cmpFalseValLd);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 31, 2,
                        ppuActionEntryPtr->rotActionEntry[i].cmpCond);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 36, 8,
                        ppuActionEntryPtr->rotActionEntry[i].cmpMask);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 44, 8,
                        ppuActionEntryPtr->rotActionEntry[i].cmpTrueValLd);
            }
            else
            {
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 15, 1,
                        (GT_U32)ppuActionEntryPtr->rotActionEntry[i].shiftLeftRightSel);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 16, 3,
                        ppuActionEntryPtr->rotActionEntry[i].shiftNum);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 19, 4,
                        ppuActionEntryPtr->rotActionEntry[i].setBitsNum);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 23, 8,
                        ppuActionEntryPtr->rotActionEntry[i].setBitsVal);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 31, 5,
                        ppuActionEntryPtr->rotActionEntry[i].srcOffset);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 36, 16,
                        (GT_U32)ppuActionEntryPtr->rotActionEntry[i].addSubConst);
                U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 52, 1,
                        (GT_U32)ppuActionEntryPtr->rotActionEntry[i].addSubConstSel);
            }
        }
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 53, 2,
                ppuActionEntryPtr->rotActionEntry[i].func);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 55, 3,
                ppuActionEntryPtr->rotActionEntry[i].funcSecondOperand);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 58, 1,
                (GT_U32)ppuActionEntryPtr->rotActionEntry[i].target);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 59, 3,
                ppuActionEntryPtr->rotActionEntry[i].ldSpBusNumBytes);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 62, 5,
                ppuActionEntryPtr->rotActionEntry[i].ldSpBusOffset);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 67, 1,
                (GT_U32)ppuActionEntryPtr->rotActionEntry[i].interrupt);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 68, 3,
                ppuActionEntryPtr->rotActionEntry[i].interruptIndex);

        offset += 71;
    }

    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 355, 8,
            ppuActionEntryPtr->setNextState);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 363, 7,
            ppuActionEntryPtr->setConstNextShift);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 370, 1,
            (GT_U32)ppuActionEntryPtr->setLoopBack);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 371, 1,
            (GT_U32)ppuActionEntryPtr->counterSet);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 372, 5,
            ppuActionEntryPtr->counterSetVal);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 377, 2,
            ppuActionEntryPtr->counterOper);

    /* write action entry in ppu action table*/
    rc = prvCpssDxChWriteTableEntry(devNum,
            (CPSS_DXCH_TABLE_ENT)((GT_U32)CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E + kstgNum),
            index, &hwData[0]);

    return rc;
}

/**
* @internal prvCpssDxChPpuActionTableEntryGet function
* @endinternal
*
* @brief    This function reads PPU action table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] kstgNum                - K_stg number
* @param[in] index                  - entry index
* @param[out] ppuActionEntryPtr     - pointer to PPU action profile entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuActionTableEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              kstgNum,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regData = 0;     /* register data    */
    GT_U32      i;         /* iterator         */
    GT_U32      offset;

    GT_U32      hwData[CPSS_DXCH_PPU_ACTION_TABLE_ENTRY_WIDTH_IN_WORDS_CNS]; /* table entry data */

    /* read ppu action table enty */
    rc = prvCpssDxChReadTableEntry(devNum,
            (CPSS_DXCH_TABLE_ENT)((GT_U32)CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E + kstgNum),
            index, &hwData[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* populate ppu action entry datastructure from hardware action table entry data */
    offset = 0;
    for(i = 0; i < CPSS_DXCH_PPU_KSTG_ROTS_MAX_CNS; i++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 0, 2, regData);
        ppuActionEntryPtr->rotActionEntry[i].srcRegSel = regData & BIT_MASK_MAC(2);

        if(ppuActionEntryPtr->rotActionEntry[i].srcRegSel
                == CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 2, 32, regData);
            ppuActionEntryPtr->rotActionEntry[i].setCmd4Byte = regData;
        }
        else
        {
            U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 2, 8, regData);
            ppuActionEntryPtr->rotActionEntry[i].srcOffset = regData & BIT_MASK_MAC(8);
            U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 10, 5, regData);
            ppuActionEntryPtr->rotActionEntry[i].srcNumValBits = regData & BIT_MASK_MAC(5);

            if(ppuActionEntryPtr->rotActionEntry[i].func == CPSS_DXCH_PPU_KSTG_ROT_FUNC_CMP_REG_E)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 15, 8, regData);
                ppuActionEntryPtr->rotActionEntry[i].cmpVal = regData & BIT_MASK_MAC(8);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 23, 8, regData);
                ppuActionEntryPtr->rotActionEntry[i].cmpFalseValLd = regData & BIT_MASK_MAC(8);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 31, 2, regData);
                ppuActionEntryPtr->rotActionEntry[i].cmpCond = regData & BIT_MASK_MAC(2);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 36, 8, regData);
                ppuActionEntryPtr->rotActionEntry[i].cmpMask = regData & BIT_MASK_MAC(8);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 44, 8, regData);
                ppuActionEntryPtr->rotActionEntry[i].cmpTrueValLd = regData & BIT_MASK_MAC(8);
            }
            else
            {
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 15, 1, regData);
                ppuActionEntryPtr->rotActionEntry[i].shiftLeftRightSel = (GT_BOOL)(regData & BIT_MASK_MAC(1));
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 16, 3, regData);
                ppuActionEntryPtr->rotActionEntry[i].shiftNum = regData & BIT_MASK_MAC(3);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 19, 4, regData);
                ppuActionEntryPtr->rotActionEntry[i].setBitsNum = regData & BIT_MASK_MAC(4);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 23, 8, regData);
                ppuActionEntryPtr->rotActionEntry[i].setBitsVal = regData & BIT_MASK_MAC(8);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 31, 5, regData);
                ppuActionEntryPtr->rotActionEntry[i].srcOffset = regData & BIT_MASK_MAC(5);
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 36, 16, regData);
                ppuActionEntryPtr->rotActionEntry[i].addSubConst = (GT_U16)(regData & BIT_MASK_MAC(16));
                U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 52, 1, regData);
                ppuActionEntryPtr->rotActionEntry[i].addSubConstSel = (GT_BOOL)(regData & BIT_MASK_MAC(1));
            }
        }
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 53, 2, regData);
        ppuActionEntryPtr->rotActionEntry[i].func = regData & BIT_MASK_MAC(2);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 55, 3, regData);
        ppuActionEntryPtr->rotActionEntry[i].funcSecondOperand = regData & BIT_MASK_MAC(3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 58, 1, regData);
        ppuActionEntryPtr->rotActionEntry[i].target = (GT_BOOL)(regData & BIT_MASK_MAC(1));
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 59, 3, regData);
        ppuActionEntryPtr->rotActionEntry[i].ldSpBusNumBytes = regData & BIT_MASK_MAC(3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 62, 5, regData);
        ppuActionEntryPtr->rotActionEntry[i].ldSpBusOffset = regData & BIT_MASK_MAC(5);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 67, 1, regData);
        ppuActionEntryPtr->rotActionEntry[i].interrupt = (GT_BOOL)(regData & BIT_MASK_MAC(1));
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 68, 3, regData);
        ppuActionEntryPtr->rotActionEntry[i].interruptIndex = regData & BIT_MASK_MAC(3);

        offset += 71;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 355, 8, regData);
    ppuActionEntryPtr->setNextState = regData & BIT_MASK_MAC(8);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 363, 7, regData);
    ppuActionEntryPtr->setConstNextShift = regData & BIT_MASK_MAC(7);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 370, 1, regData);
    ppuActionEntryPtr->setLoopBack = (GT_BOOL)(regData & BIT_MASK_MAC(1));
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 371, 1, regData);
    ppuActionEntryPtr->counterSet = (GT_BOOL)(regData & BIT_MASK_MAC(1));
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 372, 5, regData);
    ppuActionEntryPtr->counterSetVal = regData & BIT_MASK_MAC(5);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 377, 2, regData);
    ppuActionEntryPtr->counterOper = regData & BIT_MASK_MAC(2);

    return GT_OK;
}

/**
* @internal  prvCpssDxChPpuDauProfileEntrySet function
* @endinternal
*
* @brief    This function sets DAU profile entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileIndex           - DAU profile index
* @param[in] dauProfilePtr          - pointer to PPU DAU profile entry data.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuDauProfileEntrySet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    IN  CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      i;         /* iterator         */
    GT_U32      offset;

    GT_U32      hwData[CPSS_DXCH_PPU_DAU_PROFILE_WIDTH_IN_WORDS_CNS]; /* table entry data */

    /* create hardware table entry from DAU profile entry data structure */
    offset = 0;
    for(i = 0; i < CPSS_DXCH_PPU_DAU_DESC_BYTE_SET_MAX_CNS; i++)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 0, 1, (GT_U32)dauProfilePtr->setDescBits[i].spByteWriteEnable);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 1, 3, dauProfilePtr->setDescBits[i].spByteNumBits);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 4, 8, dauProfilePtr->setDescBits[i].spByteSrcOffset);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 12, 12, (GT_U32)dauProfilePtr->setDescBits[i].spByteTargetOffset);
        offset += 24;
    }

    /* write DAU table entry */
    rc = prvCpssDxChWriteTableEntry(devNum, CPSS_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E, profileIndex, &hwData[0]);

    return rc;
}

/**
* @internal prvCpssDxChPpuDauProfileEntryGet function
* @endinternal
*
* @brief    This function reads DAU profile table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileIndex           - DAU profile index
* @param[out] dauProfilePtr         - pointer to PPU DAU profile entry data;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuDauProfileEntryGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              profileIndex,
    OUT CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regData;     /* register data    */
    GT_U32      i;         /* iterator         */
    GT_U32      offset;

    GT_U32      hwData[CPSS_DXCH_PPU_DAU_PROFILE_WIDTH_IN_WORDS_CNS]; /* table entry data */

    /* read DAU table entry */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E, profileIndex, &hwData[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* parse DAU table entry and populate DAU profile structure */
    offset = 0;
    for(i = 0; i < CPSS_DXCH_PPU_DAU_DESC_BYTE_SET_MAX_CNS; i++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  0,  1, regData);
        dauProfilePtr->setDescBits[i].spByteWriteEnable = (GT_BOOL)(regData & BIT_MASK_MAC(1));
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  1,  3, regData);
        dauProfilePtr->setDescBits[i].spByteNumBits = regData & BIT_MASK_MAC(3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  4,  8, regData);
        dauProfilePtr->setDescBits[i].spByteSrcOffset = regData & BIT_MASK_MAC(8);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 12, 12, regData);
        dauProfilePtr->setDescBits[i].spByteTargetOffset = (GT_U16)(regData & BIT_MASK_MAC(12));

        offset += 24;
    }

    return GT_OK;
}

/**
* @internal  prvCpssDxChPpuDauProtectedWindowSet function
* @endinternal
*
* @brief    This function sets DAU protected window table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] index                  - index to DAU protected window table entry.
* @param[in] protWinPtr             - pointer to PPU DAU protected window table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuDauProtectedWindowSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    IN  CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData = 0;     /* register data    */

    /* get address for DAU protected window profile */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).dau.dauProtWin[index];

    /* create register value from DAU protected window data structure */
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_DAU_PROT_WIN_ENABLE_OFFSET,
            CPSS_DXCH_PPU_DAU_PROT_WIN_ENABLE_SIZE, protWinPtr->protWinEnable);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_DAU_PROT_WIN_START_OFFSET_OFFSET,
            CPSS_DXCH_PPU_DAU_PROT_WIN_START_OFFSET_SIZE, protWinPtr->protWinStartOffset);
    U32_SET_FIELD_MAC(regData, CPSS_DXCH_PPU_DAU_PROT_WIN_END_OFFSET_OFFSET,
            CPSS_DXCH_PPU_DAU_PROT_WIN_END_OFFSET_SIZE, protWinPtr->protWinEndOffset);

    /* write register value to dau protected window register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);

    return rc;
}

/**
* @internal  prvCpssDxChPpuDauProtectedWindowGet function
* @endinternal
*
* @brief    This function gets DAU protected window table entry.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] index                  - index to DAU protected window table entry.
* @param[out] protWinPtr            - pointer to PPU DAU protected window table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuDauProtectedWindowGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      index,
    OUT CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
)
{
    GT_STATUS   rc;          /* return code      */
    GT_U32      regAddr;     /* register address */
    GT_U32      regData;     /* register data    */

    /* get register address for DAU protected window profile */
    regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).dau.dauProtWin[index];

    /* read DAU protected window profile data */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);

    /* populate DAU protected window profile data structure */
    protWinPtr->protWinEnable      = U32_GET_FIELD_MAC(regData,
                                         CPSS_DXCH_PPU_DAU_PROT_WIN_ENABLE_OFFSET,
                                         CPSS_DXCH_PPU_DAU_PROT_WIN_ENABLE_SIZE);
    protWinPtr->protWinStartOffset = U32_GET_FIELD_MAC(regData,
                                         CPSS_DXCH_PPU_DAU_PROT_WIN_START_OFFSET_OFFSET,
                                         CPSS_DXCH_PPU_DAU_PROT_WIN_START_OFFSET_SIZE);
    protWinPtr->protWinEndOffset   = U32_GET_FIELD_MAC(regData,
                                         CPSS_DXCH_PPU_DAU_PROT_WIN_END_OFFSET_OFFSET,
                                         CPSS_DXCH_PPU_DAU_PROT_WIN_END_OFFSET_SIZE);

    return rc;
}

/**
* @internal prvCpssDxChPpuErrorProfileSet function
* @endinternal
*
* @brief    This function gets error profile for a given profile and descriptor field.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] profileNum            - profile number
* @param[in] errorProfilePtr       - pointer to PPU error profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuErrorProfileSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    IN  CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
)
{
    GT_STATUS   rc = GT_OK;          /* return code      */
    GT_U32      regAddr;             /* register address */
    GT_U32      regData;             /* regData          */
    GT_U32      fld;                 /* field            */

    for(fld = 0; fld < CPSS_DXCH_PPU_ERROR_PROFILE_FIELDS_MAX_CNS; fld ++)
    {
        /* get address for DAU protected window profile */
        regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).errProfile[profileNum].field[fld];

        regData = 0;
        regData |= (errorProfilePtr->errProfileField[fld].writeEnable    & 0x1  ) << 0 ;
        regData |= (errorProfilePtr->errProfileField[fld].numBits        & 0x7  ) << 1 ;
        regData |= (errorProfilePtr->errProfileField[fld].errorDataField & 0xFF ) << 4 ;
        regData |= (errorProfilePtr->errProfileField[fld].targetOffset   & 0xFFF) << 12;

        /* write field value to register */
        rc = prvCpssHwPpWriteRegister(devNum, regAddr, regData);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChPpuErrorProfileGet function
* @endinternal
*
* @brief    This function gets error profile for a given profile and descriptor field.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] profileNum             - profile number
* @param[out] errorProfilePtr       - pointer to PPU error profile.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuErrorProfileGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          profileNum,
    OUT CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
)
{
    GT_STATUS   rc = GT_OK;          /* return code      */
    GT_U32      regAddr;             /* register address */
    GT_U32      regData;             /* regData          */
    GT_U32      fld;                 /* field            */

    for(fld = 0; fld < CPSS_DXCH_PPU_ERROR_PROFILE_FIELDS_MAX_CNS; fld ++)
    {
        /* get address for DAU protected window profile */
        regAddr = PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).errProfile[profileNum].field[fld];

        /* write field value to register */
        rc = prvCpssHwPpReadRegister(devNum, regAddr, &regData);
        if(rc != GT_OK)
        {
            return rc;
        }

        errorProfilePtr->errProfileField[fld].writeEnable    = (regData >> 0 ) &  0x1  ;
        errorProfilePtr->errProfileField[fld].numBits        = (regData >> 1 ) &  0x7  ;
        errorProfilePtr->errProfileField[fld].errorDataField = (regData >> 4 ) &  0xFF ;
        errorProfilePtr->errProfileField[fld].targetOffset   = (regData >> 12) &  0xFFF;
    }

    return rc;
}

/**
* @internal prvCpssDxChPpuDebugCountersGet function
* @endinternal
*
* @brief    This function reads the debug counters.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum               - device number
* @param[out] dbgCountersPtr       - pointer to debug counters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuDebugCountersGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PPU_DEBUG_COUNTERS_STC *dbgCountersPtr
)
{
    GT_STATUS   rc = GT_OK;          /* return code      */

    /* read incoming packet count for PPU */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterIn, &dbgCountersPtr->counterIn);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterIn, 0);

    /* read outgoing packet count from PPU */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterOut, &dbgCountersPtr->counterOut);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterOut, 0);

    /* read incoming packet count for K_stg 0 unit */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg0, &dbgCountersPtr->counterKstg0);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg0, 0);

    /* read incoming packet count for K_stg 1 unit */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg1, &dbgCountersPtr->counterKstg1);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg1, 0);

    /* read incoming packet count for K_stg 2 unit */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg2, &dbgCountersPtr->counterKstg2);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg2, 0);

    /* read incoming packet count for DAU unit */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterDau, &dbgCountersPtr->counterDau);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterDau, 0);

    /* read loopback count */
    rc |= prvCpssHwPpReadRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterLoopback, &dbgCountersPtr->counterLoopback);
    /* Clear on read */
    rc |= prvCpssHwPpWriteRegister(devNum,
            PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterLoopback, 0);

    return rc;
}

/**
* @internal prvCpssDxChPpuDebugCountersClear function
* @endinternal
*
* @brief    This function clears debug counters and sets to zero.
*
* @note   APPLICABLE DEVICES:  AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; BobCat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChPpuDebugCountersClear
(
    IN  GT_U8                           devNum
)
{
    GT_STATUS   rc = GT_OK;          /* return code      */

    /* clear incoming packet count for PPU */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterIn, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear outgoing packet count from PPU */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterOut, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear incoming packet count for K_stg 0 unit */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg0, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear incoming packet count for K_stg 1 unit */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg1, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear incoming packet count for K_stg 2 unit */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterKstg2, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear incoming packet count for DAU unit */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterDau, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* clear loopback count */
    rc = prvCpssHwPpWriteRegister(devNum, PRV_DXCH_REG1_UNIT_PPU_MAC(devNum).debugRegs.ppuDebugCounterLoopback, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
}

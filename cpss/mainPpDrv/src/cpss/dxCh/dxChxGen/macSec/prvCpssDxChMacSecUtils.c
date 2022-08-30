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
 * @file prvCpssDxChMacSecUtils.c
 *
 * @brief CPSS utils relate to MAC Security (or MACsec) feature which is a 802.1AE IEEE
 *        industry-standard security technology that provides secure communication for traffic
 *        on Ethernet links.
 *
 *   NOTEs:
 *   GT_NOT_INITIALIZED will be return for any 'MACSec' APIs if called before cpssDxChMacSecInit(..)
 *   (exclude cpssDxChMacSecInit(...) itself)
 *
 * @version   1
 ********************************************************************************
 */

#include <cpss/dxCh/dxChxGen/macSec/private/prvCpssDxChMacSecUtils.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <Kit/SABuilder_MACsec/Example_AES/incl/aes.h>
#include <Integration/Adapter_EIP163/incl/adapter_cfye_support.h>
#include <Integration/Adapter_EIP164/incl/adapter_secy_support.h>


/* EIP-163 devices IDs */
#define PRV_CPSS_CFYE_EGRESS_DP0_DEVICE_ID_CNS   0
#define PRV_CPSS_CFYE_INGRESS_DP0_DEVICE_ID_CNS  1
#define PRV_CPSS_CFYE_EGRESS_DP1_DEVICE_ID_CNS   2
#define PRV_CPSS_CFYE_INGRESS_DP1_DEVICE_ID_CNS  3

/* EIP-164 devices IDs */
#define PRV_CPSS_SECY_EGRESS_DP0_DEVICE_ID_CNS   0
#define PRV_CPSS_SECY_INGRESS_DP0_DEVICE_ID_CNS  1
#define PRV_CPSS_SECY_EGRESS_DP1_DEVICE_ID_CNS   2
#define PRV_CPSS_SECY_INGRESS_DP1_DEVICE_ID_CNS  3

/* Array with MACSec units IDs */
static const GT_U32 macSecUnitIdArr[2 /*DP0:DP1*/][2 /*EIP163:EIP164*/][2 /*Egress:Ingress*/] =
{
    {/* DP0 */
        {/* EIP-163 */
            PRV_CPSS_CFYE_EGRESS_DP0_DEVICE_ID_CNS,  /* Egress  */
            PRV_CPSS_CFYE_INGRESS_DP0_DEVICE_ID_CNS  /* Ingress */
        },
        {/* EIP-164 */
            PRV_CPSS_SECY_EGRESS_DP0_DEVICE_ID_CNS,  /* Egress  */
            PRV_CPSS_SECY_INGRESS_DP0_DEVICE_ID_CNS  /* Ingress */
        }
    },
    {/* DP1 */
        {/* EIP-163 */
            PRV_CPSS_CFYE_EGRESS_DP1_DEVICE_ID_CNS,  /* Egress  */
            PRV_CPSS_CFYE_INGRESS_DP1_DEVICE_ID_CNS  /* Ingress */
        },
        {/* EIP-164 */
            PRV_CPSS_SECY_EGRESS_DP1_DEVICE_ID_CNS,  /* Egress  */
            PRV_CPSS_SECY_INGRESS_DP1_DEVICE_ID_CNS  /* Ingress */
        }
    }
};


/**
* @internal prvCpssMacSecGetUnitId function
* @endinternal
*
* @brief  Get the desired MACSec unit ID
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  dpId                - Data Path ID in which this MACSec unit is connected
* @param[in]  unitType            - MACSec unit type: Classifier or Transformer
* @param[in]  unitDir             - MACSec unit direction: Egress or Ingress
*
* @retval unitId                  - MACSec unit ID
*/
GT_U32
prvCpssMacSecGetUnitId(GT_U8 devNum, GT_U32 dpId, GT_U32 unitType, GT_U32 unitDir)
{
    GT_U32 unitId;  /* MACSec unit ID */

    unitId = (devNum*4) + macSecUnitIdArr[dpId][unitType][unitDir];
    return unitId;
}

/**
* @internal prvCpssMacSecExtByPassEnable function
* @endinternal
*
* @brief  Set MACSec unit to bypass or not bypass on the PCA EXT wrapper level
*         - if bypass then disable the clock towards the MacSec shell
*         - if not bypass then enable the clock towards the MacSec shell
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  dpId                - Data Path ID
* @param[in]  unitType            - MACSec unit type: Classifier or Transformer
* @param[in]  unitDir             - MACSec unit direction: Egress or Ingress
* @param[in]  macSecBypass        - MACSec mode: For bypass set true, for MACSec mode set false
*
* @retval GT_OK                - on success
* @retval GT_FAIL              - on hardware error
* @retval GT_NOT_INITIALIZED   - if the driver was not initialized
* @retval GT_HW_ERROR          - on hardware error
*/
GT_STATUS
prvCpssMacSecExtByPassEnable(GT_U8 devNum, GT_U32 dpId, GT_U32 unitType, GT_U32 unitDir, GT_BOOL macSecBypass)
{
    GT_STATUS rc;
    GT_U32    msWrapperMiscRegAddr;
    GT_U32    hwValue;

    /* Get MacSec Wrapper Misc register address */
    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC *regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum);
    msWrapperMiscRegAddr = regsAddrPtr->PCA_MACSEC_EXT[dpId][unitType][unitDir].msWrapperMisc;

    if (macSecBypass)
    {
        /* MACSec shell is bypassed
           - Sets macsec_bypass field (bit#0) to '1'. MacSec shell will be bypassed.
           - Sets core_clk_disable field (bit#1) to '1'. Disables the core clock towards the MacSec shell.
         */
        hwValue = 3;
    }
    else
    {
        /* MACSec shell is enabled
           - Clears macsec_bypass field (bit#0) to '0'. MacSec shell will not be bypassed.
           - Clears core_clk_disable field (bit#1) to '0'. Enables the core clock towards the MacSec shell.
         */
        hwValue = 0;
    }

    /* Write hwValue in MacSec Wrapper Misc register */
    rc = prvCpssDrvHwPpSetRegField(devNum, msWrapperMiscRegAddr, 0/*fieldOffset*/, 2/*fieldLength*/, hwValue/*fieldData*/);

    return rc;
}

/**
* @internal prvCpssMacSecInitCheck function
* @endinternal
*
* @brief   Verify this specific MACSec unit was indeed initialized at this point
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] macSecUnitType  - MACSec unit type: Classifier or Transformer.
* @param[in] macSecUnitId    - MACSec unit number.
*
* @retval GT_OK               - on success
* @retval GT_NOT_INITIALIZED  - if unit is not initialized
*/
GT_STATUS prvCpssMacSecInitCheck
(
    IN  CPSS_DXCH_MACSEC_DEVICE_TYPE_ENT   macSecUnitType,
    IN  GT_U32                             macSecUnitId
)
{
    GT_STATUS rc = GT_OK;

    if (PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.macSecDriverDir.dbPtr) == NULL)
    {
        /* MACSec Global DB was not allocated yet */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* Verify this MACSec unit was indeed initialized */
    if (macSecUnitType == CPSS_DXCH_MACSEC_CLASSIFIER_E)
    {
         /* Classifier type */
        if (!PRV_SHARED_MACSEC_CFYE_GLOBAL_VAR_GET(CfyE_Device_StaticFields)[macSecUnitId].fInitialized)
        {
            rc = GT_NOT_INITIALIZED;
        }
    }
    else
    {
        /* Transformer type */
        if (!PRV_SHARED_MACSEC_SECY_GLOBAL_VAR_GET(SecY_Device_StaticFields)[macSecUnitId].fInitialized)
        {
            rc = GT_NOT_INITIALIZED;
        }
    }

    return rc;
}

/**
* @internal prvCpssMacSecBmpCheck function
* @endinternal
*
* @brief   Check if bits map is valid for MACSec usage.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum          - device number.
* @param[in] macSecBmpPtr    - (pointer to) the bits map. Each bit represents one DP.
*                               Return MACSec bits map while restricet it to 2 bits 1 bit per Data path
*
* @retval GT_OK              - on success
* @retval GT_BAD_PARAM       - on invalid *macSecBmpPtr
*/
GT_STATUS prvCpssMacSecBmpCheck
(
    IN     GT_U32                devNum,
    INOUT  GT_MACSEC_UNIT_BMP    *macSecBmpPtr,
    IN     GT_BOOL               multipleUnitsValid
)
{
    GT_MACSEC_UNIT_BMP  macSecBmp = *macSecBmpPtr;

    /* Check if device is single data path or multiple data paths */
    if ( PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp == 1 )
    {
        /* Single data path therefore ignore macSecBmp */
        *macSecBmpPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp;
    }
    else if ((macSecBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS) &&
             (macSecBmp == 0 ||
             ((PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp & macSecBmp) != macSecBmp)))  {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "prvCpssMacSecBmpCheck: Invalid Data path bits map:[0x%8.8x]", macSecBmp);
             }
    else
    {
        /* Restrict bits map range according to device type */
        *macSecBmpPtr = (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp & macSecBmp);

        /* Return error if bits map of multiple Data paths is not allowed */
        if ( (*macSecBmpPtr == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.macSecInfo.macSecDpBmp) &&  (!multipleUnitsValid) )
        {
            /* Invalid macSecBmp */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "prvCpssMacSecBmpCheck: Bits map which represent multiple Data path is not allowed");
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssMacSecUnitBitsMapToUnitId function
* @endinternal
*
* @brief   Get MACSec unit ID from DP bits map, direction and unit type
*          Check MACSec idetntification parameters are valid
*          Verify MACSec unit was intialized
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress direction
* @param[in]  macSecUnitType      - MACSec classifier or Transformer
* @param[out] unitIdPtr           - (pointer to) MACSec unit ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssMacSecUnitBitsMapToUnitId
(
    IN  GT_U8                              devNum,
    IN  GT_MACSEC_UNIT_BMP                 unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT     macSecDirection,
    IN  CPSS_DXCH_MACSEC_DEVICE_TYPE_ENT   macSecType,
    OUT GT_U32                             *macSecIdPtr
)
{
    GT_STATUS rc;
    GT_U32    macSecDpId;

    /* Check that MACSec direction and type are valid */
    PRV_CPSS_DXCH_MACSEC_DIRECTION_CHECK_MAC(macSecDirection);
    PRV_CPSS_DXCH_MACSEC_TYPE_CHECK_MAC(macSecType);

    /* Check unitBmp is valid */
    rc = prvCpssMacSecBmpCheck(devNum, &unitBmp, GT_FALSE /* multipe units are not allowed */);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Convert MACSec DP bits map to MACSec DP ID */
    PRV_CPSS_DXCH_MACSEC_DP_BITS_MAP_TO_DP_ID_MAC(unitBmp,macSecDpId);

    /* Get MACSec unit ID */
    *macSecIdPtr = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,macSecDpId,macSecType,macSecDirection);

    /* Verify this specific MACSec unit was indeed initialized */
    rc = prvCpssMacSecInitCheck(macSecType,*macSecIdPtr);

    return rc;
}

/**
* @internal prvCpssMacSecPortToUnitId function
* @endinternal
*
* @brief   Get MACSec unit ID from physical port, direction and unit type
*          Get the channel number from physical port
*          Check port number is valid and MACSec idetntification parameters are valid
*          Verify MACSec unit was intialized
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress direction
* @param[in]  macSecUnitType       - MACSec classifier or Transformer
* @param[out] macSecChannelNumPtr  - (pointer to) MACSec channel number
* @param[out] unitIdPtr            - (pointer to) MACSec unit ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssMacSecPortToUnitId
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT     macSecDirection,
    IN  CPSS_DXCH_MACSEC_DEVICE_TYPE_ENT   macSecType,
    OUT GT_U32                             *macSecChannelNumPtr,
    OUT GT_U32                             *macSecIdPtr
)
{
    GT_STATUS    rc;
    GT_U32       dpId;
    GT_U32       channelNum;

    /* Check that MACSec direction and type are valid */
    PRV_CPSS_DXCH_MACSEC_DIRECTION_CHECK_MAC(macSecDirection);
    PRV_CPSS_DXCH_MACSEC_TYPE_CHECK_MAC(macSecType);

    /* Check portNum is valid plus get Data path ID and local channel */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndMacSecConvert(devNum, portNum, &dpId, &channelNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* MACSec channel number */
    *macSecChannelNumPtr = channelNum;

    /* Get MACSec unit ID */
    *macSecIdPtr = PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(devNum,dpId,macSecType,macSecDirection);

    /* Verify this specific MACSec unit was indeed initialized */
    rc = prvCpssMacSecInitCheck(macSecType,*macSecIdPtr);

    return rc;
}


/**
* @internal prvCpssMacSecBuildSaTransformRecord function
* @endinternal
*
* @brief   Create SA transform record using MACsec SA Builder.
*          Return pointer to buffer containing this SA record + size of record in 4B words
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  paramsPtr            - (pointer to) transform record parameters
* @param[out] saWordCountPtr       - (pointer to) number of transform record size in 4B words
* @param[out] trRecBufferPtr       - (pointer to) transform record buffer
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NO_RESOURCE           - on out of memory space
*/
GT_STATUS prvCpssMacSecBuildSaTransformRecord
(
    IN  PRV_CPSS_SECY_TR_REC_PARAMS_STC *paramsPtr,
    OUT GT_U32                          *saWordCountPtr,
    OUT GT_U32                          **trRecBufferPtr
)
{
    SABuilder_Params_t saParams;
    GT_STATUS          rc;
    GT_U32             saWordCount;
    GT_U32             *bufferPtr;

    rc = SABuilder_InitParams(&saParams, paramsPtr->an, paramsPtr->direction, SAB_OP_MACSEC);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"SABuilder_InitParams failure");
    }

    /* Set transform record parameters */
    saParams.HKey_p       = paramsPtr->hKeyPtr;
    saParams.Key_p        = paramsPtr->keyPtr;
    saParams.KeyByteCount = paramsPtr->keyByteCount;
    saParams.SCI_p        = paramsPtr->sciPtr;
    saParams.SeqNumLo     = paramsPtr->seqNumLo;
    saParams.WindowSize   = paramsPtr->seqMask;
    if (paramsPtr->saltPtr)
    {
        saParams.flags   |= SAB_MACSEC_FLAG_LONGSEQ;
        saParams.Salt_p   = paramsPtr->saltPtr;
        saParams.SSCI_p   = paramsPtr->ssciPtr;
        saParams.SeqNumHi = paramsPtr->seqNumHi;
    }

    /* Compute offsets for various fields. Get SA size */
    rc = SABuilder_GetSize(&saParams, &saWordCount);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"SABuilder_GetSize failure");
    }

    /* Allocate memory for transform record */
    bufferPtr = cpssOsMalloc(saWordCount * sizeof(uint32_t));
    if (bufferPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "prvCpssMacSecBuildSaTransformRecord: fail to allocate memory");
    }

    /* Set number of 4B words */
    *saWordCountPtr = saWordCount;

    /* Build SA record data */
    rc = SABuilder_BuildSA(&saParams, bufferPtr, AES_Encrypt);
    if (rc != GT_OK)
    {
        cpssOsFree(bufferPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"SABuilder_BuildSA failure");
    }

    /* Set transform record pointer */
    *trRecBufferPtr = bufferPtr;

    return rc;
}

/**
* @internal prvCpssDxChPortPhysicalPortGet function
* @endinternal
*
* @brief   Function gets the physical port based on DP index and local DMA number.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] dpIndex                  - the Data Path (DP) Index
* @param[in] localDmaNum              - the DMA local number
*
* @param[out] physicalPortNumPtr       - (pointer to) physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, origPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_FOUND             - not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortPhysicalPortGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          dpId,
    IN  GT_U32                          localDmaNum,
    OUT GT_PHYSICAL_PORT_NUM            *physicalPortNumPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    globalDmaNum;

    /* Get global DMA number */
    rc = prvCpssDxChHwPpDmaLocalNumInDpToGlobalNumConvert(devNum, dpId, localDmaNum, &globalDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* Get physical port number */
    rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum, PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, globalDmaNum, physicalPortNumPtr);

    return rc;
}


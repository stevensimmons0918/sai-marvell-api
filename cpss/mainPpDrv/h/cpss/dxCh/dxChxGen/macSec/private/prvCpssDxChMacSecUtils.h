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
* @file prvCpssDxChMacSecUtils.h
*
* @brief CPSS DxCh MAC Security (or MACsec) private API.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChMacSecUtilsh
#define __prvCpssDxChMacSecUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/macSec/cpssDxChMacSec.h>
#include <Kit/SABuilder_MACsec/incl/sa_builder_macsec.h>


/* Macro to check that the MACSec direction is valid */
#define PRV_CPSS_DXCH_MACSEC_DIRECTION_CHECK_MAC(_direction)                                                          \
    if( (_direction != CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E) && (_direction != CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E) ) \
    {                                                                                                                 \
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MACSec direction is not valid:[%d]", _direction);               \
    }

/* Macro to check that the MACSec type is valid */
#define PRV_CPSS_DXCH_MACSEC_TYPE_CHECK_MAC(_type)                                                                    \
    if( (_type != CPSS_DXCH_MACSEC_CLASSIFIER_E) && (_type != CPSS_DXCH_MACSEC_TRANSFORMER_E) )                       \
    {                                                                                                                 \
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MACSec type is not valid:[%d]", _type);                         \
    }


#define PRV_CPSS_DXCH_MACSEC_GET_UNIT_ID_MAC(_devNum,_macSecDpId,_macSecType,_macSecDir)                   \
                                            ( prvCpssMacSecGetUnitId(_devNum,_macSecDpId,_macSecType,_macSecDir) )

/* Macro to convert MACSec Data path bits map to MACSec DP ID */
#define PRV_CPSS_DXCH_MACSEC_DP_BITS_MAP_TO_DP_ID_MAC(_unitBmp,_unitId)    _unitId = (_unitBmp - 1)

/* Parameters for SA transform record creation */
typedef struct
{
    /* Egress or Ingress */
    SABuilder_Direction_t direction;
    /* Association Number */
    GT_U8                 an;
    /* MACsec key */
    GT_U8                 *keyPtr;
    /* MACsec key size */
    GT_U32                keyByteCount;
    /* SCI (8B) */
    GT_U8                 *sciPtr;
    /* SSCI (4B) */
    GT_U8                 *ssciPtr;
    /* SALT (16B) */
    GT_U8                 *saltPtr;
    /* Sequence number low part */
    GT_U32                seqNumLo;
    /* Sequence number high part */
    GT_U32                seqNumHi;
    /* Replay window size: specifies the window size for ingress sequence number checking*/
    GT_U32                seqMask;
    /* Pointer to 16 bytes Hash key for the authentication operation */
    GT_U8                 *hKeyPtr;
} PRV_CPSS_SECY_TR_REC_PARAMS_STC;


/* Common APIs */


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
GT_U32 prvCpssMacSecGetUnitId
(
    GT_U8 devNum,
    GT_U32 dpId,
    GT_U32 unitType,
    GT_U32 unitDir
);

/**
* @internal prvCpssMacSecExtByPassEnable function
* @endinternal
*
* @brief  Set MACSec unit to bypass or not bypass on the PCA EXT wrapper level
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
GT_STATUS prvCpssMacSecExtByPassEnable
(
    GT_U8     devNum,
    GT_U32    dpId,
    GT_U32    unitType,
    GT_U32    unitDir,
    GT_BOOL   macSecBypass
);

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
);

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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChMacSecUtilsh */




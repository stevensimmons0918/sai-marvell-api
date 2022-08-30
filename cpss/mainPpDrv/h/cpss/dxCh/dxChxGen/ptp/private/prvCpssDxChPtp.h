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
* @file prvCpssDxChPtp.h
*
* @brief CPSS DXCH private PTP debug API implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPtph
#define __prvCpssDxChPtph
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

    /* port number to access the PTP ILKN registers DB */
#define PRV_CPSS_DXCH_ILKN_PTP_PORT_NUM_CNS 72

/* Get port MAC num if MAC type or ILKN indication */
#define PRV_CPSS_DXCH_PTP_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_dev, _port, _portMacNum)              \
    {                                                                                               \
        GT_STATUS __rc;                                                                             \
        CPSS_DXCH_DETAILED_PORT_MAP_STC *_portMapShadowPtr;                                         \
        __rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(_dev, _port, &_portMapShadowPtr);          \
        if (__rc != GT_OK)                                                                          \
        {                                                                                           \
            return __rc;                                                                            \
        }                                                                                           \
        if (GT_FALSE == _portMapShadowPtr->valid)                                                   \
        {                                                                                           \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                          \
        }                                                                                           \
        if (CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == _portMapShadowPtr->portMap.mappingType)   \
        {                                                                                           \
            _portMacNum = PRV_CPSS_DXCH_ILKN_PTP_PORT_NUM_CNS;                                      \
        }                                                                                           \
        else                                                                                        \
        {                                                                                           \
            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(_dev, _port ,_portMacNum);             \
        }                                                                                           \
    }

/**
* @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_FALCON_ENT
 *
 * @brief Instance of TAI unit sub type.
 * (APPLICABLE DEVICES: Falcon.)
*/
typedef enum
{
    /** @brief TAI instance sub type Master
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_MASTER_E = 0,

    /** @brief TAI instance sub type CP slave for control pipe0
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CP_TAI_SLAVE_PIPE0_E,

    /** @brief TAI instance sub type CP slave for control pipe1
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CP_TAI_SLAVE_PIPE1_E,

    /** @brief TAI instance sub type TXQ slave for control pipe0
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQ_TAI_SLAVE_PIPE0_E,

    /** @brief TAI instance sub type TXQ slave for control pipe1
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQ_TAI_SLAVE_PIPE1_E,

    /** @brief TAI slave instance for GOP0
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_0_E,

    /** @brief TAI slave instance for GOP1
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_1_E,

    /** @brief TAI slave instance for GOP2
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_2_E,

    /** @brief TAI slave instance for GOP3
     */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_GOP_3_E
}PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_FALCON_ENT;

/**
* @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HAWK_ENT
 *
 * @brief Instance of TAI unit sub type.
 * (APPLICABLE DEVICES: AC5P.)
*/
typedef enum
{
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TAI_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQS_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TXQS1_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_TTI_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHA_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_EPCL_HA_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_CPU_PORT_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G1_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G2_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_400G3_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_USX1_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA2_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA3_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PCA1_TAI0_E,
}PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HAWK_ENT;


/**
* @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_ENT
 *
 * @brief Instance of TAI unit sub type.
 * (APPLICABLE DEVICES: AC5X; Harrier; Ironman.)
*/
typedef enum
{
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_L2I_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_TTI_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_PHA_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_EPCL_HA_TAI0_E ,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_TXQS_TAI0_E, /* Single slave; only TAI0 no TAI1 */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_PCA0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_USX2_1_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUM_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_MAC_CPUC_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_100G_TAI0_E,
}PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_PHOENIX_ENT;

/**
* @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_ENT
 *
 * @brief Instance of TAI unit sub type.
*/
typedef enum
{
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_L2I_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_TTI_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_TXQS_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_TXQS1_TAI0_E, /* Single slave; only TAI0 no TAI1 */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PHA_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_EPCL_HA_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_400G1_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_200G2_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA0_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA1_TAI0_E,
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_PCA2_TAI0_E

}PRV_CPSS_DXCH_PTP_TAI_INSTANCE_SUB_TYPE_HARRIER_ENT;

/********* enums *********/
/**
 * @enum PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT
 * @brief Type of TAI instance.
 */
typedef enum{

    /** @brief Master instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_MASTER_E,

    /** @brief CP slave instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_CP_SLAVE_E,

    /** @brief TXQ slave instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_TXQ_SLAVE_E,

    /** @brief Chiplet slave instance in GOP die */
    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_CHIPLET_SLAVE_E
} PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT;

/********* enums *********/
/**
 * @enum PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_ENT
 * @brief Type of the clock to control.
 */
typedef enum{

    /** @brief Master instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_GEN_E,

    /** @brief CP slave instance in Processing die */
    PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_RECEPTION_E

} PRV_CPSS_DXCH_PTP_TAI_CLOCK_CONTROL_TYPE_ENT;


/**
* @enum PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_ENT
 *
 * @brief State of TAI Iteration Structure.
*/
typedef enum{

    /** Start - value for initialization */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_START_E,

    /** Valid TAI subunit identified */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_E,

    /** @brief Last Valid TAI subunit identified
     *  typicaly checked in read functions that require the single TAI.
     */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_VALID_LAST_E,

    /** No TAI subunits more */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_NO_MORE_E

} PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_ENT;

/**
* @struct PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC
 *
 * @brief Structure for TAI Iteration.
*/
typedef struct{

    /** TAI Iterator state */
    PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STATE_ENT state;

    /** GOP Number */
    GT_U32 gopNumber;

    /** TAI Number */
    GT_U32 taiNumber;

    /** number of TAIs to iterate when taiNumber == CPSS_DXCH_PTP_TAI_NUMBER_ALL_E */
    GT_U32 numOfTais;

    /** port group Id */
    GT_U32 portGroupId;

} PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC;

/********* lib API DB *********/
/**
* @internal prvCpssDxChPtpTaiDebugInstanceSet function
* @endinternal
*
* @brief   Sets the TAI debug instance.
*
* @note   APPLICABLE DEVICES:     Falcon.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[in] tileId               - Processing die identifier
* @param[in] taiInstanceType      - Type of the instance
*                                   (Master, CP_Slave, TXQ_Slave, Chiplet_Slave )
* @param[in] instanceId           - TAI instance identifier for a particular tile.
*                                   Master              - Not used.
*                                   CP_Slave,TXQ_Slave  - 0..1 - processing pipe number.
*                                   Chiplet_Slave       - 0..3 - local GOP die number of specific tile.
*
* @retval GT_OK                   - on success
* @retval GT_BAD_PARAM            - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpTaiDebugInstanceSet
(
    IN    GT_U8                                     devNum,
    IN    GT_U32                                    tileId,
    IN    PRV_CPSS_DXCH_PTP_TAI_INSTANCE_TYPE_ENT   taiInstanceType,
    IN    GT_U32                                    instanceId
);

/**
* @internal prvCpssDxChPtpNextTaiGet function
* @endinternal
*
* @brief   Gets next TAI subunit.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Id.
*                                      For single TAI devices taiIdPtr ignored,
*                                      iterator state evaluation is START_E => VALID_LAST => NO_MORE.
* @param[in,out] iteratorPtr              - (pointer to) TAI iterator
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpNextTaiGet
(
    IN    GT_U8                               devNum,
    IN    CPSS_DXCH_PTP_TAI_ID_STC            *taiIdPtr,
    INOUT PRV_CPSS_DXCH_PTP_TAI_ITERATOR_STC  *iteratorPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPtph */

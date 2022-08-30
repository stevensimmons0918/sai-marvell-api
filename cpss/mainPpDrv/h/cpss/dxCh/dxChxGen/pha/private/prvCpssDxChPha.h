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
* @file prvCpssDxChPha.h
*
* @brief Common private PHA declarations.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPhah
#define __prvCpssDxChPhah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpssCommon/cpssPresteraDefs.h>



/*************************** Constants definitions ***********************************/

/* The max value that the ERSPAN_III_FT support */
#define PRV_CPSS_DXCH_PHA_ERSPAN_III_FT_MAX_MAC     0x1F

/* Max value that the NSH Tenant ID field support */
#define PRV_CPSS_DXCH_PHA_NSH_TENANT_ID_MAX_MAC     0xFFFF

/* Max value that the NSH Source Class field support */
#define PRV_CPSS_DXCH_PHA_NSH_SOURCE_CLASS_MAX_MAC  0xFFFF

/* Indication that thread is not valid/not implemented */
#define PRV_CPSS_DXCH_PHA_INVALID_FW_THREAD_ADDRESS_CNS   0xFFFFFFFF

/**
 * @enum: HEADER_WINDOW_SIZE
 *
 * @brief: Maximum packet expansion options in PPN buffer
 */
typedef enum{

    /** 128B header with maximum expansion 32 bytes; Maximum expansion 32 bytes */
    EXPAND_32B,
    /** 112B header with maximum expansion 48 bytes; Maximum expansion 48 bytes */
    EXPAND_48B,
    /** 96B header with maximum expansion 64 bytes; Maximum expansion 64 bytes */
    EXPAND_64B
}HEADER_WINDOW_SIZE;

/**
 * @enum: HEADER_WINDOW_SIZE
 *
 * @brief: Start point options in PPN buffer
 */
typedef enum{

    /** Start of packet - offset 0 */
    Outer_L2_Start,
    /** L3 start byte without tunnel start. Tunnel L3 start with tunnel start */
    Outer_L3_Start,
    /** Passenger L2 start byte. Relevant only for tunnel start packets with passenger having L2 layer */
    Inner_L2_Start,
    /** Passenger L3 start byte. Relevant only for tunnel start packets */
    Inner_L3_Start
}HEADER_WINDOW_ANCHOR;

/* PHA FW thread ID */
typedef enum{
    THR_ID_0,
    THR_ID_1,
    THR_ID_2,
    THR_ID_3,
    THR_ID_4,
    THR_ID_5,
    THR_ID_6,
    THR_ID_7,
    THR_ID_8,
    THR_ID_9,
    THR_ID_10,
    THR_ID_11,
    THR_ID_12,
    THR_ID_13,
    THR_ID_14,
    THR_ID_15,
    THR_ID_16,
    THR_ID_17,
    THR_ID_18,
    THR_ID_19,
    THR_ID_20,
    THR_ID_21,
    THR_ID_22,
    THR_ID_23,
    THR_ID_24,
    THR_ID_25,
    THR_ID_26,
    THR_ID_27,
    THR_ID_28,
    THR_ID_29,
    THR_ID_30,
    THR_ID_31,
    THR_ID_32,
    THR_ID_33,
    THR_ID_34,
    THR_ID_35,
    THR_ID_36,
    THR_ID_37,
    THR_ID_38,
    THR_ID_39,
    THR_ID_40,
    THR_ID_41,
    THR_ID_42,
    THR_ID_43,
    THR_ID_44,
    THR_ID_45,
    THR_ID_46,
    THR_ID_47,
    THR_ID_48,
    THR_ID_49,
    THR_ID_50,
    THR_ID_51,
    THR_ID_52,
    THR_ID_53,
    THR_ID_54,
    THR_ID_55,
    THR_ID_56,
    THR_ID_57,
    THR_ID_58,
    THR_ID_59,
    THR_ID_60,
    THR_ID_61,
    THR_ID_62,
    THR_ID_63,
    THR_ID_64,
    THR_ID_65,
    THR_ID_66,
    THR_ID_67,
    THR_ID_68,
    THR_ID_69,
    THR_ID_70,
    THR_ID_71,
    THR_ID_72,
    THR_ID_73,
    THR_ID_74,
    THR_ID_75,
    THR_ID_76,
    THR_ID_77,
    THR_ID_78,
    THR_ID_79,
    TOTAL_THREADS_NUMBER,
} PHA_FW_THREADS_ID_ENT;

/* max number of threads that the firmware supports */
#define PRV_CPSS_DXCH_PHA_MAX_THREADS_CNS     TOTAL_THREADS_NUMBER


/*************************** Macro definitions ****************************************/

/* Macro to check PHA library was initialized properly as all PHA related APIs are not allowed if not initialized first */
#define PRV_CPSS_DXCH_PHA_LIB_INIT_CHECK_MAC(_devNum)                                             \
    if(GT_FALSE == PRV_CPSS_DXCH_PP_MAC(_devNum)->phaInfo.phaInitDone)                            \
    {                                                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "The PHA library was NOT initialized"); \
    }

/* Macro to check that the PHA threadId is valid value */
#define PRV_CPSS_DXCH_PHA_ERSPAN_III_FT_CHECK_MAC(_devNum, _erspanIII_FT)                          \
    if(_erspanIII_FT > (GT_U32)PRV_CPSS_DXCH_PHA_ERSPAN_III_FT_MAX_MAC)                            \
    {                                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "The Erspan_III_FT value is OUT OF RANGE"); \
    }

/* Macro to check that PHA NSH Tenant ID is valid value */
#define PRV_CPSS_DXCH_PHA_CLASSIFIER_NSH_OVER_VXLAN_GPE_TENANT_ID_CHECK_MAC(_devNum, _nshTenantId) \
    if(_nshTenantId > (GT_U32)PRV_CPSS_DXCH_PHA_NSH_TENANT_ID_MAX_MAC)                             \
    {                                                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "NSH Tenant ID value is OUT OF RANGE");     \
    }

/* Macro to check that PHA NSH Source Class is valid value */
#define PRV_CPSS_DXCH_PHA_CLASSIFIER_NSH_OVER_VXLAN_GPE_SOURCE_CLASS_CHECK_MAC(_devNum, _nshSourceClass) \
    if(_nshSourceClass > (GT_U32)PRV_CPSS_DXCH_PHA_NSH_SOURCE_CLASS_MAX_MAC)                             \
    {                                                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "NSH Source Class value is OUT OF RANGE");        \
    }



/*************************** Structure definitions ****************************************/

/**
 * @struct PRV_CPSS_DXCH_PHA_FW_THREAD_INFO_STC
 *
 * @brief Information per PHA FW threadId
*/
typedef struct {

    /** PC address of firmware thread */
    GT_U32               firmwareInstructionPointer;

    /** Skip counter value of firmware thread  */
    GT_U32               skipCounter;

    /** Packet start point in PPN buffer */
    HEADER_WINDOW_ANCHOR headerWindowAnchor;

    /** Maximum packet expansion in PPN buffer */
    HEADER_WINDOW_SIZE   headerWindowSize;
}PRV_CPSS_DXCH_PHA_FW_THREAD_INFO_STC;

/**
 * @struct PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC
 *
 * @brief Information about firmware image version
*/
typedef struct {

    /** Firmware image version number */
    GT_U32    imageVersion;

    /** Firmware image ID */
    GT_U32    imageId;

    /** Firmware image name */
    char      *imageName;
}PRV_CPSS_DXCH_PHA_FW_IMAGE_VERSION_INFO_STC;



/*************************** Functions declarations ****************************************/

/**
* @internal prvCpssDxChPhaThreadValidityCheck function
* @endinternal
*
* @brief   Check if the required firmware thread is valid (exists in current fw image)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - the device number
* @param[in] threadType            - type of the firmware thread that should handle the egress packet.
*
* @param[out] firmwareThreadIdPtr  - (pointer to) the firmware Thread Id
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid threadType
* @retval GT_BAD_STATE             - return thread ID is not in valid range
* @retval GT_NOT_SUPPORTED         - thread does not exist in fw image
*/
GT_STATUS prvCpssDxChPhaThreadValidityCheck
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadType,
    OUT GT_U32                          *firmwareThreadIdPtr
);

/**
* @internal prvCpssDxChPhaClockEnable function
* @endinternal
*
* @brief   enable/disable the clock to PPA in the PHA unit in the device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum   - device number.
* @param[in] enable   - Enable/Disable the clock to PPA in the PHA unit in the device.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhaClockEnable
(
    IN  GT_U8   devNum,
    IN GT_BOOL  enable
);





#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPhah */


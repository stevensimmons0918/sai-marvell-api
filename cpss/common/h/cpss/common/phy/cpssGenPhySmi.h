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
* @file common/h/cpss/common/phy/cpssGenPhySmi.h
*
* @brief API implementation for port Core Serial Management Interface facility.
*
*
* @version   14
********************************************************************************
*/

#ifndef __cpssGenPhySmi_h
#define __cpssGenPhySmi_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_PHY_SMI_INTERFACE_ENT
 *
 * @brief Generic Serial Management Interface numbering
 * used for both usual SMI and XSMI
*/
typedef enum{

    /** SMI interface\controller 0 */
    CPSS_PHY_SMI_INTERFACE_0_E = 0

    /** SMI interface\controller 1 */
    ,CPSS_PHY_SMI_INTERFACE_1_E

    /** @brief SMI interface\controller 2
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    ,CPSS_PHY_SMI_INTERFACE_2_E

    /** @brief SMI interface\controller 3
     *  (APPLICABLE DEVICES: Bobcat2)
     */
    ,CPSS_PHY_SMI_INTERFACE_3_E

    /** to simplify validity check */
    ,CPSS_PHY_SMI_INTERFACE_MAX_E

    ,CPSS_PHY_SMI_INTERFACE_INVALID_E = (~0)

} CPSS_PHY_SMI_INTERFACE_ENT;

/**
* @enum CPSS_PHY_XSMI_INTERFACE_ENT
 *
 * @brief XSMI interface number.
*/
typedef enum{

    /** @brief XSMI0 master interface number
     *  @brief Falcon 2T/4T-XSMI0 master interface number
     */
    CPSS_PHY_XSMI_INTERFACE_0_E,

    /** @brief XSMI1 master interface number
     *  @brief Falcon 12.8T-XSMI0 master interface number
     *  @brief Falcon 6.4T-XSMI0 master interface number
     */
    CPSS_PHY_XSMI_INTERFACE_1_E,

    /** @brief XSMI2 master interface number
     *  @brief Falcon 2T/4T-XSMI1 master interface number
     *  (APPLICABLE DEVICES: Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman)
     */
    CPSS_PHY_XSMI_INTERFACE_2_E,

    /** @brief XSMI3 master interface number
     *  @brief Falcon 6.4T-XSMI1 master interface number
     *  (APPLICABLE DEVICES: Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman)
     */
    CPSS_PHY_XSMI_INTERFACE_3_E,

    /** @brief XSMI4 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_4_E,

    /** @brief XSMI5 master interface number
     *  @brief Falcon 12.8T-XSMI1 master interface number
     *  @brief Falcon 6.4T-XSMI2 master interface number
     *  @brief Falcon 2T/4T-XSMI2 master interface number
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_PHY_XSMI_INTERFACE_5_E,

    /** @brief XSMI6 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_6_E,

    /** @brief XSMI7 master interface number
     *  @brief Falcon 6.4T-XSMI3 master interface number
     *  @brief Falcon 2T/4T-XSMI3 master interface number
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_PHY_XSMI_INTERFACE_7_E,

    /** @brief XSMI8 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_8_E,

    /** @brief XSMI9 master interface number
     *  @brief Falcon 12.8T-XSMI2 master interface number
     * (APPLICABLE DEVICES: Falcon)
     */
    CPSS_PHY_XSMI_INTERFACE_9_E,

    /** @brief XSMI10 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_10_E,

    /** @brief XSMI11 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_11_E,

    /** @brief XSMI12 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_12_E,

    /** @brief XSMI13 master interface number
     *  @brief Falcon 12.8T-XSMI3 master interface number
     * (APPLICABLE DEVICES: Falcon)
     */
    CPSS_PHY_XSMI_INTERFACE_13_E,

    /** @brief XSMI14 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_14_E,

    /** @brief XSMI15 master interface number
     *  (APPLICABLE DEVICES: None)
     */
    CPSS_PHY_XSMI_INTERFACE_15_E,

    /** @brief to simplify validity check */
    CPSS_PHY_XSMI_INTERFACE_MAX_E
} CPSS_PHY_XSMI_INTERFACE_ENT;

/**
* @enum CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT
 *
 * @brief Generic Serial Management Interface clock (MDC)
 * division factor
*/
typedef enum{

    /** 8 division: Core clock divided by 8 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_8_E,

    /** 16 division: Core clock divided by 16 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_16_E,

    /** 32 division: Core clock divided by 32 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E,

    /** 64 division: Core clock divided by 64 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_64_E,

    /** 128 division: Core clock divided by 128 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_128_E,

    /** 256 division: Core clock divided by 256 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_256_E,

    /** 512 division: Core clock divided by 512 */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_512_E,

    /** to simplify validity check */
    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_MAX_E

} CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT;

/*******************************************************************************
* CPSS_VCT_PHY_READ_PHY_REGISTER_FUN
*
* DESCRIPTION:
*       Read specified SMI Register on a specified port on specified device.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev    - physical device number
*       port   - physical port number
*       phyReg - SMI register
*
* OUTPUTS:
*       data  - data read.
*
* RETURNS:
*       GT_OK       - on success
*       GT_FAIL     - on error
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_VCT_PHY_READ_PHY_REGISTER_FUN)
(
     IN  GT_U8     dev,
     IN  GT_PHYSICAL_PORT_NUM  port,
     IN  GT_U8     phyReg,
     OUT GT_U16    *data
);
/*******************************************************************************
* CPSS_VCT_PHY_WRITE_PHY_REGISTER_FUN
*
* DESCRIPTION:
*       Write value to specified SMI Register on a specified port on
*       specified device.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       dev    - physical device number
*       port   - physical port number
*       phyReg - SMI register
*       data   - value to write
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK       - on success
*       GT_FAIL     - on error
*       GT_HW_ERROR - on hardware error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_VCT_PHY_WRITE_PHY_REGISTER_FUN)
(
     IN  GT_U8     dev,
     IN  GT_PHYSICAL_PORT_NUM  port,
     IN  GT_U8     phyReg,
     IN  GT_U16    data
);

/*******************************************************************************
* CPSS_VCT_PHY_AUTONEG_SET_FUN
*
* DESCRIPTION:
*       This function sets the auto negotiation process between the PP and
*       Phy status.
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number
*       port            - port number
*       enable          - enable/disable Auto Negotiation status
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number
*
* COMMENTS:
*

*
*******************************************************************************/
typedef GT_STATUS (*CPSS_VCT_PHY_AUTONEG_SET_FUN)
(
     IN  GT_U8     devNum,
     IN  GT_PHYSICAL_PORT_NUM   port,
     IN  GT_BOOL   enable
);

/*******************************************************************************
* CPSS_VCT_PHY_AUTONEG_GET_FUN
*
* DESCRIPTION:
*       This function sets the auto negotiation process between the PP and
*       Phy status.
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum          - the device number
*       port            - port number
*       enablePtr       - read Auto Negotiation status
* OUTPUTS:
*       none.
*
* RETURNS:
*       GT_OK       - successful completion
*       GT_FAIL     - an error occurred.
*       GT_HW_ERROR - on hardware error
*       GT_BAD_PARAM - bad device number
*
* COMMENTS:
*

*
*******************************************************************************/
typedef GT_STATUS (*CPSS_VCT_PHY_AUTONEG_GET_FUN)
(
     IN  GT_U8     devNum,
     IN  GT_PHYSICAL_PORT_NUM     port,
     OUT GT_BOOL   *enablePtr
);

/**
* @struct CPSS_VCT_GEN_BIND_FUNC_STC
 *
 * @brief A structure to hold common VCT functions for PP Family needed
 * in CPSS
 * INFO "PER DEVICE FAMILY"
*/
typedef struct{

    /** pointer to read phy register function; */
    CPSS_VCT_PHY_READ_PHY_REGISTER_FUN cpssPhyRegisterRead;

    /** pointer to write phy register function; */
    CPSS_VCT_PHY_WRITE_PHY_REGISTER_FUN cpssPhyRegisterWrite;

    /** pointer to set phy auto */
    CPSS_VCT_PHY_AUTONEG_SET_FUN cpssPhyAutoNegotiationSet;

    /** pointer to get phy auto */
    CPSS_VCT_PHY_AUTONEG_GET_FUN cpssPhyAutoNegotiationGet;

} CPSS_VCT_GEN_BIND_FUNC_STC;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenPhySmi_h */


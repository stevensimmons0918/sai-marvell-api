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
* @file prvCpssDxChBootChannelHandler.h
*
* @brief private CPSS BootChannelHandler facility API.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChBootChannelHandlerh
#define __prvCpssDxChBootChannelHandlerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/generic/version/gtVersion.h>


/**
* @struct PRV_CPSS_BOOT_CH_PORT_STATUS_STC
 *
 * @brief This structure contains the port info returned from MI (HS process).
*/
typedef struct{
    GT_BOOL                               isLinkUp;
    CPSS_PORT_INTERFACE_MODE_ENT          ifMode;
    CPSS_PORT_SPEED_ENT                   speed;
    CPSS_PORT_FEC_MODE_ENT                fecMode;
    GT_BOOL                               apMode;
}PRV_CPSS_BOOT_CH_PORT_STATUS_STC;



/* Taken from freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/boot_channel.h */
typedef enum {  PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_EPROM_LIKE_E = 2,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_CLI_LIKE_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_RERUN_FILES_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_ALL_FILES,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_SET_PORT_CONFIG = 10,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_SET_PRBS,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_PRBS,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_SET_SERDES_TX,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_SET_SERDES_RX,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_AP_PORT_CONFIG,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_UPGRADE_FW_E = 16,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_AP_ADV_RX_CFG = 17,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_AP_ADV_TX_CFG = 18,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_LOOPBACK_MODE_E = 19,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_SERDES_TX = 21,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_SERDES_RX = 22,
                PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_GET_HA_ADDRESSES = 0x100
} PRV_CPSS_MICRO_INIT_BOOT_CH_OPCODE_ENT;

typedef enum {  PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_MAGIC_KEY_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_BOOT_STATE_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_SPI_LOG_ADDR_E,
                PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REG_FW_DEBUG_FLAGS_E
} PRV_CPSS_MICRO_INIT_BOOT_CH_STATUS_REGS_ENT;


/**
* @internal prvCpssDxChBootChannelHandlerVersionGet function
* @endinternal
*
* @brief   This function gets superImage, microInit and booton versions
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                     - device number
* @param[OUT] superImageVersionPtr      - (pointer to) super image version .
* @param[OUT] miVersionPtr              - (pointer to) MI version .
* @param[OUT] bootonVersionPtr          - (pointer to) booton version .
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE        - not applicable device
*
*/
GT_STATUS prvCpssDxChBootChannelHandlerVersionGet
(
    IN   GT_U8                      devNum,
    OUT  CPSS_VERSION_INFO_STC     *superImageVersionPtr,
    OUT  CPSS_VERSION_INFO_STC     *miVersionPtr,
    OUT  CPSS_VERSION_INFO_STC     *bootonVersionPtr
);

/**
* @internal prvCpssDxChBootChannelHandlerPortStatusGet function
* @endinternal
*
* @brief  Gets link Status the speed and mode of the port configured by MI.
*
* @note   APPLICABLE DEVICES:       AC5P;AC5X;HARRIER;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[out] portInfoPtr             - (pointer to) port info stc
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_SUPPORTED         - on not supported port mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChBootChannelHandlerPortStatusGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT PRV_CPSS_BOOT_CH_PORT_STATUS_STC      *portInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBootChannelHandler */


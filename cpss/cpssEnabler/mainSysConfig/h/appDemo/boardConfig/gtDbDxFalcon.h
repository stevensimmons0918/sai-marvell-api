/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxFalconRdBoard.h
*
* @brief Initialization functions for the Falcon - SIP6 - RD board.
*
* @version   1
********************************************************************************
*/
#ifndef __gtDbDxFalcon_H
#define __gtDbDxFalcon_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/phy/gtAppDemoPhyConfig.h>

#define RD_FIRST_PORT_NUM                 128
#define RD_LAST_PORT_NUM                  255
#define RD_NUM_OF_SLOTS                   8
#define RD_NUM_OF_PORTS_PER_SLOT          (( RD_LAST_PORT_NUM - RD_FIRST_PORT_NUM + 1) / RD_NUM_OF_SLOTS)
#define RD_NUM_OF_PHY_PER_SLOT            4

#define RD_SHORT_TRACE_PARAMS             0
#define RD_LONG_TRACE_PARAMS              1

#define RD_GET_SLOT_NUM_FROM_PORT_NUM_MAC(falconPortNum,slotNum) \
        slotNum = ((falconPortNum - RD_FIRST_PORT_NUM) / RD_NUM_OF_PORTS_PER_SLOT);

#define RD_PHY_SHORT_OR_LONG_TRACE_PORT_PARAMS(falconPortNum) \
        (GT_APPDEMO_XPHY_BOTH_SIDES * (((falconPortNum >= 144) && (falconPortNum <= 239)) ? RD_SHORT_TRACE_PARAMS : RD_LONG_TRACE_PARAMS))

#define RD_FALCON_SHORT_OR_LONG_TRACE_PORT_PARAMS(falconPortNum) \
        (((falconPortNum >= 160) && (falconPortNum <= 223)) ? RD_SHORT_TRACE_PARAMS : RD_LONG_TRACE_PARAMS)

#define FALCON_RD_MAC_TO_SERDES_MAP_ARR_SIZE     32
#define FALCON_RD_POLARITY_ARR_SIZE              258
#define FALCON_RD_PHY_INFO_ARR_SIZE              32
#define FALCON_RD_PHY_TUNE_PARAMS_ARR_SIZE       GT_APPDEMO_XPHY_BOTH_SIDES*2

#define FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE   16
#define FALCON_6_4T_POLARITY_ARR_SIZE            132

extern GT_APPDEMO_XPHY_INFO_STC*          falcon_PhyInfo;

extern CPSS_PORT_MAC_TO_SERDES_STC        falcon_RD_MacToSerdesMap[FALCON_RD_MAC_TO_SERDES_MAP_ARR_SIZE];
extern APPDEMO_SERDES_LANE_POLARITY_STC   falcon_RD_PolarityArray[FALCON_RD_POLARITY_ARR_SIZE];
extern GT_APPDEMO_XPHY_INFO_STC           falcon_RD_PhyInfo[FALCON_RD_PHY_INFO_ARR_SIZE];
extern CPSS_PORT_SERDES_TUNE_STC          falcon_RD_PhyTuneParams[FALCON_RD_PHY_TUNE_PARAMS_ARR_SIZE];
extern CPSS_PORT_SERDES_TX_CONFIG_STC     falcon_RD_serdesTxParams[2];

extern CPSS_PORT_MAC_TO_SERDES_STC        falcon_6_4T_MacToSerdesMap[FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE];
extern APPDEMO_SERDES_LANE_POLARITY_STC   falcon_6_4T_PolarityArray[FALCON_6_4T_POLARITY_ARR_SIZE];

/*
 * typedef: struct CPSS_FALCON_LED_STREAM_INDICATIONS_STC
 *
 * Description:
 *      Positions of LED bit indications in stream.
 *
 * Fields:
 *
 *      ledStart            - The first bit in the LED stream indication to be driven in current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      ledEnd              - The last bit in the LED stream indication to be driven in the current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      cpuPort             - CPU port is connected to the current chiplet
 */
typedef struct {
    GT_U32     ledStart;
    GT_U32     ledEnd;
    GT_BOOL    cpuPort;
} CPSS_FALCON_LED_STREAM_INDICATIONS_STC;

/**
 * @internal falcon_phy_getPortNum
 * @endinternal
 *
 * @param devNum
 * @param falconPortNum
 * @param phyIndex
 * @param phyPortNum
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_getPortNum
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_U32                   falconPortNum,
    OUT GT_U32                   *phyIndex,
    OUT GT_U32                   *phyPortNum
);

/**
 * @internal falcon_phy_deletePort
 * @endinternal
 *
 *  @brief   Power down PHY port
 *
 * @param devNum
 * @param falconPortNum
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 */
GT_STATUS falcon_phy_deletePort
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_U32                   falconPortNum
);

/**
* @internal falcon_phy_createPort function
* @endinternal
*
* @brief   Create PHY port
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] falconPortNum         - The CPSS portNum.
* @param[in] opMode                - PHY port's operation mode.
* @param[in] fecMode               - PHY port's FEC mode
* @param[in] shortChannel          - short/long channel boolean.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_phy_createPort
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_U32                   falconPortNum,
    IN  GT_APPDEMO_XPHY_OP_MODE  opMode,
    IN  GT_APPDEMO_XPHY_FEC_MODE fecModeHost,
    IN  GT_APPDEMO_XPHY_FEC_MODE fecModeLine
);

/**
 * @internal falcon_phy_GetThermal
 *
 * @param devNum
 * @param falconPortNum
 * @param coreTemperature
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_GetThermal
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    OUT GT_32 *coreTemperature

);

/**
* @internal falcon_phy_tunePort function
* @endinternal
*
* @brief   Tune PHY port
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] falconPortNum         - The CPSS portNum.
* @param[in] hostOrLine            - Choose Host/Line/Both sides
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_phy_tunePort
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      falconPortNum,
    IN  GT_APPDEMO_XPHY_HOST_LINE   hostOrLine
);

/**
 * @internal falcon_phy_getPortLinkStatus
 * @endinternal
 *
 * @brief Get PCS link status
 *
 * @param devNum
 * @param falconPortNum
 * @param linkStatus
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_getPortLinkStatus
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    OUT GT_U16                          *linkStatus
);

/**
 * @internal falcon_RD_ledInit  function
 * @endinternal
 *
 * @param devNum
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_ledInit
(
    GT_U8 devNum
);


/**
 * @internal falcon_RD_XSmiSelectCard
 * @endinternal
 *
 * @param hostDevNum
 * @param portGroupId
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_XSmiSelectCard
(
     GT_U8    hostDevNum,
     GT_U32   portGroupId
);

/**
 * @internal falcon_get_mpd_if_index function
 * @endinternal
 *
 */
GT_STATUS falcon_get_mpd_if_index
(
    IN GT_U32    devNum,
    IN GT_U32    portNum,
    OUT GT_U32   *mpd_ifIndex
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif





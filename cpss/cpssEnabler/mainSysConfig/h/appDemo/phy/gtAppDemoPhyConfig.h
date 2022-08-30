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
* @file gtAppDemoPhyConfig.h
*
* @brief Generic support for PHY init.
*
* @version   1
********************************************************************************
*/
#ifndef __gtAppDemoPhyConfigh
#define __gtAppDemoPhyConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/smi/cpssGenSmi.h>

#define SKIP_PHY 0xffff
#ifdef INCLUDE_MPD
#undef MAX_UINT_8
#undef MIN
#include <mpdPrv.h>
#endif
typedef enum
{
    GT_APPDEMO_UNKNOWN_XPHY_MODEL,
    GT_APPDEMO_XPHY_88X7120,
    GT_APPDEMO_XPHY_88X7121,

    GT_APPDEMO_XPHY_MODEL_LAST = GT_APPDEMO_XPHY_88X7121,
} GT_APPDEMO_XPHY_MODEL;

typedef struct GT_APPDEMO_XPHY_INFO_STCT
{
    GT_U32                              phyIndex;
    GT_U32                              portGroupId;
    CPSS_PHY_XSMI_INTERFACE_ENT         xsmiInterface;
    GT_U16                              phyAddr;
    GT_U32                              phyType;
    GT_U32                              hostDevNum;
    GT_VOID_PTR                         driverObj;
} GT_APPDEMO_XPHY_INFO_STC;

typedef GT_APPDEMO_XPHY_INFO_STC* GT_APPDEMO_XPHY_INFO_PTR;

typedef enum
{   /* Changes in this enum order must be made in conjunction with changes of the PHYs' translation tables  */
    RETIMER_HOST_25G_KR_LINE_25G_KR,
    RETIMER_HOST_50G_KR_LINE_50G_KR,
    RETIMER_HOST_50G_KR2_LINE_50G_KR2,
    RETIMER_HOST_100G_KR2_LINE_100G_KR2,
    RETIMER_HOST_100G_KR4_LINE_100G_KR4,
    RETIMER_HOST_200G_KR4_LINE_200G_KR4,
    RETIMER_HOST_200G_KR8_LINE_200G_KR8,
    RETIMER_HOST_400G_KR8_LINE_400G_KR8,
    GT_APPDEMO_XPHY_OP_MODE_LAST_RETIMER = RETIMER_HOST_400G_KR8_LINE_400G_KR8,

    PCS_HOST_25G_KR_LINE_25G_KR,
    PCS_HOST_50G_KR_LINE_50G_KR,
    PCS_HOST_100G_KR2_LINE_100G_CR4,
    PCS_HOST_100G_KR2_LINE_100G_SR4,
    PCS_HOST_100G_KR2_LINE_100G_LR4,
    PCS_HOST_100G_KR2_AP_LINE_100G_LR4_AP,
    PCS_HOST_100G_KR2_AP_LINE_100G_SR4,
    PCS_HOST_100G_KR2_LINE_100G_KR2,
    PCS_HOST_100G_KR2_LINE_100G_CR2,

    GT_APPDEMO_XPHY_OP_MODE_LAST
} GT_APPDEMO_XPHY_OP_MODE;

typedef struct
{
   CPSS_PORT_SPEED_ENT               speed;
   CPSS_PORT_INTERFACE_MODE_ENT      ifMode;
   CPSS_PORT_FEC_MODE_ENT            fecMode;
}GT_APPDEMO_XPHY_MPD_PARAM_STC;

typedef struct
{
    GT_APPDEMO_XPHY_MPD_PARAM_STC hostSide;
    GT_APPDEMO_XPHY_MPD_PARAM_STC lineSide;
    GT_BOOL                       isRetimerMode;
}GT_APPDEMO_XPHY_SPEED_EXT_STC;

typedef enum
{
    GT_APPDEMO_XPHY_NO_FEC,             /* No FEC            */
    GT_APPDEMO_XPHY_FC_FEC,             /* FC-FEC (Firecode) */
    GT_APPDEMO_XPHY_RS_FEC,             /* RS-FEC (528, 514) */
    GT_APPDEMO_XPHY_RS_FEC_544_514,     /* RS-FEC (544, 514) */

    GT_APPDEMO_XPHY_FEC_MODE_LAST
} GT_APPDEMO_XPHY_FEC_MODE;

typedef enum
{
    GT_APPDEMO_XPHY_HOST_SIDE,
    GT_APPDEMO_XPHY_LINE_SIDE,
    GT_APPDEMO_XPHY_BOTH_SIDES
} GT_APPDEMO_XPHY_HOST_LINE;

typedef enum
{
    GT_APPDEMO_XPHY_SERDES_TX,
    GT_APPDEMO_XPHY_SERDES_RX,
    GT_APPDEMO_XPHY_BOTH
} GT_APPDEMO_XPHY_SERDES_PARAMS;

/*************************** Bind Functions Headers ***************************/
#ifndef ASIC_SIMULATION
typedef GT_STATUS (*GT_APPDEMO_XPHY_INIT_DRIVER_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_BOOL                      loadImage
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_ARRAY_INIT_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfoArr,
    GT_U32                       phyInfoArrSize,
    CPSS_PHY_XSMI_INTERFACE_ENT  broadcastInterface,
    GT_U16                       broadcastAddr,
    GT_BOOL                      loadImage
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_UNLOAD_DRIVER_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_SET_PORT_MODE_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_U32                       portsBmp,
    GT_APPDEMO_XPHY_OP_MODE      opMode,
    GT_APPDEMO_XPHY_FEC_MODE     fecMode,
    GT_VOID_PTR                  args
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_GET_PORT_MODE_FUNC_PTR)
(
   GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
   GT_U32                       portNum,
   GT_APPDEMO_XPHY_OP_MODE      *opMode,
   GT_APPDEMO_XPHY_FEC_MODE     *fecMode,
   GT_VOID_PTR                  args
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_GET_PORT_LANE_BMP_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_U32                       portNum,
    GT_U32                       *hostSideLanesBmp,
    GT_U32                       *lineSideLanesBmp
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_PORT_POWERDOWN_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_U32                       portNum
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_GET_PORT_LINK_STATUS_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_U32                       portNum,
    GT_U16                       *linkStatus
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_LANE_PLOARITY_SWAP_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR             phyInfo,
    GT_APPDEMO_XPHY_HOST_LINE            hostOrLineSide,
    APPDEMO_SERDES_LANE_POLARITY_STC*    polaritySwapParams
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_SERDES_TUNE_FUNC_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    GT_U32                                   lanesBmp,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_SERDES_TX_CONFIG_SET_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    GT_U32                                   lanesBmp,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_PKT_GEN_GET_COUNTER_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide,
    GT_U16  whichCounter
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_PKT_GEN_COUNTER_RESET_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_PKT_GEN_CHKR_ENABLE_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide,
    GT_BOOL  enableGenerator ,
    GT_BOOL  enableChecker
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_PKT_GEN_CHKR_CONFIG_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide,
    GT_BOOL  readToClear,
    GT_U16   frameLengthControl
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_LOOPBACK_SET_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    GT_U16                                   loopback_type ,
    GT_U16               enable
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_DIAG_DUMP_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide
);


typedef GT_STATUS (*GT_APPDEMO_XPHY_GET_TEMPERATURE_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_32                                    *temperature
);

typedef GT_STATUS (*GT_APPDEMO_XPHY_XSMI_EXTENTION_PTR)
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    void                                     *callback
);

typedef struct
{
    GT_APPDEMO_XPHY_INIT_DRIVER_FUNC_PTR              gtAppDemoPhyInitDriverFunc;
    GT_APPDEMO_XPHY_ARRAY_INIT_FUNC_PTR               gtAppDemoPhyArrayInitFunc;
    GT_APPDEMO_XPHY_UNLOAD_DRIVER_FUNC_PTR            gtAppDemoPhyUnloadDriverFunc;
    GT_APPDEMO_XPHY_SET_PORT_MODE_FUNC_PTR            gtAppDemoPhySetPortModeFunc;
    GT_APPDEMO_XPHY_GET_PORT_MODE_FUNC_PTR            gtAppDemoPhyGetPortModeFunc;
    GT_APPDEMO_XPHY_PORT_POWERDOWN_FUNC_PTR           gtAppDemoPhyPortPowerdownFunc;
    GT_APPDEMO_XPHY_GET_PORT_LINK_STATUS_FUNC_PTR     gtAppDemoPhyGetPortLinkStatusFunc;
    GT_APPDEMO_XPHY_GET_PORT_LANE_BMP_FUNC_PTR        gtAppDemoPhyGetPortLanesBmpFunc;
    GT_APPDEMO_XPHY_LANE_PLOARITY_SWAP_FUNC_PTR       gtAppDemoPhySetLanePolaritySwapFunc;
    GT_APPDEMO_XPHY_SERDES_TUNE_FUNC_PTR              gtAppDemoPhySerdesTuneFunc;
    GT_APPDEMO_XPHY_SERDES_TX_CONFIG_SET_PTR          gtAppDemoPhySerdesTxConfigSetFunc;
    GT_APPDEMO_XPHY_PKT_GEN_GET_COUNTER_PTR           gtAppDemoPhyPktGeneratorGetCounterFunc;
    GT_APPDEMO_XPHY_PKT_GEN_COUNTER_RESET_PTR         gtAppDemoPhyPktGeneratorCounterResetFunc;
    GT_APPDEMO_XPHY_PKT_GEN_CHKR_ENABLE_PTR           gtAppDemoPhyEnablePktGeneratorCheckerFunc;
    GT_APPDEMO_XPHY_PKT_GEN_CHKR_CONFIG_PTR           gtAppDemoPhyConfigurePktGeneratorCheckerFunc;
    GT_APPDEMO_XPHY_LOOPBACK_SET_PTR                  gtAppDemoPhyLoopbackFunc;
    GT_APPDEMO_XPHY_DIAG_DUMP_PTR                     gtAppDemoPhyDiagStateDumpFunc;
    GT_APPDEMO_XPHY_GET_TEMPERATURE_PTR               gtAppDemoPhyGetTemperatureFunc;
    GT_APPDEMO_XPHY_XSMI_EXTENTION_PTR                gtAppDemoPhyXSmiExtentionFunc;
} GT_APPDEMO_XPHY_FUNC_PTRS;

GT_STATUS gtAppDemoPhy7120BindFuncPtr(GT_APPDEMO_XPHY_FUNC_PTRS *funcPtrArray);

/**
 *
 * gtAppDemoPhy7120BindXSmiExtnFuncPtr
 *
 * @param funcPtrArray
 * @param funcPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120BindXSmiExtnFuncPtr(GT_APPDEMO_XPHY_FUNC_PTRS *funcPtrArray, const void * funcPtr);

/*********************************** APIs ************************************/

/**
 * gtAppDemoPhyBindFuncPtr
 *
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyBindFuncPtr
(
);

/**
 * gtAppDemoPhyBindXSmiExtnFuncPtr
 *
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyBindXSmiExtnFuncPtr
(
    const void * funcPtr
);

/**
 * gtAppDemoPhyXSmiExtention
 *
 *
 * @param devNum
 * @param portGroupId
 * @param cardNum
 * @param callback
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyXSmiExtention
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    void                         *callback
);

/**
 * gtAppDemoPhyApOpModeDetect
 *
 * @param opMode
 * @param isHostOpModeAp
 * @param isLineOpModeAp
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyApOpModeDetect
(
    IN  GT_APPDEMO_XPHY_OP_MODE  opMode,
    OUT GT_BOOL *isHostOpModeAp,
    OUT GT_BOOL *isLineOpModeAp
);

/**
 * gtAppDemoPhyInitDriver
 *
 *
 * @param phyInfo
 * @param loadImage
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyInitDriver
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_BOOL                      loadImage
);

/**
 * gtAppDemoPhyArrayInit
 *
 *
 * @param phyInfoArr
 * @param phyInfoArrSize
 * @param broadcastMode
 * @param broadcastInterface
 * @param broadcastAddr
 * @param loadImage
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyArrayInit
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfoArr,
    GT_U32                       phyInfoArrSize,
    GT_BOOL                      broadcastMode,
    CPSS_PHY_XSMI_INTERFACE_ENT  broadcastInterface,
    GT_U16                       broadcastAddr,
    GT_BOOL                      loadImage
);

/**
 * gtAppDemoPhyUnloadDriver
 *
 *
 * @param phyInfo
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyUnloadDriver
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo
);

/**
 * gtAppDemoPhySetPortMode
 *
 *
 * @param portNum
 * @param speedParam
 * @param apEnableHost
 * @param apEnableLine
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhySetPortMode
(
   GT_U32                        portNum,
   GT_APPDEMO_XPHY_SPEED_EXT_STC speedParam,
   GT_U32                        apEnableHost,
   GT_U32                        apEnableLine
);

/**
 * gtAppDemoPhyGetPortMode
 *
 * @param portNum
 * @param opMode
 * @param fecModeHost
 * @param fecModeLine
 * @param apEnableHost
 * @param apEnableLine
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyGetPortMode
(
   GT_U32                       portNum,
   GT_APPDEMO_XPHY_OP_MODE      *opMode,
   GT_APPDEMO_XPHY_FEC_MODE     *fecModeHost,
   GT_APPDEMO_XPHY_FEC_MODE     *fecModeLine,
   GT_U32                       *apEnableHost,
   GT_U32                       *apEnableLine
);

/**
 * gtAppDemoPhyPortPowerdown
 *
 *
 * @param phyInfo
 * @param portNum
 * @param enable
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPortPowerdown
(
   GT_SW_DEV_NUM            devNum,
   GT_U32                   portNum,
   GT_BOOL                  enable
);


GT_STATUS gtAppDemoPhyGetPortLinkStatus
(
    GT_U32                       portNum,
    GT_U16                       *linkStatus
);

/**
 * gtAppDemoPhySetLanePolaritySwap
 *
 *
 * @param phyInfo
 * @param hostOrLineSide
 * @param polaritySwapParamsPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhySetLanePolaritySwap
(
   GT_APPDEMO_XPHY_INFO_PTR             phyInfo,
   GT_APPDEMO_XPHY_HOST_LINE            hostOrLineSide,
   APPDEMO_SERDES_LANE_POLARITY_STC     *polaritySwapParamsPtr
);

/**
 * gtAppDemoPhySerdesTune
 *
 *
 * @param phyInfo
 * @param hostOrLineSide
 * @param lanesBmp
 * @param tuneParamsPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhySerdesTune
(
    GT_U32                                   portNum,
    GT_U32                                   laneBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
);

/**
 * gtAppDemoPhySerdesTxConfigSet
 *
 *
 * @param phyInfo
 * @param hostOrLineSide
 * @param lanesBmp
 * @param tuneParamsPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhySerdesTxConfigSet
(
    GT_U32                                   portNum,
    GT_U32                                   laneBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
);

/**
 * gtAppDemoPhyIsPcsMode
 *
 *
 * @param opMode
 *
 * @return GT_BOOL
 */
GT_BOOL gtAppDemoPhyIsPcsMode
(
    GT_APPDEMO_XPHY_OP_MODE opMode
);

/**
 * gtAppDemoPhyPolarityArraySet
 *
 *
 * @param phyInfo
 * @param hostOrLineSide
 * @param lanesBmp
 * @param polaritySwapParamsArr
 * @param polaritySwapParamsArrSize
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPolarityArraySet
(
   GT_U32                               portNum,
   GT_APPDEMO_XPHY_HOST_LINE            hostOrLineSide,
   GT_U32                               lanesBmp,
   APPDEMO_SERDES_LANE_POLARITY_STC     *polaritySwapParamsArr,
   GT_U32                               polaritySwapParamsArrSize
);

/**
 * gtAppDemoPhyPortTune
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostSideTuneParamsPtr
 * @param lineSideTuneParamsPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPortTune
(
    GT_U32                                   portNum,
    GT_U32                                   lanesBmpHost,
    GT_U32                                   lanesBmpLine,
    CPSS_PORT_SERDES_TUNE_STC                *hostSideTuneParamsPtr,
    CPSS_PORT_SERDES_TUNE_STC                *lineSideTuneParamsPtr
);

/**
 * gtAppDemoPhyPortTxConfigSet
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostSideTuneParamsPtr
 * @param lineSideTuneParamsPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPortTxConfigSet
(
   GT_U32                            portNum,
   GT_U32                            lanesBmpHost,
   GT_U32                            lanesBmpLine,
   CPSS_PORT_SERDES_TUNE_STC         *hostSideTuneParamsPtr,
   CPSS_PORT_SERDES_TUNE_STC         *lineSideTuneParamsPtr
);

/**
 * gtAppDemoPhyPortSetAndTune
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param opMode
 * @param fecMode
 * @param serdesTuneParams
 * @param args
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPortSetAndTune
(
   GT_U32                       portNum,
   GT_APPDEMO_XPHY_OP_MODE      opMode,
   GT_APPDEMO_XPHY_FEC_MODE     fecMode,
   CPSS_PORT_SERDES_TUNE_STC    serdesTuneParams[GT_APPDEMO_XPHY_BOTH_SIDES]
);

/**
 * gtAppDemoPhyPktGeneratorGetCounter
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostOrLineSide
 * @param whichCounter
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPktGeneratorGetCounter
(
    IN GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    IN GT_U32                       portsBmp,
    IN GT_APPDEMO_XPHY_HOST_LINE    hostOrLineSide,
    IN GT_U16                       whichCounter
);

/**
 * gtAppDemoPhyPktGeneratorCounterReset
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostOrLineSide
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyPktGeneratorCounterReset
(
    IN GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    IN GT_U32                       portsBmp,
    IN GT_APPDEMO_XPHY_HOST_LINE    hostOrLineSide
);

/**
 * gtAppDemoPhyEnablePktGeneratorChecker
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostOrLineSide
 * @param enableGenerator
 * @param enableChecker
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyEnablePktGeneratorChecker
(
    IN GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    IN GT_U32                       portsBmp,
    IN GT_APPDEMO_XPHY_HOST_LINE    hostOrLineSide,
    IN GT_BOOL                      enableGenerator,
    IN GT_BOOL                      enableChecker
);

/**
 * gtAppDemoPhyConfigurePktGeneratorChecker
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostOrLineSide
 * @param readToClear
 * @param frameLengthControl
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyConfigurePktGeneratorChecker
(
    IN GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    IN GT_U32                       portsBmp,
    IN GT_APPDEMO_XPHY_HOST_LINE    hostOrLineSide,
    IN GT_BOOL                      readToClear,
    IN GT_U16                       frameLengthControl
);

/**
 * gtAppDemoPhyLoopback
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostOrLineSide
 * @param loopback_type
 * @param enable
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyLoopback
(
    IN GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    IN GT_U32                       portsBmp,
    IN GT_APPDEMO_XPHY_HOST_LINE    hostOrLineSide,
    IN  GT_U16                      loopback_type,
    IN  GT_U16                      enable
);

/**
 * gtAppDemoPhyGetTemperature
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param temperature
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyGetTemperature
(
    IN GT_U32                       devNum,
    IN GT_U32                       portNum,
    OUT GT_32                       *temperature
);

/**
 * gtAppDemoPhyDiagStateDump
 *
 *
 * @param phyInfo
 * @param portsBmp
 * @param hostOrLineSide
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyDiagStateDump
(
    IN GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    IN GT_U32                       portsBmp,
    IN GT_APPDEMO_XPHY_HOST_LINE    hostOrLineSide
) ;

GT_STATUS gtAppDemoPhySerdesConfigSet
(
    GT_U32                                   portNum,
    GT_U32                                   lanebmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
);

#ifdef INCLUDE_MPD
GT_STATUS gtAppDemoPhyMpdInit
(
    GT_U8 devNum,
    GT_U8 boardRevId
);

GT_U32 gtAppDemoPhyMpdIndexGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum
);

GT_STATUS phySetPortAdminOn
(
    IN GT_U32              devNum,
    IN GT_U16              port,
    IN GT_BOOL             status
);

GT_STATUS phyEnableSpeed
(
    IN GT_U32              devNum,
    IN GT_U16              port,
    IN MPD_SPEED_ENT       portSpeed
);

MPD_SPEED_ENT phyMpdSpeedGet
(
    GT_U16 speed_bits
);

#endif

#define GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo) \
    if(phyInfo == NULL || phyInfo->driverObj == NULL)        \
    {                                                        \
        return GT_NOT_INITIALIZED;                           \
    }

#endif /* ASIC_SIMULATION */
#endif /*__gtAppDemoPhyConfigh*/


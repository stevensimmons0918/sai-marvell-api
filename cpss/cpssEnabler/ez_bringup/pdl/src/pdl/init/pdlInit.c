/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlInit.c   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Platform driver layer - PDL driver initialization
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/button/private/prvPdlBtn.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/sensor/private/prvPdlSensor.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/serdes/private/prvPdlSerdes.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/power_supply/private/prvPdlPower.h>
#include <pdl/cpu/private/prvPdlCpu.h>
#include <pdl/oob_port/private/prvPdlOobPort.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/init/pdlInit.h>
#include <pdl/fan/private/prvPdlFanTc654.h>
#include <pdl/fan/private/prvPdlFanAdt7476.h>
#include <pdl/fan/private/prvPdlFanEMC2305.h>
#include <pdl/fan/private/prvPdlFanPWM.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

static BOOLEAN                                 pdlFanInternalDriverBind = FALSE;
static XML_PARSER_ROOT_DESCRIPTOR_TYP          pdlInitxmlRootId;

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                  \
    if (__pdlStatus != PDL_OK) {                                                                       \
    if (prvPdlInitDebugFlag) {                                                                         \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __pdlStatus;                                                                                \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                  \
    if (__xmlStatus != PDL_OK) {                                                                       \
    if (prvPdlInitDebugFlag) {                                                                         \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __xmlStatus;                                                                                \
    }  

/**
 * @fn  PDL_STATUS pdlInitDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Init debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlInitDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlInitDebugFlag = state;
    return PDL_OK;
}

/**
 * @fn  void pdlInitDone ()
 *
 * @brief   Called by application to indicate PDL init has been completed for all XML files
 *
 */

PDL_STATUS pdlInitDone (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /* mark init is completed and switch all debug flags to FALSE since they can be controlled using CLI commands */
    prvPdlInitDone = TRUE;
    prvPdlInitDebugFlag = FALSE;
    prvPdlBtnDebugFlag = FALSE;
    prvPdlPpDebugFlag = FALSE;
    prvPdlSfpDebugFlag = FALSE;
    prvPdlSerdesDebugFlag = FALSE;
    prvPdlOobPortDebugFlag = FALSE;
    prvPdlFanDebugFlag = FALSE;
    prvPdlSensorDebugFlag = FALSE;
    prvPdlPowerDebugFlag = FALSE;
    prvPdlPhyDebugFlag = FALSE;
    prvPdlLedDebugFlag = FALSE;
    prvPdlLibDebugFlag = FALSE;
    prvPdlCpuDebugFlag = FALSE;
    prvPdlFanControllerDebugFlag = FALSE;
    prvPdlInterfaceDebugFlag = FALSE;
    prvPdlI2CInterfaceMuxDebugFlag = FALSE;

    /* verify all external drivers are bounded correctly */
    pdlStatus = prvPdlSensorVerifyExternalDrivers();
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlLedVerifyExternalDrivers();
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlFanVerifyExternalDrivers();
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF pdlInitDone */

/**
 * @fn  PDL_STATUS pdlInit ( IN char * xmlFilePathPtr, IN char * xmlTagPrefixPtr, IN PDL_OS_CALLBACK_API_STC * callbacksPTR )
 *
 * @brief   initialize PDL
 *
 * @param [in]  xmlFilePathPtr  XML location.
 * @param [in]  xmlTagPrefixPtr XML tag's prefix.
 * @param [in]  callbacksPTR    application-specific implementation for os services.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlInit (
    IN char                       * xmlFilePathPtr,
    IN char                       * xmlTagPrefixPtr,
    IN PDL_OS_CALLBACK_API_STC    * callbacksPTR,
    IN PDL_OS_INIT_TYPE_ENT         initType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              rc;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlNodeId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    rc = pdlLibInit(callbacksPTR, initType);
    PDL_CHECK_STATUS(rc);

    xmlStatus = xmlParserBuild (xmlFilePathPtr, xmlTagPrefixPtr, TRUE, &pdlInitxmlRootId);
    XML_CHECK_STATUS(xmlStatus);

    xmlNodeId = XML_PARSER_ROOT2NODE_CONVERT_MAC(pdlInitxmlRootId);
   
    rc = prvPdlGpioInit(initType);
    PDL_CHECK_STATUS(rc);
    rc = prvPdlGpioMppXmlParser(xmlNodeId);
    PDL_CHECK_STATUS(rc);
    rc = prvPdlI2cInit();
    PDL_CHECK_STATUS(rc);
    rc = pdlSmiXsmiInit();
    PDL_CHECK_STATUS(rc);

    if (initType == PDL_OS_INIT_TYPE_STACK_E) {
        rc = PdlPhyInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);
        rc = PdlSerdesInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);
        rc = pdlPacketProcessorInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);
    }

    else {

        rc = PdlPhyInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = PdlSerdesInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = pdlBtnInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = pdlSensorInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = pdlPacketProcessorInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);
  
        if (pdlFanInternalDriverBind == FALSE) {
            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksTc654, PDL_FAN_CONTROLLER_TYPE_TC654_E);
            PDL_CHECK_STATUS(rc);

            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksAdt7476, PDL_FAN_CONTROLLER_TYPE_ADT7476_E);
            PDL_CHECK_STATUS(rc);

            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksEMC2305, PDL_FAN_CONTROLLER_TYPE_EMC2305_E);
            PDL_CHECK_STATUS(rc);

            rc = prvPdlFanBindInternalFanDriver(&pdlFanCallbacksPWM, PDL_FAN_CONTROLLER_TYPE_PWM_E);
            PDL_CHECK_STATUS(rc);
            pdlFanInternalDriverBind = TRUE;
        }

        rc = pdlFanInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = pdlPowerInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = pdlLedInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = prvPdlI2cBusFdInit();
        PDL_CHECK_STATUS(rc);

        rc = prvPdlCpuInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = prvPdlOobPortInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);

        rc = prvPdlLibBoardDescriptionInit(xmlNodeId);
        PDL_CHECK_STATUS(rc);
    }

    return PDL_OK;
}
/*$ END OF pdlInit */

/**
 * @fn  void pdlDestroy ()
 *
 * @brief   release all memory allocated by Pdl
 *
 */

PDL_STATUS pdlDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlParserDestroy (&pdlInitxmlRootId);
    prvPdlGpioDestroy();
    prvPdlI2cDestroy();
    prvPdlSmiXsmiDestroy();
    prvPdlPhyDestroy();
    prvPdlSerdesDestroy();
    prvPdlPacketProcessorDestroy();
    prvPdlBtnDestroy();
    prvPdlSensorDestroy();
    pdlFanInternalDriverBind = FALSE;
    prvPdlFanDestroy();
    prvPdlPowerDestroy();
    prvPdlLedDestroy();
    prvPdlOobDestroy();

    return PDL_OK;
}
/*$ END OF pdlDestroy */

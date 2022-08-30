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
* @file ezBringupTools.h
*
* @brief  hold common function (tools) for the board config files to use to get
*         info from the 'ez_bringup' xml
*
* @version   1
********************************************************************************
*/
#ifndef __common_ezBringupTools_H
#define __common_ezBringupTools_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#elif (defined PX_FAMILY)
#include <cpss/px/port/cpssPxPortMapping.h>
#endif
#define EZB_MAX_DEV_NUM 2
#define IMPL_DBG_TRACE

#ifdef IMPL_DBG_TRACE
    #ifdef  _WIN32
        #define DBG_TRACE(x) osPrintf x
    #else /*!_WIN32*/
        #define DBG_TRACE(x) osPrintSync x
    #endif /*!_WIN32*/
#else
    #define DBG_TRACE(x)
#endif

typedef struct {
    GT_U32      groupId;           /* the group Id (global to the 'board' ) */
    GT_U32      localIndexInGroup; /* the local index in the group */
}EZB_XML_FRONT_PANEL_PORT_INFO_STC;

typedef struct{
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT          speed;
}EZB_XML_SPEED_IF_MODE_STC;

typedef struct{
    EZB_XML_SPEED_IF_MODE_STC old;
    EZB_XML_SPEED_IF_MODE_STC new;
}EZB_XML_SPEED_IF_MODE_REPLACE_STC;

/* this entry must be last in array of APP_DEMO_EZB_XML_SPEED_IF_MODE_REPLACE_STC */
#define EZB_XML_SPEED_IF_MODE_REPLACE___MUAST_BE_LAST__CNS        \
{                                                                          \
    /*old*/{CPSS_PORT_INTERFACE_MODE_NA_E          ,CPSS_PORT_SPEED_NA_E}, \
    /*new*/{CPSS_PORT_INTERFACE_MODE_NA_E          ,CPSS_PORT_SPEED_NA_E}  \
}

#define EZB_DBG_TRACE_RC_MAC(_title, _rc)     \
  /* if(_rc != GT_OK || appDemoTraceOn_GT_OK)*/             \
      /* DBG_TRACE(("%s returned: 0x%X at file %s, line = %d\r\n", _title, _rc, __FILE__, __LINE__))*/

typedef enum {
/**
 *   @brief I2C that is connected to CPU
 */
    EZB_XML_INTERFACE_TYPE_I2C_E = 1,
/**
 *   @brief SMI that is connected to PP
 */
    EZB_XML_INTERFACE_TYPE_SMI_E,
/**
 *   @brief XSMI that is connected to PP
 */
    EZB_XML_INTERFACE_TYPE_XSMI_E,
/**
 *   @brief Memory mapped, used for: multi purpose pins, PCI/PEX ext...
 */
    EZB_XML_INTERFACE_TYPE_GPIO_E,
/**
 *   @brief SMI that is connected to CPU (OOB port for example)
 */
    EZB_XML_INTERFACE_TYPE_SMI_CPU_E,
    EZB_XML_INTERFACE_TYPE_EXTERNAL_E,
    EZB_XML_INTERFACE_TYPE_LAST_E,
}EZB_XML_INTERFACE_TYPE_ENT;

typedef struct {
    EZB_XML_INTERFACE_TYPE_ENT interfaceType;
    GT_U8                          devNum;
    GT_U32                         interfaceId;
    GT_U32                         address;
    GT_BOOL                        secondaryAddressSupported;
    GT_U32                         secondaryAddress;
}EZB_XML_INTERFACE_SMI_XSMI_STC;

/**
 * @struct  PDL_PHY_CONFIGURATION_STC
 *
 * @brief   defines phy configuration parameters
 */

typedef struct {
    /** @brief   The phy number */
    GT_U32                             phyNumber;
    /** @brief   The phy position */
    GT_U32                             phyPosition;

    /**
     * @struct  smiXmsiInterface
     *
     * @brief   A smi xmsi interface.
     */
    EZB_XML_INTERFACE_SMI_XSMI_STC    smiXmsiInterface;

    GT_U32           phyType;        /* one of PDL_PHY_TYPE_ENT */
    GT_U32           phyDownloadType;/* one of PDL_PHY_DOWNLOAD_TYPE_ENT */

    GT_BOOL adminStatus;
}EZB_XML_PHY_INFO_STC;
typedef struct EZB_XML_MAC_INFO_STCT EZB_XML_MAC_INFO_STC;

/**
 * @enum    PDL_TRANSCEIVER_TYPE_ENT
 *
 * @brief   Copper/Fiber/Combo
 */

typedef enum {
    EZB_GEN_TRANSCEIVER_TYPE_FIBER_E,          /* Fiber media */
    EZB_GEN_TRANSCEIVER_TYPE_COPPER_E,         /* Copper media */
    EZB_GEN_TRANSCEIVER_TYPE_COMBO_E,          /* Combo media */
    EZB_GEN_TRANSCEIVER_TYPE_LAST_E
} EZB_GEN_TRANSCEIVER_TYPE_ENT;

typedef struct{
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT          speed;
}EZB_GEN_SPEED_IF_MODE_STC;

typedef struct {
    GT_BOOL      holdSerdesMuxingInfo;/* is the serdes muxing info valid  */
    CPSS_PORT_MAC_TO_SERDES_STC  macToSerdesMap;/* the values in indexes 0..7 of the array are
                                                indexes of 'lanes' 0..7 'as order' of lanes */
    GT_U32       numOfSerdeses; /* number of SERDESes for the MAC        */
    GT_U32       firstSerdesId; /* the Id of the first SERDES of the MAC */

    EZB_XML_MAC_INFO_STC *firstMacPtr;
}EZB_XML_MAC_SERDES_INFO_STC;

/**
 * @enum    PDL_TRANSCEIVER_TYPE_ENT
 *
 * @brief   Copper/Fiber/Combo
 */

typedef enum {
    EZB_XML_TRANSCEIVER_TYPE_FIBER_E,          /* Fiber media */
    EZB_XML_TRANSCEIVER_TYPE_COPPER_E,         /* Copper media */
    EZB_XML_TRANSCEIVER_TYPE_COMBO_E,          /* Combo media */
    EZB_XML_TRANSCEIVER_TYPE_LAST_E
} EZB_XML_TRANSCEIVER_TYPE_ENT;

typedef struct EZB_XML_MAC_INFO_STCT{
    GT_U32      macNum;             /* The MAC number of the port . (pp-port-num in the XML if exists there) */
    GT_BOOL     isMacUsed;          /* indication that this MAC is in use or not */
                                    /* NOTE: if not used then all next info not relevant for it */

    /* all the next info is associated with pp-port-num in the XML          */

    GT_U32      physicalPort;       /* logical-port-number in the XML  */

    GT_U32      mpd_ifIndex;        /* needed for MPD that supports also phys that ware not on 'dev,port' but directly to CPU SMI.*/
                                    /* NOTE: this is a global number and not per device */
    EZB_XML_FRONT_PANEL_PORT_INFO_STC frontPanelInfo;/* front panel info. */
                                    /*NOTE: relevant only for 'non-cscade' port.  */


    EZB_XML_SPEED_IF_MODE_STC     defaultSpeedAndIfMode;
    GT_BOOL                       isPhyUsed;
    EZB_XML_PHY_INFO_STC          phyInfo;/* valid when isPhyUsed = GT_TRUE */
    GT_BOOL                       isSerdesInfoValid;
    EZB_XML_MAC_SERDES_INFO_STC   macSerdesInfo;/* valid when isSerdesInfoValid = GT_TRUE */
    GT_BOOL                       isCascadeValid;
    EZB_XML_TRANSCEIVER_TYPE_ENT  transceiverType;

}EZB_XML_MAC_INFO_STC;

/**
* @struct EZB_XML_SERDES_LANE_POLARITY_STC
 *
 * @brief Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*/
typedef struct{

    /** number of SERDES lane */
    GT_U32 laneNum;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertTx;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertRx;

} EZB_XML_SERDES_LANE_POLARITY_STC;

typedef struct {
    GT_BOOL                             polarityValid;
    EZB_XML_SERDES_LANE_POLARITY_STC    polarity;
}EZB_XML_SERDES_INFO_STC;

/**
 * @struct EZB_SERDES_LANE_POLARITY_STC
 *
 * @brief Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
 */
typedef struct{

    /** number of SERDES lane */
    GT_U32 laneNum;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertTx;

    /** @brief GT_TRUE
     *  GT_FALSE - no invert
     */
    GT_BOOL invertRx;
} EZB_SERDES_LANE_POLARITY_STC;

typedef struct {
    GT_BOOL                             polarityValid;
    EZB_SERDES_LANE_POLARITY_STC    polarity;
}EZB_GEN_SERDES_INFO_STC;

typedef struct EZB_GEN_MAC_INFO_STCT EZB_GEN_MAC_INFO_STC;

typedef struct {
    /** @brief   The phy number */
    GT_U32                             phyNumber;
    /** @brief   The phy position */
    GT_U32                             phyPosition;

    /**
     * @struct  smiXmsiInterface
     *
     * @brief   A smi xmsi interface.
     */
    EZB_XML_INTERFACE_SMI_XSMI_STC smiXmsiInterface;

    GT_U32           phyType;        /* one of PDL_PHY_TYPE_ENT */
    GT_U32           phyDownloadType;/* one of PDL_PHY_DOWNLOAD_TYPE_ENT */

    GT_BOOL adminStatus;
}EZB_GEN_PHY_INFO_STC;

typedef struct {
    GT_BOOL      holdSerdesMuxingInfo;/* is the serdes muxing info valid  */
    CPSS_PORT_MAC_TO_SERDES_STC  macToSerdesMap;/* the values in indexes 0..7 of the array are
                                                indexes of 'lanes' 0..7 'as order' of lanes */
    GT_U32       numOfSerdeses; /* number of SERDESes for the MAC        */
    GT_U32       firstSerdesId; /* the Id of the first SERDES of the MAC */

    EZB_GEN_MAC_INFO_STC *firstMacPtr;
}EZB_GEN_MAC_SERDES_INFO_STC;

typedef struct {
    GT_U32      groupId;           /* the group Id (global to the 'board' ) */
    GT_U32      localIndexInGroup; /* the local index in the group */
}EZB_GEN_FRONT_PANEL_PORT_INFO_STC;

typedef struct{
    GT_U32                      numOfMacPorts;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;/* allocated in run time as array of
                                                  'mac ports' according to numOfMacPorts
                                                  hold info to be used by the 'cpss init system'

NOTE: running 'ez_bringup' will get info from the XML
                                                */
    GT_U32                      numOfSerdeses;
    EZB_GEN_SERDES_INFO_STC *serdesGenInfoPtr;/* allocated in run time as array of
                                                      'serdeses' according to numOfSerdeses
                                                      hold info to be used by the 'cpss init system'

NOTE: running 'ez_bringup' will get info from the XML
                                                    */
    CPSS_PORTS_BMP_STC          existingPhyNumbers;/* bmp of PHY numbers that did init .
                                                      needed by MPD init of 'phyFw.isRepresentative' */
}EZB_PP_CONFIG;

extern EZB_PP_CONFIG ezbPpConfigList[EZB_MAX_DEV_NUM];

typedef struct EZB_GEN_MAC_INFO_STCT{
    GT_U32      macNum;             /* The MAC number of the port . (pp-port-num in the XML if exists there) */
    GT_BOOL     isMacUsed;          /* indication that this MAC is in use or not */
                                    /* NOTE: if not used then all next info not relevant for it */

    /* all the next info is associated with pp-port-num in the XML          */

    GT_U32      physicalPort;       /* logical-port-number in the XML  */

    GT_U32      mpd_ifIndex;        /* needed for MPD that supports also phys that ware not on 'dev,port' but directly to CPU SMI.*/
                                    /* NOTE: this is a global number and not per device */
    EZB_GEN_FRONT_PANEL_PORT_INFO_STC frontPanelInfo;/* front panel info. */
                                    /*NOTE: relevant only for 'non-cscade' port.  */


    EZB_GEN_SPEED_IF_MODE_STC  defaultSpeedAndIfMode;
    GT_BOOL                         isPhyUsed;
    EZB_GEN_PHY_INFO_STC       phyInfo;/* valid when isPhyUsed = GT_TRUE */
    GT_BOOL                         isSerdesInfoValid;
    EZB_GEN_MAC_SERDES_INFO_STC macSerdesInfo;/* valid when isSerdesInfoValid = GT_TRUE */
    GT_BOOL                         isCascadeValid;
    EZB_GEN_TRANSCEIVER_TYPE_ENT  transceiverType;

}EZB_GEN_MAC_INFO_STC;

#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)
/**
* @internal ezbIsXmlWithDevNum function
* @endinternal
*
* @brief  is XML of EZ_BRINGUP hold info about the device.
*
*
* @retval GT_TRUE / GT_FALSE
*/
GT_BOOL ezbIsXmlWithDevNum
(
    IN GT_U32  devNum
);

/**
* @internal ezbXmlLoad function
* @endinternal
*
* @brief  load XML for EZ_BRINGUP about the board.
*         if already loaded , the operation will destroy the previous one and then load the new one.
*
*
* @retval
*/
GT_STATUS ezbXmlLoad(
    IN char*       fileNamePtr
);


/**
* @internal ezbXmlName function
* @endinternal
*
* @brief  get the XML name use for the EZ_BRINGUP info loaded .
*
*
* @retval 'string' - the name of the XML or "no XML used"
*/
char* ezbXmlName(GT_VOID);

/**
* @internal ezbXmlDestroy function
* @endinternal
*
* @brief  unload XML for EZ_BRINGUP .
*
*
* @retval
*/
GT_STATUS ezbXmlDestroy(GT_VOID);


/**
* @internal ezbIsXmlLoaded function
* @endinternal
*
* @brief  is XML of EZ_BRINGUP info loaded to be used.
*
*
* @retval GT_TRUE / GT_FALSE
*/
GT_BOOL ezbIsXmlLoaded(GT_VOID);

/**
* @internal ezbMacAttributesGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific MAC of specific device
*
* @param[in] xmlDevNum             - The XML device number.
* @param[in] cpssDevNum            - The CPSS SW device number.
* @param[in] macNum                - The mac number of the port. (not physical port number)
*
* @param[out] xmlPortInfoPtr       - the generic port info extracted from the XML for the port.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum or the macPort not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS ezbMacAttributesGet
(
    IN GT_U32                     xmlDevNum,
    IN GT_U8                      cpssDevNum,
    IN GT_U32                     macNum,
    OUT EZB_XML_MAC_INFO_STC     *xmlPortInfoPtr
);

/**
* @internal ezbSerdesAttributesGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific serdes of specific device
*
* @param[in] xmlDevNum             - The XML device number.
* @param[in] cpssDevNum            - The CPSS SW device number.
*
* @param[out] xmlSerdesInfoPtr     - the generic serdes info extracted from the XML for the port.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum or the macPort not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS ezbSerdesAttributesGet
(
    IN GT_U32       xmlDevNum,
    IN GT_U32       serdesNum,
    OUT EZB_XML_SERDES_INFO_STC   *xmlSerdesInfoPtr
);

#else   /*!EZ_BRINGUP*/

/* implement MACROS STUBs for compilation */
#define ezbXmlName()                        "no XML used"
#define ezbIsXmlWithDevNum(a)               GT_FALSE
#define ezbXmlLoad(a)                       GT_NOT_IMPLEMENTED
#define ezbXmlDestroy()                     GT_NOT_IMPLEMENTED
#define ezbIsXmlLoaded()                    GT_FALSE
#define ezbMacAttributesGet(a,b,c,d)        GT_NOT_IMPLEMENTED; (*d)=0;
#define ezbSerdesAttributesGet(a,b,c)       GT_NOT_IMPLEMENTED; (*c)=0;
#endif /*!EZ_BRINGUP*/

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /*__common_ezBringupTools_H*/




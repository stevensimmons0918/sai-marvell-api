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
* @file mpdTools.h
*
* @brief  hold common function (tools) for the board config files to use to get
*   MPD functions to use. NOTE: EZ_BRINGUP also exists.
*
* @version   1
********************************************************************************
*/
#ifndef __common_mpdTools_H
#define __common_mpdTools_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if (defined INCLUDE_MPD) && (defined CHX_FAMILY)

typedef enum {
    EZB_MPD_PORT_TP_NA,
    EZB_MPD_PORT_TP_MDI,
    EZB_MPD_PORT_TP_MDIX,
    EZB_MPD_PORT_TP_AUTO
}EZB_MPD_PORT_TP_ENT;


GT_U32 mpdIsIfIndexForDevPortExists
(
    IN GT_U32 devNum,
    IN GT_U32 macNum,
    OUT GT_U32 *ifIndexPtr
);

/**
* @internal prvEzbMpdLoad function
* @endinternal
*
* @brief  load MPD for EZ_BRINGUP about the board.
*
*
* @retval
*/
GT_STATUS prvEzbMpdLoad(
    void
);

/**
* @internal prvEzbMpdGetNextFreeGlobalIfIndex function
* @endinternal
*
* @brief  allocate EZ_BRINGUP MPD ifIndex to : {devNum,macNum}
*         if already exists , return the existing one
* @param[in] devNum            - the cpss SW devNum
* @param[in] macNum            - the mac number
*
* @retval
* @retval get new MPD ifIndex to the port
*/
GT_U32 prvEzbMpdGetNextFreeGlobalIfIndex(
    IN GT_U8  devNum,
    IN GT_U32 macNum
);


/**
* @internal prvEzbMpdInitHw function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           call this one only after prvAppDemoEzbMpdPortInit(...) called  an all ports in all devices
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS prvEzbMpdInitHw
(
    void
);

/**
* @internal prvEzbMpdPortInit function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           after done an all ports in all devices need to call to : ezbMpdInitHw()
*
* @param[in] macNum               - the mac number
* @param[in] ezbDbIndex           - ezBringup PP Config DB index
*
* @param[out] isInitDonePtr       - (pointer to) is the port hold phy and did init
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvEzbMpdPortInit
(
    IN GT_U32                   macPort,
	IN GT_U32                   ezbDbIndex,
    OUT GT_BOOL                *isInitDonePtr
);
/**
* @internal ezbMpdIsPortWithIfIndex function
* @endinternal
*
* @brief  check EZ_BRINGUP {devNum,macNum} : hold MPD ifIndex
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[in] ifIndexPtr           - (pointer to) the ifIndex of MPD for a port
*
* @retval
* @retval GT_TRUE- have ifIndex , GT_FALSE - no ifindex
*/
GT_BOOL ezbMpdIsPortWithIfIndex
(
    IN GT_U32 devNum,
    IN GT_U32 macNum
);

/**
* @internal ezbMpdIfIndexToDevMacConvert function
* @endinternal
*
* @brief  convert EZ_BRINGUP MPD ifIndex to : {devNum,macNum}
*
* @param[in] ifIndex                - the ifIndex of MPD for a port
*
* @param[out] devNumPtr            - (pointer to)the cpss SW devNum
* @param[out] macNumPtr            - (pointer to)the mac number
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - the ifIndex is out of range.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS ezbMpdIfIndexToDevMacConvert
(
    IN  GT_U32  ifIndex,
    OUT GT_U8  *devNumPtr,
    OUT GT_U32 *macNumPtr
);

/**
* @internal ezbMpdDevMacToIfIndexConvert function
* @endinternal
*
* @brief  convert EZ_BRINGUP {devNum,macNum} : MPD ifIndex
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[in] ifIndexPtr           - (pointer to) the ifIndex of MPD for a port
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_FOUND             - the {devNum,macNum} not found to have ifIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS ezbMpdDevMacToIfIndexConvert
(
    IN GT_U32 devNum,
    IN GT_U32 macNum,
    OUT GT_U32 *ifIndexPtr
);

/**
 * @internal ezbMpdDevMacToIfIndexAdd function
 * @endinternal
 *
 * @brief  Add MAC and dev no details to specific ifIndex : {devNum,macNum}
 *
 * @param[in] ifIndex                - the ifIndex of MPD for a port
 * @param[in] devNum                 - the cpss SW devNum
 * @param[in] macNum                 - the mac number
 *
 * @retval
 * @retval GT_OK                    - on success,
 * @retval GT_BAD_PARAM             - the ifIndex is out of range.
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 */
GT_STATUS ezbMpdDevMacToIfIndexAdd
(
    IN  GT_U32  ifIndex,
    IN  GT_U8   devNum,
    IN  GT_U32  macNum
);

/**
* @internal ezbMpdPortAdminStatusSet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : Admin status up
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[in] adminUp              - admin link up
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortAdminStatusSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    IN GT_BOOL                  adminUp
);

/**
* @internal ezbMpdPortAutoNegResolvedCheck function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : check that AN resolved
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] speedPtr            - (pointer to) what speed  PHY AN selected ?
* @param[out] isFullDuplexPtr     - (pointer to) what duplex PHY AN selected ?
* @param[out] adminUpPtr          - (pointer to) is admin force link up ?
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
* @retval GT_NOT_READY             - the port not finished AN !
*/
GT_STATUS ezbMpdPortAutoNegResolvedCheck
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT CPSS_PORT_SPEED_ENT     *speedPtr,
    OUT GT_BOOL                 *isFullDuplexPtr,
    OUT GT_BOOL                 *adminUpPtr
);

/**
* @internal ezbMpdPortDuplexSet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : set full/half duplex
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
* @param[in] isFullDuplex         - full/half duplex
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortDuplexSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    IN GT_BOOL                  isFullDuplex
);

/**
* @internal ezbMpdPortSpeedSet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : set speed
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
* @param[in] speed                - the speed
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortSpeedSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    IN CPSS_PORT_SPEED_ENT      speed
);

/**
* @internal ezbMpdPortAutoNegSet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : autoneg enable and capabilities set
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
* @param[in] anEnable             - autoneg enable
* @param[in] capabilities         - capabilities (see MPD_AUTONEG_CAPABILITIES_TYP)
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortAutoNegSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    IN GT_BOOL                  anEnable,
    IN GT_U32                   capabilities
);

/**
* @internal ezbMpdPortMdixSet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : set MDIX admin mode
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[in] adminMode            - MDIX mode
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortMdixSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT EZB_MPD_PORT_TP_ENT     adminMode
);

/**
* @internal ezbMpdPortAutoNegRestart function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : autoneg restart
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortAutoNegRestart
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort
);

/**
* @internal ezbMpdPortCapabilitiesGet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : get capabilities
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] capabilitiesPtr     - (pointer to) capabilities (see MPD_AUTONEG_CAPABILITIES_TYP)
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortCapabilitiesGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_U32                 *capabilitiesPtr
);

/**
* @internal ezbMpdPortRemoteCapabilitiesGet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : get remote capabilities
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] capabilitiesPtr     - (pointer to) capabilities (see MPD_AUTONEG_CAPABILITIES_TYP)
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortRemoteCapabilitiesGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_U32                   *capabilitiesPtr
);

/**
* @internal ezbMpdPortMdixGet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : get mdix mode (oper and admin)
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] operStatusPtr       - (pointer to) oper status (mode)
* @param[out] adminModePtr        - (pointer to) admin mode
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortMdixGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT EZB_MPD_PORT_TP_ENT     *operStatusPtr,
    OUT EZB_MPD_PORT_TP_ENT     *adminModePtr
);

/**
* @internal ezbMpdPortLinkPartnerFcInfoGet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : get link partner FC info
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] linkPartnerPause_CapablePtr               - (pointer to) is capable
* @param[out] linkPartnerPause_asymetricRequestedPtr    - (pointer to) is Asymmetric Requested
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortLinkPartnerFcInfoGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_BOOL                 *linkPartnerPause_CapablePtr,
    OUT GT_BOOL                 *linkPartnerPause_asymetricRequestedPtr
);

/**
* @internal ezbSwitchMacAutoNegCompleted function
* @endinternal
*
* @brief  notify that got event of 'CPSS_PP_PORT_AN_COMPLETED_E' from the port in the PP
*       and need to synch the PP with the MPD info (with the PHY)
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] portNum              - the physical port number
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS ezbSwitchMacAutoNegCompleted(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
  * @internal ezbMpdPortLinkGet function
  * @endinternal
  *
  * @brief   EZ_BRINGUP do MPD operation : get link status
  *
  * @param[in] devNum               - the cpss SW devNum
  * @param[in] macNum               - the mac number
  *
  * @param[out] linkStatusPtr                                - (pointer to) link status of the port
  *
  * @retval
  * @retval GT_OK                    - on success,
  * @retval GT_FAIL                  - on fail,
  * @retval GT_NOT_FOUND             - the port not valid for phy operations
  */
GT_STATUS ezbMpdPortLinkGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            macNum,
    OUT GT_BOOL                         *linkStatusPtr
);

/**
* @internal ezbMpdPortLinkStatusGet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : get link status
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
GT_STATUS ezbMpdPortLinkStatusGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            macNum
);

#else /*!INCLUDE_MPD*/
/* implement MACROS STUBs for compilation */
#define mpdIsIfIndexForDevPortExists(a,b,c)     GT_NOT_IMPLEMENTED; (*c)=0
#define prvEzbMpdLoad()                         GT_NOT_IMPLEMENTED
#define prvEzbMpdGetNextFreeGlobalIfIndex(a,b)  GT_NOT_IMPLEMENTED
#define prvEzbMpdInitHw()                       GT_NOT_IMPLEMENTED
#define prvEzbMpdPortInit(a,b,c,d)              GT_NOT_IMPLEMENTED; (*d)=0;
#define ezbMpdIfIndexToDevMacConvert(a,b,c)     GT_NOT_IMPLEMENTED; (*b)=(*c)=0;
#define ezbMpdDevMacToIfIndexConvert(a,b,c)     GT_NOT_IMPLEMENTED; (*c)=0;
#define ezbMpdDevMacToIfIndexAdd(a,b,c)         GT_NOT_IMPLEMENTED
#define ezbMpdPortAdminStatusSet(a,b,c)         GT_NOT_IMPLEMENTED
#define ezbMpdPortAutoNegResolvedCheck(a,b,c,d,e) GT_NOT_IMPLEMENTED; (*c)=(*d)=(*e)=0;
#define ezbMpdPortDuplexSet(a,b,c)              GT_NOT_IMPLEMENTED
#define ezbMpdPortSpeedSet(a,b,c)               GT_NOT_IMPLEMENTED
#define ezbMpdPortAutoNegSet(a,b,c,d)           GT_NOT_IMPLEMENTED
#define ezbMpdPortMdixSet(a,b,c)                GT_NOT_IMPLEMENTED
#define ezbMpdPortAutoNegRestart(a,b)           GT_NOT_IMPLEMENTED
#define ezbMpdPortCapabilitiesGet(a,b,c)        GT_NOT_IMPLEMENTED; (*c)=0
#define ezbMpdPortRemoteCapabilitiesGet(a,b,c)  GT_NOT_IMPLEMENTED; (*c)=0
#define ezbMpdPortMdixGet(a,b,c,d)              GT_NOT_IMPLEMENTED; (*c)=(*d)=0;
#define ezbMpdPortLinkPartnerFcInfoGet(a,b,c,d) GT_NOT_IMPLEMENTED; (*c)=(*d)=0;
#define ezbSwitchMacAutoNegCompleted(a,b)       GT_NOT_IMPLEMENTED
#define ezbMpdPortLinkGet(a,b,c)                GT_NOT_IMPLEMENTED; (*c)=0
#endif /*!INCLUDE_MPD*/

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /*__common_mpdTools_H*/




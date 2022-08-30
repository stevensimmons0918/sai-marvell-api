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
* @file ezbBoardConfig_mpdTools.c
*
* @brief  hold common function (tools) for the board config files to use to get
*   MPD functions to use. NOTE: EZ_BRINGUP also exists.
*
*   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) and 'INCLUDE_MPD' is enabled.
*   meaning that 'stub' (if needed) are implemented elsewhere.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <ezBringupTools.h>
#include <mpdTools.h>
#include <pdl/init/pdlInit.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/sfp/pdlSfp.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



#undef MIN
#include <mpdPrv.h>
#define CHECK_PORT_WITH_PHY(/*in*/_devNum,/*in*/_macPort,/*out*/_mpd_ifIndex)   \
{                                                                               \
    EZB_PP_CONFIG        *_ezbPpConfigPtr;                             \
    GT_U32                     _ezbDbIndex = 0;                             \
    /*ezbCpssDevNumToAppDemoIndexConvert(_devNum,&_ezbDbIndex); */       \
    _ezbPpConfigPtr = &ezbPpConfigList[_ezbDbIndex];                \
                                                                                \
    if(_ezbPpConfigPtr->macGenInfoPtr[_macPort].isMacUsed == GT_FALSE ||    \
       _ezbPpConfigPtr->macGenInfoPtr[_macPort].isPhyUsed == GT_FALSE)      \
    {                                                                           \
        /* port hold no PHY */                                                  \
        return GT_NOT_FOUND;                                                    \
    }                                                                           \
    _mpd_ifIndex = _ezbPpConfigPtr->macGenInfoPtr[_macPort].mpd_ifIndex;    \
}

#define MAX_MPD_PORTS_CNS   128

static struct{
    GT_U8       devNum;
    GT_U32      macNum;
}ifIndexInfoArr[MAX_MPD_PORTS_CNS];

/* MPD global numbers for ifIndex */
static GT_U32 nextFreeGlobalIfIndex = 1;

static GT_STATUS mpdConvertIfIndexToDevPort(
    IN  GT_U32  ifIndex,
    OUT GT_U8  *devNumPtr,
    OUT GT_U32 *macNumPtr
)
{
    if(ifIndex >= nextFreeGlobalIfIndex)
    {
        return GT_BAD_PARAM;
    }

    *devNumPtr = ifIndexInfoArr[ifIndex].devNum;
    *macNumPtr = ifIndexInfoArr[ifIndex].macNum;
    return GT_OK;
}

GT_U32 mpdIsIfIndexForDevPortExists(
    IN GT_U32 devNum,
    IN GT_U32 macNum,
    OUT GT_U32 *ifIndexPtr
)
{
    GT_U32 ifIndex;

    for(ifIndex = 0;ifIndex < nextFreeGlobalIfIndex;ifIndex++)
    {
        if(ifIndexInfoArr[ifIndex].devNum == devNum &&
           ifIndexInfoArr[ifIndex].macNum == macNum)
        {
            *ifIndexPtr = ifIndex;
            return 1; /* found */
        }
    }

    return 0;/*not found*/
}

static GT_STATUS mpdConvertIfIndexToDevPortAdd(
    IN  GT_U8   devNum,
    IN  GT_U32  macNum,
    IN  GT_U32  ifIndex
)
{
    ifIndexInfoArr[ifIndex].devNum = devNum;
    ifIndexInfoArr[ifIndex].macNum = macNum;
    return GT_OK;
}
/**
* @internal prvezbMpdGetNextFreeGlobalIfIndex function
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
)
{
    GT_U32  ifIndex;
    if(mpdIsIfIndexForDevPortExists(devNum,macNum,&ifIndex))
    {
        return ifIndex;
    }
    /* we return nextFreeGlobalIfIndex , and only then the '++' happen !!! */
    return nextFreeGlobalIfIndex++;
}

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
)
{
    GT_U32  ifIndex;

    return mpdIsIfIndexForDevPortExists(devNum,macNum,&ifIndex);
}
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
)
{
    CPSS_NULL_PTR_CHECK_MAC(devNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(macNumPtr);

    return mpdConvertIfIndexToDevPort(ifIndex,devNumPtr,macNumPtr);
}

/**
* @internal ezbMpdDevMacToIfIndexAdd function
* @endinternal
*
* @brief  Add MAC and dev no details to specific ifIndex : {devNum,macNum}
*
* @param[in] ifIndex                - the ifIndex of MPD for a port
*
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
)
{
    GT_STATUS rc = GT_OK;

    if(mpdIsIfIndexForDevPortExists(devNum, macNum, &ifIndex) == 0)
    {
        rc = mpdConvertIfIndexToDevPortAdd(devNum, macNum, ifIndex);
    }

    return  rc;
}

/**
* @internal ezbMpdDevMacToIfIndexConvert function
* @endinternal
*
* @brief  convert EZ_BRINGUP {devNum,macNum} : MPD ifIndex
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] ifIndexPtr           - (pointer to) the ifIndex of MPD for a port
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
)
{
    CPSS_NULL_PTR_CHECK_MAC(ifIndexPtr);

    if(mpdIsIfIndexForDevPortExists(devNum,macNum,ifIndexPtr))
    {
        return GT_OK;
    }

    return GT_NOT_FOUND;
}

/**
* @internal prvEzbMpdPortInit function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           after done an all ports in all devices need to call to : prvezbMpdInitHw()
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
    GT_U32                      ezbDbIndex,
    OUT GT_BOOL                *isInitDonePtr
)
{
    MPD_PORT_INIT_DB_STC        phy_entry;
    EZB_GEN_PHY_INFO_STC       *phyInfoPtr;
    GT_U32                      mpd_ifIndex;
    EZB_PP_CONFIG              *ezbPpConfigPtr;

    CPSS_NULL_PTR_CHECK_MAC(isInitDonePtr);
    *isInitDonePtr = GT_FALSE;

    ezbPpConfigPtr = &ezbPpConfigList[ezbDbIndex];

    if(ezbPpConfigPtr->macGenInfoPtr[macPort].isMacUsed == GT_FALSE ||
       ezbPpConfigPtr->macGenInfoPtr[macPort].isPhyUsed == GT_FALSE)
    {
        /* nothing to do for this port */
        return GT_OK;
    }

    phyInfoPtr = &ezbPpConfigPtr->macGenInfoPtr[macPort].phyInfo;

    mpd_ifIndex = ezbPpConfigPtr->macGenInfoPtr[macPort].mpd_ifIndex;

    cpssOsMemSet(&phy_entry, 0, sizeof(phy_entry));
    phy_entry.port = (UINT_8) macPort;

    switch (phyInfoPtr->phyType)
    {
        case PDL_PHY_TYPE_alaska_88E1543_E:
            /* TBD -
            phy_entry.mdioInfo.smi.internalSmiAddress = secondary_phy_addr; */
            break;
        case PDL_PHY_TYPE_alaska_88E1548_E:
        case PDL_PHY_TYPE_alaska_88E1545_E:
        case PDL_PHY_TYPE_alaska_88E1680_E:
        case PDL_PHY_TYPE_alaska_88E1680L_E:
        case PDL_PHY_TYPE_alaska_88E3680_E:
            phy_entry.mdioInfo.mdioBus = (UINT_8)phyInfoPtr->smiXmsiInterface.interfaceId;
            phy_entry.mdioInfo.mdioDev       = phyInfoPtr->smiXmsiInterface.devNum;
            phy_entry.mdioInfo.mdioAddress   = phyInfoPtr->smiXmsiInterface.address;
            break;
        case PDL_PHY_TYPE_alaska_88E32x0_E:
        case PDL_PHY_TYPE_alaska_88E33X0_E:
        case PDL_PHY_TYPE_alaska_88E20X0_E:
        case PDL_PHY_TYPE_alaska_88E2180_E:
        case PDL_PHY_TYPE_alaska_88X3540_E:
            phy_entry.mdioInfo.mdioBus   = (UINT_8)phyInfoPtr->smiXmsiInterface.interfaceId;
            phy_entry.mdioInfo.mdioDev         = phyInfoPtr->smiXmsiInterface.devNum;
            phy_entry.mdioInfo.mdioAddress     = phyInfoPtr->smiXmsiInterface.address;
            break;

        default:
            break;
    }

    phy_entry.phyType           = phyInfoPtr->phyType;
    phy_entry.transceiverType   = ezbPpConfigPtr->macGenInfoPtr[macPort].transceiverType;
    phy_entry.phySlice          = (UINT_8)phyInfoPtr->phyPosition;
    /*phy_entry.phyTxSerdesParams -> TBD update from PDL*/
    /*phy_entry.shortReachSerdes -> TBD update from PDL */
    /*phy_entry.abcdSwapRequired -> TBD update from PDL */
    /*phy_entry.doSwapAbcd = TBD -> TBD update from PDL */

    if (mpdPortDbUpdate(mpd_ifIndex, &phy_entry) != MPD_OK_E)
    {
        /*CPSS_ENABLER_DBG_TRACE_RC_MAC("mpdPortDbUpdate", GT_FAIL);*/
        return GT_FAIL;
    }

    *isInitDonePtr = GT_TRUE;

    return GT_OK;
}

static MPD_MDIX_MODE_TYPE_ENT swDevMdixToMpd(IN EZB_MPD_PORT_TP_ENT mode)
{
    switch(mode)
    {
        case EZB_MPD_PORT_TP_MDI:
            return MPD_MDI_MODE_MEDIA_E;
        case EZB_MPD_PORT_TP_MDIX:
            return MPD_MDIX_MODE_MEDIA_E;
        case EZB_MPD_PORT_TP_AUTO:
        default:
            return MPD_AUTO_MODE_MEDIA_E;
    }
}

static EZB_MPD_PORT_TP_ENT mpdMdixToSwDev(MPD_MDIX_MODE_TYPE_ENT mode)
{
    switch(mode)
    {
        case MPD_MDI_MODE_MEDIA_E:
            return EZB_MPD_PORT_TP_MDI;
        case MPD_MDIX_MODE_MEDIA_E:
            return EZB_MPD_PORT_TP_MDIX;
        case MPD_AUTO_MODE_MEDIA_E:
            return EZB_MPD_PORT_TP_AUTO;
        default:
            return EZB_MPD_PORT_TP_NA;
    }
}
static MPD_SPEED_ENT swDevCpssSpeedToMpd
(
    IN CPSS_PORT_SPEED_ENT speed
)
{
    switch (speed)
    {
    case CPSS_PORT_SPEED_10_E:
        return MPD_SPEED_10M_E;
    case CPSS_PORT_SPEED_100_E:
        return MPD_SPEED_100M_E;
    case CPSS_PORT_SPEED_1000_E:
        return MPD_SPEED_1000M_E;
    case CPSS_PORT_SPEED_10000_E:
        return MPD_SPEED_10000M_E;
    case CPSS_PORT_SPEED_2500_E:
        return MPD_SPEED_2500M_E;
    case CPSS_PORT_SPEED_5000_E:
        return MPD_SPEED_5000M_E;
    case CPSS_PORT_SPEED_20000_E:
        return MPD_SPEED_20000M_E;
    default:
        return MPD_SPEED_LAST_E;
    }
}
static CPSS_PORT_SPEED_ENT swDevMpdSpeedToCpss
(
    IN MPD_SPEED_ENT speed
)
{
    switch (speed)
    {
    case MPD_SPEED_10M_E:
        return CPSS_PORT_SPEED_10_E;
    case MPD_SPEED_100M_E:
        return CPSS_PORT_SPEED_100_E;
    case MPD_SPEED_1000M_E:
        return CPSS_PORT_SPEED_1000_E;
    case MPD_SPEED_10000M_E:
        return CPSS_PORT_SPEED_10000_E;
    case MPD_SPEED_2500M_E:
        return CPSS_PORT_SPEED_2500_E;
    case MPD_SPEED_5000M_E:
        return CPSS_PORT_SPEED_5000_E;
    case MPD_SPEED_20000M_E:
        return CPSS_PORT_SPEED_20000_E;
    default:
        return CPSS_PORT_SPEED_NA_E;
    }
}
/*
    define operations needed by MPD
*/
typedef struct{
    /*MPD_OP_CODE_SET_PHY_DISABLE_OPER_E - set*/
    /* 'from DB'                         - get */
    GT_BOOL                  do_adminLinkUp;
    GT_BOOL                  adminLinkUp;

    /*MPD_OP_CODE_SET_SPEED_E - set */
    /*MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E - get */
    GT_BOOL                  do_speed;
    CPSS_PORT_SPEED_ENT      speed;

    /*MPD_OP_CODE_SET_DUPLEX_MODE_E - set */
    /*MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E - get */
    GT_BOOL                  do_duplex;
    GT_BOOL                  isFullDuplex;

    /*MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E*/
    GT_BOOL                  do_anCompleted;
    GT_BOOL                  anCompleted;   /* only for 'get' */

    /*MPD_OP_CODE_SET_AUTONEG_E - set */
    /*MPD_OP_CODE_GET_AUTONEG_ADMIN_E - get*/
    GT_BOOL                  do_anEnable;
    GT_BOOL                  anEnable;
    GT_U32                   anCapabilities;

    /*MPD_OP_CODE_GET_AUTONEG_SUPPORT_E - get*/
    GT_BOOL                  do_anCapabilitiesGet;   /* only for 'get' */
    GT_U32                   anCapabilitiesGet;      /* only for 'get' */

    /*MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E - get */
    GT_BOOL                  do_anRemoteCapabilities; /* only for 'get' */
    GT_U32                   anRemoteCapabilities;    /* only for 'get' */

    /*MPD_OP_CODE_SET_MDIX_E - set */
    /*MPD_OP_CODE_GET_MDIX_E - get */
    GT_BOOL                  do_mdixMode;
    EZB_MPD_PORT_TP_ENT      mdixMode;

    /*MPD_OP_CODE_GET_MDIX_ADMIN_E - get */
    GT_BOOL                  do_mdixAdminMode;/* only for 'get' */
    EZB_MPD_PORT_TP_ENT      mdixAdminMode;   /* only for 'get' */

    /*MPD_OP_CODE_SET_RESTART_AUTONEG_E - set */
    GT_BOOL                  do_restart;      /* only for 'set' */

    /*MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E - get */
    GT_BOOL                  do_linkPartnerPause;                /* only for 'get' */
    GT_BOOL                  linkPartnerPause_Capable;           /* only for 'get' */
    GT_BOOL                  linkPartnerPause_asymetricRequested;/* only for 'get' */

    /*MPD_OP_CODE_GET_SPEED_EXT_E - get(only for link status)*/
    GT_BOOL                  do_linkStatusGet;
    GT_BOOL                  linkStatus;

}SIMPLE_MPD_OPER_STC;


#define  MPD_OPER_MAC(_mpd_ifIndex,_oper,_opereInfoPtr)       \
    {                                                         \
        status = mpdPerformPhyOperation(_mpd_ifIndex, _oper,  \
                                           (_opereInfoPtr));  \
        if (MPD_OK_E != status)                               \
        {                                                     \
            cpssOsPrintf("Error : devNum[%d]macPort[%d]mpd_ifIndex[%d] : failed on [%s] status[%d] \n", \
                devNum,macPort,_mpd_ifIndex,                  \
                #_oper,                                       \
                status);                                      \
            return GT_FAIL;                                   \
        }                                                     \
        didOper++;                                            \
    }



/**
* @internal doMpdOper_simpleSet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : simple struct as input for 'MPD PHY set' operations
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[in] operPtr              - type of 'set' operations that needed.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
static GT_STATUS doMpdOper_simpleSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    IN SIMPLE_MPD_OPER_STC      *operPtr
)
{
    MPD_RESULT_ENT            status;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_U32                    mpd_ifIndex;
    GT_U32                    didOper = 0;

    CHECK_PORT_WITH_PHY(/*in*/devNum,/*in*/macPort,/*out*/mpd_ifIndex);

    cpssOsMemSet(&opParams,0,sizeof(opParams));

    if(operPtr->do_adminLinkUp)
    {
        /* force link down ? */
        opParams.phyDisable.forceLinkDown = (operPtr->adminLinkUp == GT_TRUE) ? 0 : 1;
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                                        &opParams);

        opParams.phyDisable.forceLinkDown = 0;
    }

    if(operPtr->do_speed)
    {
        opParams.phySpeed.speed     = swDevCpssSpeedToMpd(operPtr->speed);
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_SET_SPEED_E,
                                           &opParams);

        opParams.phySpeed.speed     = 0;
    }

    if(operPtr->do_duplex)
    {
        opParams.phyDuplex.mode = operPtr->isFullDuplex == GT_TRUE ?
            MPD_DUPLEX_ADMIN_MODE_FULL_E :
            MPD_DUPLEX_ADMIN_MODE_HALF_E;

        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_SET_DUPLEX_MODE_E,
                                           &opParams);

        opParams.phyDuplex.mode     = 0;
    }

    if(operPtr->do_anEnable)
    {
        opParams.phyAutoneg.enable       = operPtr->anEnable == GT_TRUE ?
            MPD_AUTO_NEGOTIATION_ENABLE_E :
            MPD_AUTO_NEGOTIATION_DISABLE_E;
        opParams.phyAutoneg.capabilities = operPtr->anCapabilities;
        opParams.phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_UNKNOWN_E;


        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_SET_AUTONEG_E,
                                           &opParams);

        opParams.phyAutoneg.enable       = 0;
        opParams.phyAutoneg.capabilities = 0;
        opParams.phyAutoneg.masterSlave  = 0;
    }

    if(operPtr->do_mdixMode)
    {
        opParams.phyMdix.mode = swDevMdixToMpd(operPtr->mdixMode);

        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_SET_MDIX_E,
                                           &opParams);

        opParams.phyMdix.mode = 0;
    }

    if(operPtr->do_restart)/* no extra params */
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_SET_RESTART_AUTONEG_E,
                                           &opParams);
    }

    if(didOper == 0)
    {
        cpssOsPrintf("Error : devNum[%d]macPort[%d]mpd_ifIndex[%d] : no paramter was set \n",
            devNum,macPort,mpd_ifIndex);
        return GT_BAD_PARAM;
    }

    return GT_OK;
}


/**
* @internal doMpdOper_simpleGet function
* @endinternal
*
* @brief   EZ_BRINGUP do MPD operation : simple struct as input for 'MPD PHY set' operations
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
* @param[in] operPtr              - type of 'get' operations that needed.
*
* @param[out] operPtr             - hold the 'get' values .
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_NOT_FOUND             - the port not valid for phy operations
*/
static GT_STATUS doMpdOper_simpleGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    INOUT SIMPLE_MPD_OPER_STC     *operPtr
)
{
    MPD_RESULT_ENT            status;
    MPD_OPERATIONS_PARAMS_UNT opParams;
    GT_U32                    mpd_ifIndex;
    GT_U32                    didOper = 0;

    CHECK_PORT_WITH_PHY(/*in*/devNum,/*in*/macPort,/*out*/mpd_ifIndex);

    cpssOsMemSet(&opParams,0,sizeof(opParams));

    if(operPtr->do_adminLinkUp)
    {
        GT_U32  ezbDbIndex = devNum;
        /*ezbCpssDevNumToAppDemoIndexConvert(devNum,&ezbDbIndex);*/
        /* get from DB */
        operPtr->adminLinkUp = ezbPpConfigList[ezbDbIndex].macGenInfoPtr[macPort].phyInfo.adminStatus;
        didOper++;
    }

    if(operPtr->do_speed || operPtr->do_duplex || operPtr->do_anCompleted)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                                           &opParams);

        operPtr->anCompleted  = opParams.phyInternalOperStatus.isAnCompleted ? GT_TRUE : GT_FALSE;
        operPtr->speed        = swDevMpdSpeedToCpss(opParams.phyInternalOperStatus.phySpeed);
        operPtr->isFullDuplex = opParams.phyInternalOperStatus.duplexMode ? GT_TRUE : GT_FALSE;
    }

    if(operPtr->do_anEnable)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
                                           &opParams);

        operPtr->anEnable       = opParams.phyAutoneg.enable;
        operPtr->anCapabilities = opParams.phyAutoneg.capabilities;
    }


    if(operPtr->do_anCapabilitiesGet)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
                                           &opParams);

        operPtr->anCapabilitiesGet = opParams.PhyAutonegCapabilities.capabilities;
    }


    if(operPtr->do_anRemoteCapabilities)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
                                           &opParams);

        operPtr->anRemoteCapabilities = opParams.phyRemoteAutoneg.capabilities;
    }

    if(operPtr->do_mdixMode)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_MDIX_E,
                                           &opParams);

        operPtr->mdixMode = mpdMdixToSwDev(opParams.phyMdix.mode);
    }

    if(operPtr->do_mdixAdminMode)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_MDIX_ADMIN_E,
                                           &opParams);

        operPtr->mdixAdminMode = mpdMdixToSwDev(opParams.phyMdix.mode);
    }

    if(operPtr->do_linkPartnerPause)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
                                           &opParams);

        operPtr->linkPartnerPause_Capable            = opParams.phyLinkPartnerFc.pauseCapable;
        operPtr->linkPartnerPause_asymetricRequested = opParams.phyLinkPartnerFc.asymetricRequested;
    }

    if(operPtr->do_linkStatusGet)
    {
        MPD_OPER_MAC(mpd_ifIndex, MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
                                            &opParams);
        operPtr->linkStatus                         = opParams.phyInternalOperStatus.isOperStatusUp;
    }

    if(didOper == 0)
    {
        cpssOsPrintf("Error : devNum[%d]macPort[%d]mpd_ifIndex[%d] : no paramter was set \n",
            devNum,macPort,mpd_ifIndex);
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

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
)
{
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_adminLinkUp = GT_TRUE;
    mpdOper.adminLinkUp    = adminUp;

    return doMpdOper_simpleSet(devNum,macPort,&mpdOper);
}

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
)
{
    GT_STATUS               rc;
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_speed       = GT_TRUE;
    mpdOper.do_duplex      = GT_TRUE;
    mpdOper.do_anCompleted = GT_TRUE;
    mpdOper.do_adminLinkUp = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macPort,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (GT_FALSE == mpdOper.anCompleted)
    {
        cpssOsPrintf("Error Mac [%d] PHY not 'AN completed' \n",
                          macPort);
        return GT_NOT_READY;
    }

    *speedPtr        = mpdOper.speed;
    *isFullDuplexPtr = mpdOper.isFullDuplex;
    *adminUpPtr      = mpdOper.adminLinkUp;

    return GT_OK;
}


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
)
{
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_duplex = GT_TRUE;
    mpdOper.isFullDuplex    = isFullDuplex;

    return doMpdOper_simpleSet(devNum,macPort,&mpdOper);
}

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
)
{
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_speed = GT_TRUE;
    mpdOper.speed    = speed;

    return doMpdOper_simpleSet(devNum,macPort,&mpdOper);
}

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
)
{
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_duplex       = GT_TRUE;
    mpdOper.anEnable        = anEnable;
    mpdOper.anCapabilities  = capabilities;

    return doMpdOper_simpleSet(devNum,macPort,&mpdOper);
}

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
)
{
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_mdixMode = GT_TRUE;
    mpdOper.mdixMode    = adminMode;

    return doMpdOper_simpleSet(devNum,macPort,&mpdOper);
}

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
)
{
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_restart       = GT_TRUE;

    return doMpdOper_simpleSet(devNum,macPort,&mpdOper);
}

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
    OUT GT_U32                   *capabilitiesPtr
)
{
    GT_STATUS   rc;
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_anCapabilitiesGet = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macPort,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    *capabilitiesPtr = mpdOper.anCapabilitiesGet;

    return GT_OK;
}

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
)
{
    GT_STATUS   rc;
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_anRemoteCapabilities = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macPort,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    *capabilitiesPtr = mpdOper.anRemoteCapabilities;

    return GT_OK;
}

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
)
{
    GT_STATUS   rc;
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_mdixMode      = GT_TRUE;
    mpdOper.do_mdixAdminMode = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macPort,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    *operStatusPtr = mpdOper.mdixMode;
    *adminModePtr  = mpdOper.mdixAdminMode;

    return GT_OK;
}



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
)
{
    GT_STATUS   rc;
    SIMPLE_MPD_OPER_STC  mpdOper = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    mpdOper.do_linkPartnerPause      = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macPort,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    *linkPartnerPause_CapablePtr            = mpdOper.linkPartnerPause_Capable;
    *linkPartnerPause_asymetricRequestedPtr = mpdOper.linkPartnerPause_asymetricRequested;

    return GT_OK;
}

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
)
{
    GT_STATUS               rc = GT_OK;
    SIMPLE_MPD_OPER_STC  mpdOper;

    mpdOper.do_linkStatusGet       = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macNum,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(linkStatusPtr != NULL)
    {
        *linkStatusPtr      = mpdOper.linkStatus;
    }
    else
    {
        rc = GT_BAD_PTR;
    }

    return rc;
}

/**
* @internal ezbMpdPortLinkStatusGet function
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
GT_STATUS ezbMpdPortLinkStatusGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            macNum
)
{
    GT_STATUS               rc = GT_OK;
    SIMPLE_MPD_OPER_STC  mpdOper;

    mpdOper.do_linkStatusGet       = GT_TRUE;

    rc = doMpdOper_simpleGet(devNum,macNum,&mpdOper);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("------- Link Status of MAC[%d]: %s -------\n",
            macNum, mpdOper.linkStatus ? "GT_TRUE" : "GT_FALSE");

    return rc;
}

/**
* @internal prvezbMpdInitHw function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           call this one only after ezbMpdPortInit(...) called  an all ports in all devices
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS prvEzbMpdInitHw
(
    void
)
{
    GT_U32 i;
    for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++){
        prvMpdGlobalDb_PTR->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_FLASH_E;
    }
    if (MPD_OK_E != mpdDriverInitHw())
    {
        return GT_FAIL;
    }
    return GT_OK;
}


/*****************************************************************************
* FUNCTION NAME: util_sleep
*
* DESCRIPTION:         implementation of sleep in ms for MPD
*
*
*****************************************************************************/
static BOOLEAN util_sleep
(
    IN UINT_32 time_in_ms
)
{
    cpssOsTimerWkAfter(time_in_ms);
    return TRUE;
}
/*****************************************************************************
* FUNCTION NAME: util_malloc
*
* DESCRIPTION:         implementation of malloc for pdl
*
*
*****************************************************************************/
static void * util_malloc
(
    UINT_32  size
)
{
    return cpssOsMalloc(size);
}
/*****************************************************************************
* FUNCTION NAME: util_free
*
* DESCRIPTION:         implementation of free for pdl
*
*
*****************************************************************************/
static void util_free
(
    void*  ptr
)
{
    cpssOsFree(ptr);
}

/*****************************************************************************
* FUNCTION NAME: util_phy_transceiver_enable
*
* DESCRIPTION:  implementation of transceiver for MPD
*
*
*****************************************************************************/
static BOOLEAN util_phy_transceiver_enable
(
    IN UINT_32     rel_ifIndex,
    IN BOOLEAN     enable
)
{
    GT_U32           max_retries = 3, count = 0;
    GT_U8            dev, port;
    PDL_STATUS       pdl_status;
    PDL_SFP_TX_ENT   pdl_read_value = PDL_SFP_TX_DISABLE_E, pdl_write_value;
    PRV_MPD_PORT_HASH_ENTRY_STC *port_entry_PTR = NULL;
    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    port = port_entry_PTR->initData_PTR->port;

    pdl_write_value = enable ? PDL_SFP_TX_ENABLE_E : PDL_SFP_TX_DISABLE_E;
    pdl_read_value = enable ? PDL_SFP_TX_DISABLE_E : PDL_SFP_TX_ENABLE_E;

    while (pdl_read_value != pdl_write_value)
    {
        if (count >= max_retries)
            break;

        pdl_status = pdlSfpHwTxSet (dev, port, pdl_write_value);
        if (pdl_status != PDL_OK) {
            return FALSE;
        }

        util_sleep(100); /* sleep in-order to give the peripheral to finish the transaction */

        pdl_status = pdlSfpHwTxGet(dev, port, &pdl_read_value);
        if (pdl_status != PDL_OK) {
            return FALSE;
        }

        count++;
    }

    return TRUE;

}

/*****************************************************************************
* FUNCTION NAME: util_phy_handle_failure
*
* DESCRIPTION:         implementation of handle failure for MPD
*
*
*****************************************************************************/
static MPD_RESULT_ENT util_phy_handle_failure
(
    IN UINT_32                   rel_ifIndex,
    IN MPD_ERROR_SEVERITY_ENT    severity,
    IN UINT_32                   line,
    IN const char              * calling_func_PTR,
    IN const char              * error_text_PTR
)
{
    cpssOsPrintf("MPDError - rel_ifIndex[%d],severity[%d] line: %1d func: %2s msg: %3s\n",
        rel_ifIndex ,severity,
        line, calling_func_PTR, error_text_PTR);
    return MPD_OK_E;
}

/*****************************************************************************
* FUNCTION NAME: util_phy_file_load
*
* DESCRIPTION:         implementation of Load a file
*
*
*****************************************************************************/
static void util_phy_file_load
(
    IN char    *codeFileName_PTR,
    IN void    *mem_PTR
)
{
    FILE        * filePTR;
    size_t        fileSize;
    size_t        actuallyRead;

    if ( (codeFileName_PTR == NULL) || (mem_PTR == NULL) )
        return;

    if ( (filePTR = fopen(codeFileName_PTR, "rb")) != NULL )
    {
        /* Calculate the file length */
        fseek(filePTR, 0, SEEK_END);
        fileSize = ftell(filePTR);
        fseek(filePTR, 0, SEEK_SET);

        actuallyRead = fread((void *)mem_PTR, 1, fileSize, filePTR);
        if(actuallyRead == GT_NA)
        {
            cpssOsPrintf("");
        }

        fclose(filePTR);
    }
}

/*****************************************************************************
* FUNCTION NAME: util_phy_file_get_size
*
* DESCRIPTION: implementation of get packed file size of phy FW
*
*
*****************************************************************************/
static UINT_32 util_phy_file_get_size
(
    IN char          *codeFileName_PTR      /* File name */
)
{
    FILE * filePTR;
    UINT_32 fileSize;


    if ( codeFileName_PTR == NULL )
        return 0;

    /* Open file stream */
    if ( (filePTR = fopen(codeFileName_PTR, "rb")) == NULL )
        return (0);

    /* Calculate the file length */
    fseek(filePTR, 0, SEEK_END);
    fileSize = ftell(filePTR);
    fclose(filePTR);

    return fileSize;
}

typedef struct {
        MPD_TYPE_ENT         phy_type;
        char                    * main_file_name;
        char                    * slave_file_name;
}util_phy_firmware_files_STC;

static util_phy_firmware_files_STC util_phy_firmware_file_names[] = {
        {MPD_TYPE_88X32x0_E, "32x0/phycode_3220.hdr","slave_download_file/flashdownloadlslave.bin"},
        {MPD_TYPE_88X33x0_E, "33x0/phycode_3340.hdr","slave_download_file/flashdownloadlslave.bin"},
        {MPD_TYPE_88X20x0_E, "20x0/phycode_2040.hdr","slave_download_file/flashdownloadlslave.bin"},
        {MPD_TYPE_88X2180_E, "2180/phycode_2180.hdr","slave_download_file/flashdownloadlslave.bin"}
};

#define util_phy_firmware_files_size_CNS        (sizeof(util_phy_firmware_file_names) / sizeof (util_phy_firmware_files_STC))

/*****************************************************************************
* FUNCTION NAME: util_phy_get_fw_files
*
* DESCRIPTION:         implementation of handle get FW files for MPD
*
*
*****************************************************************************/
static BOOLEAN util_phy_get_fw_files
(
    IN  MPD_TYPE_ENT          phyType,
    OUT MPD_FW_FILE_STC      * mainFile_PTR
)
{
    UINT_32  i;
    util_phy_firmware_files_STC            * file_names_PTR;
    char     main_file_name[256];

    if (phyType >= MPD_TYPE_NUM_OF_TYPES_E)
        return FALSE;

    for (i = 0; i < util_phy_firmware_files_size_CNS; i++) {
        if (util_phy_firmware_file_names[i].phy_type == phyType) {
            /* found */
            break;
        }
    }
    if (i >= util_phy_firmware_files_size_CNS) {
        return FALSE;
    }
    file_names_PTR = &util_phy_firmware_file_names[i];

    #ifdef ASIC_SIMULATION
    cpssOsSprintf (main_file_name,  "./phy_firmware/%s", file_names_PTR->main_file_name);
    #else
    cpssOsSprintf (main_file_name,  "/usr/phy_firmware/%s", file_names_PTR->main_file_name);
    #endif

    /* MAIN FW file handling */
    if (main_file_name == NULL)
        return FALSE;

    /* get firmware file size */
    mainFile_PTR->dataSize = util_phy_file_get_size(main_file_name);
    if (mainFile_PTR->dataSize == 0) {
        mainFile_PTR->data_PTR = NULL;
        return FALSE;
    }

    /* allocate space for firmware file to load into ram */
    mainFile_PTR->data_PTR = PRV_MPD_ALLOC_MAC(mainFile_PTR->dataSize);
    if (mainFile_PTR->data_PTR == NULL) {
        return FALSE;
    }

    /* load firmware file into ram */
    util_phy_file_load(main_file_name, mainFile_PTR->data_PTR);

    return TRUE;
}

static MPD_RESULT_ENT utils_phy_disable_smi_autoneg
(
    UINT_32                         rel_ifIndex,
    BOOLEAN                         disable,
    BOOLEAN                       * prev_status_PTR
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    UINT_8      		smiDev, port;
    GT_BOOL             prev_state, new_status;
    GT_STATUS           gt_status;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (port_entry_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}

    smiDev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
	port = port_entry_PTR->initData_PTR->port;

    gt_status = cpssDxChPhyAutonegSmiGet(smiDev, port, &prev_state);
    if (gt_status != GT_OK) {
        return MPD_OP_FAILED_E;
    }

    (* prev_status_PTR) = (prev_state == GT_TRUE)?FALSE:TRUE;
    new_status = ( disable == TRUE ) ? GT_FALSE : GT_TRUE;
    if (prev_state != new_status) {
    	gt_status = cpssDxChPhyAutonegSmiSet(smiDev, port, new_status );
    	if (gt_status != GT_OK) {
    		return MPD_OP_FAILED_E;
    	}
    }

    return MPD_OK_E;
}

static BOOLEAN utils_phy_smi_register_read
(
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		 mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
    OUT UINT_16    * value_PTR
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
	UINT_8      							smiDev, port;
	UINT_8      							smiInterface;
	UINT_32     							portGroup, portGroupsBmp;
    GT_STATUS                               result;
	PRV_MPD_PORT_HASH_ENTRY_STC 		  * port_entry_PTR;
    EZB_GEN_PHY_INFO_STC             * phyInfoPtr;
    EZB_PP_CONFIG                    * ezbPpConfigPtr;
    GT_U32                                  ezbDbIndex;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
	port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (port_entry_PTR == NULL) {
		return FALSE;
	}

	if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
		return FALSE;
	}

	smiDev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;

    /*ezbEzbCpssDevNumToAppDemoIndexConvert(smiDev,&ezbDbIndex);*/
    ezbDbIndex = smiDev;
    ezbPpConfigPtr = &ezbPpConfigList[ezbDbIndex];
	port = port_entry_PTR->initData_PTR->port;
    phyInfoPtr = &ezbPpConfigPtr->macGenInfoPtr[port].phyInfo;

	smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);
    if(phyInfoPtr->smiXmsiInterface.interfaceType == EZB_XML_INTERFACE_TYPE_SMI_E)
    {
        result = cpssSmiRegisterReadShort(	smiDev,
                portGroupsBmp,
                smiInterface,
                mdioAddress,
                address,
                value_PTR);
    }
    else
    {
        result = cpssXsmiPortGroupRegisterRead( smiDev,
                portGroupsBmp,
                smiInterface,
                mdioAddress,
                address,
                deviceOrPage,
                value_PTR);
    }
    return (result == GT_OK) ? TRUE : FALSE;
}


static BOOLEAN utils_phy_smi_register_write
(
	IN UINT_32      rel_ifIndex,
	IN UINT_8		mdioAddress,
	IN UINT_16      deviceOrPage,
	IN UINT_16      address,
	IN UINT_16      value
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    UINT_8      							smiDev, port;
    UINT_8      							smiInterface;
    UINT_32     							portGroup, portGroupsBmp;
    GT_STATUS                               result;
	PRV_MPD_PORT_HASH_ENTRY_STC 		  * port_entry_PTR;
    EZB_GEN_PHY_INFO_STC             * phyInfoPtr;
    EZB_PP_CONFIG                    * ezbPpConfigPtr;
    GT_U32                                  ezbDbIndex;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }
	smiDev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;

    /*ezbEzbCpssDevNumToAppDemoIndexConvert(smiDev,&ezbDbIndex);*/
    ezbDbIndex = smiDev;
    ezbPpConfigPtr = &ezbPpConfigList[ezbDbIndex];
	port = port_entry_PTR->initData_PTR->port;
    phyInfoPtr = &ezbPpConfigPtr->macGenInfoPtr[port].phyInfo;

    smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);
    if(phyInfoPtr->smiXmsiInterface.interfaceType == EZB_XML_INTERFACE_TYPE_SMI_E)
    {
        result = cpssSmiRegisterWriteShort(	smiDev,
                portGroupsBmp,
                smiInterface,
                mdioAddress,
                address,
                value);
    }
    else
    {
        result = cpssXsmiPortGroupRegisterWrite(    smiDev,
                portGroupsBmp,
                smiInterface,
                mdioAddress,
                address,
                deviceOrPage,
                value);
    }
	return (result == GT_OK) ? TRUE : FALSE;
}

/*****************************************************************************
* FUNCTION NAME: utils_pdl_phy_init
*
* DESCRIPTION:         implementation of phy init for pdl
*
*
*****************************************************************************/
static PDL_STATUS utils_pdl_phy_init
(
    void
)
{
    MPD_CALLBACKS_STC   phy_callbacks;
    MPD_RESULT_ENT      rc;

    /* MPD callbacks */
    phy_callbacks.sleep_PTR          = util_sleep;
    phy_callbacks.alloc_PTR          = util_malloc;
    phy_callbacks.free_PTR           = util_free;
    phy_callbacks.debug_bind_PTR     = NULL;
    phy_callbacks.is_active_PTR      = NULL;
    phy_callbacks.debug_log_PTR      = NULL;
    phy_callbacks.txEnable_PTR       = util_phy_transceiver_enable;
    phy_callbacks.handle_failure_PTR = util_phy_handle_failure;
    phy_callbacks.getFwFiles_PTR     = util_phy_get_fw_files;
    phy_callbacks.mdioRead_PTR       = utils_phy_smi_register_read;
    phy_callbacks.mdioWrite_PTR      = utils_phy_smi_register_write;
    phy_callbacks.smiAn_disable_PTR  = utils_phy_disable_smi_autoneg;

    /* initialize DBs & bind debug log ids */
    rc = mpdDriverInitDb(&phy_callbacks);
    if (rc != MPD_OK_E) {
        PRV_MPD_HANDLE_FAILURE_MAC( 0, MPD_ERROR_SEVERITY_FATAL_E, "mpdDriverInitDb failed");
    }

    return PDL_OK;
}

/**
* @internal prvezbMpdLoad function
* @endinternal
*
* @brief  load MPD for EZ_BRINGUP about the board.
*
*
* @retval
*/
GT_STATUS prvEzbMpdLoad(
    void
)
{
    PDL_STATUS pdl;
    GT_STATUS portIdx;

    /* PHY init */
    pdl = utils_pdl_phy_init();
    if (pdl != PDL_OK)
    {
        cpssOsPrintf(" -- ezb : ERROR : Can't initialize PHY. utils_pdl_phy_init returned #[%d \n", pdl);
        return GT_FAIL;
    }
    nextFreeGlobalIfIndex = 1;/* reset the ifIndexes for the coming session */

    for(portIdx = 0; portIdx < MAX_MPD_PORTS_CNS ; portIdx++)
    {
        ifIndexInfoArr[portIdx].macNum = 0xFFFFFFFF;
        ifIndexInfoArr[portIdx].devNum = 0xFF;
    }
    return GT_OK;
}

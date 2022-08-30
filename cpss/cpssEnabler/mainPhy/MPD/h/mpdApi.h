/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdApi.h
 * @brief Contain types that are required in order to perform PHY operations
 *      
 */

#ifndef MPD_API_H_
#define MPD_API_H_

#include <mpdTypes.h>

#define MPD_PORT_NUM_TO_GROUP_MAC(__port)                             (__port / 16)
#define MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(__portGroup)             (1<<__portGroup)
#define MPD_MAX_PORT_NUMBER_CNS  (128)


/** @addtogroup PHYs Supported PHY types
 * @{
 * @enum MPD_TYPE_ENT
 * @brief PHY types supported by MPD
 */

typedef enum {
	/** @brief No PHY, fiber */
	MPD_TYPE_DIRECT_ATTACHED_FIBER_E /*= PDL_PHY_TYPE_direct_attached_fiber_E*/,
	/** @brief 10M/100M/1G 				2xSGMII 	<--> 	QSGMII <--> 2xAMD/F/C  */
	MPD_TYPE_88E1543_E				 /*= PDL_PHY_TYPE_alaska_88E1543_E*/,
	/** @brief 10M/100M/1G 				QSGMII 		<--> 	4xC */
	MPD_TYPE_88E1545_E				 /*= PDL_PHY_TYPE_alaska_88E1545_E*/,
	/** @brief 10M/100M/1G 				QSGMII 		<--> 	4xAMD/F/C */
	MPD_TYPE_88E1548_E				 /*= PDL_PHY_TYPE_alaska_88E1548_E*/,
	/** @brief 10M/100M/1G 				2xQSGMII 	<--> 	8xC */
	MPD_TYPE_88E1680_E 				 /*= PDL_PHY_TYPE_alaska_88E1680_E*/,
	/** @brief 10M/100M/1G 				2xQSGMII 	<--> 	8xC with LinkCrypt */
	MPD_TYPE_88E1680L_E 			 /*= PDL_PHY_TYPE_alaska_88E1680L_E*/,
	/** @brief 10M/100M/1G 				2xSGMII		<--> 	2xC					88E1512 / 88E1514 */
	MPD_TYPE_88E151x_E 			 	 /*= PDL_PHY_TYPE_alaska_88E151X_E*/,
	/** @brief 10M/100M 				2xQSGMII 	<--> 	8xC */
	MPD_TYPE_88E3680_E 				 /*= PDL_PHY_TYPE_alaska_88E3680_E*/,
	/** @brief 1G/10G 					2/4xXFI		<-->	2/4xAMD/F/C			88X3220 / 88X3240 */
	MPD_TYPE_88X32x0_E 				 /*= PDL_PHY_TYPE_alaska_88E32x0_E*/,
	/** @brief 10M/100M/1G/2.5G/5G/10G 	USXGMII 	<--> 	1/4xAMD/F/C			88X3310 / 88X3340 */
	MPD_TYPE_88X33x0_E 				 /*= PDL_PHY_TYPE_alaska_88E33X0_E*/,
	/** @brief 10M/100M/1G/2.5G/5G 		USXGMII 	<--> 	1/4xAMD/F/C			88E2010 / 88E2040 */
	MPD_TYPE_88X20x0_E 				 /*= PDL_PHY_TYPE_alaska_88E20X0_E*/,
	/** @brief 10M/100M/1G/2.5G/5G 		1/2xUSXGMII <-->	1/8xC				88E2110 / 88E2180 */
	MPD_TYPE_88X2180_E 				 /*= PDL_PHY_TYPE_alaska_88E2180_E*/,
	/** @brief 10M/100M/1G/2.5G/5G 		MP/1xUSXGMII <-->	1/4xC				88E2540 */
	MPD_TYPE_88E2540_E 				 /*= PDL_PHY_TYPE_alaska_88E2540_E*/,
	/** @brief 10M/100M/1G/2.5G/5G/10G 		MP/1/2xUSXGMII <-->	1/8xC			88X3540 */
	MPD_TYPE_88X3540_E 				 /*= PDL_PHY_TYPE_alaska_88X3540_E*/,
    /** @brief 10M/100M/1G 				2xQSGMII 	<--> 	8xC with LinkCrypt */
    MPD_TYPE_88E1780_E 			     /*= PDL_PHY_TYPE_alaska_88E1780_E*/,
    /** @brief 10M/100M/1G/2.5G/5G 		MP/1xUSXGMII <-->	1/4xC				88E2580 */
    MPD_TYPE_88E2580_E 				 /*= PDL_PHY_TYPE_alaska_88E2580_E*/,
	/** @brief 1G/10G/25G/50G/100G/200G/400G  */
	MPD_TYPE_88X7120_E 							/*  88x7120	*/,
	MPD_TYPE_LAST_SUPPERTED_E = MPD_TYPE_88X7120_E,
	/** @brief User defined reserved, allow application to bind its' own PHY types & driver see \a mpdBindExternalPhyCallback */
	MPD_TYPE_FIRST_USER_DEFINED_E,
	MPD_TYPE_LAST_USER_DEFINED_E = MPD_TYPE_FIRST_USER_DEFINED_E +32,
	MPD_TYPE_NUM_OF_TYPES_E,
	MPD_TYPE_INVALID_E
} MPD_TYPE_ENT;
/** @} */

/** @addtogroup Register_Access Register Access
 * @{
 * @enum MPD_MDIO_ACCESS_TYPE_ENT
 * @brief MDIO access type
 */
typedef enum {
	/** @brief MDIO read */
	MPD_MDIO_ACCESS_READ_E = 0,
	/** @brief MDIO write */
	MPD_MDIO_ACCESS_WRITE_E = 1
} MPD_MDIO_ACCESS_TYPE_ENT;
/** @} */


/** @brief MAX INTERFACE ID NUMBER */
/** Bus Number */
#define MPD_MAX_INTERFACE_ID_NUMBER	   2


/** @addtogroup VCT
 * @{
 * @typedef MPD_SPEED_CAPABILITY_TYP
 * @brief speed capability bitmap
 * - MPD_SPEED_CAPABILITY_UNKNOWN_CNS                           <B>  0   </B> 
 * - MPD_SPEED_CAPABILITY_10M_CNS                               <B> (1<<1) </B>
 * - MPD_SPEED_CAPABILITY_10M_HD_CNS                            <B> (1<<2)  </B>
 * - MPD_SPEED_CAPABILITY_100M_CNS                              <B> (1<<3)  </B>
 * - MPD_SPEED_CAPABILITY_100M_HD_CNS                           <B> (1<<4)  </B>
 * - MPD_SPEED_CAPABILITY_1G_CNS                                <B> (1<<5)  </B>
 * - MPD_SPEED_CAPABILITY_10G_CNS                               <B> (1<<6)  </B>
 * - MPD_SPEED_CAPABILITY_2500M_CNS                             <B> (1<<7)  </B>
 * - MPD_SPEED_CAPABILITY_5G_CNS                                <B> (1<<8)  </B>
 * - MPD_SPEED_CAPABILITY_12G_CNS                               <B> (1<<9)  </B>
 * - MPD_SPEED_CAPABILITY_16G_CNS                               <B> (1<<10) </B>
 * - MPD_SPEED_CAPABILITY_13600M_CNS                            <B> (1<<11) </B>
 * - MPD_SPEED_CAPABILITY_20G_CNS                               <B> (1<<12) </B>
 * - MPD_SPEED_CAPABILITY_40G_CNS                               <B> (1<<13) </B>
 * - MPD_SPEED_CAPABILITY_100G_CNS                              <B> (1<<14) </B>
 * - MPD_SPEED_CAPABILITY_25G_CNS                               <B> (1<<15) </B>
 * - MPD_SPEED_CAPABILITY_50G_CNS                               <B> (1<<16) </B>
 * - MPD_SPEED_CAPABILITY_24G_CNS                               <B> (1<<17) </B>
 */
 
typedef UINT_32 MPD_SPEED_CAPABILITY_TYP;
#define MPD_SPEED_CAPABILITY_UNKNOWN_CNS                             0x0
#define MPD_SPEED_CAPABILITY_10M_CNS                                 (1<<1)
#define MPD_SPEED_CAPABILITY_10M_HD_CNS                              (1<<2)
#define MPD_SPEED_CAPABILITY_100M_CNS                                (1<<3)
#define MPD_SPEED_CAPABILITY_100M_HD_CNS                             (1<<4)
#define MPD_SPEED_CAPABILITY_1G_CNS                                  (1<<5)
#define MPD_SPEED_CAPABILITY_10G_CNS                                 (1<<6)
#define MPD_SPEED_CAPABILITY_2500M_CNS                               (1<<7)
#define MPD_SPEED_CAPABILITY_5G_CNS                                  (1<<8)
#define MPD_SPEED_CAPABILITY_12G_CNS                                 (1<<9)
#define MPD_SPEED_CAPABILITY_16G_CNS                                 (1<<10)
#define MPD_SPEED_CAPABILITY_13600M_CNS                              (1<<11)
#define MPD_SPEED_CAPABILITY_20G_CNS                                 (1<<12)
#define MPD_SPEED_CAPABILITY_40G_CNS                                 (1<<13)
#define MPD_SPEED_CAPABILITY_100G_CNS                                (1<<14)
#define MPD_SPEED_CAPABILITY_25G_CNS                                 (1<<15)
#define MPD_SPEED_CAPABILITY_50G_CNS                                 (1<<16)
#define MPD_SPEED_CAPABILITY_24G_CNS                                 (1<<17)
#define MPD_SPEED_CAPABILITY_200G_CNS                                (1<<18)
/** @} */

/** @addtogroup Speed
 * @{
 * @enum MPD_SPEED_ENT
 */
typedef enum {
    MPD_SPEED_10M_E  	= 0,   	/* HALG_config_common_portSpeed_10_CNS		*/
    MPD_SPEED_100M_E 	= 1,   	/* HALG_config_common_portSpeed_100_CNS		*/
    MPD_SPEED_1000M_E	= 2,   	/* HALG_config_common_portSpeed_1000_CNS	*/
    MPD_SPEED_10000M_E	= 3,	/* HALG_config_common_portSpeed_10000_CNS	*/
    MPD_SPEED_2500M_E	= 5,   	/* HALG_config_common_portSpeed_2500_CNS	*/
    MPD_SPEED_5000M_E	= 6,   	/* HALG_config_common_portSpeed_5000_CNS	*/
    MPD_SPEED_20000M_E	= 8,   	/* HALG_config_common_portSpeed_20000_CNS 	*/
    MPD_SPEED_40G_E     = 9,
    MPD_SPEED_100G_E	= 13,
    MPD_SPEED_50G_E	= 14,
    MPD_SPEED_25G_E	= 21,
    MPD_SPEED_200G_E	= 24,
    MPD_SPEED_400G_E	= 25,
    MPD_SPEED_LAST_E	= 29   	/* HALG_config_common_portSpeed_NA_CNS 	*/
} MPD_SPEED_ENT;
/** @} */
/** @internal
 * @enum MPD_EEE_ENABLE_MODE_TYP
 * @brief EEE Enable mode
 */
typedef enum {
/** @internal @brief EEE enabled/disabled on link up/down event, and on admin configuration */
	MPD_EEE_ENABLE_MODE_LINK_CHANGE_E	=	0,
/** @internal @brief EEE enabled/disabled on admin configuration only */
	MPD_EEE_ENABLE_MODE_ADMIN_ONLY_E 	= 	1
} MPD_EEE_ENABLE_MODE_ENT;
/** @addtogroup EEE
 * @{
 * @enum MPD_EEE_SPEED_ENT
 * @brief EEE Speeds
 */
typedef enum {
	/**@brief 10M */
	MPD_EEE_SPEED_10M_E   = 0,
	/**@brief 100M */
	MPD_EEE_SPEED_100M_E  = 1,
	/**@brief 1G */
	MPD_EEE_SPEED_1G_E    = 2,
	/**@brief 10G */
	MPD_EEE_SPEED_10G_E   = 3,
	/**@brief 2.5G */
	MPD_EEE_SPEED_2500M_E = 4,
	/**@brief 5G */
	MPD_EEE_SPEED_5G_E    = 5,
	MPD_EEE_SPEED_MAX_E   = 6
}MPD_EEE_SPEED_ENT;
 /** @} */
 
/** @addtogroup Autonegotiation
 * @{
 * @typedef MPD_AUTONEG_CAPABILITIES_TYP
 * @brief Auto Negotiation bitmap values
 * - MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS                         \b 0x0080
 * - MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS                         \b 0x0040
 * - MPD_AUTONEG_CAPABILITIES_TENHALF_CNS                         \b 0x0020
 * - MPD_AUTONEG_CAPABILITIES_TENFULL_CNS                         \b 0x0010
 * - MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS                        \b 0x0008
 * - MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS                        \b 0x0004
 * - MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS                        \b 0x0002
 * - MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS                        \b 0x0001
 * - MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS                       \b 0x2000
 * - MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS                         \b 0x4000
 * - MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS                        \b 0x8000
 * - MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS                        \b 0x10000
 * - MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS                        \b 0x20000
 * - MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS                        \b 0x40000
 * - MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS                       \b 0x80000
 */
/* Must be the same values as MIB_swIfSpeedDuplexAutoNegCapab in lib/mib/mib_l2.c */
typedef UINT_32 MPD_AUTONEG_CAPABILITIES_TYP;
#define MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS                         0x0080 /* default */
#define MPD_AUTONEG_CAPABILITIES_UNKNOWN_CNS                         0x0040 /* unknown */
#define MPD_AUTONEG_CAPABILITIES_TENHALF_CNS                         0x0020 /* 10/Half */
#define MPD_AUTONEG_CAPABILITIES_TENFULL_CNS                         0x0010 /* 10/Full */
#define MPD_AUTONEG_CAPABILITIES_FASTHALF_CNS                        0x0008 /* 100/Half */
#define MPD_AUTONEG_CAPABILITIES_FASTFULL_CNS                        0x0004 /* 100/Full */
#define MPD_AUTONEG_CAPABILITIES_GIGAHALF_CNS                        0x0002 /* 1000/Half */
#define MPD_AUTONEG_CAPABILITIES_GIGAFULL_CNS                        0x0001 /* 1000/Full */

#define MPD_AUTONEG_CAPABILITIES_10G_FULL_CNS                        0x8000 /* 10G/Full */
#define MPD_AUTONEG_CAPABILITIES_5G_FULL_CNS                         0x4000 /* 5000/Full */
#define MPD_AUTONEG_CAPABILITIES_2_5G_FULL_CNS                       0x2000 /* 2500/Full */
#define MPD_AUTONEG_CAPABILITIES_25G_FULL_CNS                        0x1000 /* 25G/Full */
#define MPD_AUTONEG_CAPABILITIES_40G_FULL_CNS                        0x0800 /* 40G/Full */
#define MPD_AUTONEG_CAPABILITIES_50G_FULL_CNS                        0x0400 /* 50G/Full */
#define MPD_AUTONEG_CAPABILITIES_100G_FULL_CNS                       0x0200 /* 100G/Full */
#define MPD_AUTONEG_CAPABILITIES_200G_FULL_CNS                       0x0100 /* 200G/Full */

/** @} */

/** @addtogroup Green
 * @{
 * @enum MPD_GREEN_SET_ENT
 * @brief Green setting
 */
typedef enum {
/** @brief Do nothing */
	MPD_GREEN_NO_SET_E  = 0,
/** @brief Enable */
	MPD_GREEN_ENABLE_E  = 1,
/** @brief Disable */
	MPD_GREEN_DISABLE_E = 2
}MPD_GREEN_SET_ENT;
/**
 * @enum MPD_GREEN_READINESS_TYPE_ENT
 * @brief Green readiness type (ED/SR)
 */
typedef enum {
	/**@brief Short Reach */
	MPD_GREEN_READINESS_TYPE_SR_E = 0,
	/**@brief Energy Detect */
	MPD_GREEN_READINESS_TYPE_ED_E = 1
}MPD_GREEN_READINESS_TYPE_ENT;
/**
 * @enum MPD_GREEN_READINESS_ENT
 * @brief Green readiness value
 */
typedef enum {
/** @brief Normal */
	MPD_GREEN_READINESS_OPRNORMAL_E = 0,
/** @brief Fiber */
	MPD_GREEN_READINESS_FIBER_E = 1,
/** @brief Combo fiber */
	MPD_GREEN_READINESS_COMBO_FIBER_E = 2,
/** @brief Green (type) not supported */
	MPD_GREEN_READINESS_NOT_SUPPORTED_E = 3,
/** @brief Green (type) is always enabled */
	MPD_GREEN_READINESS_ALWAYS_ENABLED_E = 4
}MPD_GREEN_READINESS_ENT;
/** @} */

/** @addtogroup VCT
 * @{
 * @enum MPD_VCT_RESULT_ENT
 * @brief Result of VCT Test
 */
/* start with 1 to fit SNMP values - don't change */
typedef enum {
	/**@brief Cable O.K */
	MPD_VCT_RESULT_CABLE_OK_E 			= 1,
	/**@brief Indicates that a 2 pair cable is used */
	MPD_VCT_RESULT_2_PAIR_CABLE_E		= 2,
	/**@brief No cable is connected */
	MPD_VCT_RESULT_NO_CABLE_E			= 3,
	/**@brief Open ended cable (not terminated) */
	MPD_VCT_RESULT_OPEN_CABLE_E			= 4,
	/**@brief Short Cable (inter Pair short) */
	MPD_VCT_RESULT_SHORT_CABLE_E		= 5,
	/**@brief Bad Cable not matching other criteria */
	MPD_VCT_RESULT_BAD_CABLE_E			= 6,
	/**@brief Impedence Mismatch */
	MPD_VCT_RESULT_IMPEDANCE_MISMATCH_E = 7
} MPD_VCT_RESULT_ENT;
/**
 * @enum MPD_VCT_TEST_TYPE_ENT
 * @brief VCT test type
 */
/* start with 13 to fit SNMP values - don't change */
typedef enum {
	MPD_VCT_TEST_TYPE_CABLECHANNEL1_E 	= 13,
	MPD_VCT_TEST_TYPE_CABLECHANNEL2_E 	= 14,
	MPD_VCT_TEST_TYPE_CABLECHANNEL3_E	= 15,
	MPD_VCT_TEST_TYPE_CABLECHANNEL4_E	= 16,
	MPD_VCT_TEST_TYPE_CABLEPOLARITY1_E	= 17,
	MPD_VCT_TEST_TYPE_CABLEPOLARITY2_E	= 18,
	MPD_VCT_TEST_TYPE_CABLEPOLARITY3_E	= 19,
	MPD_VCT_TEST_TYPE_CABLEPOLARITY4_E  = 20,
	MPD_VCT_TEST_TYPE_CABLEPAIRSKEW1_E  = 21,
	MPD_VCT_TEST_TYPE_CABLEPAIRSKEW2_E  = 22,
	MPD_VCT_TEST_TYPE_CABLEPAIRSKEW3_E  = 23,
	MPD_VCT_TEST_TYPE_CABLEPAIRSKEW4_E  = 24
}MPD_VCT_TEST_TYPE_ENT;
/** @} */
/** @addtogroup Fiber
 * @{
 * @enum MPD_OP_MODE_ENT
 * @brief Fiber port operation mode
 */
typedef enum {
/** @brief	SFP and SFP+ */
	MPD_OP_MODE_FIBER_E                                     =  0,
/** @brief	Direct attached/ DAC cable */
	MPD_OP_MODE_DIRECT_ATTACH_E                             =  1,
/** @brief	Copper SFP 1000Base-T/10GBase-T */
	MPD_OP_MODE_COPPER_SFP_E                                =  2,
/** @brief	In case not preset or failed to identify */
	MPD_OP_MODE_UNKNOWN_E                                   =  3,
	MPD_OP_MODE_LAST_E                                 		=  4
}MPD_OP_MODE_ENT;
/**
 * @typedef MPD_TYPE_BITMAP_TYP
 * @brief Port type
 */
typedef UINT_32 MPD_TYPE_BITMAP_TYP;
#define MPD_TYPE_BITMAP_10BASETX_RESERVED_CNS       0x0001  /* not in use */
#define MPD_TYPE_BITMAP_100BASETX_CNS               0x0002
#define MPD_TYPE_BITMAP_1000BASET_CNS               0x0004
#define MPD_TYPE_BITMAP_10GBASET_CNS                0x0008
#define MPD_TYPE_BITMAP_1000BASEKX_CNS              0x0010
#define MPD_TYPE_BITMAP_10GBASEKX4_CNS              0x0020
#define MPD_TYPE_BITMAP_10GBASEKR_CNS               0x0040
#define MPD_TYPE_BITMAP_2500BASET_CNS               0x0080
#define MPD_TYPE_BITMAP_5GBASET_CNS                 0x0100

/** @} */
/** @internal
 * @typedef MPD_ERRATA_TYP
 * @brief Errata
 */
typedef UINT_32 MPD_ERRATA_TYP;
/** @internal @brief Some oarts may have slow link issue with short cable\n
 * write 250.25 = 0 after soft-reset/power up and before auto-negotiation process is complete
 */
#define MPD_ERRATA_3_3_SLOW_LINK_SHORT_CABLE_CNS (1 << 0)

/** @internal
 * @enum MPD_COMBO_MODE_ENT
 * @brief Combo mode
 */
typedef enum {
    MPD_COMBO_MODE_FORCE_FIBER_E,
    MPD_COMBO_MODE_FORCE_COPPER_E,
    MPD_COMBO_MODE_PREFER_FIBER_E,
    MPD_COMBO_MODE_PREFER_COPPER_E,
    MPD_COMBO_MODE_LAST_E
} MPD_COMBO_MODE_ENT;

/**
 * @enum    MPD_FW_DOWNLOAD_TYPE_ENT
 *
 * @brief   phy download type
 */
typedef enum {
    MPD_FW_DOWNLOAD_TYPE_NONE_E, 	/*= PDL_PHY_DOWNLOAD_TYPE_NONE_E,*/              /* no download          */
	MPD_FW_DOWNLOAD_TYPE_RAM_E, 	/*= PDL_PHY_DOWNLOAD_TYPE_RAM_E, */              /* ram download         */
    MPD_FW_DOWNLOAD_TYPE_FLASH_E,   /*= PDL_PHY_DOWNLOAD_TYPE_FLASH_E,*/             /* flash download       */
    MPD_FW_DOWNLOAD_TYPE_LAST_E		/*= PDL_PHY_DOWNLOAD_TYPE_LAST_E*/
} MPD_FW_DOWNLOAD_TYPE_ENT;

/**
* @addtogroup API
* @{Logical operations definition
*/
/**
 *	@enum MPD_OP_CODE_ENT
 *	@brief operation to be performed on PHY
 */
typedef enum {
	/** @brief Set MDI/X mode. See \ref MPD_MDIX_MODE_PARAMS_STC */
    MPD_OP_CODE_SET_MDIX_E,
	/** @brief Get MDI/X operational status. See \ref MPD_MDIX_MODE_PARAMS_STC */
    MPD_OP_CODE_GET_MDIX_E,
	/** @brief Get MDI/X admin configuration. See \ref MPD_MDIX_MODE_PARAMS_STC */
    MPD_OP_CODE_GET_MDIX_ADMIN_E,
	/** @brief Set auto-negotiation parameters. See \ref MPD_AUTONEG_PARAMS_STC */
    MPD_OP_CODE_SET_AUTONEG_E,
	/** @brief Get auto-negotiation admin configuration. See \ref MPD_AUTONEG_PARAMS_STC */
    MPD_OP_CODE_GET_AUTONEG_ADMIN_E,
	/** @brief Restart auto-negotiation on copper port */
    MPD_OP_CODE_SET_RESTART_AUTONEG_E,
	/** @brief Set duplex mode (Half/Full). See \ref MPD_DUPLEX_MODE_PARAMS_STC */
    MPD_OP_CODE_SET_DUPLEX_MODE_E,
	/** @brief Set speed. See \ref MPD_SPEED_PARAMS_STC */
    MPD_OP_CODE_SET_SPEED_E,
	/** @brief Execute Basic VCT (TDR) test (get status and cable length). See \ref MPD_VCT_PARAMS_STC */
    MPD_OP_CODE_SET_VCT_TEST_E,
	/** @brief Execute advanced VCT (TDR & DSP) test (get polarity, sqew, impedance per pair). See \ref MPD_EXT_VCT_PARAMS_STC */
    MPD_OP_CODE_GET_EXT_VCT_PARAMS_E,
	/** @brief Execute advanced VCT (TDR & DSP) test and get cable length (only). See \ref MPD_CABLE_LEN_PARAMS_STC */
    MPD_OP_CODE_GET_CABLE_LEN_E,
	/** @brief Perform (Soft) reset */
    MPD_OP_CODE_SET_RESET_PHY_E,
	/** @brief Admin disable (shutdown). See \ref MPD_PHY_DISABLE_PARAMS_STC */
    MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
	/** @brief Get link partner (advertised) auto-negotiation capabilities. See \ref MPD_AUTONEG_CAPABILITIES_PARAMS_STC */
    MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E,
	/** @brief Advertise flow control capabilities. See \ref MPD_ADVERTISE_FC_PARAMS_STC */
    MPD_OP_CODE_SET_ADVERTISE_FC_E,
	/** @brief Get link partner (advertised) flow control capabilities. See \ref MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC */
    MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E,
	/** @brief Configure Green (ED & SR). See \ref MPD_POWER_MODULES_PARAMS_STC */
    MPD_OP_CODE_SET_POWER_MODULES_E,
	/** @brief Get power consumption values (DB based, see prvMpdUpdatePhyPowerConsumption). see \ref MPD_GREEN_POW_CONSUMPTION_PARAMS_STC */
    MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E,
	/** @brief Get green operational support. See \ref MPD_GREEN_READINESS_PARAMS_STC */
    MPD_OP_CODE_GET_GREEN_READINESS_E,
	/** @brief Execute VCT (DSP) and get accurate cable length. See \ref MPD_CABLE_LEN_NO_RANGE_PARAMS_STC */
    MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E,
	/** @brief Get PHY kind and active media (copper/fiber). See \ref MPD_KIND_AND_MEDIA_PARAMS_STC */
    MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E,
	/** @brief Perform I2C read (for supporting PHY, when I2C is connected to PHY). See \ref MPD_I2C_READ_PARAMS_STC */
    MPD_OP_CODE_GET_I2C_READ_E,
	/** @brief Set SFP present. See \ref MPD_PRESENT_NOTIFICATION_PARAMS_STC */
    MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E,
	/** @brief Perform MDIO read/write. See \ref MPD_MDIO_ACCESS_PARAMS_STC */
    MPD_OP_CODE_SET_MDIO_ACCESS_E,
	/** @brief Advertise EEE capabilities. See \ref MPD_EEE_ADVERTISE_PARAMS_STC */
    MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E,
	/** @brief Enable/Disable EEE. See \ref MPD_EEE_MASTER_ENABLE_PARAMS_STC */
    MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E,
	/** @brief Get EEE remote (Advertised) status. See \ref MPD_EEE_CAPABILITIES_PARAMS_STC */
    MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E,
	/** @brief EEE, set LPI Exit time. See \ref MPD_EEE_LPI_TIME_PARAMS_STC */
    MPD_OP_CODE_SET_LPI_EXIT_TIME_E,
	/** @brief EEE, set LPI Enter time. See \ref MPD_EEE_LPI_TIME_PARAMS_STC */
    MPD_OP_CODE_SET_LPI_ENTER_TIME_E,
	/** @brief Get (local) EEE capability. See \ref MPD_EEE_CAPABILITIES_PARAMS_STC */
    MPD_OP_CODE_GET_EEE_CAPABILITY_E,
	/** @brief Get PHY status (link & speed). See \ref MPD_INTERNAL_OPER_STATUS_STC */
    MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E,
	/** @brief Get VCT capability. See \ref MPD_VCT_CAPABILITY_PARAMS_STC */
    MPD_OP_CODE_GET_VCT_CAPABILITY_E,
	/** @brief Get Data Terminal Equipment status\n
	 * Relevant for devices thet support DTE power function, the DTE power function is used to detect
	 * if a link partner requires power supplied by the POE PSE device
	 * See \ref MPD_DTE_STATUS_PARAMS_STC */
    MPD_OP_CODE_GET_DTE_STATUS_E,
	/** @brief Get PHY temperature. See \ref MPD_TEMPERATURE_PARAMS_STC */
    MPD_OP_CODE_GET_TEMPERATURE_E,
	/** @brief Get PHY revision. See \ref MPD_REVISION_PARAMS_STC */
    MPD_OP_CODE_GET_REVISION_E,
	/** @brief Get supported speed & duplex modes. See \ref MPD_AUTONEG_CAPABILITIES_PARAMS_STC */
    MPD_OP_CODE_GET_AUTONEG_SUPPORT_E,
        /** @brief Set extended speed configurations. See \ref MPD_SPEED_EXT_PARAMS_STC*/
    MPD_OP_CODE_SET_SPEED_EXT_E,
        /** @brief Get extended speed configurations. See \ref MPD_SPEED_EXT_PARAMS_STC*/
    MPD_OP_CODE_GET_SPEED_EXT_E,
        /** @brief Set Serdes rx, tx tune params. See \ref MPD_SERDES_TUNE_STC*/
    MPD_OP_CODE_SET_SERDES_TUNE_E,
        /** @brief Set Serdes Lane Rx/Tx polarity.See \ref MPD_SERDES_LANE_POLARITY_PARAMS_STC */
    MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E,
        /** @brief Multispeed AP advertise*/
    MPD_OP_CODE_SET_AUTONEG_MULTISPEED_E,
    MPD_OP_NUM_OF_OPS_E
} MPD_OP_CODE_ENT;
/** @} */

/** @addtogroup Autonegotiation
 * @{
 *	@enum MPD_AUTO_NEGOTIATION_ENT
 *	@brief Negotiation Enable/Disable
 */
typedef enum {
    MPD_AUTO_NEGOTIATION_ENABLE_E,
    MPD_AUTO_NEGOTIATION_DISABLE_E
} MPD_AUTO_NEGOTIATION_ENT;
/** @} */

/** @addtogroup Duplex
 * @{
 *	@enum MPD_DUPLEX_ADMIN_ENT
 *	@brief Duplex mode (Half/Full)
 */
typedef enum {
    MPD_DUPLEX_ADMIN_MODE_HALF_E,
    MPD_DUPLEX_ADMIN_MODE_FULL_E 
} MPD_DUPLEX_ADMIN_ENT; 
/** @} */

/** @addtogroup MDIX
 * @{
 *	@enum MPD_MDIX_MODE_TYPE_ENT
 *	@brief MDI/X mode
 */
typedef enum {
    MPD_MDI_MODE_MEDIA_E,
    MPD_MDIX_MODE_MEDIA_E,
    MPD_AUTO_MODE_MEDIA_E
} MPD_MDIX_MODE_TYPE_ENT;
/** @} */

/** @addtogroup Fiber
 * @{
 *	@enum MPD_KIND_ENT
 *	@brief PHY Kind
 */
typedef enum {
    MPD_KIND_COPPER_E = 0,
    MPD_KIND_SFP_E = 1,
    MPD_KIND_COMBO_E = 2,
    MPD_KIND_INVALID_E = 3
} MPD_KIND_ENT;
/** @} */

/** @addtogroup Admin
 * @{
 *	@enum MPD_PORT_ADMIN_ENT
 *	@brief Admin status
 */
typedef enum {
    MPD_PORT_ADMIN_UP_E     =   TRUE,
    MPD_PORT_ADMIN_DOWN_E   =   FALSE
} MPD_PORT_ADMIN_ENT;
/** @} */

/** @addtogroup Autonegotiation
 * @{
 *	@enum MPD_AUTONEGPREFERENCE_ENT
 *	@brief Auto-negotiation preference (master/slave)
 */
typedef enum {
    MPD_AUTONEGPREFERENCE_MASTER_E 	= 0,
    MPD_AUTONEGPREFERENCE_SLAVE_E  	= 1,
	MPD_AUTONEGPREFERENCE_UNKNOWN_E = 2
} MPD_AUTONEGPREFERENCE_ENT;
/** @} */
/** @addtogroup Fiber
 * @{
 *	@enum MPD_COMFIG_PHY_MEDIA_TYPE_ENT
 *	@brief Active media
 */
typedef enum {
    MPD_COMFIG_PHY_MEDIA_TYPE_COPPER_E,
    MPD_COMFIG_PHY_MEDIA_TYPE_FIBER_E,
    MPD_COMFIG_PHY_MEDIA_TYPE_INVALID_E
} MPD_COMFIG_PHY_MEDIA_TYPE_ENT;
/** @} */

/** @addtogroup VCT
 * @{
 *  @enum MPD_CABLE_LENGTH_ENT
 *	@brief Cable length ranges (Result of DSP)
 */
/* the enum start with 1 to fit SNMP values */
typedef enum {
    MPD_CABLE_LENGTH_LESS_THAN_50M_E = 1,
    MPD_CABLE_LENGTH_50M_80M_E,
    MPD_CABLE_LENGTH_80M_110M_E,
    MPD_CABLE_LENGTH_110M_140M_E,
    MPD_CABLE_LENGTH_MORE_THAN_140M_E,
    MPD_CABLE_LENGTH_UNKNOWN_E,
} MPD_CABLE_LENGTH_ENT;
/** @} */

/** @internal
 *	@enum MPD_GIG_PHY_LED_OPERATION_ENT
 *	@brief Turn 1G Led On/Off
 */
typedef enum {
    MPD_GIG_PHY_LED_ON_E,
    MPD_GIG_PHY_LED_OFF_E,
    MPD_GIG_PHY_LED_UNKNOWN_E
} MPD_GIG_PHY_LED_OPERATION_ENT;


/**
* @addtogroup MDIX
* @{MPD API
* - Use \ref MPD_OP_CODE_SET_MDIX_E in order to set MDIX mode
* - Use \ref MPD_OP_CODE_GET_MDIX_E in order to retreive operational MDIX status
* - Use \ref MPD_OP_CODE_GET_MDIX_ADMIN_E in order to retreive MDIX administrative configuration
* @remark MPD_OP_CODE_GET_MDIX_E is relevant only when PHY link is up
*
* @struct  MPD_MDIX_MODE_PARAMS_STC
* @brief Used for
* - \ref MPD_OP_CODE_GET_MDIX_E
* - \ref MPD_OP_CODE_SET_MDIX_E
* - \ref MPD_OP_CODE_GET_MDIX_ADMIN_E
*/
typedef struct {
    /** @param [inout] mode - the MDI/X mode */
	MPD_MDIX_MODE_TYPE_ENT mode;
} MPD_MDIX_MODE_PARAMS_STC;
/** @} */

/** @addtogroup Autonegotiation
 * @{MPD API
 * - Use \ref MPD_OP_CODE_SET_AUTONEG_E in order to configure auto-negotiation
 * - Use \ref MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E in order to retrieve link partner capabilities
 * - Use \ref MPD_OP_CODE_SET_ADVERTISE_FC_E in order to configure flow control advertisement
 * - Use \ref MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E in order to retrieve link partner flow control advertised capabilities
 * - Use \ref MPD_OP_CODE_SET_RESTART_AUTONEG_E in order to manually trigger restart auto-negotiation on copper port
 * - Use \ref MPD_OP_CODE_GET_AUTONEG_ADMIN_E in order to get admin configuration of auto-negotiation
 * @struct  MPD_AUTONEG_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_AUTONEG_E
 * - \ref MPD_OP_CODE_GET_AUTONEG_ADMIN_E
 */
typedef struct {
    /**
     * @param [in] enable - Auto negotiation enable/disable
     */
    MPD_AUTO_NEGOTIATION_ENT     enable;
    /**
     * @param [in] capabilities - Auto negotiation advertised (speed & duplex) capabilities
     * @remark Relevant when \a enable is TRUE
     */
	MPD_AUTONEG_CAPABILITIES_TYP capabilities;
    /**
     * @param [in] masterSlave - Auto-negotiation Master/Slave preference (not force)
     * @remark Relevant when \a enable is TRUE
     */
    MPD_AUTONEGPREFERENCE_ENT    masterSlave;
}  MPD_AUTONEG_PARAMS_STC;
/** @} */

/** @addtogroup Duplex
 * @{MPD API Set Duplex mode
 * - Use MPD_OP_CODE_SET_DUPLEX_MODE_E in order to set duplex mode
 * @remark Half duplex is supported only for 10M & 100M speeds
 * @remark In order to force duplex, first disable auto-negotiation using MPD_OP_CODE_SET_AUTONEG_E
 * @struct  MPD_DUPLEX_MODE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_DUPLEX_MODE_E
 */
typedef struct {
    /**
     * @param [in] mode - Duplex mode (HALF/FULL)
     */
    MPD_DUPLEX_ADMIN_ENT mode;
} MPD_DUPLEX_MODE_PARAMS_STC;
/** @} */

/** @addtogroup VCT
 * @{MPD API Virtual cable test operations
 * - Use \ref MPD_OP_CODE_GET_VCT_CAPABILITY_E in order to get for which speeds VCT is supported
 * - Use \ref MPD_OP_CODE_SET_VCT_TEST_E in order to execute basic TDR, retreive result and cable length
 * - Use \ref MPD_OP_CODE_GET_EXT_VCT_PARAMS_E in order to execute advanced VCT (TDR & DSP) test (get polarity, sqew, impedance per pair)
 * - Use \ref MPD_OP_CODE_GET_CABLE_LEN_E in order to execute advanced VCT (TDR & DSP) test and get cable length (only)
 * @struct  MPD_VCT_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_VCT_TEST_E
 */
typedef struct {    
    /**
     * @param [out] testResult - VCT Test result
     */
    MPD_VCT_RESULT_ENT 	  testResult;
    /**
     * @param [out] cableLength - Cable length in meters
     */
    UINT_32               cableLength;
} MPD_VCT_PARAMS_STC;

/**
 * @struct  MPD_EXT_VCT_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_EXT_VCT_PARAMS_E
 */
typedef struct {
    /**
     * @param [in] testType - VCT Test data that is queried
     */
	MPD_VCT_TEST_TYPE_ENT   testType;
    /**
     * @param [out] result - Result according to test type
     */
	UINT_32                 result;
} MPD_EXT_VCT_PARAMS_STC;

/**
 * @struct  MPD_VCT_CAPABILITY_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_VCT_CAPABILITY_E
 */
typedef struct {
    /**
     * @param [out] vctSupportedSpeedsBitmap - Bit map of speeds that support VCT
     */
    MPD_SPEED_CAPABILITY_TYP vctSupportedSpeedsBitmap;
} MPD_VCT_CAPABILITY_PARAMS_STC;

/**
 * @struct  MPD_CABLE_LEN_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_CABLE_LEN_E
 */
typedef struct {
	/**
	 * @param [out] cableLength - When running DSP with link up without breaking link, VCT measured length in low resulution
	 */
	MPD_CABLE_LENGTH_ENT  cableLength;
	/**
	 * @param [out] accurateLength - Accurate cable length, when available
	 */
    UINT_32               accurateLength;
} MPD_CABLE_LEN_PARAMS_STC;
/**
 * @struct  MPD_CABLE_LEN_NO_RANGE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E
 */
typedef struct {
	/**
	 * @param [out] cableLen - Cable length in meters
	 */
    UINT_32 cableLen;
} MPD_CABLE_LEN_NO_RANGE_PARAMS_STC;
/** @} */


/** @addtogroup Autonegotiation
 * @{
 * @struct  MPD_AUTONEG_CAPABILITIES_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E
 * - \ref MPD_OP_CODE_GET_AUTONEG_SUPPORT_E
 */
typedef struct {
	/**
	 * @param [out] capabilities -
	 * When calling MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E: Link partner (detected) negotiation capabilities
	 * When calling MPD_OP_CODE_GET_AUTONEG_SUPPORT_E: Local supported negotiated capabilities
	 */
	MPD_AUTONEG_CAPABILITIES_TYP capabilities;
}  MPD_AUTONEG_CAPABILITIES_PARAMS_STC;

/**
 * @struct  MPD_ADVERTISE_FC_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_ADVERTISE_FC_E
 */
typedef struct {
	/**
	 * @param [in] advertiseFc - Advertise Flow control (requires auto-negotiation enabled)
	 */
	BOOLEAN advertiseFc;
} MPD_ADVERTISE_FC_PARAMS_STC;

/**
 * @struct  MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E
 */
typedef struct {
	/**
	 * @param [out] pauseCapable - Link partner (negotiated) pause (FC) ability
  	 */
    BOOLEAN pauseCapable;
	/**
	 * @param [out] asymetricRequested - Link partner (negotiated) requests asymetric pause
  	 */
    BOOLEAN	asymetricRequested;
} MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC;
/** @} */

/** @addtogroup Admin
 * @{MPD API Set PHY administrative mode
 * - Use \ref MPD_OP_CODE_SET_PHY_DISABLE_OPER_E in order to shutdown
 * @struct  MPD_PHY_DISABLE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_PHY_DISABLE_OPER_E
 */
typedef struct {
	/**
	 * @param [in] forceLinkDown - Force link down or unforce link down
	 */
    BOOLEAN forceLinkDown;
} MPD_PHY_DISABLE_PARAMS_STC;
/** @} */

/** @addtogroup Green
 * @{MPD API Green related configuration
 * - Use \ref MPD_OP_CODE_SET_POWER_MODULES_E in order to enable or disable
 * - Energy Detect (ED) feature
 * - Short Reach (SR) feature
 * - Use \ref MPD_OP_CODE_GET_GREEN_READINESS_E in order to get support type of Green features
 * - Use \ref MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E in order to get power consumption values
 * @remark Power consumption valuse are not measured, these are calculated based on static beforhand measurements
 * @struct MPD_POWER_MODULES_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_POWER_MODULES_E
 */
typedef struct {
	/**
	 * @param [in] energyDetetct - Enery Detect configuration to apply
  	 */
    MPD_GREEN_SET_ENT energyDetetct;
	/**
	 * @param [in] shortReach - Short Reach configuration to apply
  	 */
    MPD_GREEN_SET_ENT shortReach;
	/**
	 * @param [in] performPhyReset - Perform PHY reset
  	 */
    BOOLEAN           performPhyReset;
} MPD_POWER_MODULES_PARAMS_STC;

/**
 * @struct MPD_GREEN_READINESS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_GREEN_READINESS_E
 */
typedef struct {        
	/**
	 * @param [in] type - Green readiness (support) query type
  	 */
    MPD_GREEN_READINESS_TYPE_ENT type;
	/**
	 * @param [out] readiness - Support type
  	 */
    MPD_GREEN_READINESS_ENT      readiness;
	/**
	 * @param [out] srSpeeds - Bit map of speeds that the PHY support SR
  	 */
    MPD_SPEED_CAPABILITY_TYP     srSpeeds;
} MPD_GREEN_READINESS_PARAMS_STC;

/**
 * @struct MPD_GREEN_POW_CONSUMPTION_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E
 */
typedef struct {
	/**
	 * @param [in] energyDetetct - ED status
	 */
    MPD_GREEN_SET_ENT  	energyDetetct;
	/**
	 * @param [in] shortReach - SR Status
	 */
    MPD_GREEN_SET_ENT  	shortReach;
	/**
	 * @param [in] portUp - is port up
	 */
    BOOLEAN             portUp;
	/**
	 * @param [in] portSpeed - port speed
	 */
    MPD_SPEED_ENT		portSpeed;
	/**
	 * @param [in] getMax - if TRUE - return max power consumption
	 */
    BOOLEAN             getMax;
	/**
	 * @param [in] greenConsumption - PHY power consumption in mW
	 */
    UINT_32             greenConsumption;
} MPD_GREEN_POW_CONSUMPTION_PARAMS_STC;

/** @} */
/** @addtogroup Register_Access
 * @{MPD API that allow direct access to registers
 * - Use \ref MPD_OP_CODE_GET_I2C_READ_E in order to read using I2C from inserted module
 * @remark On supported PHYs, when I2C is connected to PHY
 * - Use \ref MPD_OP_CODE_SET_MDIO_ACCESS_E in order to read and write to an MDIO device
 * @struct MPD_I2C_READ_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_I2C_READ_E
 */
typedef struct {
	/**
	 * @param [in] address - I2C address
  	 */
    UINT_8 address;
	/**
	 * @param [in] offset - Register offset
  	 */
    UINT_8 offset;
	/**
	 * @param [in] length - Read length (1..4)
  	 */
    UINT_8 length;
	/**
	 * @param [out] value - Read value
  	 */
    UINT_8 value[4];
} MPD_I2C_READ_PARAMS_STC;

/**
 * @struct MPD_MDIO_ACCESS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_MDIO_ACCESS_E
 */
typedef struct {
	/**
	 * @param [in] type - Access type (read/write)
  	 */
    MPD_MDIO_ACCESS_TYPE_ENT             type;
	/**
	 * @param [in] device - Device Or Page within PHY
  	 */
    UINT_16                              deviceOrPage;
	/**
	 * @param [in] address - Register address/offset
  	 */
    UINT_16                              address;
	/**
	 * @param [inout] data - Register Value
  	 */
    UINT_16                              data;
} MPD_MDIO_ACCESS_PARAMS_STC;

/** @} */

/** @addtogroup EEE
 * @{MPD API  Energy Efficient Ethernet, 802.3az
 * - Use \ref MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E in order to configure EEE advertisement capabilities
 * - Use \ref MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E in order to enable and disable EEE
 * - Use \ref MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E in order to get remote (Advertised) status
 * - Use \ref MPD_OP_CODE_SET_LPI_EXIT_TIME_E in order to configure Low Power Idle (LPI) exit timer
 * - Use \ref MPD_OP_CODE_SET_LPI_ENTER_TIME_E in order to configure Low Power Idle (LPI) enter timer
 * - Use \ref MPD_OP_CODE_GET_EEE_CAPABILITY_E in order to get local EEE capability
 * @struct MPD_EEE_ADVERTISE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E
 */
typedef struct {
	/**
	 * @param [in] speedBitmap - Speeds on which to advertise EEE
  	 */
    MPD_SPEED_CAPABILITY_TYP    speedBitmap;
	/**
	 * @param [in] advEnable - Advertise EEE
  	 */
    BOOLEAN                     advEnable;
} MPD_EEE_ADVERTISE_PARAMS_STC;

/**
 * @struct MPD_EEE_MASTER_ENABLE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E
 */
typedef struct {
	/**
	 * @param [in] masterEnable - EEE Admin mode
  	 */
	BOOLEAN     masterEnable;
} MPD_EEE_MASTER_ENABLE_PARAMS_STC;

/**
 * @struct MPD_EEE_CAPABILITIES_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E
 * - \ref MPD_OP_CODE_GET_EEE_CAPABILITY_E
 */
typedef struct {
	/**
	 * @param [out] enableBitmap - EEE ability speed bitmap
	 */
	MPD_SPEED_CAPABILITY_TYP enableBitmap;
} MPD_EEE_CAPABILITIES_PARAMS_STC;

/**
 * @struct MPD_EEE_LPI_TIME_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_LPI_EXIT_TIME_E
 * - \ref MPD_OP_CODE_SET_LPI_ENTER_TIME_E
 */
typedef struct {
	/**
	 * @param [in] speed - speed on which to configure
	 */
    MPD_EEE_SPEED_ENT    speed;
	/**
	 * @param [in] time_us - time in microseconds
	 */
    UINT_16              time_us;
} MPD_EEE_LPI_TIME_PARAMS_STC;

/** @} */

/** @addtogroup MISC Miscelanious
 * @{MPD API - miscelanious
 * @struct MPD_DTE_STATUS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_DTE_STATUS_E
 */
typedef struct {
	/**
	 * @param [out] dteDetetcted - Is DTE detected
	 */
    BOOLEAN     dteDetetcted;
} MPD_DTE_STATUS_PARAMS_STC;

/**
 * @struct MPD_TEMPERATURE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_TEMPERATURE_E
 */
typedef struct {
	/**
	 * @param [out] temperature - in celcius
	 */
    UINT_16     temperature;
} MPD_TEMPERATURE_PARAMS_STC;

/**
 * @struct MPD_REVISION_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_REVISION_E
 */
typedef struct {
	/**
	 * @param [out] revision - PHY revision
	 */
    UINT_16     	revision;
	/**
	 * @param [out] phyType - PHY Type
	 */
    MPD_TYPE_ENT	phyType;
} MPD_REVISION_PARAMS_STC;
/** @} */

/** @addtogroup Speed
 * @{MPD API Configure PHY speed
 * - Use \ref MPD_OP_CODE_SET_SPEED_E in order to configure PHY speed
 * @remark in order to force speed first disable auto-negotiation using MPD_OP_CODE_SET_AUTONEG_E
 * @struct MPD_SPEED_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SPEED_E
 */
typedef struct {
	/**
	 * @param [in] speed - PHY speed to configure
	 */
    MPD_SPEED_ENT                	speed;

} MPD_SPEED_PARAMS_STC;
/** @} */

/** @addtogroup Fiber
 * @{MPD API Fiber related APIs
 * - Use \ref MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E in order to update PHY driver with SFP present (LoS) status
 * - Use \ref MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E in order to get operational media
 * @struct MPD_PRESENT_NOTIFICATION_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E
 */
typedef struct {
	/**
	 * @param [in] isSfpPresent - Is SFP signal (LOS) present
	 */
	BOOLEAN      		isSfpPresent;
	/**
	 * @param [in] opMode - type of media detected
	 */
    MPD_OP_MODE_ENT     opMode;
	/**
	 * @param [in] sfpWa - WA, avoid reconfiguration of MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E on specific PHYs
	 * @warning Need to remove
	 */
    BOOLEAN             sfpWa;
} MPD_PRESENT_NOTIFICATION_PARAMS_STC;


/**
 * @struct MPD_KIND_AND_MEDIA_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E
 */
typedef struct {
	/**
	 * @param [out] phyKind - Copper/Fiber/Combo
	 */
	MPD_KIND_ENT        			phyKind;
	/**
	 * @param [out] isSfpPresent - Is SFP signal present (DB)
	 */
	BOOLEAN                        	isSfpPresent;
	/**
	 * @param [out] mediaType - Active Media
	 */
	MPD_COMFIG_PHY_MEDIA_TYPE_ENT  	mediaType;
} MPD_KIND_AND_MEDIA_PARAMS_STC;
/** @} */

/** @addtogroup OperationalStatus Operational Status
 * @{MPD API Get operational status
 * - Use \ref MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E in order to retreive PHY operationla status
 * @struct MPD_INTERNAL_OPER_STATUS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E
 */
typedef struct {
	/**
	 * @param [out] isOperStatusUp - Is PHY operational UP
	 * @note All following fields are relevant only when this is TRUE
	 */
	BOOLEAN                         isOperStatusUp;
	/**
	 * @param [out] phySpeed - PHY operational speed
	 */
    MPD_SPEED_ENT        			phySpeed;
	/**
	 * @param [out] phySpeed - PHY operational duplex TRUE - full duplex, FALSE - half duplex
	 */
    BOOLEAN                         duplexMode;
	/**
	 * @param [out] phySpeed - Active media type
	 */
    MPD_COMFIG_PHY_MEDIA_TYPE_ENT   mediaType;
	/**
	 * @param [out] phySpeed - Is negotiation complete
	 */
    BOOLEAN                         isAnCompleted;
} MPD_INTERNAL_OPER_STATUS_STC;
/** @} */

/** @addtogroup FecMode
 * @{
 *      @enum MPD_FEC_MODE_SUPPORTED_ENT
 *	@brief Forward Error Correction modes
 */
typedef enum {

    /** FC-FEC enabled on port */
    MPD_FEC_MODE_ENABLED_E,

    /** FEC disabled on port */
    MPD_FEC_MODE_DISABLED_E,

    /** @brief Reed-Solomon (528,514) FEC mode
     */
    MPD_RS_FEC_MODE_ENABLED_E,

    /** @brief Used only for AP advertisment. Both FC and RS fec supported.
     */
    MPD_BOTH_FEC_MODE_ENABLED_E,

    /** @brief Reed-Solomon (544,514) FEC mode
     */
    MPD_RS_FEC_544_514_MODE_ENABLED_E,

    MPD_FEC_MODE_LAST_E
} MPD_FEC_MODE_SUPPORTED_ENT;
/** @} */

/** @addtogroup Speed_Extended
 * @{
 *      @enum MPD_INTERFACE_MODE_ENT
 *      @brief Interface mode
 */

typedef enum {
    MPD_INTERFACE_MODE_KR_E     = 16,
    MPD_INTERFACE_MODE_SR_LR_E  = 20,
    MPD_INTERFACE_MODE_KR2_E    = 27,
    MPD_INTERFACE_MODE_KR4_E    = 28,
    MPD_INTERFACE_MODE_SR_LR2_E = 29 ,
    MPD_INTERFACE_MODE_SR_LR4_E = 30,
    MPD_INTERFACE_MODE_CR_E     = 36,
    MPD_INTERFACE_MODE_CR2_E    = 37,
    MPD_INTERFACE_MODE_CR4_E    = 38,
    MPD_INTERFACE_MODE_KR8_E    = 41,
    MPD_INTERFACE_MODE_CR8_E    = 42,
    MPD_INTERFACE_MODE_SR_LR8_E = 43,
    MPD_INTERFACE_MODE_NA_E     = 56
}MPD_INTERFACE_MODE_ENT;

/**
 * @struct MPD_SPEED_PHY_SIDE_PARAMS_STC
 * @brief Used for Speed Extended configuration at line side and host side of PHY
 */
typedef struct{
    /**
     * @param [in] apEnable - Port auto-neg enable
     */
    BOOLEAN                         apEnable;
    /**
     * @param [in] speed - Port speed
     */
    MPD_SPEED_ENT                   speed;
    /**
     * @param [in] ifMode - Interface mode
     */
    MPD_INTERFACE_MODE_ENT          ifMode;
    /**
     * @param [in] fecMode - Port FEC mode at host/line side
     */
    MPD_FEC_MODE_SUPPORTED_ENT      fecMode;
}MPD_SPEED_PHY_SIDE_PARAMS_STC;

/**
 * MPD API Speed related APIs
 * - Use \ref MPD_OP_CODE_SET_SPEED_EXT_E in order to update PHY driver with extended speed params
 * - Use \ref MPD_OP_CODE_GET_SPEED_EXT_E in order to get speed and fec mode
 * @struct MPD_SPEED_EXT_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SPEED_EXT_E
 * - \ref MPD_OP_CODE_GET_SPEED_EXT_E
 */
typedef struct {
    /**
     * @param [in] isRetimerMode - ie retimer mode.
     */
    BOOLEAN                         isRetimerMode;

        /**
	 * @param [in] hostSide - Phy hostSide Port Params.
         */

    MPD_SPEED_PHY_SIDE_PARAMS_STC   hostSide;
	/**
	 * @param [in] lineSide - Phy lineSide Port Params.
	 */
    MPD_SPEED_PHY_SIDE_PARAMS_STC   lineSide;
}MPD_SPEED_EXT_PARAMS_STC;

/**
 * @enum MPD_PHY_HOST_LINE_ENT
 * @brief Used for Speed Extended configuration at line side and hot side of PHY
 */
typedef enum {
    /*@brief host side */
    MPD_PHY_SIDE_HOST_E,
    /*@brief line side */
    MPD_PHY_SIDE_LINE_E,
    MPD_PHY_SIDE_LAST_E
}MPD_PHY_HOST_LINE_ENT;
/** @} */

/** @addtogroup Serdes_Tune
 * @{
 *      @enum MPD_SERDES_PARAMS_TYPE_ENT
 *      @brief Port Serdes Rx/Tx params type.
 */
typedef enum {
    /*@brief Serdes tx side params */
    MPD_SERDES_PARAMS_TX_E,
    /*@brief Serdes rx side params */
    MPD_SERDES_PARAMS_RX_E,
    /*@brief Serdes tx and rx side params */
    MPD_SERDES_PARAMS_BOTH_E,
    MPD_SERDES_PARAMS_LAST_E
}MPD_SERDES_PARAMS_TYPE_ENT;

/**
 *      @struct MPD_SERDES_TX_CONFIG_STC
 *      @brief Port Serdes Tx params .
 */
typedef struct{
  /**
    * @param [in] atten - attenuation
    */
    UINT_16  atten;
  /**
    * @param [in] post - post cursor
    */
    UINT_16  post;
  /**
    * @param [in] pre - pre cursor
    */
    UINT_16  pre;
  /**
    * @param [in] pre2 - pre cursor2
    */
    UINT_16  pre2;
  /**
    * @param [in] pre3 - pre cursor3
    */
    UINT_16  pre3;

}MPD_SERDES_TX_CONFIG_STC;

/**
 *      @struct MPD_SERDES_RX_CONFIG_STC
 *      @brief Port Serdes Rx params .
 */
typedef struct{
    /*@brief dc  */
    UINT_16   dc;

    /*@brief low-frequency  */
    UINT_16   lf;

    /*@brief high-frequency  */
    UINT_16   hf;

    /*@brief bandwidth  */
    UINT_16   bw;

}MPD_SERDES_RX_CONFIG_STC;

/**
 *      @struct MPD_SERDES_TUNE_STC
 *      @brief Port Serdes Tune params .
 */
typedef struct {
    /** @brief paramsType -
     *  tx/rx/both
     */
    MPD_SERDES_PARAMS_TYPE_ENT paramsType;
/** @brief txTune -
     *  TX parameters
     */
    MPD_SERDES_TX_CONFIG_STC txTune;
    /** @brief rxTune -
     *  RX parameters
     */
    MPD_SERDES_RX_CONFIG_STC rxTune;
}MPD_SERDES_TUNE_STC;

/**
 * MPD API Serdes rx/tx tune  related APIs
 * - Use \ref MPD_OP_CODE_SET_SERDES_TUNE_E in order to update PHY driver with serdes rx/tx params
 * @struct MPD_SERDES_TUNE_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SERDES_TUNE_E
 */
typedef struct {
    /** @brief hostOrLineSide -
     * PHY side to configure
     */
    MPD_PHY_HOST_LINE_ENT            hostOrLineSide;
    /** @brief lanesBmp -
     * Port lane number
     */
    UINT_32                          lanesBmp;
    /** @brief tuneParams -
     * Serdes Tune params
     */
    MPD_SERDES_TUNE_STC              tuneParams;

}MPD_SERDES_TUNE_PARAMS_STC;
/** @} */

/** @addtogroup Serdes_Polarity
 * @{ MPD API Serdes rx/tx tune  related APIs
 * - Use \ref MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E in order to update PHY driver with serdes rx/tx polarity
 * @struct MPD_SERDES_LANE_POLARITY_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E
 */
typedef struct {
    /** @brief hostOrLineSide -
     * PHY side to configure
     */
    MPD_PHY_HOST_LINE_ENT            hostOrLineSide;
    /** @brief laneNum -
     * Port Lane number
     */
    UINT_32                          laneNum;
    /** @brief invertTx -
     * Port Lane polarity invert at Tx
     */
    BOOLEAN                          invertTx;
    /** @brief invertRx -
     * Port Lane polarity invert at Rx
     */
    BOOLEAN                          invertRx;

}MPD_SERDES_LANE_POLARITY_PARAMS_STC;
/** @} */

/* ******************* INTERNAL ****************/
/** @internal
 * @struct PRV_MPD_INITIALIZE_PHY_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_INIT_E
 */
typedef struct {
	/**
	 * @param [in] initializeDb - DB or HW initialization
	 */
    BOOLEAN initializeDb;
} PRV_MPD_INITIALIZE_PHY_STC;

/** @internal
 * @struct PRV_MPD_FIBER_MEDIA_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E
 * - PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E
 * @warning Internal - do not use
 */
typedef struct {
    /*  INPUT  */
    BOOLEAN             			fiberPresent;
    MPD_COMBO_MODE_ENT  			comboMode;
    BOOLEAN             			comboModeWa;
    MPD_SPEED_ENT       			phySpeed;/* todo - move to application */
    /*  INPUT / OUTPUT  */
    MPD_COMFIG_PHY_MEDIA_TYPE_ENT  	mediaType;
} PRV_MPD_FIBER_MEDIA_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_LOOP_BACK_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_LOOP_BACK_E
 */
typedef struct {
	/** @internal
	 * @param [in] enable - enable loopback
	 */
    BOOLEAN enable;
} PRV_MPD_LOOP_BACK_STC;

/** @internal
 * @struct  PRV_MPD_VCT_OFFSET_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_VCT_OFFSET_E
 * @warning - internal, do not use
 */
typedef struct {
    BOOLEAN do_set;
    UINT_32 offset;
} PRV_MPD_VCT_OFFSET_PARAMS_STC;

/** @internal
 * @struct MPD_ERRATAS_PARAMS_STC
 * @brief Used for
 * - \ref MPD_OP_CODE_SET_ERRATA_E
 * @warning Internal
 */
typedef struct {
    MPD_ERRATA_TYP erratasBitmap;
} PRV_MPD_ERRATAS_PARAMS_STC;

/** @internal
 * @struct MPD_FAST_LINK_DOWN_ENABLE_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E
 */
typedef struct {
	/**
	 * @param [in] enable - enable fast link down
	 */
    BOOLEAN     enable;
} PRV_MPD_FAST_LINK_DOWN_ENABLE_PARAMS_STC;

/** @internal
 * @struct MPD_EEE_MAX_TX_VAL_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E
 */
typedef struct {
	/**
	 * @param [out] maxTxVal_ARR - Maximum LPI Exit time per speed
	 */
    UINT_16     maxTxVal_ARR[MPD_EEE_SPEED_MAX_E];
} PRV_MPD_EEE_MAX_TX_VAL_PARAMS_STC;

/** @internal
 * @struct MPD_EEE_ENABLE_MODE_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E
 */
typedef struct {
	/**
	 * @param [out] enableMode - EEE enable mode
	 */
    MPD_EEE_ENABLE_MODE_ENT  enableMode;
} PRV_MPD_EEE_ENABLE_MODE_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_ACTTUAL_LINK_STATUS_PARAMS_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E
 */
typedef struct {
	/**
	 * @param [in] phy_validStatus - PHY valid link status
	 */
	BOOLEAN     phyValidStatus;
} PRV_MPD_ACTTUAL_LINK_STATUS_PARAMS_STC;

/** @internal
 * @struct PRV_MPD_PORT_LANE_BMP_STC
 * @brief Used for
 * - PRV_MPD_OP_CODE_GET_LANE_BMP_E
 */
typedef struct {
	UINT_32 hostSideLanesBmp;
	UINT_32 lineSideLanesBmp;

}PRV_MPD_PORT_LANE_BMP_STC;

/** @internal
 * @struct  PRV_MPD_PAGE_SELECT_PARAMS_STC
 * @brief Used for PHYs which support page select
 * - PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E
 * @warning - internal, do not use
 */
typedef struct {
    UINT_16   page;
    UINT_16   prevPage;
    BOOLEAN   readPrevPage;
} PRV_MPD_PAGE_SELECT_PARAMS_STC;
/* ******************* END INTERNAL ****************/

/**
* @addtogroup API API
* @{MPD API, all information required in order to perform PHY operations*
 * @union MPD_OPERATIONS_PARAMS_UNT
 * @brief Data union for logical operations
*/

typedef union {
	/**
	 * - \ref MPD_OP_CODE_GET_MDIX_E
	 * - \ref MPD_OP_CODE_SET_MDIX_E
	 * - \ref MPD_OP_CODE_GET_MDIX_ADMIN_E
	 */
    MPD_MDIX_MODE_PARAMS_STC                    phyMdix;
    /**
     * - \ref MPD_OP_CODE_SET_AUTONEG_E
     * - \ref MPD_OP_CODE_GET_AUTONEG_ADMIN_E
     */
    MPD_AUTONEG_PARAMS_STC                      phyAutoneg;
    /**
     * - \ref MPD_OP_CODE_SET_DUPLEX_MODE_E
     */
    MPD_DUPLEX_MODE_PARAMS_STC                  phyDuplex;
    /**
     * - \ref MPD_OP_CODE_SET_SPEED_E
     */
    MPD_SPEED_PARAMS_STC                        phySpeed;
    /**
     * - \ref MPD_OP_CODE_SET_VCT_TEST_E
     */
    MPD_VCT_PARAMS_STC                          phyVct;
    /**
     * - \ref MPD_OP_CODE_GET_EXT_VCT_PARAMS_E
     */
    MPD_EXT_VCT_PARAMS_STC                      phyExtVct;
    /**
     * - \ref MPD_OP_CODE_GET_CABLE_LEN_E
     */
    MPD_CABLE_LEN_PARAMS_STC                    phyCableLen;
    /**
     * - \ref MPD_OP_CODE_SET_PHY_DISABLE_OPER_E
     */
    MPD_PHY_DISABLE_PARAMS_STC                  phyDisable;
    /**
     * - \ref MPD_OP_CODE_GET_AUTONEG_REMOTE_CAPABILITIES_E
     */
    MPD_AUTONEG_CAPABILITIES_PARAMS_STC  		phyRemoteAutoneg;
    /**
     * - \ref MPD_OP_CODE_GET_AUTONEG_SUPPORT_E
     */
    MPD_AUTONEG_CAPABILITIES_PARAMS_STC			PhyAutonegCapabilities;
    /**
     * - \ref MPD_OP_CODE_SET_ADVERTISE_FC_E
     */
    MPD_ADVERTISE_FC_PARAMS_STC                 phyFc;
    /**
     * - \ref MPD_OP_CODE_GET_LINK_PARTNER_PAUSE_CAPABLE_E
     */
    MPD_PARTNER_PAUSE_CAPABLE_PARAMS_STC        phyLinkPartnerFc;
    /**
     * - \ref MPD_OP_CODE_SET_POWER_MODULES_E
     */
    MPD_POWER_MODULES_PARAMS_STC                phyPowerModules;
    /**
     * - \ref MPD_OP_CODE_GET_GREEN_POW_CONSUMPTION_E
     */
    MPD_GREEN_POW_CONSUMPTION_PARAMS_STC        phyPowerConsumptions;
    /**
     * - \ref MPD_OP_CODE_GET_GREEN_READINESS_E
     */
    MPD_GREEN_READINESS_PARAMS_STC              phyGreen;
    /**
     * - \ref MPD_OP_CODE_GET_CABLE_LEN_NO_RANGE_E
     */
    MPD_CABLE_LEN_NO_RANGE_PARAMS_STC           phyCableLenNoRange;
    /**
     * - \ref MPD_OP_CODE_GET_PHY_KIND_AND_MEDIA_E
     */
    MPD_KIND_AND_MEDIA_PARAMS_STC               phyKindAndMedia;
    /**
     * - \ref MPD_OP_CODE_GET_I2C_READ_E
     */
    MPD_I2C_READ_PARAMS_STC                     phyI2cRead;
    /**
     * - \ref MPD_OP_CODE_SET_PRESENT_NOTIFICATION_E
     */
    MPD_PRESENT_NOTIFICATION_PARAMS_STC         phySfpPresentNotification;
    /**
     * - \ref MPD_OP_CODE_SET_MDIO_ACCESS_E
     */
    MPD_MDIO_ACCESS_PARAMS_STC                  phyMdioAccess;
    /**
     * - \ref MPD_OP_CODE_SET_EEE_ADV_CAPABILITY_E
     */
    MPD_EEE_ADVERTISE_PARAMS_STC                phyEeeAdvertize;
    /**
     * - \ref MPD_OP_CODE_SET_EEE_MASTER_ENABLE_E
     */
    MPD_EEE_MASTER_ENABLE_PARAMS_STC            phyEeeMasterEnable;
    /**
     * - \ref MPD_OP_CODE_GET_EEE_REMOTE_STATUS_E
     * - \ref MPD_OP_CODE_GET_EEE_CAPABILITY_E
     */
    MPD_EEE_CAPABILITIES_PARAMS_STC             phyEeeCapabilities;
    /**
     * - \ref MPD_OP_CODE_SET_LPI_EXIT_TIME_E
     * - \ref MPD_OP_CODE_SET_LPI_ENTER_TIME_E
     */
    MPD_EEE_LPI_TIME_PARAMS_STC                 phyEeeLpiTime;
    /**
     * - \ref MPD_OP_CODE_GET_INTERNAL_OPER_STATUS_E
     */
    MPD_INTERNAL_OPER_STATUS_STC                phyInternalOperStatus;
    /**
     * - \ref MPD_OP_CODE_GET_VCT_CAPABILITY_E
     */
    MPD_VCT_CAPABILITY_PARAMS_STC               phyVctCapab;
    /**
     * - \ref MPD_OP_CODE_GET_DTE_STATUS_E
     */
    MPD_DTE_STATUS_PARAMS_STC                   phyDteStatus;
    /**
     * - \ref MPD_OP_CODE_GET_TEMPERATURE_E
     */
    MPD_TEMPERATURE_PARAMS_STC                  phyTemperature;
    /**
     * - \ref MPD_OP_CODE_GET_REVISION_E
     */
    MPD_REVISION_PARAMS_STC			phyRevision;
    /**
     * - \ref MPD_OP_CODE_SET_SPEED_EXT_E
     * - \ref MPD_OP_CODE_GET_SPEED_EXT_E
     */
    MPD_SPEED_EXT_PARAMS_STC                    phySpeedExt;
    /**
     * - \ref MPD_OP_CODE_SET_SERDES_TUNE_E
     */
    MPD_SERDES_TUNE_PARAMS_STC                  phyTune;
    /**
     * - \ref MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E
     */
    MPD_SERDES_LANE_POLARITY_PARAMS_STC         phySerdesPolarity;
	/**
	 * @internal
	 * @brief - internal operations, implemented per PHY
	 *  but we expect these to be called only from withing MPD
	 */
	union {
		/**
		 * @internal
		 * @brief - Peform PHY initialization sequence.
		 * Called per port
		 */
		PRV_MPD_INITIALIZE_PHY_STC                   phyInit;
		PRV_MPD_FIBER_MEDIA_PARAMS_STC               phyFiberParams;
		PRV_MPD_VCT_OFFSET_PARAMS_STC                phyVctOffset;
		PRV_MPD_LOOP_BACK_STC                         phyLoopback;
		PRV_MPD_ERRATAS_PARAMS_STC                   phyErrata;
	    PRV_MPD_FAST_LINK_DOWN_ENABLE_PARAMS_STC   phyFastLinkDown;
	    PRV_MPD_EEE_MAX_TX_VAL_PARAMS_STC           phyEeeMaxTxVal;
	    PRV_MPD_EEE_ENABLE_MODE_PARAMS_STC          phyEeeEnableMode;
	    PRV_MPD_ACTTUAL_LINK_STATUS_PARAMS_STC     phyLinkStatus;
        PRV_MPD_PORT_LANE_BMP_STC                    phyLaneBmp;
        PRV_MPD_PAGE_SELECT_PARAMS_STC              phyPageSelect;
	} internal;
} MPD_OPERATIONS_PARAMS_UNT;



/**
 * @brief Perform PHY operation
 * @param [in] 		rel_ifIndex - The port on which to perform operation
 * @param [in] 		op			- The logical operation
 * @param [inout]	params_PTR	- Data for operation (Set & Get)
 * @return MPD_RESULT_ENT
 */

extern MPD_RESULT_ENT mpdPerformPhyOperation(
    IN UINT_32          			rel_ifIndex,
    IN MPD_OP_CODE_ENT  		 	op,
    INOUT MPD_OPERATIONS_PARAMS_UNT * params_PTR
);
/** @}*/

#endif /* MPD_API_H_ */

/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/**
 * @file mpdPrv.h
 * @brief private (MPD internal) types and declarations
 *
 */

#ifndef PRV_MPD_H_
#define PRV_MPD_H_
 
#include <mpdTypes.h>
#include <mpd.h>
#include <mpdApi.h>
#include <mtdApiTypes.h>
#include <mtdFwDownload.h>
#include <mydApiTypes.h>
#include <mydDiagnostics.h>
#include <mtdDiagnosticsRegDumpData.h>

typedef unsigned int    GT_STATUS;

#ifndef PHY_SIMULATION
#define PRV_MPD_CHECK_RETURN_STATUS_MAC(_Status, _rel_ifIndex)\
	if (_Status != MPD_OK_E) { \
    	PRV_MPD_HANDLE_FAILURE_MAC(_rel_ifIndex, MPD_ERROR_SEVERITY_ERROR_E, "Check Status");\
    	return _Status; \
    }
#else
#define PRV_MPD_CHECK_RETURN_STATUS_MAC(_Status, _rel_ifIndex)\
	MPD_UNUSED_PARAM(_Status)
#endif

/***********  DEBUG ***************** */
typedef struct MTD_reg_debug_STCT
{
    char * description_STR;	/* register description */
    UINT_32 numOfRegs;		/* num of registers to read */
    const MTD_MDIO_ADDR_TYPE *regs_ARR; /* array of registers */
} MTD_reg_debug_STC;

/* ******** PORT LIST RELATED *********/

#define PRV_MPD_PORT_LIST_REL_IFINDEX_IS_LEGAL_MAC(relIf_)\
    (((relIf_ > 0) && (relIf_ < ((PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS) * 32)))?TRUE:FALSE)

#define PRV_MPD_PORT_LIST_GET_SHIFT_AND_WORD_MAC(relIf_, shift_, word_)\
    word_ = ((relIf_ - 1) / 32);\
    shift_ = ((relIf_-1) % 32)

#define PRV_MPD_PORT_LIST_WORD_AND_BIT_TO_REL_IFINDEX_MAC(word_, bit_)\
    ((word_ * 32) + bit_ + 1)

/* number of words in ports bitmap  */
#define PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS	(2)
#define PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS  ((MPD_MAX_PORT_NUMBER_CNS + 31)/32)

#define PRV_MPD_VALID_PHY_TYPE(_phy_type)	(_phy_type < MPD_TYPE_NUM_OF_TYPES_E)

#define PRV_MPD_MAX_MDIO_ADRESS_CNS	(31)

typedef struct {
    UINT_32 portsList[PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS];
} PRV_MPD_PORT_LIST_STC;

typedef PRV_MPD_PORT_LIST_STC PRV_MPD_PORT_LIST_TYP;


/* ********* PORT DB *************/
typedef enum {
    PRV_MPD_MAC_ON_PHY_NO_BYPASS_E = FALSE,
    PRV_MPD_MAC_ON_PHY_BYPASS_E	   = TRUE,
    PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E
} PRV_MPD_MAC_ON_PHY_ENT;

typedef struct {
    MPD_PHY_USX_TYPE_ENT    usxType;
    MTD_U16                 masterMacType;
    MTD_U16                 slaveMacType;
}PRV_MPD_USX_MAC_TYPE_STC;

typedef struct {
	BOOLEAN							sfpPresent;
    MPD_COMBO_MODE_ENT 				comboMode;
    MPD_SPEED_ENT               	speed;
    MPD_OP_MODE_ENT					opMode;
    PRV_MPD_MAC_ON_PHY_ENT          macOnPhyState; /* mac on phy bypass state */
    INT_32                          vct_offset;    /** @brief PHY VCT offset to get cable length. */
    MPD_PHY_VCT_INFO_STC            vctInfo;       /** @brief PHY VCT information. */
	MPD_PORT_ADMIN_ENT              adminMode; /* Port Admin mode */
    /** @brief PHY FW information. This information may be shared among several ports, 
        (for example, ports in same PHY, or same PHY type and and same bus).
        This field is updated internally by MPD.
    */
	MPD_PHY_FW_PARAMETERS_STC * phyFw_PTR;
    /** @brief part of PHY fw flash download mode to know which port need to be downloaded. */
    BOOLEAN                     isRepresentative; 
} PRV_MPD_RUNNING_DB_STC;

/* port entry */
typedef struct {
    UINT_32							rel_ifIndex;
    MPD_PORT_INIT_DB_STC  	    *	initData_PTR;
    PRV_MPD_RUNNING_DB_STC		*	runningData_PTR;
} PRV_MPD_PORT_HASH_ENTRY_STC;

typedef struct {
    UINT_32                     numOfMembers; 	/* number of entries which hold valid data in porthash*/
    PRV_MPD_PORT_LIST_TYP  		validPorts;    	/* indicates which rel_ifIndexes hold valid data (helps speeding up search in get_next_rel_ifIndex */
    MPD_FW_DOWNLOAD_TYPE_ENT	fwDownloadType_ARR[MPD_TYPE_NUM_OF_TYPES_E];	/* assumption - Download type detemined by phy type */
    PRV_MPD_PORT_HASH_ENTRY_STC entries[MPD_MAX_PORT_NUMBER_CNS]; /* array of entries */
} PRV_MPD_GLOBAL_DB_STC;


typedef MPD_RESULT_ENT MPD_OPERATIONS_FUN(
  /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
  /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR
  /*     OUTPUTS:            */
);


typedef struct {
    MPD_OPERATIONS_FUN  	* phyOperation_PTR;
    char                    * funcName_PTR;
} PRV_MPD_FUNC_INFO_ENTRY_STC;

/**
 * @internal mpdDriverInitDb function
 * @endinternal
 *
 * @brief   bind function
 * 			connect betweem PHY type & logical operation to PHY (type specific) implementation
 *
 * @param [in ]   	phyType   		- The PHY type
 * @param [in ]   	opType			- The logical operation
 * @param [in ]   	func_PTR		- The implementation of <opType> for <phyType>
 * @param [in ]		func_text_PTR	- Function name, for debug (may be NULL)
 *
 * @return MPD_RESULT_ENT
 */
extern BOOLEAN prvMpdBind (
    IN MPD_TYPE_ENT           phyType,
	IN MPD_OP_CODE_ENT        opType,
	IN MPD_OPERATIONS_FUN   * func_PTR,
	IN char                 * func_text_PTR
);

extern MPD_RESULT_ENT prvMpdBindInitFunc(
    /*     INPUTS:             */
    MPD_TYPE_ENT phyType
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern char * prvMpdGetPhyOperationName(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT              op
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdReadLinkCryptReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_32    * data_PTR
);

extern MPD_RESULT_ENT prvMpdWriteLinkCryptReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_16 address,
    UINT_32 data
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);


#define  MPD_ID_HIGH_ALASKA_88E1340_CNS      0x0141

#define  MPD_ID_LOW_ALASKA_88E1340_CNS       0x0C90

#define  MPD_ID_MASK_ALASKA_88E1145_CNS      0xFFFFFFF0

/* alaska: Phy */
#define  PRV_MPD_CTRLREG_OFFSET_CNS				0
#define  PRV_MPD_CTRLREG_FULL_DPLX_CNS			0x0100
#define  PRV_MPD_AUTO_NEGOTIATION_ENABLE_CNS	0x1000
#define  PRV_MPD_CTRLREG_100BT_CNS				0x2000
#define  PRV_MPD_CTRLREG_SPEED_1000BT_CNS		0x0040
#define  PRV_MPD_CTRLREG_RESET_BIT_CNS			0x8000
#define  PRV_MPD_CTRLREG_POWER_DOWN_CNS			0x0800   /* bit 0.11 */
#define  PRV_MPD_PORT_CNTRL_REG_SPEED_10M_CNS	0x0000
#define  PRV_MPD_PORT_CNTRL_REG_SPEED_100M_CNS	0x2000
#define  PRV_MPD_CTRLREG_EN_AUTO_NEG_CNS		0x1000
#define  PRV_MPD_CTRLREG_AUTO_NEG_CNS			0x1200
#define  PRV_MPD_CTRLREG_RESATRT_AUTO_NEG_CNS	0x0200
#define  PRV_MPD_CTRLREG_AMD_COPPER_100FX_CNS	0x0040

#define  PRV_MPD_COPPER_GIG_PHY_IDENTIFIER_CNS   0x3

#define  PRV_MPD_PAGE_SELECT_ADDRESS_CNS        22
#define  PRV_MPD_PAGE0_ADDRESSREG_OFFSET_CNS    0x00
#define  PRV_MPD_PAGE1_ADDRESSREG_OFFSET_CNS    0x01

#define  PRV_MPD_CTRLREG1_OFFSET_CNS             1
#define  PRV_MPD_COPPER_STATUS_REGISTER_CNS      1
#define  PRV_MPD_COPPER_AUTONEG_COMPLETE_CNS	 0x20

#define  PRV_MPD_LINK_PARTNER_ABILITY_REG_CNS          5
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100F_CNS  0x0100
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_100H_CNS  0x0080
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10F_CNS   0x0040
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_10H_CNS   0x0020
#define  PRV_MPD_LINK_PARTNER_FIBER_ABILITY_1000H_CNS  0x0040
#define  PRV_MPD_LINK_PARTNER_FIBER_ABILITY_1000F_CNS  0x0020

#define  PRV_MPD_1000_BASE_T_STATUS_REG_CNS            10
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_1000F_CNS 0x0800
#define  PRV_MPD_LINK_PARTNER_COPPER_ABILITY_1000H_CNS 0x0400

/* Phy Control Register */
#define  PRV_MPD_SPECIFIC_CONTROL_REG_CNS       16
#define  PRV_MPD_SPECIFIC_CONTROL_REG_3_CNS     26
/* Phy Specific Control Register -> MDI/MDIX Crossover */
#define  PRV_MPD_MDI_CROSSOVER_MODE_MASK_CNS    0x0060
#define  PRV_MPD_MANUAL_MDI_CONFIGURATION_CNS   0x0000
#define  PRV_MPD_MANUAL_MDIX_CONFIGURATION_CNS  0x0020
#define  PRV_MPD_AUTOMATIC_CROSSOVER_CNS        0x0060
/*values for the 3081/3083 phys */
#define  PRV_MPD_FE_MDI_CROSSOVER_MODE_MASK_CNS        0x0030
#define  PRV_MPD_FE_MANUAL_MDIX_CONFIGURATION_CNS      0x0000
#define  PRV_MPD_FE_MANUAL_MDI_CONFIGURATION_CNS       0x0010
#define  PRV_MPD_FE_AUTOMATIC_CROSSOVER_10_CNS         0x0020
#define  PRV_MPD_FE_AUTOMATIC_CROSSOVER_11_CNS         0x0030
#define  PRV_MPD_FE_AUTOMATIC_CROSSOVER_MAIN_BIT_CNS   0x0020

#define  PRV_MPD_SPECIFIC_STATUS_REG_CNS        17
/* PHY_Specific_Status_Reg */
#define  PRV_MPD_DTE_POWER_STATUS_STATUS_CNS    0x0004
#define  PRV_MPD_MDI_CROSSOVER_STATUS_CNS       0x0040
#define  PRV_MPD_PORT_STATUS_LINK_MASK_CNS      0x0400
#define  PRV_MPD_PORT_STATUS_DPLX_MOD_MASK_CNS  0x2000

#define  PRV_MPD_PORT_STATUS_SPEED_AND_DUPLEX_RESOLVED_CNS 0x0800
#define  PRV_MPD_PORT_STATUS_SPEED_10M_CNS      0x0000
#define  PRV_MPD_PORT_STATUS_SPEED_100M_CNS     0x4000
#define  PRV_MPD_PORT_STATUS_SPEED_1000M_CNS    0x8000

/* Extended PHY Specific Control*/
#define  PRV_MPD_AUTNEGADVER_OFFSET_CNS         0x0004
#define  PRV_MPD_ADVERTISE_BITS_CNS             0x01e1
#define  PRV_MPD_ADVERTISE_10_100_CNS           0x01e0
#define  PRV_MPD_FIBER_ADVERTISE_1000H_CNS      0x0040 /* 1000 half  */
#define  PRV_MPD_FIBER_ADVERTISE_CNS            0x0020 /* 1000 full only */
#define  PRV_MPD_ADVERTISE_100_T_FULL_CNS       0x0100
#define  PRV_MPD_ADVERTISE_100_T_HALF_CNS       0x0080
#define  PRV_MPD_ADVERTISE_10_T_FULL_CNS        0x0040
#define  PRV_MPD_ADVERTISE_10_T_HALF_CNS        0x0020

#define  PRV_MPD_CTRLREG_AUTONEG_MASK_CNS       0x1000


#define  PRV_MPD_1000BASE_T_CONTROL_OFFSET_CNS  0x9
#define  PRV_MPD_1000BASE_AN_PREFER_MASK_CNS    0x1700
#define  PRV_MPD_ADVERTISE_1000_T_MASK_CNS      0x0300
#define  PRV_MPD_1000BASE_AN_MANUALCONFIG_CNS   0x1000
#define  PRV_MPD_1000BASE_AN_PREFERMASTER_CNS   0x0400
#define  PRV_MPD_ADVERTISE_1000_T_FULL_CNS      0x0200
#define  PRV_MPD_ADVERTISE_1000_T_HALF_CNS      0x0100

#define  PRV_MPD_LINK_UP_CNS                           0xf7ff
#define  PRV_MPD_LINK_DOWN_CNS                         0x0800


/* Giga phy uses register 26 and 27 for the vct activation and results */
#define  PRV_MPD_VCT_2_PAIR_CABLE_CNS                     2
#define  PRV_MPD_VCT_4_PAIR_CABLE_CNS                     4


/* Phy 1240 */
#define  PRV_MPD_88E1240_REVISION_S0_CNS                      0x0E30
#define  PRV_MPD_88E1240_PAGE_ADDRESS_OFFSET_CNS              22
#define  PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS               0x300

/* Phy 151x */
#define  PRV_MPD_88E151X_PAGE_LED_CTRL_CNS                    0x3
#define  PRV_MPD_88E151X_LED_CTRL_REG_CNS                     16

/* green ethernet energy detect mode */
/*#define  PRV_MPD_88E1240_ENERGY_DETECT_MASK_CNS   0x0300* see line 264*/
#define  PRV_MPD_88E1240_ENERGY_DETECT_ON_CNS          0x0300
#define  PRV_MPD_88E1240_ENERGY_DETECT_OFF_CNS         0x0000
#define  PRV_MPD_SR_SPECIFIC_CONTROL_REG_CNS    24
#define  PRV_MPD_SR_SPECIFIC_CONTROL1_REG_CNS   23
#define  PRV_MPD_SR_SPECIFIC_CONTROL2_REG_CNS   25
#define  PRV_MPD_SR_SPECIFIC_CONTROL1_REG_SET_VALUE_CNS    0x2002
#define  PRV_MPD_SR_SPECIFIC_CONTROL1_REG_GET_VALUE_CNS    0x1002
#define  PRV_MPD_88E1240_SHORT_REACH_ON_MASK_CNS           0x0100

/* green ethernet mac interface effect on low power mode */
#define  PRV_MPD_88E1240_MAC_INTERFACE_POW_MASK_CNS   0x0008
#define  PRV_MPD_88E1240_MAC_INTERFACE_POW_OFF_CNS    0x0008
#define  PRV_MPD_88E1240_MAC_INTERFACE_POW_ON_CNS     0x0000


#define PRV_MPD_MDIO_WRITE_ALL_MASK_CNS                  0xFFFF
#define PRV_MPD_IGNORE_PAGE_CNS                  		0xFFFF
#define PRV_MPD_PORT_GROUP_UNAWARE_MODE_CNS             0xFFFFFFFF

/*Phy 3083 */

/* green ethernet energy detect mode */
#define  PRV_MPD_88E3083_ENERGY_DETECT_MASK_CNS               0x4000
#define  PRV_MPD_88E3083_ENERGY_DETECT_ON_CNS                 0x4000
#define  PRV_MPD_88E3083_ENERGY_DETECT_OFF_CNS                0x0000

/* EEE on phy 88E1540 */
#define  PRV_MPD_88E1540_BUFF_SIZE_KBITS_CNS                160
#define  PRV_MPD_88E1540_MAX_EXIT_LPI_TIMER_VAL             255
#define  PRV_MPD_EEE_MASTER_ENABLE_SLEEP_CNS                100

/* phy 1543 general control register */
#define  PRV_MPD_88E1543_GENERAL_CONTROL_REGISTER_CNS          18

#define  PRV_MPD_88E1543_REVISION_A0_CNS                       0x0EA1
#define  PRV_MPD_88E1543_REVISION_A1_CNS                       0x0EA2

/* this values refer to phy mode configuration page 20 register 6 bits [0..2]
*  see Marvell\AE Alaska\AE 88E1340/88E1322 Auto-Media Detect - MV-S301681-00A 4.1.1
*/
#define PRV_MPD_MODE_SGMII_TO_100_FX_CNS                               3
#define PRV_MPD_MODE_SGMII_TO_AUTO_MEDIA_COPPER_1000_BASEX_CNS         7
#define PRV_MPD_MODE_MASK_CNS                                          7
#define PRV_MPD_PAGE_20_MEDIA_SELECT_CONTROL_CNS                       20
#define PRV_MPD_PAGE_0_COPPER_CONTROL_CNS                              0
#define PRV_MPD_PAGE_1_FIBER_CONTROL_CNS                               1
#define PRV_MPD_GENERAL_CONTROL_REGISTER_PAGE_0_1_CNS                  0


/* ****** 3140 (SolarFlare) ***************/
#define PRV_MPD_3140_EEE_LPI_EXIT_TIME_CNS                              5 /* equal to NETP_green_eee_min_tx_10GBaseT_CNS --> Standard: 4.48  uSec (case 2) */


#ifdef PHY_SIMULATION
    #define prvMpd_autoneg_num_of_retries_CNS                1
#else
    #define prvMpd_autoneg_num_of_retries_CNS               10
#endif

/* page use to access linkCrypt registers*/
#define PRV_MPD_LINKCRYPT_INDIRECT_ACCESS_PAGE_CNS 16
/* register to which indirect read address is specified */
#define PRV_MPD_LINKCRYPT_READ_ADDRESS_REG_CNS    0
/* register to which indirect write address is specified */
#define PRV_MPD_LINKCRYPT_WRITE_ADDRESS_REG_CNS   1
/* register to which data low is written/read from */
#define PRV_MPD_LINKCRYPT_DATA_LOW_REG_CNS        2
/* register to which data high is written/read from */
#define PRV_MPD_LINKCRYPT_DATA_HIGH_REG_CNS       3

/* page use to access XMDIO registers*/
#define PRV_MPD_MMD_INDIRECT_ACCESS_PAGE_CNS      0
/* register to which the function (address/data) and to which device */
#define PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS   13
/* register to which the address/data is written to */
#define PRV_MPD_MMD_ADDRESS_DATA_REG_CNS          14

#define PRV_MPD_MMD_DEVICE_MASK_CNS               0x1F
#define PRV_MPD_MMD_FUNCTION_ADDRESS_CNS          0x0000
#define PRV_MPD_MMD_FUNCTION_DATA_CNS             0x4000

#define PRV_MPD_MAX_NUM_OF_SLICES_CNS             4

#define PRV_MPD_1540_SLICE_0_OFFSET_CNS           0x000   /*/< Port 0 registers/memory */
#define PRV_MPD_1540_SLICE_1_OFFSET_CNS           0x800   /*/< Port 1 registers/memory */
#define PRV_MPD_1540_SLICE_2_OFFSET_CNS           0x1000  /*/< Port 2 registers/memory */
#define PRV_MPD_1540_SLICE_3_OFFSET_CNS           0x1800  /*/< Port 3 registers/memory */

#define PRV_MPD_1540_BYPASS_DISABLE_BIT_CNS       0x2000 /* (1<<13) */

/* Definitions for BIST Status Register 1.C00C of 88E32x0/88E33x0*/
#define PRV_MPD_BIST_ABNOMINAL_RESET_POS 	 	(0)
#define PRV_MPD_BIST_CHECKSUM_EXCEEDED_POS 	(1)
#define PRV_MPD_BIST_PMA_FAILED_POS 	     	(2)
#define PRV_MPD_BIST_ENX_FAILED_POS 		 	(3)
#define PRV_MPD_BIST_RTOS_FAILED_POS 		 	(4)
#define PRV_MPD_BIST_SW_ERROR_POS 			(5)
#define PRV_MPD_BIST_FW_ECC_ERROR_POS 		(6)
#define PRV_MPD_BIST_WAIT_LOW_POWER_CLEAR_POS (7)
#define PRV_MPD_BIST_RESERVED_POS 			(8)
#define PRV_MPD_BIST_DEVICE_NOT_SUPORTED_POS  (9)

#define PRV_MPD_BIST_ABNOMINAL_RESET_MASK			(0x1<<PRV_MPD_BIST_ABNOMINAL_RESET_POS)
#define PRV_MPD_BIST_CHECKSUM_EXCEEDED_MASK 		(0x1<<PRV_MPD_BIST_CHECKSUM_EXCEEDED_POS)
#define PRV_MPD_BIST_PMA_FAILED_MASK 	     		(0x1<<PRV_MPD_BIST_PMA_FAILED_POS)
#define PRV_MPD_BIST_ENX_FAILED_MASK 		 		(0x1<<PRV_MPD_BIST_ENX_FAILED_POS)
#define PRV_MPD_BIST_RTOS_FAILED_MASK 			(0x1<<PRV_MPD_BIST_RTOS_FAILED_POS)
#define PRV_MPD_BIST_SW_ERROR_MASK 				(0x1<<PRV_MPD_BIST_SW_ERROR_POS)
#define PRV_MPD_BIST_FW_ECC_ERROR_MASK 			(0x1<<PRV_MPD_BIST_FW_ECC_ERROR_POS)
#define PRV_MPD_BIST_WAIT_LOW_POWER_CLEAR_MASK 	(0x1<<PRV_MPD_BIST_WAIT_LOW_POWER_CLEAR_POS)
#define PRV_MPD_BIST_RESERVED_MASK 				(0x1<<PRV_MPD_BIST_RESERVED_POS)
#define PRV_MPD_BIST_DEVICE_NOT_SUPORTED_MASK 	(0x1<<PRV_MPD_BIST_DEVICE_NOT_SUPORTED_POS)

#define PRV_MPD_BIST_PASSED_MASK					(0x3FFF)

/* VCT result */
#define PRV_MPD_vctResult_invalid_CNS            0 /* invalid or incomplete test */
#define PRV_MPD_vctResult_pairOk_CNS             1 /* pair okay, no fault */
#define PRV_MPD_vctResult_pairOpen_CNS           2 /* pair open */
#define PRV_MPD_vctResult_intraPairShort_CNS     3 /* intra pair short */
#define PRV_MPD_vctResult_interPairShort_CNS     4 /* inter pair short */

#define PRV_MPD_master_manual_mask_CNS            0x8000
#define PRV_MPD_master_preferred_mask_CNS         0x2000

#define PRV_MPD_3240_REVISION_NUM_Z2_CNS      1 /* PHY 3240 Revision Z2 number */
#define PRV_MPD_fw_version_num_offset_CNS         0x160 /* offset of version number after the header */
#define PRV_MPD_fw_3240_version_num_offset_CNS    0x120 /* offset of version number after the header */

#define PRV_MPD_MTD_MASTER_PREFERENCE_MASK_CNS		0xA000
#define	PRV_MPD_MTD_MASTER_PREFERENCE_MASTER_CNS	0x2000
#define	PRV_MPD_MTD_MASTER_PREFERENCE_SLAVE_CNS		0


#define PRV_MPD_TO_STRING_MAC(_val) #_val

#define PRV_MPD_BIND_MAC(PhyType, _op, _func_PTR)         \
		prvMpdBind( PhyType,                                \
                     _op,                                      \
                     _func_PTR,                                \
                     PRV_MPD_TO_STRING_MAC(_func_PTR))


#define PRV_MPD_DEBUG_BIND_MAC(_comp, _pkg, _flag, _help, _flagId)\
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.debug_bind_PTR != NULL))? \
	prvMpdCallBacks.debug_bind_PTR(_comp, _pkg, _flag, _help, _flagId):\
	FALSE)

#define PRV_MPD_IS_VCT_SUPPORTED_MAC(_rel_ifIndex, _supported_PTR)\
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.vct_supported_PTR != NULL))? \
	prvMpdCallBacks.vct_supported_PTR(_rel_ifIndex, _supported_PTR):\
	FALSE)


#define PRV_MPD_ALLOC_MAC(_alloc_size)\
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.alloc_PTR != NULL))? \
	prvMpdCallBacks.alloc_PTR(_alloc_size):0)

#define PRV_MPD_REALLOC_MAC(_ptr, _alloc_size)\
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.realloc_PTR != NULL))? \
	prvMpdCallBacks.realloc_PTR(_ptr, _alloc_size):0)

#define PRV_MPD_CALLOC_MAC(_num_of, Len) prvMpdCalloc(_num_of,Len)

#define PRV_MPD_FREE_MAC(_data)\
    (((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.free_PTR != NULL))? \
    prvMpdCallBacks.free_PTR(_data):FALSE)


#define PRV_MPD_HANDLE_FAILURE_MAC(_rel_ifIndex, _severity, _error_text)\
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.handle_failure_PTR != NULL))? \
	prvMpdCallBacks.handle_failure_PTR(_rel_ifIndex, _severity, __LINE__, PRV_MPD_DEBUG_FUNC_NAME_MAC(), _error_text):\
	FALSE)

#define PRV_MPD_LOGGING_MAC(_log_text)\
    (((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.logging_PTR != NULL))? \
    prvMpdCallBacks.logging_PTR(_log_text):\
    FALSE)

#define PRV_MPD_SLEEP_MAC(_time_in_ms) 	\
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.sleep_PTR != NULL))? \
	prvMpdCallBacks.sleep_PTR(_time_in_ms):\
	FALSE)

#define PRV_MPD_DEBUG_IS_ACTIVE_MAC(_flag) \
	(	(prvMpdCallBacks.initialized == TRUE) && \
		(prvMpdCallBacks.is_active_PTR) 		&& \
		(prvMpdCallBacks.is_active_PTR(_flag)))

#define PRV_MPD_DEBUG_LOG_MAC(_flag) \
	if ( 	(PRV_MPD_DEBUG_IS_ACTIVE_MAC(_flag) && \
			(prvMpdCallBacks.debug_log_PTR)))	prvMpdCallBacks.debug_log_PTR

#define PRV_MPD_DEBUG_LOG_PORT_MAC(_flag,_rel_ifIndex) \
	PRV_MPD_DEBUG_LOG_MAC(_flag)

#define PRV_MPD_TRANSCEIVER_ENABLE_MAC(_rel_ifIndex, Enable) \
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.txEnable_PTR != NULL))?\
		prvMpdCallBacks.txEnable_PTR(_rel_ifIndex, (Enable)):FALSE)

#define PRV_MPD_GET_FW_FILES_MAC(_phyType, _main) \
	(((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.getFwFiles_PTR != NULL))?\
		prvMpdCallBacks.getFwFiles_PTR(_phyType, _main):FALSE)

#define PRV_MPD_SMI_AUTONEG_DISABLE_MAC(_rel_ifIndex, _disable, _prev_state_PTR)\
    (((prvMpdCallBacks.initialized == TRUE) && (prvMpdCallBacks.smiAn_disable_PTR != NULL))?\
    prvMpdCallBacks.smiAn_disable_PTR((_rel_ifIndex),(_disable), (_prev_state_PTR)): FALSE)

#define MPD_MDIO_ACCESS_DONT_SET_PAGE_CNS	(0xFFFF)

#define PRV_MPD_MDIO_WRITE_MAC(_rel_ifIndex, _deviceOrPage, _address, _value)\
		prvMpdMdioWrapWrite(_rel_ifIndex, prvMpdGetPortEntry(_rel_ifIndex)->initData_PTR->mdioInfo.mdioAddress, _deviceOrPage, _address, _value)

#define PRV_MPD_MDIO_READ_MAC(_rel_ifIndex, _deviceOrPage, _address, _value)\
		prvMpdMdioWrapRead(_rel_ifIndex, prvMpdGetPortEntry(_rel_ifIndex)->initData_PTR->mdioInfo.mdioAddress, _deviceOrPage, _address, _value)

#define PRV_MPD_DEBUG_FUNC_NAME_MAC() ((const char *)__FUNCTION__)
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#if BYTE_ORDER == LITTLE_ENDIAN_MEM_FORMAT
#define PRV_MPD_SWAP16_MAC(val) \
    (UINT_16)(((UINT_16)((val) & 0xFF) << 8) + ((UINT_16)((val) & 0xFF00) >> 8))
#else
#define PRV_MPD_SWAP16_MAC(val)   (val)
#endif

#if BYTE_ORDER == LITTLE_ENDIAN_MEM_FORMAT
#define PRV_MPD_SWAP32_MAC(val)   (UINT_32 ) ( (((val) & 0xFF) << 24) | (((val) & 0xFF00) << 8) | (((val) & 0xFF0000) >> 8) | (((val) & 0xFF000000) >> 24) )
#else
#define PRV_MPD_SWAP32_MAC(val)   (val)
#endif


typedef enum {
    PRV_MPD_SPECIFIC_FEATURE_FW_LOAD_SF_E,
    PRV_MPD_SPECIFIC_FEATURE_WA_1680_E,
    PRV_MPD_SPECIFIC_FEATURE_USER_DEFIND_1_E,
    PRV_MPD_SPECIFIC_FEATURE_USER_DEFIND_2_E,
    PRV_MPD_SPECIFIC_FEATURE_USER_DEFIND_3_E,
    /* must be last */
    PRV_MPD_SPECIFIC_FEATURE_LAST_E
} PRV_MPD_SPECIFIC_FEATURE_ENT;

typedef void PRV_MPD_SPECIFIC_FEATURE_FUN(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

typedef struct {
    BOOLEAN supported;
    PRV_MPD_SPECIFIC_FEATURE_FUN *preInitFeatureExecuteCallback_PTR;
    PRV_MPD_SPECIFIC_FEATURE_FUN *postInitFeatureExecuteCallback_PTR;
} PRV_MPD_SPECIFIC_FEATURE_STC;

typedef struct {
    MPD_SPEED_CAPABILITY_TYP	ifSpeedCapabBit;
    MPD_TYPE_BITMAP_TYP     	phyTypeBit;
} PRV_MPD_CONVERT_IF_SPEED_TO_PHY_TYPE_STC;


typedef enum {
    PRV_MPD_PHY_READY_AFTER_RESET_E,
    PRV_MPD_PHY_READY_AFTER_FW_DOWNLOAD_E
} PRV_MPD_PHY_READY_SEQUENCE_ENT;

#define PRV_MPD_NUM_OF_EXTERNAL_PHY_TYPES_SUPPORTED_CNS (MPD_TYPE_NUM_OF_TYPES_E)


typedef struct {
	UINT_32			max; /* maximum consumption - must be one of the options  */
	struct {
		UINT_32 sr_on[MPD_SPEED_LAST_E];
		UINT_32 sr_off[MPD_SPEED_LAST_E];
	}up; /* power consumption when port is up */

	struct {
		UINT_32 ed_on;
		UINT_32 ed_off;
	}down; /* power consumption when port is down */
} PRV_MPD_POWER_CONSUMPTION_STC;


extern PRV_MPD_POWER_CONSUMPTION_STC 	prvMpdPowerConsumptionPerPhy[MPD_TYPE_NUM_OF_TYPES_E];
/*extern PRV_MPD_PORT_LIST_TYP 			prvMpdDebugWriteDisabledPortList;*/
extern PRV_MPD_PORT_LIST_TYP 			prvMpdActivePorts;
extern MPD_CALLBACKS_STC 				prvMpdCallBacks;
extern PRV_MPD_PORT_LIST_TYP 			prvMpd88E1680PortList;
extern PRV_MPD_GLOBAL_DB_STC	*		prvMpdGlobalDb_PTR;
extern UINT_32 							prvMpdDebugTraceFlagId;

#define PRV_MPD_fw_version_index_major_CNS        0
#define PRV_MPD_fw_version_index_minor_CNS        1
#define PRV_MPD_fw_version_index_inc_CNS          2
#define PRV_MPD_fw_version_index_test_CNS         3
#define PRV_MPD_fw_version_size_CNS               4

typedef union {
    UINT_8  ver_bytes[PRV_MPD_fw_version_size_CNS];
    UINT_32 ver_num;
} PRV_MPD_sw_version_UNT;


typedef struct {
    MPD_TYPE_ENT        		phyType;
    MTD_DEV           			* mtdObject_PTR;
    MPD_FW_FILE_STC     		fw_main;
    PRV_MPD_sw_version_UNT    fw_version;
}PRV_MPD_MTD_OBJECT_STC;

typedef struct {
    BOOLEAN exist;
    MPD_PHY_FW_PARAMETERS_STC *phyFw_PTR;
    BOOLEAN representativePerPhyNumber_ARR [MPD_MAX_PORTS_TO_DOWNLOAD_CNS];
    PRV_MPD_MTD_OBJECT_STC *obj_PTR;
} PRV_MPD_MTD_OBJECT_INFO_STC;


typedef struct {
    UINT_8       mdioDev;   /* pp device number */
    UINT_8       mdioBus;   /* bus id */
    MPD_TYPE_ENT phyType; /* phy type */
} PRV_MPD_APP_DATA_STC;

extern PRV_MPD_MTD_OBJECT_STC * prvMpdMtdPortDb_ARR[MPD_MAX_PORT_NUMBER_CNS+1];

#define PRV_MPD_MTD_OBJECT_MAC(_rel_ifindex)         prvMpdMtdPortDb_ARR[_rel_ifindex]->mtdObject_PTR
#define PRV_MPD_MTD_PORT_OBJECT_MAC(_rel_ifindex)    PRV_MPD_MTD_OBJECT_MAC(_rel_ifindex), prvMpdGetPortEntry(_rel_ifindex)->initData_PTR->mdioInfo.mdioAddress
#define PRV_MPD_MTD_ASSERT_REL_IFINDEX(_rel_ifIndex) 					\
		if (_rel_ifIndex > MPD_MAX_PORT_NUMBER_CNS) {					\
			PRV_MPD_HANDLE_FAILURE_MAC(	rel_ifIndex,					\
										MPD_ERROR_SEVERITY_ERROR_E,		\
										"Illegal rel_ifIndex");			\
			return MPD_OP_FAILED_E;										\
		}

typedef struct {
    UINT_16 mtd_val; 
    UINT_32 hal_val;
}PRV_MPD_MTD_TO_MPD_CONVERT_STC;

#define PRV_MPD_GAIN_COF_BITS1_CNS      3
#define PRV_MPD_GAIN_COF_BITS2_CNS      7

#define PRV_MPD_MAX_GAINS_NUMBER_CNS    4

typedef struct {
    UINT_16 gainCoefficient;
    UINT_16 maxVal;
    double  gainVars [PRV_MPD_MAX_GAINS_NUMBER_CNS * 2];
}PRV_MPD_VCT_EXT_GAIN_INFO_STC;


typedef struct {
    UINT_8                               mdioBus;
    UINT_32                              phyAddr;
    UINT_32                              phyType;
    UINT_32                              hostDevNum;
    MYD_DEV                             * mydObject_PTR;
}PRV_MPD_MYD_OBJECT_STC;

typedef struct {
            BOOLEAN exist;
            MPD_PHY_FW_PARAMETERS_STC *phyFw_PTR;
            PRV_MPD_MYD_OBJECT_STC *obj_PTR;
}PRV_MPD_MYD_OBJECT_INFO_STC;

extern PRV_MPD_MYD_OBJECT_STC * prvMpdMydPortDb_ARR[MPD_MAX_PORT_NUMBER_CNS+1];
#define PRV_MPD_MYD_OBJECT_MAC(_rel_ifindex)         prvMpdMydPortDb_ARR[_rel_ifindex]->mydObject_PTR
#define PRV_MPD_MYD_CONTEXT_MAC(_rel_ifindex)        prvMpdMydPortDb_ARR[_rel_ifindex]->mydObject_PTR->hostContext

#define PRV_MPD_STUB_CREATE_MAC(__func_name) \
    extern MPD_RESULT_ENT __func_name (   \
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR, \
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR   \
)

PRV_MPD_STUB_CREATE_MAC(prvMpdGetMdixOperType_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetMdixAdminMode_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetVctTest_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetExtVctParams_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetCableLen_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdResetPhy_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetLinkPartnerPauseCapable_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenConsumption);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetCableLenNoRange_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationRemoteCapabilities_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationRemoteCapabilities_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdAdvertiseFc_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetAutoNegotiation_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetComboMediaType_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetDuplexMode_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetAutoNegotiation_7);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindCopper);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindSfp);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindCombo);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyKindByType);
PRV_MPD_STUB_CREATE_MAC(prvMpdSfpPresentNotification);
PRV_MPD_STUB_CREATE_MAC(prvMpdMdioAccess);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetEeeAdvertize);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeRemStatus);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeCapability);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetPowerModules_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdEnableFiberPortStatus_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetComboMediaType_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetEeeMasterEnable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLpiExitTime_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLpiEnterTime_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeMaxTxVal_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_9);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_11);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_16);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpecificFeatures_3 );
PRV_MPD_STUB_CREATE_MAC(prvMpdSetVctTest_7);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpecificFeatures_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_14);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetFastLinkDownEnable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeGetEnableMode_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeGetEnableMode_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetCableLenNoRange_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetInternalOperStatus_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdSfpPresentNotification_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetEeeMaxTxVal_3 );
PRV_MPD_STUB_CREATE_MAC(prvMpdSetAutoNegotiation_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetDuplexMode_8);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetMdixAdminMode_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdResetPhy_7);
PRV_MPD_STUB_CREATE_MAC(prvMpdDisableOperation_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLoopback);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetPhyIdentifier);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdMydGetPhyIdentifier);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetPhyPageSelect);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetLoopback_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_6);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetGreenReadiness_8);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctOffset_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_4);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetVctCapability_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdSetSpeed_9);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetDteStatus_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetTemperature);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetMdixMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDuplexMode_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetMdixMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetVctTest);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetCableLenghNoBreakLink);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdPreFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdFwDownload_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdPostFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiEnterTimer_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeAdvertise_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLegacyEnable_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeRemoteStatus_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetNearEndLoopback_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiExitTimer_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetPowerModules);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetSpeed_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetLpAnCapabilities);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetAdvertiseFc);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetLpAdvFc);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdRestartAutoNeg);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetPresentNotification);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetSpeed_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetSpeed_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetAutoNeg_5);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDisable_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetDisable_3);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeCapabilities);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeRemoteStatus_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeAdvertise_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLegacyEnable_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetEeeConfig_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiExitTimer_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetEeeLpiEnterTimer_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdSetCheckLinkUp);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetTemperature);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit33x0And32x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit20x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit2180);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit3540);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdInit25x0);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetMdixAdmin_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetMdixAdminMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdRestartAutoNegotiation_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationAdmin_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationAdmin_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetAutonegAdmin_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdMtdGetAutonegSupport);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationSupport_1);
PRV_MPD_STUB_CREATE_MAC(prvMpdGetAutoNegotiationSupport_2);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydPreFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydPostFwDownload);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetPortMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetPortMode);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetSerdesTune);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetSerdesLanePolarity);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetAutoNeg);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetPortLaneBmp);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetPortLinkStatus);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydSetDisable);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydGetTemperature);
PRV_MPD_STUB_CREATE_MAC(prvMpdMydInit7120);
/* *****************************************/
/* Private PHY operations */

typedef enum {
	/** @internal @brief Set combo media type */
    PRV_MPD_OP_CODE_SET_COMBO_MEDIA_TYPE_E = MPD_OP_NUM_OF_OPS_E,
	/** @internal @brief Enable fiber part in PHY */
    PRV_MPD_OP_CODE_SET_ENABLE_FIBER_PORT_STATUS_E,
	/** @internal @brief Pre firmware download */
    PRV_MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E,
	/** @internal @brief Download FW to PHY */
    PRV_MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
	/** @internal @brief Post FW download */
    PRV_MPD_OP_CODE_SET_PHY_POST_FW_DOWNLOAD_E,
	/** @internal @brief Set PHY specific feature hooks (internal PHY operations) */
    PRV_MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E,
	/** @internal @brief Set PHY (towards host) loopback */
    PRV_MPD_OP_CODE_SET_LOOP_BACK_E,
	/** @internal @brief Debug - run VCT when no cable is connected */
    PRV_MPD_OP_CODE_GET_VCT_OFFSET_E,
	/** @internal @brief Initialize PHY */
    PRV_MPD_OP_CODE_INIT_E,
	/** @internal @brief Execure Errata */
    PRV_MPD_OP_CODE_SET_ERRATA_E,
	/** @internal @brief Enable fast link down */
    PRV_MPD_OP_CODE_SET_FAST_LINK_DOWN_ENABLE_E,
	/** @internal @brief Get max EEE tx exit time */
    PRV_MPD_OP_CODE_GET_EEE_MAX_TX_VAL_E,
	/** @internal @brief Get EEE enable mode (link change / admin) */
    PRV_MPD_OP_CODE_GET_EEE_ENABLE_MODE_E,
	/**  @internal @brief Verify Link & Speed match negotiation capabilities (confgured)\n
	 *  Restart auto-neg in case of missmatch
	 */
    PRV_MPD_OP_CODE_SET_CHECK_LINK_UP_E,
    /** @internal @brief get the number of lanes for configured mode */
    PRV_MPD_OP_CODE_GET_LANE_BMP_E,
    /** @internal @brief used for PHYs which support page select */
    PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,

	PRV_MPD_NUM_OF_OPS_E
}PRV_MPD_OP_CODE_ENT;


/* the following operations don't have parameters
 * MPD_OP_CODE_SET_RESET_PHY_E,
 * MPD_OP_CODE_SET_PHY_FW_DOWNLOAD_E,
 * MPD_OP_CODE_SET_PHY_SPECIFIC_FEATURES_E
 * MPD_OP_CODE_SET_PHY_PRE_FW_DOWNLOAD_E
 */

/* *****************************************/


extern MPD_RESULT_ENT mpdMtdInit(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP   * fw_port_list_PTR,
    MPD_TYPE_ENT         * phyType_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioReadRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioReadRegister(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 device,
	UINT_16 address,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *value_PTR
);
extern MPD_RESULT_ENT prvMpdMdioReadRegisterNoPage(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *value_PTR
);
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioWriteRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioWriteRegisterNoPage(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	UINT_16 value
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioWriteRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
extern MPD_RESULT_ENT prvMpdMdioWriteRegister(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 device,
	UINT_16 address,
	UINT_16 mask,
	UINT_16 value
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpd1540BypassSet(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    BOOLEAN bypass /* TRUE - bypass mode, FALSE - mac on PHY mode */
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdEeeIpg_1 (
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdCalloc
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern void* prvMpdCalloc (
	/*     INPUTS:             */
    UINT_32                 numOfObjects,
    UINT_32                 len
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: HALP_config_port_hash_create
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortHashCreate (
	/*     INPUTS:             */
	void
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListRemove
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListRemove (
	/*     INPUTS:             */
	UINT_32							rel_ifIndex,
	PRV_MPD_PORT_LIST_TYP * portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListClear
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern void prvMpdPortListClear(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
  /*     INPUTS / OUTPUTS:   */
  /*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListAdd
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListAdd (
	/*     INPUTS:             */
	UINT_32							rel_ifIndex,
	PRV_MPD_PORT_LIST_TYP * portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdGetPortEntry
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdGetPortEntry (
    /*     INPUTS:             */
    UINT_32 rel_ifIndex
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListIsMember
*
* DESCRIPTION: TRUE when rel_ifIndex is member of portList_PTR
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListIsMember (
	/*     INPUTS:             */
	UINT_32					rel_ifIndex,
	PRV_MPD_PORT_LIST_TYP * portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListGetNext
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListGetNext(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR,
  /*     INPUTS / OUTPUTS:   */
    UINT_32    * relative_ifIndex_PTR
  /*     OUTPUTS:            */
);

extern BOOLEAN prvMpdPortListIsEmpty(
  /*!     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
  /*!     INPUTS / OUTPUTS:   */
  /*!     OUTPUTS:            */
);

extern UINT_32 prvMpdPortListNumOfMembers(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR
  /*     INPUTS / OUTPUTS:   */
  /*     OUTPUTS:            */
);


extern MPD_RESULT_ENT mpdBindExternalPhyCallback(
    /*     INPUTS:             */
    MPD_TYPE_ENT           phyType,
    MPD_OP_CODE_ENT        op,
    MPD_OPERATIONS_FUN   * func_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdXmdioWriteReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_8  device,
    UINT_16 address,
    UINT_16 data
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT prvMpdXmdioReadReg(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    UINT_8  device,
    UINT_16 address,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16    * data_PTR
);

extern MPD_RESULT_ENT prvMpdPerformPhyOperation(
    /*     INPUTS:             */
    PRV_MPD_PORT_HASH_ENTRY_STC   * portEntry_PTR,
    MPD_OP_CODE_ENT op,
    /*     INPUTS / OUTPUTS:   */
    MPD_OPERATIONS_PARAMS_UNT  * params_PTR
    /*     OUTPUTS:            */
);

extern MTD_STATUS mpdMtdMdioReadWrap (
    /*     INPUTS:             */
    MTD_DEV *dev_PTR,
    UINT_16 rel_ifIndex,
    UINT_16 dev,
    UINT_16 reg,
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
    UINT_16  *vlaue_PTR
);

extern MTD_STATUS mpdMtdMdioWriteWrap (
    /*     INPUTS:             */
    MTD_DEV *dev_PTR,
    UINT_16 rel_ifIndex,
    UINT_16 dev,
    UINT_16 reg,
    UINT_16 value
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern void prvMpdFwLoadSequence(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * fw_port_list_PTR
  /*     INPUTS / OUTPUTS:   */
  /*     OUTPUTS:            */
);

extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdFindPortEntry(
	/*     INPUTS:             */
	PRV_MPD_APP_DATA_STC	* app_data_PTR,
	UINT_16					mdio_address
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
);

/* ***************************************************************************
* FUNCTION NAME: mpdMtdInit
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern MPD_RESULT_ENT mpdMtdInit(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP     * fw_portList_PTR,
    MPD_TYPE_ENT           * phyType_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);
/*$ END OF mpdMtdInit */


extern MPD_RESULT_ENT prvMpdUpdatePhyPowerConsumption(
    /*     INPUTS:             */
    MPD_TYPE_ENT    phyType,
    PRV_MPD_POWER_CONSUMPTION_STC *powerConsumption_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
);

extern MPD_RESULT_ENT mpdMydInit(
    /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP   * exhw_portList_PTR,
    MPD_TYPE_ENT           * phyType_PTR
);

extern MPD_RESULT_ENT prvMpdMdioWrapWrite(
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		 mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
    IN  UINT_16      value
);
extern MPD_RESULT_ENT prvMpdMdioWrapRead (
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		 mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
    OUT UINT_16    * value_PTR
);
#endif /* PRV_MPD_H_ */


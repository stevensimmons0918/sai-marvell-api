/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

#ifndef MPD_H_
#define MPD_H_
/**
 * @file mpd.h
 *
 * @brief This file contains types and declarations
 *  required in order to initialize MPD
 */

#include <mpdTypes.h>
#include <mpdApi.h>

/**
* @defgroup Initialization Initialization
* @{MPD initialization:
*  - initialize DBs
*  - provide OS & application callbacks
*  - Create ports
*  - Initialize PHY modules
*/

/**
* @defgroup Database Database
* @{Port DB initialization information
*/

/**
 * @enum    MPD_TRANSCEIVER_TYPE_ENT
 *
 * @brief   Enumerator Transceiver type
 */
typedef enum {
    MPD_TRANSCEIVER_SFP_E,	     /*= PDL_TRANSCEIVER_TYPE_FIBER_E*/
    MPD_TRANSCEIVER_COPPER_E,    /*= PDL_TRANSCEIVER_TYPE_COPPER_E*/
    MPD_TRANSCEIVER_COMBO_E, 	 /*= PDL_TRANSCEIVER_TYPE_COMBO_E*/
    MPD_TRANSCEIVER_MAX_TYPES_E	 /*= PDL_TRANSCEIVER_TYPE_LAST_E */
} MPD_TRANSCEIVER_TYPE_ENT;

/**
 * @enum    MPD_FW_TYPE_ENT
 *
 * @brief   Enumerator for firmware type
 */
typedef enum {
	/** 
     * @brief PHY doesn't require/support firmware 
     */
    MPD_FW_TYPE_NO_FW_E,
	/** 
     * @brief Firmware type SF, compatible with 88X3540, 88X2540, 88X32X0, 88X33X0, 88X21X0, 88X20X0 
     */
    MPD_FW_TYPE_SOLAR_FLARE_E,
	/** @brief compatible with X7120/X6181/X6141*/
    MPD_FW_TYPE_ALASKA_C_E,
    MPD_NUM_OF_FW_TYPES_E
} MPD_FW_TYPE_ENT;


/**
 * @brief MPD_MDIO_WRITE_FUNC \n
 *  prototype function for write to PHY
 *  @remark deviceOrPage used by 10G PHYs (clause 45), 1G PHYs (clause 22) use page swapping internally - should be used for user defined PHYs
 */
typedef BOOLEAN MPD_MDIO_WRITE_FUNC (
    IN UINT_32      rel_ifIndex,
    IN UINT_8       mdioAddress,
    IN UINT_16      deviceOrPage,   /* PRV_MPD_IGNORE_PAGE_CNS */
    IN UINT_16      address,
    IN UINT_16      value
);

/**
 * @brief MPD_MDIO_READ_FUNC \n
 *  prototype function for read from PHY
 *  @remark deviceOrPage used by 10G PHYs (clause 45), 1G PHYs (clause 22) use page swapping internally - should be used for user defined PHYs
 */
typedef BOOLEAN MPD_MDIO_READ_FUNC (
    IN  UINT_32      rel_ifIndex,
    IN  UINT_8       mdioAddress,
    IN  UINT_16      deviceOrPage,
    IN  UINT_16      address,
    OUT UINT_16    * value_PTR
);


/**
 * @struct MPD_PHY_TX_SERDERS_PARAMETERS_STC
 * @brief relevant to MTD PHYs \n
 * defines the TX serdes tuning parameters
 * @remark	lane0: 3.0x29, lane1: 3.0x2A
 */
typedef struct {
    struct {
        BOOLEAN     value_set;
        UINT_8      pre_tap;    /* 15:11 */
        UINT_8      main_tap;   /* 10:5  */
        UINT_8      post_tap;   /* 4:0   */
    } lane_0;
    struct {
        BOOLEAN     value_set;
        UINT_8      pre_tap;    /* 15:11 */
        UINT_8      main_tap;   /* 10:5  */
        UINT_8      post_tap;   /* 4:0   */
    } lane_1;
}MPD_PHY_TX_SERDERS_PARAMETERS_STC;

/** 
  * @brief maximum number of pairs in cable. Mostly: (1,2) (3,6) (4,5) (7,8)  
  */
#define MPD_VCT_MDI_PAIR_NUM_CNS    4

/** 
 * @brief number of channel pairs in cable: A/B and C/D 
 */
#define MPD_VCT_CHANNEL_PAIR_NUM_CNS 2

/** 
 * @brief Amplitude threshold for cables shorter then 110m 
 */
#define MPD_VCT_THRESHOLD_AMPLITUDE_VALUE_CNS 40

/** 
 * @brief Amplitude threshold for cables longer then 110m 
 */
#define MPD_VCT_THRESHOLD_AMPLITUDE_VALUE_FOR_110M_CABLE_CNS 20

/** 
 * @brief 110m minimum possible measured length due to 10% possible deviation. 
 * (110 * 90% = 99 > 98).                                               
 */

#define  MPD_VCT_MIN_110M_CABLE_LEN_DUE_TO_DEVIATION_CNS 98
/**
 * @enum MPD_VCT_TEST_STATUS_ENT
 *
 * @brief Enumeration of VCT test status
 */
typedef enum{

    /**
     * @brief virtual cable test failed (can't run the test) 
     */
    MPD_VCT_TEST_FAIL_E,

    /** 
     * @brief  normal cable. 
     */
    MPD_VCT_NORMAL_CABLE_E,

    /**
     * @brief  open in cable. 
     */
    MPD_VCT_OPEN_CABLE_E,

    /**
     * @brief  short in cable. 
     */
    MPD_VCT_SHORT_CABLE_E,

    /** 
     * @brief impedance mismatch in cable
     * (two cables of different quality is connected each other).
     */
    MPD_VCT_IMPEDANCE_MISMATCH_E,

    /**
     * @brief  short between Tx pair and Rx pair 0 
     */
    MPD_VCT_SHORT_WITH_PAIR0_E,
    /**
     * @brief  short between Tx pair and Rx pair 1
     */
     MPD_VCT_SHORT_WITH_PAIR1_E,
    
    /**
     * @brief  short between Tx pair and Rx pair 2 
     */
     MPD_VCT_SHORT_WITH_PAIR2_E,

    /**
     * @brief  short between Tx pair and Rx pair 3 
     */
     MPD_VCT_SHORT_WITH_PAIR3_E

} MPD_VCT_TEST_STATUS_ENT;


/**
 * @enum MPD_VCT_NORMAL_CABLE_LEN_ENT
 *
 * @brief Enumeration for normal cable length
 */
typedef enum{

    /**
     * @brief  cable length less than 50 meter. 
     */
    MPD_VCT_LESS_THAN_50M_E,

    /**
     * @brief cable length between 50 - 80 meter. 
     */
    MPD_VCT_50M_80M_E,

    /**
     * @brief cable length between 80 - 110 meter. 
     */
    MPD_VCT_80M_110M_E,

    /**
     * @brief  cable length between 110 - 140 meter. 
     */
    MPD_VCT_110M_140M_E,

    /**
     * @brief cable length more than 140 meter. 
     */
    MPD_VCT_MORE_THAN_140_E,

    /**
     * @brief unknown length. 
     */
    MPD_VCT_UNKNOWN_LEN_E

} MPD_VCT_NORMAL_CABLE_LEN_ENT;

/**
 * @struct MPD_VCT_STATUS_STC
 *
 * @brief Structure holding VCT results
 */
typedef struct{

    /** 
     * @brief VCT test status. 
     */
    MPD_VCT_TEST_STATUS_ENT testStatus;

    /** 
     * @brief for cable failure the estimate fault distance in meters. 
     */
    UINT_8 errCableLen;

} MPD_VCT_STATUS_STC;


/**
 * @enum MPD_VCT_PAIR_SWAP_ENT
 *
 * @brief Enumeration for pair swap
 */
typedef enum{

    /** @brief channel A on MDI[0] and B on MDI[1]
     *  or channel C on MDI[3] and D on MDI[4]
     */
    MPD_VCT_CABLE_SWAP_STRAIGHT_E,

    /** @brief channel B on MDI[0] and A on MDI[1]
     *  or channel D on MDI[3] and C on MDI[4]
     */
    MPD_VCT_CABLE_SWAP_CROSSOVER_E,

    /**
     * @brief in FE just two first channels are checked 
     */
    MPD_VCT_NOT_APPLICABLE_SWAP_E

} MPD_VCT_PAIR_SWAP_ENT;

/**
 * @enum MPD_VCT_POLARITY_SWAP_ENT
 *
 * @brief Enumeration for pair polarity swap
 */
typedef enum{

    /**
     * @brief good polarity 
     */
    MPD_VCT_POSITIVE_POLARITY_E,

    /**
     * @brief reversed polarity 
     */
    MPD_VCT_NEGATIVE_POLARITY_E,

    /** 
     * @brief in FE there just one value for all the cable then just first pair is applicable
     */
    MPD_VCT_NOT_APPLICABLE_POLARITY_E

} MPD_VCT_POLARITY_SWAP_ENT;

/**
 * @struct MPD_VCT_PAIR_SKEW_STC
 *
 * @brief pair skew values.
*/
typedef struct{

    /** @brief whether results are valid
     *  (not valid for FE).
     */
    BOOLEAN isValid;

    /** 
     * @brief  
     */
    UINT_32 skew[MPD_VCT_MDI_PAIR_NUM_CNS];

} MPD_VCT_PAIR_SKEW_STC;

/**
* @struct MPD_VCT_EXTENDED_STATUS_STC
 *
 * @brief extended virtual cable diagnostic status per MDI pair/channel
 * pair.
*/
typedef struct{

    /** @brief if results are valid (if not, maybe there is no
     *  gigabit link for GE or 100M link for FE).
     */
    BOOLEAN isValid;

    MPD_VCT_PAIR_SWAP_ENT pairSwap[MPD_VCT_CHANNEL_PAIR_NUM_CNS];

    MPD_VCT_POLARITY_SWAP_ENT pairPolarity[MPD_VCT_MDI_PAIR_NUM_CNS];

    /** @brief the skew among the four pairs of the cable
     *  (delay between pairs in n-Seconds)
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  in this case:
     *  swap - just pairSwap[0] relevant
     *  polarity - just pairPolarity[0] and pairPolarity[1] are relevant
     *  skew - will be not relevant
     */
    MPD_VCT_PAIR_SKEW_STC pairSkew;

} MPD_VCT_EXTENDED_STATUS_STC;

/**
 * @struct MPD_VCT_ACCURATE_CABLE_LEN_STC
 *
 * @brief accurate cable length for each MDI pair.
 */
typedef struct{

    BOOLEAN isValid[MPD_VCT_MDI_PAIR_NUM_CNS];

    UINT_16 cableLen[MPD_VCT_MDI_PAIR_NUM_CNS];

} MPD_VCT_ACCURATE_CABLE_LEN_STC;

/** 
 * @struct MPD_VCT_CABLE_EXTENDED_STATUS_STC
 *
 * @brief extended virtual cable diag. status per MDI pair/channel pair.
 */
typedef struct{

    /** 
     * @brief extended VCT cable status. 
     */
    MPD_VCT_EXTENDED_STATUS_STC vctExtendedCableStatus;

    /** @brief accurate cable lenght.
     *  accurateCableLen - not relevant
     */
    MPD_VCT_ACCURATE_CABLE_LEN_STC accurateCableLen;

} MPD_VCT_CABLE_EXTENDED_STATUS_STC;


/**
 * @struct MPD_VCT_CABLE_STATUS_STC
 *
 * @brief virtual cable diagnostic status per MDI pair.
*/
typedef struct{
    /** 
     * @brief structure holding vct results per pair
     */
    MPD_VCT_STATUS_STC cableStatus[MPD_VCT_MDI_PAIR_NUM_CNS];

    /** @brief cable lenght for normal cable. */
    MPD_VCT_NORMAL_CABLE_LEN_ENT normalCableLen;

    /** @brief type of phy (100M phy or Gigabit phy)
     *  Comments:
     *  If PHY is PHY_100M cableStatus will have only 2 pairs relevant.
     *  One is RX Pair (cableStatus[0] or cableLen[0]) and
     *  the other is TX Pair (cableStatus[1] or cableLen[1]).
     */

} MPD_VCT_CABLE_STATUS_STC;


/**
 * @brief this is the maximum number of ports that can be downloaded
 * in parallel ram or parallel flash download functions
 */
#define MPD_MAX_PORTS_TO_DOWNLOAD_CNS 32

typedef struct {
		/**
		 * @brief When the fw on the phy is different from the the desired fw file. 
         * TRUE -The fw on the phy is different from the the desired fw file.
		 */
		BOOLEAN						isDownloadRequired;
        /**
		 * @brief update when fw download is completed successfully on the phy.
		 * TRUE -Fw is already download.
		 */
        BOOLEAN						isDownloaded;
		/**
		 * @brief The phy addresses of ports to which FW will be loaded in parallel. should be all phy addresses of same phyType in PP and on the same MDIO bus.
		 * @warning Relevant only when fwDownloadType == PDL_PHY_DOWNLOAD_TYPE_RAM_E, parallel download to flash is not implemented
		 */
		UINT_16						parallel_list[MPD_MAX_PORTS_TO_DOWNLOAD_CNS];
		/**
		 * @brief Number of ports in parallel_list
		 * @remarks	Relevant when isRepresentative is TRUE
		 * @warning Relevant only when fwDownloadType == PDL_PHY_DOWNLOAD_TYPE_RAM_E, parallel download to flash is not implemented
		 */
        UINT_16						num_of_parallel_ports;
}MPD_PHY_FW_PARAMETERS_STC;


typedef struct {
        /**
		 * @brief current launching signal pair.
		 */
        UINT_16     currentTxPair;
		/**
		 * @brief database to hold results of all 4 RX pairs per each TX pair. 		 
         */
		UINT_16     rxPairsRes[16];
}MPD_PHY_VCT_INFO_STC;


/**
 * @enum MPD_PHY_USX_TYPE_ENT
 *
 * @brief Enumeration for usx interface type.
 * usx type refers to the case where several ports(2/4/8) are sharing one lane.
 */

typedef enum{
     /**
     * @brief no usx interface type. 
     */
    MPD_PHY_USX_TYPE_NONE,
    /**
     * @brief two ports sharing one lane in 10G speed.
     */
    MPD_PHY_USX_TYPE_10G_DXGMII,
    /**
     * @brief two ports sharing one lane in 20G speed.
     */
    MPD_PHY_USX_TYPE_20G_DXGMII,
    /**
     * @brief four ports sharing one lane in 10G speed.
     */
    MPD_PHY_USX_TYPE_10G_QXGMII,
    /**
     * @brief four ports sharing one lane in 20G speed.
     */
    MPD_PHY_USX_TYPE_20G_QXGMII,
    /**
     * @brief eight ports from two different phy's sharing one lane. 
     */
    MPD_PHY_USX_TYPE_OXGMII,
     /**
     * @brief max number of usx types. 
     */
    MPD_PHY_USX_MAX_TYPES
} MPD_PHY_USX_TYPE_ENT;


typedef struct {
    /**
     * @brief usx interface type.
     */
    MPD_PHY_USX_TYPE_ENT    usxType;
    
}MPD_PHY_USX_INFO_STC;


/**
 * @struct MPD_PORT_INIT_DB_STC
 * @brief all initialization information (per port)
 */
typedef struct {
    /** @brief MDIO access information */
    /** Mandatory in all PHY types for successful operation of MDIO transactions */
    struct {
        /** @brief The device (packet processor) through which MDIO is connected */
        UINT_8 					mdioDev;
        /** @brief MDIO bus ID */
        UINT_8					mdioBus;
        /** @brief MDIO address (Unique in bus ID) */
        UINT_8					mdioAddress;
        /** @brief function for write (clause 45 or clause 22) to PHY */ 
        /* Mandatory if MPD_CALLBACKS_STC.mdioWrite_PTR not provided - e.g OOB */
        MPD_MDIO_WRITE_FUNC * writeFunc_PTR;
        /** @brief function for read (clause 45 or clause 22) from PHY */ 
        /* Mandatory if MPD_CALLBACKS_STC.mdioRead_PTR not provided - e.g OOB */
        MPD_MDIO_READ_FUNC  * readFunc_PTR;
    }mdioInfo;
    /** @brief PHY type */
    /** Mandatory in all PHY types for successful init. */ 
    MPD_TYPE_ENT 					phyType;
    /** @brief Transceiver type */
    /** Mandatory in all PHYS except MYD to get auto-negotiation admin configuration. */
    MPD_TRANSCEIVER_TYPE_ENT		transceiverType;
    /** @brief represent the phy number which the port belong to. It must be unique in MDIO bus */
    /** Mandatory in MTD, MYD for phy fw download operation. */
    UINT_32 					    phyNumber;
    /** @brief PHY USX information. */
    /** Mandatory in MTD for (88E25x0 & 88X3540). */
    MPD_PHY_USX_INFO_STC         usxInfo;
    /** @brief Port location in multi-port PHY */
    /** Mandatory in 1680L for MACSEC & MYD. */
    UINT_8							phySlice;
    /** @brief The port (in packet processor) through which data path is connected */
    /** Mandatory in MYD phy type. */
    UINT_8                          port;
    /** @brief if true PHY should be disabled on init. */
    /** Optional for all PHYs. */
    BOOLEAN                       disableOnInit;
    /** @brief Is invert (mdi->mdix/mdix->mdi) required.
     *  @depending on board assembly, MDI result reported by PHY,
     *  @may require inversion in order to describe the actual state.
     *  Optional and relevant to 1G PHYs */
    BOOLEAN                       invertMdi;
    /** @brief Optional and relevant to MTD for PHY TX tuning parameters. */
	MPD_PHY_TX_SERDERS_PARAMETERS_STC	phyTxSerdesParams;
	/** @brief Swap pairs ABCD to DCBA. */
    /** Optional and relevant to MTD PHYs. */
	BOOLEAN						   doSwapAbcd;
	/** @brief Enable the SERDES short reach mode. */
    /** Optional and relevant to MTD PHYs. */
	BOOLEAN						   shortReachSerdes;
} MPD_PORT_INIT_DB_STC;


/** @}*/

/**
* @defgroup Callbacks Callbacks
* @{MPD requires several callbacks for normal operation, some mandatory and some are optional:
*  - OS services (alloc/free/sleep ext..)
*  - Debug services (bind/log ext...)
*  - Application services (handle failures, get FW file ext...)
*/


/**
 *  @typedef  MPD_SLEEP_FUNC
 * 	@brief Prototype function for application implementation of sleep
 * 	@param [in] time to sleep in miliseconds
 * 	@return Success/Fail
 */
typedef BOOLEAN MPD_SLEEP_FUNC (
	IN UINT_32	time_in_ms
);

/**
 *  @typedef MPD_ALLOC_FUNC
 *  @brief Prototype function for application implementation of malloc
 * 	@param [in] size of memory to allocate in bytes
 * 	@return	Pointer to allocated memory, 0 in case of allocation failure
 */
typedef void* MPD_ALLOC_FUNC (
	IN UINT_32	size
);

/**
 *  @typedef MPD_REALLOC_FUNC
 *  @brief Prototype function for application implementation of realloc
 * 	@param [in] ptr  - pointer to previously allocated buffer
 * 	@param [in] size of memory to allocate in bytes
 * 	@return	Pointer to allocated memory, 0 in case of allocation failure
 */
typedef void* MPD_REALLOC_FUNC (
        IN void         *ptr,
	IN UINT_32	size
);
/**
 * @typedef MPD_FREE_FUNC
 * @brief prototype function for application implementation of free
 * @param [in] pointer to memory
 */
typedef void MPD_FREE_FUNC (
	IN void*	data_PTR
);

/**
 * @typedef MPD_DEBUG_BIND_FUNC
 * @brief Debug bind function \n
 * Allows application to implement specific PHY debug flags
 * @param [in] comp_PTR - Component nam
 * @param [in] pkg_PTR	- Package name
 * @param [in] flag_PTR	- Flag name
 * @param [in] help_PTR	- Help on flag
 * @param [out] "flag id" that can be used to identify debug incident
 * @return Success/Fail
 * */

typedef BOOLEAN MPD_DEBUG_BIND_FUNC (
    IN const char   * comp_PTR,
	IN const char   * pkg_PTR,
	IN const char   * flag_PTR,
	IN const char   * help_PTR,
    OUT UINT_32		* flagId_PTR
);

/**
 * @typedef MPD_IS_FLAG_ACTIVE_FUNC
 * @brief Is debug flag active
 * @param [in] flagId	- debug flag
 * @return Is debug flag active
 * */
typedef BOOLEAN MPD_IS_FLAG_ACTIVE_FUNC (
    IN UINT_32	flagId
);

/**
 * @typedef MPD_DEBUG_LOG_FUNC
 * @brief Debug Log function
 * @param [in] funcName_PTR	- calling function name
 * @param [in] format_PTR	- printf style format string
 * @param [in] ...			- variadic list of params (printf style)
 * */
typedef void MPD_DEBUG_LOG_FUNC (
    IN const char    * funcName_PTR,
    IN const char    * format_PTR,
    IN ...
);

/**
 * @typedef MPD_TRANSCEIVER_ENABLE_FUNC
 * @brief Enable/Disable (periferal) Lazer Transceiver, relevant only for optic ports
 * @param [in] rel_ifIndex	- port
 * @param [in] enable		- Enable/Disable
 * @return Success/Fail
 * */

typedef BOOLEAN MPD_TRANSCEIVER_ENABLE_FUNC (
    IN UINT_32  rel_ifIndex,
    IN BOOLEAN  enable
);

/**
 * @enum MPD_ERROR_SEVERITY_ENT
 * @brief Failure Severity
 */
typedef enum {
	/** @brief Phy driver can't operate, luck of memory, uninitailized services ext... */
	MPD_ERROR_SEVERITY_FATAL_E,
	/** @brief  Operation can't be executed, no rollback, port behavior is undefined */
	MPD_ERROR_SEVERITY_ERROR_E,
	/** @brief Operation can't be executed but no adverse affect other that this action failure */
	MPD_ERROR_SEVERITY_MINOR_E
} MPD_ERROR_SEVERITY_ENT;

/**
 * @typedef MPD_HANDLE_FAILURE_FUNC
 * @brief Application handle failure function
 * @param [in]  rel_ifIndex			- Port
 * @param [in]	severity			- Severity
 * @param [in]	line				- line of failure
 * @param [in]	calling_func_PTR	- failing function
 * @param [in]	error_text_PTR		- text describing error
 * @return Result
 * */
typedef MPD_RESULT_ENT MPD_HANDLE_FAILURE_FUNC(
	IN UINT_32      			rel_ifIndex,
	IN MPD_ERROR_SEVERITY_ENT	severity,
	IN UINT_32		 			line,
	IN const char 				* calling_func_PTR,
	IN const char				* error_text_PTR
);


/**
 * @typedef MPD_LOGGING_FUNC
 * @brief Application logging function
 * @param [in]	log_text_PTR		- Log text. 
 * @return Result
 * */
typedef MPD_RESULT_ENT MPD_LOGGING_FUNC(
	IN const char       * log_text_PTR
);

/**
 * @typedef MPD_DISABLE_SMI_AUTONEG_FUNC
 * @brief Enable disable SMI auto-negotiation
 * @remark SMI auto-negotiation is the process by which PP polls standard
 * 	PHY registers in order to determine link, duplex, speed and FC status.
 * 	It is also know as "Out of band negotiation"
 * @param [in]  rel_ifIndex			- Port
 * @param [in]	disable				- Disable/Enable
 * @param [out]	prev_statues_PTR	- Previous SMI negotiation status
 * @return MPD status
 * */
typedef MPD_RESULT_ENT MPD_DISABLE_SMI_AUTONEG_FUNC(
    IN  UINT_32 rel_ifIndex,
    IN  BOOLEAN disable,
	OUT BOOLEAN *prev_statues_PTR
);

/**
 * @typedef MPD_GET_ADMIN_STATUS_FUNC
 * @brief Get admin status
 * @param [in]  rel_ifIndex			- Port
 * @return Admin Up/Down
 * @note if not implemented, assume UP
 * */
typedef BOOLEAN MPD_GET_ADMIN_STATUS_FUNC (
	IN UINT_32 rel_ifIndex
);

/**
 * @struct MPD_FW_FILE_STC
 * @brief relevant to MTD PHYs \n
 * contains the firmware file (pointer to RAM)
 */
typedef struct {
    UINT_8  *data_PTR;
    UINT_32 dataSize;
} MPD_FW_FILE_STC;

/**
 * @typedef MPD_GET_FW_FILES_FUNC
 * @brief Get FW file for Phy Type that require FW download
 * @param [in]  phyType - PHY type
 * @param [out]  main file  - the actual FW
 * @note MPD will free memory when done
 * @return Success/Fail
 */

typedef BOOLEAN MPD_GET_FW_FILES_FUNC (
  IN  MPD_TYPE_ENT		  phyType,
  OUT MPD_FW_FILE_STC	* mainFile_PTR
);


/**
 * @struct MPD_CALLBACKS_STC
 */
typedef struct {
	BOOLEAN							  	initialized;
/* memory */
	MPD_SLEEP_FUNC						* sleep_PTR;
	MPD_ALLOC_FUNC						* alloc_PTR;
	MPD_REALLOC_FUNC					* realloc_PTR;
	MPD_FREE_FUNC						* free_PTR;
/* debug */
	MPD_DEBUG_BIND_FUNC					* debug_bind_PTR;
	MPD_IS_FLAG_ACTIVE_FUNC				* is_active_PTR;
	MPD_DEBUG_LOG_FUNC					* debug_log_PTR;

	MPD_TRANSCEIVER_ENABLE_FUNC			* txEnable_PTR;
	MPD_HANDLE_FAILURE_FUNC				* handle_failure_PTR;
    MPD_LOGGING_FUNC                    * logging_PTR;
    MPD_DISABLE_SMI_AUTONEG_FUNC		* smiAn_disable_PTR;
	MPD_GET_ADMIN_STATUS_FUNC			* getAdminStatus_PTR;	/* if not defined, treat as always TRUE */
	MPD_GET_FW_FILES_FUNC				* getFwFiles_PTR;       /* Expect from user to override the fw download type ram/flash --> prvMpdGlobalDb_PTR->fwDownloadType_ARR[per phyType] */
    MPD_MDIO_READ_FUNC                  * mdioRead_PTR;
    MPD_MDIO_WRITE_FUNC                 * mdioWrite_PTR;
} MPD_CALLBACKS_STC;            
 
/** @}*/

/**
* @defgroup Functions Functions
* @{MPD initialization functions:
*/


/**
 *
 * @brief   initialize PHY driver DB
 * @param [in ]   callbacks_PTR  - application callbacks
 *
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdDriverInitDb(
    IN MPD_CALLBACKS_STC	* callbacks_PTR
);


/* ***************************************************************************
* FUNCTION NAME: mpdPortDbUpdate
*
* DESCRIPTION: update entry for <rel_ifIndex>
*      		allocates and adds entry if not allocated
*
*****************************************************************************/
/**
 * @brief This function is used in order to update MPD port DB
 * - Set data_PTR to NULL in order to delete entry
 * - Calling with new rel_ifIndex and data_PTR != NULL will add a new entry
 * - Calling with existing rel_ifIndex and data_PTR != NULL will update existing entry
 * @note Function must be called after calling mpdDriverInitDb and before calling mpdDriverInitHw
 * @param [in]   rel_ifIndex - a unique number identifying port
 * @remark	rel_ifindex must be between 0 to 128 (excluding)
 * @param [in]   data_PTR  - port information
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdPortDbUpdate (
	IN UINT_32				   rel_ifIndex,
	IN MPD_PORT_INIT_DB_STC  * data_PTR
);


/**
 *
 * @brief   initialize PHY driver for all ports\n
 *          initialize PHY (HW) for all ports
 *
 *          @note Initialization is done based on information
 *				gathered through mpdPortDbUpdate
 *
 * @return MPD_RESULT_ENT
 */
extern MPD_RESULT_ENT mpdDriverInitHw(
    void
);


/** @}*/
/** @}*/


#endif /* MPD_H_ */


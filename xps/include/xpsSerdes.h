// xpsSerdes.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsSerdes_h_
#define _xpsSerdes_h_

#include "xpsInit.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpsSerdes.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Serdes Manager
 */

/* Note:  BOOL is a macro, not a typedef "bool" with "false" and "true" values,
 * to avoid reserved word conflicts for C++ compilations.
 * FALSE must have the value 0; if compiling using C++ it may also be false.
 * TRUE must have the value 1; if compiling using C++ it may also be true.
 */
#if !defined BOOL
#define BOOL int
#endif

#define MAX_CHIPS 2
#define MAX_RINGS 2

typedef void (* userLogFunc)(int warningOrError, const char * Buffer,
                             size_t newItemLength); ///< user defined log function
typedef int (* userLogOpenFunc)(void);       ///< user defined log-open function
typedef int (* userLogCloseFunc)(
    void);      ///< user defined log-close function

/**
 * \brief state of the serdes
 */
typedef enum
{
    XP_RESET=0,     ///< serdes reset state
    XP_RUNNING=1,   ///< serdes running state
    XP_PAUSE=2,     ///< serdes pause state
    XP_ERROR=3      ///< serdes error state
} xpSerdesState_t;

/**
 * \brief Process Identifier for each chip
 */
typedef enum
{
    UNKNOWN_PROCESS = 0,    ///< TBD
    TSMC_90,                ///< TBD
    TSMC_65,                ///< TBD
    TSMC_40,                ///< TBD
    TSMC_28,                ///< TBD
    TSMC_20,                ///< TBD
    TSMC_16                 ///< TBD
    /* If new enums are added here, add them to serdes/str_conv.c as well! */
} ProcessId_t;

/**
 * \brief type of serdes used in xdk
 */
typedef enum
{
    XP_AAPL_SERDES = 0,        ///< serdes type is aapl
    XP_INVALID_SERDES_TYPE     ///< serdes type is invalid
} xpSerdesType;

/**
 * \brief FIRMWARE processor status values
 */
typedef struct xpSerdesStatus
{
    uint32_t enabled;               ///< Indicates if processor is enabled and runnning
    uint32_t pc;                    ///< Program Counter value
    uint32_t revision;              ///< Revision of firmware
    uint32_t build;                 ///< Build ID of firmware
    xpSerdesClk_t clk;              /**< Clock Processor is running on */
    xpSerdesState_t state;          ///< State the Processor is in
} xpSerdesStatus_t;

/**
 * \struct xpDiagConfig_t
 * \brief diagnostic configuration
 */
typedef struct xpDiagConfig
{
    BOOL sbusDump;             ///< Dump SBus
    BOOL serdesInitOnly;       ///< Stop after running SerDes init
    BOOL stateDump;            ///< Dump SerDes state information
    BOOL dmaDump;              ///< Dump LSB/ESB DMA data
    BOOL dmemDump;             ///< Dump FIRMWARE DMEM data
    BOOL imemDump;             ///< Dump FIRMWARE IMEM data
    BOOL binary;               ///< display binary along with hex
    BOOL columns;              ///< Display data in multiple columns

    BOOL destructive;           ///< perform destructive tests
    BOOL useExistingDivider;    ///< When performing destructive tests, use current divider
    int *dividers;              ///< When performing destructive tests, use this built in divider list (populated in the constructor)

} xpDiagConfig_t;

/**
 * \struct xpSerdes_t
 * \brief TBD
 */
typedef struct xpSerdes_t
{
    /* START of public SWIG members */
    /* These first variables are public to the SWIG output (they're listed in the #ifdef SWIG section below) */

    uint32_t debug;                               ///< global debug level
    uint32_t verbose;                             ///< global verbosity level
    uint32_t suppressErrors;                     ///< Turn serdes_log_add ERR and WARNINGS into DEBUG1 messages
    uint32_t upgradeWarnings;                    ///< If set, all WARNINGS are upgraded to ERRORS

    int32_t enableDebugLogging;               ///< TBD
    int32_t enableStreamLogging;              ///< TBD
    int32_t enableStreamErrLogging;           ///< TBD
    char *loggingFilePath;                    ///< Redirect all INFO and DEBUGn messages to user specified file.
    ///< Setting file path to this variable will enable redirecting data to the file.
    ///< This is intended for short term debugging.

    int32_t logTimeStamps;        ///< TBD

    int32_t serdesIntTimeout;                   ///< 28nm SerDes Firmware interrupt maximum number of tries
    int32_t sbusMdioTimeout;

    uint32_t enableSerdesCorePortInterrupt;   ///< When enabled, calls the user_supplied_serdes_interrupt_function() for SerDes interrupts.


    uint32_t chips;                               ///< number of die this struct points to
    uint32_t sbusRings;                           ///< number of SBus rings this struct points to

    int32_t maxCmdsBuffered;                    ///< Maximum bumber of commands to queue before sending them via TCP over AACS.
    ///< Setting this to 0 disables command buffering
    int32_t priorConnection;
    int32_t cmdsBuffered;                        ///< number of commands in buf_cmd (which are bufferred AACS commands)
    int32_t disableReconnect;                    ///< Don't allow AAPL to reconnect to TCP clients
    uint32_t capabilities;                       ///< Bitmask of the remote AACS Server's capabilities

    BOOL aacs;                                   ///< Is AACS being used for communication?
    uint32_t tapGen;                             ///< Avago TAP generation being communicated with
    uint32_t sbusCommands;                       ///< SBus commands executed
    uint32_t commands;                           ///< AACS Server commands received

    void *clientData;
    volatile int asyncCancel;
    BOOL ansiColors; ///< Defaults to the value in aapl.h. When 1, colorized output will be used when isatty() is true. When set to 2 colorized output will always be used.

    /* END of public SWIG members */
# ifndef SWIG    /* stuff in this block doesn't need to be exported for SWIG. */

    char *bufCmd,
         *bufCmdEnd;              ///< Pointer to start/end of AACS command buffer

    const char *chipName[MAX_CHIPS];  ///< Array of chip name pointers
    const char *chipRev[MAX_CHIPS];   ///< Array of chip revision pointers
    char *aacsServer;          ///< Server name (or IP address) used to open the AACS socket
    int32_t socket;                 ///< Socket used for AACS TCP communications
    int32_t tcpPort;               ///< TCP port used to open the AACS socket
    uint32_t jtagIdcode[MAX_CHIPS];       ///< JTAG IDCODE for each chip
    ProcessId_t processId[MAX_CHIPS]; ///< Process Identifier for each chip

    uint32_t lastMdioAddr[MAX_CHIPS];    ///< Last MDIO address used
    uint32_t mdioBasePortAddr;               ///< MDIO base port address.
    int32_t  i2cBaseAddr;                     ///< I2C base address.
    /*comment out below three array to resolved swig compilation error
    //unsigned short   ipRev[MAX_CHIPS][MAX_RINGS][256]; ///< IP revision for each SBus Rx */
    //unsigned short firmRev[MAX_CHIPS][MAX_RINGS][256]; ///< Revision of firmware load, populated if ip_type is SERDES or FIRMWARE */
    //char      serdesCtrlrRunning[MAX_CHIPS][MAX_RINGS][256]; ///< Indicator of firmware processor is running. */
    unsigned char
    ipType[MAX_CHIPS][MAX_RINGS][256]; ///< ipType_t identifier for each SBus Rx
    unsigned char
    lsbRev[MAX_CHIPS][MAX_RINGS][256]; ///< Revision of LSB block, populated if ip_type is SERDES
    unsigned char
    maxSbusAddr[MAX_CHIPS][MAX_RINGS];  ///< maxSBus address for each die and SBus ring

    /* IO variables used to pass data to/from most functions and to maintain command sent and error logs */
    int32_t returnCode;        ///< set by most functions to indicate success/fail status

    int32_t   logSize;         ///< memory log managment
    int32_t   data;             ///< used for functions that return int data
    int32_t   dataCharSize;   ///< data_char memory management
    char *dataChar;        ///< used for functions that return strings
    char *dataCharEnd;    ///< to truncate data_char, set data_char_end = data_char;

    char *log;              ///< memory log, logs commands, info,errors, warnings, and debug statements
    char *logEnd;          ///< to truncate log, set log_end = log;
    char *lastLog;        ///< pointer to start of most recent item stored in the log
    char *lastErr;        ///< pointer to start of most recent warning or error
    int warnings;  ///< Incremented each time an error occurs
    int errors;   ///< Incremented each time an error occurs
# endif  /* not SWIG */
} xpSerdes_t;


/**
 * \struct xpSerdesDfeState_t
 * \brief Rx DFE Settings and Control functions for DFE tuning.
 */
typedef struct xpSerdesDfeState
{
    uint32_t dc;               /**< DC-Restore value [0-255] */
    uint32_t lf;               /**< CTLE Low-Frequency setting [0-15] */
    uint32_t hf;               /**< CTLE High-Frequency setting [0-15] */
    uint32_t bw;               /**< CTLE Band-width setting [0-15] */
    uint32_t gainshape1;       /**< CTLE gainshape1 setting */
    uint32_t gainshape2;       /**< CTLE gainshape2 setting */
    uint32_t short_channel_en; /**< CTLE Short channel setting */

    uint32_t dfeGAIN;    /**< DFE Gain Tap strength setting [0-15] */
    uint32_t dfeGAIN2;   /**< DFE Gain Tap 2 strength setting [0-15] */
} xpSerdesDfeState_t;

/**
 * \brief Select when BTC are done for a serdes_dfe_repeat call
 */
typedef enum
{
    DFE_REPEAT_INIT_ALL,  ///< For every loop
    DFE_REPEAT_INIT_ONCE, ///< On first loop only
    DFE_REPEAT_INIT_NEVER ///< Never
} xpDfeRepeatInitSel_t;

/**
 * \brief Controls which Rx DFE settings are updated.
 */
typedef enum
{
    PMD_RESTART,        ///< Reset the PMD training
    PMD_BYPASS,         ///< Set Tx Eq to Initialize and start sending core data
    PMD_TRAIN           ///< Run full PMD training
} xpSerdesPmdTrainMode_t;

/**
 * \brief Controls which Rx DFE settings are updated.
 */
typedef enum
{
    PMD_FC16G,         ///< PRBS11 - x^11 + x^9 + 1
    PMD_CL72,          ///< PRBS11 - x^11 + x^9 + 1
    PMD_CL84,          ///< PRBS11 - x^11 + x^9 + 1
    PMD_CL92           ///< PRBS11 - sequence based on lane config
} xpSerdesPmdClause_t;


/**
 * \brief Link Training Configuration struct.
 */
typedef struct xpSerdesPmdConfig
{
    uint32_t initialPre; ///< Number of pre-cursor DEC commands to issue after PRESET & before DFE tuning (max of 15)
    uint32_t initialPost;///< Number of post-cursor DEC commands to issue after PRESET & before DFE tuning (max of 15)

    xpSerdesPmdTrainMode_t
    trainMode;///< Select pmd link training mode to execute by serdes_pmd_train

    uint32_t sbusAddr;        ///< SBus Rx Address of SerDes to run training on, when asymmetric = TRUE this is Rx portion of the duplex link
    uint32_t asymTx;          ///< SBus Rx Address of SerDes that is the Tx portion of the duplex link, ignored if asymmetric_mode = FALSE

    BOOL disableTimeout;  ///< Disables the timeout timer
    BOOL disableTxeqAdj; ///< Disables requesting TxEq adjustments
    BOOL asymmetricMode;  ///< When enabled o_core_status & i_core_cntl used to transfer control/status
    ///<  channels to another SerDes that is the partner to form the duplex link
    BOOL invertRequests;  ///< Invert requests made to remote Tx, (DEC -> INC, INC -> DEC)

    BOOL resetParameters;    ///< Reset any previously configured parameters, ie. clause,lane,FEC*

    xpSerdesPmdClause_t clause; ///< CL72,CL84,CL92
    uint32_t
    lane; ///< When clause is set to CL92, defines which PMD lane this SerDes is
    uint32_t
    prbsSeed; ///< When non-zero overrides the default PRBS seed value

    /* FC specific configs - Ingored unless clause == FC16G */
    BOOL  ttFECreq;      ///< Set to transmit FEC request in TT training frames.
    BOOL  ttFECcap;      ///< Set to transmit FEC capability in TT training frames.
    BOOL  ttTF;          ///< Set to transmit Fixed TxEq in TT training frames.

} xpSerdesPmdConfig_t;

/**
 * \brief Select when BTC are done for a serdes_dfe_repeat call
 */
typedef enum
{
    DFE_REPEAT_BTC_NONE,  ///< Never
    DFE_REPEAT_BTC_FIRST, ///< After first DFE tune only
    DFE_REPEAT_BTC_LAST,  ///< After last loop of DFE tune only
    DFE_REPEAT_BTC_ALL    ///< After every DFE tune
} xpDfeRepeatBtcSel_t;


/**
 * \struct xpSerdesDfeRepeat_t
 * \brief Control serdes_dfe_repeat flow.
 */
typedef struct xpSerdesDfeRepeat
{
    uint32_t loops;             ///< Number of dfe tunes to execute
    BOOL multiLine;        ///< Set to TRUE to print data over multiple lines rather single line

    /* Chip Configuration */
    float refclk;           ///< REFCLK frequency
    float voltage;          ///< Analog Supply Voltage
    int32_t   temp;             ///< Part Temperature, milli-degrees C
    char  channelName[20]; ///< Channel Name

    /* SerDes Initialization control */
    BOOL disableOtherSlices;          ///< Issue a firmware Enable Off for all SerDes before loops start
    xpDfeRepeatInitSel_t init;   ///< Control when a SerDes Init is done
    BOOL initTx;                       ///< Control whether to modify tx
    BOOL initRx;                       ///< Control whether to modify rx
    uint32_t widthMode;     ///< Value for serdes init
    BOOL phaseCal;          ///< Value for serdes init
    uint32_t divider;                       ///< Divider value to configure serdes to
    xpSerdesRxTerm_t term;        ///< Rx termination configuration
    xpSerdesTxDataSel_t prbs;    ///< PRBS sequence to set Transmitter to

    /* Tx Eq Control */
    int32_t  pre;   ///< Pre-cursor setting to use, number of Initial Precur loops to issue in pmd mode
    uint32_t atten; ///< Attenuator setting to use, ignored in pmd mode
    int32_t  post;  ///< Post-cursor setting to use, number of Initial Precur loops to issue in pmd mode


    /* Rx Eq Control */
    BOOL pmdTrain;                 ///< Set to TRUE to execute PMD link training (default: FALSE)
    xpSerdesPmdConfig_t
    *pmdConfig;///< Pointer to PMD CONFIG struct to configure pmd link training opeartion
    xpSerdesDfeState_t
    *dfeState;  ///< Pointer to DFE STATE struct for configure dfe tune mode

    /* Validation Control */
    BOOL enableErrorCheck;     ///< Controls doing an error count after tuning completes
    uint32_t errorLoops;            ///< Number of error accumulations to run after tuning
    uint32_t errorThreshold;        ///< Error threshold to stop accumulations on if exceeded
    xpDfeRepeatBtcSel_t
    btcMode; ///< Control when Bath-Tub Curve information is gathered

    /* Temperature Sensor Configuration */
    uint32_t rxTempAddr;      ///< Serdes Rx addr for temperature sensor
    BOOL rxTempConfigure; ///< Control whether or to configure the thermal sensor divider
    uint32_t tempSensorCnt;   ///< Number of temp sensors to read

} xpSerdesDfeRepeat_t;

/**
 * \brief Type of eye measurement to perform:
 */
typedef enum
{
    EYE_CROSS = 0,  ///< measure single horizontal and vertical cross sections (for BTC = bathtub curve plotting).
    EYE_SIZE = 0,   ///< measure decimal eye height and width.
    EYE_SIZE_VDUAL, ///< EYE_SIZE + get height using test clock.
    EYE_HEIGHT,     ///< measure decimal eye height only.
    EYE_WIDTH,      ///< measure decimal eye width only.
    EYE_FULL,       ///< measure entire eye, including bathtub curve plots.
    EYE_HEIGHT_DVOS ///< Retrieve DFE determined height and width only.
} xpSerdesEyeType_t;

/**
 * \struct
 * \brief Struct for configuring eye measurements
 */
typedef struct xpSerdesEyeConfig
{
    xpSerdesEyeType_t     ecEyeType;  ///< Default = EYE_FULL.
    xpSerdesRxDataQual_t ecDataQual; ///< Default = SERDES_RX_DATA_QUAL_DEFAULT
    xpSerdesRxCmpMode_t  ecCmpMode;  ///< Default = SERDES_RX_CMP_MODE_XOR.

    /** REAL TIME PLOT:  This advisory flag clues the data engine that the caller */
    /** prefers eye pixels (data points) to be gathered in a good order for */
    /** real-time display (such as hollow rectangle inwards, to draw bathtub curves */
    /** in real time) rather than just as fast as possible (such as raster-scan). */
    /** Useful if second thread can watch and plot the data as it is being gathered. */

    BOOL ecRealTimePlot;  ///< Default = false.

    /** CENTERING:  Eye measurements are taken using the SerDes Test (offset) */
    /** channel centered on the present SerDes PI (phase interpolator = timing */
    /** offset = X = horizontal) setting, but with the specified DAC (digital to */
    /** analog converter = sampler offsets = Y = vertical) setting.  Except in case */
    /** of (some) errors, the PI and DAC are returned to initial settings when done. */

    BOOL ecSetPhaseCenter;  ///< Center before gathering eye. Default = true.

    /** The code internally uses some combination of setting dwell */
    /** (sampling) times, and gathering multiple samples at each data point (eye */
    /** pixels), to derive the return values, based on these settings: */
    /** */
    /** - Dwell time is the number of RX bits (amount of real time depends on RX bit */
    /**   rate) for which to count errors at each data point.  The actual RX bits */
    /**   for each data point is returned in ed_bitsp[] (see later). */
    /** */
    /** - ec_min_dwell_bits is the lower limit on how long to sample at each data point. */
    /** - ec_max_dwell_bits is the upper limit on how long to sample at each data point. */
    /** */
    /** - If the minimum useful dwell time internally (based on time between */
    /**   serdes_int() calls to start data gathering and read back results) exceeds */
    /**   either value, the code uses the internal practical minimum. */
    /** */
    /** - Otherwise ed_bitsp[] (see later) reported for each data point never exceeds */
    /**   ec_max_dwell_bits. */
    /** */
    /** - If ec_max_dwell_bits == ec_min_dwell_bits, dynamic dwelling is disabled. */
    /** - If ec_max_dwell_bits > ec_min_dwell_bits, dynamic dwelling is enabled. */
    /**   When dynamic dwelling is enabled, errors are accumulated at each point */
    /**   using shorter than ec_max_dwell_bits dwells until the accumulated errors */
    /**   exceed ec_error_threshold (see later), or the total bit accumulation */
    /**   is ec_max_dwell_bits. */

    /** - Dynamic dwelling requires much less time to gather data, especially */
    /**   for large dwell times, than using a high dwell time on every bit: bits */
    /**   with lots of errors will dwell at ec_min_dwell_bits, so time is spent */
    /**   efficiently on just those bits on the edge of the eye. */

    int64_t ecMinDwellBits;   ///< Minimum samples to measure. Default = 1e8.
    ///< Library enforces a reasonable minimum.
    int64_t ecMaxDwellBits;   ///< Maximum samples to measure. Default = 5e5.
    BOOL   ecFastDynamic;     ///< Use >ec_error_threshold rather than >0
    ///< for neighbor dwelling.

    /** Specifies the minimum errors above which we stop processing a bit */
    /**   in dynamic dwell mode. */
    /** Processing always stops when ec_max_dwell_bits are sampled. */

    uint32_t   ecErrorThreshold;    ///< Specifies the minimum errors

    /** Various values for specialized uses. */

    float  ecDcBalance;       ///< Default is 0.5
    float  ecTransDensity;    ///< Default is 0.5
    int32_t    ecApplyVertAlpha; ///< Default is true

    /** Specify X-axis in terms of: */
    /** */
    /** - ec_x_UI: Number of unit intervals (UI) to gather. */
    /**     One UI is always the time required for one RX bit, no matter the clock */
    /**     frequency.  Valid values are  1,2,3,4,8: */
    /**     <= 0 is treated as 1, 5-7 are treated as 4, > 8 is treated as 8. */

    uint32_t ecXUI;   ///< Number of UI to gather.  Default = 1.

    /** - ec_x_resolution: The number of X points to gather for each UI. */
    /**     The requested value will be rounded down to the next lower */
    /**     power of 2, with a minimum of 8 points per UI. */
    /** */

    uint32_t ecXResolution;  ///< Number of points per UI to gather. Default = 64.

    /** Specify Y-axis in terms of: */
    /** */
    /** - ec_y_center_point: A DAC value which will be the center of the */
    /**                      range returned. */
    /** */
    /** - ec_y_points: The number of points to gather.  Valid range is 1..256 and */
    /**                will be adjusted if an invalid value is passed, or the */
    /**                specified combination with ec_y_step_size is too large. */
    /**                Points will be gathered around the specified center point. */
    /**                Value is ignored if ec_y_auto_scale is true. */
    /** */
    /** - ec_y_step_size: The vertical step size between gathered points. */
    /** */
    /** The total number of points returned will be adjusted to match the hardware */
    /**  capability and returned in the xpSerdesEyeData_t structure. */

    uint32_t ecYCenterPoint;  ///< Center around which ec_y_points is gathered. Default = 128.
    uint32_t ecYPoints;        ///< Number of points to gather. Default = 129, max = 256.
    uint32_t ecYStepSize;     ///< Default = 1.

    /** Auto scale the x resolution to gather sufficient data for valid */
    /**   horizontal BTC calculations.  If true, the effective x resolution */
    /**   is at least ec_x_resolution, if valid. */

    int32_t ecXAutoScale;      ///< Default = 1.

    /** Auto scale the y points and y step size to gather */
    /**   sufficient data for valid vertical BTC calculations. */
    /**    If true, the actual y points gathered will be determined automatically, */
    /**    while the effective y step size will be no greater than ec_y_step_size. */

    int32_t ecYAutoScale;      ///< Default = 1.
    BOOL ecNoSbm; /// If ec_no_sbm is true then sbus master assist will not be used even if avaiable. Default = false.
    uint32_t ecMeanDirection; ///If ec_gather_mean is given, use this value to control direction.
    int64_t ecMeanThreshold; ///If ec_gather_mean is given, use this value as the 50% error point, overriding calculation.
    BOOL ecGatherMean; ///If ec_gather_mean is true, gather only the eye points that bracket the 50% error point. Default = false.
    int32_t ecXShift;  /// PI steps to adjust the eye center before gathering.
} xpSerdesEyeConfig_t;



/*============================================================================= */
/** \brief Struct for returning eye measurement results, created and initialized by */
/** serdesEyeDataConstruct() and released by serdesEyeDataDestruct(). */
/** */
/** \details Note for IPC (inter-process communication) using this structure as a */
/** form of shared memory between caller and data-gathering threads: */
/** */
/** - The data engine guarantees to fill in ed_x_points and ed_y_points values, */
/**   and zero the malloc()'d memory under ed_errorsp and ed_bitsp, before */
/**   setting the pointers to non-null. */
/** */
/** - The data engine guarantees to write the ed_bitsp value before the */
/**   ed_errorsp for each new data sample. */
/** */
/** Thus, a caller that wishes to "sniff" the data while it's being gathered, */
/** say to do real-time plotting (see ec_real_time_plot), should follow this */
/** procedure: */
/** */
/** - Allocate xpSerdesEyeType_t using serdesEyeDataConstruct(). */
/** */
/** - After passing the struct to serdesEyeGet(), wait until (ed_errorsp && */
/**   ed_bitsp) before using the data; that is, the pointers are both non-null. */
/** */
/** - When exploring the eye data under the pointers (in real time as it is */
/**   revised), ignore any point where the bits value is 0 (no data yet). */
/** */
/** - Be aware that a non-zero bits value is no assurance that the corresponding */
/**   errors value is consistent.  It could lag behind (not updated yet) by one */
/**   sample/gather in the worst-case timing scenario, leading to a zero or */
/**   understated BER.  (The unimplemented alternative is to have mutex locking */
/**   around all accesses to the combined errors + bits data, since there's no */
/**   way to write both fields atomically.) */
/** */
/** - After serdesEyeGet() returns successfully, all errors and bits data */
/**   should be valid and consistent. */

typedef struct xpSerdesEyeData
{
    uint32_t  edWidth;     ///< Eye width (in real PI steps).
    uint32_t  edHeight;    ///< Eye height (in DAC steps).

    uint32_t  edWidthMUI; ///< Eye width (in mUI).
    uint32_t  edHeightMV; ///< Eye height (in mV).

    uint32_t  edXUI;          ///< Number of UI gathered.
    uint32_t  edXPoints;      ///< Width dimension of packed output arrays.
    uint32_t  edXStep;        ///< Separation between x coordinates with data.
    uint32_t  edXResolution;  ///< Pixels per UI; always a power of 2.
    ///< Available resolution is ed_x_step times this.
    int  edXMin;         ///< Min PI coordinate (corresponds to data column 0).
    int32_t  edXMax;         ///< Max PI coordinate.
    int  edXSbmCenter;  /**< Phase at which SBM accelerator is centered. */
    int  edXCenter[3];   /**< SerDes FW center phase for each of the eyes. */

    uint32_t  edYPoints;  ///< Height dimension of packed output arrays.
    uint32_t  edYStep;    ///< Separation between y coordinates with data.
    uint32_t  edYMin;     ///< Min y (dac) coordinate with data.
    uint32_t  edYMax;     ///< Max y (dac) coordinate with data.

    /** Two-dimensional arrays of [ed_x_points][ed_y_points]: */
    /** */
    /** Note: */
    /** */
    /** - Basic (unadjusted) BER = bit error rate = (errors / bits). */
    /** */
    /** - If any requested eye pixel (data point) would fall outside the DAC range */
    /**   (0..255), the error value is set to -1 and the bit value remains 0.  The */
    /**   caller must recognize and ignore these data points. */
    /** */
    /** - C programmers may use the _*_GET(sed,x,y) macros to access the values */
    /**       in the arrays. */
    /**   Non-C programmers should use the accessor functions: */
    /** */

    int64_t * edErrorsp;  ///< Number of errors at each data point (eye pixel).
    int64_t * edBitsp;    ///< Total RX bits sampled at each data point.
    float  * edGradp;    ///< Gradient field for RX bits sampled at each data point.
    float  * edQvalp;    ///< Qinv value at each data point

    int64_t edMissionErrors[256];      ///< Num errors, mission channel
    int64_t edMissionBits[256];        ///< Num bits,   mission channel

    /* Phase center data: */
    char *edHardwareLog;      ///< Holds hardware info buffer.
    char *edPhaseCenterLog;  ///< Holds phase centering info.
    char *edComment;           ///< Holds any user comment string.

    xpSerdesEyeType_t     edEyeType;    ///< Type of measurement
    xpSerdesRxDataQual_t edDataQual;   ///< Data qual
    xpSerdesRxCmpMode_t  edCmpMode;    ///< Compare mode
    xpSerdesDfeState_t    edDfeState;   ///< DFE state during eye capture

    xpSerdesEyeVbtc_t edVbtc;    ///< Vertical BathTub Curve data, mission clock
    xpSerdesEyeHbtc_t edHbtc;    ///< Horizontal Bathtub Curve data
    float edDcBalance;                ///< Default is 0.5
    float edTransDensity;             ///< Default is 0.5
    uint32_t  edErrorThreshold;           ///< error threshold use by BTC calcs.

    int64_t edMinDwellBits;   ///< Minimum samples to measure. Default = 1e6.
    ///< Library enforces a reasonable minimum.
    int64_t edMaxDwellBits;   ///< Maximum samples to measure. Default = 1e6.
    BOOL   edFastDynamic;     ///< Use >ec_error_threshold rather than >0
    ///< for neighbor dwelling.
    int32_t edXShift;         ///< PI steps to shift eye center.
    double edCombinedBtcBer; ///< Eye center BER value.
    float edCombinedBtcQval; ///< Eye center Q value.
    float  edPhaseCenter;      /**< Phase location of eye center */
    int32_t edPhaseTableLen;   /**< Length of phase table */
    float *edPhaseTable;       /**< mUI position adjustment for each phase step location. */
    BOOL edGatherMean;        ///< Read/Write only eye points that bracket the 50% error point
    ///< (only non-zero points). Default = false.
    uint32_t edMeanDirection;     ///< Gather mean scan direction.
    int64_t edMeanThreshold;   ///< Optional gather mean threshold of calculated 50% value.
} xpSerdesEyeData_t;

/**
 * \brief Initialize a SerDes slice.
 */
typedef enum
{
    PRBS31_ILB,    ///< Run a PRBS check in ILB; leave SerDes in ILB running PRBS 31 data.
    PRBS31_ELB,    ///< Run a PRBS check in ILB; leave SerDes in ELB running PRBS 31 data.
    CORE_DATA_ILB, ///< Run a PRBS check in ILB; leave SerDes in ILB sending data from the ASIC core.
    CORE_DATA_ELB, ///< Run a PRBS check in ILB; leave SerDes in ELB sending data from the ASIC core.
    INIT_ONLY      ///< Don't check any errors during init, just init the serdes.
    ///< SerDes will be left in ELB mode sending data from the ASIC core.
} xpSerdesInitMode_t;

/**
 * \struct xpSerdesInitConfig_t
 * \brief structure for serdes init configuration
 */
typedef struct xpSerdesInitConfig
{
    BOOL sbusReset;    ///< Sbus reset
    BOOL initTx;    ///< init tx
    BOOL initRx;    ///< init rx
    xpSerdesInitMode_t initMode;    ///< serdes init mode
    uint txDivider;    ///< Tx divider
    uint rxDivider;    ///< Rx divider
    uint txWidth;      ///< TX width mode: 10, 20 or 40
    uint rxWidth;      ///< RX width mode: 10, 20 or 40
    BOOL txPhaseCal;   ///< tx phase calculation
    BOOL txOutputEn;   ///< tx output enable
    BOOL signalOkEn;   ///< signal Ok state enable
    int  signalOkThreshold;   ///< [0-15]
    BOOL enableTxGray;        /**< Enable Gray encoding on TX during initialization */
    BOOL enableRxGray;        /**< Enable Gray encoding on RX during initialization */
    BOOL enableTxPrecode;     /**< Enable pre-code on TX during initialization */
    BOOL enableRxPrecode;     /**< Enable pre-code on RX during initialization */
    BOOL skipCrc;
} xpSerdesInitConfig_t;

/**
 * \struct xpSerdesPllState_t
 * \brief SerDes PLL status information.
 */
typedef struct xpSerdesPllState
{
    uint32_t ready;         ///< Ready indicator, PLL is calibrated and ready for use.
    uint32_t calCode;      ///< Calibration code
    uint32_t bbGAIN;        ///< Bang-Bang gain value
    uint32_t intGAIN;       ///< Integrator gain value
    uint32_t divider;       ///< Divider setting
    int64_t estRate;    ///< Estimated Line Rate in bits per second
} xpSerdesPllState_t;

/**
 * \struct serdesPmdConfig_t
 * \brief Link Training Configuration struct
 */
typedef struct serdesPmdConfig
{
    uint32_t initialPre; ///< Number of pre-cursor DEC commands to issue after PRESET & before DFE tuning (max of 15)
    uint32_t initialPost;///< Number of post-cursor DEC commands to issue after PRESET & before DFE tuning (max of 15)

    xpSerdesPmdTrainMode_t
    train_mode;///< Select pmd link training mode to execute by serdes_pmd_train

    uint32_t sbusAddr;        ///< SBus Rx Address of SerDes to run training on, when asymmetric = TRUE this is Rx portion of the duplex link
    uint32_t asymTx;          ///< SBus Rx Address of SerDes that is the Tx portion of the duplex link, ignored if asymmetric_mode = FALSE

    BOOL disableTimeout;  ///< Disables the timeout timer
    BOOL disableTxeqAdj; ///< Disables requesting TxEq adjustments
    BOOL asymmetricMode;  ///< When enabled o_core_status & i_core_cntl used to transfer control/status
    ///<  channels to another SerDes that is the partner to form the duplex link
    BOOL invertRequests;  ///< Invert requests made to remote Tx, (DEC -> INC, INC -> DEC)

    BOOL resetParameters;    ///< Reset any previously configured parameters, ie. clause,lane,FEC*

    xpSerdesPmdClause_t clause; ///< CL72,CL84,CL92
    uint32_t
    lane; ///< When clause is set to CL92, defines which PMD lane this SerDes is
    uint32_t
    prbsSeed; ///< When non-zero overrides the default PRBS seed value

    /* FC specific configs - Ingored unless clause == FC16G */
    BOOL  ttFECreq;      ///< Set to transmit FEC request in TT training frames.
    BOOL  ttFECcap;      ///< Set to transmit FEC capability in TT training frames.
    BOOL  ttTF;          ///< Set to transmit Fixed TxEq in TT training frames.

} serdesPmdConfig_t;

/**
 * \struct xpSerdesPmdRequest_t
 * \brief Tap request & response tracking struct
 */
typedef struct xpSerdesPmdRequest
{
    uint32_t inc; ///< Number of inc requests
    uint32_t dec; ///< Number of dec requests
    uint32_t max; ///< Number of max responses
    uint32_t min; ///< Number of min responses
} xpSerdesPmdRequest_t;

/**
 * \struct xpSerdesPmdTapStats_t
 * \brief Statistics for a given transmitter (local or remote)
 */
typedef struct xpSerdesPmdTapStats
{
    uint32_t  preset;                 ///< Number of preset requests
    uint32_t  initialize;             ///< Number of initialize requests
    xpSerdesPmdRequest_t *tap[3];     ///< Stats for taps (pre,cursor,post)
} xpSerdesPmdTapStats_t;


/**
 * \struct xpSerdesPmdDebug_t
 * \brief Link Training Debug Information
 */
typedef struct xpSerdesPmdDebug
{
    BOOL reset; ///< Query statistics and then clear them (Note: also done when training is started)

    uint32_t rxMetric;              ///< RxEq metric

    xpSerdesPmdTapStats_t
    *lcl;    ///< Stats for change requests done by local training code
    xpSerdesPmdTapStats_t *remote; ///< Stats for change requests done by remote

    uint32_t lastRemoteRequest[8]; ///< Last 8 requests sent by remote partner
    uint32_t lastLocalRequest;     ///< Last request made by local training logic

} xpSerdesPmdDebug_t;

/**
 * \brief serdes interrupt flags
 */
typedef enum
{
    INT_ALL=0,       ///< TBD
    INT_FIRST=1,     ///< TBD
    INT_NOT_FIRST=2  ///< TBD
} xpIntFlags_t;

/**
 * \struct xpSerdesInt_t
 * \brief TBD
 */
typedef struct xpSerdesInt
{
    int interrupt;      ///< TBD
    int param;          ///< TBD
    int ret;            ///< TBD
    xpIntFlags_t flags; ///< interrupt flags
} xpSerdesInt_t;


/**
 * \struct xpSerdesDdr3TrainingValues_t
 * \brief DDR3 training values
 */
typedef struct xpSerdesDdr3TrainingValues
{
    uint32_t writeLeveling[16];    ///< TBD
    uint32_t readGateDelay[16];    ///< TBD
    uint32_t writePbdsStb[16];     ///< TBD
    uint32_t readPbdsStb[16];      ///< TBD
    uint32_t writePbds[16][9];     ///< TBD
    uint32_t readPbds[16][9];      ///< TBD
} xpSerdesDdr3TrainingValues_t;

/**
 * \struct xpSerdesAddr_t
 * \brief Internal hardware address structure.
 *        Not generally end-user accessible.
 *        Each value can be all ones to indicate broadcast to all at that level.
 *        Else the valid values are hardware dependent.
 */
typedef struct xpSerdesAddr_t
{
    uint32_t chip;   ///< Device number.
    uint32_t ring;   ///< Ring on device.
    uint32_t sbus;   ///< SBus address on ring.
    uint32_t lane;   ///< Lane at SBus address.

    struct xpSerdesAddr_t *
        next;  ///< Address of next serdesAddr_t element -- used to generate a linked list of addresses
} xpSerdesAddr_t;

/**
 * \struct xpSerdesPmroLoop_t
 * \brief TBD
 */
typedef struct xpSerdesPmroLoop
{
    int totalDelay;           ///< Total delay when all cells active
    int delay[16];             ///< Total delay when cell X is removed
} xpSerdesPmroLoop_t;

/**
 * \struct xpSerdesPmro_t
 * \brief TBD
 */
typedef struct xpSerdesPmro
{
    double clockFreq;         ///< Frequency of clock driving PMRO in MHz
    uint   repetitions;        ///< Loops run
    BOOL   disableLoop[4];    ///< Disables gathering of data for VT loop(s)
    BOOL   disableFineGrain; ///< Disables gathering of individual cell delays
    xpSerdesPmroLoop_t *loop[4]; ///< Struct of delay for each cell in VT loops
} xpSerdesPmro_t;

/**
 * \struct xpSerdesDfeTune_t
 * \brief If fixed[n] is set then the value[n] setting of the struct
 *        will be loaded before tuning is executed.
 *        NOTE: Not all versions of the firmware support all settings.
 */
typedef struct xpSerdesDfeTune
{
    /* order is dc, lf, hf, bw */
    BOOL fixed[4];      ///< TBD
    int32_t  value[4];  ///< TBD

    uint32_t dfeDisable; ///< When set, skip tuning of dfeTaps.

    xpSerdesDfeTuneMode_t tuneMode; ///< Select tuning mode to execute.

} xpSerdesDfeTune_t;

/**
 * \struct xpSerdesHealthCheckConfig_t
 * \brief TBD
 */
typedef struct xpSerdesHealthCheckConfig
{
    uint32_t verbose;    ///< TBD
    uint32_t failed;     ///< TBD

    uint32_t crc;            ///< TBD
    uint32_t spicoRunning;   ///< TBD
    uint32_t ready;          ///< TBD

    uint32_t txInvertCheck;  ///< TBD
    uint32_t txInvert;       ///< TBD
    uint32_t txOut;          ///< TBD

    uint32_t rxInvertCheck; ///< TBD
    uint32_t rxInvert;      ///< TBD
    uint32_t loopbackCheck; ///< TBD
    uint32_t loopback;      ///< TBD
    uint32_t elecIdleCheck; ///< TBD
    uint32_t signalOkCheck; ///< TBD
    uint32_t errorsCheck;   ///< TBD

} xpSerdesHealthCheckConfig_t;

/**
 * \struct xpSerdesLinkInitConfig_t
 * \brief TBD
 */
typedef struct xpSerdesLinkInitConfig
{
    BOOL info;   ///< print information when running
    BOOL dfeTune; ///< enable DFE tuning
    BOOL eye;    ///< gather eye information when complete

    /* SerDes A: */
    xpSerdes_t *serdesA; ///< TBD
    uint32_t addrA; ///< Address for SerDes A
    xpSerdesInitConfig_t *serdesConfigA; ///< config struct for SerDes A
    xpSerdesDfeState_t *serdesDfeConfigA; ///< serdes DFE config

    /* SerDes B: */
    xpSerdes_t *serdesB; ///< TBD
    uint32_t addrB; ///< Address for SerDes B
    xpSerdesInitConfig_t *serdesConfigB; ///< config struct for SerDes B
    xpSerdesDfeState_t *serdesDfeConfigB; ///< serdes DFE config
} xpSerdesLinkInitConfig_t;

/**
 * \brief TBD
 */
typedef enum
{
    SERDES_AN_HCD_1000BASE_KX,    ///< TBD
    SERDES_AN_HCD_10GBASE_KX4,    ///< TBD
    SERDES_AN_HCD_10GBASE_KR,     ///< TBD
    SERDES_AN_HCD_40GBASE_KR4,    ///< TBD
    SERDES_AN_HCD_40GBASE_CR4,    ///< TBD
    SERDES_AN_HCD_100GBASE_CR10,  ///< TBD
    SERDES_AN_HCD_100GBASE_KP4,   ///< TBD
    SERDES_AN_HCD_0X07,    ///< TBD
    SERDES_AN_HCD_100GBASE_KR4,    ///< TBD
    SERDES_AN_HCD_100GBASE_CR4,    ///< TBD
    SERDES_AN_HCD_0X0A,    ///< TBD
    SERDES_AN_HCD_0X0B,    ///< TBD
    SERDES_AN_HCD_0X0C,    ///< TBD
    SERDES_AN_HCD_0X0D,    ///< TBD
    SERDES_AN_HCD_0X0E,    ///< TBD
    SERDES_AN_HCD_NONE    ///< TBD
} xpSerdesAnHcd_t;

/**
 * \struct xpSerdesAnInfo_t
 * \brief TBD
 */
typedef struct xpSerdesAnInfo
{
    uint ipRev;
    BOOL enabled;
    xpSerdesAnHcd_t hcd;

} xpSerdesAnInfo_t;

/**
 * \struct xpSerdesCtle_t
 * \ brief Structure of ctrl values
 */
typedef struct xpSerdesCtle
{
    uint32_t hf;
    uint32_t lf;
    uint32_t dc;
    uint32_t bw;
} xpSerdesCtle_t;


/**
 * \brief get serdes aapl return code
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesAaplGetReturnCode(xpsDevice_t devId, uint8_t serdesId);

/**
 * \public
 * \brief Get default values of serdes eye configuration variables
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes id. valid values are 10-139
 * \param [out] serdesEyeConfigPtr pointer of xpSerdesEyeConfig_t type
 *
 * \return XP_NO_ERR on success
 */
XP_STATUS xpsSerdesGetEyeWithDefaultConfig(xpDevice_t devId, uint8_t serdesId,
                                           xpSerdesEyeConfig_t *serdesEyeConfigPtr);

/**
 * \brief set serdes aapl IP type
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesAaplSetIpType(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief get Serdes Hardware Info Format
 * \param [in] devId
 * \param [in] serdesId
 * \param [out] hwInfo
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesHardwareInfoFormat(xpsDevice_t devId, uint8_t serdesId,
                                      char *hwInfo);

/**
 * \brief serdes Diag Rw Test
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] cycles
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDiagSerdesRwTest(xpsDevice_t devId, uint8_t serdesId,
                                    int32_t cycles);

/**
 * \brief Serdes Diag Dump
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] binEnable
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDiagSerdesDump(xpsDevice_t devId, uint8_t serdesId,
                                  int32_t binEnable);

/**
 * \brief Serdes Diag1
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDiag1(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief get serdes aap log
 * \param [in] devId
 * \param [in] serdesId
 * \return [const]
 */
const char *xpsSerdesGetAaplLog(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Serdes Dfe Pause
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] rrEnabled
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDfePause(xpsDevice_t devId, uint8_t serdesId,
                            uint32_t *rrEnabled);

/**
 * \brief Serdes Dfe Wait
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDfeWait(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Serdes Dfe Wait Timeout
 * \param [in] devId Device Id. Valid values are 0-63
 * \param [in] serdesId serdes id. valid values are 10-139
 * \param [in] timeoutInMilliseconds timeout in milli seconds.
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsSerdesDfeWaitTimeout(xpsDevice_t devId, uint8_t serdesId,
                                  int32_t timeoutInMilliseconds);

/**
 * \brief Serdes Dfe Resume
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] rrEnable
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDfeResume(xpsDevice_t devId, uint8_t serdesId,
                             uint32_t rrEnable);

/**
 * \brief Serdes Pmd Train
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesPmdTrain(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Serdes Pmd Debug
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesPmdDebug(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Serdes Pmd Debug Print
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesPmdDebugPrint(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Serdes Command Exec
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] regAddr
 * \param [in] command
 * \param [in] serdesData
 * \param [in] recvDataBack
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesCommandExec(xpsDevice_t devId, uint8_t serdesId,
                               unsigned char regAddr, unsigned char command, uint32_t serdesData,
                               int32_t recvDataBack, uint32_t *serdesStatus);

/**
 * \brief serdes write
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] regAddr
 * \param [in] serdesData
 * \param [out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesWr(xpsDevice_t devId, uint8_t serdesId,
                      unsigned char regAddr, uint32_t serdesData,  uint32_t *serdesStatus);

/**
 * \brief Serdes Rmw
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] regAddr
 * \param [in] serdesData
 * \param [in] mask
 * \param [in,out] initialVal
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesRmw(xpsDevice_t devId, uint8_t serdesId,
                       unsigned char regAddr, uint32_t serdesData, uint32_t mask,
                       uint32_t *initialVal);

/**
 * \brief Serdes Rd
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] regAddr
 * \param [in,out] rdData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesRd(xpsDevice_t devId, uint8_t serdesId,
                      unsigned char regAddr, uint32_t *rdData);

/**
 * \brief Serdes Reset
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] hard
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesReset(xpsDevice_t devId, uint8_t serdesId, int32_t hard);

/**
 * \brief Serdes Diag
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDiag(xpsDevice_t devId, uint8_t serdesId,
                        uint32_t *serdesStatus);

/**
 * \brief Serdes State Dump
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesStateDump(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Get Serdes Eye Errors
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] xPoint
 * \param [in] yPoint
 * \param [in] errCnt
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeErrorsGet(xpsDevice_t devId, uint8_t serdesId,
                                uint32_t xPoint, uint32_t yPoint, int64_t *errCnt);

/**
 * \brief Get Serdes Eye Bits
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] xPoint
 * \param [in] yPoint
 * \param [in,out] bitCnt
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeBitsGet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t xPoint, uint32_t yPoint, int64_t *bitCnt);

/**
 * \brief Get Serdes Eye Grad
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] xPoint
 * \param [in] yPoint
 * \param [in,out] gradVal
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeGradGet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t xPoint, uint32_t yPoint, Float *gradVal);

/**
 * \brief Get Serdes Eye Qval
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] xPoint
 * \param [in] yPoint
 * \param [in,out] qVal
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeQvalGet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t xPoint, uint32_t yPoint, Float *qVal);

/**
 * \brief Print Serdes Eye Plot Log
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] level
 * \param [in,out] func
 * \param [in] line
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyePlotLogPrint(xpsDevice_t devId, uint8_t serdesId,
                                   xpSerdesLogType_t level, const char *func, int32_t line);

/**
 * \brief Print Serdes Eye Hbtc Log
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] level
 * \param [in,out] func
 * \param [in] line
 * \param [in,out] hbtcp
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeHbtcLogPrint(xpsDevice_t devId, uint8_t serdesId,
                                   xpSerdesLogType_t level, const char *func, int32_t line,
                                   xpSerdesEyeHbtc_t *hbtcp);

/**
 * \brief Print Serdes Eye Vbtc Log
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] level
 * \param [in,out] func
 * \param [in] line
 * \param [in,out] vbtcp
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeVbtcLogPrint(xpsDevice_t devId, uint8_t serdesId,
                                   xpSerdesLogType_t level, const char *func, int32_t line,
                                   xpSerdesEyeVbtc_t *vbtcp);

/**
 * \brief Read Serdes Eye Data File
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] filename
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeDataReadFile(xpsDevice_t devId, uint8_t serdesId,
                                   const char *filename);

/**
 * \brief Read Serdes Eye Data File
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] tcpPort
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesAacsServer(xpsDevice_t devId, uint8_t serdesId, int tcpPort);

/**
 * \brief Get Serdes Firmware Rev
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] fwRev
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetFirmwareRev(xpsDevice_t devId, uint8_t serdesId,
                                  uint32_t *fwRev);

/**
 * \brief Get Serdes Firmware Build Id
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] fwBldId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetFirmwareBuildId(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t *fwBldId);

/**
 * \brief Get Serdes Lsb Rev
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetLsbRev(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief Get serdes TxRx Ready
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] tx
 * \param [in,out] rx
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxRxReady(xpsDevice_t devId, uint8_t serdesId, int *tx,
                                int *rx);

/**
 * \brief set serdes TxRx Enable status
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] tx
 * \param [in] rx
 * \param [in] txOutput
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxRxEnable(xpsDevice_t devId, uint8_t serdesId,
                                 uint8_t tx, uint8_t rx, uint8_t txOutput);

/**
 * \brief get serdes Tx Output Enable
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] enable
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxOutputEnable(xpsDevice_t devId, uint8_t serdesId,
                                     int *enable);

/**
 * \brief set tx output enable status
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] enable
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxOutputEnable(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t enable);

/**
 * \brief set Serdes Tx Output Enable
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] txWidth
 * \param [in,out] rxWidth
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxRxWidth(xpsDevice_t devId, uint8_t serdesId,
                                int32_t *txWidth, int32_t *rxWidth);

/**
 * \brief set Serdes Tx Rx Width
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] txWidth
 * \param [in] rxWidth
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxRxWidth(xpsDevice_t devId, uint8_t serdesId,
                                int32_t txWidth, int32_t rxWidth);

/**
 * \brief set Serdes Tx Invert
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] invert
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t invert);

/**
 * \brief set Serdes Rx Invert
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] invert
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetRxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t invert);

/**
 * \brief Serdes Tx Inject Error
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] numBits
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesTxInjectError(xpsDevice_t devId, uint8_t serdesId,
                                 uint32_t numBits);

/**
 * \brief Serdes Rx Inject Error
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] numBits
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesRxInjectError(xpsDevice_t devId, uint8_t serdesId,
                                 uint32_t numBits);

/**
 * \brief set Serdes Rx Input Loopback
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] selectInternal
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetRxInputLoopback(xpsDevice_t devId, uint8_t serdesId,
                                      uint8_t selectInternal);

/**
 * \brief get Serdes Tx User Data
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] data
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxUserData(xpsDevice_t devId, uint8_t serdesId,
                                 long data[4]);

/**
 * \brief set Serdes Tx User Data
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] data
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxUserData(xpsDevice_t devId, uint8_t serdesId,
                                 long data[4]);

/**
 * \brief set Serdes Tx Data Sel
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] input
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxDataSel(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesTxDataSel_t input);

/**
 * \brief set Serdes Rx Cmp Data
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] cmpData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetRxCmpData(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpData_t cmpData);

/**
 * \brief Get Serdes Tx Eq
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] txEq
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxEq(xpsDevice_t devId, uint8_t serdesId,
                           xpSerdesTxEq_t *txEq);

/**
 * \brief Set serdes Tx Eq
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] txEq
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxEq(xpsDevice_t devId, uint8_t serdesId,
                           xpSerdesTxEq_t *txEq);

/**
 * \brief Set serdes Rx Term
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] term
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetRxTerm(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesRxTerm_t term);

/**
 * \brief Set serdes Rx Cmp Mode
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] mode
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetRxCmpMode(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpMode_t mode);

//XP_STATUS xpsSerdesSetSpicoClkSrc(xpsDevice_t devId, uint8_t serdesId, xpSerdesClk_t src);

/**
 * \brief Set serdes Tx Pll Clk Src
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] src
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxPllClkSrc(xpsDevice_t devId, uint8_t serdesId,
                                  xpSerdesTxPllClk_t src);

/**
 * \brief Serdes Mem Rd
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] type
 * \param [in] memAddr
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesMemRd(xpsDevice_t devId, uint8_t serdesId,
                         xpSerdesMemType_t type, uint32_t memAddr);

/**
 * \brief Serdes Mem Wr
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] type
 * \param [in] memAddr
 * \param [in] data
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesMemWr(xpsDevice_t devId, uint8_t serdesId,
                         xpSerdesMemType_t type, uint32_t memAddr, uint32_t data);

/**
 * \brief Serdes Mem Rmw
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] type
 * \param [in] memAddr
 * \param [in] data
 * \param [in] mask
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesMemRmw(xpsDevice_t devId, uint8_t serdesId,
                          xpSerdesMemType_t type, uint32_t memAddr, uint32_t data, uint32_t mask);

/**
 * \brief get serdes Signal Ok
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] reset
 * \param [in] signalOk
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetSignalOk(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t reset,  int *signalOk);

/**
 * \brief serdes Initialize Signal Ok
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] threshold
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesInitializeSignalOk(xpsDevice_t devId, uint8_t serdesId,
                                      int32_t threshold);

/**
 * \brief get Serdes Errors
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] type
 * \param [in] resetCountAfterGet
 * \param [in,out] errCnt
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetErrors(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesMemType_t type, uint8_t resetCountAfterGet, uint32_t *errCnt);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] getTxInvert
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t *getTxInvert);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] getRxInvert
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxInvert(xpsDevice_t devId, uint8_t serdesId,
                               uint8_t *getRxInvert);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] getRxInputLoopback
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxInputLoopback(xpsDevice_t devId, uint8_t serdesId,
                                      uint8_t *getRxInputLoopback);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] txDataSel
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxDataSel(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesTxDataSel_t *txDataSel);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] rxCmpData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxCmpData(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpData_t *rxCmpData);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] getRxTerm
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxTerm(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdesRxTerm_t *getRxTerm);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] rxCmpModeT
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxCmpMode(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesRxCmpMode_t *rxCmpModeT);

//XP_STATUS xpsSerdesGetSpicoClkSrc(xpsDevice_t devId, uint8_t serdesId, xpSerdesClk_t *spicoClkT);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] txPllClkT
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxPllClkSrc(xpsDevice_t devId, uint8_t serdesId,
                                  xpSerdesTxPllClk_t *txPllClkT);

/**
 * \brief xpsSerdesEyeDataWriteFile
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] filename
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesEyeDataWriteFile(xpsDevice_t devId, uint8_t serdesId,
                                    const char *filename);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] file
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeDataWrite(xpsDevice_t devId, uint8_t serdesId,
                                FILE *file);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] fileName
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeVbtcWrite(xpsDevice_t devId, uint8_t serdesId,
                                const char *fileName);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] fileName
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeHbtcWrite(xpsDevice_t devId, uint8_t serdesId,
                                const char *fileName);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] file
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyePlotWrite(xpsDevice_t devId, uint8_t serdesId,
                                FILE *file);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] vbtcp
 * \param [out] eyeData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeVbtcFormat(xpsDevice_t devId, uint8_t serdesId,
                                 xpSerdesEyeVbtc_t *vbtcp, char *eyeData);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] hbtcp
 * \param [out] eyeData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeHbtcFormat(xpsDevice_t devId, uint8_t serdesId,
                                 xpSerdesEyeHbtc_t *hbtcp, char *eyeData);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [out] eyeData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyePlotFormat(xpsDevice_t devId, uint8_t serdesId,
                                 char *eyeData);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] dataCol
 * \param [in,out] results
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeVbtcExtrapolate(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t dataCol, xpSerdesEyeVbtc_t *results);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] dataRow
 * \param [in,out] results
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeHbtcExtrapolate(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t dataRow, xpSerdesEyeHbtc_t *results);

/**
 * \brief xpsSerdesCtrlrStatus
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [out] st
 * \param [in] no_cache
 *
 * \return XP_STATUS
 */

XP_STATUS xpsSerdesCtrlrStatus(xpsDevice_t devId, uint8_t serdesId,
                               xpSerdesStatus_t *st, bool no_cache);

/**
 * \brief xpsSerdesCtrlrRunning
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [out] runStatus
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrRunning(xpsDevice_t devId, uint8_t serdesId,
                                uint32_t *runStatus);

/**
 * \brief xpsSerdesCtrlrReset
 *
 * \param [in] devId
 * \param [in] serdesId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrReset(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief xpsSerdesCtrlrUploadSwapImage
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] words
 * \param [in] rom
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrUploadSwapImage(xpsDevice_t devId, uint8_t serdesId,
                                        int32_t words, const int32_t *rom);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] ramBist
 * \param [in] words
 * \param [in] rom
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesFirmwareUpload(xpsDevice_t devId, uint8_t serdesId,
                                  uint8_t ramBist, int32_t words, const int32_t *rom);

/**
 * \brief xpsSerdesCtrlrInt
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] intNum
 * \param [in] param
 * \param [out] intStatus
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrInt(xpsDevice_t devId, uint8_t serdesId, int32_t intNum,
                            int32_t param, uint32_t *intStatus);

/**
 * \brief xpsSerdesCtrlrIntArray
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] numElements
 * \param [out] interrupts
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrIntArray(xpsDevice_t devId, uint8_t serdesId,
                                 int32_t numElements, xpSerdesInt_t *interrupts);

/**
 * \brief xpsSerdesCtrlrCrc
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [out] crcResult
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrCrc(xpsDevice_t devId, uint8_t serdesId,
                            uint32_t *crcResult);

/**
 * \brief xpsSerdesCtrlrRamBist
 *
 * \param [in] devId
 * \param [in] serdesId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrRamBist(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] dfeRunning
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDfeRunning(xpsDevice_t devId, uint8_t serdesId,
                              uint8_t* dfeRunning);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] signalOkEnable
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetSignalOkEnable(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t *signalOkEnable);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetSignalOkThreshold(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeGet(xpsDevice_t devId, uint8_t serdesId);


/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] newValue
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetAsyncCancelFlag(xpsDevice_t devId, uint8_t serdesId,
                                      int32_t newValue);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetFirmwareBuild(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] addrStruct
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesAddrInitBroadcast(xpsDevice_t devId, uint8_t serdesId,
                                     xpSerdesAddr_t *addrStruct, uint32_t *serdesStatus);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] addrStruct
 * \param [in] type
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesDeviceInfo(xpsDevice_t devId, uint8_t serdesId,
                              xpSerdesAddr_t *addrStruct, xpSerdesIpType_t type);

//XP_STATUS xpsSerdesSerdesCtrlrGetRefclk(xpsDevice_t devId, uint8_t serdesId, uint32_t addr);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] reg
 * \param [in] data
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSerdesWrFlush(xpsDevice_t devId, uint8_t serdesId,
                                 unsigned char reg, uint32_t data, uint32_t *serdesStatus);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] input
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxDataSel(xpsDevice_t devId, uint8_t serdesId,
                                xpSerdesTxDataSel_t input);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetTxLineEncoding(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t *serdesStatus);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxLineEncoding(xpsDevice_t devId, uint8_t serdesId,
                                     uint8_t *serdesStatus);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] txWidth
 * \param [in] rxWidth
 * \param [in] txEncoding
 * \param [in] rxEncoding
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSetTxRxDataWidth(xpsDevice_t devId, uint8_t serdesId,
                                    int32_t txWidth, int32_t rxWidth, xpSerdesEncodingMode_t txEncoding,
                                    xpSerdesEncodingMode_t rxEncoding);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] bits
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSlipBits(xpsDevice_t devId, uint8_t serdesId, uint32_t bits,
                            uint8_t *serdesStatus);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] bits
 * \param [in] applyAtInit
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSlipRxPhase(xpsDevice_t devId, uint8_t serdesId,
                               uint32_t bits, uint8_t applyAtInit, uint8_t *serdesStatus);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] bits
 * \param [in] applyAtInit
 * \param [in,out] serdesStatus
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSlipTxPhase(xpsDevice_t devId, uint8_t serdesId,
                               uint32_t bits, uint8_t applyAtInit, uint8_t *serdesStatus);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] control
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesTuneInit(xpsDevice_t devId, uint8_t serdesId,
                            xpSerdesDfeTune_t *control);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] eyeMetric
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesEyeGetSimpleMetric(xpsDevice_t devId, uint8_t serdesId,
                                      uint32_t *eyeMetric);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetRxLiveData(xpsDevice_t devId, uint8_t serdesId);

/**
 * \brief xpsSerdesCtrlrDiag
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] cycles
 * \param [out] serdesStatus
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrDiag(xpsDevice_t devId, uint8_t serdesId,
                             int32_t cycles, uint32_t *serdesStatus);
/**
 * \brief xpsSerdesFirmwareUpload_2
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] serdesRomSize
 * \param [in] serdesRom
 * \param [in] serdesCtrlrRomSize
 * \param [in] serdesCtrlrRom
 * \param [in] sdiRomSize
 * \param [in] sdiRom
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesFirmwareUpload_2(xpsDevice_t devId, uint8_t serdesId,
                                    int32_t serdesRomSize, const int32_t *serdesRom, int32_t serdesCtrlrRomSize,
                                    const int32_t *serdesCtrlrRom, int32_t sdiRomSize, const int32_t *sdiRom);
/**
 * \brief xpsSerdesCtrlrWaitForUpload
 *
 * \param [in] devId
 * \param [in] serdesId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrWaitForUpload(xpsDevice_t devId, uint8_t serdesId);
/**
 * \brief xpsSerdesTwiWaitForComplete
 *
 * \param [in] devId
 * \param [in] serdesId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesTwiWaitForComplete(xpsDevice_t devId, uint8_t serdesId);

/**
 * \public
 * \brief Get values of ctle(hf,lf,dc,bw).
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes id. valid values are 10-139
 * \param [in] ctleVal structure of ctle values
 *
 * \return XP_STATUS
*/
XP_STATUS xpsSerdesGetSerdesCtleVal(xpDevice_t devId, uint8_t serdesId,
                                    xpSerdesCtle_t* ctleVal);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] serdesRomFile
 * \param [in,out] serdesCtrlrRomFile
 * \param [in,out] sdiRomFile
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesFirmwareUploadFile_2(xpsDevice_t devId, uint8_t serdesId,
                                        const char *serdesRomFile, const char *serdesCtrlrRomFile,
                                        const char *sdiRomFile);

/**
 * \brief xpsSerdesCtrlrIntCheck
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] caller
 * \param [in] line
 * \param [in] intNum
 * \param [in] param
 * \param [in] serdesStatus
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesCtrlrIntCheck(xpsDevice_t devId, uint8_t serdesId,
                                 const char *caller, int32_t line, int32_t intNum, int32_t param,
                                 uint8_t *serdesStatus);

/**
 * \brief xpsSerdesSetStruct
 *
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] serdesData
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesSetStruct(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdes_t *serdesData);
/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in,out] serdesData
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesGetStruct(xpsDevice_t devId, uint8_t serdesId,
                             xpSerdes_t *serdesData);

/**
 * \brief To get serdes eye Height and Width value
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] serdesId - serdes id
 * \param [out] eyeHeight - eye height value
 * \param [out] eyeWidth - eye width value
 * \return [XP_STATUS]
 */

XP_STATUS xpsSerdesGetEyeMeasurement(xpsDevice_t devId, uint8_t serdesId,
                                     uint32_t *eyeHeight, uint32_t *eyeWidth);

/**
 * \public
 * \brief To get serdes eye Height and Width value
 *        This API use user provided values for serdes eye configuration parameters.
 *        if serdesEyeConfigPtr is NULL, than it use default values for all serdes eye configuration parameters.
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] serdesId - serdes id. Valid values are 10-139
 * \param [in] serdesEyeConfigPtr pointer of xpSerdesEyeConfig_t type
 * \param [out] eyeHeight eye height value
 * \param [out] eyeWidth eye width value
 *
 * \return XP_STATUS on success
 */
XP_STATUS xpsSerdesGetEyeMeasurementWithCustomConfig(xpDevice_t devId,
                                                     uint8_t serdesId, xpSerdesEyeConfig_t *serdesEyeConfigPtr, uint32_t *eyeHeight,
                                                     uint32_t *eyeWidth);

/**
 * \brief
 * \param [in] devId
 * \param [in] serdesId
 * \param [in] timeoutVal
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesTimeoutSet(xpsDevice_t devId, uint8_t serdesId,
                              uint32_t timeoutVal);

/**
 * \brief
 * \param [in] devId
 * \param [in,out] temp
 * \return [XP_STATUS]
 */
XP_STATUS xpsSerdesSensorGetTemperature(xpsDevice_t devId, int32_t *temp);

/**

* \brief  To get serdes clock divider
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] divider clock divider
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesCtrlrGetSerdesClockDivider(xpDevice_t devId,
                                              uint8_t serdesId, int *divider);

/**

* \brief  To get serdes connection status
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] status serdes status
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesConnectionStatus(xpDevice_t devId, uint8_t serdesId,
                                    int *status);

/**

* \brief  To close serdes connection
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesCloseConnection(xpDevice_t devId, uint8_t serdesId);

/**

* \brief  To set serdes control running flag
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [in] isSerdesCtrlRunning
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSetCtrlrRunningFlag(xpDevice_t devId, uint8_t serdesId,
                                       int isSerdesCtrlRunning);

/**

* \brief  To get serdes control running flag
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] status serdes control running status
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetCtrlrRunningFlag(xpDevice_t devId, uint8_t serdesId,
                                       int *status);

/**

* \brief  To get interrupt revision of a serdes
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] intNum Interrupt number
* \param [in] serdesId Serdes Id
* \param [out] rev interrupt revision of a serdes
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetInterruptRev(xpDevice_t devId, int intNum,
                                   uint8_t serdesId, int *rev);

/**

* \brief  To get sd revision of a serdes
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] rev sd revision of a serdes
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetSdrev(xpDevice_t devId, uint8_t serdesId, int *rev);

/**

* \brief  To init serdes quickly
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] divider
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS, On success returns XP_NO_ERR
*/
XP_STATUS xpsSerdesInitQuick(xpDevice_t devId, uint32_t divider,
                             uint8_t serdesId);

/**

* \brief  To resume the serdes
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] initialState
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS, On success returns XP_NO_ERR
*/
XP_STATUS xpsSerdesResume(xpDevice_t devId, int initialState, uint8_t serdesId);

/**

* \brief  To get serdes run state
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] runState serdes run state
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesHalt(xpDevice_t devId, uint8_t serdesId, int *runState);

/**

* \brief  To get serdes pmd status
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] status serdes pmd status
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesPmdStatus(xpDevice_t devId, uint8_t serdesId, int *status);

/**

* \brief  To write serdes eye plot in a file
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] fileName file in which eye plot would be written
* \param [in] serdesId Serdes Id
* \return XP_STATUS status On success XP_NO_ERR
*
*/
XP_STATUS xpsSerdesEyeGradientPlotWrite(xpDevice_t devId, const char *fileName,
                                        uint8_t serdesId);

/**

* \brief  To set serdes Tx datapath encoding
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] grayCode
* \param [in] preCode
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSetTxDatapathEncoding(xpDevice_t devId, BOOL grayCode,
                                         BOOL preCode, uint8_t serdesId);

/**

* \brief  To get serdes Tx datapath encoding
*
* \param [in] devId device Id. Valid values are 0-63
* \param [out] grayCode
* \param [out] preCode
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetTxDatapathEncoding(xpDevice_t devId, BOOL *grayCode,
                                         BOOL *preCode, uint8_t serdesId);

/**

* \brief  To set serdes Rx datapath encoding
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] grayCode
* \param [in] preCode
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSetRxDatapathEncoding(xpDevice_t devId, BOOL grayCode,
                                         BOOL preCode, uint8_t serdesId);

/**

* \brief  To get serdes Rx datapath encoding
*
* \param [in] devId device Id. Valid values are 0-63
* \param [out] grayCode
* \param [out] preCode
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetRxDatapathEncoding(xpDevice_t devId, BOOL *grayCode,
                                         BOOL *preCode, uint8_t serdesId);

/**
* \brief Reads 80 bits from the RX data stream.
*        The data is captured using the Rx compare circuitry, with the
*        side-effect that rx_cmp_data is set to OFF.
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] 80-bit data (4 x 20). data[0] bit 0 is first bit.
*
* \return On success - return XP_NO_ERR
*/
XP_STATUS xpsSerdesGetRxData(xpDevice_t devId, uint8_t serdesId, long data[4]);

/**
 * \public
 * \brief This API is used to determine the connection status of media.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId
 * \param [out] isConnected 1->if media attached/0->if not attached
 *
 * \return On success - return XP_NO_ERR
 */
XP_STATUS xpsSerdesGetMediaStatus(xpDevice_t devId, uint8_t serdesId,
                                  uint8_t *isConnected);

/**

* \brief  To get serdes frequency lock status
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] locked serdes frequency lock status
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetFrequencyLock(xpDevice_t devId, uint8_t serdesId,
                                    int *locked);

/**

* \brief  To get serdes OK live signal
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId Serdes Id
* \param [out] isLive serdes OK live signal
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesGetSignalOkLive(xpDevice_t devId, uint8_t serdesId,
                                   int *isLive);

/**

* \brief  To get serdes error flag
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] reset
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS On success returns XP_NO_ERR
*/
XP_STATUS xpsSerdesGetErrorFlag(xpDevice_t devId, BOOL reset, uint8_t serdesId);

/**

* \brief  To enable serdes core
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] txRxEnable Tx/Rx enable
* \param [in] lowPowerEnable Low power enable
* \param [in] serdesId Serdes Id
*
* \return XP_STATUS, On success returns XP_NO_ERR
*/
XP_STATUS xpsSerdesEnableCoreToControl(xpDevice_t devId, BOOL txRxEnable,
                                       BOOL lowPowerEnable, uint8_t serdesId);

/**

* \brief  To init serdes address
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] addrStruct serdes address structure
* \param [in] serdesId serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesAddrInit(xpDevice_t devId, xpSerdesAddr_t *addrStruct,
                            uint8_t serdesId);

/**

* \brief  To start serdes temperature sensor
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSensorStartTemperature(xpDevice_t devId, uint8_t serdesId);

/**

* \brief  To read temperature reading from serdes temp sensor
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
* \param [out] temp temperature reading from serdes temp sensor
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSensorWaitTemperature(xpDevice_t devId, uint8_t serdesId,
                                         int *temp);

/**

* \brief  To get voltage from serdes sensor
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
* \param [out] mVolt voltage from serdes sensor in milli volt
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSensorGetVoltage(xpDevice_t devId, uint8_t serdesId,
                                    int *mVolt);

/**

* \brief  To start serdes voltage sensor
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSensorStartVoltage(xpDevice_t devId, uint8_t serdesId);

/**

* \brief  To read voltage reading from serdes voltage sensor
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
* \param [out] mVolt voltage reading from serdes voltage sensor in milli volt
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesSensorWaitVoltage(xpDevice_t devId, uint8_t serdesId,
                                     int *mVolt);

/**

* \brief  To broadcast serdes interrupt with mask
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
* \param [in] addrMask
* \param [in] intNum interrupt number
* \param [in] param
* \param [in] count
* \param [in] args it's variable length of arguments
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesBroadcastIntWMask(xpDevice_t devId, uint8_t serdesId,
                                     uint32_t addrMask, int intNum, int param, int count, int *args);

/**

* \brief  To broadcast serdes interrupt
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id
* \param [in] intNum interrupt number
* \param [in] param
* \param [in] count
* \param [in] args it's variable length of arguments
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesBroadcastInt(xpDevice_t devId, uint8_t serdesId, int intNum,
                                int param, int count, int *args);

/**
* \public
* \brief This API used to set electrical idle thresold
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes ID
* \param [in] eiThreshold electrical threshold value
* \param [out] intStatus depends on the interrupt.For 28nm, it is often the same as the interrupt number
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesElectricalIdleThresholdSet(xpDevice_t devId,
                                              uint8_t serdesId, uint8_t eiThreshold, uint32_t *intStatus);

/**
* \public
* \brief This API used to get pmro metric
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id.
* \param [out] pmroMetric pmro metric value
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesPmroMetricGet(xpDevice_t devId, uint8_t serdesId,
                                 uint32_t *pmroMetric);

/**
* \public
* \brief This API used to get serdes dfe status
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id.
* \param [out] dfeStatus dfeStatus value
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesDfeStatusGet(xpDevice_t devId, uint8_t serdesId,
                                int32_t *dfeStatus);

/**
* \public
* \brief This API can be used to get dfe state
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id.
* \param [in] dfeState dfe state data
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesDfeStateGet(xpDevice_t devId, uint8_t serdesId,
                               xpSerdesDfeState_t *dfeState);

/**
* \public
* \brief This API used to print the dfe state
*
* \param [in] devId device Id. Valid values are 0-63
* \param [in] serdesId serdes Id.
* \param [in] dfeState  dfe state data
* \param [in] singleLine single line
*
* \return XP_STATUS status On success XP_NO_ERR
*/
XP_STATUS xpsSerdesDfeStatePrint(xpDevice_t devId, uint8_t serdesId,
                                 xpSerdesDfeState_t *dfeState, uint8_t singleLine);

/**
 * \public
 * \brief Loads phase data from a file.
 *        Does not require AAPL connection to any hardware.
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes Id.
 * \param [in] filename Name of file containing phase data
 * \param [out] dataRdPhaseVal data read phase value, 0 on success, non-zero
 *        on error, and logs specific problem
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesEyeDataReadPhase(xpDevice_t devId, uint8_t serdesId,
                                    const char *filename, int32_t* dataRdPhaseVal);

/**
 * \public
 * \brief Checks if signal is loss during dfe tuning.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes Id.
 * \param [out] isDfeLosDetected TRUE if dfe is LOS detected, FALSE otherwise.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSerdesDfeLos(xpDevice_t devId, uint8_t serdesId,
                          uint8_t* isDfeLosDetected);

/**
 * \public
 * \brief Register user defined logging function
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes Id.
 * \param [in] logFunc user defined logging function
 * \param [in] logOpenFunc user defined log open function
 * \param [in] logCloseFunc user defined log close function
 *
 * \return XP_STATUS
 */
XP_STATUS xpsRegisterLoggingFunc(xpDevice_t devId, uint8_t serdesId,
                                 userLogFunc LogFunc, \
                                 userLogOpenFunc LogOpenFunc, userLogCloseFunc LogCloseFunc);


/**
 * \public
 * \brief Get a simple eye metric threshold value in range [0..1000]
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes id. valid values are 10-139
 * \param [out] eyeMetricThreshold Threshold value of eye metric
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsSerdesEyeSimpleMetricThresholdGet(xpsDevice_t devId,
                                               uint8_t serdesId, uint32_t *eyeMetricThreshold);
/**
 * \public
 * \brief Set a simple eye metric threshold value in range [0..1000]
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] serdesId serdes id. valid values are 10-139
 * \param [in] eyeMetricThreshold Threshold value of eye metric
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsSerdesEyeSimpleMetricThresholdSet(xpsDevice_t devId,
                                               uint8_t serdesId, uint32_t eyeMetricThreshold);


#ifdef __cplusplus
}
#endif

#endif //_XPSLINK_H_



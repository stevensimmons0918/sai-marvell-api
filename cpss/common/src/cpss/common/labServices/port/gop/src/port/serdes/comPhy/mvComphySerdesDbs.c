/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file mvComphySerdesDbs.c \
*
* @brief Comphy SerDes related Databases
*
* @version   1
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

typedef struct {
        GT_U8 hwsEnum;
        GT_U8 mcesdEnum;
} MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD;

#define MV_HWS_COMPHY_INIT_ARR_PTR_MAC(_myArr , _arrPtr , _arrLen) \
    _arrPtr = _myArr;                                              \
    _arrLen = sizeof(_myArr)/sizeof(_myArr[0]);

#ifdef C12GP41P2V
#include "C12GP41P2V/mcesdC12GP41P2V_Defs.h"
static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C12GP41P2V_hwsToMcesdSerdesSpeedDb[]=
{
    {_5G,        C12GP41P2V_SERDES_5G      },
    {_5_15625G,  C12GP41P2V_SERDES_5P15625G},
    {_1_25G,     C12GP41P2V_SERDES_1P25G   },
    {_3_125G,    C12GP41P2V_SERDES_3P125G  },
    {_6_25G,     C12GP41P2V_SERDES_6P25G   },
    {_10_3125G,  C12GP41P2V_SERDES_10P3125G},
    {_11_5625G,  C12GP41P2V_SERDES_11P5625G},
    {_12_5G,     C12GP41P2V_SERDES_12P5G   }
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C12GP41P2V_hwsToMcesdRefClockFreqDb[]=
{
    {_25Mhz,        C12GP41P2V_REFFREQ_25MHZ    },
    {_125Mhz,       C12GP41P2V_REFFREQ_125MHZ   },
    {_156dot25Mhz,  C12GP41P2V_REFFREQ_156P25MHZ}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C12GP41P2V_hwsToMcesdRefClockSourceDb[]=
{
    {PRIMARY,   C12GP41P2V_REFCLK_SEL_GROUP1},
    {SECONDARY, C12GP41P2V_REFCLK_SEL_GROUP2}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C12GP41P2V_hwsToMcesdDataBusWidthDb[]=
{
    {_10BIT_ON,  C12GP41P2V_DATABUS_20BIT},
    {_10BIT_OFF, C12GP41P2V_DATABUS_20BIT},
    {_20BIT_ON,  C12GP41P2V_DATABUS_20BIT},
    {_40BIT_ON,  C12GP41P2V_DATABUS_40BIT}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C12GP41P2V_hwsToMcesdPatternDb[]=
{
    {_1T,         C12GP41P2V_PAT_JITTER_1T  },
    {_2T,         C12GP41P2V_PAT_JITTER_2T  },
    {_4T,         C12GP41P2V_PAT_JITTER_4T  },
    {_5T,         C12GP41P2V_PAT_JITTER_5T  },
    {_8T,         C12GP41P2V_PAT_JITTER_8T  },
    {_10T,        C12GP41P2V_PAT_JITTER_10T },
    {PRBS7,       C12GP41P2V_PAT_PRBS7      },
    {PRBS9,       C12GP41P2V_PAT_PRBS9      },
    {PRBS15,      C12GP41P2V_PAT_PRBS15     },
    {PRBS23,      C12GP41P2V_PAT_PRBS23     },
    {PRBS31,      C12GP41P2V_PAT_PRBS31     },
    {Other,       C12GP41P2V_PAT_USER       },
    {PRBS11,      C12GP41P2V_PAT_PRBS11     },
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C12GP41P2V_hwsToMcesdLoopbackDb[]=
{
    {SERDES_LP_AN_TX_RX,  C12GP41P2V_PATH_LOCAL_ANALOG_LB},
    {SERDES_LP_DISABLE,   C12GP41P2V_PATH_EXTERNAL       },
    {SERDES_LP_DIG_RX_TX, C12GP41P2V_PATH_FAR_END_LB     },
    {SERDES_LP_DISABLE,   C12GP41P2V_PATH_UNKNOWN        },
};
#endif /*C12GP41P2V*/

#ifdef C28GP4X1
#include "C28GP4X1/mcesdC28GP4X1_Defs.h"
static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X1_hwsToMcesdSerdesSpeedDb[]=
{
    {_1_25G,        C28GP4X1_SERDES_1P25G     },
    {_2_578125,     C28GP4X1_SERDES_2P57813G  },
    {_3_125G,       C28GP4X1_SERDES_3P125G    },
    {_5G,           C28GP4X1_SERDES_5G        },
    {_5_15625G,     C28GP4X1_SERDES_5P15625G  },
    {_10G,          C28GP4X1_SERDES_10G       },
    {_10_3125G,     C28GP4X1_SERDES_10P3125G  },
    {_20_625G,      C28GP4X1_SERDES_20P625G_LC},
    {_25_78125G,    C28GP4X1_SERDES_25P78125G },
    {_26_5625G,     C28GP4X1_SERDES_26P5625G  }
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X1_hwsToMcesdRefClockFreqDb[]=
{
    {_25Mhz,        C28GP4X1_REFFREQ_25MHZ    },
    {_125Mhz,       C28GP4X1_REFFREQ_125MHZ   },
    {_156dot25Mhz,  C28GP4X1_REFFREQ_156P25MHZ}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X1_hwsToMcesdRefClockSourceDb[]=
{
    {PRIMARY,   C28GP4X1_REFCLK_SEL_GROUP1},
    {SECONDARY, C28GP4X1_REFCLK_SEL_GROUP2}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X1_hwsToMcesdDataBusWidthDb[]=
{
    {_40BIT_ON, C28GP4X1_DATABUS_40BIT}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X1_hwsToMcesdPatternDb[]=
{
    {_1T,         C28GP4X1_PAT_JITTER_1T  },
    {_2T,         C28GP4X1_PAT_JITTER_2T  },
    {_4T,         C28GP4X1_PAT_JITTER_4T  },
    {_5T,         C28GP4X1_PAT_JITTER_5T  },
    {_8T,         C28GP4X1_PAT_JITTER_8T  },
    {_10T,        C28GP4X1_PAT_JITTER_10T },
    {PRBS7,       C28GP4X1_PAT_PRBS7      },
    {PRBS9,       C28GP4X1_PAT_PRBS9      },
    {PRBS15,      C28GP4X1_PAT_PRBS15     },
    {PRBS23,      C28GP4X1_PAT_PRBS23     },
    {PRBS31,      C28GP4X1_PAT_PRBS31     },
    {Other,       C28GP4X1_PAT_USER       },
    {PRBS11,      C28GP4X1_PAT_PRBS11     },
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X1_hwsToMcesdLoopbackDb[]=
{
    {SERDES_LP_AN_TX_RX,  C28GP4X1_PATH_LOCAL_ANALOG_LB},
    {SERDES_LP_DISABLE,   C28GP4X1_PATH_EXTERNAL       },
    {SERDES_LP_DIG_RX_TX, C28GP4X1_PATH_FAR_END_LB     },
    {SERDES_LP_DISABLE,   C28GP4X1_PATH_UNKNOWN        },
};
#endif /*C28GP4X1*/

#ifdef C28GP4X4
#include "C28GP4X4/mcesdC28GP4X4_Defs.h"
static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X4_hwsToMcesdSerdesSpeedDb[]=
{
    {_1_25G,        C28GP4X4_SERDES_1P25G     },
    {_2_578125,     C28GP4X4_SERDES_2P57813G  },
    {_3_125G,       C28GP4X4_SERDES_3P125G    },
    {_5G,           C28GP4X4_SERDES_5G        },
    {_5_15625G,     C28GP4X4_SERDES_5P15625G  },
    {_10G,          C28GP4X4_SERDES_10G       },
    {_10_3125G,     C28GP4X4_SERDES_10P3125G  },
    {_20_625G,      C28GP4X4_SERDES_20P625G_LC},
    {_25_78125G,    C28GP4X4_SERDES_25P78125G },
    {_26_5625G,     C28GP4X4_SERDES_26P5625G  }

};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X4_hwsToMcesdRefClockFreqDb[]=
{
    {_25Mhz,        C28GP4X4_REFFREQ_25MHZ    },
    {_125Mhz,       C28GP4X4_REFFREQ_125MHZ   },
    {_156dot25Mhz,  C28GP4X4_REFFREQ_156P25MHZ}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X4_hwsToMcesdRefClockSourceDb[]=
{
    {PRIMARY,   C28GP4X4_REFCLK_SEL_GROUP1},
    {SECONDARY, C28GP4X4_REFCLK_SEL_GROUP2}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X4_hwsToMcesdDataBusWidthDb[]=
{
    {_40BIT_ON, C28GP4X4_DATABUS_40BIT}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X4_hwsToMcesdPatternDb[]=
{
    {_1T,         C28GP4X4_PAT_JITTER_1T  },
    {_2T,         C28GP4X4_PAT_JITTER_2T  },
    {_4T,         C28GP4X4_PAT_JITTER_4T  },
    {_5T,         C28GP4X4_PAT_JITTER_5T  },
    {_8T,         C28GP4X4_PAT_JITTER_8T  },
    {_10T,        C28GP4X4_PAT_JITTER_10T },
    {PRBS7,       C28GP4X4_PAT_PRBS7      },
    {PRBS9,       C28GP4X4_PAT_PRBS9      },
    {PRBS15,      C28GP4X4_PAT_PRBS15     },
    {PRBS23,      C28GP4X4_PAT_PRBS23     },
    {PRBS31,      C28GP4X4_PAT_PRBS31     },
    {Other,       C28GP4X4_PAT_USER       },
    {PRBS11,      C28GP4X4_PAT_PRBS11     },
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C28GP4X4_hwsToMcesdLoopbackDb[]=
{
    {SERDES_LP_AN_TX_RX,  C28GP4X4_PATH_LOCAL_ANALOG_LB},
    {SERDES_LP_DISABLE,   C28GP4X4_PATH_EXTERNAL       },
    {SERDES_LP_DIG_RX_TX, C28GP4X4_PATH_FAR_END_LB     },
    {SERDES_LP_DISABLE,   C28GP4X4_PATH_UNKNOWN        },
};
#endif /*C28GP4X4*/

#ifdef C112GX4
#include "C112GX4/mcesdC112GX4_Defs.h"
static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C112GX4_hwsToMcesdSerdesSpeedDb[]=
{
    {_1_25G,         C112GX4_SERDES_1P25G    },
    {_2_578125,      C112GX4_SERDES_2P5G     },
    {_3_125G,        C112GX4_SERDES_3P125G   },
    {_5G,            C112GX4_SERDES_5G       },
    {_5_15625G,      C112GX4_SERDES_5P15625G },
    {_6_25G,         C112GX4_SERDES_6P25G    },
    {_10G,           C112GX4_SERDES_10G      },
    {_10_3125G,      C112GX4_SERDES_10P3125G },
    {_20_625G,       C112GX4_SERDES_20P625G  },
    {_25_78125G,     C112GX4_SERDES_25P78125G},
    {_26_5625G,      C112GX4_SERDES_26P5625G },
    {_26_5625G_PAM4, C112GX4_SERDES_53P125G  }
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C112GX4_hwsToMcesdRefClockFreqDb[]=
{
    {_25Mhz,        C112GX4_REFFREQ_25MHZ    },
    {_125Mhz,       C112GX4_REFFREQ_125MHZ   },
    {_156dot25Mhz,  C112GX4_REFFREQ_156P25MHZ}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C112GX4_hwsToMcesdRefClockSourceDb[]=
{
    {PRIMARY,   C112GX4_REFCLK_SEL_GROUP1},
    {SECONDARY, C112GX4_REFCLK_SEL_GROUP2}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C112GX4_hwsToMcesdDataBusWidthDb[]=
{
    {_40BIT_ON, C112GX4_DATABUS_40BIT},
    {_80BIT_ON, C112GX4_DATABUS_80BIT}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C112GX4_hwsToMcesdPatternDb[]=
{
    {_1T,         C112GX4_PAT_JITTER_1T  },
    {_2T,         C112GX4_PAT_JITTER_2T  },
    {_4T,         C112GX4_PAT_JITTER_4T  },
    {_5T,         C112GX4_PAT_JITTER_5T  },
    {_8T,         C112GX4_PAT_JITTER_8T  },
    {_10T,        C112GX4_PAT_JITTER_10T },
    {PRBS7,       C112GX4_PAT_PRBS7      },
    {PRBS9,       C112GX4_PAT_PRBS9      },
    {PRBS13,      C112GX4_PAT_PRBS13_0   },
    {PRBS15,      C112GX4_PAT_PRBS15     },
    {PRBS23,      C112GX4_PAT_PRBS23     },
    {PRBS31,      C112GX4_PAT_PRBS31     },
    {Other,       C112GX4_PAT_USER       },
    {PRBS11,      C112GX4_PAT_PRBS11     },
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD C112GX4_hwsToMcesdLoopbackDb[]=
{
    {SERDES_LP_AN_TX_RX,  C112GX4_PATH_LOCAL_ANALOG_LB},
    {SERDES_LP_DISABLE,   C112GX4_PATH_EXTERNAL       },
    {SERDES_LP_DIG_RX_TX, C112GX4_PATH_FAR_END_LB     },
    {SERDES_LP_DISABLE,   C112GX4_PATH_UNKNOWN        },
};
#endif /*C112GX4*/

#ifdef N5XC56GP5X4
#include "N5XC56GP5X4/mcesdN5XC56GP5X4_Defs.h"
static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD N5XC56GP5X4_hwsToMcesdSerdesSpeedDb[]=
{
    {_1_25G,                N5XC56GP5X4_SERDES_1P25G   },
    {_1_25G_SR_LR,          N5XC56GP5X4_SERDES_1P25G   },
    {_2_578125,             N5XC56GP5X4_SERDES_2P57812G},
    {_3_125G,               N5XC56GP5X4_SERDES_3P125G  },
    {_4_25G,                N5XC56GP5X4_SERDES_4P25G   },
    {_5G,                   N5XC56GP5X4_SERDES_5G      },
    {_5_15625G,             N5XC56GP5X4_SERDES_5P15625G},
    {_6_25G,                N5XC56GP5X4_SERDES_6P25G   },
    {_7_5G,                 N5XC56GP5X4_SERDES_7P5G    },
    {_10G,                  N5XC56GP5X4_SERDES_10G     },
    {_10_3125G,             N5XC56GP5X4_SERDES_10P3125G},
    {_10_3125G_SR_LR,       N5XC56GP5X4_SERDES_10P3125G},
    {_12_1875G,             N5XC56GP5X4_SERDES_12P1875G},
    {_12_5G,                N5XC56GP5X4_SERDES_12P5G   },
    {_12_8906G,             N5XC56GP5X4_SERDES_12P8906G},
    {_20_625G,              N5XC56GP5X4_SERDES_20P625G },
    {_25_78125G,            N5XC56GP5X4_SERDES_25P7812G},
    {_25_78125G_SR_LR,      N5XC56GP5X4_SERDES_25P7812G},
    {_26_5625G,             N5XC56GP5X4_SERDES_26P5625G},
    {_27_5G,                N5XC56GP5X4_SERDES_275G    },
    {_28_05G,               N5XC56GP5X4_SERDES_28P05G  },
    {_26_5625G_PAM4,        N5XC56GP5X4_SERDES_53P125G },
    {_26_5625G_PAM4_SR_LR,  N5XC56GP5X4_SERDES_53P125G },
    {_28_125G_PAM4,         N5XC56GP5X4_SERDES_56P25G  }
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD N5XC56GP5X4_hwsToMcesdRefClockFreqDb[]=
{
    {_25Mhz,        N5XC56GP5X4_REFFREQ_25MHZ },
    {_125Mhz,       N5XC56GP5X4_REFFREQ_125MHZ},
    {_156dot25Mhz,  N5XC56GP5X4_REFFREQ_156MHZ}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD N5XC56GP5X4_hwsToMcesdRefClockSourceDb[]=
{
    {PRIMARY,   N5XC56GP5X4_REFCLK_SEL_G1},
    {SECONDARY, N5XC56GP5X4_REFCLK_SEL_G2}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD N5XC56GP5X4_hwsToMcesdDataBusWidthDb[]=
{
    {_40BIT_ON, N5XC56GP5X4_DATABUS_40BIT},
    {_80BIT_ON, N5XC56GP5X4_DATABUS_80BIT}
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD N5XC56GP5X4_hwsToMcesdPatternDb[]=
{
    {_1T,         N5XC56GP5X4_PAT_JITTER_1T  },
    {_2T,         N5XC56GP5X4_PAT_JITTER_2T  },
    {_4T,         N5XC56GP5X4_PAT_JITTER_4T  },
    {_5T,         N5XC56GP5X4_PAT_JITTER_5T  },
    {_8T,         N5XC56GP5X4_PAT_JITTER_8T  },
    {_10T,        N5XC56GP5X4_PAT_JITTER_10T },
    {PRBS7,       N5XC56GP5X4_PAT_PRBS7      },
    {PRBS9,       N5XC56GP5X4_PAT_PRBS9      },
    {PRBS15,      N5XC56GP5X4_PAT_PRBS15     },
    {PRBS23,      N5XC56GP5X4_PAT_PRBS23     },
    {PRBS31,      N5XC56GP5X4_PAT_PRBS31     },
    {Other,       N5XC56GP5X4_PAT_USER       },
    {PRBS11,      N5XC56GP5X4_PAT_PRBS11     },
    {PRBS13,      N5XC56GP5X4_PAT_PRBS13_0   },
};

static const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD N5XC56GP5X4_hwsToMcesdLoopbackDb[]=
{
    {SERDES_LP_AN_TX_RX,  N5XC56GP5X4_PATH_LOCAL_LB       },
    {SERDES_LP_DIG_TX_RX, N5XC56GP5X4_PATH_NEAR_END_LB    },
    {SERDES_LP_DIG_RX_TX, N5XC56GP5X4_PATH_FAR_END_LB     },
    {SERDES_LP_DISABLE,   N5XC56GP5X4_PATH_EXTERNAL       },
};
#endif /*N5XC56GP5X4*/

static GT_STATUS mvHwsComphyGetConversionDatabase
(
    IN  GT_U8                                        devNum,
    IN  MV_HWS_SERDES_TYPE                           serdesType,
    IN  MV_HWS_COMPHY_MCESD_ENUM_TYPE                enumType,
    OUT const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD  *conversionDb[],
    OUT GT_U32                                       *conversionDbSize
)
{
    GT_UNUSED_PARAM(devNum);
    *conversionDb = NULL;
    switch( enumType )
    {
        case MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED:
            IF_C12GP41P2V(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C12GP41P2V_hwsToMcesdSerdesSpeedDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X1(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X1_hwsToMcesdSerdesSpeedDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X4_hwsToMcesdSerdesSpeedDb,*conversionDb,*conversionDbSize));
            IF_C112GX4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C112GX4_hwsToMcesdSerdesSpeedDb,*conversionDb,*conversionDbSize));
            IF_N5XC56GP5X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(N5XC56GP5X4_hwsToMcesdSerdesSpeedDb,*conversionDb,*conversionDbSize));
            break;
        case MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ:
            IF_C12GP41P2V(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C12GP41P2V_hwsToMcesdRefClockFreqDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X1(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X1_hwsToMcesdRefClockFreqDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X4_hwsToMcesdRefClockFreqDb,*conversionDb,*conversionDbSize));
            IF_C112GX4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C112GX4_hwsToMcesdRefClockFreqDb,*conversionDb,*conversionDbSize));
            IF_N5XC56GP5X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(N5XC56GP5X4_hwsToMcesdRefClockFreqDb,*conversionDb,*conversionDbSize));
            break;
        case MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE:
            IF_C12GP41P2V(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C12GP41P2V_hwsToMcesdRefClockSourceDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X1(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X1_hwsToMcesdRefClockSourceDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X4_hwsToMcesdRefClockSourceDb,*conversionDb,*conversionDbSize));
            IF_C112GX4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C112GX4_hwsToMcesdRefClockSourceDb,*conversionDb,*conversionDbSize));
            IF_N5XC56GP5X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(N5XC56GP5X4_hwsToMcesdRefClockSourceDb,*conversionDb,*conversionDbSize));
            break;
        case MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH:
            IF_C12GP41P2V(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C12GP41P2V_hwsToMcesdDataBusWidthDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X1(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X1_hwsToMcesdDataBusWidthDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X4_hwsToMcesdDataBusWidthDb,*conversionDb,*conversionDbSize));
            IF_C112GX4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C112GX4_hwsToMcesdDataBusWidthDb,*conversionDb,*conversionDbSize));
            IF_N5XC56GP5X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(N5XC56GP5X4_hwsToMcesdDataBusWidthDb,*conversionDb,*conversionDbSize));
            break;
        case MV_HWS_COMPHY_MCESD_ENUM_PATTERN:
            IF_C12GP41P2V(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C12GP41P2V_hwsToMcesdPatternDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X1(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X1_hwsToMcesdPatternDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X4_hwsToMcesdPatternDb,*conversionDb,*conversionDbSize));
            IF_C112GX4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C112GX4_hwsToMcesdPatternDb,*conversionDb,*conversionDbSize));
            IF_N5XC56GP5X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(N5XC56GP5X4_hwsToMcesdPatternDb,*conversionDb,*conversionDbSize));
            break;
        case MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK:
            IF_C12GP41P2V(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C12GP41P2V_hwsToMcesdLoopbackDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X1(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X1_hwsToMcesdLoopbackDb,*conversionDb,*conversionDbSize));
            IF_C28GP4X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C28GP4X4_hwsToMcesdLoopbackDb,*conversionDb,*conversionDbSize));
            IF_C112GX4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(C112GX4_hwsToMcesdLoopbackDb,*conversionDb,*conversionDbSize));
            IF_N5XC56GP5X4(serdesType,MV_HWS_COMPHY_INIT_ARR_PTR_MAC(N5XC56GP5X4_hwsToMcesdLoopbackDb,*conversionDb,*conversionDbSize));
            break;

        default:
            return GT_NOT_SUPPORTED;
    }

    if( NULL == *conversionDb )
    {
        return GT_NOT_IMPLEMENTED;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyConvertHwsToMcesdType function
* @endinternal
*
* @brief   Convert type from HWS to MCESD
*
* @param[in] devNum                   - system device number
* @param[in] serdesType               - serdesType (Comphy only)
* @param[in] enumType                 - enum type to convert
* @param[in] hwsEnum                  - source hws to convert
* @param[out] mcesdEnum               - dest mcesd to convert
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyConvertHwsToMcesdType
(
    IN  GT_U8                            devNum,
    IN  MV_HWS_SERDES_TYPE               serdesType,
    IN  MV_HWS_COMPHY_MCESD_ENUM_TYPE    enumType,
    IN  GT_U32                           hwsEnum,
    OUT GT_U32                           *mcesdEnum
)
{
    const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD *hwsToMcesdDb = NULL;
    GT_U32 hwsToMcesdDbSize = 0;
    GT_U32 i;

    GT_UNUSED_PARAM(devNum);

    CHECK_STATUS(mvHwsComphyGetConversionDatabase(devNum, serdesType ,enumType ,&hwsToMcesdDb ,&hwsToMcesdDbSize));

    for( i = 0; i < hwsToMcesdDbSize; i++ )
    {
        if( hwsToMcesdDb[i].hwsEnum == hwsEnum )
        {
            *mcesdEnum = hwsToMcesdDb[i].mcesdEnum;
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}

/**
* @internal mvHwsComphyConvertMcesdToHwsType function
* @endinternal
*
* @brief   Convert type from MCESD to HWS
*
* @param[in] devNum                   - system device number
* @param[in] serdesType               - serdesType (Comphy only)
* @param[in] enumType                 - enum type to convert
* @param[in] mcesdEnum                - source mcesd to convert
* @param[out] hwsEnum                 - dest hws to convert
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyConvertMcesdToHwsType
(
    IN  GT_U8                            devNum,
    IN  MV_HWS_SERDES_TYPE               serdesType,
    IN  MV_HWS_COMPHY_MCESD_ENUM_TYPE    enumType,
    IN  GT_U32                           mcesdEnum,
    OUT GT_U32                           *hwsEnum
)
{
    const MV_HWS_COMPHY_ENUMS_CONVERSION_RECORD *mcesdToHwsDb = NULL;
    GT_U32 mcesdToHwsDbSize = 0;
    GT_U32 i;

    GT_UNUSED_PARAM(devNum);

    CHECK_STATUS(mvHwsComphyGetConversionDatabase(devNum, serdesType ,enumType ,&mcesdToHwsDb ,&mcesdToHwsDbSize));

    for( i = 0; i < mcesdToHwsDbSize; i++ )
    {
        if( mcesdToHwsDb[i].mcesdEnum == mcesdEnum )
        {
            *hwsEnum = mcesdToHwsDb[i].hwsEnum;
            return GT_OK;
        }
    }
    return GT_NOT_FOUND;
}


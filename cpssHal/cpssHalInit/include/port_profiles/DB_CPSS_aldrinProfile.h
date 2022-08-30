{
    /* port mapping  for aldrin copied from aldrinDefaultMap (gtDbDxBobcat2.c) also base on cpssTypes.h mapping for C81F11AB
     * macNum is same as interface num in the mapping array*/
    /*                                                                  portNum macNum  txQNum  interfaceMode                       interfaceSpeed */
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {0,   0,        0,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {1,   1,        1,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {2,     2,      2,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {3,     3,      3,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {4,     4,      4,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {5,     5,      5,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {6,     6,      6,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {7,     7,      7,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {8,     8,      8,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {9,     9,      9,  INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {10,    10,     10, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {11,    11,     11, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {12,    12,     12, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {13,    13,     13, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {14,    14,     14, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {15,    15,     15, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {16,    16,     16, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {17,    17,     17, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {18,    18,     18, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {19,    19,     19, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {20,    20,     20, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {21,    21,     21, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {22,    22,     22, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {23,    23,     23, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {24,    24,     24, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {25,    25,     25, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {26,    26,     26, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {27,    27,     27, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {28,    28,     28, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {29,    29,     29, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {30,    30,     30, INT_FOR_10G,    _10G_SPEED}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,            .profileValue.portMap = {31,    31,     31, INT_FOR_10G,    _10G_SPEED}},
    /* CPU Port */
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,        .profileValue.portMap = {63,    127,    63, INT_FOR_10G,        10}},

    // from the AppDemo
    //static APPDEMO_SERDES_LANE_POLARITY_STC  aldrinA0_DB_PolarityArray[] =
    /* Polarity                                                                 laneNum  invertTx    invertRx*/
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 1,    GT_TRUE,    GT_TRUE  }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 2,    GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 3,    GT_TRUE,    GT_TRUE  }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 4,    GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 5,    GT_TRUE,    GT_TRUE  }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 6,    GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 7,    GT_FALSE,   GT_TRUE  }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 9,    GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 13,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 14,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 16,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 17,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 22,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 24,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 25,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 27,   GT_TRUE,    GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity ={ 29,   GT_TRUE,    GT_FALSE }},

    /* simulation */
    //{.profileType = PROFILE_TYPE_SIM_INIFILE_E,   .profileValue.sim_inifile = "-i ./iniFiles/aldrin_x2_pss_wm.ini"          },
    {.profileType = PROFILE_TYPE_SIM_INIFILE_E, .profileValue.sim_inifile = "-i ./iniFiles/aldrin_pss_wm.ini"         },
    {.profileType = PROFILE_TYPE_HW_SKU_E, .profileValue.hwTableSizes = &hw_sku_profiles_ald[0]},
    /* last */
    {.profileType = PROFILE_TYPE_LAST_E,        .profileValue.no_param = 0 }
};

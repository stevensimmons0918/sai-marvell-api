{
/*   Device 0
     1G port                                                           portNum 
                                                                       |   macNum 
                                                                       |   |   txQNum 
                                                                       |   |   |  interfaceMode                     interfaceSpeed  existsPhy                                             frontPanelPortIdx
                                                                       |   |   |  |                                 |               |                                                                     |    */
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 0,  0,  0, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {1, 0x00, CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E},  0}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 1,  1,  1, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {1, 0x01, CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E},  1}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 2,  2,  2, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {1, 0x02, CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E},  2}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 3,  3,  3, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {1, 0x03, CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E},  3}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 4,  4,  4, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0},  4}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 5,  5,  5, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0},  5}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 6,  6,  6, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0},  6}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 7,  7,  7, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0},  7}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 8,  8,  8, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0},  8}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 9,  9,  9, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0},  9}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {10, 10, 10, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 10}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {11, 11, 11, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 11}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {12, 12, 12, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0}, 12}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {13, 13, 13, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 13}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {14, 14, 14, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 14}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {15, 15, 15, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 15}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {16, 16, 16, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 16}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {17, 17, 17, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 17}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {18, 18, 18, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 18}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {19, 19, 19, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 19}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {20, 20, 20, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 20}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {21, 21, 21, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 21}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {22, 22, 22, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 22}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {23, 23, 23, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 23}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {24, 24, 24, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 24}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {25, 25, 25, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 25}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {26, 26, 26, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 26}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {27, 27, 27, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 27}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {28, 28, 28, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 28}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {29, 29, 29, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 29}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {30, 30, 30, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 30}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {31, 31, 31, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 31}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = {32 ,32 ,32, CPSS_PORT_INTERFACE_MODE_SGMII_E, 10, .phyInfo = {0, 0, 0, 0}, 32}},

    /* CPU Port */
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {63, 127, 63, PROFILE_INTERFACE_MODE_KR_E, 10}},

    /* Polarity                                                                 laneNum  invertTx    invertRx*/
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  0,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  1,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  2,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  3,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  4,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  5,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  6,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  7,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  8,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  9,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 10,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 11,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 12,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 13,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 14,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 15,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 16,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 17,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 18,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 19,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 19,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 20,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 21,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 22,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 23,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 24,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 25,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 26,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 27,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 28,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 29,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 29,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 30,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 31,     GT_FALSE,   GT_FALSE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 32,     GT_FALSE,   GT_FALSE   }},

    /* simulation */
    /* {.profileType = PROFILE_TYPE_SIM_INIFILE_E,   .profileValue.sim_inifile = "-e xcat3_b2b_pss_wm.ini"             }, */
    {.profileType = PROFILE_TYPE_SIM_INIFILE_E, .profileValue.sim_inifile = "-i ./iniFiles/ac3x_fujitsu_small.ini"          },
    {.profileType = PROFILE_TYPE_HW_SKU_E, .profileValue.hwTableSizes = &hw_sku_profiles_ald[0]},

    /* last */
    {.profileType = PROFILE_TYPE_LAST_E,        .profileValue.no_param = 0 }

};

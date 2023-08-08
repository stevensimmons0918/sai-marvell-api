{
    /*                                                               portNum          macNum  txQNum            interfaceMode      interfaceSpeed       */
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 28, ALDRIN2_MAC_AND_TXQ_PORT_MAC(28), PROFILE_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0}, 28 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 30, ALDRIN2_MAC_AND_TXQ_PORT_MAC(30), PROFILE_INTERFACE_MODE_1000BASE_X_E, 10, .phyInfo = {0, 0, 0, 0}, 30 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 39, ALDRIN2_MAC_AND_TXQ_PORT_MAC(39), PROFILE_INTERFACE_MODE_SR_LR_E, 10, .phyInfo = {0, 0, 0, 0}, 39 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 44, ALDRIN2_MAC_AND_TXQ_PORT_MAC(44), PROFILE_INTERFACE_MODE_SR_LR_E, 10, .phyInfo = {0, 0, 0, 0}, 44 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 45, ALDRIN2_MAC_AND_TXQ_PORT_MAC(45), PROFILE_INTERFACE_MODE_SR_LR_E, 10, .phyInfo = {0, 0, 0, 0}, 45 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 46, ALDRIN2_MAC_AND_TXQ_PORT_MAC(46), PROFILE_INTERFACE_MODE_SR_LR_E, 10, .phyInfo = {0, 0, 0, 0}, 46 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E, .profileValue.portMap = { 47, ALDRIN2_MAC_AND_TXQ_PORT_MAC(47), PROFILE_INTERFACE_MODE_SGMII_E, 25, .phyInfo = {0, 0, 0, 0}, 47 }},

    //CPU SDMA PORTs
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E, .profileValue.portMap = { 63, 0, 99, PROFILE_INTERFACE_MODE_KR_E, 10 }},
    
    /* Polarity                                                                lane   invTx    invRx */
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 28, GT_FALSE, GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 30, GT_FALSE, GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 39, GT_FALSE, GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 44, GT_FALSE, GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 45, GT_FALSE, GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 46, GT_FALSE, GT_FALSE }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E, .profileValue.serdes_polarity = { 47, GT_FALSE, GT_FALSE }},

    /* simulation */
    {.profileType = PROFILE_TYPE_SIM_INIFILE_E, .profileValue.sim_inifile  = "-i iniFiles/aldrin2_xl_fujitsu_large.ini" },
    {.profileType = PROFILE_TYPE_HW_SKU_E,      .profileValue.hwTableSizes = &hw_sku_profiles_aldrin2_fl[0] },
    /* last */
    {.profileType = PROFILE_TYPE_LAST_E,        .profileValue.no_param = 0 }
};

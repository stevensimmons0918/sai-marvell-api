{
    /* port mapping */

    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            0, 0,      0,  PROFILE_INTERFACE_MODE_SR_LR_E,     100,
            .phyInfo = {0, 0, 0, 0}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            1, 1,      1,  PROFILE_INTERFACE_MODE_SR_LR_E,     100,
            .phyInfo = {0, 0, 0, 0}
        }
    },
    {
        .profileType = PROFILE_TYPE_CASCADE_PORT_MAP_E,    .profileValue.portMap = {
            2,     2,      2,  PROFILE_INTERFACE_MODE_KR2_E,       250,
            .phyInfo = {0, 0, 0, 0}
        }
    },
    //TODO : How to skip unused lanes??
    //{.profileType = PROFILE_TYPE_CASCADE_PORT_MAP_E,  .profileValue.portMap = {3,     3,      3,  PROFILE_INTERFACE_MODE_KR2_E,       250}},
    {
        .profileType = PROFILE_TYPE_CASCADE_PORT_MAP_E,    .profileValue.portMap = {
            4,     4,      4,  PROFILE_INTERFACE_MODE_KR2_E,       250,
            .phyInfo = {0, 0, 0, 0}
        }
    },
    //{.profileType = PROFILE_TYPE_CASCADE_PORT_MAP_E,  .profileValue.portMap = {5,     5,      5,  PROFILE_INTERFACE_MODE_KR2_E,       250}},
    {
        .profileType = PROFILE_TYPE_CASCADE_PORT_MAP_E,    .profileValue.portMap = {
            6,     6,      6,  PROFILE_INTERFACE_MODE_KR2_E,       250,
            .phyInfo = {0, 0, 0, 0}
        }
    },
    //{.profileType = PROFILE_TYPE_CASCADE_PORT_MAP_E,  .profileValue.portMap = {7,     7,      7,  PROFILE_INTERFACE_MODE_KR2_E,       250}},
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            8,     8,      8,  PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x00,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            9,     9,      9,  PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x01,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            10,    10,     10, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x02,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            11,    11,     11, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x03,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            12,    12,     12, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x04,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            13,    13,     13, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x05,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            14,    14,     14, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x06,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            15,    15,     15, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x07,   CPSS_PHY_SMI_INTERFACE_0_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            16,    16,     16, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x0c,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            17,    17,     17, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x0d,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            18,    18,     18, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x0e,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            19,    19,     19, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x0f,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            20,    20,     20, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x08,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            21,    21,     21, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x09,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            22,    22,     22, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x0a,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            23,    23,     23, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x0b,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            24,    24,     24, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x04,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            25,    25,     25, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x05,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            26,    26,     26, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x06,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            27,    27,     27, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x07,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            28,    28,     28, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x00,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            29,    29,     29, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x01,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            30,    30,     30, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x02,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },
    {
        .profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = {
            31,    31,     31, PROFILE_INTERFACE_MODE_QSGMII_E,        10,
            .phyInfo = {1,  0x03,   CPSS_PHY_SMI_INTERFACE_1_E, CPSS_PHY_XSMI_INTERFACE_MAX_E}
        }
    },


    /* CPU Port */
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {63,    127,    63, PROFILE_INTERFACE_MODE_KR_E,        10}},

    /* Polarity                                                                 laneNum  invertTx    invertRx*/
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  0,     GT_TRUE,    GT_FALSE   }},  // Need to confirm Lane 0
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = {  8,     GT_FALSE,   GT_TRUE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 12,     GT_FALSE,   GT_TRUE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 16,     GT_FALSE,   GT_TRUE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 20,     GT_FALSE,   GT_TRUE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 24,     GT_FALSE,   GT_TRUE   }},
    {.profileType = PROFILE_TYPE_LANE_SWAP_E,   .profileValue.serdes_polarity = { 28,     GT_FALSE,   GT_TRUE   }},

    /* simulation */
    {.profileType = PROFILE_TYPE_SIM_INIFILE_E, .profileValue.sim_inifile = "-e xcat3_b2b_pss_wm.ini"             },

    /* last */
    {.profileType = PROFILE_TYPE_LAST_E,        .profileValue.no_param = 0 }
};

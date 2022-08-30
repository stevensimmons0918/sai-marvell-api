{
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 0, 0,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  0 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 1, 8,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  1 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 2, 16,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  2 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 3, 24,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  3 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 4, 32,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  4 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 5, 40,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  5 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 6, 48,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  6 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 7, 56,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  7 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 8, 64,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0},  8 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 9, 72,  0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 9 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 10, 80, 0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 10}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 11, 88, 0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 11}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 12, 96, 0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 12}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 13, 104, 0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 13}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 14, 112, 0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 14}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 15, 120, 0, PROFILE_INTERFACE_MODE_KR_E, 250, .phyInfo = {0, 0, 0, 0}, 15}},

    //10G CPU Ports
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 16, 128,  0, PROFILE_INTERFACE_MODE_KR_E, 100, .phyInfo = {0, 0, 0, 0}, 16}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 17, 135,  0, PROFILE_INTERFACE_MODE_KR_E, 100, .phyInfo = {0, 0, 0, 0}, 17}},

    // CPU PORT
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {60,    136,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {61,    137,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {62,    138,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {63,    139,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},

    /* simulation */
    {.profileType = PROFILE_TYPE_SIM_INIFILE_E, .profileValue.sim_inifile = "-i ./iniFiles/falcon_6_4_A0_wm.ini"},
    {.profileType = PROFILE_TYPE_HW_SKU_E, .profileValue.hwTableSizes = &hw_sku_profiles_falcon[0]},

    /* last */
    {.profileType = PROFILE_TYPE_LAST_E, .profileValue.no_param = 0}

};

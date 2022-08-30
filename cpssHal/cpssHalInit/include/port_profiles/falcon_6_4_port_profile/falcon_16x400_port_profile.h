{
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 0,  0,   0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0},  0 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 8,  8,   0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0},  8 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 16, 16,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 16 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 24, 24,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 24 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 32, 32,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 32 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 40, 40,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 40 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 48, 48,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 48 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 56, 56,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 56 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 64, 64,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 64 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 72, 72,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 72 }},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 80, 80,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 80}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 88, 88,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 88}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 96, 96,  0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 96}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 104, 104, 0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 104}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 112, 112, 0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 112}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 120, 120, 0, PROFILE_INTERFACE_MODE_KR8_E, 4000, .phyInfo = {0, 0, 0, 0}, 120}},

    //10G CPU Ports
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 128, 128, 0, PROFILE_INTERFACE_MODE_KR_E, 100, .phyInfo = {0, 0, 0, 0}, 128}},
    {.profileType = PROFILE_TYPE_PORT_MAP_E,    .profileValue.portMap = { 129, 135, 0, PROFILE_INTERFACE_MODE_KR_E, 100, .phyInfo = {0, 0, 0, 0}, 129}},

    // CPU PORT
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {63,     136,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {130,    137,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {131,    138,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},
    {.profileType = PROFILE_TYPE_CPU_PORT_MAP_E,    .profileValue.portMap = {132,    139,    0,  PROFILE_INTERFACE_MODE_KR_E,        10}},

    /* simulation */
    {.profileType = PROFILE_TYPE_SIM_INIFILE_E, .profileValue.sim_inifile = "-i ./iniFiles/falcon_6_4_A0_wm.ini"},
    {.profileType = PROFILE_TYPE_HW_SKU_E, .profileValue.hwTableSizes = &hw_sku_profiles_falcon[2]},

    /* last */
    {.profileType = PROFILE_TYPE_LAST_E, .profileValue.no_param = 0}

};

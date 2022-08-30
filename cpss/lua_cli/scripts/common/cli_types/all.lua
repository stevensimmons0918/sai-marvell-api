require("common/cli_types/dev_port")
require("common/cli_types/devID")
require("common/cli_types/port_range")
require("common/cli_types/mac")
require("common/cli_types/hexstring")
require("common/cli_types/num_range")
require("common/cli_types/filename")
require("common/cli_types/ipv4")
require("common/cli_types/ipv6")
require("common/cli_types/types_led")
require("common/cli_types/log_enums")
require("common/cli_types/type_cpss_enable")
require("common/cli_types/trafficGeneratorTypes")
require("common/cli_types/hostname")
require("common/cli_types/eventTable_types")
require("common/cli_types/cpu_mdc_frequency_mode")


CLI_type_dict["GT_U32"] = {
    checker = CLI_check_param_number,
    min=0,
    max=4294967295,
    help="32 bit unsigned integer value"
}

CLI_type_dict["GT_U16"] = {
    checker = CLI_check_param_number,
    min=0,
    max=65535,
    help="16 bit unsigned integer value"
}

CLI_type_dict["GT_U8"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help="8 bit unsigned integer value"
}

CLI_type_dict["priority"] = {
    checker = CLI_check_param_number,
    min=0,
    max=7,
    help = "Enter priority 0..7"
}


CLI_type_dict["enable_disable"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Disable or enable\n",
    enum = {
        ["enable"] = { value=true, help="True" },
        ["disable"] = { value=false, help="False" }
   }
}

CLI_type_dict["bool"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "true or false\n",
    enum = {
        ["true"] = { value=true, help="true" },
        ["false"] = { value=false, help="false" }
   }
}

CLI_type_dict["dumpMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "HW access mode",
    enum = {
        ["read"] = {
            value = 0,
            help  = "read register access"
        },
        ["write"] = {
            value = 1,
            help  = "write register access"
        },
        ["both"] = {
            value = 2,
            help  = "both read and write register access"
        }
    }
}

CLI_type_dict["GT_SW_DEV_NUM"] = {
    checker = CLI_check_param_number,
    min=0,
    max=255,
    help="Device number"
}

CLI_type_dict["minFragSize"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Minimal fragment size for preemption",
    enum = {
        ["64"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E",
            help  = "64 byte minimal fragment for preemption size"
        },
        ["128"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_128_BYTE_E",
            help  = "128 byte minimal fragment for preemption size"
        },
        ["192"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_192_BYTE_E",
            help  = "192 byte minimal fragment for preemption size"
        },
        ["256"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_256_BYTE_E",
            help  = "256 byte minimal fragment for preemption size"
        },
        ["320"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_320_BYTE_E",
            help  = "320 byte minimal fragment for preemption size"
        },
        ["384"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_384_BYTE_E",
            help  = "384 byte minimal fragment for preemption size"
        },
        ["448"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_448_BYTE_E",
            help  = "448 byte minimal fragment for preemption size"
        },
        ["512"] = {
            value = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_512_BYTE_E",
            help  = "512 byte minimal fragment for preemption size"
        }
    }
}

CLI_type_dict["packetCommand"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "paket command",
    enum = {
        ["CPSS_PACKET_CMD_FORWARD_E"] = {
            value = "CPSS_PACKET_CMD_FORWARD_E",
            help  = "CPSS_PACKET_CMD_FORWARD_E"
        },
        ["CPSS_PACKET_CMD_MIRROR_TO_CPU_E"] = {
            value = "CPSS_PACKET_CMD_MIRROR_TO_CPU_E",
            help  = "CPSS_PACKET_CMD_MIRROR_TO_CPU_E"
        },
        ["CPSS_PACKET_CMD_TRAP_TO_CPU_E"] = {
            value = "CPSS_PACKET_CMD_TRAP_TO_CPU_E",
            help  = "CPSS_PACKET_CMD_TRAP_TO_CPU_E"
        },
        ["CPSS_PACKET_CMD_DROP_HARD_E"] = {
            value =  "CPSS_PACKET_CMD_DROP_HARD_E",
            help  = "CPSS_PACKET_CMD_DROP_HARD_E"
        }
    }
}


CLI_type_dict["tdCode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "tail drop code",
    enum = {
        ["CPSS_NET_FIRST_USER_DEFINED_E"] = {
            value = "CPSS_NET_FIRST_USER_DEFINED_E",
            help  = "CPSS_NET_FIRST_USER_DEFINED_E"
        },
        ["CPSS_NET_USER_DEFINED_1_E"] = {
            value = "CPSS_NET_USER_DEFINED_1_E",
            help  = "CPSS_NET_USER_DEFINED_1_E"
        },
        ["CPSS_NET_USER_DEFINED_2_E"] = {
            value = "CPSS_NET_USER_DEFINED_2_E",
            help  = "CPSS_NET_USER_DEFINED_2_E"
        },
        ["CPSS_NET_USER_DEFINED_3_E"] = {
            value = "CPSS_NET_USER_DEFINED_3_E",
            help  = "CPSS_NET_USER_DEFINED_3_E"
        },
        ["CPSS_NET_USER_DEFINED_4_E"] = {
            value = "CPSS_NET_USER_DEFINED_4_E",
            help  = "CPSS_NET_USER_DEFINED_4_E"
        }
    }
}

-- ************************************************************************
---
--  complete_param_with_predefined_values
--        @description  complete parameters with a set of predefined values
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter range
--        @param values         - array of predefined values to complete
--                                Example:
--                                {{"00:00:00:00:00:01",  "Unicast mac-address"}}
--
--        @return       complete and help lists
--
function complete_param_with_predefined_values(param, name, desc, values)
    local compl, help, index

    compl   = { }
    help    = { }

    for index = 1, #values do
        if prefix_match(param, tostring(values[index][1])) then
            table.insert(compl, tostring(values[index][1]))
            help[#compl] = values[index][2]
        end
    end

    return compl, help
end

--------------------------------------------
-- CPSS integer type registration
--------------------------------------------
do
  local inttypes = {"GT_SW_DEV_NUM"}
  local i, n
  for i, n in pairs(inttypes) do
    _G["mgmType_to_c_"..n] = mgmType_to_c_int
    _G["mgmType_to_lua_"..n] = mgmType_to_lua_int
  end
end
require_safe("common/cli_types/init_system_type")
if dxCh_family == true then
    require_safe_dx("cli_types/all")
end

if px_family == true then
    require_safe_px("cli_types/all")
end


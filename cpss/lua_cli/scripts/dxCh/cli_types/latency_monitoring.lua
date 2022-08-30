--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* latency_monitoring.lua
--*
--* DESCRIPTION:
--*       Latency monitoring type definition
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

CLI_type_dict["latProf"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    help = "Latency monitoring profile",
    min=0,
    max=511
}

CLI_type_dict["latProfRange"] = {
    checker = CLI_check_param_number_range,
    complete = CLI_complete_param_number_range,
    help = "Latency monitoring profile renge. Examples: \"7\", \"0,2-4,7\"",
    min=0,
    max=511
}

CLI_type_dict["smpProf"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    help = "Sampling profile",
    min=0,
    max=7
}

CLI_type_dict["smpProfRange"] = {
    checker = CLI_check_param_number_range,
    complete = CLI_complete_param_number_range,
    help = "Sampling profile range. Examples: \"7\", \"0,2-4,7\"",
    min=0,
    max=7
}

CLI_type_dict["latency"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    help = "Latency value, ns",
    min=0,
    max=0x3FFFFFFF
}

CLI_type_dict["lmPortDir"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Latency monitoring port role",
    enum = {
        ["ingress"] = {
            value = "CPSS_DIRECTION_INGRESS_E",
            help  = "Ingress role"
        },
        ["egress"] = {
            value = "CPSS_DIRECTION_EGRESS_E",
            help  = "Egress role"
        }
    }
}

CLI_type_dict["smpMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Latency monitoring sampling mode",
    enum = {
        ["deterministic"] = {
            value = "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E",
            help  = "Deterministic mode"
        },
        ["random"] = {
            value = "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E",
            help  = "Random mode"
        }
    }
}

CLI_type_dict["lmonPortIdx"] = {
    checker = CLI_check_param_number,
    complete = CLI_complete_param_number,
    help = "Latency monitoring port index",
    min=0,
    max=255
}
--Latency Profile Mode
CLI_type_dict["prfMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Latency monitoring profile mode",
    enum = {
        ["port"] = {
            value = "CPSS_DXCH_LATENCY_PROFILE_MODE_PORT_E",
            help  = "Port mode"
        },
        ["queue"] = {
            value = "CPSS_DXCH_LATENCY_PROFILE_MODE_QUEUE_E",
            help  = "Queue mode"
        }
    }
}

-- Traffic Class Type
CLI_type_dict["tc_ranges"] = {
    checker = CLI_check_param_number_range,
    min=0,
    max=15,
    complete = CLI_complete_param_number_range,
    help = "Traffic Classes (0..15) range. Examples: \"15\", \"0,2-4,15\", \"all\""
}


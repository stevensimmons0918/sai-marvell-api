--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_eee.lua
--*
--* DESCRIPTION:
--*       define types for Serdes.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--********************************************************************************
--      types
--********************************************************************************
CLI_type_dict["type_sqlch"]={
	checker = CLI_check_param_number,
	min=0,
	max=310,
	complete = CLI_complete_param_number,
	help = "Threshold that trips the Squelch detector"
}

CLI_type_dict["type_ffeRes"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "Mainly controls the low frequency gain"
}

CLI_type_dict["type_ffeCap"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "Mainly controls the high frequency gain"
}

CLI_type_dict["type_dcGain"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "DC Gain"
}

CLI_type_dict["type_bandWidth"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE Band-width"
}

CLI_type_dict["short_channel"]={
    checker = CLI_check_param_number,
    min=0,
    max=1,
    complete = CLI_complete_param_number,
    help = "CTLE Short-channel"
}

CLI_type_dict["type_gainShape1"]={
	checker = CLI_check_param_number,
	min=0,
	max=3,
	complete = CLI_complete_param_number,
	help = "CTLE gain shape 1"
}

CLI_type_dict["type_gainShape2"]={
	checker = CLI_check_param_number,
	min=0,
	max=3,
	complete = CLI_complete_param_number,
	help = "CTLE gain shape 2"
}

CLI_type_dict["type_minLf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE minLf"
}

CLI_type_dict["type_maxLf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE maxLf"
}

CLI_type_dict["type_minHf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE minLf"
}

CLI_type_dict["type_maxHf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "CTLE maxHf"
}

CLI_type_dict["type_BfLf"]={
	checker = CLI_check_param_number,
	min=0,
	max=8,
	complete = CLI_complete_param_number,
	help = "DFE BfLf"
}

CLI_type_dict["type_BfHf"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE BfHf"
}

CLI_type_dict["type_minPre1"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE min Pre1"
}

CLI_type_dict["type_maxPre1"]={
	checker = CLI_check_param_number,
	min=0,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE max Pre1"
}

CLI_type_dict["type_minPre2"]={
	checker = CLI_check_param_number,
	min=-10,
	max=10,
	complete = CLI_complete_param_number,
	help = "DFE min Pre2"
}

CLI_type_dict["type_maxPre2"]={
	checker = CLI_check_param_number,
	min=-10,
	max=10,
	complete = CLI_complete_param_number,
	help = "DFE max Pre2"
}

CLI_type_dict["type_minPost"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE min Post"
}
CLI_type_dict["type_maxPost"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "DFE max Post"
}

CLI_type_dict["type_gain1"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "DFE gain Tap1 strength"
}

CLI_type_dict["type_gain2"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "DFE gain Tap2 strength"
}

CLI_type_dict["type_laneNum"]={
	checker = CLI_check_param_number,
	min=0,
	max=7,
	complete = CLI_complete_param_number,
	help = "serdes lane number"
}

CLI_type_dict["type_post1"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "Serdes Post-cursor"
}
CLI_type_dict["type_pre1"]={
	checker = CLI_check_param_number,
	complete = CLI_complete_param_number,
	help = "Serdes Pre-cursor"
}

CLI_type_dict["type_pre2"]={
	checker = CLI_check_param_number,
	min=-15,
	max=15,
	complete = CLI_complete_param_number,
	help = "Serdes Pre-cursor2"
}

-- serdes tx params--
CLI_type_dict["type_amp"]={
	checker = CLI_check_param_number,
	min=0,
	max=31,
	complete = CLI_complete_param_number,
	help = "Tx Amplitude/Attenuation of the signal"
}

function CLI_check_param_tx_even(param, name, desc)
    param = tonumber(param)
    if param == nil then
        return false, name .. " not a number"
    end
    if ( desc.min ~= nil and desc.max ~= nil ) and ( param < desc.min or param > desc.max ) then
        return false, name.." is out of range "..tostring(desc.min)..".."..tostring(desc.max)
    end
    if desc.min ~= nil and param < desc.min then
        return false, name.." less than a minimal value "..tostring(desc.min)
    end
    if desc.max ~= nil and param > desc.max then
        return false, name.." greater than a maximum value "..tostring(desc.max)
    end

    if param % 2 ~= 0 then
        return false, " Must be even value "
    end

    return true, param
end

CLI_type_dict["type_txpost"]={
	checker = CLI_check_param_tx_even,
        min=-18,
        max=18,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Post-cursor"
}

CLI_type_dict["type_txpre"]={
	checker = CLI_check_param_tx_even,
        min=-10,
        max=10,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Pre-cursor1"
}

CLI_type_dict["type_txpre2"]={
	checker = CLI_check_param_number,
        min=-15,
        max=15,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Pre-cursor2"
}

CLI_type_dict["type_txpre3"]={
	checker = CLI_check_param_number,
        min=-1,
        max=1,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Pre-cursor2"
}

CLI_type_dict["type_rxtermination"]={
	checker = CLI_check_param_number,
	min=0,
	max=2,
	complete = CLI_complete_param_number,
	help = "Set Rx Termination"
}

CLI_type_dict["type_rxenvelope"]={
	checker = CLI_check_param_number,
	min=0,
	max=255,
	complete = CLI_complete_param_number,
	help = "Set Rx Envelope settings"
}

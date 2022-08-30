--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* types_serdes.lua
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

-- serdes tx params--
CLI_type_dict["type_amp"]={
	checker = CLI_check_param_number,
	min=0,
	max=31,
	complete = CLI_complete_param_number,
	help = "Tx Amplitude/Attenuation of the signal"
}

CLI_type_dict["type_txpost"]={
	checker = CLI_check_param_number,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Post-cursor"
}

CLI_type_dict["type_txpre"]={
	checker = CLI_check_param_number,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Pre-cursor1"
}

CLI_type_dict["type_txpre2"]={
	checker = CLI_check_param_number,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Pre-cursor2"
}

CLI_type_dict["type_txpre3"]={
	checker = CLI_check_param_number,
	complete = CLI_complete_param_number,
	help = "Serdes Tx Pre-cursor2"
}

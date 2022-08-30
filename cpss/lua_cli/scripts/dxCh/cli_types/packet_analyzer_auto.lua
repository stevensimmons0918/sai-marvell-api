--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_auto.lua
--*
--* DESCRIPTION:
--*       the file defines types needed for the packet analyzer commands.
--*       !!! This file is generated automatically !!!
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

CLI_type_dict["manager_id"] = {
    checker = CLI_check_param_number,
    min=1,
    max=10,
    help="a Manager ID (1-10)"
}

CLI_type_dict["profile_idx"] = {
    checker = CLI_check_param_number,
    min=1,
    max=2,
    help="a profile ID (1-2)"
}

CLI_type_dict["sampling_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "packet analyzer sampling mode",
    enum = {
        ["first_match"] = { value="CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E", help="Sample until the first match occurs" },
        ["last_match"] = { value="CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E", help="Continue sampling" }
   }
}

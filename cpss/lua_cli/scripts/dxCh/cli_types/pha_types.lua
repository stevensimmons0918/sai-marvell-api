--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pha_types.lua
--*
--* DESCRIPTION:
--*       typed for the PHA (sip6)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

pha___thread_id__min = 1
pha___thread_id__max = 255
pha___analyzer_id__min = 0
pha___analyzer_id__max = 6

CLI_type_dict["pha___thread_id"] = {
    checker = CLI_check_param_number,
    min=pha___thread_id__min,
    max=pha___thread_id__max,
    complete = CLI_complete_param_number,
    help=help_thread_id
}

CLI_type_dict["pha___busyStallMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "The role (CB - Control Bridge , PE - port extender)",
    enum = {
        ["push-back"]       = { value="CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E"      , help="Back-pressure to Control Pipe " },
        ["drop"]            = { value="CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E"           , help="Send to Bypass path and Drop the packet"},
        ["ordered-bypass"]  = { value="CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E" , help="Send to PHA. PHA will not modify the packet"}
    }
}

CLI_type_dict["pha___analyzer_id"] = {
    checker = CLI_check_param_number,
    min=pha___analyzer_id__min,
    max=pha___analyzer_id__max,
    complete = CLI_complete_param_number,
    help=help_analyzer_id
}

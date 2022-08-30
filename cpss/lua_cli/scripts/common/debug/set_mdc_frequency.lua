--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* set_mdc_frequency.lua
--*
--* DESCRIPTION:
--*       set SoC MDC frequency mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

function cpu_mdc_frequency(params)
    ret, val = myGenWrapper("appDemoSoCMDCFrequencySet", {
            { "IN", "GT_U8",  "mode", params.mode }})
    if (ret==0) then
       print("Success")
       return true
    else
       print("Failed")
       return false
    end
end

CLI_addCommand("debug", "cpu mdc frequency", {
    func = cpu_mdc_frequency,
    help = "Set CPU MDC frequency. Note: Changing MDC frequency could affect ongoing transactions",
    params={{type="named",
              {format="mode %cpu_mdc_frequency_mode", name="mode", help="MDC frequency mode"},
              mandatory = {"mode"}
           }}
})

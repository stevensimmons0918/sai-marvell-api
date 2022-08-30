--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* require_ez_bringup.lua
--*
--* DESCRIPTION:
--*       lua ez_bringup 'including' file
--*       also for mpd (phy driver)
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--******************************************************************************** 


local currDir = "common/ez_bringup"

CLI_prompts["pdl_test"] = "(pdlMode)#"
CLI_addCommand("debug", "pdl-mode", {
    func=CLI_change_mode_push,
    constFuncArgs={"pdl_test"},
    help="Switch to pdl mode"
})

require_safe(currDir .. "/pdlInbandPorts")
require_safe(currDir .. "/pdlPp")
require_safe(currDir .. "/pdlSerdes")
require_safe(currDir .. "/pdlBtn")
require_safe(currDir .. "/pdlSensor")
require_safe(currDir .. "/pdlLed")
require_safe(currDir .. "/pdlFan")
require_safe(currDir .. "/pdlOob")
require_safe(currDir .. "/pdlInterface")
require_safe(currDir .. "/pdlCpu")
require_safe(currDir .. "/pdlPhy")
require_safe(currDir .. "/pdlDebug")
require_safe(currDir .. "/pdlInit")
require_safe(currDir .. "/pdlPower")
require_safe(currDir .. "/pdlB2b")

CLI_prompts["mpd_test"] = "(mpdMode)#"
CLI_addCommand("debug", "mpd-mode", {
    func=CLI_change_mode_push,
    constFuncArgs={"mpd_test"},
    help="Switch to mpd mode"
})

require_safe(currDir .. "/mpd")

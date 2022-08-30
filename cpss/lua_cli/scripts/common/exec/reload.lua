--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* reload.lua
--*
--* DESCRIPTION:
--*       reloading of the device
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  crc_port_disable_func
--        @description  reload's of the device
--
--        @return       true on success, otherwise false and error message
--
local function reload_func(params)
    -- Common variables declaration
    local result, values

    result, values = myGenWrapper("cpssResetSystem",
                 {{"IN","GT_BOOL","doHwReset",1}})

    if 0x10 == result then
         print("It is not allowed to do soft reset on system.")
    elseif 0 ~= result then
         print("Error at system reset: %s.", returnCodes[result])
    else
        --call init system with same parameters it was initialized before
        result, values = myGenWrapper("cpssReInitSystem", {})
        if 0 ~= result then
            print("Error at init system: %s.", returnCodes[result])
        end
    end

    return true
end


--------------------------------------------------------
-- command registration: reload
--------------------------------------------------------
CLI_addCommand("exec", "reload", {
  func   = reload_func,
  help   = "Reload all stack units or specific unit"
})

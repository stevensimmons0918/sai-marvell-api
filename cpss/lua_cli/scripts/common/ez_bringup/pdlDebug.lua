--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlDebug.lua
--*
--* DESCRIPTION:
--*       setting of pdl commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_debug_i2c_read")

--------------------------------------------
-- Local functions: 
--------------------------------------------
local function pdl_debug_i2c_read(params)
  local api_result=1
    api_result = wr_utils_pdl_debug_i2c_read(params["i2c-addr"],params["buf-len"],params["bus-id"],params["offset-type"],params["offset-val"])
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show debug",  "Debug commands")

--------------------------------------------
-- Types registration: 
--------------------------------------------

--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show debug i2c", {
    func   = pdl_debug_i2c_read,
    help   = "issue direct i2c read command",
--      Examples:
--      show debug i2c 1 1 1 1 1
--      I2C Register Value:
--      [0x28]
        
  params = {
    {
      type = "values",
      {format="%integer",name="i2c-addr",help="The target device slave address on I2C bus"},
      {format="%integer",name="buf-len",help="Buffer length"},
      {format="%integer",name="bus-id",help="The I2C bus id"},
      {format="%integer", name="offset-type", help="Receive mode"},
      {format="%integer", name="offset-val", help="Internal offset to read from"},
      mandatory = { "i2c-addr", "buf-len", "bus-id","offset-type", "offset-val"},
    }
  }
})

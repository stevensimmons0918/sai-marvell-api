--********************************************************************************
--*              (c), Copyright 2018, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pdlB2b.lua
--*
--* DESCRIPTION:
--*       setting of pdl commands
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--------------------------------------------
-- Parameters registration: 
--------------------------------------------


--------------------------------------------
-- Functions registration: 
--------------------------------------------
cmdLuaCLI_registerCfunction("wr_utils_pdl_b2b_get_info")
cmdLuaCLI_registerCfunction("wr_utils_pdl_b2b_run_validation")


--------------------------------------------
-- Local functions: 
--------------------------------------------
-- show functions

local function pdl_b2b_show_info(params)
  local api_result=1
    api_result = wr_utils_pdl_b2b_get_info()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

local function pdl_b2b_run_validation(params)
  local api_result=1
    api_result = wr_utils_pdl_b2b_run_validation()
    if api_result ~= 0 then
      printMsg ("command failed rc = "..api_result)
    end
    return true    
end

--------------------------------------------
-- Help registration: 
--------------------------------------------
CLI_addHelp("pdl_test",    "show back-to-back",  "Show back-to-back information")
CLI_addHelp("pdl_test",    "run back-to-back",  "Run back-to-back validation")


--------------------------------------------
-- Types registration: 
--------------------------------------------



--------------------------------------------
-- Commands registration: 
--------------------------------------------
CLI_addCommand("pdl_test", "show back-to-back info all", {
    func   = pdl_b2b_show_info,
    help   = "show back to back XML information",
    
--          Examples:
--          show back-to-back info all
--    link list  | left-pp | right-pp | left-pp-port |right-pp-port | speed |    mode    |  left-serdes | right-serdes |
--   ----------- | ------- | -------- | ------------ |------------- | ----- | ---------- | ------------ | ------------ |
--   0           | 0       | 2        | 1            |2             | 24G   | XHGS       | Not Found    | Not Found    |
--   1           | 0       | 4        | 1            |4             | 24G   | XHGS       | Not Found    | Not Found    |
--   2           | 0       | 6        | 1            |6             | 24G   | XHGS       | Not Found    | Not Found    |

})


CLI_addCommand("pdl_test", "run back-to-back validate all", {
    func   = pdl_b2b_run_validation,
    help   = "Run back-to-back validation - validate info for all existing b2b links can be retrieved",
    
--          Examples: 
--          run back-to-back validate all
--        link list  |      Info       |
--       ----------- | --------------- |
--       0           | Pass            |
--       1           | Pass            |
--       2           | Pass            |


})







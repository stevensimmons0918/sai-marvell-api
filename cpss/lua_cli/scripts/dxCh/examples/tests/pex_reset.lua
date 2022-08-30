--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pex_reset.lua
--*
--* DESCRIPTION:
--*       test simulate sw reset with resetting pex
--*       
--*       purpose of example:
--*       1. Check that SW reset is done properly with pex
--*       2. Check that test run after sw reset
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local devNum  = devEnv.dev
local port1   = devEnv.port[1]

local flush_fdb = "dxCh/examples/configurations/flush_fdb.txt"


-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then 
        setFailState() 
    end
	print("Done running test "..testName)
end



local devFamily = wrlCpssDeviceFamilyGet(devNum)
if devFamily ~= "CPSS_PP_FAMILY_DXCH_FALCON_E" then
    setTestStateSkipped()
	return
end


local function callWithErrorHandling(cpssApi, params)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       print("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, value
end




-- run pre-test config
local function doTest()
	
	printLog("\nGraceful exit\n ")
    printLog("1.Add token in order enable pex reset\n ")
	
	    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","includePexInSwReset"},
        {"IN","GT_U32","value",1}
    })
	
	--disable dequeue on port 0 queue 0
	
	callWithErrorHandling(
        "cpssDxChPortTxQueueTxEnableSet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port1},
		{"IN","GT_U8","tcQueue",0},
		{"IN","GT_BOOL","enable",0}
    })
	
	printLog("2.Remove device and reset system\n ")
		 
	callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        }) 
	
		
	
	--call init system with same parameters it was initialized before
    callWithErrorHandling("cpssReInitSystem",{}) 
	
		--get dequeue enable on port 0 queue 0
	--It should be equal to 0 since this is default HW value and cpssInit does not configure this value
		
		
	result,value = callWithErrorHandling(
        "cpssDxChPortTxQueueTxEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",port1},
		{"IN","GT_U8","tcQueue",0},
		{"OUT","GT_BOOL","enablePtr"}
    })
	
	if tonumber(value) == 0 then
        print("\nERROR. Reset was not done.\n")
        setFailState()
	else
		print("\nReset was done correctly.\n")
    end 
	
	printLog("\nFlush FDB.\n ")

	executeLocalConfig(flush_fdb)
	
	-- run the tests:
-- 	1. 'sanity_send_and_capture.lua'
-- 	2. 'sanity_learn_mac.lua'		
	runTest("sanity_send_and_capture.lua")
	executeLocalConfig(flush_fdb)
	runTest("sanity_learn_mac.lua")
	executeLocalConfig(flush_fdb)    
end


-- run the test
doTest()





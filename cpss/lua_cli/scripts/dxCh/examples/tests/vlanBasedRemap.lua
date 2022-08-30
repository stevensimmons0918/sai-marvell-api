--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* vlanBasedRemap.lua
--*
--* DESCRIPTION:
--*       Test check VLAN  based remap
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local simulation_log = "dxCh/examples/configurations/simulation_log.txt"
local simulation_log_stop = "dxCh/examples/configurations/simulation_log_stop.txt"

--executeLocalConfig(simulation_log)
--executeLocalConfig(simulation_log_stop)

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4] 

SUPPORTED_FEATURE_DECLARE(devNum, "EGF_VLAN_BASED_TXQ_REMAP") 

-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then
        setFailState()
    end
    print("Done running test "..testName)
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
    local revision,value,rc
    local vlanOffset=247
    
    printLog("1.Get current revision\n ")
    rc,value = myGenWrapper("cpssInitSystemGet",{
            {"OUT","GT_U32","boardIdx"},
            {"OUT","GT_U32","boardRevId"},
            {"OUT","GT_U32","multiProcessApp"},
        })
    printLog("Current revision is  "..value["boardRevId"])
    revision = value["boardRevId"]

    printLog("2a.Remove device and reset system\n ")
         
    callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })
    printLog("2b.Clean mapping db\n ")
    callWithErrorHandling("harrier_clearMappingDb", {})
    
        
    printLog("3.Init with max Q  mode\n ")
    callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",40},
            {"IN","GT_U32","boardRevId",3},
            {"IN","GT_U32","reloadEeprom",0}
        })
    --run with custom ports
    devEnv.port[1] = 0
    devEnv.port[2] = 16
    devEnv.port[3] = 32
    devEnv.port[4] = 34
    
     --Initialize CNC counters to count pass/drop for each port
     callWithErrorHandling("prvCpssFalconQueueStatisticInit",{
            {"IN","GT_U32","devNum",devNum}
        })
    
    --configure VLAN offset
     printLog("Set VLAN based Q offset to "..vlanOffset.." for VLAN 1")
     callWithErrorHandling("cpssDxChBrgEgrFltVlanQueueOffsetSet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_U32","vidIndex",1},
            {"IN","GT_U32","vlanQueueOffset",vlanOffset},       
        })
     printLog("Enable VLAN based Q offset remap on port "..tostring(devEnv.port[0]))    
     
     callWithErrorHandling("cpssDxChPortTxQueueOffsetModeSet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_PORT_NUM","portNum",devEnv.port[0]},
            {"IN","CPSS_DXCH_QUEUE_OFFSET_MODE_ENT","offsetMode","CPSS_DXCH_QUEUE_OFFSET_MODE_VLAN_E"},     
        })
    --executeLocalConfig(simulation_log)
    --Run the test here
    runTest("sanity_send_and_capture.lua")  
    --executeLocalConfig(simulation_log_stop)
    --check CNC counters
    rc,value =callWithErrorHandling("prvCpssSip6TxqDebugCncCounterGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",devEnv.port[0]},
        {"IN","GT_U32","queue",vlanOffset},
        {"IN","GT_BOOL","preemptiveMode",false},
        {"OUT","GT_U32","passPcktsPtr"},
        {"OUT","GT_U32","droppedPcktsPtr"},
    })
     printLog(" Check counters for VLAN based Q offset "..vlanOffset)
     printLog(" CNC Pass packets "..value.passPcktsPtr)
     cncPassCounter = tonumber(value.passPcktsPtr)
     printLog(" CNC Drop packets "..value.droppedPcktsPtr)
     if(value.passPcktsPtr~=3 or value.droppedPcktsPtr~=0)then
       printLog(" Counter value unexpected.Failing the test.")
      setFailState()
     else
      printLog(" CNC counters value as expected.")
     end
    
    --restore ports
    devEnv.port[1] = port1
    devEnv.port[2] = port2
    devEnv.port[3] = port3
    devEnv.port[4] = port4
    
    --Take care of CNC counters deinit
     callWithErrorHandling("prvCpssFalconQueueStatisticDeInit",{
            {"IN","GT_U32","devNum",devNum}
        })
        
    printLog("4.Remove device and reset system\n ")
    callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        }) 
    printLog("4b.Clean mapping db\n ")
    
    callWithErrorHandling("harrier_clearMappingDb", {})
    

    
    printLog("5.Init in previous mode\n ")
    callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",40},
            {"IN","GT_U32","boardRevId",revision},
            {"IN","GT_U32","reloadEeprom",0}
        })
    
     
end


-- run the test
doTest()





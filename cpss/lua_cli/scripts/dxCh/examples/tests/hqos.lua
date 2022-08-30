--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* hqos.lua
--*
--* DESCRIPTION:
--*       Test check HQOS
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local devNum  = devEnv.dev

local configFileName = "hqos"
local simulation_log = "dxCh/examples/configurations/simulation_log.txt"
local simulation_log_stop = "dxCh/examples/configurations/simulation_log_stop.txt"
local  revision, boardType,res
local devFamily = wrlCpssDeviceFamilyGet(devNum)
if devFamily ~= "CPSS_PP_FAMILY_DXCH_FALCON_E" then
    setTestStateSkipped()
    return
end

res,value = myGenWrapper("appDemoAskModeEnableGet",{
            {"OUT","GT_BOOL","askModeEnablePtr"}
        })

if value["askModeEnablePtr"] == true then
    printLog("AppDemo in ASK mode does not support this test  ")
    setTestStateSkipped()
    return
end

res,value = myGenWrapper("cpssInitSystemGet",{
            {"OUT","GT_U32","boardIdx"},
            {"OUT","GT_U32","boardRevId"},
            {"OUT","GT_U32","multiProcessApp"},
        })
printLog("Current revision is  "..value["boardRevId"])
revision = value["boardRevId"]
boardType = value["boardIdx"]
--skip for 2T4T or multiple device
if revision == 7 or boardType==39 then
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

-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then
        setFailState()
    end
    print("Done running test "..testName)
end


-- run pre-test config
local function doTest()
    local value,rc
    local testedRemotePort =302
    local cascadeCpuPort =256
    local queue = 6
    local pm=0
    local tileNum=4
    local prevPortNum=0

    --check if PM active
    rc,value = myGenWrapper("prvWrAppDbEntryGet",{
            {"IN","string","namePtr","portMgr"},
            {"OUT","GT_U32","valuePtr"}
        })

    if val and value.valuePtr ~= 0 then
        pm=1
    end

    --check number of tiles
     rc,value = myGenWrapper("prvCpssTxqSip6DebugTileNumGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","GT_U32","numberOfTilesPtr"}
        })

    printLog("PM  "..pm)
    printLog("Number of tiles  "..value.numberOfTilesPtr)

    tileNum = value.numberOfTilesPtr

    if tileNum == 1 then
      setTestStateSkipped()
      printLog("Test is not supported for single tile device")
      return
    end


    if tileNum==2 then
        rc,value = myGenWrapper("prvWrAppDbEntryGet",{
            {"IN","string","namePtr","numberPhysicalPorts"},
            {"OUT","GT_U32","valuePtr"}
        })

        if val and value.valuePtr ~= 0 then
            prevPortNum= value.valuePtr
            printLog("prevPortNum  "..prevPortNum)
        end

      callWithErrorHandling(
            "appDemoDbEntryAdd",{
            {"IN","string","namePtr","numberPhysicalPorts"},
            {"IN","GT_U32","value",512}
        })
        cascadeCpuPort = 129
    end


    rc,value = myGenWrapper("appDemoDbEntryGet",{
           {"IN","string","namePtr","hqos"},
           {"OUT","GT_U32","value"}
        })

    if value["value"]==1 and  revision==13 then
        printLog("2a.Already in correct configuration\n ")
    else

        printLog("2a.Remove device and reset system\n ")

        callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })
    --printLog("2b.Clean mapping db\n ")
    --callWithErrorHandling("harrier_clearMappingDb", {})
        callWithErrorHandling(
            "appDemoDbEntryAdd",{
            {"IN","string","namePtr","hqos"},
            {"IN","GT_U32","value",1}
        })

        printLog("3.Init with 2CPU NW ports\n ")
        callWithErrorHandling("cpssInitSystem",{
                {"IN","GT_U32","boardIdx",35},
                {"IN","GT_U32","boardRevId",13},
                {"IN","GT_U32","reloadEeprom",0}
            })
    end
    --execute config file
    executeLocalConfig(luaTgfBuildConfigFileName(configFileName))

    if pm==1 then
     printLog("Enable MAC\n ")
     callWithErrorHandling("internal_original_cpssDxChPortEnableSet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",cascadeCpuPort},
                {"IN","GT_BOOL","enable",1}
            })

    end

    rc,value = callWithErrorHandling("cpssDxChPortEnableGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",cascadeCpuPort},
            {"OUT","GT_BOOL","statePtr"}
        })

    print("Port "..cascadeCpuPort.." MAC enable is "..tostring(value["statePtr"]))


    rc,value = callWithErrorHandling("cpssDxChBrgEgrFltPortLinkEnableGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",testedRemotePort},
            {"OUT","CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT","portLinkStatusStatePtr"}
        })


    print("Port "..testedRemotePort.." EGF link state is "..tostring(value["portLinkStatusStatePtr"]))

    runTest("sanity_send_and_capture.lua")

    --check counters
res, val = myGenWrapper("prvCpssSip6TxqDebugCncCounterGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",testedRemotePort},
                {"IN","GT_U32","queue",queue},
                {"IN","GT_BOOL","preemptiveMode",false},
                {"OUT","GT_U32","passPcktsPtr"},
                {"OUT","GT_U32","droppedPcktsPtr"},
                {"OUT","GT_U32","passPcktsMsbPtr"},
                {"OUT","GT_U32","droppedPcktsMsbPtr"}
           })

        if(res~=0)then
          print("Error "..res.." calling prvCpssSip6TxqDebugCncCounterGet")
          return
        end

       print(" CNC Pass packets")
       print(" ================")
       print("(MSB) "..val.passPcktsMsbPtr)
       print("(LSB) "..val.passPcktsPtr)
       print("")
       print(" CNC Drop packets")
       print(" ================")
       print("(MSB) "..val.droppedPcktsMsbPtr)
       print("(LSB) "..val.droppedPcktsPtr)

       print("Expect 1 packet to be mirrored to cpu port "..testedRemotePort.." queue "..queue)
       if(val.passPcktsPtr ~=3)then
        setFailState()
        print("NOT OK!")
      else
         print("OK!")
      end


    --restore

    printLog("4.Remove device and reset system\n ")
    callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })

        callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","hqos"},
        {"IN","GT_U32","value",0}
    })

    if tileNum==2  then
          callWithErrorHandling(
            "appDemoDbEntryAdd",{
            {"IN","string","namePtr","numberPhysicalPorts"},
            {"IN","GT_U32","value",prevPortNum}
        })
    end

    printLog("5.Init in previous mode "..boardType.." "..revision)
    callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",boardType},
            {"IN","GT_U32","boardRevId",revision},
            {"IN","GT_U32","reloadEeprom",0}
        })


end


-- run the test
doTest()





--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* rppInit.lua
--*
--* DESCRIPTION:
--*       Test check that cps init with remote port pass
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local devNum  = devEnv.dev

--add here supported devices

local supportedDevices =
    {
        "CPSS_PP_FAMILY_DXCH_FALCON_E",
        "CPSS_PP_FAMILY_DXCH_HARRIER_E",
        "CPSS_PP_FAMILY_DXCH_AC5P_E",
        "CPSS_PP_FAMILY_DXCH_AC5X_E"
    }

local falconAppDemoDbEntries =
    {
        {"rppEmptyTile",1},
        {"numberPhysicalPorts",1024}
    }
local ac5pAppDemoDbEntries =
    {
        {"rppMap",1}
    }
local ac5xAppDemoDbEntries =
    {
        {"rppMap",1}
    }


local testSupported = false



local devFamily = wrlCpssDeviceFamilyGet(devNum)
for i, supportedFamily in pairs(supportedDevices) do
    if devFamily == supportedFamily then
        testSupported=true
        break
    end
end

--not suported for 6.4T
if testSupported == true and devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" then
    local rc,value
    rc,value = myGenWrapper("prvCpssTxqSip6DebugTileNumGet",{
            {"IN","GT_U8","devNum",devNum},
            {"OUT","GT_U32","numberOfTilesPtr"}
        })
    if value["numberOfTilesPtr"]~=4 then
     testSupported=false
    end
end


res,value = myGenWrapper("appDemoAskModeEnableGet",{
            {"OUT","GT_BOOL","askModeEnablePtr"}
        })

if value["askModeEnablePtr"] == true then
   printLog("AppDemo in ASK mode does not support this test  ")
   setTestStateSkipped()
   return
end


if testSupported==false then
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
    local revOld,value,rc
    local boardIdx,revNew

    printLog("1.Get current revision\n ")
    rc,value = myGenWrapper("cpssInitSystemGet",{
            {"OUT","GT_U32","boardIdx"},
            {"OUT","GT_U32","boardRevId"},
            {"OUT","GT_U32","multiProcessApp"},
        })
    printLog("Current revision is  "..value["boardRevId"])
    revOld = value["boardRevId"]
    boardIdx = value["boardIdx"]

    printLog("2a.Remove device and reset system\n ")

    rc = myGenWrapper("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })
    -- GT_NOT_SUPPORTED. Skip if reset is not supported
    if rc == 0x10 then
     setTestStateSkipped()
     return
    end
    if rc ~= 0 then
       setFailState()
       print("ERROR of calling cpssResetSystem")
    end
    revNew=revOld
    --device specific initializations
    if devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" then
        for i, entry in pairs(falconAppDemoDbEntries) do
            printLog("2b."..i..". appDemoDbEntryAdd "..entry[1]..","..entry[2])
            callWithErrorHandling(
                "appDemoDbEntryAdd",{
                {"IN","string","namePtr",entry[1]},
                {"IN","GT_U32","value",entry[2]}
            })
        end
    elseif  devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" then
        printLog("2b.0. Clean mapping db\n ")
        callWithErrorHandling("ac5p_clearMappingDb", {})
        for i, entry in pairs(ac5pAppDemoDbEntries) do
            printLog("2b."..i..". appDemoDbEntryAdd "..entry[1]..","..entry[2])
            callWithErrorHandling(
                "appDemoDbEntryAdd",{
                {"IN","string","namePtr",entry[1]},
                {"IN","GT_U32","value",entry[2]}
            })
        end
    elseif  devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" then
        printLog("2b.0. Clean mapping db\n ")
        callWithErrorHandling("ac5x_clearMappingDb", {})
        for i, entry in pairs(ac5xAppDemoDbEntries) do
            printLog("2b."..i..". appDemoDbEntryAdd "..entry[1]..","..entry[2])
            callWithErrorHandling(
                "appDemoDbEntryAdd",{
                {"IN","string","namePtr",entry[1]},
                {"IN","GT_U32","value",entry[2]}
            })
        end
    elseif  devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
        printLog("2b.Clean mapping db\n ")
        callWithErrorHandling("harrier_clearMappingDb", {})
        revNew=2
    end



    printLog("3.Init with remote ports\n ")
    callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",boardIdx},
            {"IN","GT_U32","boardRevId",revNew},
            {"IN","GT_U32","reloadEeprom",0}
        })
    printLog("4.Remove device and reset system\n ")
    callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })
    printLog("4b.Restore DB\n ")

    if devFamily == "CPSS_PP_FAMILY_DXCH_FALCON_E" then
        for i, entry in pairs(falconAppDemoDbEntries) do
            printLog("4b."..i..". appDemoDbEntryAdd "..entry[1]..",0")
            callWithErrorHandling(
                "appDemoDbEntryAdd",{
                {"IN","string","namePtr",entry[1]},
                {"IN","GT_U32","value",0}
            })
        end
    elseif  devFamily == "CPSS_PP_FAMILY_DXCH_AC5P_E" then
        printLog("4b.0. Clean mapping db\n ")
        callWithErrorHandling("ac5p_clearMappingDb", {})
        for i, entry in pairs(ac5pAppDemoDbEntries) do
            printLog("4b."..i..". appDemoDbEntryAdd "..entry[1]..",0")
            callWithErrorHandling(
                "appDemoDbEntryAdd",{
                {"IN","string","namePtr",entry[1]},
                {"IN","GT_U32","value",0}
            })
        end
    elseif  devFamily == "CPSS_PP_FAMILY_DXCH_AC5X_E" then
        printLog("4b.0. Clean mapping db\n ")
        callWithErrorHandling("ac5x_clearMappingDb", {})
        for i, entry in pairs(ac5xAppDemoDbEntries) do
            printLog("4b."..i..". appDemoDbEntryAdd "..entry[1]..",0")
            callWithErrorHandling(
                "appDemoDbEntryAdd",{
                {"IN","string","namePtr",entry[1]},
                {"IN","GT_U32","value",0}
            })
        end
    elseif  devFamily == "CPSS_PP_FAMILY_DXCH_HARRIER_E" then
        printLog("4b.1.Clean mapping db\n ")
        callWithErrorHandling("harrier_clearMappingDb", {})
    end

    printLog("5.Init in previous mode\n ")
    callWithErrorHandling("cpssInitSystem",{
            {"IN","GT_U32","boardIdx",boardIdx},
            {"IN","GT_U32","boardRevId",revOld},
            {"IN","GT_U32","reloadEeprom",0}
        })

end


-- run the test
doTest()





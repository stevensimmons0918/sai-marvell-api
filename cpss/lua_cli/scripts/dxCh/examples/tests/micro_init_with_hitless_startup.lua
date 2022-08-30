--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* micro_init_with_hitless_startup.lua
--*
--* DESCRIPTION:
--*       test simulate micro init load through pex and sanity traffic
--*
--*       purpose of example:
--*       1. Verify micro init load.
--*       2. Verify boot channel.Get version of micro init.
--*       3. Verify init under HS
--*       4. Verify port sync under HS
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev

local devFamily = wrlCpssDeviceFamilyGet(devNum)
local pexMode
local superImageName = "super_image_harrier.bin"
local fwImageName = "firmware_cm3_falcon.bin"
local traffigConfigFile = "cont_traf.txt"
local testMode = 0  --"0" value for regular test on one profile "3" "1" value for full nine profiles test
local profile = 3
local setSlan = 1   --set 4 defulte slans

dofile ("dxCh/examples/common/high_availability_utils.lua")

--add here supported devices
local supportedDevices =
    {
        "CPSS_PP_FAMILY_DXCH_HARRIER_E",
        "CPSS_PP_FAMILY_DXCH_AC5P_E"
    }

local testSupported = false

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

--check that device is supported

for i, supportedFamily in pairs(supportedDevices) do
    if devFamily == supportedFamily then
        testSupported=true
        break
    end
end

if testSupported==true then
 --check that device work with memory access via BAR0,BAR2
 pexMode = myGenWrapper("cpssHwDriverGenWmInPexModeGet",{})

 if pexMode ==0 then
    printLog("\n usePexLogic flag is not set.\n ")
    testSupported=false
 end
else
    printLog("\n Device family does not support this test.\n ")
end

if testSupported==true then
 if isSimulationUsed()==false or isGmUsed()==true then
  printLog("\n This test can run only on white model.\n ")
  testSupported=false
 end
end

if testSupported==true then
--check that super image exist
 if file_exists(superImageName)==false then
 printLog("\n File "..superImageName.." not found.")
 testSupported=false
 else
 printLog("\n File "..superImageName.." found.")
 end
end

if testSupported==true then
--check that fw image exist
 if file_exists(fwImageName)==false then
 printLog("\n File "..fwImageName.." not found.")
 testSupported=false
 else
 printLog("\n File "..fwImageName.." found.")
 end
end


if testSupported==false then
    setTestStateSkipped()
    return
else
    printLog("\n Test support check is done.Test can be executed.\n ")
end

local function callWithErrorHandling(cpssApi, params)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       printLog("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, value
end

-- run pre-test config
local function doTest()
     local rc, val
     local systemRecoveryInfo = {}
     local enterCmd

    --save current port configuration before reset
    preTestSaveCurrentPorts()

    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false,
            ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
            haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E"
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E"
    }

    printLog ("\n test started : profile : "..profile.."  \n ")

    --reset device
    printLog("1.Add token in order enable pex reset\n")
    callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","includePexInSwReset"},
        {"IN","GT_U32","value",1}
    })

    printLog("2.Remove device and reset system\n ")

    callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })

    --init global DB
    callWithErrorHandling("cpssPpInit",{})

    printLog("3.Load super image \n")
    callWithErrorHandling("appDemoLoadSuperImageToDdr",{
            {"IN","GT_BOOL","runUboot",1}
        })

    --change MI port profile
    callWithErrorHandling("cm3BootChannelChangeProfile",{
            {"IN","GT_U32","profileNum",profile}
        })

    callWithErrorHandling("appDemoDbEntryAdd",{
            {"IN","string","namePtr","hitless_startup_HwDevNum"},
            {"IN","GT_U32","value",0}
        })

    if setSlan == 1 then
        callWithErrorHandling("skernelBindDevPort2Slan",{
                {"IN","GT_U32","deviceId",0},
                {"IN","GT_U32","portNum",6},
                {"IN","string","slanNamePtr","slan00"},
                {"IN","GT_BOOL","unbindOtherPortsOnThisSlan",1}
            })

        callWithErrorHandling("skernelBindDevPort2Slan",{
                {"IN","GT_U32","deviceId",0},
                {"IN","GT_U32","portNum",8},
                {"IN","string","slanNamePtr","slan01"},
                {"IN","GT_BOOL","unbindOtherPortsOnThisSlan",1}
            })

        callWithErrorHandling("skernelBindDevPort2Slan",{
                {"IN","GT_U32","deviceId",0},
                {"IN","GT_U32","portNum",16},
                {"IN","string","slanNamePtr","slan02"},
                {"IN","GT_BOOL","unbindOtherPortsOnThisSlan",1}
            })

        callWithErrorHandling("skernelBindDevPort2Slan",{
                {"IN","GT_U32","deviceId",0},
                {"IN","GT_U32","portNum",36},
                {"IN","string","slanNamePtr","slan03"},
                {"IN","GT_BOOL","unbindOtherPortsOnThisSlan",1}
            })
    end

    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_INIT_STATE_E ")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("3. Verify init under HS \n")
    printLog("cpssReInitSystem Start ")
    --call init system with same parameters it was initialized before
    callWithErrorHandling("cpssReInitSystem",{})

    rc, val = callWithErrorHandling("prvHitlessStartupInitStateGet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "OUT", "GT_U32", "value"}
            })
    if val["value"] ~= 3  then
        setFailState()
    end

    systemRecoveryInfo.systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E"

    --Set system recovery state catchup
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E ")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("\n4. Verify port sync under HS")
    rc,val = callWithErrorHandling("cpssDxChHitlessStartupSyncPortManager",{
            {"IN","GT_U8","devNum",devNum}
    })


    systemRecoveryInfo.systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E"

    --Set system recovery state catchup
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E ")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    rc, val = callWithErrorHandling("prvHitlessStartupInitStateGet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "OUT", "GT_U32", "value"}
            })

    if val["value"] ~= 4  then
        setFailState()
    end

    --gives "show interfaces status" time to update
    delay(50)

    enterCmd = [[
        end
        do show interfaces status ethernet 0/6,8,16,36
        end]]

    executeStringCliCommands(enterCmd)

    print("test is done \n restore ports configurtion - MI ports configurtion stay same ")
    replayPortsAfterHs(devNum)

end

-- run the test
if testMode == 0 then
    doTest()
else
    for ii = 0,9 do
        --skip profile 4 due to bug in disable ports by micro init ,only happens when testing profiles one after the other
        if ii == 4 then
            ii = ii+1
        end
        profile = ii
        doTest()
    end
end

--clean up
callWithErrorHandling(
        "appDemoDbEntryAdd",{
        {"IN","string","namePtr","includePexInSwReset"},
        {"IN","GT_U32","value",0}
        })

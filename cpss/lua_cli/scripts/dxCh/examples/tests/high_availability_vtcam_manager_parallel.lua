--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* high_availebility_vtcam_manager_parallel.lua
--*
--* DESCRIPTION:
--*       test simulate sw crash and high availability scenario for VTCAM Manager parallel mode
--*
--*       purpose of example:
--*       1. Check that HW is not affected by HA procedure
--*       2. Check that test run after HA
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

-- void function without parameters clears all the TCAM
-- used in all Enhanced UT of VTCAM_HA to clean TCAM after tests leaving rules.
cmdLuaCLI_registerCfunction("wrlCpssClearAllTcam")

-- SIP6 devices supported
SUPPORTED_FEATURE_DECLARE(devNum, "VTCAM_MANAGER_HA")

local devNum  = devEnv.dev
local flush_fdb = "dxCh/examples/configurations/flush_fdb.txt"

local devFamily = wrlCpssDeviceFamilyGet(devNum)
dofile ("dxCh/examples/common/high_availability_utils.lua")

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

    local systemRecoveryInfo = {}
    local managerArray={}
    local rc

    printLog("\n vTCAM Manager HA parallel Test \n");
    printLog("\n  Init before Simulation Crash \n");
    callWithErrorHandling("vtcamHaApp",{
        {"IN",  "GT_U8",    "devNum",           devNum},
        {"IN",  "GT_U8",    "createMngr",       1},
        {"IN",  "GT_U8",    "createVtcam",      1},
        {"IN",  "GT_U32",   "vtcamId",          0},
        {"IN",  "GT_U32",   "ruleSize",         5},
        {"IN",  "GT_U32",   "vtcamSize",        100},
        {"IN",  "GT_U32",   "numRules",         90},
        {"IN",  "GT_U8",    "vtcamIdOffset",    0},
        {"IN",  "GT_U32",   "ruleIdOffset",     0},
        {"IN",  "GT_U8",    "vtcamMode",        0},
        {"IN",  "GT_U32",   "priority",         0}
    })

    --save current port configuration before crash
    preTestSaveCurrentPorts()

    printLog("\nSimulate application crash.\n ")
    callWithErrorHandling(
        "appDemoHaEmulateSwCrash", {})

    --Set system recovery state INIT
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
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
    }

    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_INIT_STATE_E ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("\nInit with port configuration ")

    --call init system with same parameters it was initialized before
    callWithErrorHandling("cpssReInitSystem",{})

    --Set system recovery state catchup
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false,
            ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
            haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E"
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E"
    }

    --Set system recovery state catchup
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E")

    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("\n Replay ports only ")
    --set initial port config
    replayPorts()
    systemRecoveryInfo = {
        systemRecoveryState = "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
        systemRecoveryMode = {
            continuousRx = false,
            continuousTx = false,
            continuousAuMessages = false,
            continuousFuMessages = false,
            haCpuMemoryAccessBlocked = false,
            ha2phasesInitPhase = "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
            haReadWriteState = "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E"
        },
        systemRecoveryProcess = "CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E"
    }

    --Set system recovery state completion
    printLog("\nSet system recovery state CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E PARALLEL ")
    callWithErrorHandling(
        "cpssSystemRecoveryStateSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_INFO_STC", "info",  systemRecoveryInfo}
    })

    printLog("\nStart event handler tasks\n ")
    callWithErrorHandling("appDemoEventRequestDrvnModeInit", {})

    printLog("\nEnable rx sdma \n ")
    callWithErrorHandling("cpssDxChNetIfRestore",{
            {"IN","GT_U32","devNum",devNum}
    })
    printLog("\nSet PARALLEL catch up for TCAM")

    managerArray= {[0] = "CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E"}
    myGenWrapper("cpssSystemRecoveryHaParallelModeSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_STATE_ENT",    "state","CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E"},
        {"IN",  "GT_U32",    "numOfManagers", 1},
        {"IN",  "CPSS_SYSTEM_RECOVERY_MANAGER_ENT[10]", "managerListArray", managerArray}
    })

    printLog("\nvTCAM Manager Replay after Simulation Crash" )
    callWithErrorHandling("vtcamHaApp",{
        {"IN",  "GT_U8",    "devNum",           devNum},
        {"IN",  "GT_U8",    "createMngr",       1},
        {"IN",  "GT_U8",    "createVtcam",      1},
        {"IN",  "GT_U32",   "vtcamId",          0},
        {"IN",  "GT_U32",   "ruleSize",         5},
        {"IN",  "GT_U32",   "vtcamSize",        100},
        {"IN",  "GT_U32",   "numRules",         90},
        {"IN",  "GT_U8",    "vtcamIdOffset",    0},
        {"IN",  "GT_U32",   "ruleIdOffset",     0},
        {"IN",  "GT_U8",    "vtcamMode",        0},
        {"IN",  "GT_U32",   "priority",         0}
    })

    printLog("\nSet PARALLEL completion for TCAM ")
    myGenWrapper("cpssSystemRecoveryHaParallelModeSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_STATE_ENT",    "state","CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E"},
        {"IN",  "GT_U32",    "numOfManagers", 1},
        {"IN",  "CPSS_SYSTEM_RECOVERY_MANAGER_ENT[10]", "managerListArray", managerArray}
    })

    printLog("\nREPLAY CONFIGURATION  DONE")

    printLog ("test add and remove of rules ")

    -- add 3 rules at the end of last tcam rule
    callWithErrorHandling(
            "vtcamHaApp",{
            {"IN", "GT_U8" , "devNum"       , devNum},
            {"IN", "GT_U8" , "createMngr"   , 0},
            {"IN", "GT_U8" , "createVtcam"  , 0},
            {"IN", "GT_U32", "vtcamId"      , 0},
            {"IN", "GT_U32", "ruleSize"     , 5},
            {"IN", "GT_U32", "vtcamSize"    , 100},
            {"IN", "GT_U32", "numRules"     , 3},
            {"IN", "GT_U8" , "vtcamIdOffset", 0},
            {"IN", "GT_U32", "ruleIdOffset" , 90},
            {"IN", "GT_U8" , "vtcamMode"    , 0},
            {"IN", "GT_U32", "priority"     , 0}
        })

    --same vTcamMngId as set during vtcamHaApp
    vTcamMngId = devNum % 31 -- CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS

    --delete old rule
    callWithErrorHandling(
            "cpssDxChVirtualTcamRuleDelete",{
            {"IN", "GT_U32" , "vTcamMngId"                  , vTcamMngId},
            {"IN", "GT_U32" , "vTcamId"                     , 0},
            {"IN", "CPSS_DXCH_VIRTUAL_TCAM_RULE_ID","ruleId", 50},
        })
    --delete new rule
    callWithErrorHandling(
            "cpssDxChVirtualTcamRuleDelete",{
            {"IN", "GT_U32" , "vTcamMngId"                  , vTcamMngId},
            {"IN", "GT_U32" , "vTcamId"                     , 0},
            {"IN", "CPSS_DXCH_VIRTUAL_TCAM_RULE_ID","ruleId", 91},
        })


--  sanity checks after HA:
--  1. 'sanity_send_and_capture.lua'
--  2. 'sanity_learn_mac.lua'
    runTest("sanity_send_and_capture.lua")
    executeLocalConfig(flush_fdb)
    runTest("sanity_learn_mac.lua")
    executeLocalConfig(flush_fdb)


    printLog("\nCLEAR all DB \n")
    callWithErrorHandling(
        "prvCpssDxChVirtualTcamHaSystemRecoverySet",{
        {"IN", "GT_U8",                             "devNum",                devNum},
        {"IN", "GT_U32",                            "vTcamMngId",            devNum},
       {"IN", "CPSS_SYSTEM_RECOVERY_STATE_ENT",    "systemRecoveryState",   2},
        {"IN", "CPSS_SYSTEM_RECOVERY_PROCESS_ENT",  "systemRecoveryProcess", 2},
        {"IN", "GT_U32",                            "tcamReset",             1}
    })

    wrlCpssClearAllTcam(devNum)
    executeLocalConfig(flush_fdb)

    --needs to recover lpm DB to avoid errors in next test
    managerArray= {[0] = "CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E"};

    myGenWrapper("cpssSystemRecoveryHaParallelModeSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_STATE_ENT",    "state","CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E"},
        {"IN",  "GT_U32",    "numOfManagers", 1},
        {"IN",  "CPSS_SYSTEM_RECOVERY_MANAGER_ENT[10]", "managerListArray", managerArray}
    })
    myGenWrapper("cpssSystemRecoveryHaParallelModeSet",{
        {"IN",  "CPSS_SYSTEM_RECOVERY_STATE_ENT",    "state","CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E"},
        {"IN",  "GT_U32",    "numOfManagers", 1},
        {"IN",  "CPSS_SYSTEM_RECOVERY_MANAGER_ENT[10]", "managerListArray", managerArray}
    })



end


-- run the test
doTest()


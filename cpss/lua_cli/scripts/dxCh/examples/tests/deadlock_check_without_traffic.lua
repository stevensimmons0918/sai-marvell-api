--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* deadlock_check_without_traffic.lua
--*
--* DESCRIPTION:
--*       The test to check granular locking in multi threaded environment.
--*       The test operates with two predefined sets of UT's suits and launch
--*       them in concurency with same and different device number, once and
--*       multiple times.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

MARK_LONG_TEST()

-- temporarily skip the test on xCat3 due to crash
NOT_SUPPORTED_DEV_DECLARE(devEnv.dev, "CPSS_PP_FAMILY_DXCH_XCAT3_E","CPSS_PP_FAMILY_DXCH_AC5_E") 

local tasks = 2
local maxDelay = 600
local taskIds = {}
local i
local rc
local msgQ

rc = cpssGenWrapper("osIsThreadLocalSupported", { })
if rc == 0 then
    setTestStateSkipped()
    return
end

function run_tasks(tasks_prefix, tasks_num)
    if isTestFailed == true then
        return
    end
    for i=1,tasks_num do
        local f = loadstring("return "..tasks_prefix..i)

        -- common format to pass
        -- { msgQ, task_param, [ count ] }
        taskIds[i] = {}
        taskIds[i].tid = luaTaskCreate("test_task"..i, "dxCh/examples/common/cpss_locking_task_utils.lua", { msgQ, f() })
        taskIds[i].tname = "test_task"..i
        taskIds[i].params = f()

        -- passing not a table of calls, but individual call also possible, like this
        -- taskIds[i] = luaTaskCreate("test_task"..i, "traffic/test_task.lua", { msgQ, task_param1[i] })
    end
end

function test_insert(table_name, test_name, test_count, test_devnum)
    table.insert(table_name, {
        api = "prvUtfSkipAllDevices",
        param = { },
        count = 1,
        delay = 0
    })
    table.insert(table_name, {
        api = "prvUtfSetActiveDevice",
        param = {
            {"IN","GT_U8","devNum", test_devnum},
        },
        count = 1,
        delay = 0
    })
    --[[ -- uncomment to enable log output
    table.insert(table_name, {
        api = "utfLogOutputSelect",
        param = {
            {"IN","GT_U8","eLogOutput", 0},
        },
        count = 1,
        delay = 0
    })
    --]]
    table.insert(table_name, {
        api = "utfTestsRun",
        param = {
            {"IN","string","testPathPtr", test_name},
            {"IN","GT_U32","numOfRuns", 1},
            {"IN","GT_BOOL","continueFlag", true},
        },
        count = test_count,
        delay = 1,
    })
end

function wait_tasks()
    if isTestFailed == true then
        return
    end
    if #taskIds > 0 then
        printLog("Wait for tasks")
        for i=1,tasks do
            local st = false
            while st == false do
                st = new_instance_shell_cmd_wait(taskIds[i].tid, maxDelay)
            end
        end
    else
        return
    end

    local msgs = 0
    local msgTaskIds = {}
    for i=1,tasks do
        local status, msg = luaMsgQRecv(msgQ, 1000)
        if status == 0 then
            msgs = msgs + 1
            table.insert(msgTaskIds, msg)
        end
    end
    table.sort(msgTaskIds)

    rc = cpssGenWrapper("prvUtfSafeDeadLockCheck", {
    }, true)
    if rc == 1 then
        setFailState()
        printLog("Deadlock detected!!!")
        rc = cpssGenWrapper("prvUtfSafeDeadLockStatusClear", { }, true)
        return
    end

    if msgs ~= tasks then
        setFailState()
        printLog("The number of replies is not equal to the number of threads")
        printLog("Some threads still running")
    end

    delay(3000)
    taskIds = {}
end

local ut_tests_set1 = {
    "cpssDxChDiag",
    "cpssDxChNetIf",
    "cpssDxChIpLpm",
    "cpssDxChMirror",
    "cpssDxChProtection",
    "cpssTmNodesTree",
}

local ut_tests_set2 = {
    "cpssDxChDiag",
    "cpssDxChNetIf",
    "cpssFormatConvert",
    "cpssDxChVirtualTcam",
    "cpssDxChL2Mll",
    "cpssDxChCutThrough",
}

--local ut_samedev_params1
--local ut_samedev_params2
--local ut_diffdev_params1
--local ut_diffdev_params2

-- MAIN
-- Enable safe mutexes implementation
cpssGenWrapper("utfStartUsingSafeLocking", { }, true)

-- Temporary disable printouts from UT's
cpssGenWrapper("prvUtfLogPrintDisableOutput", { }, true)

printLog("Create task")
msgQ = luaMsgQCreate("tasks", "GT_U32", 5)

-- Same devNum - UT
ut_samedev_params1 = {}
ut_samedev_params2 = {}

-- once
-- devnum 1
for i=1,#ut_tests_set1 do
    test_insert(ut_samedev_params1, ut_tests_set1[i], 1, boardEnv.devList[1].dev)
    test_insert(ut_samedev_params2, ut_tests_set2[i], 1, boardEnv.devList[1].dev)
end
run_tasks("ut_samedev_params", 2)
wait_tasks()


if #boardEnv.devList > 1 then
    -- Different devNum - UT
    ut_diffdev_params1 = {}
    ut_diffdev_params2 = {}

    -- once
    -- devnum 1 and 2
    for i=1,#ut_tests_set1 do
        test_insert(ut_diffdev_params1, ut_tests_set1[i], 1, boardEnv.devList[1].dev)
        test_insert(ut_diffdev_params2, ut_tests_set2[i], 1, boardEnv.devList[2].dev)
    end
    run_tasks("ut_diffdev_params", 2)
    wait_tasks()
end

--[[
    -- Same devNum one UT multiple times
    ut_samedev_params1 = {}
    ut_samedev_params2 = {}

    -- 5 times
    -- devnum 1
    test_insert(ut_samedev_params1, "cpssDxChDiag", 5, boardEnv.devList[1].dev)
    test_insert(ut_samedev_params2, "cpssDxChDiag", 5, boardEnv.devList[1].dev)
    run_tasks("ut_samedev_params", 2)
    wait_tasks()


    if #boardEnv.devList > 1 then
        -- Different devNum one UT multiple times
        ut_diffdev_params1 = {}
        ut_diffdev_params2 = {}

        -- 5 times
        -- devnum 1 and 2
        test_insert(ut_diffdev_params1, "cpssDxChDiag", 5, boardEnv.devList[1].dev)
        test_insert(ut_diffdev_params2, "cpssDxChDiag", 5, boardEnv.devList[2].dev)
        run_tasks("ut_diffdev_params", 2)
        wait_tasks()
    end
--]]

-- Restore temporary disabled printouts from UT's
cpssGenWrapper("prvUtfLogPrintEnableOutput", { }, true)

-- Disable safe mutexes implementation
cpssGenWrapper("utfStopUsingSafeLocking", { }, true)


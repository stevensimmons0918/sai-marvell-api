
--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* high_availability_utils.lua
--*
--* DESCRIPTION:
--*       Any functions, variables etc that are required in several tests.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
local recreationRequired = {}
local devNum  = devEnv.dev


--replay ports after HA event ,use recreationRequired list to configure the ports
function replayPorts()
    local result ,values


    --get first valid device number
    result, values =myGenWrapper("cpssPpCfgNextDevGet",{
                {"IN","GT_U8","devNum",0xFF},
                {"OUT","GT_U8","nextDevNumPtr"}
        })
        if (result ~= 0 and result ~= 12) then
            err = returnCodes[result]
            printLog("cpssPpCfgNextDevGet returned "..err)
            return err
        end
    devNum = values.nextDevNumPtr

    --replay all port for device
    repeat
        result, maxPhysicalPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
        if(result ~= 0) then maxPhysicalPort = 0 end
        maxPhysicalPort = maxPhysicalPort + 1

        for portNum=0, maxPhysicalPort do
           -- for portNum, portParams in pairs(recreationRequired) do
            if recreationRequired[portNum] ~= nil and recreationRequired[portNum].valid == true then
                result, values = myGenWrapper("cpssDxChSamplePortManagerMandatoryParamsSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode", recreationRequired[portNum].ifMode },
                        { "IN", "CPSS_PORT_SPEED_ENT", "speed",recreationRequired[portNum].speed},
                        { "IN", "CPSS_PORT_FEC_MODE_ENT", "fecMode",recreationRequired[portNum].fecMode},
                })
                if (result ~= 0) then
                    err = returnCodes[result]
                    printLog(portNum.." : cpssDxChSamplePortManagerMandatoryParamsSet returned "..err)
                    return err
                end

                --add loopback if needed
                if recreationRequired[portNum].isMacLoopback == true then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerLoopbackSet", {
                            { "IN", "GT_U8", "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT", "ifMode", CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E },
                            { "IN", "GT_BOOL", "macLoopbackEnable",true},
                            { "IN", "CPSS_PORT_SERDES_LOOPBACK_MODE_ENT", "serdesLoopbackMode",CPSS_PORT_SERDES_LOOPBACK_DISABLE_E },
                            { "IN", "GT_BOOL", "enableRegularTrafic",false},
                    })
                    if (result ~= 0) then
                        err = returnCodes[result]
                        printLog(portNum.." : cpssDxChSamplePortManagerLoopbackSet returned "..err)
                        return err
                    end
                end

                --recreate port
                local portEventStc = {}
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E
                result, values = myGenWrapper("cpssDxChPortManagerEventSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "IN", "CPSS_PORT_MANAGER_STC", "portEventStc", portEventStc },
                })
                if (result ~= 0) then
                    printLog("FAIL to recreate port   "..portNum)
                    err = returnCodes[result]
                    printLog("cpssDxChPortManagerEventSet returned "..err)
                    return err
                end
            end
        end
        result, values =myGenWrapper("cpssPpCfgNextDevGet",{
                {"IN","GT_U8","devNum",devNum},
                {"OUT","GT_U8","nextDevNumPtr"}
        })
        if (result ~= 0 and result ~= 12) then
            err = returnCodes[result]
            printLog("cpssPpCfgNextDevGet returned "..err)
            return err
        end
        devNum = values.nextDevNumPtr
    until (devNum == 255)
end

--configure ports after HS ,this function configure all ports that werent configure by MI ,
--use recreationRequired list to configure the ports
function replayPortsAfterHs(devNum)
    local result ,values


    --replay all port for device
    result, maxPhysicalPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if(result ~= 0) then maxPhysicalPort = 0 end
    maxPhysicalPort = maxPhysicalPort + 1

    for portNum=0, maxPhysicalPort do
        rc, val = myGenWrapper("cpssDxChPortManagerPortParamsGet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParams" }
        })
        if rc == 0  then
            print("skip port : "..tostring(port).." , configured by MI \n")
        else
            if recreationRequired[portNum] ~= nil and recreationRequired[portNum].valid == true then
                result, values = myGenWrapper("cpssDxChSamplePortManagerMandatoryParamsSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "IN", "CPSS_PORT_INTERFACE_MODE_ENT", "ifMode", recreationRequired[portNum].ifMode },
                        { "IN", "CPSS_PORT_SPEED_ENT", "speed",recreationRequired[portNum].speed},
                        { "IN", "CPSS_PORT_FEC_MODE_ENT", "fecMode",recreationRequired[portNum].fecMode},
                })
                if (result ~= 0) then
                    err = returnCodes[result]
                    printLog(portNum.." : cpssDxChSamplePortManagerMandatoryParamsSet returned "..err)
                    return err
                end

                --add loopback if needed
                if recreationRequired[portNum].isMacLoopback == true then
                    result, values = myGenWrapper("cpssDxChSamplePortManagerLoopbackSet", {
                            { "IN", "GT_U8", "devNum", devNum},
                            { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                            { "IN", "CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT", "ifMode", CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E },
                            { "IN", "GT_BOOL", "macLoopbackEnable",true},
                            { "IN", "CPSS_PORT_SERDES_LOOPBACK_MODE_ENT", "serdesLoopbackMode",CPSS_PORT_SERDES_LOOPBACK_DISABLE_E },
                            { "IN", "GT_BOOL", "enableRegularTrafic",false},
                    })
                    if (result ~= 0) then
                        err = returnCodes[result]
                        printLog(portNum.." : cpssDxChSamplePortManagerLoopbackSet returned "..err)
                        return err
                    end
                end

                --recreate port
                local portEventStc = {}
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E
                result, values = myGenWrapper("cpssDxChPortManagerEventSet", {
                        { "IN", "GT_U8", "devNum", devNum},
                        { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                        { "IN", "CPSS_PORT_MANAGER_STC", "portEventStc", portEventStc },
                })
                if (result ~= 0) then
                    printLog("FAIL to recreate port   "..portNum)
                    err = returnCodes[result]
                    printLog("cpssDxChPortManagerEventSet returned "..err)
                    return err
                end
            end
        end -- else
    end -- for loop
end

--init recreationRequired list with vaild port parames pre SW crash
function preTestSaveCurrentPorts()

    local result ,values,portValidity,MacValidity,mappingType
    local portMapShadow

    result, maxPhysicalPort = wrLogWrapper("wrlCpssDevicePortNumberGet", "(devNum)", devNum)
    if(result ~= 0) then
        maxPhysicalPort = 0
    end
    maxPhysicalPort = maxPhysicalPort + 1

    for portNum = 0, maxPhysicalPort do
        result, values = myGenWrapper("cpssDxChPortPhysicalPortDetailedMapGet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                { "OUT", "CPSS_DXCH_DETAILED_PORT_MAP_STC", "portMapShadowPtr" }
        })

        if (result ~= 0) then
            err = returnCodes[result]
            printLog(portNum.." : cpssDxChPortPhysicalPortDetailedMapGet returned "..err)
            setFailState()
            return err
        end
        portMapShadow = values.portMapShadowPtr

        --skip 'CPU SDMA' and 'remote'
        portValidity = portMapShadow.valid
        mappingType = portMapShadow.portMap.mappingType
        if tostring(mappingType) ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
            MacValidity = false
        else
            MacValidity = true
        end

        if ((portValidity == true ) and (MacValidity == true )) then
            result, values = myGenWrapper("cpssDxChPortManagerPortParamsGet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                    { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParams" }
            })
            if result == 0  then
                recreationRequired[portNum] = {}
                recreationRequired[portNum].valid  = true
                recreationRequired[portNum].speed  = values.portParams.portParamsType.regPort.speed
                recreationRequired[portNum].ifMode = values.portParams.portParamsType.regPort.ifMode
                recreationRequired[portNum].fecMode = values.portParams.portParamsType.regPort.portAttributes.fecMode
                recreationRequired[portNum].isMacLoopback = values.portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable

            elseif result ~= 0 and result ~= 18 then
                err = returnCodes[result]
                printLog(portNum..": cpssDxChPortManagerPortParamsGet returned "..err)
                setFailState()
                return err
            end
        end
    end
end

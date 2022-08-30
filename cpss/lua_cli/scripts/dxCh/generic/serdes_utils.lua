--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* serdes_utils.lua
--*
--* DESCRIPTION:
--*       serdes utils
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--[[
the functions are :

]]--
-- ************************************************************************
function GetPortSerdesNum(devNum, portNum,laneNum)
    local ret, val
    local macNum

    --print("Portnum" .. portNum)
    ret, val = myGenWrapper("cpssDxChPortPhysicalPortMapGet", {
            {"IN", "GT_U8","devNum",devNum},
            {"IN", "GT_U32","firstPhysicalPortNumber",portNum},
            {"IN", "GT_U32", "portMapArraySize", 1 },
            {"OUT","CPSS_DXCH_PORT_MAP_STC","portMap"}  })

    if ret == 0 then
        if val.portMap.mappingType == "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
            macNum = val.portMap.interfaceNum
        else
           return -1
        end
    else
        --print("Error at cpssDxChPortPhysicalPortMapGet")
        return -1
    end
    ret, val = cpssGenWrapper("hwsPortModeParamsGetToBuffer",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","portGroup",0},
                {"IN","GT_U32","portNum",macNum},
                {"IN","MV_HWS_PORT_STANDARD","portMode","NON_SUP_MODE"},
                {"OUT","MV_HWS_PORT_INIT_PARAMS","portParamsBuffer"}})

    if 0 ~= ret then
        print("Error at hwsPortModeParamsGetToBuffer")
        return -1
    end

    --print("serdes no" ..val.portParamsBuffer.activeLanesList[laneNum])
    return val.portParamsBuffer.activeLanesList[laneNum]
end

function GetPortLaneSpeed(devNum, portNum, laneNum)
    local ret, val
    local macNum

    --print("Portnum" .. portNum)
    ret, val = myGenWrapper("cpssDxChPortPhysicalPortMapGet", {
            {"IN", "GT_U8","devNum",devNum},
            {"IN", "GT_U32","firstPhysicalPortNumber",portNum},
            {"IN", "GT_U32", "portMapArraySize", 1 },
            {"OUT","CPSS_DXCH_PORT_MAP_STC","portMap"}  })

    if ret == 0 then
        if val.portMap.mappingType == "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
            macNum = val.portMap.interfaceNum
        else
           return SPEED_NA
        end
    else
        --print("Error at cpssDxChPortPhysicalPortMapGet")
        return SPEED_NA
    end
    ret, val = cpssGenWrapper("hwsPortModeParamsGetToBuffer",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","portGroup",0},
                {"IN","GT_U32","portNum",macNum},
                {"IN","MV_HWS_PORT_STANDARD","portMode","NON_SUP_MODE"},
                {"OUT","MV_HWS_PORT_INIT_PARAMS","portParamsBuffer"}})

    if 0 ~= ret then
        print("Error at hwsPortModeParamsGetToBuffer")
        return SPEED_NA
    end

    --print("serdes speed" ..val.portParamsBuffer.serdesSpeed)
    return val.portParamsBuffer.serdesSpeed
end

function GetNuberSerdesOfPort(devNum,portNum)

    local ret,val
    local ifMode
    local port_interface_mode
    local numOfSerdesLanesPtr=0



    ret,val = cpssPerPortParamGet("cpssDxChPortInterfaceModeGet",
                                         devNum, portNum, "mode", 
                                         "CPSS_PORT_INTERFACE_MODE_ENT")
    if 0 == ret then 

        port_interface_mode = val["mode"]
        if port_interface_mode ~= "CPSS_PORT_INTERFACE_MODE_NA_E" then 

            --print("port_interface_mode " .. port_interface_mode .. " of device ".. tostring(devNum) .. " port " .. tostring(portNum))

            ret, val = myGenWrapper("cpssDxChPortInterfaceModeGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"OUT","CPSS_PORT_INTERFACE_MODE_ENT","ifModePtr"}})

            if(ret~=0) then
                --print(tostring(devNum).."/"..tostring(portNum).." Error cpssDxChPortInterfaceModeGet ret "..to_string(ret).." = "..returnCodes[ret])
                return ret,0
            end

            ifMode=val["ifModePtr"] 

            ret, val = myGenWrapper("prvCpssDxChPortIfModeSerdesNumGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                {"IN","CPSS_PORT_INTERFACE_MODE_ENT","ifMode",ifMode},
                {"OUT","GT_U32","startSerdesPtr"},
                {"OUT","GT_U32","numOfSerdesLanesPtr"}})

            if(ret~=0) then
                --print(tostring(devNum).."/"..tostring(portNum).." Error prvCpssDxChPortIfModeSerdesNumGet ret "..to_string(ret).." = "..returnCodes[ret])
                return ret,0
            end

            startSerdesPtr=val["startSerdesPtr"]
            numOfSerdesLanesPtr=val["numOfSerdesLanesPtr"] 


        end --if val["mode"] ~= "CPSS_PORT_INTERFACE_MODE_QSGMII_Eof"

    else
        --print("Error at interface mode getting cpssDxChPortInterfaceModeGet " ..
        --                      "of device %d port %d: %s", devNum, 
        --                      portNum, returnCodes[ret])
    end


    if(ret==0 and numOfSerdesLanesPtr ~=nil and numOfSerdesLanesPtr > 0  ) then
        return ret,numOfSerdesLanesPtr
    else
        return ret,0
    end

end


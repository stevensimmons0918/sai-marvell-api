--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* px_show_interfaces_ap_introp.lua
--*
--* DESCRIPTION:
--*     Returns AP port introp information
--*
--*
--*
--*
--********************************************************************************

--includes


--constants


-- ************************************************************************
---
--  px_ap_introp_get_func
--        @description  returns ap port introp information
--
--
--
--        @return       error message if fails
--
local function px_ap_introp_get_func(params)
    local command_data = Command_Data()
    local devNum, portNum, ret
    local GT_OK         = 0
    local apIntrop      = {}
    local outVals

    local header_string1 =
        "\n" ..
        "Ap Introp\n" ..
        "---------\n" ..
        "Interface   attrBitMask     txDisDuration   abilityDuration  abilityMaxInterval abilityFailMaxInterval     \n" ..
        "--------- ---------------- ---------------- ---------------- ------------------ ----------------------     \n"
    local footer_string1 = "\n"

    local header_string2 =
        "\n" ..
        "Interface  apLinkDuration  apLinkMaxInterval  pdLinkDuration  pdLinkMaxInterval                            \n" ..
        "--------- ---------------- ----------------- ---------------- -----------------                            \n"
    local footer_string2 = "\n"

    local tableInformation          = {}
    local currentPortInformation    = {}
    local portCount                 = 0
    local index

    -- Common variables declaration
    local command_data = Command_Data()


    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)
    if params.all ~= nil then
        local dev, ports
        for dev,ports in pairs(command_data.dev_port_range) do
            table.insert(ports,"CPU")
        end
    end

    -- System specific data initialization.
    command_data:enablePausedPrinting()



    if true == command_data["status"] then
        local iterator
        for iterator, devNum, portNum in
                        command_data:getPortIterator() do
            command_data:clearPortStatus()
            if portNum == "CPU" then portNum = 16 end
            -- DevNum/PortNum string forming.
            devnum_portnum_string = alignLeftToCenterStr(tostring(devNum) ..
                                                         '/' ..
                                                         tostring(portNum), 9)
            -- Traffic statistic getting.
            command_data:clearLocalStatus()
            command_data:updateStatus()

            if(nil ~= devNum) then
                ret, localApParamsPtr = myGenWrapper("cpssPxPortApIntropGet",{
                    {"IN" ,"GT_U8"                          ,"devNum"       ,devNum },
                    {"IN" ,"GT_PHYSICAL_PORT_NUM"           ,"portNum"      ,portNum},
                    {"OUT","CPSS_PX_PORT_AP_INTROP_STC"   ,"apIntropPtr"          }})
                if (ret ~= GT_OK) then
                    local index = string.find(returnCodes[ret],"=")
                    print("Error at ap introp get command " .. devNum .. "/" .. portNum .. " : ".. string.sub(returnCodes[ret],index + 1))
                else
                    outVals                                             = localApParamsPtr["apIntropPtr"]
                    tableInformation[portCount] = {}
                    tableInformation[portCount].devPort                 = devNum .. "/" .. portNum
                    tableInformation[portCount].attrBitMask             = outVals.attrBitMask
                    tableInformation[portCount].txDisDuration           = outVals.txDisDuration
                    tableInformation[portCount].abilityDuration         = outVals.abilityDuration
                    tableInformation[portCount].abilityMaxInterval      = outVals.abilityMaxInterval
                    tableInformation[portCount].abilityFailMaxInterval  = outVals.abilityFailMaxInterval
                    tableInformation[portCount].apLinkDuration          = outVals.apLinkDuration
                    tableInformation[portCount].apLinkMaxInterval       = outVals.apLinkMaxInterval
                    tableInformation[portCount].pdLinkDuration          = outVals.pdLinkDuration
                    tableInformation[portCount].pdLinkMaxInterval       = outVals.pdLinkMaxInterval
                    portCount = portCount + 1
                end
            end
        end
    end

    if(portCount == 1) then
        print("\n" ..
              "Ap Introp\n" ..
              "---------\n")
        currentPortInformation = tableInformation[0]
        print("Interface:               "  .. currentPortInformation.devPort)
        print("attrBitMask:             "  .. currentPortInformation.attrBitMask)
        print("txDisDuration:           "  .. currentPortInformation.txDisDuration)
        print("abilityDuration:         "  .. currentPortInformation.abilityDuration)
        print("abilityMaxInterval:      "  .. currentPortInformation.abilityMaxInterval)
        print("abilityFailMaxInterval:  "  .. currentPortInformation.abilityFailMaxInterval)
        print("apLinkDuration:          "  .. currentPortInformation.apLinkDuration)
        print("apLinkMaxInterval:       "  .. currentPortInformation.apLinkMaxInterval)
        print("pdLinkDuration:          "  .. currentPortInformation.pdLinkDuration)
        print("pdLinkMaxInterval:       "  .. currentPortInformation.pdLinkMaxInterval)
    end

    if(portCount > 1) then
        print(header_string1)
        for index = 0,portCount - 1, 1 do
            currentPortInformation = tableInformation[index]
            print(
                    string.format("%-9s" , alignLeftToCenterStr(currentPortInformation.devPort,9))                               .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.attrBitMask),16))               .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.txDisDuration),16))             .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.abilityDuration),16))           .. " " ..
                    string.format("%-18s", alignLeftToCenterStr(to_string(currentPortInformation.abilityMaxInterval),18))        .. " " ..
                    string.format("%-22s", alignLeftToCenterStr(to_string(currentPortInformation.abilityFailMaxInterval),22))    .. " "
                    )
        end
        print(footer_string1)

        print(header_string2)
        for index = 0,portCount - 1, 1 do
            currentPortInformation = tableInformation[index]
            print(
                    string.format("%-9s" , alignLeftToCenterStr(currentPortInformation.devPort,9))                               .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.apLinkDuration),16))            .. " " ..
                    string.format("%-17s", alignLeftToCenterStr(to_string(currentPortInformation.apLinkMaxInterval),17))         .. " " ..
                    string.format("%-16s", alignLeftToCenterStr(to_string(currentPortInformation.pdLinkDuration),16))            .. " " ..
                    string.format("%-17s", alignLeftToCenterStr(to_string(currentPortInformation.pdLinkMaxInterval),17))         .. " "
                    )
        end
        print(footer_string2)
    end

    -- Command data postprocessing
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end


--------------------------------------------
-- command registration: show interfaces ap introp
--------------------------------------------
CLI_addCommand("exec", "show interfaces ap introp", {
  func   = px_ap_introp_get_func,
  help   = "Returns AP port introp information",
  params = {
        {
            type = "named",
            "#all_interfaces",
            mandatory = { "all_interfaces" }
        }
    }
  }
)


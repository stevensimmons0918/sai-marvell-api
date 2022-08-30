--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_tail_drop.lua
--*
--* DESCRIPTION:  show tail drop dump
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
---
local GT_OK=0

local function booleanToNum (bool)
    if bool then return 1 end
    return 0

end
local function booleanToStatus (bool)
    if bool then return "enabled" end
    return "disabled"

end

local alphaDict =
{
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E"] =     "0",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_125_E"] = "0.125",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E"] =  "0.25",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E"] =   "0.5",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E"] =     "1",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E"] =     "2",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E"] =     "4",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_8_E"] =     "8",
    --SIP 6
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_03125_E"] = "0.03125",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_0625_E"] = "0.0625",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_16_E"] = "16",
    ["CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E"] = "32"
}

local dropProbabilityDict =
{
    ["CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E"] = "100",
    ["CPSS_PORT_TX_TAIL_DROP_PROBABILITY_89_E"] =  "89",
    ["CPSS_PORT_TX_TAIL_DROP_PROBABILITY_73_E"] =  "73",
    ["CPSS_PORT_TX_TAIL_DROP_PROBABILITY_67_E"] =  "67",
    ["CPSS_PORT_TX_TAIL_DROP_PROBABILITY_50_E"] =  "50",
    ["CPSS_PORT_TX_TAIL_DROP_PROBABILITY_25_E"] =  "25"
}

local function getAvailableDbaBuffers(devNum)
    local devFamily, subFamily = wrlCpssDeviceFamilyGet(devNum);
    local policy_status
    local str,val,val2,ret

    if is_sip_5_20(devNum) ~= true then
        return "" --Do not print anything
    end

    if (is_sip_6(device)== false) then
     ret, val = myGenWrapper("cpssDxChPortTxDbaAvailBuffGet",{
        {"IN",  "GT_U8",  "devNum", devNum},
        {"OUT", "GT_U32", "maxBuffPtr"}
    })

    return string.format("Available DBA buffers: %d\n\n", val["maxBuffPtr"])
    else

         ret, val = myGenWrapper("cpssDxChPortTxTailDropGlobalParamsGet",{
            {"IN",  "GT_U8",  "devNum", devNum},
            {"OUT", "CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT", "resourceModePtr"},
            {"OUT", "GT_U32", "globalAvailableBuffersPtr"},
            {"OUT", "GT_U32", "pool0AvailableBuffersPtr"},
            {"OUT", "GT_U32", "pool1AvailableBuffersPtr"}
        })
        if(ret~=0)then
            return "Error "..ret.." calling cpssDxChPortTxTailDropGlobalParamsGet\n"
        end

        ret, val2 = myGenWrapper("cpssDxChPortTxMcastAvailableBuffersGet",{
            {"IN",  "GT_U8",  "devNum", devNum},
            {"OUT", "GT_U32", "mcastAvailableBuffersNumPtr"},
        })
        if(ret~=0)then
            return "Error "..ret.." calling cpssDxChPortTxMcastAvailableBuffersGet\n"
        end

        str=     "Available DBA buffers\n"
        str=str.."======================\n"
        str=str.."Mode     : %s\n"
        str=str.."Global   : %d\n"
        str=str.."Pool 0   : %d\n"
        str=str.."Pool 1   : %d\n"
        str=str.."Multicast: %d\n\n"
        return string.format(str, tostring(val["resourceModePtr"]),val["globalAvailableBuffersPtr"],
         val["pool0AvailableBuffersPtr"],val["pool1AvailableBuffersPtr"],val2["mcastAvailableBuffersNumPtr"])
    end
end

local function getTailDropPolicy(devNum)
    local devFamily, subFamily = wrlCpssDeviceFamilyGet(devNum);
    local policy_status
    local out_string

    if is_sip_5_20(devNum) ~= true then
        return "" --Do not print anything
    end



    if (devFamily == "CPSS_PP_FAMILY_DXCH_BOBCAT3_E") then
        if enabled == true then
            policy_status = "DBA firmware is loaded to CM3. Cannot determine whether  DBA is operational now"
        else
            policy_status = "static"
        end
    elseif (is_sip_6(device)== true) then
     policy_status = "dynamic" --sip6 only dynamic policy
     enabled = true
    else
     local ret, val = myGenWrapper("cpssDxChPortTxDbaEnableGet",{
        {"IN", "GT_U8",   "devNum", devNum},
        {"OUT", "GT_BOOL", "enablePtr"}
        })
        local enabled = val["enablePtr"]

        if enabled == true then
            policy_status = "dynamic"
        else
            policy_status = "static"
        end
    end

    out_string =  "Tail-drop policy: " .. policy_status .. "\n\n"

    if enabled == true then
        --Show available DBA buffers
        out_string = out_string .. getAvailableDbaBuffers(devNum)
    end

    return out_string
end

--  getTailDropConfiguration
--        @description  show tail drop
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--
local function getTailDropConfiguration(params)
    local devNum=params["device"]
    local notApplicableMsg="N/A"
    local errorMsg="ERR"
    local NOT_APPLICABLE=30
    local dataToPrint=""
    local errors=""
    local chunk, chunk2
    local enabled
    local maxTailDropProfileIndex = 16;

    local maxPort
    local ret, val
    local enqueEnable
    local wredSizeArray={}

    --Get tail-drop statistics enabled status
    ret, val = myGenWrapper("cpssDxChPortTxBufferStatisticsEnableGet",{
        {"IN", "GT_U8",   "devNum", devNum},
        {"OUT", "GT_BOOL", "enPortStatPtr"},
        {"OUT", "GT_BOOL", "enQueueStatPtr"}
      })
    if NOT_APPLICABLE == ret then
        -- Do nothing
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxBufferStatisticsEnableGet failed: "..returnCodes[ret].."\n"
    else
        if val["enPortStatPtr"] == true then enabled = "enabled" else enabled = "disabled" end
        dataToPrint=dataToPrint..string.format("Tail-drop statistics per port %s\n", enabled)
        if val["enQueueStatPtr"] == true then enabled = "enabled" else enabled = "disabled" end
        dataToPrint=dataToPrint..string.format("Tail-drop statistics per queue %s\n", enabled)
    end

    --getting max port for device
    ret, maxPort = wrLogWrapper("wrlCpssDevicePortNumberGet","(devNum)",devNum)
    if 0 ~= ret then
        errors=errors.."Couldn't get number of ports for device"..devNum..": "..returnCodes[ret]" \n"
    end
    maxPort = maxPort - 1
    portDigits=2
    if maxPort>99 then
        portDigits=3
    end


    local temp, temp2

    --Show tail-drop policy
    dataToPrint = dataToPrint .. getTailDropPolicy(devNum)

    --table of descriptors/buffers
    dataToPrint=dataToPrint..("Number of Buffers/Descriptors:\n")
    dataToPrint=dataToPrint..("==============================\n")
    dataToPrint=dataToPrint..("                    |   Buffers   |   Descriptors   |\n")


    --Total
    ret,val = myGenWrapper("cpssDxChPortTxGlobalBufNumberGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","numberPtr"}
        })
    temp=val
    if NOT_APPLICABLE == ret then
        temp={numberPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxGlobalBufNumberGet failed: "..returnCodes[ret].."\n"
        temp={numberPtr=errorMsg}
    end

    ret,val = myGenWrapper("cpssDxChPortTxGlobalDescNumberGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","numberPtr"}
        })

    if NOT_APPLICABLE == ret then
        val={numberPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxGlobalDescNumberGet failed: "..returnCodes[ret].."\n"
        val={numberPtr=errorMsg}
    end

    dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Total allocated",temp["numberPtr"],val["numberPtr"]).."\n")


    --Total limit

    ret,val = myGenWrapper("cpssDxChPortTxGlobalDescLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","limitPtr"}
        })

    if NOT_APPLICABLE == ret then
        val={limitPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxGlobalDescLimitGet failed: "..returnCodes[ret].."\n"
        val={limitPtr=errorMsg}
    end

    if(is_sip_6(devNum)==false)then
     dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Total limit","N/A",val["limitPtr"]).."\n")  --API for buffers limit not implemented yet
    else
     dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Total limit",val["limitPtr"],"N/A").."\n") --There is descriptors in SIP6 ,only virtual buffers
    end


    --Sniffer Tx and Rx
    ret,val = myGenWrapper("cpssDxChPortTxSniffedBufNumberGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","rxNumberPtr"},
        {"OUT","GT_U32","txNumberPtr"}
        })
    temp=val
    if NOT_APPLICABLE == ret then
        temp={rxNumberPtr=notApplicableMsg,
                txNumberPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxSniffedBufNumberGet failed: "..returnCodes[ret].."\n"
        temp={rxNumberPtr=errorMsg,
                txNumberPtr=errorMsg}
    end

    ret,val = myGenWrapper("cpssDxChPortTxSniffedDescNumberGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","rxNumberPtr"},
        {"OUT","GT_U32","txNumberPtr"}
        })

    if NOT_APPLICABLE == ret then
        val={rxNumberPtr=notApplicableMsg,
            txNumberPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxSniffedDescNumberGet failed: "..returnCodes[ret].."\n"
        val={rxNumberPtr=errorMsg,
             txNumberPtr=errorMsg}
    end

    dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Sniffer Tx allocated",temp["txNumberPtr"],val["txNumberPtr"]).."\n")
    dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Sniffer Rx allocated",temp["rxNumberPtr"],val["rxNumberPtr"]).."\n")




        --Sniffer Tx and Rx limit
    ret,val = myGenWrapper("cpssDxChPortTxSniffedBuffersLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","rxSniffMaxBufNumPtr"},
        {"OUT","GT_U32","txSniffMaxBufNumPtr"}
        })
    temp=val
    if NOT_APPLICABLE == ret then
        temp={rxSniffMaxBufNumPtr=notApplicableMsg,
              txSniffMaxBufNumPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxSniffedBuffersLimitGet failed: "..returnCodes[ret].."\n"
        temp={rxSniffMaxBufNumPtr=errorMsg,
                txSniffMaxBufNumPtr=errorMsg}
    end

    ret,val = myGenWrapper("cpssDxChPortTxSniffedPcktDescrLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","rxSniffMaxDescNumPtr"},
        {"OUT","GT_U32","txSniffMaxDescNumPtr"}
        })

    if NOT_APPLICABLE == ret then
        val={rxSniffMaxDescNumPtr=notApplicableMsg,
            txSniffMaxDescNumPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxSniffedPcktDescrLimitGet failed: "..returnCodes[ret].."\n"
        val={rxSniffMaxDescNumPtr=errorMsg,
            txSniffMaxDescNumPtr=errorMsg}
    end

    dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Sniffer Tx limit",temp["txSniffMaxBufNumPtr"],val["txSniffMaxDescNumPtr"]).."\n")
    dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","Sniffer Rx limit",temp["rxSniffMaxBufNumPtr"],val["rxSniffMaxDescNumPtr"]).."\n")



    --MC
    ret,val = myGenWrapper("cpssDxChPortTxMcastBufNumberGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","numberPtr"}
        })
    temp=val
    if NOT_APPLICABLE == ret then
        temp={numberPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxMcastBufNumberGet failed: "..returnCodes[ret].."\n"
        temp={numberPtr=errorMsg}
    end

    ret,val = myGenWrapper("cpssDxChPortTxMcastDescNumberGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","numberPtr"}
        })

    if NOT_APPLICABLE == ret then
        val={numberPtr=notApplicableMsg}
    elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxMcastDescNumberGet failed: "..returnCodes[ret].."\n"
        val={numberPtr=errorMsg}
    end

    dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","MC allocated",temp["numberPtr"],val["numberPtr"]).."\n")


        --MC limit
    if(is_sip_6(device)==false) then
     ret,val = myGenWrapper("cpssDxChPortTxMcastBuffersLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","mcastMaxBufNumPtr"}
        })
     temp=val
     if NOT_APPLICABLE == ret then
        temp={mcastMaxBufNumPtr=notApplicableMsg}
     elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxMcastBuffersLimitGet failed: "..returnCodes[ret].."\n"
        temp={mcastMaxBufNumPtr=errorMsg}
     end

     ret,val = myGenWrapper("cpssDxChPortTxMcastPcktDescrLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_U32","mcastMaxDescNumPtr"}
        })

     if NOT_APPLICABLE == ret then
        val={mcastMaxDescNumPtr=notApplicableMsg}
     elseif GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxMcastPcktDescrLimitGet failed: "..returnCodes[ret].."\n"
        val={mcastMaxDescNumPtr=errorMsg}
     end

     dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","MC limit",temp["mcastMaxBufNumPtr"],val["mcastMaxDescNumPtr"]).."\n")

     dataToPrint=dataToPrint.."\n"
    else
     --low
     ret,val = myGenWrapper("cpssDxChPortTxMcastBuffersPriorityLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E"},
        {"OUT","GT_U32","mcastGuaranteedLimitPtr"},
        {"OUT","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","alphaPtr"},
        })
     temp = val

     if GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxMcastBuffersPriorityLimitGet failed: "..returnCodes[ret].."\n"
        val={mcastMaxBufNumPtr=errorMsg}
     end
     dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","MC low prio limit",val["mcastGuaranteedLimitPtr"],"N/A").."\n")
     --high
      ret,val = myGenWrapper("cpssDxChPortTxMcastBuffersPriorityLimitGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","CPSS_PORT_TX_DROP_MCAST_PRIORITY_HI_E"},
        {"OUT","GT_U32","mcastGuaranteedLimitPtr"},
        {"OUT","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","alphaPtr"},
        })

     if GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxMcastBuffersPriorityLimitGet failed: "..returnCodes[ret].."\n"
        val={mcastMaxBufNumPtr=errorMsg}
     end
     dataToPrint=dataToPrint..(string.format("%-20s|%13s|%17s|","MC low prio high",val["mcastGuaranteedLimitPtr"],"N/A").."\n\n")
     dataToPrint=dataToPrint..(string.format("Alfa MC low  : %s\n",tostring(temp["alphaPtr"])))
     dataToPrint=dataToPrint..(string.format("Alfa MC high : %s\n\n",tostring(val["alphaPtr"])))
    end

    --[[ Tail drop global configuration]]
    ret,val = myGenWrapper("cpssDxChPortTxTailDropUcEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })
    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..(string.format("UC Tail Drop status:[%s]",notApplicableMsg).."\n")
    elseif GT_OK == ret then
        dataToPrint=dataToPrint..(string.format("UC Tail Drop status:[%s]",booleanToStatus(val["enablePtr"])).."\n")
    else
        errors=errors.."cpssDxChPortTxTailDropUcEnableGet failed: "..returnCodes[ret].."\n"
    end



    ret,val = myGenWrapper("cpssDxChPortTxBufferTailDropEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })


    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..(string.format("Buffer Tail Drop status:[%s]",notApplicableMsg).."\n")
    elseif GT_OK == ret then
        dataToPrint=dataToPrint..(string.format("Buffer Tail Drop status:[%s]",booleanToStatus(val["enablePtr"])).."\n")
    else
        errors=errors.."cpssDxChPortTxBufferTailDropEnableGet failed: "..returnCodes[ret].."\n"
    end


    ret,val = myGenWrapper("cpssDxChPortTxRandomTailDropEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })

    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..(string.format("Random Tail Drop status:[%s]",notApplicableMsg).."\n")
    elseif GT_OK == ret then
        dataToPrint=dataToPrint..(string.format("Random Tail Drop status:[%s]",booleanToStatus(val["enablePtr"])).."\n")
    else
        errors=errors.."cpssDxChPortTxRandomTailDropEnableGet failed: "..returnCodes[ret].."\n"
    end


    --[[ enable queueing ]]
    --[[ global Tail Drop ]]

    enum={
        CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E="Byte",
        CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E="Packet"
         }

    ret,val = myGenWrapper("cpssDxChPortTxTailDropBufferConsumptionModeGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT","modePtr"},
        {"OUT","GT_U32","lengthPtr"}
        })
    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..(string.format("Tail Drop Consumption mode:[%s]",notApplicableMsg).."\n")
    elseif GT_OK == ret then
        if "Byte"==enum[val["modePtr"]] then
            dataToPrint=dataToPrint..("Tail Drop Consumption mode:[Byte]\n")
        else
            dataToPrint=dataToPrint..(string.format("Tail Drop Consumption mode:[Packet], length:[%s]",val["lengthPtr"]).."\n")
        end
    else
        errors=errors.."cpssDxChPortTxTailDropBufferConsumptionModeGet failed: "..returnCodes[ret].."\n"
    end

    ret,val = myGenWrapper("cpssDxChPortTxQueueEnableGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","GT_BOOL","enablePtr"}
        })

    if GT_OK ~= ret then
        errors=errors.."cpssDxChPortTxQueueEnableGet failed: "..returnCodes[ret].."\n"
    else
        dataToPrint=dataToPrint..(string.format("Tx Queueing status:[%s]",booleanToStatus(val["enablePtr"])).."\n")
    end

    enum={
        CPSS_DXCH_PORT_TX_SHARED_POLICY_UNCONSTRAINED_E="unconstrained",
        CPSS_DXCH_PORT_TX_SHARED_POLICY_CONSTRAINED_E="constrained"
         }

    ret,val = myGenWrapper("cpssDxChPortTxSharedPolicyGet",{
        {"IN","GT_U8","devNum",devNum},
        {"OUT","CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT","policyPtr"}
        })
    if NOT_APPLICABLE == ret then
        dataToPrint=dataToPrint..(string.format("Tail Drop Shared Pool Share Policy:[%s]\n",notApplicableMsg).."\n")
    elseif GT_OK == ret then
        if "unconstrained"==enum[val["policyPtr"]] then
            dataToPrint=dataToPrint..("Tail Drop Shared Pool Share Policy:[unconstrained]\n\n")
        else
            dataToPrint=dataToPrint..("Tail Drop Shared Pool Share Policy:[constrained]\n\n")
        end
    else
        errors=errors.."cpssDxChPortTxSharedPolicyGet failed: "..returnCodes[ret].."\n"
    end


    chunk=""
    dataToPrint=dataToPrint..("TC EnQ/DeQ enabled/disabled:\n")
    dataToPrint=dataToPrint..("==============================================================================================\n")

    dataToPrint=dataToPrint..(" Port|    TC0   |    TC1   |    TC2   |     TC3  |     TC4  |    TC5   |    TC6   |    TC7   |\n")
    dataToPrint=dataToPrint..("     |  EnQ  DeQ|  EnQ  DeQ|  EnQ  DeQ|  EnQ  DeQ|  EnQ  DeQ|  EnQ  DeQ|  EnQ  DeQ|  EnQ  DeQ|\n")
    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then
            ret,txQ = myGenWrapper("prvCpssDxChPortPhysicalPortMapCheckAndConvert",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN",TYPE["ENUM"],"convertedPortType",4}, --PRV_CPSS_DXCH_PORT_TYPE_TXQ_E
                    {"OUT","GT_U32","convertedPortNumPtr"}
                    })

            if GT_OK ~= ret then

                if txQ["convertedPortNumPtr"] == 0xffffffff then  --CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS
                    chunk=string.format("%5s|",portNum)
                    chunk=chunk..(string.format("%3s %3s|%3s %3s|%3s %3s|%3s %3s|%3s %3s|%3s %3s|%3s %3s|%3s %3s|","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A","N/A"))
                    dataToPrint=dataToPrint..(chunk.."\n")
                else
                    errors=errors.."prvCpssDxChPortPhysicalPortMapCheckAndConvert failed: "..returnCodes[ret].."\n"
                end

            else
                chunk=string.format("%5s|",portNum)
                for tcQueue = 0,7 do
                  if(is_sip_6(device)==false)then
                     ret,val = myGenWrapper("cpssDxChPortTxQueueingEnableGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U8","tcQueue",tcQueue},
                        {"OUT","GT_BOOL","enablePtr"}
                        })

                     temp=val
                     if GT_OK ~= ret then
                        errors=errors.."cpssDxChPortTxQueueingEnableGet failed: "..returnCodes[ret].."\n"
                        temp={enablePtr=errorMsg}
                     end

                     enqueEnable = booleanToNum(temp["enablePtr"])
                    else
                     enqueEnable = "1" --for sip6 enqueue is always enabled
                    end --end of if(is_sip_6(device)==false)

                    ret,val = myGenWrapper("cpssDxChPortTxQueueTxEnableGet",{
                        {"IN","GT_U8","devNum",devNum},
                        {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                        {"IN","GT_U8","tcQueue",tcQueue},
                        {"OUT","GT_BOOL","enablePtr"}
                        })
                    if GT_OK ~= ret then
                        errors=errors.."cpssDxChPortTxQueueTxEnableGet failed: "..returnCodes[ret].."\n"
                        val={enablePtr=errorMsg}
                    end
                    chunk=chunk..(string.format("%3s %4s  |",enqueEnable,booleanToNum(val["enablePtr"])))
                end
                dataToPrint=dataToPrint..(chunk.."\n")
            end
        end
    end

    profileToNum =
    {
                CPSS_PORT_TX_DROP_PROFILE_1_E = 1,
                CPSS_PORT_TX_DROP_PROFILE_2_E = 2,
                CPSS_PORT_TX_DROP_PROFILE_3_E = 3,
                CPSS_PORT_TX_DROP_PROFILE_4_E = 4,
                CPSS_PORT_TX_DROP_PROFILE_5_E = 5,
                CPSS_PORT_TX_DROP_PROFILE_6_E = 6,
                CPSS_PORT_TX_DROP_PROFILE_7_E = 7,
                CPSS_PORT_TX_DROP_PROFILE_8_E = 8,
                CPSS_PORT_TX_DROP_PROFILE_9_E = 9,
                CPSS_PORT_TX_DROP_PROFILE_10_E = 10,
                CPSS_PORT_TX_DROP_PROFILE_11_E = 11,
                CPSS_PORT_TX_DROP_PROFILE_12_E = 12,
                CPSS_PORT_TX_DROP_PROFILE_13_E = 13,
                CPSS_PORT_TX_DROP_PROFILE_14_E = 14,
                CPSS_PORT_TX_DROP_PROFILE_15_E = 15,
                CPSS_PORT_TX_DROP_PROFILE_16_E = 16
    }
    chunk=""
    counter=0


    for portNum = 0,maxPort do
        --[[ skip not existed ports ]]
        if does_port_exist(devNum, portNum) then

            ret,txQ = myGenWrapper("prvCpssDxChPortPhysicalPortMapCheckAndConvert",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"IN",TYPE["ENUM"],"convertedPortType",4}, --PRV_CPSS_DXCH_PORT_TYPE_TXQ_E
                    {"OUT","GT_U32","convertedPortNumPtr"}
                    })

            if GT_OK ~= ret then

                if txQ["convertedPortNumPtr"] == 0xffffffff then  --CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS
                    -- nothing to do, the port has no txQ
                else
                    errors=errors.."prvCpssDxChPortPhysicalPortMapCheckAndConvert failed: "..returnCodes[ret].."\n"
                end

            else
                ret,val = myGenWrapper("cpssDxChPortTxBindPortToDpGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","GT_PHYSICAL_PORT_NUM","portNum",portNum},
                    {"OUT","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSetPtr"}
                    })

                if GT_OK ~= ret then
                    errors=errors.."cpssDxChPortTxBindPortToDpGet failed: "..returnCodes[ret].."\n"
                else
                    if 0 == portNum then
                        chunk=chunk.."\nPort<->Tail Drop Profile Binding: [port:TD Profile]\n"
                    end
                    if ((counter % 8) == 0) and 0 ~= portNum then
                        chunk=chunk.."\n"
                    end
                    counter=counter+1
                    chunk=chunk..string.format("[%3s:%2s] ",portNum,profileToNum [val["profileSetPtr"]])
                end
            end
        end
    end
    dataToPrint=dataToPrint..(chunk.."\n\n")
    if "" ~= errors then
        dataToPrint=dataToPrint..("Errors:\n-------\n"..errors.."\n\n")
    end

   tdProfileToString =
   {
    "CPSS_PORT_TX_DROP_PROFILE_1_E",
    "CPSS_PORT_TX_DROP_PROFILE_2_E",
    "CPSS_PORT_TX_DROP_PROFILE_3_E",
    "CPSS_PORT_TX_DROP_PROFILE_4_E",
    "CPSS_PORT_TX_DROP_PROFILE_5_E",
    "CPSS_PORT_TX_DROP_PROFILE_6_E",
    "CPSS_PORT_TX_DROP_PROFILE_7_E",
    "CPSS_PORT_TX_DROP_PROFILE_8_E",
    "CPSS_PORT_TX_DROP_PROFILE_9_E",
    "CPSS_PORT_TX_DROP_PROFILE_10_E",
    "CPSS_PORT_TX_DROP_PROFILE_11_E",
    "CPSS_PORT_TX_DROP_PROFILE_12_E",
    "CPSS_PORT_TX_DROP_PROFILE_13_E",
    "CPSS_PORT_TX_DROP_PROFILE_14_E",
    "CPSS_PORT_TX_DROP_PROFILE_15_E",
    "CPSS_PORT_TX_DROP_PROFILE_16_E"
    }

    dataToPrint=dataToPrint..("\n\nTail Drop profiles\n")
    dataToPrint=dataToPrint..("==================\n")
    if not is_sip_5(devNum) then
        maxTailDropProfileIndex = 8
    end
      for tailDropProfileIndex=1,maxTailDropProfileIndex do
        tailDropProfile=tdProfileToString [tailDropProfileIndex]
        ret,val = myGenWrapper("cpssDxChPortTxTailDropProfileGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet",tailDropProfile},
            {"OUT","CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT","portAlphaPtr"},
            {"OUT","GT_U32","portMaxBuffLimitPtr"},
            {"OUT","GT_U32","portMaxDescrLimitPtr"}
            })

        if GT_OK ~= ret then
            errors=errors.."cpssDxChPortTxTailDropProfileGet failed: "..returnCodes[ret].."\n"
            dataToPrint=dataToPrint..(string.format("profile:[%3s] \n"..errorMsg,
                        tailDropProfile))
        else
            if is_sip_6(devNum) then

            ret,temp = myGenWrapper("cpssDxChPortTxTailDropWredProfileGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet",tailDropProfile},
                {"OUT","CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC","tailDropWredProfileParamsPtr"},
                })
             if GT_OK ~= ret then
              errors=errors.."cpssDxChPortTxTailDropProfileGet failed: "..returnCodes[ret].."\n"
              dataToPrint=dataToPrint..(string.format("profile:[%3s] \n"..errorMsg,
                        tailDropProfile))
             else
              dataToPrint=dataToPrint..(string.format(
                    "profile:[%3s] \n"..
                    "Guaranteed Buffer Limit:[%s]\n"..
                    "Alpha:[%s]\n",
                    tailDropProfile,
                    temp["tailDropWredProfileParamsPtr"]["guaranteedLimit"],
                    tostring(val["portAlphaPtr"])
                ))
             end
            elseif is_sip_5_20(devNum) then
                dataToPrint=dataToPrint..(string.format(
                    "profile:[%3s] \n"..
                    "Max buffers Limit:[%s]\n"..
                    "Max Descriptors Limit:[%s]\n"..
                    "Alpha:[%s]\n",
                    tailDropProfile,
                    val["portMaxBuffLimitPtr"],
                    val["portMaxDescrLimitPtr"],
                    alphaDict[val["portAlphaPtr"]]
                ))
            else
                dataToPrint=dataToPrint..(string.format("profile:[%3s] \nMax buffers Limit:[%s]\nMax Descriptors Limit:[%s]\n",
                            tailDropProfile,val["portMaxBuffLimitPtr"],val["portMaxDescrLimitPtr"]))
            end
        end

        if is_sip_6(devNum) then
        dataToPrint=dataToPrint..( "------------------------------------------------------------------------------------------------------------------------------\n");
        chunk= (" TC|Max Buf|DP0 Grn Buf|DP0 Alpha|DP0 Drop Pr|DP1 Grn Buf|DP1 Alpha|DP1 Drop Pr|DP2 Grn Buf|DP2 Alpha|DP2 Drop Pr|\n")
        elseif is_sip_5_20(devNum) then
            dataToPrint=dataToPrint..( "------------------------------------------------------------------------------------------------------------------------------\n");
            chunk= (" TC|Max Buf|Max Desc|DP0 Max Buf|DP0 Max Desc|DP0 Alpha|DP1 Max Buf|DP1 Max Desc|DP1 Alpha|DP2 Max Buf|DP2 Max Desc|DP2 Alpha|\n")
        else
            dataToPrint=dataToPrint..("------------------------------------------------------------------\n");
            chunk= (" TC|Max Buf|Max Desc|DP0 Max Buf|DP0 Max Desc|DP1 Max Buf|DP1 Max Desc|DP2 Max Buf|DP2 Max Desc|\n")
        end
        for tcQueue = 0,7 do
            ret,val = myGenWrapper("cpssDxChPortTx4TcTailDropProfileGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet",tailDropProfile},
                {"IN","GT_U8","trafficClass",tcQueue},
                {"OUT","CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC","tailDropProfileParamsPtr"}
                })


            if GT_OK ~= ret then
                errors=errors.."cpssDxChPortTx4TcTailDropProfileGet failed: "..returnCodes[ret].."\n"
                chunk=string.format("%3s | "..errorMsg,tcQueue).."\n"
            else
                if is_sip_6(devNum) then
                ret,temp = myGenWrapper("cpssDxChPortTx4TcTailDropWredProfileGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","profileSet",tailDropProfile},
                    {"IN","GT_U8","trafficClass",tcQueue},
                    {"OUT","CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC","tailDropProfileParamsPtr"}
                    })
                if GT_OK ~= ret then
                 errors=errors.."cpssDxChPortTx4TcTailDropWredProfileGet failed: "..returnCodes[ret].."\n"
                 chunk=string.format("%3s | "..errorMsg,tcQueue).."\n"
                end

                wredSizeArray[1+tailDropProfileIndex*16 +tcQueue*8+0] = temp["tailDropProfileParamsPtr"]["dp0WredAttributes"]["wredSize"]
                wredSizeArray[1+tailDropProfileIndex*16 +tcQueue*8+1] = temp["tailDropProfileParamsPtr"]["dp1WredAttributes"]["wredSize"]
                wredSizeArray[1+tailDropProfileIndex*16 +tcQueue*8+2] = temp["tailDropProfileParamsPtr"]["dp2WredAttributes"]["wredSize"]


                chunk=chunk..string.format("%3s|%7s|%11s|%9s|%11s|%11s|%9s|%11s|%11s|%9s|%11s|\n",tcQueue,
                                val["tailDropProfileParamsPtr"]["tcMaxBuffNum"],
                                temp["tailDropProfileParamsPtr"]["dp0WredAttributes"]["guaranteedLimit"],
                                alphaDict[val["tailDropProfileParamsPtr"]["dp0QueueAlpha"]],
                                dropProbabilityDict[temp["tailDropProfileParamsPtr"]["dp0WredAttributes"]["probability"]],
                                temp["tailDropProfileParamsPtr"]["dp1WredAttributes"]["guaranteedLimit"],
                                alphaDict[val["tailDropProfileParamsPtr"]["dp1QueueAlpha"]],
                                dropProbabilityDict[temp["tailDropProfileParamsPtr"]["dp1WredAttributes"]["probability"]],
                                temp["tailDropProfileParamsPtr"]["dp2WredAttributes"]["guaranteedLimit"],
                                alphaDict[val["tailDropProfileParamsPtr"]["dp2QueueAlpha"]],
                                dropProbabilityDict[temp["tailDropProfileParamsPtr"]["dp2WredAttributes"]["probability"]]
                                )
                elseif is_sip_5_20(devNum) then
                    chunk=chunk..string.format("%3s|%7s|%8s|%11s|%12s|%9s|%11s|%12s|%9s|%11s|%12s|%9s|\n",tcQueue,
                                val["tailDropProfileParamsPtr"]["tcMaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["tcMaxDescrNum"],
                                val["tailDropProfileParamsPtr"]["dp0MaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["dp0MaxDescrNum"],
                                alphaDict[val["tailDropProfileParamsPtr"]["dp0QueueAlpha"]],
                                val["tailDropProfileParamsPtr"]["dp1MaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["dp1MaxDescrNum"],
                                alphaDict[val["tailDropProfileParamsPtr"]["dp1QueueAlpha"]],
                                val["tailDropProfileParamsPtr"]["dp2MaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["dp2MaxDescrNum"],
                                alphaDict[val["tailDropProfileParamsPtr"]["dp2QueueAlpha"]]
                                )
                else
                    chunk=chunk..string.format("%3s|%7s|%8s|%11s|%12s|%11s|%12s|%11s|%12s|",tcQueue,
                                val["tailDropProfileParamsPtr"]["tcMaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["tcMaxDescrNum"],
                                val["tailDropProfileParamsPtr"]["dp0MaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["dp0MaxDescrNum"],
                                val["tailDropProfileParamsPtr"]["dp1MaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["dp1MaxDescrNum"],
                                val["tailDropProfileParamsPtr"]["dp2MaxBuffNum"],
                                val["tailDropProfileParamsPtr"]["dp2MaxDescrNum"]).."\n"
                end
            end
        end
        dataToPrint=dataToPrint..(chunk.."\n")
    end

    --WRED size
    if is_sip_6(devNum) then
     for tailDropProfileIndex=1,maxTailDropProfileIndex do
      dataToPrint=dataToPrint..("Profile "..tailDropProfileIndex.."\n")
      for tcQueue = 0,7 do
        dataToPrint=dataToPrint..("WRED size TC"..tcQueue)
        for ii = 0,2 do
         dataToPrint=dataToPrint..(" DP "..ii.." = "..wredSizeArray[1+tailDropProfileIndex*16 +tcQueue*8+ii] )
        end
        dataToPrint=dataToPrint..("\n")
       end
      end
    end

     flag=false
    dataToPrint=dataToPrint..("\nRandom tail Drop\n")
    dataToPrint=dataToPrint..("================\n")
    for tailDropProfileIndex=1,maxTailDropProfileIndex do
        tailDropProfile= tdProfileToString[tailDropProfileIndex]
        dataToPrint=dataToPrint..(string.format("\nTail Drop Limits Enabled/Disabled For WRTD Per profile:[%s]\n",tailDropProfile))
        dataToPrint=dataToPrint..("----------------------------------------------------------------\n")

        for tcQueue = 0,7 do

            for ii = 0,2 do
                ret,val = myGenWrapper("cpssDxChPortTxProfileWeightedRandomTailDropEnableGet",{
                    {"IN","GT_U8","devNum",devNum},
                    {"IN","CPSS_PORT_TX_DROP_PROFILE_SET_ENT","pfSet",tailDropProfile},
                    {"IN","GT_U32","dp",ii},
                    {"IN","GT_U8","tc",tcQueue},
                    {"OUT","CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC","enablersPtr"}
                    })



                if NOT_APPLICABLE == ret then
                    dataToPrint=dataToPrint..(notApplicableMsg.."\n")
                    flag=true
                    break
                elseif GT_OK ~= ret then
                    if 2 ~=ii or 4 ~= ret then
                        errors=errors.."cpssDxChPortTxProfileWeightedRandomTailDropEnableGet failed: "..returnCodes[ret].."\n"
                    end
                else
                    if 0 == tcQueue and 0 == ii then
                        dataToPrint=dataToPrint..(" TC | DP | portLimit | shared Pool Limit | tcDpLimit | tcLimit |\n")
                    end
                    dataToPrint=dataToPrint..(string.format("%3s |%4s|%11s|%19s|%11s|%9s|\n", tcQueue,ii,
                                 booleanToNum(val["enablersPtr"]["portLimit"]),
                                 booleanToNum(val["enablersPtr"]["sharedPoolLimit"]),
                                 booleanToNum(val["enablersPtr"]["tcDpLimit"]),
                                 booleanToNum(val["enablersPtr"]["tcLimit"])))

                    end
            end
            if flag then
                break
            end
            dataToPrint=dataToPrint..("    |    |           |                   |           |         |\n")

        end
        if flag then
            break
        end
    end



    lines = splitString(dataToPrint, "\n")
    for ii=1,#lines do
        print(lines[ii])
    end
end

--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
--CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show tail-drop", {
  func   =  function(params)
              do_command_with_print_nice("getTailDropConfiguration", getTailDropConfiguration, params)
            end,
  help   = 'The dump of all tail drop related information',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          mandatory = {"device"}}
  }
})

function luaWrap_cpssDxChPortTx4TcBufNumberGet(command_data, devNum, portNum, tc)
    local ret, val
    local portNumBuf

    ret,val = myGenWrapper("cpssDxChPortTx4TcBufNumberGet",{
        {"IN",  "GT_U8",                "devNum",    devNum},
        {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
        {"IN",  "GT_U8",                "trafClass", tc},
        {"OUT", "GT_U32",               "numPtr"}
    })
    if GT_OK ~= ret then
        command_data:addError("cpssDxChPortTx4TcBufNumberGet failed: "..returnCodes[ret].."\n")
        command_data:setFailPortAndLocalStatus()
        portNumBuf = 0
    else
        portNumBuf = val["numPtr"]
    end
    return ret, portNumBuf
end

function luaWrap_cpssDxChPortTx4TcDescNumberGet(command_data, devNum, portNum, tc)
    local ret, val
    local portNumDesc

    ret,val = myGenWrapper("cpssDxChPortTx4TcDescNumberGet",{
        {"IN",  "GT_U8",                "devNum",    devNum},
        {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum",   portNum},
        {"IN",  "GT_U8",                "trafClass", tc},
        {"OUT", "GT_U16",               "numberPtr"}
    })
    if GT_OK ~= ret then
        command_data:addError("cpssDxChPortTx4TcDescNumberGet failed: "..returnCodes[ret].."\n")
        command_data:setFailPortAndLocalStatus()
        portNumDesc = 0
    else
        portNumDesc = val["numberPtr"]
    end
    return ret, portNumDesc
end

function luaWrap_cpssDxChPortTxBufNumberGet(command_data, devNum, portNum)
    local ret,val
    local portNumBuf

    ret,val = myGenWrapper("cpssDxChPortTxBufNumberGet",{
        {"IN",  "GT_U8",                "devNum",devNum},
        {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum",portNum},
        {"OUT", "GT_U32",               "numPtr"}
    })
    if GT_OK ~= ret then
        command_data:addError("cpssDxChPortTxBufNumberGet failed: "..returnCodes[ret].."\n")
        command_data:setFailPortAndLocalStatus()
        portNumBuf = 0
    else
        portNumBuf = val["numPtr"]
    end
    return ret, portNumBuf
end

function luaWrap_cpssDxChPortTxDescNumberGet(command_data, devNum, portNum)
    local ret,val
    local portNumDesc

    ret,val = myGenWrapper("cpssDxChPortTxDescNumberGet",{
        {"IN",  "GT_U8",                "devNum",devNum},
        {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum",portNum},
        {"OUT", "GT_U16",               "numberPtr"}
    })
    if GT_OK ~= ret then
        command_data:addError("cpssDxChPortTxDescNumberGet failed: "..returnCodes[ret].."\n")
        command_data:setFailPortAndLocalStatus()
        portNumDesc = 0
    else
        portNumDesc = val["numberPtr"]
    end
    return ret, portNumDesc
end

local function showAllocatedResourcesPortRange(params, command_data)
    local header_string1, header_string2, footer_string1, footer_string2
    local tc
    local portNumDesc, portNumBuf
    local txDescs, txBufs

    if params["tc"] == nil then
        header_string =
            "-------------------------------------------------------------------------------------------------\n"..
            "     |          |                          Per Port and Traffic Class                           |\n"..
            "Port | Per port | TC0     | TC1     | TC2     | TC3     | TC4     | TC5     | TC6     | TC7     |\n"..
            "-----|----------|---------|---------|---------|---------|---------|---------|---------|---------|"
        footer_string =
            "-------------------------------------------------------------------------------------------------\n"
        string_format =
            "%d/%-3d|%-10d|%-9d|%-9d|%-9d|%-9d|%-9d|%-9d|%-9d|%-9d|"
    else
        header_string = string.format(
            "----------------\n"..
            "Port | TC%d     |\n"..
            "-----|----------", params["tc"])
        string_format =
            "%d/%-3d|%-9d|"
        footer_string = "----------------\n"
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        if portNum == "CPU" then portNum = 63 end

        command_data:clearLocalStatus()

        if true == command_data["local_status"] then
            if params["isBuffers"] == true then
                if params["tc"] ~= nil then
                    ret, val = luaWrap_cpssDxChPortTx4TcBufNumberGet(command_data, devNum, portNum, params["tc"])
                    command_data["result"] =
                        string.format(string_format, devNum, portNum, val)
                    command_data:addResultToResultArray()
                    command_data:updatePorts()
                end
                ret, portNumBuf = luaWrap_cpssDxChPortTxBufNumberGet(command_data, devNum, portNum)
                txBufs = {}
                for tc=0,7 do
                    ret, txBufs[tc] = luaWrap_cpssDxChPortTx4TcBufNumberGet(command_data, devNum, portNum, tc)
                end
                command_data["result"] =
                    string.format(string_format, devNum, portNum, portNumBuf,
                        txBufs[0], txBufs[1], txBufs[2], txBufs[3], txBufs[4],
                        txBufs[5], txBufs[6], txBufs[7])
                command_data:addResultToResultArray()
                command_data:updatePorts()
            else
                if params["tc"] ~= nil then
                    ret,val = luaWrap_cpssDxChPortTx4TcDescNumberGet(command_data, devNum, portNum, params["tc"])
                    command_data["result"] =
                        string.format(string_format, devNum, portNum, val)
                    command_data:addResultToResultArray()
                    command_data:updatePorts()
                end
                ret, portNumDesc = luaWrap_cpssDxChPortTxDescNumberGet(command_data, devNum, portNum)
                txDescs = {}
                for tc=0,7 do
                    ret, txDescs[tc] = luaWrap_cpssDxChPortTx4TcDescNumberGet(command_data, devNum, portNum, tc)
                end
                command_data["result"] =
                    string.format(string_format, devNum, portNum, portNumDesc,
                        txDescs[0], txDescs[1], txDescs[2], txDescs[3], txDescs[4],
                        txDescs[5], txDescs[6], txDescs[7])
                command_data:addResultToResultArray()
                command_data:updatePorts()
            end
        end
        command_data:updatePorts()
    end
    command_data:setResultArrayToResultStr(1, 1)
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
        footer_string, "There is no port information to show.\n")
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function showAllocatedResources(params)
    local result, values
    -- Common variables declaration
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    result, values = showAllocatedResourcesPortRange(params, command_data)

    return result, values
end

--------------------------------------------------------------------------------
-- command registration: show tail-drop-allocated
--------------------------------------------------------------------------------
CLI_type_dict["traffic_class"] = {
    checker = CLI_check_param_number,
    min = 0,
    max = 7,
    help = "Traffic class"
}

CLI_type_dict["descbuff"] = {
    checker  = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help     = "Choose buffers and descriptors",
    enum = {
        ["buffers"]=     {value=true,  help="Shows allocated buffers"},
        ["descriptors"]= {value=false, help="Shows allocated descriptors"}
    }
}

CLI_addHelp("exec", "show tail-drop-allocated", "Show current number of allocated buffers and descriptors per port, per traffic class")
CLI_addCommand("exec", "show tail-drop-allocated", {
  func   =  function(params)
              showAllocatedResources(params)
            end,
  help   = "Show current number of allocated buffers and descriptors per port, per traffic class",
  params = {
    { type = "values", {format="%descbuff", name = "isBuffers"}},
    { type = "named",
      "#all_interfaces_ver1",
      { format = "tc %traffic_class", name = "tc", help = "Traffic class"},
      mandatory = { "all_interfaces" }
    }
  }
})



--  getTailDropSharedPoolBufs
--        @description  show tail drop shared pool buffer/descriptions
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--
local function getTailDropSharedPoolBufs(params)
    local devNum=params["device"]
    local notApplicableMsg="N/A"
    local errorMsg="ERR"
    local NOT_APPLICABLE=30
    local dataToPrint=""
    local errors=""
    local ret, val
    local temp,temp1
    local maxPoolNum = 7

    if is_sip_6(devNum) then
     maxPoolNum = 1
     dataToPrint=dataToPrint..( "|--------|------------------------------|\n")
     dataToPrint=dataToPrint..( "| Shared |           buffers            |\n")
     dataToPrint=dataToPrint..( "|  pool  |        [in Use/Limit]        |\n")
     dataToPrint=dataToPrint..( "|--------|------------------------------|\n")
    else
     dataToPrint=dataToPrint..( "|--------|------------------------------|------------------------------|\n")
     dataToPrint=dataToPrint..( "| Shared |           buffers            |         Descriptors          |\n")
     dataToPrint=dataToPrint..( "|  pool  |        [in Use/Limit]        |        [In Use/Limit]        |\n")
     dataToPrint=dataToPrint..( "|--------|------------------------------|------------------------------|\n")
    end





    local temp,temp1

    for pool = 0,maxPoolNum do

        if is_sip_6(devNum) then
        ret,val = myGenWrapper("prvCpssSip6TxqPoolCounterGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","poolId",pool},
            {"OUT","GT_U32","numberBufferPtr"}
            })
        else
        ret,val = myGenWrapper("cpssDxChPortTxSharedResourceBufNumberGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","tcQueue",pool},
            {"OUT","GT_U32","numberBufferPtr"}
            })
        end

        temp=val
        if NOT_APPLICABLE == ret then
            temp={numberBufferPtr=notApplicableMsg}
        elseif GT_OK ~= ret then
            errors=errors.."cpssDxChPortTxSharedResourceBufNumberGet failed: "..returnCodes[ret].."\n"
            temp={numberBufferPtr=errorMsg}
        end

        if is_sip_6(devNum)~=true then
            ret,val = myGenWrapper("cpssDxChPortTxSharedResourceDescNumberGet",{
                {"IN","GT_U8","devNum",devNum},
                {"IN","GT_U32","tcQueue",pool},
                {"OUT","GT_U32","numberDescPtr"}
                })
            temp1=val
            if NOT_APPLICABLE == ret then
                temp1={numberDescPtr=notApplicableMsg}
            elseif GT_OK ~= ret then
                errors=errors.."cpssDxChPortTxSharedResourceDescNumberGet failed: "..returnCodes[ret].."\n"
                temp1={numberDescPtr=errorMsg}
            end
        end

        ret,val = myGenWrapper("cpssDxChPortTxSharedPoolLimitsGet",{
            {"IN","GT_U8","devNum",devNum},
            {"IN","GT_U32","poolNum",pool},
            {"OUT","GT_U32","maxBufNumPtr"},
            {"OUT","GT_U32","maxDescNumPtr"}
            })
        if NOT_APPLICABLE == ret then
            val={maxBufNumPtr=notApplicableMsg,
                maxDescNumPtr=notApplicableMsg}
        elseif GT_OK ~= ret then
            errors=errors.."cpssDxChPortTxSharedPoolLimitsGet failed: "..returnCodes[ret].."\n"
            val={maxBufNumPtr=errorMsg,
                 maxDescNumPtr=errorMsg}
        end
        if is_sip_6(devNum)~=true then
         dataToPrint=dataToPrint..string.format("|   %2d   |    %11s/%4s          |    %11s/%4s          |\n",pool,temp["numberBufferPtr"],val["maxBufNumPtr"],temp1["numberDescPtr"],val["maxDescNumPtr"])
        else
         dataToPrint=dataToPrint..string.format("|   %2d   |    %11s/%4s          |\n",pool,temp["numberBufferPtr"],val["maxBufNumPtr"])
        end
    end

    if is_sip_6(devNum) then
     dataToPrint=dataToPrint..( "|--------|------------------------------|\n")
    else
     dataToPrint=dataToPrint..( "|--------|------------------------------|------------------------------|\n")
    end

    lines = splitString(dataToPrint, "\n")
    for ii=1,#lines do
        print(lines[ii])
    end
end

CLI_addHelp("exec", "show tail-drop-shared-pool-allocated", "Show current number of allocated Buffers/Descriptors per shared pool")
CLI_addCommand("exec", "show tail-drop-shared-pool-allocated", {
  func   =  function(params)
              do_command_with_print_nice("getTailDropSharedPoolBufs", getTailDropSharedPoolBufs, params)
            end,
  help   = 'Show current number of allocated Buffers/Descriptors per shared pool',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device"},
                          mandatory = {"device"}}
  }
})



local function showTailDropStatisticsPortRange(params, command_data)
    local header_string, footer_string, string_format
    local tc

    if params["tc"] == nil then
        header_string = "Show statistics per port\n"
    else
        header_string = string.format("Show statistics per traffic class %d\n", params["tc"])
    end

    header_string = header_string..
        "--------------------\n"..
        "     |             |\n"..
        "Port | Max Buffers |\n"..
        "--------------------"
    footer_string =
        "--------------------\n"

    string_format =
        "%d/%-3d|%-13d|"

    for iterator, devNum, portNum in command_data:getPortIterator() do
        command_data:clearPortStatus()
        if portNum == "CPU" then portNum = 63 end

        command_data:clearLocalStatus()

        if true == command_data["local_status"] then
            if params["tc"] ~= nil then
                ret,val = myGenWrapper("cpssDxChPortTxQueueMaxBufferFillLevelGet",{
                    {"IN",  "GT_U8",                "devNum",  devNum},
                    {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    {"IN",  "GT_U8",                "tc",      params["tc"]},
                    {"OUT", "GT_U32",               "maxBuffFillLvlPtr"}
                })
                if GT_OK ~= ret then
                    command_data:addError("cpssDxChPortTxQueueMaxBufferFillLevelGet failed: "..returnCodes[ret].."\n")
                    command_data:setFailPortAndLocalStatus()
                else
                    val = val["maxBuffFillLvlPtr"]
                end
                command_data["result"] =
                    string.format(string_format, devNum, portNum, val)
                command_data:addResultToResultArray()
                command_data:updatePorts()

                ret,val = myGenWrapper("cpssDxChPortTxQueueMaxBufferFillLevelSet",{
                    {"IN",  "GT_U8",                "devNum",  devNum},
                    {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    {"IN",  "GT_U8",                "tc",      params["tc"]},
                    {"IN",  "GT_U32",               "maxBuffFillLvl", 0}
                })
                if GT_OK ~= ret then
                    command_data:addError("cpssDxChPortTxQueueMaxBufferFillLevelSet failed: "..returnCodes[ret].."\n")
                    command_data:setFailPortAndLocalStatus()
                end
            else
                ret,val = myGenWrapper("cpssDxChPortTxMaxBufferFillLevelGet",{
                    {"IN",  "GT_U8",                "devNum",  devNum},
                    {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    {"OUT", "GT_U32",               "maxBuffFillLvlPtr"}
                })
                if GT_OK ~= ret then
                    command_data:addError("cpssDxChPortTxMaxBufferFillLevelGet failed: "..returnCodes[ret].."\n")
                    command_data:setFailPortAndLocalStatus()
                else
                    val = val["maxBuffFillLvlPtr"]
                end
                command_data["result"] =
                    string.format(string_format, devNum, portNum, val)
                command_data:addResultToResultArray()
                command_data:updatePorts()

                ret,val = myGenWrapper("cpssDxChPortTxMaxBufferFillLevelSet",{
                    {"IN",  "GT_U8",                "devNum",  devNum},
                    {"IN",  "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                    {"IN",  "GT_U32",               "maxBuffFillLvl", 0}
                })
                if GT_OK ~= ret then
                    command_data:addError("cpssDxChPortTxMaxBufferFillLevelSet failed: "..returnCodes[ret].."\n")
                    command_data:setFailPortAndLocalStatus()
                end
            end
        end
        command_data:updatePorts()
    end
    command_data:setResultArrayToResultStr(1, 1)
    command_data:setResultStrOnPortCount(header_string, command_data["result"],
        footer_string, "There is no port information to show.\n")
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function showTailDropStatistics(params)
    local result, values
    local devNum  = devEnv.dev
    -- Common variables declaration
    local command_data = Command_Data()

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllInterfacesPortIterator(params)

    -- System specific data initialization.
    command_data:enablePausedPrinting()

    --this command only support Aldrin2. Here check if device is SIP6(Falcon) to avoid run command for higher devices
   if is_sip_6(devNum) then
      print("This command is not supported by current device. It supports Aldrin2 only!\n")
      command_data:disablePausedPrinting()
      return
   end

   if is_sip_5_25(devNum) then
    result, values = showTailDropStatisticsPortRange(params, command_data)
   else
    print("This command is not supported by current device. It supports Aldrin2 only!\n")
    command_data:disablePausedPrinting()
    return
   end

    return result, values
end

CLI_addHelp("exec", "show tail-drop-statistics", "Show statistics per port or per port per queue (if traffic class specified)")
CLI_addCommand("exec", "show tail-drop-statistics", {
  func   =  showTailDropStatistics,
  help   = "Show statistics per port or per port per queue (if traffic class specified)",
  params = {
    { type = "named",
      "#all_interfaces_ver1",
      { format = "tc %traffic_class", name = "tc", help = "Traffic class"},
      mandatory = { "all_interfaces" }
    }
  }
})

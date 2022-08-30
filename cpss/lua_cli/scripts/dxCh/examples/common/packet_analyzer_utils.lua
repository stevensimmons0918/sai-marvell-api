
--********************************************************************************
--*              (C), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* packet_analyzer_utils.lua
--*
--* DESCRIPTION:
--*       Any functions, variables etc that are required in several tests.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

function packetAnalyzerCheckHits(mode,givenStage,givenField,expStages,expfield)
    local err = 0
    local groupId = 1
    local actionId = 1
    local keyId = 1
    local ruleId = 1
    local managerId =1
    local stages = {}
    local numOfValidStages, numOfApplicStages
    local applicStage, validStage
    local result
    local field

    printLog ("\n")
    printLog ("checkHits() started ")


    for i,vals in pairs(paDb.paTable) do
        if vals["stage"] == givenStage then
            groupId = paDb.paTable[i]["group"]
            keyId = paDb.paTable[i]["id"]
            ruleId = paDb.paTable[i]["id"]
        end
    end

    if  mode == "no rule" then --free mode no rule expected hits >0 on all stages
        if givenStage == nil and givenField == nil  then
            printLog("no rule set, expecte hits > 0 in all stages")

            -- prepare valid stages list
            result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStagesGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "INOUT", "GT_U32", "numOfApplicStagesPtr", 30 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "applicStagesListArr" },
                                { "INOUT", "GT_U32", "numOfValidStagesPtr", 30 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT[30]", "validStagesListArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStagesGet returned "..err)
            end

            if values["numOfValidStagesPtr"] ~= 0 then
                for i = 1, values["numOfValidStagesPtr"] do
                    validStaget = values["validStagesListArr"][i-1]
                    validStage = printPaStageCpssInt[validStaget]
                    stages[i]= printPaStageCpssIndex[validStage]
                end
            end

            for k=1,#stages do
                result, numOfHits = wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet(paDb.managerId,ruleId,groupId,stages[k])
                if (result ~=0) then
                    err = returnCodes[result]
                    printLog("wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet returned "..err)
                    return err
                end
                if numOfHits == 0 then
                    err = returnCodes[0x01]
                    printLog("stage "..stages[k])
                    printLog("error: num Of hits not as expected ,got "..tostring(numOfHits).."while expecte hit > 0")
                    return err
                end
            end
        else
            printLog("no rule set, checking specific stage and field")
            printLog(givenStage)
            strField= printPaField[givenField]

            result, values = myGenWrapper(
                "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "GT_U32", "ruleId", ruleId},
                                { "IN", "GT_U32", "groupId", groupId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", givenStage},
                                { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
            end

            found = false
            for i=1, values["numOfSampleFieldsPtr"] do
                if values["sampleFieldsValueArr"][i-1].fieldName == printPaFieldCpss[givenField] then

                    fieldsValues[1] = values["sampleFieldsValueArr"][i-1].data[0]
                    fieldsValues[2] = values["sampleFieldsValueArr"][i-1].data[1]
                    fieldsValues[3] = values["sampleFieldsValueArr"][i-1].data[2]
                    fieldsValues[4] = values["sampleFieldsValueArr"][i-1].data[3]

                    found = true
                    break
                end
            end

            if found == false then
                err = returnCodes[0x01]
                printLog("error: didn't have sampling for field "..tostring(strField))
                return err
            end

            if( fieldsValues[1] == expfield[strField]["data"][1] and
                fieldsValues[2] == expfield[strField]["data"][2] and
                fieldsValues[3] == expfield[strField]["data"][3] and
                fieldsValues[4] == expfield[strField]["data"][4])then
                printLog(strField..": hits and data as expected !")
            else
                printLog(strField..": not as expected !")
                for k=1,4 do
                    printLog("expected")
                    printLog(expfield[strField]["data"][k])
                    printLog("got")
                    printLog(fieldsValues[k])
                end
                err = returnCodes[0x01]
                return err
            end
        end
        return err
    else -- rule seted
        printLog("rule is set, expecte hits as expected")
        for k,currentStage in pairs(expStages) do
            printLog(currentStage["stage"])

            result, values = myGenWrapper(
                 "cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet", {
                                { "IN", "GT_U32", "managerId", paDb.managerId},
                                { "IN", "GT_U32", "keyId", keyId},
                                { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", currentStage["stage"]},
                                { "INOUT", "GT_U32", "numOfFieldsPtr", 120 },
                                { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT[120]", "fieldsArr" }
                 }
            )
            if (result ~= 0) then
                status = false
                err = returnCodes[result]
                print("cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet returned "..err)
            end

            result, numOfHits = wrlCpssDxChPacketAnalyzerRuleMatchHitsOnAllStagesGet(paDb.managerId,ruleId,groupId,currentStage["stage"])
            if (result ~=0) then
                err = returnCodes[result]
                printLog("wrlCpssDxChPacketAnalyzerLogicalKeyFieldPerStageGet returned "..err)
                return err
            end

            --check hits as expected
            if currentStage["hit"] ~= numOfHits then
                printLog("error: number Of hits not as expected ,got "..tostring(numOfHits).." while expecte "..tostring(currentStage["hit"]))
                err = returnCodes[0x01]
                return err
            end

            if (currentStage["hit"] ==0) then
                printLog("hit == 0 as expected !")
            else
                for i = 1, values["numOfFieldsPtr"] do
                    fieldt = values["fieldsArr"][i-1]
                    field = printPaFieldCpssInt[fieldt]
                    strField= printPaField[field]

                    --check that field is in the expected fields
                    if expfield[strField] == nil then
                        printLog("error : "..strField.." is not in expected field array !")
                        err = returnCodes[0x01]
                        return err
                    end

                    result, values1 = myGenWrapper(
                        "cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet", {
                                        { "IN", "GT_U32", "managerId", paDb.managerId},
                                        { "IN", "GT_U32", "ruleId", ruleId},
                                        { "IN", "GT_U32", "groupId", groupId},
                                        { "IN", "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT", "stageId", currentStage["stage"]},
                                        { "OUT", "GT_U32", "numOfHitsPtr", numOfHitsPtr},
                                        { "INOUT", "GT_U32", "numOfSampleFieldsPtr", 120 },
                                        { "OUT", "CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC[120]", "sampleFieldsValueArr" }
                        }
                    )
                    if (result ~= 0) then
                        status = false
                        err = returnCodes[result]
                        print("cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet returned "..err)
                    end

                    found = false
                    for m=1, values1["numOfSampleFieldsPtr"] do
                        if values1["sampleFieldsValueArr"][m-1].fieldName == fieldt then

                            fieldsValues[1] = values1["sampleFieldsValueArr"][m-1].data[0]
                            fieldsValues[2] = values1["sampleFieldsValueArr"][m-1].data[1]
                            fieldsValues[3] = values1["sampleFieldsValueArr"][m-1].data[2]
                            fieldsValues[4] = values1["sampleFieldsValueArr"][m-1].data[3]

                            found = true
                            break
                        end
                    end

                    if found == false then
                        err = returnCodes[0x01]
                        printLog("error: didn't have sampling for field "..tostring(strField))
                        return err
                    end

                    if( fieldsValues[1] == expfield[strField]["data"][1] and
                        fieldsValues[2] == expfield[strField]["data"][2] and
                        fieldsValues[3] == expfield[strField]["data"][3] and
                        fieldsValues[4] == expfield[strField]["data"][4])then
                        printLog(strField..": hits and data as expected !")
                    else
                        printLog(strField..": not as expected !")
                        for j=1,4 do
                            printLog("expected")
                            printLog(expfield[strField]["data"][j])
                            printLog("got")
                            printLog(fieldsValues[j])
                        end
                        err = returnCodes[0x01]
                        return err
                    end
                end --end for numOfFields
            end --end currentStage["hit"] ==0
        end-- end for #expStages
    end--else mode ~= 0
    return err
end

function packetAnalyzerCheckReturnStatus(rc)
    if rc ~= 0 then
        printLog ("FAIL  \n ".. rc)
        setFailState()
    else
        printLog ("SUCCESS \n ")
    end
end



function getNumberOfLinkUpPorts(devNum)
    local result,maxPhysicalPort,portNum,values,sum
    local MacValidity,portValid
    sum=0

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
            return err,sum
        end

        --skip 'CPU SDMA' and 'remote'
        portValid = values.portMapShadowPtr.valid
        mappingType = values.portMapShadowPtr.portMap.mappingType
        if tostring(mappingType) ~= "CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E" then
            MacValidity = false
        else
            MacValidity = true
        end

        if ((portValid == true ) and (MacValidity == true )) then
            result, values = myGenWrapper("cpssDxChPortLinkStatusGet", {
                { "IN", "GT_U8", "devNum", devNum},
                { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum },
                { "OUT", "GT_BOOL", "linkUp" }
            })
            if (result ~= 0) then
                err = returnCodes[result]
                print("error : cpssDxChPortLinkStatusGet returned "..err)
                return err,sum
            end

            if values.linkUp == true then
                sum=sum+1
            end
        end
    end

    return 0,sum
end




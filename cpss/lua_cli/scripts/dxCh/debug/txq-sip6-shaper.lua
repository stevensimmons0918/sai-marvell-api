--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq-sip6-shaper.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for shaping status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************


local function dump_shaper_config(params)
    local res,val,shaperFreqInK,ind,tile,perInter,cscd,lvl,nodeType,alvl
    local device = tonumber(params["devID"])
    local port =   tonumber(params["portNum"])
    ---[[
    if(is_sip_6(device))then
        res, val = myGenWrapper("prvCpssFalconTxqUtilsIsCascadePort", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "port",  port},
                { "OUT", "GT_BOOL", "isCascadePtr"}
        })
        if(res~=0)then
            print("Error "..res.." calling prvCpssFalconTxqUtilsIsCascadePort")
            return
        end
        if (val["isCascadePtr"]==true)then
        cscd =true
        alvl=false
        print("Cascade port detected")
        res, val = myGenWrapper("prvCpssSip6TxqUtilsPhysicalPortToPnodeIndexGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "port",  port},
                { "OUT", "GT_BOOL", "isCascadePtr"},
                { "OUT", "GT_U32", "pNodeNumPtr"},
                { "OUT", "GT_U32", "tileNumPtr"},
        })
        else
        res, val = myGenWrapper("prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "port",  port},
                { "OUT", "GT_U32", "tileNumPtr"},
                { "OUT", "GT_U32", "aNodeNumPtr"},
        })
        alvl=true
        end
        if(res==0)then
          tile =  val["tileNumPtr"]
          if(cscd==true)then
           ind = val["pNodeNumPtr"]
           print("Port is mapped to tile "..val["tileNumPtr"].." P node "..val["pNodeNumPtr"])
           lvl=2
           nodeType ="B"
          else
           ind = val["aNodeNumPtr"]
           print("Port is mapped to tile "..val["tileNumPtr"].." A node "..val["aNodeNumPtr"])
           lvl=1
           nodeType ="A"
          end

          res, val = myGenWrapper("prvCpssFalconTxqPdqShaperInputFreqGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "OUT", "GT_U32", "freqPtr"},
           })
           if(res~=0)then
            print("Error "..res.." calling prvCpssFalconTxqPdqShaperInputFreqGet")
            return
           end
           shaperFreqInK = val["freqPtr"]/1000

           print("\nShaper frequency in Mhz : "..shaperFreqInK/1000)

           res, val = myGenWrapper("prvCpssSip6TxqPdqPerLevelShapingParametersGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "IN", "GT_U32", "level",lvl},
                { "OUT", "GT_U32", "shaperDecPtr"},
                { "OUT", "GT_U32", "perInterPtr"},
                { "OUT", "GT_U32", "perEnPtr"},
           })
           print("Level             : "..nodeType)
           print("Shaper dec        : "..val["shaperDecPtr"])
           print("Periodic interval : "..val["perInterPtr"])
           print("Periodic enable   : "..val["perEnPtr"])
           perInter = val["perEnPtr"]

           if(res~=0)then
            print("Error "..res.." calling prvCpssSip6TxqPdqPerLevelShapingParametersGet")
            return
           end

           res, val = myGenWrapper("cpssDxChPortTxShaperProfileGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "port",  port},
                { "OUT", "GT_U32", "burstSizePtr"},
                { "OUT", "GT_U32", "maxRatePtr"},
                { "OUT", "GT_BOOL", "enablePtr"},
           })

           if(res~=0)then
            print("Error "..res.." calling cpssDxChPortTxShaperProfileGet")
            return
           end

           print("Shaping enabled at the "..nodeType.." node : "..tostring(val["enablePtr"]))
          if(val["enablePtr"])then
            if(val["maxRatePtr"]>=1*1000*1000) then
             print("Effective BW   in Gbit        :"..val["maxRatePtr"]/(1*1000*1000))
            elseif (val["maxRatePtr"]>=1*1000) then
             print("Effective BW   in Mbit        :"..val["maxRatePtr"]/1000)
            else
             print("Effective BW   in Kbit        :"..val["maxRatePtr"])
            end
            res, val = myGenWrapper("prvCpssSip6TxqPdqNodeShapingParametersGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32" , "tileNum",  tile},
                    { "IN", "GT_U32" , "nodeInd",  ind},
                    { "IN", "GT_BOOL" , "aLevel", alvl},
                    { "OUT", "GT_U32", "divExpPtr"},
                    { "OUT", "GT_U32", "tokenPtr"},
                    { "OUT", "GT_U32", "resPtr"},
                    { "OUT", "GT_U32", "burstPtr"},
               })

            if(res~=0)then
                print("Error "..res.." calling prvCpssSip6TxqPdqNodeShapingParametersGet")
                return
            end

            print("Divider exponent              :"..val["divExpPtr"])
            print("Number of tokens              :"..val["tokenPtr"])
            print("Resolution exponent           :"..val["resPtr"])
            print("Max burst size(Kbyte)         :"..val["burstPtr"])
            print("Effective burst size(Kbyte)   :"..(val["tokenPtr"]*(2^val["resPtr"] ))/1000)
           end

        else
            print("Failed to find port mapping ")
        end
    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end
 --]]
end


local function dump_shaper_config_port_tc(params)
    local res,val,shaperFreqInK,ind,tile,perInter
    local firstPdqQueue,lastPdqQueue
    local device = tonumber(params["devID"])
    local port =   tonumber(params["portNum"])
    local queue =   tonumber(params["queueNum"])
    ---[[
    if(is_sip_6(device))then
        res, val = myGenWrapper("prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "port",  port},
                { "OUT", "GT_U32", "tileNumPtr"},
                { "OUT", "GT_U32", "aNodeNumPtr"},
        })

        if(res==0)then
          tile =  val["tileNumPtr"]
          ind = val["aNodeNumPtr"]
          print("Port is mapped to tile "..val["tileNumPtr"].." A node "..val["aNodeNumPtr"])
          res, val = myGenWrapper("prvCpssFalconTxqPdqShaperInputFreqGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "OUT", "GT_U32", "freqPtr"},
           })
           if(res~=0)then
            print("Error "..res.." calling prvCpssFalconTxqPdqShaperInputFreqGet")
            return
           end
           shaperFreqInK = val["freqPtr"]/1000

           print("\nShaper frequency in Mhz : "..shaperFreqInK/1000)

           res, val = myGenWrapper("prvCpssSip6TxqPdqPerLevelShapingParametersGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  tile},
                { "IN", "GT_U32", "level",0},
                { "OUT", "GT_U32", "shaperDecPtr"},
                { "OUT", "GT_U32", "perInterPtr"},
                { "OUT", "GT_U32", "perEnPtr"},
           })

           print("Shaper dec        : "..val["shaperDecPtr"])
           print("Periodic interval : "..val["perInterPtr"])
           print("Periodic enable   : "..val["perEnPtr"])
           perInter = val["perEnPtr"]

           if(res~=0)then
            print("Error "..res.." calling prvCpssSip6TxqPdqPerLevelShapingParametersGet")
            return
           end

           res, val = myGenWrapper("cpssDxChPortTxQShaperProfileGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "port",  port},
                { "IN", "GT_U8" , "tcQueue",  queue},
                { "OUT", "GT_U32", "burstSizePtr"},
                { "OUT", "GT_U32", "maxRatePtr"},
                { "OUT", "GT_BOOL", "enablePtr"},
           })

           if(res~=0)then
            print("Error "..res.." calling cpssDxChPortTxShaperProfileGet")
            return
           end

           print("Shaping enabled at the Q node : "..tostring(val["enablePtr"]))
          if(val["enablePtr"])then
            if(val["maxRatePtr"]>=1*1000*1000) then
             print("Effective BW   in Gbit        :"..val["maxRatePtr"]/(1*1000*1000))
            elseif (val["maxRatePtr"]>=1*1000) then
             print("Effective BW   in Mbit        :"..val["maxRatePtr"]/1000)
            else
             print("Effective BW   in Kbit        :"..val["maxRatePtr"])
            end

            res, val = myGenWrapper("cpssDxChPortPhysicalPortDetailedMapGet", {
                        { "IN", "GT_U8"  , "devNum", device},
                        { "IN", "GT_U32" , "portNum",  port},
                        { "OUT", "CPSS_DXCH_DETAILED_PORT_MAP_STC", "portMapShadowPtr"},
                })

            if(res~=0)then
             print("Error "..res.." calling cpssDxChPortPhysicalPortDetailedMapGet")
             return
            end

            firstPdqQueue = val["portMapShadowPtr"].extPortMap.txqInfo.primarySchFirstQueueIndex
            ind = firstPdqQueue+queue


            res, val = myGenWrapper("prvCpssFalconTxqPdqGetQueueShapingParameters", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32" , "tileNum",  tile},
                    { "IN", "GT_U32" , "nodeInd",  ind},
                    { "IN", "GT_U32" , "rateType",  0--[[PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT]]},
                    { "OUT", "GT_U32", "divExpPtr"},
                    { "OUT", "GT_U32", "tokenPtr"},
                    { "OUT", "GT_U32", "resPtr"},
                    { "OUT", "GT_U32", "burstPtr"},
               })

            if(res~=0)then
                print("Error "..res.." calling prvCpssFalconTxqPdqGetQueueShapingParameters")
                return
            end

            print("Divider exponent              :"..val["divExpPtr"])
            print("Number of tokens              :"..val["tokenPtr"])
            print("Resolution exponent           :"..val["resPtr"])
            print("Max burst size(Kbyte)          :"..val["burstPtr"])
            print("Effective burst size(Kbyte)    :"..(val["tokenPtr"]*(2^val["resPtr"] ))/1000)

           end
        else
            print("Failed to find port mapping ")
        end
    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end
 --]]
end

CLI_addHelp("debug", "txq-sip6-shaper", "Dump SIP6 txq shaper information")
--------------------------------------------------------------------------------
---- command registration: txq-sip6-shaper show-port-config
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-shaper show-port-config", {
   func = dump_shaper_config,
   help = "Dump shaper configuration for port",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
        mandatory={"devID","portNum"},
        }
    }
})



--------------------------------------------------------------------------------
---- command registration: txq-sip6-shaper show-port-config
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-shaper show-port-tc-config", {
   func = dump_shaper_config_port_tc,
   help = "Dump shaper configuration for port/tc",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
        { format="queue %queue",name="queueNum",help="The queue number" },
        mandatory={"devID","portNum","queueNum"},
        }
    }
})



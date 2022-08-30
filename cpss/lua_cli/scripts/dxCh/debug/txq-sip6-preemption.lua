--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq-sip6-preemption.lua
--*
--* DESCRIPTION:
--*       Basic debug menu and backdoor configurations for preemption status
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
dofile("dxCh/debug/txq-sip6.lua")


preemptive_queues = {0,0,0,0,0,0,0,0}
local preemption_profile_show_queue

function set_one_bit(value,offset)
 local bit
 if(value==nil)then
   print("nil value")
   return 0
  else
   bit = (math.floor(value/2^offset)%2)
   if bit==1 then
   --already set
     return value
   else
    return value+2^offset
   end
  end
end


local function dumpBrStatus(rawBrStatus)
 print("BR status :")
 print("TX_VERIFY_STATUS  "..get_bit(rawBrStatus,0,3).." [Indicates the current status of the verification.]")
 print("TX_PREEMPT_STATUS "..get_bit(rawBrStatus,3,1).." [Set to 1 when a frame is being preempted. Debug information only.]")
 print("ASSY_ERR_LH       "..get_bit(rawBrStatus,4,1).." [Seen RX assembly error. Cleared on read]")
 print("SMD_ERR_LH        "..get_bit(rawBrStatus,5,1).." [Seen invalid SMD. Cleared on read.]")
 print("VERIF_SEEN_LH     "..get_bit(rawBrStatus,6,1).." [Seen good or bad VERIFY frame. Cleared on read.]")
 print("RESP_SEEN_LH      "..get_bit(rawBrStatus,7,1).." [Seen good or bad RESPONSE frame. Cleared on read.")
end

local function dumpBrControl(rawBrControl)
 print("BR control :")
 print("TX_PREEMPT_EN           "..get_bit(rawBrControl,0,1).." [Enables preemption.]")
 print("TX_VERIF_DIS            "..get_bit(rawBrControl,1,1).." [Bypass preemption verification.]")
 print("TX_ADDFRAGSIZE          "..get_bit(rawBrControl,4,3).." [Minimum fragment size in increments of 64 bytes.]")
 print("TX_VERIFY_TIME          "..get_bit(rawBrControl,8,7).." [Preemption verification timeout in milliseconds.]")
 print("TX_PAUSE_PRI_EN         "..get_bit(rawBrControl,16,1).." [When set to 1 PAUSE frames are giving priority over EMAC frames.]")
 print("TX_FRAG_PAUSE_EN        "..get_bit(rawBrControl,17,1).." [When set to 1 the PMAC is allowed to be paused in the middle of a fragmented frame.]")
 print("TX_NON_EMPTY_PREEMPT_EN "..get_bit(rawBrControl,18,1).." [When set to 1 the PMAC can be preempted by the tx_empty indication from the EMAC transmit FIFO.]")
 print("TX_ALLOW_PMAC_IF_NVERIF "..get_bit(rawBrControl,19,1).." [When set to 0, PMAC frames are not transmitted unless preemption verification has succeeded.]")
 print("TX_ALLOW_EMAC_IF_NVERIF "..get_bit(rawBrControl,20,1).." [When set to 0, EMAC frames are not transmitted unless preemption verification has succeeded.]")
 print("BR_COUNT_CLR_ON_RD      "..get_bit(rawBrControl,21,1).." [When set to 1 reading from the BR counters cause them to be cleared.]")
 print("BR_COUNT_SAT            "..get_bit(rawBrControl,22,1).." [Selects whether to saturate (value of 1) or roll over (value of 0) for the counters]")
 print("RX_STRICT_PREAMBLE      "..get_bit(rawBrControl,23,1).." [When set to 1, the preamble is checked so all bytes except the SFD are 0x55.]")
 print("PMAC_TO_EMAC_STATS      "..get_bit(rawBrControl,24,1).." [When set to 1 (default) the PMAC statistics for both RX and TX are counted together with the EMAC statistics.]")
 print("BR_RX_SMD_DIS           "..get_bit(rawBrControl,25,1).." [When set to 1 the receiver does not decode the 802.3BR SMDs and assumes all frames are express frames.]")
end

local function getBr(device,port)
 local res,val2,brStatus,brCntrl
 res, val2 = myGenWrapper("prvCpssDxChPortSip6BrSingleParameterGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", port},
                { "IN", "GT_U32", "inBrPrm",1},
                { "OUT", "GT_U32", "outBrPrm"},
           })
 if(res~=0)then
   print("Error "..res.." calling prvCpssDxChPortSip6BrSingleParameterGet")
   return res,0,0
 end

 brCntrl = val2.outBrPrm

 res, val2 = myGenWrapper("prvCpssDxChPortSip6BrSingleParameterGet", {
     { "IN", "GT_U8"  , "devNum", device},
     { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", device},
     { "IN", "GT_U32", "inBrPrm",2},
     { "OUT", "GT_U32", "outBrPrm"},
 })
 if(res~=0)then
  print("Error "..res.." calling prvCpssDxChPortSip6BrSingleParameterGet")
  return res,0,0
 end

 brStatus = val2.outBrPrm

 return res,brStatus,brCntrl

end

local function has_value (tab, val)
  if tab[val+1]==1 then
   return true
 end
  return false
end

local function configurePreemptiveProfileWithDsaCmd (devNum,queue,enable,dsaCmd)

    for i=1,2 do
     for j=1,3 do
      tcDpRemapping = {
        tc = queue,
        dp = "CPSS_DP_GREEN_E",
        isStack = "CPSS_DXCH_PORT_PROFILE_NETWORK_E",
        dsaTagCmd = dsaCmd,
        targetPortTcProfile = "CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E",
        packetIsMultiDestination = false
      }
     --set for UC and MC
      if(i==2)then
       tcDpRemapping.packetIsMultiDestination=true
      end

      if(j==2)then
       tcDpRemapping.dp="CPSS_DP_YELLOW_E"
      elseif (j==3)then
       tcDpRemapping.dp="CPSS_DP_RED_E"
      end

      res,val = myGenWrapper("cpssDxChCscdQosTcDpRemapTableGet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
                {"OUT","GT_U32","newTcPtr"},
                {"OUT","CPSS_DP_LEVEL_ENT","newDpPtr"},
                {"OUT","GT_U32","newPfcTPtrc"},
                {"OUT","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriorityPtr"},
                {"OUT","CPSS_DP_FOR_RX_ENT","dpForRxPtr"},
                {"OUT","GT_BOOL","preemptiveTcPtr"},
      })

      if(res~=0)then
        print("Error "..res.." calling cpssDxChCscdQosTcDpRemapTableGet")
        return
      end

      res = myGenWrapper("cpssDxChCscdQosTcDpRemapTableSet",{
        {"IN","GT_U32","devNum",devNum},
        {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
        {"IN","GT_U32","newTc",queue},
        {"IN","CPSS_DP_LEVEL_ENT","newDp","CPSS_DP_GREEN_E"},
        {"IN","GT_U32","newPfcTc",queue},
        {"IN","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriority","CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E"},
        {"IN","CPSS_DP_FOR_RX_ENT","dpForRx",val.dpForRxPtr},
        {"IN","GT_BOOL","preemptiveTc",enable},
      })

       if(res~=0)then
        print("Error "..res.." calling cpssDxChCscdQosTcDpRemapTableSet")
        return
       end
      end -- end of  for j=1,3
     end -- end of for i=1,2 do
end
local function  configurePreemptiveProfile (devNum,queue,enable)
  configurePreemptiveProfileWithDsaCmd (devNum,queue,enable,"CPSS_DXCH_NET_DSA_CMD_FORWARD_E")

  configurePreemptiveProfileWithDsaCmd (devNum,queue,enable,"CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E")
end


local function hwSync()
  print("Queue state sync to HW :")
  for tc = 0,7 do
    --sync to HW
    if(preemptive_queues[tc+1]==1)then
     print(tc.." true")
     configurePreemptiveProfile(device,tc,true)
    else
     configurePreemptiveProfile(device,tc,false)
     print(tc.." false")
    end
  end --end of for tc = 0,7 do

end
local function preemption_profile_configure_queue(params)

    local device = tonumber(params["devID"])
    local queue =  tonumber(params["queueNum"])
    local profileDisplayParam={}

    if(is_supported_feature(device, "PREEMPTION_MAC_802_3BR")) then
     --first sync with HW
     profileDisplayParam.devID = device
     profileDisplayParam.countOnly=true
     profileDisplayParam.sync = true
     preemption_profile_show_queue(profileDisplayParam)

        if(params.enable==true)then
         --insert only once
         if(has_value(preemptive_queues,queue) == false)then
          print("Add TC ".. params["queueNum"].." to CPSS_DXCH_PORT_PROFILE_NETWORK_E profile.")
          preemptive_queues[queue+1]=1
         end
        else
         if(has_value(preemptive_queues,queue) == true)then
          preemptive_queues[queue+1]=0
          print("Remove TC ".. params["queueNum"].." from CPSS_DXCH_PORT_PROFILE_NETWORK_E profile.")
         end
        end

    --now sync to HW
    hwSync()

    else
        print("Device "..device ..  " is not supported since it is not SIP 6_10.")
    end
end

function preemption_profile_show_queue(params)

    local device = tonumber(params["devID"])

    if(is_supported_feature(device, "PREEMPTION_MAC_802_3BR"))then

        if(params.countOnly==nil) then
            print("Queues marked preemptive :")
        end

        for i=0,7 do
            tcDpRemapping = {
                tc = i,
                dp = "CPSS_DP_GREEN_E",
                isStack = "CPSS_DXCH_PORT_PROFILE_NETWORK_E",
                dsaTagCmd = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
                targetPortTcProfile = "CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E",
                packetIsMultiDestination = false
             }

            res,val = myGenWrapper("cpssDxChCscdQosTcDpRemapTableGet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
                {"OUT","GT_U32","newTcPtr"},
                {"OUT","CPSS_DP_LEVEL_ENT","newDpPtr"},
                {"OUT","GT_U32","newPfcTPtrc"},
                {"OUT","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriorityPtr"},
                {"OUT","CPSS_DP_FOR_RX_ENT","dpForRxPtr"},
                {"OUT","GT_BOOL","preemptiveTcPtr"},
             })

             if(res~=0)then
                print("Error "..res.." calling cpssDxChCscdQosTcDpRemapTableGet")
                return
             end

             if(params.countOnly==nil) then
                print("Queue "..i.." preemptive "..tostring(val.preemptiveTcPtr))
             else
                if(params.sync==nil) then
                    if(val.preemptiveTcPtr==true) then
                     return true
                    end
                else
                 if(val.preemptiveTcPtr==true) then
                    if(has_value(preemptive_queues,i) == false)then
                     preemptive_queues[i+1]=1
                     print("insert tc "..i)
                    end
                 else
                    if(has_value(preemptive_queues,i) == true)then
                     preemptive_queues[i+1]=0
                     print("remove  tc"..i)
                    end
                 end
                 --print("ASIC to LUA "..i.." "..tostring(val.preemptiveTcPtr))
                end
             end
        end  --for i=0,7 do
    else
        print("Device "..device ..  " is not supported since it is not SIP 6_10.")
    end
    return false
end




local function preemption_set_port_state(params)

    local device = tonumber(params["devID"])
    local port =   tonumber(params["portNum"])
    local pm =1 --always try with pm
    local sepCount=0
    local val,res,portParamVal
    local event = {}
    local profile="CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E"
    local fragSize,preemptiveQExist
    local profileDisplayParam={}
    if params.enable==false then
     profile="CPSS_DXCH_PORT_PROFILE_NETWORK_E"
    end

    if params.pm and params.pm==true then
      pm=1
    end
    -- set default
    if params.fragSize==nil then
      fragSize="CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E"
    else
      fragSize = params.fragSize
    end


    if params.sepcount and params.sepcount==true then
      sepCount=1
      print("separated count mode")
    else
     print("aggregated count mode")
    end

    if pm==1 then
     --check if PM is activated
     res,val = myGenWrapper("prvWrAppDbEntryGet",{
            {"IN","string","namePtr","portMgr"},
            {"OUT","GT_U32","valuePtr"}
        })
     if(res~=0)then
        print("Error "..res.." calling prvWrAppDbEntryGet")
        return
     end


     if val and val.valuePtr ~= 0 then
        print("Pm mode requested for applying preemption")
     else
        print("Pm mode requested but PM is not active so switch to direct mode")
        pm =0
     end
    else
     print("Direct mode requested for applying preemption")
    end


    if(is_supported_feature(device, "PREEMPTION_MAC_802_3BR"))then
        print("Check if preemption supported for device "..device ..  " port "..port)

        res, val = myGenWrapper("prvCpssFalconTxqUtilsPortPreemptionAllowedGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  port},
                { "OUT", "GT_U32", "preemptionAllowedPtr"},
           })
       if(res~=0)then
        print("Error "..res.." calling prvCpssFalconTxqUtilsPortPreemptionAllowedGet")
        return
       end

       profileDisplayParam.devID = device
       profileDisplayParam.countOnly=true
       preemptiveQExist = preemption_profile_show_queue(profileDisplayParam)

       print("preemptive queue exist "..tostring(preemptiveQExist))

       if preemptiveQExist==false  then
        -- preemptive_queues is empty
        print("Creating CPSS_DXCH_PORT_PROFILE_NETWORK_E profile with TC0 as preemptive[default] ")
        preemptive_queues[1]=1
       end -- end of if rawequal(next(preemptive_queues), nil) then

       hwSync()

       if(val["preemptionAllowedPtr"]~=0)then
        print("Preemption is allowed for device "..device ..  " port "..port)
        if pm==0 then

            res, val = myGenWrapper("prvCpssDxChTxqSip6_10PreemptionEnableSet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  port},
                { "IN", "GT_BOOL" , "enable",  params.enable},
           })
            if(res~=0)then
                print("Error "..res.." calling prvCpssDxChTxqSip6_10PreemptionEnableSet")
                return
            end
        else
            res, portParamVal = myGenWrapper("cpssDxChPortManagerPortParamsGet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port },
                    { "OUT", "CPSS_PM_PORT_PARAMS_STC", "portParams" }
            })
            if res~=0 then
                print("Error at command: cpssDxChPortManagerPortParamsGet "..tostring(res))
            end
            if params.enable == true then
             if sepCount==0 then
             portParamVal.portParams.portParamsType.regPort.portAttributes.preemptionParams.type = "CPSS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E"
             else
             portParamVal.portParams.portParamsType.regPort.portAttributes.preemptionParams.type = "CPSS_PM_MAC_PREEMPTION_ENABLED_SEPARATED_E"
             end
            else
             portParamVal.portParams.portParamsType.regPort.portAttributes.preemptionParams.type = "CPSS_PM_MAC_PREEMPTION_DISABLED_E"
            end

            portParamVal.portParams.portParamsType.regPort.portAttributes.preemptionParams.minFragSize = fragSize

            portParamVal.portParams.portParamsType.regPort.portAttributes.validAttrsBitMask =
    set_one_bit(portParamVal.portParams.portParamsType.regPort.portAttributes.validAttrsBitMask,12) --CPSS_PM_PORT_ATTR_PREEMPTION_E

            if params.hpMode and params.hpMode==true then
                portParamVal.portParams.portParamsType.regPort.portAttributes.preemptionParams.preemptionMethod = "CPSS_PM_MAC_PREEMPTION_METHOD_HIGH_PRIORITY_E"
                print("mode = CPSS_PM_MAC_PREEMPTION_METHOD_HIGH_PRIORITY_E")
            else
                portParamVal.portParams.portParamsType.regPort.portAttributes.preemptionParams.preemptionMethod = "CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E"
                print("mode = CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E")
            end

            print(string.format("New valid bit mask 0x%x", portParamVal.portParams.portParamsType.regPort.portAttributes.validAttrsBitMask))

            print("Frag "..fragSize)
            --check port state
            res, val = myGenWrapper("cpssDxChPortManagerStatusGet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port },
                    { "OUT", "CPSS_PORT_MANAGER_STATUS_STC", "portStagePtr" }
            })
            if res~=0 then
                print("Error at command: cpssDxChPortManagerStatusGet "..tostring(res))
            end

            if val["portStagePtr"]["portState"] ~="CPSS_PORT_MANAGER_STATE_RESET_E" then
                print("Send CPSS_PORT_MANAGER_EVENT_DELETE_E")
                event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
                res, val = myGenWrapper("cpssDxChPortManagerEventSet",
                                  {{ "IN", "GT_U8", "devNum", devNum},
                                   { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                                   { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

                if res~=0 then
                    print("Error at command: cpssDxChPortManagerEventSet(CPSS_PORT_MANAGER_EVENT_DELETE_E) "..tostring(res))
                end
            end

            res, val = myGenWrapper("cpssDxChPortManagerPortParamsSet", {
                    { "IN", "GT_U8", "devNum", devNum},
                    { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port },
                    { "IN", "CPSS_PM_PORT_PARAMS_STC", "portParams",portParamVal.portParams }
            })
            if res~=0 then
                print("Error at command: cpssDxChPortManagerPortParamsSet "..tostring(res))
            end

            print("Send CPSS_PORT_MANAGER_EVENT_CREATE_E")
            event.portEvent =  "CPSS_PORT_MANAGER_EVENT_CREATE_E"
            res, val = myGenWrapper("cpssDxChPortManagerEventSet",
                              {{ "IN", "GT_U8", "devNum", devNum},
                               { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", port},
                               { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

            if res~=0 then
                print("Error at command: cpssDxChPortManagerEventSet(CPSS_PORT_MANAGER_EVENT_CREATE_E) "..tostring(res))
            end

        end
        --add to profile
        res, val = myGenWrapper("cpssDxChCscdPortTcProfiletSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","CPSS_PORT_DIRECTION_ENT","portDirection","CPSS_PORT_DIRECTION_TX_E"},
                {"IN","CPSS_DXCH_PORT_PROFILE_ENT","portProfile",profile},
            })
         if(res~=0)then
            print("Error "..res.." calling cpssDxChCscdPortTcProfiletSet")
            return
         end

        print("Preemption mode set to "..tostring(params.enable).." for device "..device .." port "..port)
       else
       print("Preemption is not allowed for device "..device ..  " port "..port)
       end

    else
        print("Device "..device ..  " is not supported since it is not SIP 6_10.")
    end
end


local function preemption_display_status(params)

local preemptionAllowed,val2,brCntrl,brStatus

if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if(is_supported_feature(devices[j], "PREEMPTION_MAC_802_3BR"))then
         ports=luaCLI_getDevInfo(devices[j])[devices[j]]
         for i=1,#ports do
                res, val = myGenWrapper("prvCpssFalconTxqUtilsPortPreemptionAllowedGet", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  ports[i]},
                { "OUT", "GT_U32", "preemptionAllowedPtr"},
           })
           if(res~=0)then
            print("Error "..res.." calling prvCpssFalconTxqUtilsPortPreemptionAllowedGet")
            return
           end

           preemptionAllowed = val["preemptionAllowedPtr"]

           res, val = myGenWrapper("prvCpssSip6TxqUtilsPreemptionStatusGet", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  ports[i]},
                { "OUT", "GT_U32", "preemptionEnabledPtr"},
                { "OUT", "GT_U32", "actAsPreemptiveChannelPtr"},
                { "OUT", "GT_U32", "preemptivePhysicalPortNumPtr"},
           })

           if(res~=0)then
            print("Error "..res.." calling prvCpssSip6TxqUtilsPreemptionStatusGet")
            return
           end


          if(preemptionAllowed~=0)then
           if(val.preemptionEnabledPtr~=0)then

             res,brStatus,brCntrl = getBr(devices[j],ports[i])

            if(res~=0)then
             print("Error "..res.." calling getBr")
             return
            end

             print(string.format("Device:%5d  Port:%5d Preemption capable:%1d enable:%1d preemptive port used %d ,br ctrl 0x%x,br stat 0x%x",devices[j],ports[i],preemptionAllowed,val.preemptionEnabledPtr,
             val.preemptivePhysicalPortNumPtr,brCntrl,brStatus))
            else
             print(string.format("Device:%5d  Port:%5d Preemption capable:%1d enable:%1d ",devices[j],ports[i],preemptionAllowed,val.preemptionEnabledPtr))
            end
          else
           print(string.format("Device:%5d  Port:%5d Preemption capable:%1d act as preemptive:%1d ",devices[j],ports[i],preemptionAllowed,val.actAsPreemptiveChannelPtr))
          end
         end
        else
         print("Device "..devices[j] ..  " is not supported since it is not SIP 6_10.")
        end
    end


end

local function preemption_show_br(params)
  local device = tonumber(params["devID"])
  local port =   tonumber(params["portNum"])
  local brCntrl,brStatus,res
  if(is_supported_feature(device, "PREEMPTION_MAC_802_3BR"))then
    res, val = myGenWrapper("prvCpssFalconTxqUtilsPortPreemptionAllowedGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  port},
                { "OUT", "GT_U32", "preemptionAllowedPtr"},
           })
    if(res~=0)then
      print("Error "..res.." calling prvCpssFalconTxqUtilsPortPreemptionAllowedGet")
      return
    end
    preemptionAllowed = val["preemptionAllowedPtr"]
    if(preemptionAllowed==0)then
     print("Port "..port.." does not support preemption")
    else
     res,brStatus,brCntrl = getBr(device,port)

     if(res~=0)then
      print("Error "..res.." calling getBr")
      return
     end
     print()
     print(string.format("Raw format br ctrl 0x%x,br stat 0x%x",brCntrl,brStatus))
     print()
     dumpBrStatus(brStatus)
     print()
     dumpBrControl(brCntrl)
    end
  end
end

CLI_addHelp("debug", "txq-sip6-preemption", "Dump SIP6 txq preemption information(SIP6_10 only)")
--------------------------------------------------------------------------------
---- command registration: txq-sip6-preemption set-port-state
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-preemption set-port-state", {
   func = preemption_set_port_state,
   help = "Enable/disable preemption on port",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
        { format="enable %bool", name="enable",help="enable/disable preemption"},
        { format="pm %bool", name="pm",help="whether to use port manger or direct configuration"},
        { format="separated-count %bool", name="sepcount",help="whether counting mode is separated or aggregated"},
        { format="min-frag-size  %minFragSize", name="fragSize",help="minimal fragment size for preemption"},
        { format="hpMode  %bool", name="hpMode",help="high priority mode"},
        mandatory={"devID","portNum","enable"},
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: txq-sip6-preemption display-status
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-preemption display-status", {
   func = preemption_display_status,
   help = "Display device preemption status and capabilities",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
          mandatory={"devID"},
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: txq-sip6-preemption configure-queue
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-preemption configure-queue", {
   func = preemption_profile_configure_queue,
   help = "Handle CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E profile",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="queue %queueNum",name="queueNum",help="The queue number" },
        { format="preemptive %bool", name="enable",help="mark as preemptive true/false"},
        mandatory={"devID","queueNum","enable"},
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: txq-sip6-preemption show-queue
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-preemption show-queue", {
   func = preemption_profile_show_queue,
   help = "Show CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E profile",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        mandatory={"devID"},
        }
    }
})

--------------------------------------------------------------------------------
---- command registration: txq-sip6-preemption show-br
--------------------------------------------------------------------------------
CLI_addCommand("debug", "txq-sip6-preemption show-br", {
   func = preemption_show_br,
   help = "Show port BR configurations and state",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
        mandatory={"devID","portNum"},
        }
    }
})

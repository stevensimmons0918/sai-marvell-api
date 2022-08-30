--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq-sip6-enqueue-stat.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for enqueue counters
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--dofile("dxCh/debug/txq-sip6.lua")


local function cnc_statistic_deinit(params)
local passDropInit=false

if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
      res, val = myGenWrapper("prvCpssFalconQueueStatisticDeInit", {
                { "IN", "GT_U8"  , "devNum", devices[j]}
           })
      if(res~=0)then
          print("Error "..res.." calling prvCpssFalconQueueStatisticDeInit")
          return
      end

      res, val = myGenWrapper("prvCpssTxqSip6DropReasonDeInit", {
                { "IN", "GT_U8"  , "devNum", devices[j]}
           })
      if(res~=0)then
          print("Error "..res.." calling prvCpssTxqSip6DropReasonDeInit")
          return
      end

    end

end


local function cnc_statistic_init(params)

local tdCmd ,tdCode

if params["cmd"]  == nill then
 tdCmd = "CPSS_PACKET_CMD_DROP_HARD_E"
else
 tdCmd = params["cmd"]
end

if params["dropCode"]  == nill then
 tdCode = "CPSS_NET_FIRST_USER_DEFINED_E"
else
 tdCode = params["dropCode"]
end

print("Tail drop command is "..tdCmd)
print("Tail drop code is "..tdCode)

if params.flagNo==true then
 passDropInit=false
 return cnc_statistic_deinit(params)
end


if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
      res, val = myGenWrapper("prvCpssFalconQueueStatisticInit", {
                { "IN", "GT_U8"  , "devNum", devices[j]}
           })
      if(res~=0)then
          print("Error "..res.." calling prvCpssFalconQueueStatisticInit")
          return
      end

      res, val = myGenWrapper("prvCpssTxqSip6DropReasonInit", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "CPSS_PACKET_CMD_ENT"  , "cmd", tdCmd},
                { "IN", "CPSS_NET_RX_CPU_CODE_ENT", "tailDropCode", tdCode}
           })
      if(res~=0)then
          print("Error "..res.." calling prvCpssTxqSip6DropReasonInit")
          return
      end
      passDropInit=true
    end


end

local function cnc_statistic_pass_drop_show(params)

local port =   tonumber(params["portNum"])
local queue =  tonumber(params["queueNum"])
local preemptive ,preemptionAllowed
if (params["preemptive"]==null)then
 preemptive = false
else
 preemptive = params["preemptive"]
end

 if(passDropInit==false)then
  print("Initialize statistic unit first(dbg txq-sip6-enqueue-stat device 0)")
  return
 end

 if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
    if(preemptive==true)then
      if(is_supported_feature(devices[j], "PREEMPTION_MAC_802_3BR"))then
        res, val = myGenWrapper("prvCpssFalconTxqUtilsPortPreemptionAllowedGet", {
                    { "IN", "GT_U8"  , "devNum", devices[j]},
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
         print("Force change preemptive to false")
         preemptive = false
        end
      else
        print("Device "..devices[j].." does not support preemption")
        print("Force change preemptive to false")
        preemptive = false
      end
    end
        res, val = myGenWrapper("prvCpssSip6TxqDebugCncCounterGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_U32","queue",queue},
                {"IN","GT_BOOL","preemptiveMode",preemptive},
                {"OUT","GT_U32","passPcktsPtr"},
                {"OUT","GT_U32","droppedPcktsPtr"},
                {"OUT","GT_U32","passPcktsMsbPtr"},
                {"OUT","GT_U32","droppedPcktsMsbPtr"}
           })

        if(res~=0)then
          print("Error "..res.." calling prvCpssSip6TxqDebugCncCounterGet")
          return
        end

       print(" CNC Pass packets")
       print(" ================")
       print("(MSB) "..val.passPcktsMsbPtr)
       print("(LSB) "..val.passPcktsPtr)
       print("")
       print(" CNC Drop packets")
       print(" ================")
       print("(MSB) "..val.droppedPcktsMsbPtr)
       print("(LSB) "..val.droppedPcktsPtr)
    end

end


local function cnc_statistic_drop_reason_show(params)

local port =   tonumber(params["portNum"])

local names = {}

names[0] = "Global               "
names[1] = "PDX burst fifo global"
names[2] = "Pool                 "
names[3] = "Multicast            "
names[4] = "PDX burst fifo port  "
names[5] = "Q max  limit         "
names[6] = "Port/Q               "

 if(passDropInit==false)then
  print("Initialize statistic unit first(dbg txq-sip6-enqueue-stat device 0)")
  return
 end

 if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
          res, val = myGenWrapper("cpssDxChPortTxTailDropCommandGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"OUT","CPSS_PACKET_CMD_ENT","cmdPtr"},
                {"OUT","CPSS_NET_RX_CPU_CODE_ENT","tailDropCodePtr"}
           })

        if(res~=0)then
          print("Error "..res.." calling cpssDxChPortTxTailDropCommandGet")
         return
        end

        print("Tail drop command is "..val["cmdPtr"])
        print("Tail drop code is "..val["tailDropCodePtr"].."\n")

        for i=0,6 do
         res, val = myGenWrapper("prvCpssTxqSip6DropReasonCountGet",{
                {"IN","GT_U8","devNum",devices[j]},
                {"IN","GT_U32","reason",i},
                {"OUT","GT_U32","packetCntLsbPtr"},
                {"OUT","GT_U32","packetCntMsbPtr"},
                {"OUT","GT_U32","counterPtr"}
           })

         if(res~=0)then
          print("Error "..res.." calling prvCpssSip6TxqDebugCncCounterGet")
          return
         end
         print(names[i].." MSB "..val.packetCntMsbPtr.." LSB "..val.packetCntLsbPtr.."         CNC counter index "..val.counterPtr)
        end
    end

end


CLI_addHelp("debug", "txq-sip6-enqueue-stat", "Debug enqueue statistic")
CLI_addHelp("debug", "txq-sip6-show", "Show pass/drop counters")
--------------------------------------------------------------------------------
---- command registration: txq-sip6-enqueue-stat pass-drop-configure
--------------------------------------------------------------------------------



CLI_addCommand("debug", "txq-sip6-enqueue-stat", {
   func = cnc_statistic_init,
   help = "Configure pass drop counting",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
        { format="cmd %packetCommand",name="cmd",help="Tail drop command"},
        { format="dropCode %tdCode",name="dropCode",help="Tail drop code" },
        mandatory={"devID"},
        }
    }
})
CLI_addCommand("debug", "no txq-sip6-enqueue-stat ", {
   func = function(params)
    params.flagNo = true
    return cnc_statistic_init(params)
    end,
  help = "De-Configure pass drop counting",
  params={
     { type="named",
      { format = "device %devID_all", name = "devID", help = "Device number"}
     }
    }
})

CLI_addCommand("debug", "txq-sip6-show  pass-drop", {
   func = cnc_statistic_pass_drop_show,
   help = "Show pass drop counters per port and queue",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
        { format="queue %queueNum",name="queueNum",help="The queue offset" },
        { format="preemptive %bool", name="preemptive",help="preemption channel statistic"},
          mandatory={"devID","portNum","queueNum"},
        }
    }
})

CLI_addCommand("debug", "txq-sip6-show  drop-reason", {
   func = cnc_statistic_drop_reason_show,
   help = "Show tail drop reason",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
          mandatory={"devID"},
        }
    }
})


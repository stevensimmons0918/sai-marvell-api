--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* dce_qcn_congestion_management.lua
--*
--* DESCRIPTION:
--*       enabling of priority flow control on interface/range
--*
--* FILE REVISION NUMBER:
--*       $Revision: 6 $
--*
--********************************************************************************

--includes

--constants

--
-- CPSS DxCh congestion notification
--
--
--


-- default QCN configuration parameters (should be tuned)
local DefaultScaleFactor = 512
local DefaultCnProfileConfigthreshold = 10
local DefaultCnProfileSet = 0
local OriginalPacketLenagh = 0
local DefaultAlpha = "CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E"


-- ************************************************************************
---
--  show_dce_qcn
--        @description  Displays dce qcn on the device
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true
--
local function show_dce_qcn(params)
    local ii, status, err
    local devNum, portNum
    local arr 
    
    print("")
    print("DCE QCN summary")  
    print("---------------")
    
    
    devNum=params.devID
    status = true
    arr = {}
    ii = 0
    
--  get global QCN state
    local result, values = cpssPerDeviceParamGet("cpssDxChPortCnModeEnableGet",
            devNum, "qcn_enable", "CPSS_DXCH_PORT_CN_MODE_ENT")
--  retive results from output           
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
        
  
    if (values.qcn_enable == true) then
         print(string.format("%-11s%-22s","QCN is globally ENABLED on device ",tostring (devNum)))      
    else 
        print(string.format("%-11s%-22s","QCN is globally DISABLED on device ",tostring (devNum)))
    end

    -- get CN messages ethertype
    result, values = cpssPerDeviceParamGet("cpssDxChPortCnEtherTypeGet", 
            devNum, "ethertype")
                      
  -- retive results from output           
    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
    
    
    print(string.format("%-11s%-22x","Ethertype for CN messages is 0x",values.ethertype))      
    
    -- get CN messages prioirity queue
    result, values = cpssPerDeviceParamGet(
        "cpssDxChPortCnFrameQueueGet", devNum, "tcQueue")
          

    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end
    
   
    print(string.format("%-11s%-22s","CNM (congestion notification message) Priority Queue is ",tostring(values.tcQueue)))      
    
    
-- orginize header     
    print("")
    print("Priority   QCN")  
    print("--------   ---")
-- loop on entire priorities in the device  
    local profileIndex = DefaultCnProfileSet  
    while ii < NumberOf_PRIO do
    
            result, values = myGenWrapper(
              "cpssDxChPortCnProfileQueueConfigGet", {
                    { "IN", "GT_U8" ,"dev", devNum},                         
                { "IN", "GT_U32", "profileIndex", profileIndex},
                { "IN", "GT_U8", "tcQueue", ii},
                { "OUT", "CPSS_PORT_CN_PROFILE_CONFIG_STC", "cnProfileCfgPtr"},
              }  
          )          
          
        if (result ~=0) then
          status = false
          err = returnCodes[result]
                    return status, err                        
        end
-- print(string.format("%s %q", "dce qcn: get_global_dce_qcn dev, aware ", tostring(type(values.cnProfileCfgPtr.cnAware))))
-- print(string.format("%s %q", "dce qcn: get_global_dce_qcn dev, threshold ", tostring(values.cnProfileCfgPtr.threshold)))
-- probably it seems as not nessary now, but i prefer to store info into arr[], and then print      
       if (values.cnProfileCfgPtr.cnAware == true) then 
           arr[ii] = "enable" 
         else
        arr[ii] = "disable" 
    end 
      
      -- prepare the next iteration
      ii = ii + 1        
      
  end
  
--  print the data from arr[], in loop on entire priorities
  ii = 0
  while ii < NumberOf_PRIO do
    if type(arr[ii]) ~= "nil" then
        print(string.format("%-11s%-22s", tostring(ii), arr[ii]))
    end
    ii = ii + 1
  end
  
-- clear array
  ii = 0
  arr = nil
  arr = {}
--  loop on ports in device
  while ii < NumberOf_PORTS_IN_DEVICE do
        
        -- check if specific port does exist in range
        if does_port_exist(dev,ii) then
            
          result, values = cpssPerPortParamGet(
              "cpssDxChPortCnFcEnableGet",
                            dev, port, "enable","GT_BOOL")
                            
          if (result ~=0) then
          status = false
          err = returnCodes[result]              
          end
          --probably it seems as not nessary now
          --but it's preferable to store info into arr_port[], and then print      
           if values.enable == true then 
             arr[ii] = "enable" 
           else
          arr[ii] = "disable" 
        end 
      end

      
      -- prepare the next iteration
      ii = ii + 1        
      
  end
-- orginize header     
    print("")
    print("Interface   QCN")  
    print("---------   ---")
  
--  print the data from arr[], in loop on entire device ports
  ii = 0
  while ii<NumberOf_PORTS_IN_DEVICE do
    if type(arr[ii]) ~= "nil" then
        print(string.format("%s%s%s %-22s",tostring(devNum),"/",tostring(ii),arr[ii]))
    end
    ii = ii + 1
  end      
               
      
return status, err    
  
end


-- ************************************************************************
---
--  set_global_dce_qcn
--        @description  set dce congestion notification on the device e 
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true
--
local function set_global_dce_qcn(params)
    local ii, status, err
    local devNum, portNum
    local qcn_enable
    local entry = {}
    
    status = true
    
    if params.flagNo == nil then
      qcn_enable = true
    else
      qcn_enable = false
    end

-- device number from parameters (SW device_id)
    devNum=params.devID

--  set global QCN state
    local result, values = cpssPerDeviceParamSet("cpssDxChPortCnModeEnableSet", devNum,
            qcn_enable, "qcn_enable", "GT_BOOL")
    if (result ~=0) then
      status = false
      err = returnCodes[result]
    end

    if (true == qcn_enable) then
        -- configure entry

        entry = {qosProfileId = 0,
                 isRouted = 0,
                 overrideUp = false,
                 cnmUP = 0,
                 defaultVlanId = 1,
                 scaleFactor = DefaultScaleFactor,
                 version = 1,
                 cpidMsb = {l={[0] = 0,[1] = 0}}, -- "GT_U64 union" entry configuration
                 cnUntaggedEnable = true,
                 forceCnTag = false,
                 flowIdTag = 0,
                 appendPacket = false,
                 keepQcnSrcInfo = false
                 }
   
      result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnMessageGenerationConfigSet", devNum,
                        entry, "cnmGenerationCfgPtr", "CPSS_PORT_CNM_GENERATION_CONFIG_STC")

        -- set CN packets length (currently to original)
    result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnPacketLengthSet", devNum,
                        OriginalPacketLenagh)

      if (result ~=0) then
        status = false
        err = returnCodes[result]
      end
     end
-- need to configure the SHAPER ????

return status, err

end


-- ************************************************************************
---
--  set_prio_dce_qcn
--        @description  set dce qcn on the devicee 
--
--        @param params         - params["devID"]: the device ID
--
--        @return       true 
--
local function set_prio_dce_qcn(params)
    local ii, status, err
    local devNum
    local txQueue, AwareCn, queueID, startID, tcQueue
    local cfgThreshold, cfgAlpha, profileSet
    local entry = {}
    
    status = true
       
-- get device number and tcQueue from parameters (SW device_id)    
    devNum = params["devID"]
    txQueue = params["queue"]
    profileSet = DefaultCnProfileSet

    if txQueue == "all" then
       startID = 0
       endID = 7
    else
       startID = tcQueue
       endID = tcQueue
    end

--  set tcQueue
    if (params.flagNo == nil) then
       AwareCn = true
    else
       AwareCn = false
    end

    if (params["packet-limit"] == nil) then
       cfgThreshold = DefaultCnProfileConfigthreshold
    else
       cfgThreshold = tonumber(params["packet-limit"])
    end

    if (params["alpha"] == nil) then
       cfgAlpha = DefaultAlpha
    else
       cfgAlpha = params["alpha"]
    end

    entry = {cnAware = AwareCn, threshold = cfgThreshold, alpha = cfgAlpha}

    for tcQueue=startID, endID do
--  set threshoulds per tc into enable profile state
      local result, values = myGenWrapper(
                "cpssDxChPortCnProfileQueueConfigSet", {
                  { "IN", "GT_U8" ,"dev", devNum},
                  { "IN", "GT_U32", "profileSet", profileSet},
                  { "IN", "GT_U8", "tcQueue", tcQueue},
                  { "IN", "CPSS_PORT_CN_PROFILE_CONFIG_STC", "cnProfileCfgPtr", entry},
            }
      )

      if (result ~=0) then
        status = false
        err = returnCodes[result] 
      end
    end

return status, err
end


-- ************************************************************************
---
--  set_ethertype_dce_qcn
--        @description  set dce qcn ethrtype (ethertype) dec (devID 
--
--        @param params         - params["devID"]: the device ID
--
--        @return       true 
--
local function set_ethertype_dce_qcn(params)
    local ii, status, err
    local devNum
    local ethertype

    status = true

-- get device number and ethrtype from parameters
    devNum    = params.devID
    if (params.flagNo == nil) then
        ethertype =  tonumber(params.ethertype) -- convert of string to number with base 16
    else
        ethertype = 0x5555 --default value
    end

    devNum    = params.devID
-- call cpss configuration function
    local result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnEtherTypeSet",
                        devNum, ethertype)
    if (result ~=0) then
        status = false
        err = returnCodes[result]
    end

return status, err
end

-- ************************************************************************
---
--  set_prio_dce_qcn_cnm
--        @description  set dce priority queueu of the CN messages on the 
--                      device 
--
--        @param params         - params["devID"]: the device ID
--
local function set_prio_dce_qcn_cnm(params)
    local ii, status, err
    local devNum
    local tcQueue
     
    status = true
       
-- get device number and tcQueue from parameters (SW device_id)    
    devNum     = params.devID
    tcQueue    = params.prio
         
--  set tcQueue 
    if (params.flagNo == nil) then
       tcQueue    = params.prio
    else
       tcQueue = 0
    end      
             
--  set tc for CN messages
    local result, values = cpssPerDeviceParamSet(
              "cpssDxChPortCnFrameQueueSet",
                        devNum,
                        tcQueue, "tcQueue", "GT_U8")

    if (result ~=0) then
      status = false
      err = returnCodes[result]              
    end

      
return status, err    
  
end


-- ************************************************************************
---
--  set_interface_dce_qcn
--        @description  set dce qcn on the interfaces  
--
--        @param params         - params["devID"]: the device ID
--
local function set_interface_dce_qcn(params)
    local ii, status, err
    local devNum, portNum
    local enable, result, values
    
    status = true
    --check "intrface type" of this command. should be ethrnet or range-ethernet
    --is it working on the right interface type (not vlan or anything)
  if (tostring(getGlobal("ifType")) ~= "ethernet") then
    return false, "Can only be used on an ethernet type interface or ethernet range"
  end
  
  -- get entire interfaces range
    local all_ports = getGlobal("ifRange")
    
         
  -- get device number and tcQueue from parameters (SW device_id)    
    devNum=params.devID
 
        
  --  set profile accordingly negative and positive cases
    if (params.flagNo == nil) then
       enable = true 
    else
       enable = false
    end      

  --loop on all prots in range
    --loop on entire ports in "ethernet" or "ethernet range" and add them to the port channel  
  for dev, dev_ports in pairs(all_ports) do
    for key, port in pairs(dev_ports) do

                      
      -- set threshoulds per tc into enable profile state
      -- only in case that port exists in choosed range
      if does_port_exist(dev,port) then
      
          result, values = cpssPerPortParamSet(
                    "cpssDxChPortCnFcEnableSet",
                            dev, port,
                            enable, "enable", "GT_BOOL")
          
        if (result ~=0) then
          status = false
          err = returnCodes[result]          
        end  
        
        result, values = cpssPerDeviceParamSet(
                    "cpssDxChPortCnTerminationEnableSet",
                            dev, port,
                            enable, "enable", "GT_BOOL")
          
          
          if (result ~=0) then
          status = false
          err = returnCodes[result]
        end 
        end
        end
     end

return status, err

end

-- ************************************************************************
---
--  set_global_dce_dba_qcn
--        @description  set dce congestion notification on the device
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true
--
local function set_global_dce_dba_qcn(params)
    local ii, status, err
    local devNum, buffer
    local qcn_dba_enable

    status = true

-- device number from parameters (SW device_id)
    devNum=params["devID"]

-- read available buffer
    ret,val = myGenWrapper("cpssDxChPortCnDbaAvailableBuffGet",{
      {"IN","GT_U8","devNum",devNum},
      {"OUT","GT_U32","availableBuffPtr"}
    })
    if params.flagNo == nil then
      qcn_dba_enable = true
      if nil~=params["buffer-limit"] then
        buffer =  tonumber((params["buffer-limit"]),16) -- convert of string to number with base 16
      else
        buffer = val["availableBuffPtr"]
      end
    else
      qcn_dba_enable = false
      buffer = val["availableBuffPtr"]
    end

--  set global QCN DBA state
    local result, values = cpssPerDeviceParamSet("cpssDxChPortCnDbaModeEnableSet", devNum,
            qcn_dba_enable, "qcn_dba_enable", "GT_BOOL")
    if (result ~=0) then
      status = false
      err = returnCodes[result]
    end

    local result, values = myGenWrapper(
              "cpssDxChPortCnDbaAvailableBuffSet", {
                    { "IN", "GT_U8" ,"dev", devNum},
                { "IN", "GT_U32", "availableBuff", buffer}
              }
          )

      if (result ~=0) then
        status = false
        err = returnCodes[result]
      end

return status, err
end

-- ************************************************************************
---
--  set_global_dce_qcn_pool_buff
--        @description  set dce qcn pool buffer allocation on the device
--
--        @param params         - params["devID"]: The device ID
--        @param params         - params["poolID"]: The pool ID
--        @param params         - params[""buffer-limit""]: The pool buffer limit
--
--        @return       true
--
local function set_global_dce_qcn_pool_buff(params)
    local ii, status, err
    local devNum, buffer
    local poolNum

    poolNum = 0
    status = true

-- device number from parameters (SW device_id)
   devNum=params["devID"]

-- read pool available buffer
    ret,val = myGenWrapper("cpssDxChPortCnDbaPoolAvailableBuffGet",{
      {"IN","GT_U8","devNum",devNum},
      {"IN","GT_BOOL","poolNum",poolNum},
      {"OUT","GT_U32","poolAvailableBuffPtr"}
    })
    if params.flagNo == nil then
      poolNum = params["poolID"]
      if nil~=params["buffer-limit"] then
        buffer =  tonumber((params["buffer-limit"]),16) -- convert of string to number with base 16
      else
        buffer = val["poolAvailableBuffPtr"]
      end
    else
      poolNum = 0
      buffer = val["poolAvailableBuffPtr"]
    end

--  set global QCN DBA state
    local result, values = myGenWrapper(
              "cpssDxChPortCnDbaPoolAvailableBuffSet", {
                    { "IN", "GT_U8" ,"devNum", devNum},
                    {"IN","GT_BOOL","poolNum",poolNum},
                { "IN", "GT_U32", "poolAvailableBuffPtr", buffer}
              }
          )

      if (result ~=0) then
        status = false
        err = returnCodes[result]
      end

return status, err
end



-- ************************************************************************
---
--  set_global_dce_qcn_queue_status
--        @description  set dce congestiong queue status mode
--
--        @param params         - params["devID"]: The device ID
--
--        @return       true
--
local function set_global_dce_qcn_queue_status(params)
    local ii, status, err
    local devNum
    local q_status_enable
    local redirectPort, redirectDevNum

    status = true

 --device number from parameters (SW device_id)
    devNum=params["dev"]
    if params["redirect_ethernet"] ~= nil then
      redirectPort = params["redirect_ethernet"]["portNum"]
      redirectDevNum = params["redirect_ethernet"]["devId"]
    end 

    if params.flagNo == nil then
      q_status_enable = true
    else
      q_status_enable = false
    end

    -- configure entry
    local result, values = myGenWrapper(
              "cpssDxChPortCnQueueStatusModeEnableSet", {
                    { "IN", "GT_U8" ,"dev", devNum},
                    { "IN", "GT_BOOL" ,"enable", q_status_enable},
                    { "IN", "GT_U32" ,"targetHwDev", redirectDevNum},
                    { "IN", "GT_U32", "targetPort", redirectPort}
              }
          )

      if (result ~=0) then
        status = false
        err = returnCodes[result]
      end

return status, err
end

-- *config*
-- [no] dce qcn priority %prio enable %devID
CLI_addCommand("config", "dce qcn priority", {
    func=set_prio_dce_qcn,
    help="dce qcn priority (0-7) enable DEV_ID",
    params={
        {   type="named",
            { format="enable %devID", name="devID", help="The device id of system" },
            { format="queue %queue_id", name="queue", help="The traffic class (0..7) to configure cn profile" },
            { format="threshold %packet-limit", name="packet-limit", help="The threshold for priority (0-7)"},
            { format="alpha %tail_drop_alpha", name="alpha", help="The dynamic alpha for priority (0-7)"},
            requirements={
              ["queue"] = {"devID"},
              ["packet-limit"] = {"queue"},
              ["alpha"] = {"packet-limit"}
            },
            mandatory = {"devID", "queue"}
        }
    }
})

CLI_addCommand("config", "no dce qcn priority", {
    func = function(params)
               params.flagNo = false
               return set_prio_dce_qcn(params)
           end,
    help = "Returning to default cn profile",
    params={
        {   type="values",
            { format="%priority", name="prio", help="The priority (0-7)on which PFC is to be enabled" },
            "disable",
            "%devID"
        }
    }
})

CLI_type_dict["ethertype"] = {
    help="The ethertype of the CN messages , mandatory two bytes in hex (i.e. 5556, etc)"
}
-- *config*
-- [no] dce qcn ethertype %ethertype dev %devID
CLI_addCommand("config", "dce qcn ethertype", {
    func=set_ethertype_dce_qcn,
    help="dce qcn ethertype (for example :5556, in two bytes,hex) dev DEV_ID",
    params={
        {   type="values", "%ethertype"},
        {   type="named",
            { format= "device %devID", name="devID", help="The device number" }
        }
    }
})

CLI_addCommand("config", "no dce qcn ethertype", {
    func = function(params)
               params.flagNo = false
               return set_ethertype_dce_qcn(params)
           end,
    help = "Returning to default ether type",
    params={
        {   type="values",
            "dev",
            "%devID"
        }
    }
})

-- *config*
-- [no] dce qcn enable %devID
CLI_addCommand("config", "dce qcn enable", {
    func=set_global_dce_qcn,
    help="dce qcn enable dev DEV_ID",
    params={
        {   type="values",
            "dev",
            "%devID"
        }
    }
})

CLI_addCommand("config", "no dce qcn enable", {
    func = function(params)
               params.flagNo = false
               return set_global_dce_qcn(params)
           end,
    help = "Returning to default qcn state",
    params={
        {    type="values",
            "dev", help="Device id of the system",
            "%devID"
        }
    }
})

-- *config*
-- [no] dce qcn enable dba dev %devID [optional buffer %buff]
CLI_addCommand("config", "dce qcn enable dba", {
    func=set_global_dce_dba_qcn,
    help="dce qcn enable dba buffer BUFF dev DEV_ID",
    params = {
        { type = "named",
            { format = "device %devID", name = "devID", help = "The device id of the system"},
            { format="buffer-limit %packet-limit", name="buffer-limit", help="Maximal number of buffers for qcn dba" },
          requirements = {
            ["buffer-limit"] = { "devID"}
          }
       }
    }
})

CLI_addCommand("config", "no dce qcn enable dba", {
    func = function(params)
               params.flagNo = false
               return set_global_dce_dba_qcn(params)
           end,
    help = "Returning to default qcn dba state",
    params = {
        { type = "named",
            { format = "device %devID", name = "devID", help = "The device id of the system"},
        }
    }
})

-- *config*
-- [no] dce qcn enable dba dev %devID [optional buffer %buff]
CLI_addCommand("config", "dce qcn enable queue-status", {
    func=set_global_dce_qcn_queue_status,
    help="dce qcn enable queue status mode",
    params = {
        { type = "named",
            { format = "dev %devID", name = "dev", help = "The device id of the system"},
            { format = "redirect-ethernet %dev_port", name="redirect_ethernet",help="Redirect to output ethernet port"},
          requirements = {
            ["redirect_ethernet"] = { "dev"},
          },
          mandatory = { "dev", "redirect_ethernet"}
       }
    }
})

CLI_addCommand("config", "no dce qcn enable queue_status_mode", {
    func = function(params)
               params.flagNo = false
               return set_global_dce_qcn_queue_status(params)
           end,
    help = "Returning to default qcn queue status",
    params = {
          { type = "named",
            { format = "dev %devID", name = "dev", help = "The device id of the system"},
          }
    }
})

-- *config*
-- [no] dce qcn set dev %devID pool %poolID buffer %buff
CLI_addCommand("config", "dce qcn set", {
    func=set_global_dce_qcn_pool_buff,
    help="Set the QCN DBA buffer for the given pool id in the device",
    params = {
        { type = "named",
            { format = "device %devID", name = "devID", help = "The device id of the system"},
            { format = "pool %poolID", name = "poolID", help = "The pool id for QCN DBA"},
            { format="buffer-limit %packet-limit", name="buffer-limit", help="Maximal number of buffers for qcn dba" }
       }
    }
})

CLI_addCommand("config", "no dce qcn set", {
    func = function(params)
               params.flagNo = false
               return set_global_dce_qcn_pool_buff(params)
           end,
    help = "Returning to default qcn dba pool buffer allocation state",
    params = {
        { type = "named",
            { format = "device %devID", name = "devID", help = "The device id of the system"},
        }
    }
})



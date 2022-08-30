--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpssAPI.lua
--*
--* DESCRIPTION:
--* DX related code
--*
--* FILE REVISION NUMBER:
--*       $Revision: 26 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:
--
-- cpssAPI_desc        - Cpss API name and input parameters
--


--  ****************************************************************************
--
-- Start of <<direct call>> section

-- direct call of cpssDxChTrunkTableEntryGet____________________________________
  local function direct_call_cpssDxChTrunkTableEntryGet(params)
    local ret, val
    local devNum = params.devID
    local trunkId = params.trunkId

    ret, val = wrlCpssDxChTrunkDevicesAndPortsList(devNum, trunkId ,
                1 --[[get real order in format of CPSS_TRUNK_MEMBERS_AND_NUM_STC ]])

    if (ret ~= 0 or
        val == nil or
        val.numMembers == nil or
        val.members == nil)
    then
        if(ret ~= 0) then
            print("trunkId[".. tostring(trunkId).. "] got error " .. to_string(returnCodes[ret]))
        else
            print("trunkId[".. tostring(trunkId).. "] unknown error ")
        end

      -- did not get proper format of info
      return false
    end

    local numMembers = val.numMembers
    local members = val.members
    local NULL_Port = 62
    local is_NULL_Port = false

    if (numMembers == 0) then
        print("empty trunk (no ports)")
        return true
    end

    for index = 0, numMembers-1 do
        local member = members[index]
        if(member.port == NULL_Port) then
            is_NULL_Port = true
        end
    end

    if (numMembers == 1 and is_NULL_Port) then
        print("trunkId[".. tostring(trunkId).. "] is empty (only NULL port)")
        return true
    end

    print("trunkId[".. tostring(trunkId).. "] , numMembers= " .. tostring(numMembers))
    for index = 0, numMembers-1 do
        local member = members[index]
        print(string.format("[%d]={device=[%d],port=[%d]}",index,member.device,member.port))
    end

    return true
  end


  -- direct call of cpss api cpssDxChTrunkTableEntryGet
  CLI_addCommand("exec", "cpss-api call cpssDxChTrunkTableEntryGet",
  {
    help = "Direct call of function cpssDxChTrunkTableEntryGet",
    func = direct_call_cpssDxChTrunkTableEntryGet,
    params={{type= "named",
              {format="devNum %devID",              name="devID",       help="The device number"},
              {format="trunkId  %GT_U32",           name="trunkId",     help="Trunk ID"},
              requirements={ ["trunkId"]={"devID"}},
              mandatory = {"devID", "trunkId"}
           }}
  })

-- end of cpssDxChTrunkTableEntryGet()__________________________________________

-- direct call of cpssDxChSinglePortModeSpeedSet ____________________________________

    local function direct_call_cpssDxChSinglePortModeSpeedSet(params)
        local ret

      ret = wrlDxChPortModeSpeedSet(
          params.devID,
          params.port,
          params.powerUp,
          params.ifMode,
          params.speed)

      if (ret ~= 0) then
        printErr("cpssDxChSinglePortModeSpeedSet() failed: " .. returnCodes[ret])
        return false, "Error in cpssDxChSinglePortModeSpeedSet()"
      end
      return true
    end

    -- direct call of cpss api cpssDxChSinglePortModeSpeedSet
    CLI_addCommand("exec", "cpss-api call cpssDxChSinglePortModeSpeedSet",
    {
      help = "Direct call of function cpssDxChSinglePortModeSpeedSet",
      func = direct_call_cpssDxChSinglePortModeSpeedSet,
      params={{type= "named",
                {format="devNum %devID",                  name="devID",   help="The device number"},
                {format="portNum %GT_U32",                name="port",   help="The port number"},
                {format="powerUp  %bool",              name="powerUp", help="Serdes power up"},
                {format="ifMode  %port_interface_mode", name="ifMode",  help="Interface mode"},
                {format="speed  %port_speed",          name="speed",   help="Port speed"},
                requirements={ ["port"]={"devID"}, ["powerUp"]={"port"}, ["ifMode"]={"powerUp"}, ["speed"]={"ifMode"} },
                mandatory = {"devID", "port", "powerUp", "ifMode", "speed"}
             }}
    })

-- end of cpssDxChSinglePortModeSpeedSet()_________________________________________

-- direct call of cpssDxChHitlessStartupMiCompatibilityCheck ____________________________________

    local function direct_call_cpssDxChHitlessStartupMiCompatibilityCheck(params)
        local ret =0
        local versionInput =0
        local Compatible = "false"

        if (params.miVersion == nil) then
            versionInput = ""
        else
            versionInput = params.miVersion
        end
        ret,val = myGenWrapper("cpssDxChHitlessStartupMiCompatibilityCheck",{
            {"IN","GT_U8","devNum",params.devID},
            {"IN","GT_CHAR","miVersion",versionInput},
            {"OUT","GT_U32","value"} })
        if ret ~= 0 then
            printLog("cpssDxChHitlessStartupMiCompatibilityCheck failed : ".. returnCodes[ret])
            return false
        end

        if val["value"] == 1 then
            Compatible = "true"
            print("CPSS and Micro Init versions are compatible")
        else
            print("CPSS and Micro Init versions are NOT compatible")
        end

        print("isCompatible = "..Compatible.." \n")
        return true
    end

    -- direct call of cpss api cpssDxChHitlessStartupMiCompatibilityCheck
    CLI_addCommand("exec", "cpss-api call cpssDxChHitlessStartupMiCompatibilityCheck",
    {
      help = "Direct call of function cpssDxChHitlessStartupMiCompatibilityCheck",
      func = direct_call_cpssDxChHitlessStartupMiCompatibilityCheck,
      params={{type= "named",
                {format="devNum %GT_U8",                  name="devNum",   help="The device number"},
                {format="miVersion %GT_CHAR",             name="miVersion",   help="The micro init version"},
                requirements={ ["miVersion"]={"devNum"} },
                mandatory = {"devNum"}
             }}
    })

    -- end of cpssDxChHitlessStartupMiCompatibilityCheck()_________________________________________

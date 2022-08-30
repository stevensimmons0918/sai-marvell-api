--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* storm_control_rate_limit_enum.lua
--*
--* DESCRIPTION:
--*       enum for storm control rate limit mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes


--constants


--------------------------------------------
-- type registration: rateLimitMode
--------------------------------------------
CLI_type_dict["rateLimitMode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "Ingress rate limit mode: byte or packet",
    enum = {
        ["byte"] = { value="CPSS_RATE_LIMIT_BYTE_BASED_E", help="Rate limit based on byte count" },
        ["packet"] = { value="CPSS_RATE_LIMIT_PCKT_BASED_E", help="Rate limit based on packet count" }
   }
}


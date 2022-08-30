--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* cpu_mdc_frequency_mode.lua
--*
--* DESCRIPTION:
--*       cpu mdc clock frequency mode types
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

--
-- Types defined:eventType
--


CLI_type_dict["cpu_mdc_frequency_mode"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "CPU MDC frequency mode",
    enum = {
        ["normal"] = { value=0, help="Normal Mode - 1.5 MHz" },
        ["fast"] = { value=1, help="Fast Mode - 12.5 MHz" },
        ["accelerated"] = { value=2, help="Accelarated Mode - 25 Mhz" }
    }
}

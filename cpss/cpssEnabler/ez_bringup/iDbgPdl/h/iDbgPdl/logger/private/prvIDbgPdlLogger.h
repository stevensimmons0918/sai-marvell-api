/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */


#ifndef __prvLoggerApih

#define __prvLoggerApih

/**********************************************************************************
 * @file prvLoggerApi.h   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Debug lib Logger private definitions
 * @note Helper definitions for logger implementation
 * @version   1 
********************************************************************************
*/

#define PRV_IDBG_PDL_LOGGER_STRINGIFY_MAC(a)                     # a
#define PRV_IDBG_PDL_LOG_CONCAT_MAC(a, b)                        a ## b
#define PRV_IDBG_PDL_LOG_CONCAT2_MAC(a, b)                       PRV_IDBG_PDL_LOG_CONCAT_MAC(a,b)
#define PRV_IDBG_PDL_LOG_GET_NTH_ARGUMENT_MAC(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, ...) a31
#define PRV_IDBG_PDL_LOG_EXPAND_GET_NTH_ARGUMENT_MAC(args)       PRV_IDBG_PDL_LOG_GET_NTH_ARGUMENT_MAC args
#define PRV_IDBG_PDL_LOG_COUNT_ARGUMENTS_MAC(...)                PRV_IDBG_PDL_LOG_EXPAND_GET_NTH_ARGUMENT_MAC((__VA_ARGS__, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define PRV_IDBG_PDL_LOG_GLUE_MACRO_MAC(x, y)                    x y
#define PRV_IDBG_PDL_LOG_MACRO_OVERLOAD_LEVEL2_MAC(name, count)  name##count
#define PRV_IDBG_PDL_LOG_MACRO_OVERLOAD_LEVEL1_MAC(name, count)  PRV_IDBG_PDL_LOG_MACRO_OVERLOAD_LEVEL2_MAC(name, count)
#define PRV_IDBG_PDL_LOG_MACRO_OVERLOAD_MAC(name, count)         PRV_IDBG_PDL_LOG_MACRO_OVERLOAD_LEVEL1_MAC(name, count)
#define PRV_IDBG_PDL_LOG_CALL_MACRO_OVERLOAD_MAC(name, ...)  \
    PRV_IDBG_PDL_LOG_GLUE_MACRO_MAC( \
            PRV_IDBG_PDL_LOG_MACRO_OVERLOAD_MAC(name, PRV_IDBG_PDL_LOG_COUNT_ARGUMENTS_MAC(__VA_ARGS__)), \
            (__VA_ARGS__) \
    )

#define PRV_IDBG_PDL_LOG_API_LOG_ARG(...) PRV_IDBG_PDL_LOG_CALL_MACRO_OVERLOAD_MAC(PRV_IDBG_PDL_LOG_API_LOG_ARG_, __VA_ARGS__)

#define PRV_IDBG_PDL_LOG_API_LOG_ARG_0()
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_2 PRV_IDBG_PDL_LOG_API_LOG_ARG_3
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_3(type1, name1, ARG1) \
    type1 , name1, ARG1,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_5 PRV_IDBG_PDL_LOG_API_LOG_ARG_6
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_6(type1, name1, ARG1, type2, name2, ARG2) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_8 PRV_IDBG_PDL_LOG_API_LOG_ARG_9
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_9(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_11 PRV_IDBG_PDL_LOG_API_LOG_ARG_12
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_12(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_14 PRV_IDBG_PDL_LOG_API_LOG_ARG_15
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_15(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4, \
    type5 , name5, ARG5,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_17 PRV_IDBG_PDL_LOG_API_LOG_ARG_18
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_18(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4, \
    type5 , name5, ARG5, \
    type6 , name6, ARG6,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_20 PRV_IDBG_PDL_LOG_API_LOG_ARG_21
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_21(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4, \
    type5 , name5, ARG5, \
    type6 , name6, ARG6, \
    type7 , name7, ARG7,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_23 PRV_IDBG_PDL_LOG_API_LOG_ARG_24
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_24(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4, \
    type5 , name5, ARG5, \
    type6 , name6, ARG6, \
    type7 , name7, ARG7, \
    type8 , name8, ARG8,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_26 PRV_IDBG_PDL_LOG_API_LOG_ARG_27
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_27(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8, type9, name9, ARG9) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4, \
    type5 , name5, ARG5, \
    type6 , name6, ARG6, \
    type7 , name7, ARG7, \
    type8 , name8, ARG8, \
    type9 , name9, ARG9,
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_29 PRV_IDBG_PDL_LOG_API_LOG_ARG_30
#define PRV_IDBG_PDL_LOG_API_LOG_ARG_30(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8, type9, name9, ARG9, type10, name10, ARG10) \
    type1 , name1, ARG1, \
    type2 , name2, ARG2, \
    type3 , name3, ARG3, \
    type4 , name4, ARG4, \
    type5 , name5, ARG5, \
    type6 , name6, ARG6, \
    type7 , name7, ARG7, \
    type8 , name8, ARG8, \
    type9 , name9, ARG9, \
    type10, name10, ARG10,

#define PRV_IDBG_PDL_LOG_API_CALL_ARG(...) PRV_IDBG_PDL_LOG_CALL_MACRO_OVERLOAD_MAC(PRV_IDBG_PDL_LOG_API_CALL_ARG_ , __VA_ARGS__)
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_0()
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_2 PRV_IDBG_PDL_LOG_API_CALL_ARG_3
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_3(type1, name1, ARG1) \
    , ARG1
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_5 PRV_IDBG_PDL_LOG_API_CALL_ARG_6
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_6(type1, name1, ARG1, type2, name2, ARG2) \
    , ARG1, ARG2
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_8 PRV_IDBG_PDL_LOG_API_CALL_ARG_9
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_9(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3) \
    , ARG1, ARG2, ARG3
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_11 PRV_IDBG_PDL_LOG_API_CALL_ARG_12
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_12(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4) \
    , ARG1, ARG2, ARG3, ARG4
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_14 PRV_IDBG_PDL_LOG_API_CALL_ARG_15
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_15(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5) \
    , ARG1, ARG2, ARG3, ARG4, ARG5
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_17 PRV_IDBG_PDL_LOG_API_CALL_ARG_18
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_18(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6) \
    , ARG1, ARG2, ARG3, ARG4, ARG5, ARG6
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_20 PRV_IDBG_PDL_LOG_API_CALL_ARG_21
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_21(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7) \
    , ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_23 PRV_IDBG_PDL_LOG_API_CALL_ARG_24
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_24(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8) \
    , ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_26 PRV_IDBG_PDL_LOG_API_CALL_ARG_27
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_27(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8, type9, name9, ARG9) \
    , ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_29 PRV_IDBG_PDL_LOG_API_CALL_ARG_30
#define PRV_IDBG_PDL_LOG_API_CALL_ARG_30(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8, type9, name9, ARG9, type10, name10, ARG10) \
    , ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10

#define PRV_IDBG_PDL_LOG_ADD_PREFIX_MAC(prefix, cmd)            PRV_IDBG_PDL_LOG_CONCAT2_MAC(prefix, cmd)
#define PRV_IDBG_PDL_LOG_ADD_SUFFIX_MAC(cmd, suffix)            PRV_IDBG_PDL_LOG_CONCAT2_MAC(cmd, suffix)
#define PRV_IDBG_PDL_LOGGER_BULD_NAME_MAC(prefix, cmd, suffix)  PRV_IDBG_PDL_LOG_ADD_PREFIX_MAC(prefix, PRV_IDBG_PDL_LOG_CONCAT2_MAC(cmd, suffix))

#define PRV_IDBG_PDL_LOGGER_TYPEOF_MAC(type) PRV_IDBG_PDL_LOGGER_BULD_NAME_MAC(IDBG_PDL_LOGGER_TYPEOF_ , type , _CNS)

#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG(...) PRV_IDBG_PDL_LOG_CALL_MACRO_OVERLOAD_MAC(PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_ , __VA_ARGS__)
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_0()
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_2 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_3
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_3(type1, name1, ARG1) \
    ARG1
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_5 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_6
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_6(type1, name1, ARG1, type2, name2, ARG2) \
    ARG1, ARG2
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_8 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_9
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_9(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3) \
    ARG1, ARG2, ARG3
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_11 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_12
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_12(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4) \
    ARG1, ARG2, ARG3, ARG4
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_14 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_15
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_15(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5) \
    ARG1, ARG2, ARG3, ARG4, ARG5
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_17 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_18
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_18(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6) \
    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_20 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_21
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_21(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7) \
    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_23 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_24
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_24(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8) \
    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_26 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_27
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_27(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8, type9, name9, ARG9) \
    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_29 PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_30
#define PRV_IDBG_PDL_LOG_API_CALL_NO_1ST_ARG_30(type1, name1, ARG1, type2, name2, ARG2, type3, name3, ARG3, type4, name4, ARG4, type5, name5, ARG5, type6, name6, ARG6, type7, name7, ARG7, type8, name8, ARG8, type9, name9, ARG9, type10, name10, ARG10) \
    ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7, ARG8, ARG9, ARG10

#endif /* __prvLoggerApih */

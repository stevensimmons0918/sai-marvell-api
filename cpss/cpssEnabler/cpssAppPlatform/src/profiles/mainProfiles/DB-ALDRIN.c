/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file DB-ALDRIN.c
*
* @brief Aldrin profile related file.
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <profiles/cpssAppPlatformProfileDecl.h>

/*** Aldrin with one PP ****/
CPSS_CAP_PROFILE_START( DB_ALD_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &DB_98EX5520_6ZQSFP_48SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_ALDRIN_PP_INFO, &DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_ALDRIN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_ALDRIN_GEN_EVENT_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO )
CPSS_CAP_PROFILE_END( DB_ALD_all, "Aldrin DB" )

CPSS_CAP_PROFILE_START( DB_ALDB2B_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &DB_98EX5520_6ZQSFP_48SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &B2B_0_ALDRIN_PP_INFO, &DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO_0 )
  CPSS_CAP_ADD_PP_PROFILE( &B2B_1_ALDRIN_PP_INFO, &DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO_1 )

  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_ALDRIN_B2B_GEN_EVENT_INFO )

  CPSS_CAP_ADD_RUNTIME_PROFILE(&RUNTIME_EVENTS_B2B_ALDRIN_INFO)
  CPSS_CAP_ADD_RUNTIME_PROFILE(&RUNTIME_TRAFFIC_B2B_0_ALDRIN_INFO)
  CPSS_CAP_ADD_RUNTIME_PROFILE(&RUNTIME_TRAFFIC_B2B_1_ALDRIN_INFO)
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_ALDRIN_B2B )
CPSS_CAP_PROFILE_END( DB_ALDB2B_all, "Aldrin B2B" )


CPSS_CAP_PROFILE_START( DB_ALD_runtime )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_ALDRIN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_ALDRIN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_ALD_runtime, "Aldrin DB - runtime only" )

/*** Aldrin with one PP + legacy ports ***/
CPSS_CAP_PROFILE_START( DB_ALD_all_legacy )
  CPSS_CAP_ADD_BOARD_PROFILE( &DB_98EX5520_6ZQSFP_48SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_ALDRIN_PP_INFO, &DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_ALDRIN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_ALDRIN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_ALD_all_legacy, "Aldrin DB - non-PM" )


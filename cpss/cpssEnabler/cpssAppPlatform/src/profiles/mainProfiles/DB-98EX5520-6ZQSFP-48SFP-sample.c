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
* @file DB-98EX5520-6ZQSFP-48SFP.c
*
* @brief Aldrin profile related file.
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <profiles/cpssAppPlatformProfileDecl.h>

/*** Aldrin2 with one PP ****/
CPSS_CAP_PROFILE_START( DB_ALD2_48MG6CG_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &DB_98EX5520_6ZQSFP_48SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_98EX5520_PP_INFO, &DB_98EX5520_6ZQSFP_48SFP_PP_MAP_INFO )
CPSS_CAP_PROFILE_END( DB_ALD2_48MG6CG_all, "Aldrin2 DB - SampleMain profile" )


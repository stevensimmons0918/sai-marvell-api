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
* @file cpssAppPlatformProfile.h
*
* @brief Generic structure of profiles.
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PROFILE_H
#define __CPSS_APP_PROFILE_H

#include <cpss/common/cpssTypes.h>

#include <profiles/cpssAppPlatformBoardProfile.h>
#include <profiles/cpssAppPlatformPpProfile.h>
#include <profiles/cpssAppPlatformRunTimeProfile.h>
#include <cpssAppPlatformExtPhyConfig.h>

#if 1
#define ANSI_PROFILES
#endif

#ifndef ANSI_PROFILES
#define _SM_(x) \
     .x =
#else
#define _SM_(x)
#endif


#define CPSS_APP_PLATFORM_MAX_PROFILES_CNS  50

/* MACROs to be used to create a CPSS_APP_PLATFORM_PROFILE_STC profile
 * Eg: See
 **/

#define CPSS_CAP_PROFILE_END(name, desc)                                                               \
    { CPSS_APP_PLATFORM_PROFILE_TYPE_LAST_E,   {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*) NULL}}                                                  \
};                                                                                                     \
                                                                                                       \
__attribute__((constructor))                                                                           \
GT_VOID name##_add_profile (GT_VOID)                                                                   \
{                                                                                                      \
   extern GT_U32 get_cap_profile_count (GT_VOID);                                                      \
   extern GT_VOID increment_cap_profile_count (GT_VOID);                                               \
   extern GT_VOID set_cap_profile_name ( IN GT_U32 count, IN GT_CHAR* name );                          \
   extern GT_VOID set_cap_profile_descr ( IN GT_U32 count, IN GT_CHAR* descr );                        \
   extern GT_VOID set_cap_profile_list ( IN GT_U32 count, IN CPSS_APP_PLATFORM_PROFILE_STC* name );    \
                                                                                                       \
   GT_U32 count = get_cap_profile_count();                                                             \
   if (count < CPSS_APP_PLATFORM_MAX_PROFILES_CNS)                                                     \
   {                                                                                                   \
      set_cap_profile_name(count, #name);                                                              \
      set_cap_profile_descr(count, desc);                                                              \
      set_cap_profile_list(count,  name);                                                              \
      increment_cap_profile_count();                                                                   \
   }                                                                                                   \
}                                                                                                      \

#define CPSS_CAP_ADD_BOARD_PROFILE(brd_info)                                                                        \
    { CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E,   {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(brd_info)}},                \

#define CPSS_CAP_ADD_PP_PROFILE(pp, ppmap)                                                                          \
    { CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E,      {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(pp)}},                      \
    { CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E, {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(ppmap)}},                     \

#define CPSS_CAP_ADD_RUNTIME_PROFILE(rt)                                                                            \
    { CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E, {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(rt)}},                      \

#define CPSS_CAP_PROFILE_START(name)                                                                                \
CPSS_APP_PLATFORM_PROFILE_STC (name)[] = {                                                                          \

/* Validation/Test MACROs to be used to create a CPSS_APP_PLATFORM_PROFILE_STC profile
 * Eg: See
 **/

#define CPSS_CAP_TEST_PROFILE_END(name, desc)                                                               \
    {CPSS_APP_PLATFORM_PROFILE_TYPE_LAST_E,    {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)NULL }}               \
};                                                                                                          \
                                                                                                            \
__attribute__((constructor))                                                                                \
GT_VOID name##_add_profile (GT_VOID)                                                                        \
{                                                                                                           \
   extern GT_U32 get_cap_test_profile_count (GT_VOID);                                                      \
   extern GT_VOID increment_cap_test_profile_count (GT_VOID);                                               \
   extern GT_VOID set_cap_test_profile_name ( IN GT_U32 count, IN GT_CHAR* name );                          \
   extern GT_VOID set_cap_test_profile_descr ( IN GT_U32 count, IN GT_CHAR* descr );                        \
   extern GT_VOID set_cap_test_profile_list ( IN GT_U32 count, IN CPSS_APP_PLATFORM_PROFILE_STC* name );    \
                                                                                                            \
   GT_U32 count = get_cap_test_profile_count();                                                             \
   if (count < CPSS_APP_PLATFORM_MAX_PROFILES_CNS)                                                          \
   {                                                                                                        \
      set_cap_test_profile_name(count, #name);                                                              \
      set_cap_test_profile_descr(count, desc);                                                              \
      set_cap_test_profile_list(count,  name);                                                              \
      increment_cap_test_profile_count();                                                                   \
   }                                                                                                        \
}                                                                                                           \

#define CPSS_CAP_ADD_BOARD_TEST_PROFILE(brd_info)                                                    \
    {CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E,  {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(brd_info)}},   \

#define CPSS_CAP_ADD_PP_TEST_PROFILE(pp, ppmap)                                                      \
    {CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E, {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(pp)}},             \
    { CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E, {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(ppmap)}},      \

#define CPSS_CAP_ADD_RUNTIME_TEST_PROFILE(rt)                                                        \
    {CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E, {(CPSS_APP_PLATFORM_BOARD_PROFILE_STC*)(rt)}},        \

#define CPSS_CAP_TEST_PROFILE_START(name)                                                            \
CPSS_APP_PLATFORM_PROFILE_STC (name)[] = {                                                           \

typedef enum {
    CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E = 0x1,
    CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_RT_E = 0x2,
    CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_LAST_E
} CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT;

/*
 * @enum , CPSS_APP_PLATFORM_PROFILE_TYPE_ENT
 * Different types of profiles are enumerated here.
 */
typedef enum {
    CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E=0,
    CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E,
    CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E,
    CPSS_APP_PLATFORM_PROFILE_TYPE_LAST_E
} CPSS_APP_PLATFORM_PROFILE_TYPE_ENT;

/*
 * @struct CPSS_APP_PLATFORM_PROFILE_VALUE_UNT,
 * Place holder for board, PP or RunTime profile
 **/
typedef union _CPSS_APP_PLATFORM_PROFILE_VALUE_UNT
{
    /* Points to Board Profile */
    CPSS_APP_PLATFORM_BOARD_PROFILE_STC      *boardInfoPtr;

    /* Points to PP profile */
    CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppInfoPtr;

    /* Points to runTime profile */
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC    *runTimeInfoPtr;

    GT_VOID_PTR                               lastPtr;

} CPSS_APP_PLATFORM_PROFILE_VALUE_UNT;

/*
 * @struct CPSS_APP_PLATFORM_PROFILE_STC.
 * this is the Genric Profile structure. This structure can
 * hold board, pp or runTime profile. The profileType identifies
 * if profileValue has board, pp or runtime Profile.
 */
typedef struct _CPSS_APP_PLATFORM_PROFILE_STC
{
    /* Enum which says what kind of profile profileValue has */
    CPSS_APP_PLATFORM_PROFILE_TYPE_ENT    profileType;
    /* Actual profileValue, this can be board, pp or runtime profile*/
    CPSS_APP_PLATFORM_PROFILE_VALUE_UNT   profileValue;

} CPSS_APP_PLATFORM_PROFILE_STC;


/**
 * Function prototypes
 */

GT_STATUS
cpssAppPlatformProfileDbEntryGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT entryType,
    OUT GT_CHAR                                   **profileStr
);

GT_STATUS
cpssAppPlatformProfileDbReset
(
    GT_VOID
);

GT_STATUS
cpssAppPlatformProfileDbDelete
(
    IN  GT_U8  devNum,
    IN  CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT entryType
);

GT_STATUS
cpssAppPlatformProfileDbAdd
(
    IN  GT_U8                                       devNum,
    IN  CPSS_APP_PLATFORM_PROFILE_STC              *profilePtr,
    IN  CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_ENT entryType
);

GT_STATUS
cpssAppPlatformProfileGet
(
   IN  GT_CHAR                        *profileName,
   OUT CPSS_APP_PLATFORM_PROFILE_STC **profile
);

GT_STATUS
cpssAppPlatformListProfiles
(
   GT_VOID
);

GT_STATUS
cpssAppPlatformListTestProfiles
(
   GT_VOID
);

GT_STATUS
cpssAppPlatformPrintProfile
(
   IN  GT_CHAR                       *profileName
);

#endif /* __CPSS_APP_PROFILE_H */

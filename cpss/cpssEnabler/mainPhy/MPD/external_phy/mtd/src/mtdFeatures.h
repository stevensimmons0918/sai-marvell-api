/*******************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains MACROS definitions to control whether optional features
should be included at compile time. User should only select the supported features
on the device. Define MACROS to 1 to select, 0 to ignore it in building.

Also, the files users include to build should exactly match everything selected in
this file to avoid any issues.
********************************************************************/
#ifndef MTD_FEATURES_H
#define MTD_FEATURES_H

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/********************************************************************
 Description of Optional Features
 --------------------------------
 This API has functions available for some parts that are not required
 for other parts, or also may be excluded if that functionality is
 not desired. The purpose to exclude these would only be for the purposes
 of decreasing compile time or reducing the size of library file (or
 in the case your linker does not do "smart linking" and includes 
 functions which are not called).

 Most PHY parts share the majority of functions in this API. Below is listed
 the part numbers which have optional functions available. To include
 that functionality set the define to 1, or to exclude that functionality
 set the define to 0. The default setting is to build everything.

 With most optional defines there are one or more subdirectories which must
 be included in order to compile. See below for that information.

 List of Optional Feature Support by Part Number
 -----------------------------------------------
 PHYs 88X3220/88X3240/88X3310/88X3320/88X3340/88E2010/88E2040 and P versions
 of these parts which include Macsec/PTP:
 MTD_ORIGSERDES (to use serdes functions, for example, to swap serdes polarity)
 MTD_LINKCRYPT_MSECPTP (only for P versions to use Macsec and/or PTP)

 PHYs 88E2110/88E2180
 MTD_ORIGSERDES (to use serdes functions, for example, to swap serdes polarity)

 PHYs 88X3540/88E2540
 MTD_CE_SERDES28X2 (to use serdes functions, for example, to swap serdes polarity)
 MTD_TU_PTP (to use PTP)

 ********************************************************************/
#ifndef MTD_INCLUDE_ALL
#define MTD_INCLUDE_ALL 1 /* Need to include all subdirectories below if 1. */
                          /* Set to 0 to exclude all optional features. */
                          /* Set to 0 and change defines below to include only */
                          /* some optional features. */
#endif

#if MTD_INCLUDE_ALL
#define MTD_LINKCRYPT_MSECPTP 1
#define MTD_ORIGSERDES        1
#define MTD_TU_PTP            1
#define MTD_CE_SERDES28X2     1
#endif


/********************************************************************
 Optional features. Can force inclusion by defining as 1 before or here
 and including the following header/source files in these directories
 corresponding directories.
 ********************************************************************/

/* MACsec+PTP LinkCryptPtp Support, include subdirectory ./LinkCryptPtp */
#ifndef MTD_LINKCRYPT_MSECPTP
#define MTD_LINKCRYPT_MSECPTP 0
#endif

/* Older serdes support, no other subdirectories to include */
#ifndef MTD_ORIGSERDES
#define MTD_ORIGSERDES        0
#endif

/* PTP Support, include subdirectory ./TuPtp */
#ifndef MTD_TU_PTP
#define MTD_TU_PTP            0
#endif

/* CE Serdes Support, include sudirectories:
   ./serdes/mcesd
   ./serdes/mcesd/C28GP4X2 (for 28X2, exclude this directory if not needed)  */

#ifndef MTD_CE_SERDES28X2
#define MTD_CE_SERDES28X2        0
#endif

/* Groups of related features that share code */
/* Do not modify. */
#define MTD_PKG_CE_SERDES         MTD_CE_SERDES28X2



#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* defined MTD_FEATURES_H */

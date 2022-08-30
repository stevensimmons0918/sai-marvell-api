/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/


#ifndef MPD_SAMPLE_H_MPDSAMPLE_H_
#define MPD_SAMPLE_H_MPDSAMPLE_H_

#include 	<mpd.h>

/* ******* DEBUG ********/
#define MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS	(10)


typedef struct {
	BOOLEAN	  isFree;
	char 	* flagName_PTR;
	char	* flagHelp_PTR;
	BOOLEAN   flagStatus; /* TRUE - On */
}MPD_SAMPLE_DEBUG_FLAG_INFO_STC;

typedef enum {
	MPD_SAMPLE_MDIO_TYPE_SMI_E,
	MPD_SAMPLE_MDIO_TYPE_XSMI_E,
	MPD_SAMPLE_MDIO_TYPE_UNKNOWN_E
}MPD_SAMPLE_MDIO_TYPE_ENT;


typedef struct {
	BOOLEAN						valid;
	UINT_8						dev;
	UINT_8						port;
	MPD_SAMPLE_MDIO_TYPE_ENT	mdioType;
	MPD_TYPE_ENT				phyType;
	UINT_8						phyNumber;
	UINT_8						phySlice;
	UINT_8						mdioBus;
	UINT_8						mdioAddress;
    MPD_PHY_USX_TYPE_ENT        usxInfo;
}MPD_SAMPLE_PORT_DB_STC;


#endif /* MPD_SAMPLE_H_MPDSAMPLE_H_ */

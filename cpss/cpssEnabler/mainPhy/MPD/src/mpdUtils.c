/* *****************************************************************************
Copyright (C) 2014 - 2020, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
/**
 * @file mpdUtils.c
 *	@brief MPD utility functions that are shared among all MPD modules
 *
 */

#include 		  <mpdPrv.h>
#include 		  <mpdDebug.h>

#undef __FUNCTION__

/* this array is used in order to count num of members in PRV_MPD_PORT_LIST_TYP */

static UINT_8 prvMpdByteBitmapPopulation [256] = {
0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

static const PRV_MPD_PORT_LIST_TYP prvMpdEmptyPortList_CNS = {{0, 0, 0, 0}};

/* this array will hold rel_ifIndex per <dev, mdioBus, mdioAddress
 * 0 is an illegal ifIndex
 */
static UINT_32 prvMpdFindPortEntryAssist_ARR[PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS][MPD_MAX_INTERFACE_ID_NUMBER][PRV_MPD_MAX_MDIO_ADRESS_CNS+1] = {{{0}}};


/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListIsMember
*
* DESCRIPTION: TRUE when rel_ifIndex is member of portList_PTR
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListIsMember (
	/*     INPUTS:             */
	UINT_32					rel_ifIndex,
	PRV_MPD_PORT_LIST_TYP * portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_32	word, idx;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
		return FALSE;
	}
	word = ((rel_ifIndex - 1)/32);
	idx = ((rel_ifIndex - 1)%32);

	return (portList_PTR->portsList[word] & (1<<idx))?TRUE:FALSE;

}
/* END OF prvMpdPortListIsMember */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListIsEmpty
*
* DESCRIPTION: checks if no port is selected in set
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListIsEmpty(
  /*!     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
  /*!     INPUTS / OUTPUTS:   */
  /*!     OUTPUTS:            */
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    BOOLEAN is_empty = TRUE;
    UINT_32 word;
/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                       */
/*!*************************************************************************/

    for (word = 0; word < PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS; word++) {
        if (portList_PTR->portsList[word] != 0) {
            is_empty = FALSE;
            break;
        }
    }
    return is_empty;
}
/*$ END OF <prvMpdPortListIsEmpty> */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListGetNext
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortListGetNext(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR,
  /*     INPUTS / OUTPUTS:   */
    UINT_32    * relative_ifIndex_PTR
  /*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32 i_in_word = 0, word = 0;
/* ************************************************************************/
/*                      F U N C T I O N   L O G I C                       */
/* ************************************************************************/
    (* relative_ifIndex_PTR)++;

    if (PRV_MPD_PORT_LIST_REL_IFINDEX_IS_LEGAL_MAC((* relative_ifIndex_PTR)) == TRUE) {
        PRV_MPD_PORT_LIST_GET_SHIFT_AND_WORD_MAC((* relative_ifIndex_PTR), i_in_word, word);

        for (; word < PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS; word++, i_in_word = 0) {
            for (; i_in_word < 32; i_in_word++) {
                if (portsList_PTR->portsList[word] & (1 << i_in_word)) {
                    (* relative_ifIndex_PTR) = PRV_MPD_PORT_LIST_WORD_AND_BIT_TO_REL_IFINDEX_MAC(word, i_in_word);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

/*$ END OF  prvMpdPortListGetNext */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListNumOfMembers
*
* DESCRIPTION: get num of ports in pset
*
*
*****************************************************************************/
extern UINT_32 prvMpdPortListNumOfMembers(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portsList_PTR
  /*     INPUTS / OUTPUTS:   */
  /*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32      tmp_byte;
    UINT_32      tmp_word = 0, mask;
    UINT_32      numOfMembers = 0, i, j;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portsList_PTR == NULL) {
        return 0;
    }

    for (i = 0; i < PRV_MPD_NUM_OF_WORDS_IN_PORTS_BITMAP_CNS; i++) {
        tmp_word = portsList_PTR->portsList[i];
        mask = 0xFF;
        for (j = 0; j < 4; j++) {
            tmp_byte = ((tmp_word & mask) >> (8 * j));
            numOfMembers += prvMpdByteBitmapPopulation[tmp_byte];
            mask = mask << 8;
        }
    }
    return numOfMembers;

}

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListRemove
*
* DESCRIPTION:
*
*
*****************************************************************************/
BOOLEAN prvMpdPortListRemove (
	/*     INPUTS:             */
	UINT_32							rel_ifIndex,
	PRV_MPD_PORT_LIST_TYP * portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_32	word, idx;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
		return FALSE;
	}
	word = ((rel_ifIndex - 1)/32);
	idx = ((rel_ifIndex - 1)%32);
	portList_PTR->portsList[word] &= ~(1<<idx);
	return TRUE;
}
/* END OF prvMpdPortListRemove */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListClear
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern void prvMpdPortListClear(
  /*     INPUTS:             */
    PRV_MPD_PORT_LIST_TYP * portList_PTR
  /*     INPUTS / OUTPUTS:   */
  /*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ************************************************************************/
/*                      F U N C T I O N   L O G I C                       */
/* ************************************************************************/
    if (portList_PTR) {
        memset(portList_PTR, 0, sizeof(PRV_MPD_PORT_LIST_TYP));
    }
}

/*$ END OF  prvMpdPortListClear */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortListAdd
*
* DESCRIPTION:
*
*
*****************************************************************************/
BOOLEAN prvMpdPortListAdd (
	/*     INPUTS:             */
	UINT_32					rel_ifIndex,
	PRV_MPD_PORT_LIST_TYP * portList_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_32	word, idx;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	if (rel_ifIndex <= 0 || rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
		return FALSE;
	}
	word = ((rel_ifIndex - 1)/32);
	idx = ((rel_ifIndex - 1)%32);
	portList_PTR->portsList[word] |= (1<<idx);
	return TRUE;
}
/* END OF prvMpdPortListAdd */

/* ***************************************************************************
* FUNCTION NAME: prvMpdCheckIsPortValid
*
* DESCRIPTION: check that the port exist and valid in the DB, and return the entry
*
*
*****************************************************************************/
static PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdCheckIsPortValid (
    /*     INPUTS:             */
    UINT_32 rel_ifIndex
    /*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PRV_MPD_PORT_HASH_ENTRY_STC * portEntry_PTR = NULL;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (prvMpdGlobalDb_PTR == NULL) {
        return NULL;
    }

    if (rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
        return NULL;
    }

    if (prvMpdPortListIsMember(rel_ifIndex, &(prvMpdGlobalDb_PTR->validPorts)) == FALSE) {
        return NULL;
    }
    portEntry_PTR = &prvMpdGlobalDb_PTR->entries[rel_ifIndex];

    return portEntry_PTR;

}
/* END OF prvMpdCheckIsPortValid */

/* ***************************************************************************
* FUNCTION NAME: prvMpdPortHashCreate
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern BOOLEAN prvMpdPortHashCreate (
	/*     INPUTS:             */
	void
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	if (prvMpdGlobalDb_PTR != NULL) {
		return FALSE;
	}

	prvMpdGlobalDb_PTR = PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_GLOBAL_DB_STC));
	if (prvMpdGlobalDb_PTR) {
		memset(prvMpdGlobalDb_PTR->entries, 0, sizeof(prvMpdGlobalDb_PTR->entries));
		prvMpdGlobalDb_PTR->numOfMembers = 0;
		prvMpdGlobalDb_PTR->validPorts = prvMpdEmptyPortList_CNS;
		return TRUE;
	}
	return FALSE;
}
/* END OF prvMpdPortHashCreate */


/* ***************************************************************************
* FUNCTION NAME: mpdPortInitDbValidation
*
* DESCRIPTION: validate port init db function.
*
*****************************************************************************/
static MPD_RESULT_ENT mpdPortInitDbValidation (
    /*     INPUTS:             */
    UINT_32						rel_ifIndex,
    MPD_PORT_INIT_DB_STC	  * data_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
        char error_msg [256];
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (data_PTR->phyType >= MPD_TYPE_NUM_OF_TYPES_E) {
        sprintf(error_msg, "rel_ifIndex %d - received %d which is not valid PHY type\n", rel_ifIndex, data_PTR->phyType);
        PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                    MPD_ERROR_SEVERITY_FATAL_E,
                                    error_msg);
        return MPD_OP_FAILED_E;
    }
    if (data_PTR->transceiverType >= MPD_TRANSCEIVER_MAX_TYPES_E) {
        sprintf(error_msg, "rel_ifIndex %d - received %d which is not valid transceiver type\n", rel_ifIndex, data_PTR->transceiverType);
        PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                    MPD_ERROR_SEVERITY_FATAL_E,
                                    error_msg);
        return MPD_OP_FAILED_E;
    }
    if (data_PTR->usxInfo.usxType != MPD_PHY_USX_TYPE_NONE) {
        if (data_PTR->usxInfo.usxType >= MPD_PHY_USX_MAX_TYPES) {
            sprintf(error_msg, "rel_ifIndex %d - received %d which is not valid USX type\n", rel_ifIndex, data_PTR->usxInfo.usxType);
            PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                        MPD_ERROR_SEVERITY_FATAL_E,
                                        error_msg);
            return MPD_OP_FAILED_E;
        }
        if ((data_PTR->phyType == MPD_TYPE_88E2540_E) || (data_PTR->phyType == MPD_TYPE_88E2580_E)) {
            if (data_PTR->usxInfo.usxType == MPD_PHY_USX_TYPE_20G_DXGMII) {
                sprintf(error_msg, "rel_ifIndex %d - USX type 20G DXGMII is not supported on this PHY\n", rel_ifIndex);
                PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                            MPD_ERROR_SEVERITY_FATAL_E,
                                            error_msg);
                return MPD_OP_FAILED_E;
            }
        }

    }
    if (data_PTR->mdioInfo.mdioAddress  > PRV_MPD_MAX_MDIO_ADRESS_CNS){
        sprintf(error_msg, "rel_ifIndex %d - received %d which is not valid MDIO address\n", rel_ifIndex, data_PTR->mdioInfo.mdioAddress);
        PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                    MPD_ERROR_SEVERITY_FATAL_E,
                                    error_msg);
        return MPD_OP_FAILED_E;
    }
    if ((data_PTR->phyType == MPD_TYPE_88E1543_E) && (data_PTR->mdioInfo.mdioAddress == 0)){
        sprintf(error_msg, "rel_ifIndex %d - MDIO address 0 is not valid for this PHY\n", rel_ifIndex);
        PRV_MPD_HANDLE_FAILURE_MAC( rel_ifIndex,
                                    MPD_ERROR_SEVERITY_FATAL_E,
                                    error_msg);
        return MPD_OP_FAILED_E;
    }

    return MPD_OK_E;
}

/* END OF mpdPortInitDbValidation */

/* ***************************************************************************
* FUNCTION NAME: mpdPortRunningDbInit
*
* DESCRIPTION: init port running db function.
*
*****************************************************************************/
static MPD_RESULT_ENT mpdPortRunningDbInit (
    /*     INPUTS:             */
    UINT_32                       rel_ifIndex,
    MPD_PORT_INIT_DB_STC      * data_PTR
    /*     INPUTS / OUTPUTS:   */
    /*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->adminMode = (data_PTR->disableOnInit) ? MPD_PORT_ADMIN_DOWN_E : MPD_PORT_ADMIN_UP_E;
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->comboMode = MPD_COMBO_MODE_LAST_E;
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->macOnPhyState = PRV_MPD_MAC_ON_PHY_NOT_INITIALIZED_E;
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->opMode = MPD_OP_MODE_UNKNOWN_E;
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->sfpPresent = FALSE;
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->speed = MPD_SPEED_LAST_E;
    prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR->vct_offset = 0;

    return MPD_OK_E;
}

/* END OF mpdPortRunningDbInit */


/* ***************************************************************************
* FUNCTION NAME: mpdPortDbUpdate
*
* DESCRIPTION: update entry for <rel_ifIndex>
*      		allocates and adds entry if not allocated
*
*****************************************************************************/
extern MPD_RESULT_ENT mpdPortDbUpdate (
	/*     INPUTS:             */
	UINT_32						rel_ifIndex,
	MPD_PORT_INIT_DB_STC	  * data_PTR
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	BOOLEAN 				exists = FALSE;
    MPD_RESULT_ENT          status = MPD_OK_E;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	if (prvMpdGlobalDb_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}

	if (rel_ifIndex >= MPD_MAX_PORT_NUMBER_CNS) {
		return MPD_OP_FAILED_E;
	}

	if (prvMpdPortListIsMember(rel_ifIndex, &(prvMpdGlobalDb_PTR->validPorts))) {
		exists = TRUE;
	}
	/* remove entry */
	if (data_PTR == NULL) {
		if (exists) {
			prvMpdPortListRemove(rel_ifIndex, &(prvMpdGlobalDb_PTR->validPorts));
			prvMpdGlobalDb_PTR->numOfMembers--;
			return MPD_OK_E;
		}
		return MPD_OP_FAILED_E;
	}

	/* allocation is required */
	if (exists == FALSE) {
        status = mpdPortInitDbValidation(rel_ifIndex, data_PTR);
        if (status != MPD_OK_E){
            return MPD_OP_FAILED_E;
        }
        prvMpdGlobalDb_PTR->entries[rel_ifIndex].initData_PTR = PRV_MPD_ALLOC_MAC(sizeof(MPD_PORT_INIT_DB_STC));
		/* allocation failed */
		if (prvMpdGlobalDb_PTR->entries[rel_ifIndex].initData_PTR == NULL) {
			return MPD_OP_FAILED_E;
		}
		prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR = PRV_MPD_ALLOC_MAC(sizeof(PRV_MPD_RUNNING_DB_STC));
		/* allocation failed */
		if (prvMpdGlobalDb_PTR->entries[rel_ifIndex].runningData_PTR == NULL) {
			return MPD_OP_FAILED_E;
		}

		mpdPortRunningDbInit(rel_ifIndex, data_PTR);
		prvMpdGlobalDb_PTR->numOfMembers++;
		prvMpdPortListAdd(rel_ifIndex, &(prvMpdGlobalDb_PTR->validPorts));
		prvMpdGlobalDb_PTR->entries[rel_ifIndex].rel_ifIndex = rel_ifIndex;
	}
	/* add the entry */
	memcpy(prvMpdGlobalDb_PTR->entries[rel_ifIndex].initData_PTR, data_PTR, sizeof(MPD_PORT_INIT_DB_STC));

	return MPD_OK_E;
}
/* END OF mpdPortDbUpdate */

/* ***************************************************************************
* FUNCTION NAME: prvMpdGetPortEntry
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdGetPortEntry (
	/*     INPUTS:             */
	UINT_32	rel_ifIndex
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	return	prvMpdCheckIsPortValid(rel_ifIndex);
}
/* END OF prvMpdGetPortEntry */

/* ***************************************************************************
* FUNCTION NAME: prvMpdFindPortEntry
*
* DESCRIPTION:
*
*
*****************************************************************************/

extern PRV_MPD_PORT_HASH_ENTRY_STC * prvMpdFindPortEntry(
	/*     INPUTS:             */
	PRV_MPD_APP_DATA_STC	* app_data_PTR,
	UINT_16					mdio_address
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	UINT_32							rel_ifIndex, devIdx = 0;
	PRV_MPD_PORT_HASH_ENTRY_STC	  * port_entry_PTR;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	if (mdio_address > PRV_MPD_MAX_MDIO_ADRESS_CNS) {
		return NULL;
	}

	rel_ifIndex = prvMpdFindPortEntryAssist_ARR[app_data_PTR->mdioDev][app_data_PTR->mdioBus][mdio_address];
	if (rel_ifIndex) {
		return prvMpdGetPortEntry(rel_ifIndex);
	}
	for (rel_ifIndex = 0 ;prvMpdPortListGetNext(&prvMpdGlobalDb_PTR->validPorts,&rel_ifIndex);) {
        port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
		if (port_entry_PTR != NULL) {
			devIdx = port_entry_PTR->initData_PTR->mdioInfo.mdioDev % PRV_MPD_MAX_NUM_OF_PP_IN_UNIT_CNS;
			if (devIdx == app_data_PTR->mdioDev &&
					port_entry_PTR->initData_PTR->mdioInfo.mdioBus == app_data_PTR->mdioBus &&
					port_entry_PTR->initData_PTR->phyType == app_data_PTR->phyType &&
					port_entry_PTR->initData_PTR->mdioInfo.mdioAddress == mdio_address) {

				prvMpdFindPortEntryAssist_ARR[app_data_PTR->mdioDev][app_data_PTR->mdioBus][mdio_address] = rel_ifIndex;
				return port_entry_PTR;
			}
		}
	}

	return NULL;
}


/* ***************************************************************************
* FUNCTION NAME: prvMpdCalloc
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern void* prvMpdCalloc (
	/*     INPUTS:             */
    UINT_32                 numOfObjects,
    UINT_32                 len
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	void 		* data_PTR;
	UINT_32		size = numOfObjects*len;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
	data_PTR = PRV_MPD_ALLOC_MAC(size);
	if (data_PTR) {
		memset(data_PTR,0,size);
	}
	return data_PTR;
}
/* END OF prvMpdCalloc */

/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioReadRegisterNoPage
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioReadRegisterNoPage(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *value_PTR
)
{
	return prvMpdMdioReadRegister(	calling_func_PTR,
									portEntry_PTR,
									PRV_MPD_IGNORE_PAGE_CNS,
									address,
									value_PTR);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdXsmiReadRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioReadRegister(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 deviceOrPage,
	UINT_16 address,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *value_PTR
)
{
	UINT_16 read_value = 0;
	UINT_32 retry_counter = 10;
	MPD_RESULT_ENT status = MPD_OP_FAILED_E;
	MPD_OPERATIONS_PARAMS_UNT phy_params;

	while (status != MPD_OK_E && (retry_counter--) > 0) {
		*value_PTR = 0;

        /* check if phy type support page select register and we are called with valid page number */
        if ((deviceOrPage != PRV_MPD_IGNORE_PAGE_CNS) && (prvMpdGetPhyOperationName(portEntry_PTR, PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E) != NULL)) {
            /* moving to new page */
            phy_params.internal.phyPageSelect.page = deviceOrPage;
            phy_params.internal.phyPageSelect.readPrevPage = TRUE;
            status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                &phy_params);
            if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
                PRV_MPD_SLEEP_MAC((10));
                PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "%s. rel_ifIndex %d. new page %d. failed with status 0x%x",
                                                            calling_func_PTR,
                                                            portEntry_PTR->rel_ifIndex,
                                                            phy_params.internal.phyPageSelect.page,
                                                            status);
                continue;
#endif
            }
            /* read register */
            status = PRV_MPD_MDIO_READ_MAC( portEntry_PTR->rel_ifIndex,
                                        PRV_MPD_IGNORE_PAGE_CNS,
                                        address,
                                        &read_value);
            if (status == MPD_OK_E) {
                /* moving to old page */
                phy_params.internal.phyPageSelect.page = phy_params.internal.phyPageSelect.prevPage;
                phy_params.internal.phyPageSelect.readPrevPage = FALSE;
                status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                    PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                    &phy_params);
                if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
                    PRV_MPD_SLEEP_MAC((10));
                    PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                                "%s. rel_ifIndex %d. old page %d. failed with status 0x%x",
                                                                calling_func_PTR,
                                                                portEntry_PTR->rel_ifIndex,
                                                                phy_params.internal.phyPageSelect.page,
                                                                status);
                    continue;
#endif
                }
            }
        } else {
            status = PRV_MPD_MDIO_READ_MAC( portEntry_PTR->rel_ifIndex,
                                        deviceOrPage,
                                        address,
                                        &read_value);
        }

		*value_PTR = read_value;

		if (status != MPD_OK_E) {
#ifndef PHY_SIMULATION
			PRV_MPD_SLEEP_MAC((10));
			PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
															"%s. reading from portEntry_PTR->rel_ifIndex %d failed with status  0x%x",
															calling_func_PTR,
															portEntry_PTR->rel_ifIndex,
															status);
#endif
		}
		else {
			PRV_MPD_DEBUG_LOG_PORT_MAC( prvMpdDebugReadFlagId, portEntry_PTR->rel_ifIndex)(	calling_func_PTR,
																							"[R] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] value [0x%04x]",
																							portEntry_PTR->rel_ifIndex,
																							deviceOrPage,
																							address,
																							read_value);
		}
	}
#ifdef PHY_SIMULATION
    return MPD_OK_E;
#else
    return status;
#endif
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioWriteRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioWriteRegisterNoPage(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	UINT_16 value
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	return prvMpdMdioWriteRegister(	calling_func_PTR,
									portEntry_PTR,
									PRV_MPD_IGNORE_PAGE_CNS,
									address,
									PRV_MPD_MDIO_WRITE_ALL_MASK_CNS,
									value);
}
/* ***************************************************************************
 * FUNCTION NAME: prvMpdMdioWriteRegister
 *
 * DESCRIPTION:
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdMdioWriteRegister(
	/*     INPUTS:             */
	const char *calling_func_PTR,
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 device,
	UINT_16 address,
	UINT_16 mask,
	UINT_16 value
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	UINT_16 read_value = 0, write_value;
	MPD_RESULT_ENT status = MPD_OK_E;
	MPD_OPERATIONS_PARAMS_UNT phy_params;
	PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugWriteFlagId)(calling_func_PTR,
													"[R&W] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] mask [0x%04x] value [0x%04x]",
													portEntry_PTR->rel_ifIndex,
													device,
													address,
													mask,
													value);


	if (mask != PRV_MPD_MDIO_WRITE_ALL_MASK_CNS) {
		status = prvMpdMdioReadRegister(calling_func_PTR,
										portEntry_PTR,
									    device,
									    address,
									    &read_value);
		if (status != MPD_OK_E) {
#ifndef PHY_SIMULATION
			PRV_MPD_HANDLE_FAILURE_MAC(	portEntry_PTR->rel_ifIndex,
										MPD_ERROR_SEVERITY_FATAL_E,
										PRV_MPD_DEBUG_FUNC_NAME_MAC());
			PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
																"read failed with status 0x%x",
																status);
			return status;
#endif
		}
		write_value = (~mask & read_value) | (mask & value);
		/* no need to configure*/
		if (write_value == read_value) {
			PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(calling_func_PTR,
																"(write value)& (mask) is same as read value value - not writing");
			return MPD_OK_E;
		}
	}
	else {
		write_value = value;

		PRV_MPD_DEBUG_LOG_PORT_MAC(prvMpdDebugWriteFlagId, portEntry_PTR->rel_ifIndex)(	calling_func_PTR,
																						"[W] rel_ifIndex [%0d] deviceOrPage [%02d] address [0x%04x] value [0x%04x]",
																						portEntry_PTR->rel_ifIndex,
																						device,
																						address,
																						write_value);
	}
	/* write value */
	/* check if phy type support page select register and we are called with valid page number */
    if ((device != PRV_MPD_IGNORE_PAGE_CNS) && (prvMpdGetPhyOperationName(portEntry_PTR, PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E) != NULL)) {
        /* moving to new page */
        phy_params.internal.phyPageSelect.page = device;
        phy_params.internal.phyPageSelect.readPrevPage = TRUE;
        status = prvMpdPerformPhyOperation(portEntry_PTR,
                                            PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                            &phy_params);
        if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
            PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                        "%s. rel_ifIndex %d. new page %d. failed with status 0x%x",
                                                        calling_func_PTR,
                                                        portEntry_PTR->rel_ifIndex,
                                                        phy_params.internal.phyPageSelect.page,
                                                        status);
            return status;
#endif
        }
        /* write register */
        status = PRV_MPD_MDIO_WRITE_MAC (portEntry_PTR->rel_ifIndex,
                                     device,
                                     address,
                                     write_value);
        if (status == MPD_OK_E) {
            /* moving to old page */
            phy_params.internal.phyPageSelect.page = phy_params.internal.phyPageSelect.prevPage;
            phy_params.internal.phyPageSelect.readPrevPage = FALSE;
            status = prvMpdPerformPhyOperation(portEntry_PTR,
                                                PRV_MPD_OP_CODE_SET_PHY_PAGE_SELECT_E,
                                                &phy_params);
            if (status == MPD_OP_FAILED_E) {
#ifndef PHY_SIMULATION
                PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "%s. rel_ifIndex %d. old page %d. failed with status 0x%x",
                                                            calling_func_PTR,
                                                            portEntry_PTR->rel_ifIndex,
                                                            phy_params.internal.phyPageSelect.page,
                                                            status);
                return status;
#endif
            }
        }
    } else {
        status = PRV_MPD_MDIO_WRITE_MAC (portEntry_PTR->rel_ifIndex,
                                         device,
                                         address,
                                         write_value);
    }
    if (status != MPD_OK_E) {
#ifndef PHY_SIMULATION
        PRV_MPD_HANDLE_FAILURE_MAC( portEntry_PTR->rel_ifIndex,
                                    MPD_ERROR_SEVERITY_FATAL_E,
                                    PRV_MPD_DEBUG_FUNC_NAME_MAC());
        PRV_MPD_DEBUG_LOG_MAC( prvMpdDebugErrorFlagId)(PRV_MPD_DEBUG_FUNC_NAME_MAC(),
                                                            "write failed with status 0x%x",
                                                            status);
#endif
    }
#ifdef PHY_SIMULATION
    return MPD_OK_E;
#else
    return status;
#endif
}
static UINT_16 prvMpdSliceNumToOffset_ARR[PRV_MPD_MAX_NUM_OF_SLICES_CNS] = {
																				PRV_MPD_1540_SLICE_0_OFFSET_CNS,
																				PRV_MPD_1540_SLICE_1_OFFSET_CNS,
																				PRV_MPD_1540_SLICE_2_OFFSET_CNS,
																				PRV_MPD_1540_SLICE_3_OFFSET_CNS
};

/* ***************************************************************************
 * FUNCTION NAME: prvMpdLinkCryptGetActualAddress
 *
 * DESCRIPTION: indirect access of linkcrypt register is accessing a shared (all ports in quad)
 *	RAM area, each port in quad has it's addressing offset which is defined at prvMpdSliceNumToOffset_ARR
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdLinkCryptGetActualAddress(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	/*     INPUTS / OUTPUTS:   */
	UINT_16 *address_PTR
	/*     OUTPUTS:            */
)
{
	UINT_32 slice = 0;
	UINT_16 offset;

	slice = portEntry_PTR->initData_PTR->phySlice;
	offset = prvMpdSliceNumToOffset_ARR[slice];
	*address_PTR = *address_PTR + offset;
	return MPD_OK_E;
}


/* ***************************************************************************
 * FUNCTION NAME: prvMpdReadLinkCryptReg
 *
 * DESCRIPTION: indirect access to linkCrypt registers, 1540M
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdReadLinkCryptReg(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_32 *data_PTR
)
{
	MPD_RESULT_ENT status;
	UINT_16 data_high = 0, data_low = 0;
	prvMpdLinkCryptGetActualAddress(portEntry_PTR,
									&address);

	/* switch to page 16*/
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								PRV_MPD_LINKCRYPT_INDIRECT_ACCESS_PAGE_CNS);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* select address */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_LINKCRYPT_READ_ADDRESS_REG_CNS,
								address);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* read data low */
	status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_LINKCRYPT_DATA_LOW_REG_CNS,
								&data_low);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* read data high */
	status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_LINKCRYPT_DATA_HIGH_REG_CNS,
								&data_high);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	*data_PTR = (data_high << 16) | (data_low & 0xFFFF);
	/* write back old page */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								0);
	return status;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdWriteLinkCryptRegMask
 *
 * DESCRIPTION: write linkcrypt register using given mask, 1540M
 *
 *
 *****************************************************************************/
static MPD_RESULT_ENT prvMpdWriteLinkCryptRegMask(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	UINT_32 data,
	UINT_32 mask
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	UINT_32 write_value, old_data;
	UINT_16 data_high = 0, data_low = 0;
	prvMpdLinkCryptGetActualAddress(portEntry_PTR,
									&address);
	/* switch to page 16*/
	write_value = data;
	if (mask != 0XFFFFFFFF) {
		prvMpdReadLinkCryptReg(	portEntry_PTR,
								address,
								&old_data);
		write_value = (~mask & old_data) | (mask & data);
		if (write_value == old_data)
			return MPD_OK_E;
	}
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								PRV_MPD_LINKCRYPT_INDIRECT_ACCESS_PAGE_CNS);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* select address */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_LINKCRYPT_WRITE_ADDRESS_REG_CNS,
								address);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	data_high = (write_value >> 16);
	data_low = write_value & 0xFFFF;
	/* write data low */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_LINKCRYPT_DATA_LOW_REG_CNS,
								data_low);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* write data high */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_LINKCRYPT_DATA_HIGH_REG_CNS,
								data_high);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* write back old page */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								0);
	return status;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdWriteLinkCryptRegMask
 *
 * DESCRIPTION: indirect access to linkCrypt registers
 *
 *
 *****************************************************************************/

GT_STATUS prvMpdWriteLinkCryptReg(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_16 address,
	UINT_32 data
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	return prvMpdWriteLinkCryptRegMask(	portEntry_PTR,
										address,
										data,
										0xFFFFFFFF);
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdXmdioWriteReg
 *
 * DESCRIPTION: indirect access to XMDIO registers
 *
 *
 *****************************************************************************/

MPD_RESULT_ENT prvMpdXmdioWriteReg(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_8 device,
	UINT_16 address,
	UINT_16 data
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
)
{
	MPD_RESULT_ENT status;
	UINT_16 value;

	/* switch to page 0*/
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								PRV_MPD_MMD_INDIRECT_ACCESS_PAGE_CNS);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* select address */
	value = (PRV_MPD_MMD_FUNCTION_ADDRESS_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
								value);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* write address  */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
								address);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* select data */
	value = (PRV_MPD_MMD_FUNCTION_DATA_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
								value);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* write data  */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
								data);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;
}

/* ***************************************************************************
 * FUNCTION NAME: prvMpdXmdioReadReg
 *
 * DESCRIPTION: indirect access to XMDIO registers
 *
 *
 *****************************************************************************/
MPD_RESULT_ENT prvMpdXmdioReadReg(
	/*     INPUTS:             */
	PRV_MPD_PORT_HASH_ENTRY_STC *portEntry_PTR,
	UINT_8 device,
	UINT_16 address,
	/*     INPUTS / OUTPUTS:   */
	/*     OUTPUTS:            */
	UINT_16 *data_PTR
)
{
	MPD_RESULT_ENT status;
	UINT_16 value;

	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_PAGE_SELECT_ADDRESS_CNS,
								PRV_MPD_MMD_INDIRECT_ACCESS_PAGE_CNS);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* select address */
	value = (PRV_MPD_MMD_FUNCTION_ADDRESS_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
								value);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* write address  */
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
								address);

	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* select data */
	value = (PRV_MPD_MMD_FUNCTION_DATA_CNS | (PRV_MPD_MMD_DEVICE_MASK_CNS & device));
	status = prvMpdMdioWriteRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_FUNCTION_AND_DEVICE_REG_CNS,
								value);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);
	/* read data  */
	status = prvMpdMdioReadRegisterNoPage(	PRV_MPD_DEBUG_FUNC_NAME_MAC(),
								portEntry_PTR,
								PRV_MPD_MMD_ADDRESS_DATA_REG_CNS,
								data_PTR);
	PRV_MPD_CHECK_RETURN_STATUS_MAC(status, portEntry_PTR->rel_ifIndex);

	return MPD_OK_E;
}

extern MPD_RESULT_ENT prvMpdMdioWrapRead (
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		 mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
    OUT UINT_16    * value_PTR
)
{
	PRV_MPD_PORT_HASH_ENTRY_STC * portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (prvMpdCallBacks.initialized == FALSE) {
		return MPD_OP_FAILED_E;
	}

	if (portEntry_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}
	if (prvMpdCallBacks.mdioRead_PTR == NULL && portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}
	if (portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR) {
		return portEntry_PTR->initData_PTR->mdioInfo.readFunc_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value_PTR)?MPD_OK_E:MPD_OP_FAILED_E;
	}
	return prvMpdCallBacks.mdioRead_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value_PTR)?MPD_OK_E:MPD_OP_FAILED_E;

}

extern MPD_RESULT_ENT prvMpdMdioWrapWrite(
	IN  UINT_32      rel_ifIndex,
	IN  UINT_8		 mdioAddress,
	IN  UINT_16      deviceOrPage,
	IN  UINT_16      address,
    IN  UINT_16      value
)
{
	PRV_MPD_PORT_HASH_ENTRY_STC * portEntry_PTR = prvMpdGetPortEntry(rel_ifIndex);
	if (prvMpdCallBacks.initialized == FALSE) {
		return MPD_OP_FAILED_E;
	}

	if (portEntry_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}
	if (prvMpdCallBacks.mdioWrite_PTR == NULL && portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR == NULL) {
		return MPD_OP_FAILED_E;
	}
	if (portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR) {
		return portEntry_PTR->initData_PTR->mdioInfo.writeFunc_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value)?MPD_OK_E:MPD_OP_FAILED_E;
	}
	return prvMpdCallBacks.mdioWrite_PTR(rel_ifIndex, mdioAddress, deviceOrPage, address, value)?MPD_OK_E:MPD_OP_FAILED_E;

}

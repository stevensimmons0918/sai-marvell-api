/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 *  ppa_fw_record.c
 *
 *  Recording feature functions
 *
 */

#include "ppa_fw_ppn2mg.h"

#if (ENABLE_RECORDING)

/* ===========================
   Global variables section
   =========================== */
uint32_t threads_record_buf[RECORD_BUF_NUM_OF_ENTRIES];
uint32_t* record_buf_ptr;
uint32_t recording_action;
uint32_t rec_first_time;

/********************************************************************************************************************//**
 * funcname        recording_powerup
 * inparam         None
 * return          None
 * description     Initialize recording buffer pointer and recording flags.
 *                 Write constant word with known pattern to signal parser from which point the recording buffer starts
 ************************************************************************************************************************/
void recording_powerup()
{
	/* Set pointer to start of thread record buffer */
	record_buf_ptr = threads_record_buf;
	/* Write constnat pattern to indicate recording is on */
	*record_buf_ptr++ = 0xCAFECAFE;
	/* Initialize recording flags */
	recording_action = REC_OFF;
	rec_first_time   = 1;
}

/********************************************************************************************************************//**
 * funcname        init_record_ptr
 * inparam         None
 * return          None
 * description     Initialize record pointer to the begining of recording buffer. Called at the begining of each thread.
 ************************************************************************************************************************/
void init_record_ptr()
{
	/* Set pointer to start of thread record buffer + 1. Leave first entry for record word indication */
	record_buf_ptr = &threads_record_buf[1];
}

/********************************************************************************************************************//**
 * funcname        turn_on_recording
 * inparam         None
 * return          None
 * description     Enable recording on the fly and only one time. Should be called upon specific event.
 ************************************************************************************************************************/
void turn_on_recording()
{
	if (rec_first_time)
	{
		/* Set recording flag to on to enable recording */
		recording_action = REC_ON;
		/* Clear first time flag to enable recording only once */
		rec_first_time   = 0;
	}
}

/********************************************************************************************************************//**
 * funcname        turn_off_recording
 * inparam         None
 * return          None
 * description     Disable recording on the fly. Should be called upon specific event.
 *                 Send doorbell interrupt to host to indicate it to read recording content from SP memory.
 ************************************************************************************************************************/
void turn_off_recording()
{
	/* Clear recording flag to disable recording */
	recording_action = REC_OFF;
	/* Signal Host to read recording content from SP memory by generating doorbell interrupt towards it */
	ppn2mg_rd_request(PPN2MG_RD_SP_REQUEST,STOP_PPN);
}

/********************************************************************************************************************//**
 * funcname     record_pkt_io
 * inparam      wordInd:    Signal start of packet recording data. Includes known pattern, number of words and thread ID
 *              pktIoPtr:   pointer to header. Can be input or output.
 *              numOfWords: Number of words to read from header.
 * return       None
 * description  Read input/output header parameters and save them in recording buffer in SP memory.
 ************************************************************************************************************************/
void record_pkt_io(uint32_t wordInd, uint32_t* pktIoPtr,uint32_t numOfWords)
{
		uint32_t i;

		/* Word indication: |known pattern|number of words|thread number  */
		*record_buf_ptr++ = wordInd;

		/* Read input/output header words and save them in recording buffer  */
		for ( i=0;i<numOfWords;i++)
		{
    		*record_buf_ptr++ = *pktIoPtr++;
		}
}

#if (EN_REC_THRD0_INPUT || EN_REC_THRD0_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_0_DoNothing
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_0_DoNothing(bool inputOutputRec)
{
    struct nonQcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr         = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x03: number of words for input, 0x00: Thread#0  */
        record_pkt_io(0xD1110300, pkt_io_ptr,3);      /* Save MAC DA and MAC SA (12B) */
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x03: number of words for output, 0x00: Thread#0  */
        record_pkt_io(0xD2220300, pkt_io_ptr,3);
	}

	/* Write done word indication with the thread number  */
	*record_buf_ptr = 0xBEBEBE00 ;
}
#endif

#if (EN_REC_THRD1_INPUT || EN_REC_THRD1_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_1_E2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_1_E2U(bool inputOutputRec)
{
    struct thr1_e2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr1_e2u_cfg);
    struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* cfg_input_ptr           = (uint32_t*)(PIPE_CFG_REGs_lo);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0A: total number of words for input, 0x01: Thread#1  */
        record_pkt_io(0xD1110A01, pkt_io_ptr,3);  /* Save MAC DA and MAC SA (12B) */

		/* Save ETAG (8B) from cfg */
		*record_buf_ptr++ = *cfg_input_ptr++;
    	*record_buf_ptr++ = *cfg_input_ptr++;
		/* Save cfg input parameters: PCID and TPID */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.PCID;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.Uplink_Port;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;

		/* Save descriptor input parameters */
		*record_buf_ptr++ = desc_ptr->up0;
    	*record_buf_ptr++ = desc_ptr->dei;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE01 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x06: total number of words for output, 0x01: Thread#1  */
        record_pkt_io(0xD2220601, pkt_io_ptr,5);  /* Save MAC DA SA and ETAG (20B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE01 ;
	}
}
#endif

#if (EN_REC_THRD2_INPUT || EN_REC_THRD2_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_2_U2E
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_2_U2E(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr2_u2e_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr2_u2e_cfg);
	struct thr2_u2e_in_hdr* rec_structPkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr2_u2e_in_hdr);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0A: total number of words for input, 0x02: Thread#2  */
        record_pkt_io(0xD1110A02, pkt_io_ptr,6);  /* Save MAC DA SA, ETAG and VLAN tag (24B) */

		/* Save cfg targetPortEntry parameters: TPID, PVID and VlanEgrTagState   */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.PCID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TargetPort;
		*record_buf_ptr++ = PPA_FW_SHARED_DRAM_SHORT_READ(2*rec_structPkt_in_ptr->vlan.vid);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE02 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 8: number of words for output, 0x02: Thread#2  */
        record_pkt_io(0xD2220802, pkt_io_ptr,3);   /* Save MAC DA SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Check what happen to the packet and save accordingly */
		if (desc_ptr->fw_bc_modification == -8)
		{   /* Vlan was not removed. Save vlan tag */
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}
		else if (desc_ptr->fw_bc_modification == -12)
		{   /* Etag and vlan tag were removed */
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}
		else
		{   /* packet was dropped (save the input Etag and vlan tags) */
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;
    	*record_buf_ptr++ = desc_ptr->fw_drop;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE02 ;
	}
}
#endif

#if (EN_REC_THRD3_INPUT || EN_REC_THRD3_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_3_U2C
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_3_U2C(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr3_u2c_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr3_u2c_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 6: number of words for input, 0x03: Thread#3  */
        record_pkt_io(0xD1110603, pkt_io_ptr,5);  /* Save MAC DA,MAC SA,ETAG (20B) */

		/* Save PCID from PCID targetPortEntry   */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.PCID;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE03 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 6: number of words for output, 0x03: Thread#3  */
        record_pkt_io(0xD2220603, pkt_io_ptr,3);   /* Save MAC DA and MAC SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Save ETAG if it was not removed (8B) */
		if (desc_ptr->fw_bc_modification == 0)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE03 ;
	}
}
#endif

#if (EN_REC_THRD4_INPUT || EN_REC_THRD4_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_4_ET2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_4_ET2U(bool inputOutputRec)
{
	uint32_t* pkt_io_ptr               = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE);
	uint32_t* cfg_input_ptr            = (uint32_t*)(PIPE_CFG_REGs_lo);
    struct thr4_et2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr4_et2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0: number of words for input, 0x04: Thread#4  */
        record_pkt_io(0xD1110704, pkt_io_ptr,4);  /* Save packet input data:MAC DA,MAC SA,4B vlan tag (16B)  */

		/* Save DSA FWD tag (4B) from cfg  */
		*record_buf_ptr++ = *cfg_input_ptr++;
		/* Save src_port_num and reserved_0 from cfg  */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_port_num;
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE04 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x04: number of words for output, 0x04: Thread#4  */
        record_pkt_io(0xD2220404, pkt_io_ptr,4);   /* Save packet input data MAC DA, MAC SA,4B DSA FWD tag (16B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE04 ;
	}
}
#endif

#if (EN_REC_THRD5_INPUT || EN_REC_THRD5_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_5_EU2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_5_EU2U(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr       = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr               = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
	uint32_t* cfg_input_ptr            = (uint32_t*)(PIPE_CFG_REGs_lo);
    struct thr5_eu2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr5_eu2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x06: number of words for input, 0x05: Thread#5  */
        record_pkt_io(0xD1110605, pkt_io_ptr,3);   /* Save packet input data:MAC DA,MAC SA (12B) */

		/* Save DSA FWD tag (4B) from cfg  */
		*record_buf_ptr++ = *cfg_input_ptr++;
		/* Save src_port_num and reserved_0 from cfg  */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_port_num;
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE05 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x05: number of words for output, 0x05: Thread#5  */
        record_pkt_io(0xD2220505, pkt_io_ptr,4);   /* Save packet input data MAC DA, MAC SA, 4B DSA FWD tag (16B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE05 ;
	}
}
#endif

#if (EN_REC_THRD6_INPUT || EN_REC_THRD6_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_6_U2E
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_6_U2E(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
	uint32_t* cfg_input_ptr           = (uint32_t*)(PIPE_CFG_REGs_lo);
    struct thr6_u2e_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr6_u2e_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0: number of words for input, 0x06: Thread#6  */
        record_pkt_io(0xD1110906, pkt_io_ptr,5);   /* Save packet input data:MAC DA,MAC SA, 8B extended DSA FWD tag (20B) */

		/* Save vlan tag (4B) from cfg  */
		*record_buf_ptr++ = *cfg_input_ptr++;
		/* Save cfg targetPortEntry parameters: TPID, PVID and VlanEgrTagState   */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.PVID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.VlanEgrTagState;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE06 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 8: number of words for output, 0x06: Thread#6  */
        record_pkt_io(0xD2220806, pkt_io_ptr,3);   /* Save packet data MAC DA and MAC SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Save vlan tag if there is (4B) */
		if (desc_ptr->fw_bc_modification == -4)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
		}

		/* In case of drop no header alteration. Save extended DSA FWD tag (8B) */
		if(desc_ptr->fw_drop == 1)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;
    	*record_buf_ptr++ = desc_ptr->fw_drop;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE06 ;
	}
}
#endif

#if (EN_REC_THRD7_INPUT || EN_REC_THRD7_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_7_Mrr2E
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_7_Mrr2E(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr         = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x04: number of words for input, 0x07: Thread#7  */
        record_pkt_io(0xD1110407, pkt_io_ptr,4); /* Save packet input data:MAC DA,MAC SA, 4B DSA FROM_CPU tag (16B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE07 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 6: number of words for output, 0x07: Thread#7  */
        record_pkt_io(0xD2220607, pkt_io_ptr,3);  /* Save packet data MAC DA and MAC SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* In case of drop no header alteration. Save DSA FROM_CPU tag (4B) */
		if(desc_ptr->fw_drop == 1)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;
    	*record_buf_ptr++ = desc_ptr->fw_drop;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE07 ;
	}
}
#endif

#if (EN_REC_THRD8_INPUT || EN_REC_THRD8_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_8_E_V2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_8_E_V2U(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr        = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr8_e_v2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr8_e_v2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0A: number of words for input, 0x08: Thread#8  */
        record_pkt_io(0xD1110A08, pkt_io_ptr,4);   /* Save packet input data:MAC DA,MAC SA, 4B vlan tag (16B) */

		/* Save reserved_0 from cfg  */
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;
		/* Save cfg sourcePortEntry parameters: src_ePort_13_7, src_ePort_6_5 and src_ePort_4_0  */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_ePort_13_7;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_ePort_6_5;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_ePort_4_0;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE08 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x08: number of words for output, 0x08: Thread#8  */
        record_pkt_io(0xD2220808, pkt_io_ptr,7);  /* Save packet data MAC DA, MAC SA, 16B eDSA FWD tag (28B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE08 ;
	}
}
#endif

#if (EN_REC_THRD9_INPUT || EN_REC_THRD9_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_9_E2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_9_E2U(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr9_e2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr9_e2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x09: number of words for input, 0x09: Thread#9  */
        record_pkt_io(0xD1110909, pkt_io_ptr,3);  /* Save packet input data:MAC DA & MAC SA (12B) */

		/* Save reserved_0 from cfg (4B)  */
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;
		/* Save cfg sourcePortEntry parameters: src_ePort_13_7, src_ePort_6_5 and src_ePort_4_0  */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_ePort_13_7;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_ePort_6_5;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.src_ePort_4_0;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE09 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x08: number of words for output, 0x09: Thread#9  */
        record_pkt_io(0xD2220809, pkt_io_ptr,7);  /* Save packet data MAC DA, MAC SA, 16B eDSA FWD tag (28B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE09 ;
	}
}
#endif

#if (EN_REC_THRD10_INPUT || EN_REC_THRD10_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_10_C_V2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_10_C_V2U(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr         = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                 = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr10_c_v2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr10_c_v2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0A: number of words for input, 0x0A: Thread#10  */
        record_pkt_io(0xD1110A0A, pkt_io_ptr,6);  /* Save packet input data:MAC DA, MAC SA, ETAG, vlan (24B) */

		/* Save reserved_0 from cfg (4B)  */
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;
		/* Save reserved2 which includes temp result of E_CID_base + Min ePort-1  */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.reserved2;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0A ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x08: number of words for output, 0x0A: Thread#10  */
        record_pkt_io(0xD222080A, pkt_io_ptr,7);  /* Save packet data MAC DA, MAC SA, 16B eDSA FWD tag (28B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0A ;
	}
}
#endif

#if (EN_REC_THRD11_INPUT || EN_REC_THRD11_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_11_C2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_11_C2U(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr       = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr               = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr11_c2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr11_c2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x09: number of words for input, 0x0B: Thread#11  */
        record_pkt_io(0xD111090B, pkt_io_ptr,5);  /* Save packet input data:MAC DA, MAC SA, ETAG (20B) */

		/* Save reserved_0 from cfg  */
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;
		/* Save reserved2 which includes temp result of E_CID_base + Min ePort-1  */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.reserved2;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0B ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x08: number of words for output, 0x0B: Thread#11  */
        record_pkt_io(0xD222080B, pkt_io_ptr,7); /* Save packet data MAC DA, MAC SA, 16B eDSA FWD tag (28B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0B ;
	}
}
#endif

#if (EN_REC_THRD12_INPUT || EN_REC_THRD12_OUTPUT)
/********************************************************************************************************************//**
 * funcname        record_data_thread_12_U_UC2C
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_12_U_UC2C(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr          = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                  = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr12_u_uc2c_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr12_u_uc2c_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0B: number of words for input, 0x0C: Thread#12  */
        record_pkt_io(0xD1110B0C, pkt_io_ptr,7);  /* Save packet input data:MAC DA, MAC SA, eDSA FWD tag (28B) */

		/* Save trg_ePort_13_11 and trg_ePort_10_0 from cfg */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.trg_ePort_13_11;
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.trg_ePort_10_0;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0C ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 7: number of words for output, 0x0C: Thread#12  */
        record_pkt_io(0xD222070C, pkt_io_ptr,3);   /* Save packet data MAC DA & MAC SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Save ETAG and vlan tags (12B) */
		if (desc_ptr->fw_bc_modification == -4)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0C ;
	}
}
#endif

#if (EN_REC_THRD13_INPUT || EN_REC_THRD13_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_13_U_MC2C
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_13_U_MC2C(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr          = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                  = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr13_u_mc2c_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr13_u_mc2c_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0A: number of words for input, 0x0D: Thread#13  */
        record_pkt_io(0xD1110A0D, pkt_io_ptr,7);  /* Save packet input data:MAC DA, MAC SA, eDSA FWD tag (28B) */

		/* Save parameter cfg reserved_0 */
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0D ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x07: number of words for output, 0x0D: Thread#13  */
        record_pkt_io(0xD222070D, pkt_io_ptr,6);  /* Save packet data MAC DA, MAC SA, ETAG and vlan tags (24B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0D ;
	}
}
#endif

#if (EN_REC_THRD14_INPUT || EN_REC_THRD14_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_14_U_MR2C
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_14_U_MR2C(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr          = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                  = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr14_u_mr2c_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr14_u_mr2c_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0B: number of words for input, 0x0E: Thread#14  */
        record_pkt_io(0xD1110B0E, pkt_io_ptr,7);  /* Save packet input data:MAC DA, MAC SA, eDSA TO_ANALAYZER tag (28B) */

		/* Save trg_ePort_13_11 and trg_ePort_10_0 from cfg */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.trg_ePort_13_11;
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.trg_ePort_10_0;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->tc;
    	*record_buf_ptr++ = (desc_ptr->dp_1<<1) | (desc_ptr->dp_0);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0E ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 7: number of words for output, 0x0E: Thread#14  */
        record_pkt_io(0xD222070E, pkt_io_ptr,3);   /* Save packet data MAC DA & MAC SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Save ETAG and vlan tags (12B) */
		if (desc_ptr->fw_bc_modification == -4)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0E ;
	}
}
#endif

#if (EN_REC_THRD15_INPUT || EN_REC_THRD15_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_15_QCN
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_15_QCN(bool inputOutputRec)
{
	struct Qcn_desc* desc_ptr     = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,Qcn_desc);
	uint32_t* pkt_io_ptr          = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
	struct thr15_qcn_out_hdr* pkt_out_qcn_tag_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr15_qcn_out_hdr);
	struct thr15_qcn_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr15_qcn_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x08: number of words for input, 0x0E: Thread#15  */
        record_pkt_io(0xD111080F, pkt_io_ptr,3);  /* Save packet input data: MAC DA & MAC SA (12B) */

		/* Save reserved bits used for zero padding from cfg */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.zero_0;
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.zero_1;

		/* Save descriptor parameters: congested_port_number, qcn_q_fb and congested_queue_number */
    	*record_buf_ptr++ = desc_ptr->congested_port_number;
    	*record_buf_ptr++ = desc_ptr->qcn_q_fb;
    	*record_buf_ptr++ = desc_ptr->congested_queue_number;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0F ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x14: number of words for output, 0x0F: Thread#15  */
        record_pkt_io(0xD222140F, pkt_io_ptr,5);  /* Save packet data: MAC DA SA + 8B extended DSA tag */

		/* qcn_tag  */
    	*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.EtherType;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Version;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Reserved;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.qFb;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.CPID_63_32;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.CPID_31_16;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.CPID_15_3;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.CPID_2_0;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Qoff;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Qdelta;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Encapsulated_priority;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Encapsulated_MAC_DA_47_32;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Encapsulated_MAC_DA_31_16;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Encapsulated_MAC_DA_15_0;
		*record_buf_ptr++ = pkt_out_qcn_tag_ptr->qcn_tag.Encapsulated_SDU_length;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE0F ;
	}
}
#endif

#if (EN_REC_THRD16_INPUT || EN_REC_THRD16_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_16_U2E
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_16_U2E(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr       = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr               = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
	uint32_t* cfg_input_ptr            = (uint32_t*)(PIPE_CFG_REGs_lo);
    struct thr16_u2e_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr16_u2e_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0B: number of words for input, 0x10: Thread#16  */
        record_pkt_io(0xD1110B10, pkt_io_ptr,7);   /* Save packet input data: MAC DA, MAC SA, eDSA FWD tag (28B) */

		/* Save 4B vlan tag from cfg */
		*record_buf_ptr++ = *cfg_input_ptr;

		/* Save cfg targetPortEntry parameters: TPID, PVID and VlanEgrTagState   */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.PVID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.VlanEgrTagState;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE10 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 5: number of words for output, 0x10: Thread#16  */
        record_pkt_io(0xD2220510, pkt_io_ptr,3);  /* Save packet data MAC DA & MAC SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Save vlan tag if required(4B) */
		if (desc_ptr->fw_bc_modification == -12)
		{
			*record_buf_ptr++ = *pkt_io_ptr++;
		}
		else
		{
			*record_buf_ptr++ = 0;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE10 ;
	}
}
#endif

#if (EN_REC_THRD17_INPUT || EN_REC_THRD17_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_17_U2IPL
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_17_U2IPL(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr         = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 5: number of words for input, 0x11: Thread#17  */
        record_pkt_io(0xD1110511, pkt_io_ptr,5);  /* Save MAC DA,MAC SA,ETAG (20B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE11 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 6: number of words for output, 0x11: Thread#17  */
        record_pkt_io(0xD2220611, pkt_io_ptr,5);   /* Save MAC DA and MAC SA,ETAG (20B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE11 ;
	}
}
#endif
#if (EN_REC_THRD18_INPUT || EN_REC_THRD18_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_18_IPL2IPL
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_18_IPL2IPL(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                   = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr18_ipl2ipl_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr18_ipl2ipl_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 4: number of words for input, 0x12: Thread#18  */
        record_pkt_io(0xD1110412, pkt_io_ptr,3);  /* Save MAC DA,MAC SA (12B) */

		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TargetPort;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE12 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 5: number of words for output, 0x12: Thread#18  */
        record_pkt_io(0xD2220512, pkt_io_ptr,3);   /* Save MAC DA and MAC SA (12B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;
    	*record_buf_ptr++ = desc_ptr->fw_drop;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE12 ;
	}
}
#endif

#if (EN_REC_THRD19_INPUT || EN_REC_THRD19_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_19_E2U_Untagged
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_19_E2U_Untagged(bool inputOutputRec)
{
    struct thr19_e2u_untagged_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr19_e2u_untagged_cfg);
    struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* cfg_input_ptr           = (uint32_t*)(PIPE_CFG_REGs_lo);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0B: total number of words for input, 0x13: Thread#19  */
        record_pkt_io(0xD1110B19, pkt_io_ptr,3);  /* Save MAC DA and MAC SA (12B) */

		/* Save ETAG (8B) from cfg */
		*record_buf_ptr++ = *cfg_input_ptr++;
    	*record_buf_ptr++ = *cfg_input_ptr++;
		/* Save cfg input parameters: PCID and TPID */
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.PCID;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.Uplink_Port;
		*record_buf_ptr++ = cfgStrct_ptr->srcPortEntry.vlan_tag;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;

		/* Save descriptor input parameters */
		*record_buf_ptr++ = desc_ptr->up0;
    	*record_buf_ptr++ = desc_ptr->dei;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE13 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x07: total number of words for output, 0x13: Thread#19  */
        record_pkt_io(0xD2220713, pkt_io_ptr,6);  /* Save MAC DA SA, ETAG and vlan tag (24B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE13 ;
	}
}
#endif

#if (EN_REC_THRD20_INPUT || EN_REC_THRD20_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_20_U2E_M4
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_20_U2E_M4(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr20_u2e_m4_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr20_u2e_m4_cfg);
	struct thr20_u2e_m4_in_hdr* rec_structPkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr20_u2e_m4_in_hdr);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x0D: total number of words for input, 0x14: Thread#20  */
        record_pkt_io(0xD1110D14, pkt_io_ptr,6);  /* Save MAC DA SA, ETAG and VLAN tag (24B) */

		/* Save cfg targetPortEntry parameters: TPID, PVID and VlanEgrTagState   */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.PCID;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_1;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_2;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_3;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TargetPort;
		/* VlanEgrTagState   */
		*record_buf_ptr++ = PPA_FW_SHARED_DRAM_SHORT_READ(2*rec_structPkt_in_ptr->vlan.vid);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE14 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 8: number of words for output, 0x14: Thread#20  */
        record_pkt_io(0xD2220814, pkt_io_ptr,3);   /* Save MAC DA SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Check what happen to the packet and save accordingly */
		if (desc_ptr->fw_bc_modification == -8)
		{   /* Vlan was not removed. Save vlan tag */
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}
		else if (desc_ptr->fw_bc_modification == -12)
		{   /* Etag and vlan tag were removed */
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}
		else
		{   /* packet was dropped (save the input Etag and vlan tags) */
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;
    	*record_buf_ptr++ = desc_ptr->fw_drop;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE14 ;
	}
}
#endif

#if (EN_REC_THRD21_INPUT || EN_REC_THRD21_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_21_U2E_M8
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_21_U2E_M8(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr      = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr              = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr21_u2e_m8_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr21_u2e_m8_cfg);
	struct thr21_u2e_m8_in_hdr* rec_structPkt_in_ptr = PIPE_MEM_CAST(PIPE_PKT_REGs_lo,thr21_u2e_m8_in_hdr);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x11: total number of words for input, 0x15: Thread#21  */
        record_pkt_io(0xD1111115, pkt_io_ptr,6);  /* Save MAC DA SA, ETAG and VLAN tag (24B) */

		/* Save cfg targetPortEntry parameters: TPID, PVID and VlanEgrTagState   */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TPID;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.PCID;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_1;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_2;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_3;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_4;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_5;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_6;
    	*record_buf_ptr++ = cfgStrct_ptr->HA_Table_Extended_Ports.ExtPort_PCID_7;
    	*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.TargetPort;
		/* VlanEgrTagState   */
		*record_buf_ptr++ = PPA_FW_SHARED_DRAM_SHORT_READ(2*rec_structPkt_in_ptr->vlan.vid);

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE15 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 8: number of words for output, 0x15: Thread#21  */
        record_pkt_io(0xD2220815, pkt_io_ptr,3);   /* Save MAC DA SA (12B) */
		pkt_io_ptr +=3; /* Increment local pointer by 3 */

		/* Check what happen to the packet and save accordingly */
		if (desc_ptr->fw_bc_modification == -8)
		{   /* Vlan was not removed. Save vlan tag */
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}
		else if (desc_ptr->fw_bc_modification == -12)
		{   /* Etag and vlan tag were removed */
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
			*record_buf_ptr++ = 0;
		}
		else
		{   /* packet was dropped (save the input Etag and vlan tags) */
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
			*record_buf_ptr++ = *pkt_io_ptr++;
		}

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;
    	*record_buf_ptr++ = desc_ptr->fw_drop;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE15 ;
	}
}
#endif

#if (EN_REC_THRD22_INPUT || EN_REC_THRD22_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_22_Discard_pkt
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_22_Discard_pkt(bool inputOutputRec)
{
    struct nonQcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr         = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x03: number of words for input, 0x16: Thread#22  */
        record_pkt_io(0xD1110316, pkt_io_ptr,3);      /* Save MAC DA and MAC SA (12B) */
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x03: number of words for output, 0x16: Thread#22  */
        record_pkt_io(0xD2220316, pkt_io_ptr,3);
	}

	/* Write done word indication with the thread number  */
	*record_buf_ptr = 0xBEBEBE16 ;
}
#endif

#if (EN_REC_THRD23_INPUT || EN_REC_THRD23_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_23_evb_E2U
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_23_evb_E2U(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                   = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr23_evb_e2u_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr23_evb_e2u_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x09: number of words for input, 0x17: Thread#23  */
        record_pkt_io(0xD1110917, pkt_io_ptr,3);   /* Save packet input data:MAC DA,MAC SA (12B) */

		/* Save reserved_0 from cfg  */
		*record_buf_ptr++ = cfgStrct_ptr->vlan.TPID;
		*record_buf_ptr++ = cfgStrct_ptr->vlan.up;
		*record_buf_ptr++ = cfgStrct_ptr->vlan.cfi;
		*record_buf_ptr++ = cfgStrct_ptr->vlan.vid;

		/* Save descriptor parameters: tc & dp  */
    	*record_buf_ptr++ = desc_ptr->up0;
    	*record_buf_ptr++ = desc_ptr->dei;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE17 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x05: number of words for output, 0x17: Thread#23  */
        record_pkt_io(0xD2220517, pkt_io_ptr,4);  /* Save packet data MAC DA, MAC SA, 4B VLAN tag (16B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE17 ;
	}
}
#endif

#if (EN_REC_THRD24_INPUT || EN_REC_THRD24_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_24_evb_U2E
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_24_evb_U2E(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                   = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr24_evb_u2e_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr24_evb_u2e_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x04: number of words for input, 0x18: Thread#24  */
        record_pkt_io(0xD1110418, pkt_io_ptr,4);   /* Save packet input data:MAC DA,MAC SA and 4B VLAN tag(16B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE18 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x04: number of words for output, 0x18: Thread#24  */
        record_pkt_io(0xD2220418, pkt_io_ptr,3);  /* Save packet data MAC DA, MAC SA(12B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE18 ;
	}
}
#endif

#if (EN_REC_THRD25_INPUT || EN_REC_THRD25_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_25_evb_QCN
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_25_evb_QCN(bool inputOutputRec)
{
	struct Qcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,Qcn_desc);
	uint32_t* pkt_io_ptr      = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);
    struct thr25_evb_qcn_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr25_evb_qcn_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x03: number of words for input, 0x19: Thread#25  */
        record_pkt_io(0xD1110319, pkt_io_ptr,3);   /* Save packet input data:MAC DA,MAC SA(12B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE19 ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x04: number of words for output, 0x18: Thread#25  */
        record_pkt_io(0xD2220419, pkt_io_ptr,7);  /* Save packet data MAC DA, MAC SA vlan tag and part of QCN header */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE19 ;
	}
}
#endif


#if (EN_REC_THRD26_INPUT || EN_REC_THRD26_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_26_pre_da_u2e
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_26_pre_da_u2e(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                   = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x04: number of words for input, 0x1A: Thread#26  */
        record_pkt_io(0xD111041A, pkt_io_ptr,4);   /* Save packet input data:pre da tag,MAC DA,MAC SA(16B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE1A ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x04: number of words for output, 0x1A: Thread#26  */
        record_pkt_io(0xD222041A, pkt_io_ptr,4);  /* Save packet data MAC DA, MAC SA(12B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE1A ;
	}
}
#endif


#if (EN_REC_THRD27_INPUT || EN_REC_THRD27_OUTPUT )
/********************************************************************************************************************//**
 * funcname        record_data_thread_27_pre_da_e2u
 * inparam         None
 * return          None
 * description     Save relevant packet input parameters of header,cfg and descriptor into SP memory
 ************************************************************************************************************************/
void record_data_thread_27_pre_da_e2u(bool inputOutputRec)
{
	struct nonQcn_desc* desc_ptr           = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
	uint32_t* pkt_io_ptr                   = (uint32_t*)(PIPE_PKT_REGs_lo + EXPANSION_SPACE_SIZE - desc_ptr->fw_bc_modification);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (inputOutputRec == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD111: start of input, 0x04: number of words for input, 0x1B: Thread#27  */
        record_pkt_io(0xD111041B, pkt_io_ptr,4);   /* Save packet input data:MAC DA,MAC SA(16B) */

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE1B ;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD222: start of output, 0x04: number of words for output, 0x1B: Thread#27  */
        record_pkt_io(0xD222041B, pkt_io_ptr,4);  /* Save packet data  pre da tag, MAC DA, MAC SA(12B) */

		/* Save descriptor output parameters */
    	*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE1B ;
	}
}
#endif





#if (EN_REC_ECN)
/********************************************************************************************************************//**
 * funcname        record_data_ECN_handler
 * inparam         None
 * return          None
 * description     Save relevant input/output ECN parameters into SP memory
 ************************************************************************************************************************/
void record_data_ECN_handler(struct IPv4_Header* pkt_in_IPv4_ptr, struct IPv6_Header* pkt_in_IPv6_ptr,uint32_t rec_in_out,uint32_t thread_id )
{
	struct nonQcn_desc* desc_ptr = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if (rec_in_out == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD333: start of ECN input, 4: number of words for input, thread_id: thread id  */
		*record_buf_ptr++ = 0xD3330400 + thread_id;

		/* Save L3 offset and IP version */
		*record_buf_ptr++ = desc_ptr->outer_l3_offset;
    	*record_buf_ptr++ = pkt_in_IPv4_ptr->version;

    	if (pkt_in_IPv4_ptr->version == IPv4_VER)
    	{   /* IPv4 version */

			/* Save origin ecn and origin checksum */
    		*record_buf_ptr++ = pkt_in_IPv4_ptr->ecn;
    		*record_buf_ptr++ = pkt_in_IPv4_ptr->header_checksum;
    	}
		else if (pkt_in_IPv6_ptr->version == IPv6_VER)
		{
			/* Save origin ecn */
    		*record_buf_ptr++ = pkt_in_IPv6_ptr->ecn;
    		*record_buf_ptr++ = 0;
		}
		else
		{
			/* Should not be here invalid!!! */
    		*record_buf_ptr++ = 0;
    		*record_buf_ptr++ = 0;
		}

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE00 + thread_id;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD444: start of ECN output, 2: number of words for output, thread_id: thread id  */
		*record_buf_ptr++ = 0xD4440200 + thread_id;

		if (pkt_in_IPv4_ptr->version == IPv4_VER)
    	{   /* IPv4 version */

			/* Save new ecn and new checksum */
    		*record_buf_ptr++ = pkt_in_IPv4_ptr->ecn;
    		*record_buf_ptr++ = pkt_in_IPv4_ptr->header_checksum;
    	}
    	else if (pkt_in_IPv6_ptr->version == IPv6_VER)
		{   /* IPv6 version */

			/* Save new ecn */
    		*record_buf_ptr++ = pkt_in_IPv6_ptr->ecn;
    		*record_buf_ptr++ = 0;
    	}
		else
		{
			/* Should not be here invalid!!! */
    		*record_buf_ptr++ = 0;
    		*record_buf_ptr++ = 0;
		}

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE00 + thread_id;
	}
}
#endif

#if (EN_REC_PTP)
/********************************************************************************************************************//**
 * funcname        record_data_PTP_handler
 * inparam         None
 * return          None
 * description     Save relevant input/output ECN parameters into SP memory
 ************************************************************************************************************************/
void record_data_PTP_handler(uint16_t* ethernet_type_ptr, struct ptp_header* ptp_msg_ptr, uint16_t* udp_cs_ptr, uint32_t rec_in_out, uint32_t thread_id)
{
	struct nonQcn_desc* desc_ptr         = PIPE_MEM_CAST(PIPE_DESC_REGs_lo,nonQcn_desc);
    struct thr_default_cfg* cfgStrct_ptr = PIPE_MEM_CAST(PIPE_CFG_REGs_lo,thr_default_cfg);

	RETURN_IF_RECORDING_IS_DISABLED(recording_action)

	/* Check if for input or for output and record data */
	if ( rec_in_out == REC_INPUT)
	{	/****** INPUT ******/

		/* Word indication: 0xD555: start of PTP input, 0x0C: number of words for output, thread ID  */
		*record_buf_ptr++ = 0xD5550C00 + thread_id;

		/* Save packet ethernet type  */
		*record_buf_ptr++ = *ethernet_type_ptr ;

		/* Save PTP header parameters: PTP message length and origin correction field (64bits)   */
		*record_buf_ptr++ = ptp_msg_ptr->messageLength ;
    	*record_buf_ptr++ = ( (ptp_msg_ptr->correctionField_63_48<<16) | (ptp_msg_ptr->correctionField_47_32&0xFFFF));
    	*record_buf_ptr++ = ( (ptp_msg_ptr->correctionField_31_16<<16) | (ptp_msg_ptr->correctionField_15_0&0xFFFF) );

		/* Save descriptor parameters: timestamp_Sec, timestamp_NanoSec, ptp_offset, outer_l3_offset and fw_bc_modification  */
		*record_buf_ptr++ = desc_ptr->timestamp_Sec;
		*record_buf_ptr++ = desc_ptr->timestamp_NanoSec;
		*record_buf_ptr++ = desc_ptr->ptp_offset;
		*record_buf_ptr++ = desc_ptr->outer_l3_offset;
		*record_buf_ptr++ = desc_ptr->fw_bc_modification;

		/* Save cfg parameters: Egress_Pipe_Delay and reserved_0  */
		*record_buf_ptr++ = cfgStrct_ptr->targetPortEntry.Egress_Pipe_Delay ;
		*record_buf_ptr++ = cfgStrct_ptr->cfgReservedSpace.reserved_0 ;

		/* Save origin udp checksum  */
		*record_buf_ptr++ = *udp_cs_ptr ;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE00 + thread_id;
	}
	else
	{	/****** OUTPUT ******/

		/* Word indication: 0xD666: start of PTP output, 0x10: number of words for output, 0x21: PTP handler  */
		*record_buf_ptr++ = 0xD6661000 + thread_id;

		/* Save PTP header parameters: origin correction field (64bits)  */
    	*record_buf_ptr++ = ( (ptp_msg_ptr->correctionField_63_48<<16) | (ptp_msg_ptr->correctionField_47_32&0xFFFF));
    	*record_buf_ptr++ = ( (ptp_msg_ptr->correctionField_31_16<<16) | (ptp_msg_ptr->correctionField_15_0&0xFFFF) );

		/* Save descriptor output parameters   */
		*record_buf_ptr++ = desc_ptr->ptp_dispatching_en;
		*record_buf_ptr++ = desc_ptr->ptp_packet_format;
		*record_buf_ptr++ = desc_ptr->mac_timestamping_en;
		*record_buf_ptr++ = desc_ptr->ptp_cf_wraparound_check_en;
		*record_buf_ptr++ = desc_ptr->ptp_timestamp_queue_entry_id;
		*record_buf_ptr++ = desc_ptr->egress_pipe_delay;
		*record_buf_ptr++ = desc_ptr->ingress_timestamp_seconds;
		*record_buf_ptr++ = desc_ptr->timestamp_offset;
		*record_buf_ptr++ = desc_ptr->ptp_action;
		*record_buf_ptr++ = desc_ptr->ptp_egress_tai_sel;
		*record_buf_ptr++ = desc_ptr->ptp_timestamp_queue_select;
		*record_buf_ptr++ = desc_ptr->udp_checksum_offset;
		*record_buf_ptr++ = desc_ptr->udp_checksum_update_en;

		/* Save new udp checksum  */
		*record_buf_ptr++ = *udp_cs_ptr ;

		/* Write done word indication with the thread number  */
		*record_buf_ptr = 0xBEBEBE00 + thread_id ;
	}
}
#endif /* EN_REC_PTP */


#endif /* ENABLE_RECORDING */


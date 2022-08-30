/*
 *  ppa_fw_record.h
 *
 *  PPA fw recording definitions
 *
 */


#ifndef PPA_FW_RECORD_H
#define PPA_FW_RECORD_H

/**************************************************************************
 * Constants & Macros
 **************************************************************************/

/* Size of recording buffer is 70*4 = 280 bytes. Currently active maximum size is 62*4 = 248 bytes  */
#define RECORD_BUF_NUM_OF_ENTRIES (70)

/* Thread ID */
enum rec_threads_id{
	REC_THRD_ID_0,           /* 0  */
	REC_THRD_ID_1,           /* 1  */
	REC_THRD_ID_2,           /* 2  */
	REC_THRD_ID_3,           /* 3  */
	REC_THRD_ID_4,           /* 4  */
    REC_THRD_ID_5,           /* 5  */
	REC_THRD_ID_6,           /* 6  */
	REC_THRD_ID_7,           /* 7  */
	REC_THRD_ID_8,           /* 8  */
	REC_THRD_ID_9,           /* 9  */
	REC_THRD_ID_10,          /* 10 */
	REC_THRD_ID_11,          /* 11 */
	REC_THRD_ID_12,          /* 12 */
	REC_THRD_ID_13,          /* 13 */
	REC_THRD_ID_14,          /* 14 */
	REC_THRD_ID_15,          /* 15 */
	REC_THRD_ID_16,          /* 16 */
	REC_THRD_ID_17,          /* 17 */
	REC_THRD_ID_18,          /* 18 */
	REC_THRD_ID_19,          /* 19 */
	REC_THRD_ID_20,          /* 20 */
	REC_THRD_ID_21,          /* 21 */
	REC_THRD_ID_22,          /* 22 */
	REC_THRD_ID_23,          /* 23 */
	REC_THRD_ID_24,          /* 24 */
	REC_THRD_ID_25,          /* 25 */
	REC_THRD_ID_26,          /* 26 */
	REC_THRD_ID_27,          /* 27 */
	REC_THRD_ID_47 = 47,     /* 47 */
	REC_THRD_ID_48           /* 48 */
};


/* Record on and off definitions */
#define	REC_ON	(1)
#define	REC_OFF	(0)

/* Recording direction */
#define	REC_INPUT	(1)
#define	REC_OUTPUT  (0)

/* Recording state of each case in thread */
#define	EN_REC_THRD0_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD0_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD0_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD0_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD1_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD1_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD1_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD1_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD2_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD2_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD2_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD2_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD3_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD3_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD3_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD3_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD4_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD4_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD4_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD4_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD5_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD5_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD5_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD5_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD6_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD6_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD6_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD6_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD7_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD7_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD7_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD7_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD8_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD8_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD8_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD8_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD9_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD9_OUTPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD9_ECN     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD9_PTP     (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD10_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD10_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD10_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD10_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD11_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD11_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD11_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD11_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD12_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD12_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD12_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD12_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD13_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD13_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD13_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD13_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD14_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD14_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD14_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD14_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD15_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD15_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD16_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD16_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD16_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD16_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD17_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD17_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD17_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD17_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD18_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD18_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD18_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD18_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD19_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD19_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD19_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD19_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD20_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD20_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD20_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD20_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD21_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD21_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD21_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD21_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD22_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD22_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD22_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD22_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD23_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD23_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD23_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD23_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD24_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD24_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD24_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD24_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD25_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD25_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD26_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD26_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD26_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD26_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */

#define	EN_REC_THRD27_INPUT	 (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD27_OUTPUT (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD27_ECN    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */
#define	EN_REC_THRD27_PTP    (REC_OFF)                 /*  ( REC_OFF or REC_ON  )  */



#define	EN_REC_THRD47_ECN    (REC_OFF)
#define	EN_REC_THRD47_PTP    (REC_OFF)

#define	EN_REC_THRD48_ECN    (REC_OFF)
#define	EN_REC_THRD48_PTP    (REC_OFF)


/* Recording state of all threads */
#define ENABLE_RECORDING    ( EN_REC_THRD0   || \
							  EN_REC_THRD1   || \
							  EN_REC_THRD2   || \
							  EN_REC_THRD3   || \
							  EN_REC_THRD4   || \
							  EN_REC_THRD5   || \
							  EN_REC_THRD6   || \
							  EN_REC_THRD7   || \
							  EN_REC_THRD8   || \
							  EN_REC_THRD9   || \
							  EN_REC_THRD10  || \
							  EN_REC_THRD11  || \
							  EN_REC_THRD12  || \
							  EN_REC_THRD13  || \
							  EN_REC_THRD14  || \
							  EN_REC_THRD15  || \
							  EN_REC_THRD16  || \
							  EN_REC_THRD17  || \
							  EN_REC_THRD18  || \
							  EN_REC_THRD19  || \
							  EN_REC_THRD20  || \
							  EN_REC_THRD21  || \
							  EN_REC_THRD22  )

/* Recording state of each thread */
#define EN_REC_THRD0  (EN_REC_THRD0_INPUT  || EN_REC_THRD0_OUTPUT  || EN_REC_THRD0_ECN  || EN_REC_THRD0_PTP)
#define EN_REC_THRD1  (EN_REC_THRD1_INPUT  || EN_REC_THRD1_OUTPUT  || EN_REC_THRD1_ECN  || EN_REC_THRD1_PTP)
#define EN_REC_THRD2  (EN_REC_THRD2_INPUT  || EN_REC_THRD2_OUTPUT  || EN_REC_THRD2_ECN  || EN_REC_THRD2_PTP)
#define EN_REC_THRD3  (EN_REC_THRD3_INPUT  || EN_REC_THRD3_OUTPUT  || EN_REC_THRD3_ECN  || EN_REC_THRD3_PTP)
#define EN_REC_THRD4  (EN_REC_THRD4_INPUT  || EN_REC_THRD4_OUTPUT  || EN_REC_THRD4_ECN  || EN_REC_THRD4_PTP)
#define EN_REC_THRD5  (EN_REC_THRD5_INPUT  || EN_REC_THRD5_OUTPUT  || EN_REC_THRD5_ECN  || EN_REC_THRD5_PTP)
#define EN_REC_THRD6  (EN_REC_THRD6_INPUT  || EN_REC_THRD6_OUTPUT  || EN_REC_THRD6_ECN  || EN_REC_THRD6_PTP)
#define EN_REC_THRD7  (EN_REC_THRD7_INPUT  || EN_REC_THRD7_OUTPUT  || EN_REC_THRD7_ECN  || EN_REC_THRD7_PTP)
#define EN_REC_THRD8  (EN_REC_THRD8_INPUT  || EN_REC_THRD8_OUTPUT  || EN_REC_THRD8_ECN  || EN_REC_THRD8_PTP)
#define EN_REC_THRD9  (EN_REC_THRD9_INPUT  || EN_REC_THRD9_OUTPUT  || EN_REC_THRD9_ECN  || EN_REC_THRD9_PTP)
#define EN_REC_THRD10 (EN_REC_THRD10_INPUT || EN_REC_THRD10_OUTPUT || EN_REC_THRD10_ECN || EN_REC_THRD10_PTP)
#define EN_REC_THRD11 (EN_REC_THRD11_INPUT || EN_REC_THRD11_OUTPUT || EN_REC_THRD11_ECN || EN_REC_THRD11_PTP)
#define EN_REC_THRD12 (EN_REC_THRD12_INPUT || EN_REC_THRD12_OUTPUT || EN_REC_THRD12_ECN || EN_REC_THRD12_PTP)
#define EN_REC_THRD13 (EN_REC_THRD13_INPUT || EN_REC_THRD13_OUTPUT || EN_REC_THRD13_ECN || EN_REC_THRD13_PTP)
#define EN_REC_THRD14 (EN_REC_THRD14_INPUT || EN_REC_THRD14_OUTPUT || EN_REC_THRD14_ECN || EN_REC_THRD14_PTP)
#define EN_REC_THRD15 (EN_REC_THRD15_INPUT || EN_REC_THRD15_OUTPUT)
#define EN_REC_THRD16 (EN_REC_THRD16_INPUT || EN_REC_THRD16_OUTPUT || EN_REC_THRD16_ECN || EN_REC_THRD16_PTP)
#define EN_REC_THRD17 (EN_REC_THRD17_INPUT || EN_REC_THRD17_OUTPUT || EN_REC_THRD17_ECN || EN_REC_THRD17_PTP)
#define EN_REC_THRD18 (EN_REC_THRD18_INPUT || EN_REC_THRD18_OUTPUT || EN_REC_THRD18_ECN || EN_REC_THRD18_PTP)
#define EN_REC_THRD19 (EN_REC_THRD19_INPUT || EN_REC_THRD19_OUTPUT || EN_REC_THRD19_ECN || EN_REC_THRD19_PTP)
#define EN_REC_THRD20 (EN_REC_THRD20_INPUT || EN_REC_THRD20_OUTPUT || EN_REC_THRD20_ECN || EN_REC_THRD20_PTP)
#define EN_REC_THRD21 (EN_REC_THRD21_INPUT || EN_REC_THRD21_OUTPUT || EN_REC_THRD21_ECN || EN_REC_THRD21_PTP)
#define EN_REC_THRD22 (EN_REC_THRD22_INPUT || EN_REC_THRD22_OUTPUT || EN_REC_THRD22_ECN || EN_REC_THRD22_PTP)

/* Enable/Disable ECN recording status   */
#define EN_REC_ECN  (EN_REC_THRD0_ECN  || EN_REC_THRD1_ECN || EN_REC_THRD2_ECN  || EN_REC_THRD3_ECN  || EN_REC_THRD4_ECN  || EN_REC_THRD5_ECN  || EN_REC_THRD6_ECN  || EN_REC_THRD7_ECN  || \
					 EN_REC_THRD8_ECN  || EN_REC_THRD9_ECN || EN_REC_THRD10_ECN || EN_REC_THRD11_ECN || EN_REC_THRD12_ECN || EN_REC_THRD13_ECN || EN_REC_THRD14_ECN || EN_REC_THRD16_ECN || \
					 EN_REC_THRD17_ECN || EN_REC_THRD18_ECN || EN_REC_THRD19_ECN || EN_REC_THRD20_ECN || EN_REC_THRD21_ECN || EN_REC_THRD22_ECN)

/* Enable/Disable PTP recording status   */
#define EN_REC_PTP  (EN_REC_THRD0_PTP  || EN_REC_THRD1_PTP || EN_REC_THRD2_PTP  || EN_REC_THRD3_PTP  || EN_REC_THRD4_PTP  || EN_REC_THRD5_PTP  || EN_REC_THRD6_PTP  || EN_REC_THRD7_PTP  || \
					 EN_REC_THRD8_PTP  || EN_REC_THRD9_PTP || EN_REC_THRD10_PTP || EN_REC_THRD11_PTP || EN_REC_THRD12_PTP || EN_REC_THRD13_PTP || EN_REC_THRD14_PTP || EN_REC_THRD16_PTP || \
					 EN_REC_THRD17_PTP || EN_REC_THRD18_PTP || EN_REC_THRD19_PTP || EN_REC_THRD20_PTP || EN_REC_THRD21_PTP || EN_REC_THRD22_PTP)

/* Macro to check if recording is on or off   */
#define	RETURN_IF_RECORDING_IS_DISABLED(recording_action)	if (recording_action == REC_OFF)  \
															{                                 \
	                                                        	return;                       \
	                                                        }

/**************************************************************************
 * Externs
 **************************************************************************/
extern uint32_t threads_record_buf[RECORD_BUF_NUM_OF_ENTRIES];
extern uint32_t* threads_record_ptr;
extern uint32_t enable_recording;
typedef enum { false, true } bool;
/**************************************************************************
 * Function prototypes
 **************************************************************************/
void record_data_thread_0_DoNothing(bool inputOutputRec);
void record_data_thread_1_E2U(bool inputOutputRec);
void record_data_thread_2_U2E(bool inputOutputRec);
void record_data_thread_3_U2C(bool inputOutputRec);
void record_data_thread_4_ET2U(bool inputOutputRec);
void record_data_thread_5_EU2U(bool inputOutputRec);
void record_data_thread_6_U2E(bool inputOutputRec);
void record_data_thread_7_Mrr2E(bool inputOutputRec);
void record_data_thread_8_E_V2U(bool inputOutputRec);
void record_data_thread_9_E2U(bool inputOutputRec);
void record_data_thread_10_C_V2U(bool inputOutputRec);
void record_data_thread_11_C2U(bool inputOutputRec);
void record_data_thread_12_U_UC2C(bool inputOutputRec);
void record_data_thread_13_U_MC2C(bool inputOutputRec);
void record_data_thread_14_U_MR2C(bool inputOutputRec);
void record_data_thread_15_QCN(bool inputOutputRec);
void record_data_thread_16_U2E(bool inputOutputRec);
void record_data_thread_17_U2IPL(bool inputOutputRec);
void record_data_thread_18_IPL2IPL(bool inputOutputRec);
void record_data_thread_19_E2U_Untagged(bool inputOutputRec);
void record_data_thread_20_U2E_M4(bool inputOutputRec);
void record_data_thread_21_U2E_M8(bool inputOutputRec);
void record_data_thread_22_Discard_pkt(bool inputOutputRec);
void record_data_thread_23_evb_E2U(bool inputOutputRec);
void record_data_thread_24_evb_U2E(bool inputOutputRec);
void record_data_thread_25_evb_QCN(bool inputOutputRec);
void record_data_ECN_handler(struct IPv4_Header* pkt_in_IPv4_ptr, struct IPv6_Header* pkt_in_IPv6_ptr,uint32_t rec_in_out,uint32_t thread_id);
void record_data_PTP_handler(uint16_t* ethernet_type_ptr, struct ptp_header* ptp_msg_ptr, uint16_t* udp_cs_ptr, uint32_t rec_in_out, uint32_t rec_thrd_id);
void init_record_ptr();
void recording_powerup();


#endif	/* PPA_FW_RECORD_H */


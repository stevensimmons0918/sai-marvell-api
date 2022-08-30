/*
 *  ppa_fw_logging.c
 *
 *  PPA fw logging definitions
 *
 */


#ifndef PPA_FW_LOGGING_H
#define PPA_FW_LOGGING_H


/**************************************************************************
 * Print action definitions
 **************************************************************************/
#define DISABLE_PRINT     (0)
#define	EN_PRINT_FAST     (1)
#define	EN_PRINT_CYCLIC   (2)
#define	PRINT_ACTION      (DISABLE_PRINT)

/**************************************************************************
 * Constant definitions
 **************************************************************************/
#define	START_PRINT_DATA_BUFFER	(0xABBAABBA)
#define	END_LOG_MSG	            (0x99999999)
#define	NUM_OF_ENTRIES			(64)               /* NOTE: number of entrties must be in 2^n units for the optimized cyclic operation */
#define	CYCLIC_MASK				(NUM_OF_ENTRIES-1)

/* Log message opcodes */
#define	LG_MSG_THRD_0_START     (0xDDDDDD00)     /* Start of thread 0  */
#define	LG_MSG_THRD_0_END       (0xEEEEEE00)     /* End of thread 0  */
#define	LG_MSG_THRD_1_START     (0xDDDDDD01)     /* Start of thread 1 */
#define	LG_MSG_THRD_1_END       (0xEEEEEE01)     /* End of thread 1  */
#define	LG_MSG_THRD_2_START     (0xDDDDDD02)     /* Start of thread 2 */
#define	LG_MSG_THRD_2_END       (0xEEEEEE02)     /* End of thread 2  */
#define	LG_MSG_THRD_3_START     (0xDDDDDD03)     /* Start of thread 3  */
#define	LG_MSG_THRD_3_END       (0xEEEEEE03)     /* End of thread 3 */
#define	LG_MSG_THRD_4_START     (0xDDDDDD04)     /* Start of thread 4 */
#define	LG_MSG_THRD_4_END       (0xEEEEEE04)     /* End of thread 4 */
#define	LG_MSG_THRD_5_START     (0xDDDDDD05)     /* Start of thread 5 */
#define	LG_MSG_THRD_5_END       (0xEEEEEE05)     /* End of thread 5 */
#define	LG_MSG_THRD_6_START     (0xDDDDDD06)     /* Start of thread 6 */
#define	LG_MSG_THRD_6_END       (0xEEEEEE06)     /* End of thread 6 */
#define	LG_MSG_THRD_7_START     (0xDDDDDD07)     /* Start of thread 7 */
#define	LG_MSG_THRD_7_END       (0xEEEEEE07)     /* End of thread 7 */
#define	LG_MSG_THRD_8_START     (0xDDDDDD08)     /* Start of thread 8 */
#define	LG_MSG_THRD_8_END       (0xEEEEEE08)     /* End of thread 8 */
#define	LG_MSG_THRD_9_START     (0xDDDDDD09)     /* Start of thread 9 */
#define	LG_MSG_THRD_9_END       (0xEEEEEE09)     /* End of thread 9 */
#define	LG_MSG_THRD_10_START    (0xDDDDDD0A)     /* Start of thread 10 */
#define	LG_MSG_THRD_10_END      (0xEEEEEE0A)     /* End of thread 10 */
#define	LG_MSG_THRD_11_START    (0xDDDDDD0B)     /* Start of thread 11 */
#define	LG_MSG_THRD_11_END      (0xEEEEEE0B)     /* End of thread 11 */
#define	LG_MSG_THRD_12_START    (0xDDDDDD0C)     /* Start of thread 12 */
#define	LG_MSG_THRD_12_END      (0xEEEEEE0C)     /* End of thread 12 */
#define	LG_MSG_THRD_13_START    (0xDDDDDD0D)     /* Start of thread 13 */
#define	LG_MSG_THRD_13_END      (0xEEEEEE0D)     /* End of thread 13 */
#define	LG_MSG_THRD_14_START    (0xDDDDDD0E)     /* Start of thread 14 */
#define	LG_MSG_THRD_14_END      (0xEEEEEE0E)     /* End of thread 14 */
#define	LG_MSG_THRD_15_START    (0xDDDDDD0F)     /* Start of thread 15 */
#define	LG_MSG_THRD_15_END      (0xEEEEEE0F)     /* End of thread 15 */
#define	LG_MSG_THRD_16_START    (0xDDDDDD10)     /* Start of thread 16 */
#define	LG_MSG_THRD_16_END      (0xEEEEEE10)     /* End of thread 16 */
#define	LG_MSG_DEBUG            (0xdeba9000)      /* Opcode for debug usage */


#if(PRINT_ACTION == EN_PRINT_FAST)

/*****************************************************************************************************************************************
 * Macros for print from 0 parameter till 6 parameters + buffer print.
 * NOTE: The prints are not protected by cyclic operation and therefore it is the user responsibility
 *       to keep the logs inside the buffer and do not cross the borders !!!!!!!!
 *       On the other hand these prints are faster than the cyclic prints so if timing is critical they should be used.
 * Obviously each print add cycles that should be taken into account:   0 params add about ~8 cycles
 *                                                                      1 params add about ~11 cycles
 *                                                                      2 params add about ~14 cycles
 *                                                                      3 params add about ~17 cycles
 *                                                                      4 params add about ~20 cycles
 *                                                                      5 params add about ~23 cycles
 *                                                                      6 params add about ~26 cycles
 *                                                                      buffer print add about ~3 cycles more than dedicated print macros
 *****************************************************************************************************************************************/
#define PRINT_0_PARAMS_FAST(opcode)                                      *print_data_buf_ptr++ = opcode;
#define PRINT_1_PARAMS_FAST(opcode,data1)                                *print_data_buf_ptr++ = opcode;  *print_data_buf_ptr++ = data1;
#define PRINT_2_PARAMS_FAST(opcode,data1,data2)                          *print_data_buf_ptr++ = opcode;  *print_data_buf_ptr++ = data1; *print_data_buf_ptr++ = data2;
#define PRINT_3_PARAMS_FAST(opcode,data1,data2,data3)                    *print_data_buf_ptr++ = opcode;  *print_data_buf_ptr++ = data1; *print_data_buf_ptr++ = data2; *print_data_buf_ptr++ = data3;
#define PRINT_4_PARAMS_FAST(opcode,data1,data2,data3,data4)              *print_data_buf_ptr++ = opcode;  *print_data_buf_ptr++ = data1; *print_data_buf_ptr++ = data2; *print_data_buf_ptr++ = data3; *print_data_buf_ptr++ = data4;
#define PRINT_5_PARAMS_FAST(opcode,data1,data2,data3,data4,data5)        *print_data_buf_ptr++ = opcode;  *print_data_buf_ptr++ = data1; *print_data_buf_ptr++ = data2; *print_data_buf_ptr++ = data3; *print_data_buf_ptr++ = data4; *print_data_buf_ptr++ = data5;
#define PRINT_6_PARAMS_FAST(opcode,data1,data2,data3,data4,data5,data6)  *print_data_buf_ptr++ = opcode;  *print_data_buf_ptr++ = data1; *print_data_buf_ptr++ = data2; *print_data_buf_ptr++ = data3; *print_data_buf_ptr++ = data4; *print_data_buf_ptr++ = data5; *print_data_buf_ptr++ = data6;
#define PRINT_BUFFER_PARAMS_FAST(opcode,src_buf_ptr,num_of_words)        {                                                 \
																	   	 	uint32_t i;                                    \
																	   	 	*print_data_buf_ptr++ = opcode;                \
																       	 	for(i=0;i<num_of_words;i++){                   \
																       	 		*print_data_buf_ptr++ = *src_buf_ptr++;    \
																       	 	}                                              \
																    	 }



/**************************************************************************
 * Set user print macros with fast print macros
 **************************************************************************/
#define PRINT_0_PARAMS(opcode)                                         PRINT_0_PARAMS_FAST(opcode)
#define PRINT_1_PARAMS(opcode,data1)                                   PRINT_1_PARAMS_FAST(opcode,data1)
#define PRINT_2_PARAMS(opcode,data1,data2)                             PRINT_2_PARAMS_FAST(opcode,data1,data2)
#define PRINT_3_PARAMS(opcode,data1,data2,data3)                       PRINT_3_PARAMS_FAST(opcode,data1,data2,data3)
#define PRINT_4_PARAMS(opcode,data1,data2,data3,data4)                 PRINT_4_PARAMS_FAST(opcode,data1,data2,data3,data4)
#define PRINT_5_PARAMS(opcode,data1,data2,data3,data4,data5)           PRINT_5_PARAMS_FAST(opcode,data1,data2,data3,data4,data5)
#define PRINT_6_PARAMS(opcode,data1,data2,data3,data4,data5,data6)     PRINT_6_PARAMS_FAST(opcode,data1,data2,data3,data4,data5,data6)
#define PRINT_BUFFER_PARAMS(opcode,src_buf_ptr,num_of_words)           PRINT_BUFFER_PARAMS_FAST(opcode,src_buf_ptr,num_of_words)

#elif(PRINT_ACTION == EN_PRINT_CYCLIC)

/**************************************************************************
 * print data base structure definitions
 **************************************************************************/
struct print_data_base
{
	uint32_t* print_data_buf_ptr;
	uint32_t print_data_buf_indx;
	uint32_t print_word_indication;
	uint32_t print_data_buf[NUM_OF_ENTRIES];
};


/*****************************************************************************************************************************************
 * Macros for print from 0 parameter till 6 parameters + buffer print.
 * The prints are cyclic protected and kept inside the buffer borders.
 * Obviously each print add cycles that should be taken into account:   0 params add about ~15 cycles
 *                                                                      1 params add about ~21 cycles
 *                                                                      2 params add about ~27 cycles
 *                                                                      3 params add about ~33 cycles
 *                                                                      4 params add about ~39 cycles
 *                                                                      5 params add about ~45 cycles
 *                                                                      6 params add about ~51 cycles
 *                                                                      buffer print add about ~3 cycles more than dedicated print macros
 *****************************************************************************************************************************************/
#define PRINT_0_PARAMS_CYCLIC(opcode)                               print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;           \
																	print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_1_PARAMS_CYCLIC(opcode,data1)                         print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;           \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data1;            \
																	print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_2_PARAMS_CYCLIC(opcode,data1,data2)                   print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;           \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data1;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data2;            \
																	print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_3_PARAMS_CYCLIC(opcode,data1,data2,data3)             print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;           \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data1;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data2;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data3;            \
																	print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_4_PARAMS_CYCLIC(opcode,data1,data2,data3,data4)       print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;           \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data1;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data2;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data3;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data4;            \
																	print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_5_PARAMS_CYCLIC(opcode,data1,data2,data3,data4,data5) print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;           \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data1;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data2;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data3;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data4;            \
																	print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data5;            \
																	print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_6_PARAMS_CYCLIC(opcode,data1,data2,data3,data4,data5,data6)    print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;  \
																			 print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data1;   \
																			 print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data2;   \
																			 print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data3;   \
																			 print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data4;   \
																			 print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data5;   \
																			 print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = data6;   \
																			 print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;

#define PRINT_BUFFER_PARAMS_CYCLIC(opcode,src_buf_ptr, num_of_words)        {                                                                                               \
																			   uint32_t i;                                                                                  \
																			   print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = opcode;              \
																			   for(i=0;i<num_of_words;i++){                                                                 \
																			   		print_db.print_data_buf[(print_db.print_data_buf_indx++)&CYCLIC_MASK] = *src_buf_ptr++; \
																			   }                                                                                            \
																			   print_db.print_data_buf[print_db.print_data_buf_indx&CYCLIC_MASK] = END_LOG_MSG;             \
																			}

/**************************************************************************
 * Set user print macros with cyclic print macros
 **************************************************************************/
#define PRINT_0_PARAMS(opcode)                                         PRINT_0_PARAMS_CYCLIC(opcode)
#define PRINT_1_PARAMS(opcode,data1)                                   PRINT_1_PARAMS_CYCLIC(opcode,data1)
#define PRINT_2_PARAMS(opcode,data1,data2)                             PRINT_2_PARAMS_CYCLIC(opcode,data1,data2)
#define PRINT_3_PARAMS(opcode,data1,data2,data3)                       PRINT_3_PARAMS_CYCLIC(opcode,data1,data2,data3)
#define PRINT_4_PARAMS(opcode,data1,data2,data3,data4)                 PRINT_4_PARAMS_CYCLIC(opcode,data1,data2,data3,data4)
#define PRINT_5_PARAMS(opcode,data1,data2,data3,data4,data5)           PRINT_5_PARAMS_CYCLIC(opcode,data1,data2,data3,data4,data5)
#define PRINT_6_PARAMS(opcode,data1,data2,data3,data4,data5,data6)     PRINT_6_PARAMS_CYCLIC(opcode,data1,data2,data3,data4,data5,data6)
#define PRINT_BUFFER_PARAMS(opcode,src_buf_ptr,num_of_words)           PRINT_BUFFER_PARAMS_CYCLIC(opcode,src_buf_ptr,num_of_words)

#else   /* DISABLED */

/**************************************************************************
 * In case of print action is disabled set user print macros with none
 **************************************************************************/
#define PRINT_0_PARAMS(opcode)
#define PRINT_1_PARAMS(opcode,data1)
#define PRINT_2_PARAMS(opcode,data1,data2)
#define PRINT_3_PARAMS(opcode,data1,data2,data3)
#define PRINT_4_PARAMS(opcode,data1,data2,data3,data4)
#define PRINT_5_PARAMS(opcode,data1,data2,data3,data4,data5)
#define PRINT_6_PARAMS(opcode,data1,data2,data3,data4,data5,data6)
#define PRINT_BUFFER_PARAMS(opcode,src_buf_ptr,num_of_words)

#endif

/* Request MG to read SP memory. Stop PPN so SP won't be overrun */
#define PRINT_RD_REQ_FROM_MG     ppn2mg_rd_request(PPN2MG_RD_SP_REQUEST,STOP_PPN)



/**************************************************************************
 * Function prototypes
 **************************************************************************/
void logging_powerup();

/**************************************************************************
 * Externs
 **************************************************************************/
#if(PRINT_ACTION == EN_PRINT_FAST)
	extern uint32_t print_data_buf[NUM_OF_ENTRIES];
    extern uint32_t* print_data_buf_ptr;
#elif(PRINT_ACTION == EN_PRINT_CYCLIC)
	extern struct print_data_base print_db;
#endif




#endif	/* PPA_FW_LOGGING_H */






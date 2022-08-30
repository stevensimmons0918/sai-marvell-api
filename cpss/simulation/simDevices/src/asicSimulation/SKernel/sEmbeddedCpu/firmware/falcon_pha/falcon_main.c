/*------------------------------------------------------------
(C) Copyright Marvell International Ltd.
All Rights Reserved
------------------------------------------------------------*/

/*
 * falcon_main.c
 *
 *  Main function call falcon power up sequence
 *
 */

#ifdef ASIC_SIMULATION
#ifdef _VISUALC
    #pragma warning(disable: 4214) /* nonstandard extension used : bit field types other than int */
    #pragma warning(disable: 4244) /* conversion from 'uint32_t' to 'uint16_t', possible loss of data */
#endif /*_VISUALC*/
#endif /*ASIC_SIMULATION*/

/* ==========================
  Include headers section
  =========================== */
#ifdef ASIC_SIMULATION
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_base_types.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_defs.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/ppa_fw_exception.h"
#include "asicSimulation/SKernel/sEmbeddedCpu/firmware/falcon_pha/wm_asic_sim_defs.h"
#else
#include "ppa_fw_image_info.h"
#include "ppa_fw_base_types.h"
#include "ppa_fw_defs.h"
#include "ppa_fw_exception.h"
#include "ppa_fw_ppn2mg.h"
#include "ppa_fw_accelerator_commands.h"
#include <xtensa/hal.h>
#include <xtensa/xtruntime.h>
#include <xtensa/sim.h>
#endif /*ASIC_SIMULATION*/


/* ===========================
   Global variables section
   =========================== */
uint32_t falcon_exception_dump_buf[XTENSA_REG_NUM];
uint32_t falcon_stack_end_addr;



/********************************************************************************************************************//**
 * funcname        main
 * inparam         None
 * return          None
 * description     Perform initializations and get the first packet
 ************************************************************************************************************************/
#ifndef ASIC_SIMULATION
int main()  {
    /* xt_iss_client_command("profile","disable"); */
    uint32_t   firstThreadAddr  ;

    /* Initialization of FALCON project */
    falcon_init();

    /* Execute the packet swap sequence to jump to the first Thread */

    /* step 1 - swap instruction, activate packet In machines */
    PPA_FW_SP_WRITE(PKT_SWAP_INST_ADDR,0);
    /* step 2 - flush store before activating the blocking load */
    #pragma flush_memory
    /* step 3 - initialize a15 for pc blocking load.  load start of new packet scenario PC. */
    firstThreadAddr=PPA_FW_SP_READ(PKT_LOAD_PC_INST_ADDR);
    /* steps 4  - jump to start of scenario - start execution */
    asm volatile("JX %0" :: "a"(firstThreadAddr));



    /* Infinite loop (should not stuck here) */
    __asm__ __volatile__(
            "main_swap_Loop:                  \n\t"
            "         J main_swap_Loop        \n\t" );

    /* Call all supported Threads. Should not get in here. Only done for compiler to build the Threads routines so we can get their PC addresses */
    call_threads();

    return 0;
}
#endif /*!ASIC_SIMULATION*/


/********************************************************************************************************************//**
 * funcname        falcon_init
 * inparam         None
 * return          None
 * description     Execute FALCON initialization
 *                 1. Init SHARED DRAM memory
 *                 2. Load all accelerators instructions data into SP memory
 *                 3. Initialize exception buffer
 *                 4. Initialize ppn2mg buffer (not in ASIC simulation)
 *                 5. Set last 2 entries of stack with known pattern for option to check stack overrun (not in ASIC simulation)
 *                 6. Save firmware version in fix location 
 ************************************************************************************************************************/
void    falcon_init()
{
    uint32_t ppn_id;

#ifdef ASIC_SIMULATION
    ppn_id = 0;
#else /*ASIC_SIMULATION*/
    /* get PPN ID register */
    asm("RSR.PRID %0" : "=a"(ppn_id));

    /* Init the last 2 words of the stack with constant known pattern */
	falcon_stack_overflow_checker_power_up();

    /* PPN to MG IF initialization   */
    falcon_ppn2mg_powerup();

    /* Load accelerator commands data into SP memory. NOTE: Can be loaded by host as part of the FW image. */
    PPA_FW(load_accelerator_commands)();

    /* Get falcon fw version and save it into fixed address in SP memory */
    falcon_get_fw_version(FW_IMAGE_ID, ( (VER_YEAR<<24) | (VER_MONTH<<16) | (VER_IN_MONTH<<8) | VER_DEBUG) );
#endif /*!ASIC_SIMULATION*/

    /* Set the ppn id so we will know which PPN is using this SP memory (third word from the beginning of SP) */
    PPA_FW_SP_WRITE(PPN_ID_ADDR,0x66000000 + ppn_id);

    /* Initialize shared data memory (SRAM) */
	falcon_init_shared_data_memory(ppn_id); 
     
    /* PPN exception power up sequence */
	falcon_exception_power_up();
}


/********************************************************************************************************************//**
 * funcname        falcon_exception_power_up
 * inparam         None
 * return          None
 * description     PPN exception power up sequence.
 ************************************************************************************************************************/
void falcon_exception_power_up()
{
    /* Initialize exception status to indicate no exception occurred */
    falcon_exception_dump_buf[EXCEPTION_STATUS] = NO_EXCEPTION_TILL_NOW;
}


/********************************************************************************************************************//**
 * funcname        falcon_init_shared_data_memory
 * inparam         ppn_num: ppn id
 * return          None
 * description     Initialize shared ram memory which is common to all PPNs.
 *                 Use PPN 0 to initialize variables in shared memory and only when done set free the rest of the PPNs
 ************************************************************************************************************************/
/* Force not to optimize this function since it seems that the compiler optimized away the "else" and "while"  */
__attribute__((optimize ("-O0")))
void falcon_init_shared_data_memory(uint32_t ppn_num)
{
    uint32_t dram_init_done_val;

    /* Initializations performed by PPN_0 only */
    if (ppn_num == 0)
    {
        /*************************************************************************************
         * Mainly for initialization of tables and global variables in DRAM memory
         *************************************************************************************/

        #ifdef ENABLE_THR37_Tunnel_Terminated_Data_Traffic
        /* Initialize QL encoding table */
        falcon_queue_length_encoder_init();
        #endif /* ENABLE_THR37_Tunnel_Terminated_Data_Traffic */

#ifdef CC_ERSPAN_CHECK

        #if 1
        /* Set IPv4 header 20 bytes to Shared memory */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 0  , 0x42220100 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 4  , 0x33332222 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 8  , 0x05432000 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 12 , 0x10000001 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 16 , 0x20000002 );

        /* Set Ingress session ID and Egress session ID */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 20 , 0xcc755 );

        /* Set L2 MAC DA and MAC SA */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 24 , 0xaaaaaaaa );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 28 , 0xaaaabbbb );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 32 , 0xbbbbbbbb );
        /* Set TPID,up,cfi and vid */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 36 , 0x8100F987/*0x8100F000*/ );

        /* Set Falcon device ID */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_GLOBAL_ADDR , 0x000b0100 );
        #else
        /* Set IPv6 header 40 bytes to Shared memory */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 0  , 0x62220100 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 4  , 0x33332222 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 8  , 0x10000001 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 12 , 0x20000002 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 16 , 0x30000003 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 20 , 0x40000004 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 24 , 0x50000005 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 28 , 0x60000006 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 32 , 0x70000007 );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 36 , 0x80000008 );

        /* Set Ingress session ID and Egress session ID */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 40 , 0xcc755 );

        /* Set L2 MAC DA and MAC SA */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 44 , 0xaaaaaaaa );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 48 , 0xaaaabbbb );
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 52 , 0xbbbbbbbb );
        /* Set TPID,up,cfi and vid */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_ANALYZER_EPORT_2_TEMPLATE_ADDR + 56 , 0x8100F987/*0x8100F000*/);

        /* Set Falcon device ID */
        PPA_FW_SP_WRITE( DRAM_CC_ERSPAN_GLOBAL_ADDR , 0x000b0100 );
        #endif

#endif

        /* Initialize sFlow source port tables in shared memory. 
           Firmware is the one to maintain these counters and so clear all tables at power up.   
            - Sample Sequence Number table
            - Drops table
            - Enhanced sFlow Sequence number table
        */
        falcon_sFlowTablesInit();

        /* Mark for all the rest of PPNs in SHARED DRAM memory that init procedure is done. */
        /* NOTE: According to Doron Schupper the problem is that the first read that most ppns will get whatever is found in this address of the shared dmem. */
        /*       In Some cases this data is undefined and theoretically can cause a "false" "900d_900d". */
        PPA_FW_SHARED_DRAM_WRITE(DRAM_INIT_DONE_ADDR, DRAM_INIT_DONE_VALUE);
    }
    else
    { /* Wait for ppn0 to finish */
        dram_init_done_val = PPA_FW_SHARED_DRAM_READ(DRAM_INIT_DONE_ADDR);
        while (dram_init_done_val!= DRAM_INIT_DONE_VALUE)
        {
            dram_init_done_val = PPA_FW_SHARED_DRAM_READ(DRAM_INIT_DONE_ADDR);
        }
    }
}

/********************************************************************************************************************//**
 * funcname        falcon_sFlowTablesInit
 * inparam         None
 * return          None
 * description     Clear sFlow tables in shared memory
 *                 - Sample Sequence Number table. 128 entries of 4B each.
 *                 - Drops number table. 128 entries of 2B each.
 *                 - Enhanced sFlow sequence number table. 6 entries per target analyzer ID of 4B each. 
 ************************************************************************************************************************/
void falcon_sFlowTablesInit ()
{
    uint32_t i;

    /* Run over all table entries as the number of ports (128 ports mode) */
    for (i=0;i<128;i++)
    {
        /* Clear Sample Sequence Number table. 128 entries each of 4B */
        PPA_FW_SP_WRITE( DRAM_SFLOW_SOURCE_PORT_SAMPLE_SEQ_NUM_TABLE_ADDR + i*4 , 0x0 );

        /* Clear Drops table. 128 entries each of 2B */
        PPA_FW_SP_SHORT_WRITE( DRAM_SFLOW_SOURCE_PORT_DROPS_TABLE_ADDR + i*2 , 0x0 );
    }

    /* Run over all table entries as the number of target analyzer IDs */
    for (i=0;i<7;i++)
    {
        /* Clear enhanced Sequence Number table. 6 entries each of 4B */
        PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + i*4 , 0x0 );
    }
#if ENHANCED_SFLOW_CHECK
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 0 ,  0x00000000 );
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 4 ,  0x11111111 );
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 8 ,  0x22222222 );
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 12 , 0x33333333 );
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 16 , 0x44444444 );
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 20 , 0x55555555 );
    PPA_FW_SP_WRITE( DRAM_ENHANCED_SFLOW_SEQ_NUM_TABLE_ADDR + 24 , 0x66666666 );
#endif
}



/********************************************************************************************************************//**
 * funcname        falcon_get_fw_version
 * inparam         imageId   PHA fw image ID
 * inparam         fwVersion PHA fw version (| year<<24 | month<<16 | version number within a month<<8 | debug |)
 * return          None
 * description     Get falcon fw version and save it into SP memory
 ************************************************************************************************************************/
void falcon_get_fw_version (uint32_t imageId, uint32_t fwVersion)
{
    /* Set device name */
    PPA_FW_SP_WRITE(FALCON_FW_VERSION_ADDR,  ( ('F'<<24) | ('L'<<16) | ('C'<<8) | 'N' ) );
    /* Set fw version, (VER_MAJOR:year) (VER_MINOR:month) (VER_LOCAL:version number in month) (VER_DEBUG:version number for debug patches) */
    PPA_FW_SP_WRITE(FALCON_FW_VERSION_ADDR+4, fwVersion);
    /* Set fw image ID */
    PPA_FW_SP_BYTE_WRITE(FALCON_FW_IMAGE_ID_ADDR, imageId);
}

#ifndef ASIC_SIMULATION
/********************************************************************************************************************//**
 * funcname        falcon_stack_overflow_checker_power_up
 * inparam         None
 * return          None
 * description     Get the stack end address (goes from high address to low address).
 *                 Set the last 2 words with constant and known pattern.
 ************************************************************************************************************************/
void falcon_stack_overflow_checker_power_up()
{
    /* Get the last sp address and save it in global variable */
    asm volatile("movi  a14, falcon_stack_end_addr \n\t"
                 "movi  a15, _stack_sentry \n\t"    /* Known symbol from memory map which points to the end of the stack */
                 "s32i  a15,  a14, 0 " );           /* Store _stack_sentry inside falcon_stack_end_addr  */


    /* Set the last 2 entries with constant and known pattern (0x900dCAFE) */
    PPA_FW_SP_WRITE(falcon_stack_end_addr-4,STACK_INITIAL_VAL);
    PPA_FW_SP_WRITE(falcon_stack_end_addr,STACK_INITIAL_VAL);
}

/********************************************************************************************************************//**
 * funcname        falcon_stack_overflow_checker
 * inparam         None
 * return          None
 * description     Check if stack was overflowed by reading the last 2 words.
 *                 If known pattern was overrun notify MG by triggering doorbell interrupt.
 ************************************************************************************************************************/
void falcon_stack_overflow_checker(uint32_t thrd_id)
{

    uint32_t    stack_val[2];

    /* Read the last 2 words of the stack */
    stack_val[0] = PPA_FW_SP_READ(falcon_stack_end_addr-4);
    stack_val[1] = PPA_FW_SP_READ(falcon_stack_end_addr);

    if ( (stack_val[0] != STACK_INITIAL_VAL) || (stack_val[1] != STACK_INITIAL_VAL) )
    {
        /* Notify MG that PPN stack is overflowed and add in which thread it occurred  */
        stack_val[0] = thrd_id;
        ppn2mg_failure_event(PPN2MG_PPN_STACK_OVERFLOW_EVENT,stack_val,1, STOP_PPN);
    }
}
#endif /*!ASIC_SIMULATION*/




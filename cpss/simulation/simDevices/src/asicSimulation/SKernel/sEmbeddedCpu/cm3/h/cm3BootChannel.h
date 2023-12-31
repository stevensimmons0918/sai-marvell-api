#ifndef _CM3_BOOT_CHANNEL_H
#define _CM3_BOOT_CHANNEL_H

#define BOOT_CHANEL_VERSION_STRING_LEN (32)
#define MG0_CM3_BASE   (0x3C200000)
#define IPC_RESVD_MAGIC     0x5a5b5c5d

#define _2K             0x00000800
#define _256K           0x00040000

#define BOOT_CHANNEL_SIZE _2K
#define CONF_PROCESSOR_MEMORY_OFFSET 0x80000



/* Taken from freeRTOS/FreeRTOSV7.3.0/FreeRTOS/Demo/ARMv7_AXP_GCC/micro_init/boot_channel.h */
typedef enum {  MICRO_INIT_BOOT_CH_OPCODE_EPROM_LIKE_E = 2,
                MICRO_INIT_BOOT_CH_OPCODE_CLI_LIKE_E,
                MICRO_INIT_BOOT_CH_OPCODE_RERUN_FILES_E,
                MICRO_INIT_BOOT_CH_OPCODE_GET_VERSION,
                MICRO_INIT_BOOT_CH_OPCODE_GET_PORT_STATUS,
                MICRO_INIT_BOOT_CH_OPCODE_GET_ALL_FILES,
                MICRO_INIT_BOOT_CH_OPCODE_SET_PORT_CONFIG = 10,
                MICRO_INIT_BOOT_CH_OPCODE_SET_PRBS,
                MICRO_INIT_BOOT_CH_OPCODE_GET_PRBS,
                MICRO_INIT_BOOT_CH_OPCODE_SET_SERDES_TX,
                MICRO_INIT_BOOT_CH_OPCODE_SET_SERDES_RX,
                MICRO_INIT_BOOT_CH_OPCODE_AP_PORT_CONFIG,
                MICRO_INIT_BOOT_CH_OPCODE_UPGRADE_FW_E = 16,
                MICRO_INIT_BOOT_CH_OPCODE_AP_ADV_RX_CFG = 17,
                MICRO_INIT_BOOT_CH_OPCODE_AP_ADV_TX_CFG = 18,
                MICRO_INIT_BOOT_CH_OPCODE_LOOPBACK_MODE_E = 19,
                MICRO_INIT_BOOT_CH_OPCODE_GET_SERDES_TX = 21,
                MICRO_INIT_BOOT_CH_OPCODE_GET_SERDES_RX = 22,
                MICRO_INIT_BOOT_CH_OPCODE_GET_HA_ADDRESSES = 0x100,
                MICRO_INIT_BOOT_CH_OPCODE_GET_MI_STATE = 0x101,
} MICRO_INIT_BOOT_CH_OPCODE_ENT;


typedef enum {
    CM3_SIM_IPC_SHM_DATA_REGION_BC_HEADER_E,
    CM3_SIM_IPC_SHM_DATA_REGION_BC_DATA_E,
    CM3_SIM_IPC_SHM_DATA_REGION_REGISTERS_E,
    CM3_SIM_IPC_SHM_DATA_REGION_MAGIC_E,
    CM3_SIM_IPC_SHM_DATA_REGION_SIZE_E,
    CM3_SIM_IPC_SHM_DATA_REGION_HEAP_E,
} CM3_SIM_IPC_SHM_DATA_REGION;

typedef enum {  CM3_MICRO_INIT_BOOT_CH_STATUS_REG_MAGIC_KEY_E,
                CM3_MICRO_INIT_BOOT_CH_STATUS_REG_BOOT_STATE_E,
                CM3_MICRO_INIT_BOOT_CH_STATUS_REG_FW_ERR_CODE_E,
                CM3_MICRO_INIT_BOOT_CH_STATUS_REG_OPCODE_ERR_CODE_E,
                CM3_MICRO_INIT_BOOT_CH_STATUS_REG_SPI_LOG_ADDR_E,
                CM3_MICRO_INIT_BOOT_CH_STATUS_REG_FW_DEBUG_FLAGS_E
} CM3_MICRO_INIT_BOOT_CH_STATUS_REGS_ENT;

typedef enum {  MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_IN_PROGRESS_E = 1,
                MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_DONE_E,
                MICRO_INIT_BOOT_CH_BOOT_INIT_STATE_FAILED_E
} CM3_MICRO_INIT_BOOT_CH_BOOT_STATE_ENT;


GT_U32 cm3SimBootChannelAdressGet
(
    GT_U32 unitBase
);


GT_U32 cm3SimIpcRegionOffsetGet
(
    CM3_SIM_IPC_SHM_DATA_REGION region
);

int cm3SimShmIpcBootChannelReady
(
   GT_VOID
);


GT_VOID cm3SimShmIpcInit
(
   GT_VOID
);

int cm3SimBootChannelRecv
(
   GT_VOID
);

void cm3SimShmIpcBootChannelStateSet
(
   CM3_MICRO_INIT_BOOT_CH_BOOT_STATE_ENT state
);

#endif /* _CM3_CONFIG_FILE_PARSER_H */


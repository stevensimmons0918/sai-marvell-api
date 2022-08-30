/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#define VALUE_MAX_INPUT_SIZE 34
#define NUMBER_OF_ID_REGISTERS 8
#define NUMBER_OF_HD_FUSES 8
#define READ_FROM_REGISTERS 0
#define READ_FROM_FILE 1
#define WRITE_TO_REGISTERS 0
#define WRITE_TO_FILE 1




enum mvebu_efuse_errors {
	MVEBU_EFUSE_WRITE_FUSE_PROCEDURE,
	MVEBU_EFUSE_READ_FUSE_PROCEDURE,
	INPUT_VALUE_SAFETY_CHECK,
	MVEBU_EFUSE_SETUP_DB,
	MVEBU_EFUSE_FILE_WRITE,
	MVEBU_EFUSE_FILE_READ,
	MVEBU_EFUSE_VALUE_WRITE,
	MVEBU_EFUSE_VALUE_READ,
	MVEBU_EFUSE_ID_BIT_DUPLICATION,
	MVEBU_EFUSE_FIND_BITMASK_LOCATION,
	MVEBU_EFUSE_SAFE_OPEN_FD,
	MVEBU_EFUSE_FUSE_WRITE,
	MVEBU_EFUSE_APPLY_CHANGES,
	MVEBU_EFUSE_ADDRESSEE_SWITCH_CASE,
	MVEBU_EFUSE_READ_ALL,
	MVEBU_EFUSE_FUSE_READ_CMD,
	MVEBU_EFUSE_INIT_DB,
	MVEBU_EFUSE_DETECT_DEVICE,
	MVEBU_EFUSE_INITIALIZE_RESOURCE};



#ifdef LINUX
	typedef uint8_t mv_u8;
	typedef uint16_t mv_u16;
	typedef uint32_t mv_u32;

#else/*U-Boot*/
	typedef u8 mv_u8;
	typedef u16 mv_u16;
	typedef u32 mv_u32;
#endif

struct SecurityIDFuse_info {
	char *name;
	mv_u32 writeAddr;
	mv_u32 writeBitMask;
	mv_u8 idFuseRange;/*Each fuse belongs to a range that can be locked from further write access*/
	mv_u32 readBitMask;
};
struct SecurityHDFuse_info {
	char *name;
	mv_u16 logicalBitSize;
	mv_u32 startAddr;
};

void mvebu_efuse_status_cmd(void);
int  mvebu_efuse_apply_changes(bool verbo);
void mvebu_efuse_print_help(void);
void mvebu_efuse_print_usage(void);
int mvebu_efuse_init_DB(void);
int mvebu_efuse_read_all(void);
int mvebu_efuse_fuse_read_cmd(char *fusename);
int mvebu_efuse_fuse_write(char *fusename, char *value_arr);
void mvebu_efuse_error_print(int arg_count, ...);
void mvebu_efuse_simulation_control(bool on);


